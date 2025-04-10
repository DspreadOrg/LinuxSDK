#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sys/stat.h>
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>
#include <dotrade.h>
extern "C"
{
#include "pos_sdk.h"
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->serialPortComboBox->addItem(info.portName());
    ui->transactionTypeComboBox->addItem("GOODS");
    ui->transactionTypeComboBox->addItem("INQUERY");
    ui->transactionTypeComboBox->addItem("CASHBACK");
}

MainWindow::~MainWindow()
{
    serial.clear();
    serial.close();
    delete ui;
}
bool MainWindow::transmit(char* out,int len,int timeout)
{
    int tmplen=0;
    serial.write(out,len);
    if (serial.waitForBytesWritten(1000))
    {
        serial.clear(QSerialPort::AllDirections);
        // write request
        qDebug("write:%s",qPrintable(QByteArray::fromRawData(out, len).toHex()));
        qDebug()<<"timeout ="<<timeout;
        if(serial.waitForReadyRead(timeout*1000))
        {
            QByteArray responseData = serial.readAll();
            while (serial.waitForReadyRead(100))
                responseData += serial.readAll();
            response=responseData.toHex();
            tmplen = response.length();
            //qDebug() << "tmplen : " << tmplen;
            qDebug("read:%s",qPrintable(responseData.toHex()));
            memcpy(out,responseData.data(),response.length());
            on_package((unsigned char *)out,response.length());
            return true;
        }
    }
    else
    {
        return false;
    }
}


/*---------------------------
*transmit:send command by serial
*
*
-----------------------------*/
bool MainWindow::transmit(char* out,int len)
{
    int tmplen=0;
    serial.write(out,len);
    if (serial.waitForBytesWritten(1000))
    {
        serial.clear(QSerialPort::AllDirections);
        // write request
        qDebug("write:%s",qPrintable(QByteArray::fromRawData(out, len).toHex()));
        // read response
//        while (1) {

        int i =0;
        while(i < DoTrade().CmdTimeOut){
            if(serial.waitForReadyRead(1000)){
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(100))
                    responseData += serial.readAll();
                response=responseData.toHex();
                tmplen = response.length();
                //qDebug() << "tmplen : " << tmplen;
                qDebug("read:%s",qPrintable(responseData.toHex()));
                memcpy(out,responseData.data(),response.length());
                on_package((unsigned char *)out,response.length());
                return true;
            }
            QApplication::processEvents();
            i++;
        }
        return false;
    }
    else
    {
        return false;
    }
}
/*--------------------
*
*on_connectButton_clicked:Connect pos
*
---------------------*/

void MainWindow::on_connectButton_clicked()
{

    serial.close();
    portName=ui->serialPortComboBox->currentText();
    serial.setPortName(portName);


    if (!serial.open(QIODevice::ReadWrite))
    {
        ui->statusLabel->setText(tr("Status:Can't open %1, error code %2").arg(portName).arg(serial.error()));
    }
    else
    {
        ui->statusLabel->setText(tr("Status: Running, connected to port %1.").arg(portName));
    }
}
/*------------------
*
*on_posIDButton_clicked: Get posID
-------------------*/

void MainWindow::on_posIDButton_clicked()
{
    char* out=(char*) malloc(1024*sizeof(char));
    int len=dt.DoGetPosId(out);

    transmit(out,len);

    qDebug("PosId:%s",dt.get_key_string("PosId").data());
    ui->resultText->setText(tr("POSID: %1").arg(QString::fromLocal8Bit(dt.get_key_string("PosId"))));
}

/*------------------
*
*on_posInfoButton_clicked: Get POSinfo
*------------------*/
void MainWindow::on_posInfoButton_clicked()
{
    char* out=(char*) malloc(1024*sizeof(char));
    int len=dt.DoGetPosInfo(out);
    transmit(out,len);

    qDebug("BootLoaderVersion:%s",dt.get_key_string("BootLoaderVersion").data());
    qDebug("FirmwareVersion:%s",dt.get_key_string("FirmwareVersion").data());
    qDebug("HardwareVersion:%s",dt.get_key_string("HardwareVersion").data());

    ui->resultText->setText(tr("BootLoaderVersion:  %1\r\n"
                               "FirmwareVersion:    %2\r\n"
                               "HardwareVersion:    %3\r\n")
                            .arg(dt.get_key_string("BootLoaderVersion").data())
                            .arg(dt.get_key_string("FirmwareVersion").data())
                            .arg(dt.get_key_string("HardwareVersion").data())
                            );
}
/*-------------------------
*
*sleep():Sleep
*-------------------------*/
void MainWindow:: sleep(int ms)
{
//#ifdef WIN32
//        Sleep(1000);
//#else
//        sleep(1);
//#endif

    QTime t;
    t.start();
    while(t.elapsed()<100*ms)
        QCoreApplication::processEvents();
}
/*------------------------
*
*on_doTradeButton_clicked():Start Transaction
*-------------------------*/
void MainWindow::on_doTradeButton_clicked()
{
    char* out=(char*) malloc(4096*sizeof(char));
    int len;

    setAmount("1000");
    char time[20];
    int sleep_ms=0;
    QByteArray date=QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toLocal8Bit();
    strcpy(time,date.data());
    qDebug("time=%s",time);
    len=dt.DoSwipeOrIcCard(out,time);
    transmit(out,len);

    ui->resultText->setText("please insert/swipe card");

    while(getCmdId() ==CMD_BUSY)
    {

        qDebug() << "getCmdId() : " << getCmdId();
        sleep(1);
        len=dt.QueryLatestCommand(out);
        transmit(out,len);
    }
    if(getCmdId() !=CMD_SUC){
        qDebug() << "Transaction Terminated, ErrorCode : " << getCmdId();
        return;
    }


    QByteArray inputPinflag=dt.get_key_string("StatusCode");
    qDebug("inputPinflag:%02x",static_cast<int>(inputPinflag[0])&&0xff);
    if(inputPinflag[0] == 0x31){
        qDebug("please input pin ");
        unsigned char pin[7] = { 0x00 };
        memcpy(pin,"1234",strlen("1234"));
        len = sendpin(pin, 0x01, (unsigned char*)out);//0x00:cancel,0x01:input password
        transmit(out,len);

        while(getCmdId() ==CMD_BUSY||getCmdId()==CMD_CONTINUE)
        {

            len=dt.QueryLatestCommand(out);
            transmit(out,len);
            sleep(1);
        }
    }
    QByteArray cardflag=dt.get_key_string("CardStatus");
    qDebug("CardStatus:%02x",static_cast<int>(cardflag[0])&&0xff);

    //insert card
    if(cardflag[0]==1)                                                                          //ICC
    {
        qDebug("IC card inserted");
        int len=dt.StartEmvForICCard(out);
        int sleep_ms=0;
        transmit(out,len);

        while(getCmdId() ==CMD_BUSY||getCmdId()==CMD_CONTINUE)
        {

            len=dt.QueryLatestCommand(out);
            transmit(out,len);
            sleep(1);
        }
        //qDebug() << "get_response_result : " << get_response_result();
        if(get_response_result()==0x02)
        {
            qDebug("multi-app select : ");

            char* mydata= dt.get_key_string("SelectApp").data();
            qDebug() << "AID list : \r\n" <<mydata;

            len = EMVSelectEMVApp(0x00,(unsigned char*) out);//0x00:AID index in list
            transmit(out,len);

            while(getCmdId() ==CMD_BUSY||getCmdId()==CMD_CONTINUE)
            {

                len=dt.QueryLatestCommand(out);
                transmit(out,len);
                sleep(1);
            }

        }

        if(get_response_result()==0x31||get_response_result()==0x32)
        {
            qDebug("please input pin ");
            unsigned char pin[7] = { 0x00 };
            memcpy(pin,"1234",strlen("1234"));
            len = sendpin(pin, 0x01, (unsigned char*)out);//0x00:cancel,0x01:input password
            transmit(out,len);

            while(getCmdId() ==CMD_BUSY||getCmdId()==CMD_CONTINUE)
            {

                len=dt.QueryLatestCommand(out);
                transmit(out,len);
                sleep(1);
            }

        }


        if(getCmdId() !=CMD_SUC){
            qDebug() << "Transaction Terminated, ErrorCode : " << getCmdId();
            return;
        }

        char* mydata= dt.get_key_string("EncrptMode").data();
        qDebug("EncrptMode:%s",mydata);
        qDebug("IccDataType:%s",dt.get_key_string("IccDataType").data());
        qDebug("CardHolder:%s",dt.get_key_string("CardHolder").data());
        qDebug("PinBlock:%s",dt.get_key_string("PinBlock").data());
        qDebug("MaskPan:%s",dt.get_key_string("MaskPan").data());
        cardflag=dt.get_key_string("IccData").toHex();
        qDebug("IccData:%s",QString::fromLocal8Bit(cardflag));
        ui->resultText->setText(tr("IccData: %1.")
                             .arg(QString::fromLocal8Bit(dt.get_key_string("IccData").toHex())));
        char *res="8A023030";
        len=dt.WriteBackIc(res,out);
        sleep_ms=1000;
        transmit(out,len);

        while(getCmdId() ==CMD_BUSY||getCmdId()==CMD_CONTINUE)
        {

            len=dt.QueryLatestCommand(out);
            transmit(out,len);
            sleep(1);
        }

        if(getCmdId() !=CMD_SUC){
            qDebug() << "Transaction Terminated, ErrorCode : " << getCmdId();
            return;
        }
    }
    else if(cardflag[0]==0)//Swipe Card
    {
        qDebug("Swipe card:\r\n");
        qDebug("FormatId:%s",dt.get_key_string("FormatId").data());
        qDebug("CardExpire:%s",dt.get_key_string("CardExpire").data());
        qDebug("ServiceCode:%s",dt.get_key_string("ServiceCode").data());
        qDebug("CardHolder:%s",dt.get_key_string("CardHolder").data());
        qDebug("track1 data:%s",dt.get_key_string("Magic1").toHex());
        qDebug("track2 data:%s",dt.get_key_string("Magic2").toHex());
        qDebug("track3 data:%s",dt.get_key_string("Magic3").toHex());
        ui->resultText->setText(tr("CardHolder: %1\n\r"
                                   "TrackKsn :    %2\n\r"
                                   "track1 :    %3\n\r"
                                   "track2 :    %4\n\r"
                                   "track3 :    %5\n\r")
                                .arg(QString::fromLocal8Bit(dt.get_key_string("CardHolder")))
                                .arg(QString::fromLocal8Bit(dt.get_key_string("TrackKsn").toHex()).toUpper())
                                .arg(QString::fromLocal8Bit(dt.get_key_string("Magic1").toHex()).toUpper())
                                .arg(QString::fromLocal8Bit(dt.get_key_string("Magic2").toHex()).toUpper())
                                .arg(QString::fromLocal8Bit(dt.get_key_string("Magic3").toHex()).toUpper())
                                );
    }
    //NFC online=03     nfc reject=05
    else if(cardflag[0]==3||cardflag[0]==5)
    {
        qDebug("Tap card:\r\n");
        qDebug("FormatId:%s",dt.get_key_string("FormatId").data());
        qDebug("CardExpire:%s",dt.get_key_string("CardExpire").data());
        qDebug("ServiceCode:%s",dt.get_key_string("ServiceCode").data());
        qDebug("CardHolder:%s",dt.get_key_string("CardHolder").data());
        qDebug()<<"track1 data:"<< QString(dt.get_key_string("Magic1").toHex());
        qDebug()<<"track2 data:"<< QString(dt.get_key_string("Magic2").toHex());
        qDebug()<<"track3 data:"<< QString(dt.get_key_string("Magic3").toHex());
        ui->resultText->setText(tr("CardHolder: %1\n\r"
                                   "TrackKsn :    %2\n\r"
                                   "track1 :    %3\n\r"
                                   "track2 :    %4\n\r"
                                   "track3 :    %5\n\r"
                                   "track3 :    %5\n\r")
                                .arg(QString::fromLocal8Bit(dt.get_key_string("CardHolder")))
                                .arg(QString::fromLocal8Bit(dt.get_key_string("TrackKsn").toHex()).toUpper())
                                .arg(QString::fromLocal8Bit(dt.get_key_string("Magic1").toHex()).toUpper())
                                .arg(QString::fromLocal8Bit(dt.get_key_string("Magic2").toHex()).toUpper())
                                .arg(QString::fromLocal8Bit(dt.get_key_string("Magic3").toHex()).toUpper())
                                );

        len=dt.getNfcBatchData(out);
        sleep_ms=1;
        transmit(out,len);

        while(getCmdId() ==CMD_BUSY)
        {

            len=dt.QueryLatestCommand(out);
            transmit(out,len);
            sleep(1);
        }

        if(getCmdId() !=CMD_SUC){
            qDebug() << "Transaction Terminated, ErrorCode : " << getCmdId();
            return;
        }
        ui->resultText->append(tr("NfcBatchData: %1.")
                             .arg(QString::fromLocal8Bit(dt.get_key_string("TagList").toHex())));
    }
}


void MainWindow::on_transactionTypeComboBox_currentIndexChanged(int index)
{
    switch(index)
    {
        case 0:
            dt.emvTransType=GOODS;
            dt.TransType=GOODS;
            break;

        case 1:
            dt.emvTransType=ENQUERY;
            dt.TransType=ENQUERY;
            break;
        case 2:
            dt.emvTransType=CASHBACK;
            dt.TransType=CASHBACK;
            break;
        default:
            break;

    }
}




QString MainWindow::ByteArrayToHexString(QByteArray data){
    QString ret(data.toHex().toUpper());
    int len = ret.length()/2;

    for(int i=1;i<len;i++)
    {

        ret.insert(2*i+i-1," ");
    }

    return ret;
}


/*
 * @breif ��16�����ַ���ת��Ϊ��Ӧ���ֽ�����
 */
QByteArray MainWindow::HexStringToByteArray(QString HexString)
{
    bool ok;
    QByteArray ret;
    HexString = HexString.trimmed();
    HexString = HexString.simplified();
    QStringList sl = HexString.split(" ");

    foreach (QString s, sl) {
        if(!s.isEmpty())
        {
            char c = s.toInt(&ok,16)&0xFF;
            if(ok){
                ret.append(c);
            }
        }
    }
    return ret;
}

/************ Mifare card Function*********
|
|Mifare card Operation:
|    1.poll card
|    2.vertify card
|    3.read card or write card
|    4. Finish Transaction
|
*******************************************/

/*--------------------
*on_pullCardButton_clicked():Poll call
*
---------------------*/
void MainWindow::on_pullCardButton_clicked()
{
    char* out=(char*) malloc(1024*sizeof(char));
    int len=dt.DoMifare(0x01,out); // pull card,Command code:0x01
//    int len=dt.test(out);

    bool flag = transmit(out,len);

    while (getCmdId()==0x00) {

      transmit(0x00,1);
      sleep(1);
    }


    //if(!flag) return;


    QString mifarecmd = ByteArrayToHexString(dt.get_key_string("mifare_cmd"));
    //    qDebug() << mifarecmd.toStdString().c_str();

    //The status code returned by the command execution, 00:success
    QString mifarestatus = ByteArrayToHexString(dt.get_key_string("mifare_status"));

    QString mifarecardType = ByteArrayToHexString(dt.get_key_string("mifare_cardType"));

    QString mifareATQA = ByteArrayToHexString(dt.get_key_string("mifare_ATQA"));

    QString mifareSAK = ByteArrayToHexString(dt.get_key_string("mifare_SAK"));


    cardUid = dt.get_key_string("mifare_cardUid");

    QString mifarecardUid = ByteArrayToHexString(dt.get_key_string("mifare_cardUid"));


    QString result = "mifarecmd : "+mifarecmd;
    result.append("\r\n");
    result.append("mifarestatus : "+mifarestatus);
    result.append("\r\n");
    result.append("mifarecardType : "+mifarecardType);
    result.append("\r\n");
    result.append("mifareATQA : "+mifareATQA);
    result.append("\r\n");
    result.append("mifareSAK : "+mifareSAK);
    result.append("\r\n");
    result.append("mifarecardUid : "+mifarecardUid);
    result.append("\r\n");

    if(mifarestatus == "00")
    {

      result.append("successful");

    }
    else
    {

      result.append("failure");

    }

    ui->resultText->setText(result.toStdString().c_str());

}

/*--------------------
*on_verifyButton_clicked():Vertify card
*
*--------------------*/
void MainWindow::on_verifyButton_clicked()
{
    char* out=(char*) malloc(1024*sizeof(char));


    setMifareKeyClass(0x60);   // Key class


    setMifareBlockAddr(0x1c);   // block address

    //    QString uidstr = "c4 01 b8 4d";
    //    QByteArray cardUid = HexStringToByteArray(uidstr);
    setMifareCardUid((unsigned char*)cardUid.data(),cardUid.length()); //  cardUid

    QString valuestr = "ff ff ff ff ff ff";
    QByteArray keyValue = HexStringToByteArray(valuestr);
    setMifareKeyValue((unsigned char*)keyValue.data(),keyValue.length());

    int len=dt.DoMifare(0x02,out);

    bool flag = transmit(out,len);
    if(!flag) return;

    QString mifarecmd = ByteArrayToHexString(dt.get_key_string("mifare_cmd"));
    QString mifarestatus = ByteArrayToHexString(dt.get_key_string("mifare_status"));
    QString result = "mifarecmd : "+mifarecmd;
    result.append("\r\n");
    result.append("mifarestatus : "+mifarestatus);
    result.append("\r\n");
    if(mifarestatus == "00")
    {

      result.append("successful");

    }
    else
    {

      result.append("failure");

    }
     ui->resultText->setText(result.toStdString().c_str());
}

/*--------------------
*
*on_finishButton_clicked():Finish Transaction
*----------------------*/
void MainWindow::on_finishButton_clicked()
{
    char* out=(char*) malloc(1024*sizeof(char));
    int len=dt.test(out);

    bool flag = transmit(out,len);
    if(!flag) return;
    QString mifarecmd = ByteArrayToHexString(dt.get_key_string("mifare_cmd"));
    QString mifarestatus = ByteArrayToHexString(dt.get_key_string("mifare_status"));
    QString result = "mifarecmd : "+mifarecmd;
    result.append("\r\n");
    result.append("mifarestatus : "+mifarestatus);
    result.append("\r\n");
    if(mifarestatus == "00")
    {

        result.append("successful");

    }
    else
    {

        result.append("failure");

    }

    ui->resultText->setText(result.toStdString().c_str());


}

/*-----------------------
*on_readCardButton_clicked():read Card
*
*-----------------------*/
void MainWindow::on_readCardButton_clicked()
{
    char* out=(char*) malloc(1024*sizeof(char));
    setMifareBlockAddr(0x1c);
    int len=dt.DoMifare(0x03,out);

    bool flag = transmit(out,len);
    if(!flag) return;

    QString mifarecmd = ByteArrayToHexString(dt.get_key_string("mifare_cmd"));
    QString mifarecardData = ByteArrayToHexString(dt.get_key_string("mifare_cardData"));
    QString mifarestatus = ByteArrayToHexString(dt.get_key_string("mifare_status"));
    QString result = "mifarecmd : "+mifarecmd;
    result.append("\r\n");
    result.append("mifarestatus : "+mifarestatus);
    result.append("\r\n");
    result.append("mifarecardData : "+mifarecardData);
    result.append("\r\n");

    if(mifarestatus == "00")
    {

        result.append("successful");

    }
    else
    {

        result.append("failure");

    }
    ui->resultText->setText(result.toStdString().c_str());

}

/*-----------------------
*on_writeCardButton_clicked():Write card
*
*-----------------------*/
void MainWindow::on_writeCardButton_clicked()
{
    char* out=(char*) malloc(1024*sizeof(char));
    setMifareBlockAddr(0x1c);
    QString cardDatastr = ui->cardDateText->toPlainText();
    QByteArray cardData = HexStringToByteArray(cardDatastr);
    setMifareCardData((unsigned char*)cardData.data(),cardData.length());
    int len=dt.DoMifare(0x04,out);

    bool flag = transmit(out,len);
    if(!flag) return;
    QString mifarecmd = ByteArrayToHexString(dt.get_key_string("mifare_cmd"));
    QString mifareaddr = ByteArrayToHexString(dt.get_key_string("mifare_addr"));
    QString mifarestatus = ByteArrayToHexString(dt.get_key_string("mifare_status"));
    QString result = "mifarecmd : "+mifarecmd;
    result.append("\r\n");
    result.append("mifarestatus : "+mifarestatus);
    result.append("\r\n");
    result.append("mifareaddr : "+mifareaddr);
    result.append("\r\n");

    if(mifarestatus == "00")
    {

        result.append("successful");

    }
    else
    {

        result.append("failure");

    }

    ui->resultText->setText(result.toStdString().c_str());

}
void MainWindow::test_Buzzer(){
    bool flag = false;
    int ret =-1;
    unsigned char duty[]={0x00,0xa0};
    unsigned char period[]={0x00,0xa0};
    unsigned char beep_times=2;
    char* out=(char*) malloc(1024*sizeof(char));
    ret = dt.getBeeperBlink(beep_times,0,period,duty, out);

    flag = transmit(out,ret);
    qDebug()<<"flag  = "<<flag ;
    qDebug()<<"ret  = "<<ret ;
    free(out);
}
void MainWindow::test_ShutDown(){
    bool flag = false;
    int ret =-1;

    unsigned char opt[]={0x00,0x00};//shutdown
//    unsigned char opt[]={0x00,0x02};//reboot
    unsigned char delay_exec_s = 15;
    int timeout= 30;
    char* out=(char*) malloc(1024*sizeof(char));

    ret = dt.getDevOpt(opt, delay_exec_s, timeout, out);

    flag = transmit(out,ret);
    qDebug()<<"flag  = "<<flag ;
    qDebug()<<"ret  = "<<ret ;
    free(out);
}

void MainWindow::on_bt_apdu_icc_clicked()
{
    unsigned int timeout_powerOff = 10;
    unsigned int timeout_powerOn = 20;
    unsigned int timeout_APDU = 10;
    int isIcc = 1;
    char* out=(char*) malloc(1024*sizeof(char));
    int len =0;
    bool flag = false;
    int ret =-1;
    char apdu[1024]={0};
    int apduLen =24;

    //power on
    ret = dt.ApduHandle(POWER_ON,isIcc,timeout_powerOn,out,NULL,0);
    qDebug()<<"power on ret = "<<ret;

    flag = transmit(out,ret);
    while(getCmdId() ==CMD_BUSY ||getCmdId() ==CMD_CONTINUE)
    {
        qDebug() << "getCmdId() : " << getCmdId();
        sleep(1);
        len=dt.QueryLatestCommand(out);
        transmit(out,len,timeout_powerOn);
    }
    if(!flag) return;

    if(getCmdId() == 0 || getCmdId() == CMD_CANCEL || getCmdId() == CMD_TIMEOUT ){
        goto END;
    }

    qDebug()<<"has card:"<<ByteArrayToHexString(dt.get_key_string("HasCard"));
    qDebug()<<"ksn:"<<ByteArrayToHexString(dt.get_key_string("KSN"));
    qDebug()<<"Atr:"<<ByteArrayToHexString(dt.get_key_string("Atr"));
    sleep(1);

    //apdu
    memcpy(apdu,"\x77\x65\x6C\x63\x6F\x6D\x65\x20\x75\x73\x65\x20\x64\x73\x70\x72\x65\x61\x64\x20\x70\x6F\x73\x2E",apduLen);
    ret = dt.ApduHandle(APDU_TRANSMIT,isIcc,10,out,apdu,apduLen);

    qDebug()<<"apdu ret = "<<ret;
    flag = transmit(out,ret);
    while(getCmdId() ==CMD_BUSY)
    {
        qDebug() << "getCmdId() : " << getCmdId();
        sleep(1);
        len=dt.QueryLatestCommand(out);
        transmit(out,len);
    }

    if(!flag) goto END;// finally  close the card reader
    sleep(1);
    qDebug()<<"ApduResult:"<<ByteArrayToHexString(dt.get_key_string("ApduResult"));
    qDebug()<<"ApduLen:"<<ByteArrayToHexString(dt.get_key_string("ApduLen"));
    qDebug()<<"ApduEncrpt:"<<ByteArrayToHexString(dt.get_key_string("ApduEncrpt"));
    sleep(1);
    END:
    //power off
    ret = dt.ApduHandle(POWER_OFF,isIcc,10,out,NULL,0);
    qDebug()<<"power off ret = "<<ret;
    flag = transmit(out,ret);
    free(out);
    if(!flag) return;
}
void MainWindow::on_bt_apdu_nfc_clicked()
{

    unsigned int timeout_powerOff = 10;
    unsigned int timeout_powerOn = 20;
    unsigned int timeout_APDU = 10;
    int isIcc = 0;
    char* out=(char*) malloc(1024*sizeof(char));
    int len =0;
    bool flag = false;
    int ret =-1;
    char apdu[1024]={0};
    int apduLen =24;

    //power on
    ret = dt.ApduHandle(POWER_ON,isIcc,DoTrade().CmdTimeOut,out,NULL,0);
    qDebug()<<"power on ret = "<<ret;

    flag = transmit(out,ret);

    while(getCmdId() ==CMD_BUSY ||getCmdId() ==CMD_CONTINUE)
    {
        REDO:
        qDebug() << "getCmdId() : " << getCmdId();
        sleep(1);
        len=dt.QueryLatestCommand(out);
        flag = transmit(out,len);
        if(!flag){
            qDebug() << "wait: " << getCmdId();
            goto REDO;
        }
    }
    if(!flag) return;
    if(getCmdId() == 0 || getCmdId() == CMD_CANCEL || getCmdId() == CMD_TIMEOUT ){
        goto END;
    }

    qDebug()<<"has card:"<<ByteArrayToHexString(dt.get_key_string("HasCard"));
    qDebug()<<"ksn:"<<ByteArrayToHexString(dt.get_key_string("KSN"));
    qDebug()<<"Atr:"<<ByteArrayToHexString(dt.get_key_string("Atr"));
    sleep(1);

    //apdu
    memcpy(apdu,"\x77\x65\x6C\x63\x6F\x6D\x65\x20\x75\x73\x65\x20\x64\x73\x70\x72\x65\x61\x64\x20\x70\x6F\x73\x2E",apduLen);
    ret = dt.ApduHandle(APDU_TRANSMIT,isIcc,DoTrade().CmdTimeOut,out,apdu,apduLen);

    qDebug()<<"apdu ret = "<<ret;
    flag = transmit(out,ret);
    while(getCmdId() ==CMD_BUSY)
    {
        qDebug() << "getCmdId() : " << getCmdId();
        sleep(1);
        len=dt.QueryLatestCommand(out);
        transmit(out,len);
    }

    if(!flag) goto END;// finally  close the card reader
    sleep(1);
    qDebug()<<"ApduResult:"<<ByteArrayToHexString(dt.get_key_string("ApduResult"));
    qDebug()<<"ApduLen:"<<ByteArrayToHexString(dt.get_key_string("ApduLen"));
    qDebug()<<"ApduEncrpt:"<<ByteArrayToHexString(dt.get_key_string("ApduEncrpt"));
    sleep(1);
    END:
    //power off
    ret = dt.ApduHandle(POWER_OFF,isIcc,DoTrade().CmdTimeOut,out,NULL,0);
    qDebug()<<"power off ret = "<<ret;
    flag = transmit(out,ret);
    free(out);
    if(!flag) return;
}

void MainWindow::on_bt_set_mer_clicked()
{
    //set merchant id / name
    char* out=(char*) malloc(1024*sizeof(char));
    int len =0;
    bool flag = false;
    int ret =-1;

    //set merchant Name
    //max len =16
    ret = dt.DoSetMerchantName("dspread_mer",out);
    qDebug()<<"setMerchantName ret = "<<ret;

    flag = transmit(out,ret);
    if(!flag) return;
    sleep(3);
    if(getCmdId() == CMD_NOTSUPPORT){
        qDebug()<<"dev not support setMerchantName!";
    }

    //set merchant id
    memset(out,0,1024);
    ret = dt.DoSetMerchantId("dspread_id",out);
    qDebug()<<"setMerchantId ret = "<< ret;
    flag = transmit(out,ret);
    if(!flag) return;        
    sleep(3);
    if(getCmdId() == CMD_NOTSUPPORT){
        qDebug()<<"dev not support setMerchantId!";
    }

    len = dt.DoGetPosId(out);
    transmit(out,len);
    qDebug("MerchantId:%s",dt.get_key_string("MerchantId").data());
    qDebug("MerchantName:%s",QString::fromUtf8( dt.get_key_string("MerchantName").data()));
    free(out);
}

void MainWindow::on_bt_algorithm_clicked()
{
    char* out=(char*) malloc(1024*sizeof(char));
    int len =0;
    bool flag = false;
    int ret =-1;

    algorithm_param_t param;

    //param.key_type = ENCRYPT_KEY_TMK;
    param.keyindex =0;
    param.mode = AL_DES_ECB_ENCRYPT;
    char data[32+1]={0};
    char key[16+1] ={0};

    strcpy(data,"\x64\x73\x70\x72\x65\x61\x64\x20\x64\x65\x6D\x6F");
    strcpy(key,"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01");

    param.input=(unsigned char *)data;
    param.inputLen=strlen(data);

    param.key=(unsigned char *)key;
    param.keyLen=16;

    ret = dt.DoAlgorithm(&param,out);
    qDebug()<<"DoAlgorithm ret = "<<ret;

    flag = transmit(out,ret);
    if(!flag) return;
    qDebug("calc_result:%s",qPrintable(dt.get_key_string("calc_result").toHex()));
    qDebug("code:%s",qPrintable(dt.get_key_string("code").toHex()));

    sleep(3);
    free(out);
}
