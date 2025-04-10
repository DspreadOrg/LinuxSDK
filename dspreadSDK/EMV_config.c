#include "pos_sdk.h"
#include "p_up_c.h"
#include "string.h"


void  IntTonHex(int nHex,int Interger,char* Hex)
{
	char zerolen[20]={'\0'};
    char Int_str[10]={'\0'};

	itoa(Interger,Int_str,16);
	memcpy(zerolen,"00000000",8);
	strcat(zerolen,Int_str);
	memcpy(Hex,&zerolen[strlen(zerolen)-nHex*2],nHex*2);
}
int write_evm(char* offset,char* param_data,char* out)
{
    unsigned char offsetBcd[128] = {0x00};
	unsigned char param_dataBcd[1024] = {0x00};
	asc_2_bcd(offset, strlen(offset), offsetBcd, 0);
	asc_2_bcd(param_data, strlen(param_data), param_dataBcd, 0);
	pack_string("CustomParamDataOffset",offsetBcd,strlen(offset)/2);
	pack_string("CustomParamData",param_dataBcd,strlen(param_data)/2);
	return get_dl_package(POS_NEW_COMMAND,0x16, 0xA0, 0X3c,out);
}

int stop_send(int op_flag,int op_type,char* out)
{
	pack_u8("CustomParamOpFlag",op_flag);
	pack_u8("CustomParamOpType",op_type);
	return get_dl_package(POS_NEW_COMMAND,0x16, 0x91, 0X3c,out);

}

int start_send(int op_flag, int op_type,char* data_len, int data_type,char* out)
{
	unsigned char data_lenBcd[1024] = {0x00};
	asc_2_bcd(data_len, strlen(data_len), data_lenBcd, 0);
	pack_u8("CustomParamOpFlag",op_flag);
	pack_u8("CustomParamOpType",op_type);
	pack_string("CustomParamDataLen",data_lenBcd,4);
	pack_u8("CustomParamSrc",0x10);
	return get_dl_package(POS_NEW_COMMAND,0x16, 0x90, 0X3c,out);
}

int update_work_key(char* pik, char* pikCheck, char* trk,char* trkCheck, char* mak, char* makCheck,int keyIndex,char* out)
{

	unsigned char pikdata[512]={'\0'};
	unsigned char pikdataBCD[512]={'\0'};

	unsigned char trkdata[512]={'\0'};
	unsigned char trkdataBCD[512]={'\0'};

	unsigned char makdata[512]={'\0'};
	unsigned char makdataBCD[512]={'\0'};

	memcpy(key_str,piklenBCD,2);
	memcpy(pikdata,pik,strlen(pik));
	strcat(pikdata,pikCheck);
	memcpy(trkdata,trk,strlen(trk));
	strcat(trkdata,trkCheck);
	memcpy(makdata,mak,strlen(mak));
	strcat(makdata,makCheck);

	asc_2_bcd(pikdata, strlen(pikdata), pikdataBCD, 0);
	asc_2_bcd(trkdata, strlen(trkdata), trkdataBCD, 0);
	asc_2_bcd(makdata, strlen(makdata), makdataBCD, 0);

	pack_string("PikAndCheck",pikdataBCD,strlen(pikdata)/2);
	pack_string("TekAndCheck",trkdataBCD,strlen(trkdata)/2);
	pack_string("MakAndCheck",makdataBCD,strlen(makdata)/2);
	pack_u8("TmkKeyIndex",keyIndex);

	return get_dl_package(POS_NEW_COMMAND,0X10,0Xf0,0X3C,out);
}
/**
	 * mPOS更新主密钥
	 * @param step 步骤索引
	 * @param RN1  随机数密文   (注：step=2时需要，其它时候可以传null)
	 * @param RN2  随机数    (注：step=2时需要，其它时候可以传null)
	 * @param masterKey  主密钥密文    (注：step=3时需要，其它时候可以传null)
	 * @param masterKeyCK  主密钥 checkvalue  (注：step=3时需要，其它时候可以传null)
	 * @param timeout 指定超时时间
	 */
int update_master_key1(char* out)
{
	return get_dl_package(POS_NEW_COMMAND,0X16,0Xe0,0X3C,out);
}
int update_master_key2(char* RN1,char* RN2,char* out)
{
	unsigned char RN1bCD[1024]={'\0'};
	unsigned char RN2bCD[1024]={'\0'};
	asc_2_bcd(RN1,strlen(RN1),RN1bCD,0);
	asc_2_bcd(RN2,strlen(RN2),RN2bCD,0);
	pack_string("RN1",RN1bCD,strlen(RN1)/2);
	pack_string("RN2",RN1bCD,strlen(RN1)/2);
	return get_dl_package(POS_NEW_COMMAND,0X16,0Xe1,0X3C,out);
}
int update_master_key3(char* masterkey,char* masterkeyChekValue,int mkindex,char* out)
{
	unsigned char masterkeybCD[1024]={'\0'};
	unsigned char masterkeyChekValueCD[1024]={'\0'};
	asc_2_bcd(masterkey,strlen(masterkey),masterkeybCD,0);
	asc_2_bcd(masterkeyChekValue,strlen(masterkeyChekValue),masterkeyChekValueCD,0);
	pack_string("RN1",masterkeybCD,strlen(masterkey)/2);
	pack_string("RN2",masterkeyChekValueCD,strlen(masterkeyChekValue)/2);
	pack_u8("TmkKeyIndex",mkindex);
	return get_dl_package(POS_NEW_COMMAND,0X16,0Xe2,0X3C,out);
}
int calcuMac_single(char* calcMac,char* out)
{
	unsigned char calcMacBCD[1024]={'\0'};
	asc_2_bcd(calcMac,strlen(calcMac),calcMacBCD,0);
	pack_string("MacData",calcMacBCD,strlen(calcMac)/2);
	return get_dl_package(POS_NEW_COMMAND,0X11,0X10,0X3C,out);
}

int calcuMac_1111(int keyindex,char* calcMac,char* out)
{
	pack_u8("TmkKeyIndex",keyindex);
	unsigned char calcMacBCD[1024]={'\0'};
	asc_2_bcd(calcMac,strlen(calcMac),calcMacBCD,0);
	pack_string("MacData",calcMacBCD,strlen(calcMac)/2);
	return get_dl_package(POS_NEW_COMMAND,0X11,0X11,0X3C,out);
}

int calcuMac_double(char* calcMac,char* out)
{
	unsigned char calcMacBCD[1024]={'\0'};
	asc_2_bcd(calcMac,strlen(calcMac),calcMacBCD,0);
	pack_string("MacData",calcMacBCD,strlen(calcMac)/2);
	return get_dl_package(POS_NEW_COMMAND,0X11,0X12,0X3C,out);
}

int calcuMac_CBC(char* calcMac,char* out)
{
	unsigned char calcMacBCD[1024]={'\0'};
	asc_2_bcd(calcMac,strlen(calcMac),calcMacBCD,0);
	pack_string("MacData",calcMacBCD,strlen(calcMac)/2);
	return get_dl_package(POS_NEW_COMMAND,0X11,0X13,0X3C,out);
}
