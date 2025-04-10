#include "dotrade.h"
extern "C"
{
#include "pos_sdk.h"
}

DoTrade::DoTrade()
{
    tradeMode=NOTUP_NFC_SWIPE_ICC;
    MSRDebitCreditMode=NO_PIN;
    TransType=GOODS;
    emvTransType=GOODS;
    amount=0000;
    cashBackAmount=0000;
    tradeTime="20140816131133FF";
    tradeCurrencyCode="0840";
    Operationtype=NO_DISPLAY;
    Displaytype=DISPLAY_CPY;
}

int DoTrade::DoSwipeOrIcCard(char* out,char*TradeTime)
{
    //qDebug("TradeTime=%s",TradeTime);

    return packSwipeAndIC(tradeMode, TransType,MSRDebitCreditMode,
                          (unsigned char*)TradeTime,DoTrade::CmdTimeOut,(unsigned char*) out);
    //QDateTime time = QDateTime::currentDateTime();
    //int tradetime =time.toTime_t();
    //qDebug("timeT =%d",tradetime);
    //return pack_1620(tradeMode,MSRDebitCreditMode,tradetime,10,out);
}
int DoTrade::PackUserCancel(char* out)
{
    memcpy(out,"\x4D\x00\x06\x28\x00\x00\x05\x00\x00\x66",10);
    return 10;
}
int DoTrade::PackUserReset(char* out)
{
    memcpy(out,"\x4D\x00\x06\x20\x00\x00\xFE\x00\x00\x95",10);
    return 10;
}


int DoTrade::StartEmvForICCard(char* out)
{   /*
    *  mode:0-normal transaction 1-force online
    *  emvTransType: GOODS=0x01,ENQUERY=0x05,CASHBACK=0x04
    *  amount : transaction amount
    *  cashback: cashback amount
    */
   // int mode=0x01;
    amount=100;
    return packSwipeIc(emvTransType,amount,cashBackAmount,(unsigned char*)tradeTime, (unsigned char*)tradeCurrencyCode,DoTrade::CmdTimeOut,(unsigned char*)out);
}

int DoTrade::QueryLatestCommand(char* out)
{
    return packQueryLatestCmdResult((unsigned char*)out);
}

int DoTrade::DoGetPosId(char* out)
{

    return packPosIdInfo((unsigned char*)out);

}


int DoTrade::DoMifare(int comCode, char* out)
{

    return doMifare(comCode,DoTrade::CmdTimeOut,(unsigned char*)out);

}
int DoTrade::DoSetMerchantName(char * mer_name,char* out){
    return setMerchantName((unsigned char*)mer_name,(unsigned char*)out);
}
int DoTrade::DoSetMerchantId(char * mer_id,char* out){
    return setMerchantId((unsigned char*)mer_id,(unsigned char*)out);
}
int DoTrade::DoAlgorithm(palgorithm_param_t params,char* out){
    return lib_Algorithm(params,(unsigned char*)out);
}

int DoTrade::DoSetPosInfo(char* out,char*data)
{

    int Maxlen=0x0F;
    //char *data="world";
    //return packPosDisplay(Operationtype,Displaytype,Maxlen,data,out);

    return customInputDisplay(Operationtype,Displaytype,Maxlen,(unsigned char*)data,DoTrade::CmdTimeOut,(unsigned char*)out);
}

int DoTrade::DoGetPosInfo(char *out)
{
    return packPosInfo((unsigned char*)out);
}

QByteArray DoTrade::get_key_string( char* key)
{
    char* buffer=(char*)malloc(4096*sizeof(char));
    int len=get(key,(unsigned char *)buffer);
    return QByteArray(buffer,len);
}

int DoTrade::test(char*outData)
{
//    return screenDisplay("please input pin", 0x3c,outData);
//    return setBuzzerStatus(1,outData);
    return resetposstatus((unsigned char*)outData);
}

int DoTrade::clear(char*outData)
{
    return screenDisplay((unsigned char*)"", 0x10,(unsigned char*)outData);
}

int DoTrade::WriteBackIc(char *script, char *outData)
{
    return packWriteIc(script,(unsigned char*) outData);
}

int DoTrade::getNfcBatchData( char *out)
{
    return getIccTag( 0, 1, 0,(unsigned char*) "",(unsigned char*) out);
}

int DoTrade::getICCTag(int isEncrypted,int cardType, int tagCount, char* tagArrStr, char *out)
{
    return getIccTag((unsigned char ) isEncrypted,(unsigned char ) cardType,  (unsigned char )tagCount,  (unsigned char *)tagArrStr,(unsigned char*) out);
}
int DoTrade::getBeeperBlink(int buzzerTimes,int operationType,unsigned char* buzzerPeriod,unsigned char* buzzerDuty, char *out)
{
    return getBuzzerBlink( buzzerTimes, operationType,  buzzerPeriod,  buzzerDuty,(unsigned char*) out);
}

int DoTrade::getDevOpt(unsigned char* operationType, unsigned char delay, int timeout, char * out)
{
    unsigned char precode[2]={0};
    return getDevOperate(operationType, precode,  precode, delay,timeout, (unsigned char*)out);
}


int DoTrade::DoGetPin(char *trade_extra, char *out)
{
    return getPinBlock(0,0,6,"please input pin","1234567899876543","12345678",DoTrade::CmdTimeOut,10,(unsigned char*)out);
    //return getPin("20161223",out);
}
int DoTrade::ApduHandle(APDU_PROCESS pro,int isIcc,int timeout,char* out,char*data,int dataLen){
    switch (pro) {
    case POWER_ON:
        if(!isIcc){
            return powerOnNFC(0x00,timeout,(unsigned char*)out);
        }else{
            return powerOnIcc(CARD_IC,0x00,timeout,(unsigned char*)out);
        }
        break;
    case APDU_TRANSMIT:
        if(!isIcc){
            return sendApduByNFC((unsigned char*)data , dataLen,(unsigned int)timeout ,(unsigned char*)out);
        }else{
            return sendApdu((unsigned char*)data , dataLen,(unsigned int)timeout ,(unsigned char*)out);
        }
        break;
    case POWER_OFF:
        if(!isIcc){
            return powerOffNFC(timeout,(unsigned char*)out);
        }else{
            return powerOffIcc(timeout,(unsigned char*)out);
        }
        break;
    default:
        break;
    }
}
