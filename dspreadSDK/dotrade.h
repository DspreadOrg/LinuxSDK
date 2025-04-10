#ifndef DOTRADE_H
#define DOTRADE_H
#include <qstring.h>
#include<QTimer>
extern "C"
{
#include "pos_sdk.h"
}


class DoTrade
{
public:
    DoTrade();
    int DoSwipeOrIcCard(char* out,char*TradeTime);
    int StartEmvForICCard(char* out);
    int QueryLatestCommand(char* out);
    int DoGetPosId( char* out );
    int DoMifare(int comCode,char* out);
    int DoSetPosInfo( char* out,char*data);
    int DoGetPosInfo(char *out);
    QByteArray get_key_string(char* key);
    int test(char *outData);
    int clear(char *outData);
    int WriteBackIc(char *script, char *outData);
    int DoGetPin(char *trade_extra ,char *out );
    int ApduHandle(APDU_PROCESS pro,int isIcc,int timeout,char* out,char*data,int dataLen);
    int getNfcBatchData(char *out);
    int getICCTag(int isEncrypted,int cardType, int tagCount, char* tagArrStr, char *out);
    int DoSetMerchantId(char * mer_id,char* out);
    int DoSetMerchantName(char * mer_name,char* out);
    int DoAlgorithm(palgorithm_param_t params,char* out);
    int getBeeperBlink(int buzzerTimes,int operationType,unsigned char* buzzerPeriod,unsigned char* buzzerDuty, char *out);
    int getDevOpt(unsigned char* operationType, unsigned char delay, int timeout, char * out);
    int PackUserReset(char* out);
    int PackUserCancel(char* out);
    int tradeMode;
    int MSRDebitCreditMode;
    int TransType;
    int emvTransType;
    int amount;
    int cashBackAmount;
    char* tradeTime;
    char* tradeCurrencyCode;
    int  Operationtype;
    int  Displaytype;
    int  Maxlen;
    //QTimer *timer;
    unsigned int CmdTimeOut=60;
};

extern "C"
{
    int is_package_receive_complete();
    void on_char(unsigned char c);
    void on_package(unsigned char* p,int len);
    void setAmount(char* Amount );
    int getCmdId(void);
    int get_response_result();
    void setFormatID(char *format_id);
    void setMifareKeyClass(int keyClass );
    void setMifareBlockAddr(int addr );
    void setMifareKeyValue(unsigned char* keyValue ,int len);
    void setMifareCardUid(unsigned char* cardUid ,int len);
    void setMifareCardData(unsigned char* cardData ,int len);
    void setMifareOperation(int cmd );
    void setMifareQuickAddr(int startAddr,int endAddr );
    void mifareTranstransmission(unsigned char* data,int len);
    int screenDisplay(unsigned char* display, int timeout,unsigned char*out);
    int setBuzzerStatus(unsigned char isBuzzer,unsigned char *out);
    int setPosSleepTime(int time,unsigned char *out);
    int resetposstatus(unsigned char *out);
    int  sendpin(unsigned char* pin, unsigned char pinstatus,unsigned char* out);
    int EMVSelectEMVApp(unsigned char index,unsigned char* out);
   int update_master_key(unsigned char* masterkey,unsigned char* masterkeyChekValue,int mkindex,unsigned char* out);

    int doUpdateIPEKOperation(unsigned char * trackksn,unsigned char * trackipek,unsigned char * trackipekCheckvalue,unsigned char * emvksn,unsigned char * emvipek,unsigned char * emvipekCheckvalue,unsigned char * pinksn,unsigned char * pinipek,unsigned char * pinipekCheckvalue,int keyIndex,unsigned char *out);
}

#endif // DOTRADE_H
