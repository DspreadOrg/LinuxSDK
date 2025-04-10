#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sys/stat.h>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QFileDialog>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



typedef struct
{
	char mark[8];
	char framenum[4];
	char reserved[20];
}pos_firmware_header;
typedef struct
{
	unsigned char tag;
	unsigned char length[2];
}frame_header;
typedef struct
{
	frame_header header;
	char value[512];
}pos_firmware_frame;
unsigned int pu8_to_int(unsigned char* pdata)
{
	unsigned int val = 0;

	val += (unsigned int)pdata[0] * 256;
	val += (unsigned int)pdata[1];
	return val;
}
void sleep(int ms)
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
QSerialPort serial;
void transmit(char* out,int len)
{
    int tmplen=0;

    QString response;
    serial.write(out,len);
    if (serial.waitForBytesWritten(200)) {
        // write request
        qDebug("write:%s",qPrintable(QByteArray::fromRawData(out, len).toHex()));
        // read response
        if (serial.waitForReadyRead(6000)) {
            QByteArray responseData = serial.readAll();
            while (serial.waitForReadyRead(1000))
                responseData += serial.readAll();
            response=responseData.toHex();

            qDebug("tmplen:%d \n",qPrintable(tmplen));
            qDebug("read:%s \n",qPrintable(responseData.toHex()));
            memcpy(out,responseData.data(),response.length());
            on_package((unsigned char *)out,response.length());
        } else {
            qDebug("Wait read response timeout %1");
        }

    } else {
        qDebug("Wait write request timeout %1");
    }
}
void upgradefirmware(void)
{

    pos_firmware_header fheader;
    pos_firmware_frame fframe;
   // QSerialPort serial;
    unsigned int readlen = 0;
    unsigned int header_length = 0;
    char flag = 0;
    long File_length = 0;

    int fd=0;

    //QSerialPort *serial = new QSerialPort;
    QString path=QFileDialog::getOpenFileName(NULL,"open","../");

    if(path.isEmpty()==false)
    {
        qDebug("open success\r\n");
        QFile  fp(path);
        bool isok=fp.open(QIODevice::ReadOnly);
        if(isok)
        {
            //fp.seek(fp,0,SEEK_END);
           File_length= fp.size();
           qDebug("file length=%ld\r\n",File_length);
          readlen=fp.read((char*)&fheader,sizeof(pos_firmware_header));
         // fp.readLine(&fheader,sizeof(pos_firmware_header));
           if (memcmp(fheader.mark, "DSPREAD", strlen("DSPREAD")))
           {

               qDebug("pos_firmware_header != DSPREAD\r\n");
               return;
           }
           else
           {

               qDebug("pos_firmware_header success=%d\r\n",readlen);
           }
           File_length -= readlen;
           while (File_length>0)
           {
                readlen=fp.read((char*)&fframe.header,sizeof(fframe.header));
                header_length = pu8_to_int(fframe.header.length);
                readlen=fp.read((char*)& fframe.value,header_length);
                File_length -= (1 + 2 + header_length);
                qDebug("fframe.header.tag=%02x, header_length = %ld,  File_length =%ld \r\n",fframe.header.tag, header_length, File_length);
                switch (fframe.header.tag)
                {
                case 0x02:
                    sleep(5);

                    break;
                case 0x03:    //Open device port

                        serial.close();
                        serial. setPortName("ttyACM0");
                       fd = serial.open(QIODevice::ReadWrite);

                        if (!fd) {

                           qDebug("open port failed\r\n");
                           return ;

                        }else{
                            /*serial.setBaudRate(9600);
                            serial.setDataBits(QSerialPort::Data8);  //设置数据位数
                            serial.setParity(QSerialPort::NoParity); 	//设置奇偶校验
                            serial.setStopBits(QSerialPort::OneStop);  	//设置停止位
                            serial.setFlowControl(QSerialPort::NoFlowControl);	//设置流控制*/

                          qDebug("open port successs\r\n");

                        }

                    break;
                case 0x04://Close the device port
                    serial.close();
                    break;
                case 0x11://Send data frame
                   //qDebug("send data write:%s",qPrintable(QByteArray::fromRawData((char*)& fframe.value, header_length).toHex()));
                    transmit((char*)& fframe.value,header_length);
                    break;
                case 0x12://Enter boot mode
                    transmit((char*)& fframe.value,header_length);

                    if (getCmdId() == 0x24) {

                        qDebug("response success\r\n");
                    }
                    else
                    {
                        qDebug("response failed\r\n");
                        serial.close();
                        return;
                    }

                    sleep(2);
                    qDebug("enter boot mode\r\n");
                    break;
                default :
                    break;
                }
                memset((char*)& fheader, 0, sizeof(pos_firmware_header));
                memset((unsigned char*)& fframe, 0, sizeof(pos_firmware_frame));
                header_length = 0;
                readlen = 0;
           }

        }
    }



}

