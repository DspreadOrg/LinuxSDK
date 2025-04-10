#ifndef POS_SDK_H
#define POS_SDK_H

#include <stdio.h>
#define MAX_PACKAGE_LEN 4096
enum
{
    MONEY_TYPE__BEING,

    MONEY_TYPE_NONE = 1,
    MONEY_TYPE_RMB,
    MONEY_TYPE_DOLLAR,

    MONEY_TYPE_CUSTOM_STR,


    MONEY_TYPE__END
};

typedef enum _color_t
{
	BLUE,
	YELLOW,
	GREEN,
	RED,
	WHITE,
	CYAN,
	COLOR_END,
}Color_t;

typedef enum _led_CTR_t
{
	LED_OFF,
	LED_ON,
	LED_BLINK,
}Led_CTR_t;

typedef enum {
	AL_DES_ECB_ENCRYPT,
    AL_DES_ECB_DECRYPT,
    AL_DES_CBC_ENCRYPT,
    AL_DES_CBC_DECRYPT,
	AL_AES_ECB_ENCRYPT,
	AL_AES_ECB_DECRYPT,
    AL_AES_CBC_ENCRYPT,
	AL_AES_CBC_DECRYPT,
	AL_MAD5,
	AL_SHA1,
	AL_SHA256,
	AL_SHA512,
	AL_GCM_AES,
	AL_RSA_ENCRYPT,
	AL_RSA_DECRYPT,
	AL_RSA_CALC_SIGN,
	AL_SM2,
	AL_SM4,
	AL_CRC,
	AL_BASE64
}Algorithm_Mode;

typedef struct  _algorithm_param_t{
    Algorithm_Mode mode;
    unsigned char key_type;
    unsigned char* key;
    unsigned long keyLen;
    unsigned char* input;
    unsigned long inputLen;
    unsigned char* iv;
    unsigned long ivLen;
    unsigned char* out;
    unsigned long outSize;
    unsigned char keyindex;
    int  result;
}algorithm_param_t,*palgorithm_param_t;

typedef enum _card_type_t
{
    CARD_NFC = 1,
    CARD_IC ,
    CARD_PSAM ,
}Card_Type_t;

typedef enum {
    SWIPE_ONLY=0X01,
    ICCARD_ONLY=0X02,
    SWIPE_AND_ICCARD=0X03,
    NOT_ALLOW_DEGRADE=0X04,
    SWIPE_ICCARD_NFC=0X05,
    NFC_NOT_ALLOW_DEGRADE=0X06,
    NFC_ONLY=0X07,
    NOTUP_NFC_SWIPE_ICC=0X08
}TRADE_ID;

typedef enum {
    INPUT_PIN,	//input pin after swipe
    NO_PIN,		//no pin after swipe
}MSRDebitCredit_ID;

typedef enum {
    DISPLAY_DIGIT,  //display  digit  in second line
    NO_DISPLAY,     //no need  display
}OperationType_ID;

typedef enum{
    DISPLAY_CPY,		//display  digit  in second line
    DISPLAY_PLAINT,		//no need  display
}Display_ID;

typedef enum {
    CMD_SUC=0X24,
    CMD_BUSY=0X23,
    CMD_TIMEOUT=0X25,
    CMD_NOTSUPPORT=0X35,
    CMD_CONTINUE=0X36,
    CMD_CANCEL=0X28,
    CMD_DECLINE=0X34
}DEVICE_CMD_RESULT;

typedef enum {
    GOODS=0X01,
    CASHBACK=0X04,
    ENQUERY=0X05
}TRADE_TYPE;

typedef enum {
    POWER_ON=0,
    APDU_TRANSMIT,
    POWER_OFF,
}APDU_PROCESS;

typedef enum {
    ENCRYPT_KEY_TMK ,
    ENCRYPT_KEY_TDK ,
    ENCRYPT_KEY_TPK ,
    ENCRYPT_KEY_MCK,
    ENCRYPT_KEY_END
}EncryptLib_keyType;
//-------------greg.c
int setPosSleepTime(int time,unsigned char *out);
int setSystemDateTime(unsigned char* date,unsigned char*out);
int getMagneticTrackPlainText(unsigned char*out);
int isIdle();
int isQposPresent();
int cancelSetAmout();
int screenDisplay(unsigned char* display, int timeout,unsigned char*out);
int customInputDisplay(int operationType, int displayType, int maxLen,unsigned  char* DisplayStr, int timeout,unsigned char* out);
int manualEncrypt(unsigned char* data, int timeout,unsigned char*out);
int manualKsn(int ksnType,unsigned char* out);
int setEmvApp(int nIndex,int timeout,unsigned char* out);
int EmvApp(int nIndex,char* app);
int getAppCount();
int setMerchantId(unsigned char* MerchantId,unsigned char* out);
int setMerchantName(unsigned char* MerchantName,unsigned char* out);
int setIFDSerialNo(unsigned char* IFDSerial,unsigned char* out);

//------------- dspread.c
int packPosIdInfo(unsigned char *out);                                 // get pos id
int packPosInfo(unsigned char *out);                                   // get pos infomation
int packSwipeAndIC(int tradeMode, int tradeType,int MSRDebitCreditMode,unsigned char*TradeTime,int timeout, unsigned char *out);
//int packSwipeAndIC(int tradeMode,int MSRDebitCreditMode,char*TradeTime,int timeout, char *out);
//int packSwipeAndIC(int tradeMode,int MSRDebitCreditMode,int timeout, char *out);                   // polling card  , if magnetic stripe card ,return transaction data. if IC card , call packSwipeIc() to get transaction result
int QF_packSwipeAndIC(int mode,int amount,unsigned char* random,unsigned char *extra, int timeout,unsigned char *out);
int packSwipeIc(int tradeType,int amount, int cashback,unsigned char* tradeTime,unsigned char* tradeCurrencyCode,int timeout,unsigned char *out);   //if IC card,then return transaction result
int packQueryLatestCmdResult(unsigned char *out);                      // wait and query latest result
int packWriteIc(char *script, unsigned char *out);                 // if server validate transactin result and return . the write returned data to pos
int getIccTag(unsigned char encrptMode, unsigned char tagType,unsigned char tagCount,unsigned char *tagList,unsigned char* out);
int getCmdId(void);
int get_dl_package(int cmd_id,int cmd_code, int cmd_sub_code, int delay,unsigned char* out);
int get(char* key,unsigned char *out);
int resetposstatus(unsigned char *out);
int getPin(char *trade_extra ,unsigned char *out );
int getPinBlock(int encryptType, int keyIndex, int maxLen, char * typeFace, char * cardNo, char * data, int waitPinTime,int timeout,unsigned char *out );
int getBuzzerBlink(unsigned char buzzerTimes,unsigned char operationType,unsigned char * buzzerPeriod,unsigned char * buzzerDuty,unsigned char * out);
int getDevOperate(unsigned char *operationType,unsigned char * pre_code ,unsigned char * aft_code,unsigned char delay, int timeout,unsigned char * out);
void setAmount(char* Amount );
void setFormatId(char*     FormatId );
void setAmountIcon(char type,char* Amounticon );
void setTransMode(int mode);
int  sendpin(unsigned char* pin, unsigned char pinstatus,unsigned char* out);
int  cancelPin(unsigned char *out);
int EMVSelectEMVApp(unsigned char index,unsigned char* out);
int is_package_receive_complete();
int packDigitalEnvelopeFragment(int offset,unsigned char* fragment,unsigned char* out);
int unpackDigitalEnvelope(unsigned char *out);
int update_work_key(unsigned char* pik, unsigned char* pikCheck,unsigned char* trk,unsigned char* trkCheck,unsigned char* mak,unsigned char* makCheck,int keyIndex,unsigned char *out);
int update_master_key(unsigned char* masterkey,unsigned char* masterkeyChekValue,int mkindex,unsigned char* out);
int doUpdateIPEKOperation(unsigned char * trackksn,unsigned char * trackipek,unsigned char * trackipekCheckvalue,unsigned char * emvksn,unsigned char * emvipek,unsigned char * emvipekCheckvalue,unsigned char * pinksn,unsigned char * pinipek,unsigned char * pinipekCheckvalue,int keyIndex,unsigned char *out);
int setBuzzerStatus(unsigned char isBuzzer,unsigned char *out);
void on_package(unsigned char* p,int len);

//-------------emv config.c

int customDisplay(char* param_data,unsigned char* out);                                      // display customized message
int write_evm(unsigned char* offset,unsigned char* param_data,unsigned char* out);                             // write emv config file data
int start_send(int op_flag, int op_type,unsigned char* data_len, int data_type,unsigned char* out);   // start write emv command
int stop_send(int op_flag,int op_type,unsigned char* out);                                   // stop write emv command


/* update keys */
int update_work_key(unsigned char* pik,unsigned char* pikCheck,unsigned char* trk, unsigned char*trkCheck, unsigned char* mak, unsigned char* makCheck,int keyIndex, unsigned char* out);
int update_master_key1(unsigned char* out);
int update_master_key2(unsigned char* RN1,unsigned char* RN2,unsigned char* out);
int update_master_key3(unsigned char* masterkey,unsigned char* masterkeyChekValue,int mkindex,unsigned char* out);
// calculate Mac
int calcuMac_single(unsigned char* calcMac,unsigned char* out);
int calcuMac_1111(int keyindex,unsigned char* calcMac,unsigned char* out);
int calcuMac_double(unsigned char* calcMac,unsigned char* out);
int calcuMac_CBC(unsigned char* calcMac,unsigned char* out);
//-------------busnesscard.c  busness card and cpu card read/write

int ReadZRCPUCardDelay(int timeout,unsigned char *out);
int BuyGasInitializeGasV(int gasV,unsigned char* terminalNO, int timeout,unsigned char *out);
int BuyGasDate(unsigned char* toWriteOrdersCPUResult, unsigned char* mackey,unsigned  char* randomCode, int timeout,unsigned char *out);
int readCard(int cardType,unsigned    char* address, int readLen,unsigned char* cardPin, int vender_id , int timeout,unsigned char *out);
int writeCard(int cardType,unsigned char* address,unsigned char* data,unsigned char* cardPin, int isUpdatePinFlag, int vender_id, int timeout,unsigned char *out);
int startPowerWithVender(int vender_id, int cardType , int timeout,unsigned char *out);
int resetCard(int vender_id, int cardType , int timeout,unsigned char *out);

int get_response_result();
void set_tck(unsigned char * new_key);

int doMifare(int comCode,int timeout,unsigned char* out);
void setMifareKeyClass(int keyClass );
void setMifareBlockAddr(int addr );
void setMifareOperation(int cmd );
void setMifareKeyValue(unsigned char* keyValue ,int len);
void setMifareCardUid(unsigned char* cardUid ,int len );
void setMifareCardData(unsigned char* cardData,int len );
void setMifareQuickAddr(int startAddr,int endAddr );
void mifareTranstransmission(unsigned char* data,int len);
int pack_key_data(unsigned char * envelope,unsigned char* out);
void setFormatId(char*     FormatId );
void setCurrency(char*     Currency );
void setCashback(char* Cashback );
void setDecimal( unsigned char Data );
int readGasCard(char cardType,char* address,int readlen,unsigned char *out);
int writeGasCard(char cardType,char* password,char*address,int datalen,char* data,unsigned char *out);
void setAmountIcon(char type,char* Amounticon );
void setForceCVMRequired(unsigned char flag );

int bcd_2_asc(unsigned char * bcd, int asclen, unsigned char * asc, int align);//asc end add /0
int asc_2_bcd(unsigned char * ascstr, int asclen, unsigned char * bcdstr, int align);

int powerOnIcc(Card_Type_t cardtype,char encrptMode,unsigned int timeout ,unsigned char *out );
int sendApdu(unsigned char *cmd , unsigned int len ,unsigned int timeout ,unsigned char *out );
int powerOffIcc(unsigned int timeout ,unsigned char *out );
int powerOnNFC(char encrptMode,unsigned int timeout ,unsigned char *out );
int sendApduByNFC(unsigned char *cmd , unsigned int len ,unsigned int timeout ,unsigned char *out );
int powerOffNFC(unsigned int timeout ,unsigned char *out );
/* there are 2 methods to use algorithm.
 * 1. use the TMK TDK TPK which store in the PED.
 * 2.use the key in input params.  (if palgorithm_param_t->keyLen > 0)
*/
int lib_Algorithm(palgorithm_param_t param,unsigned char *out);
#endif
