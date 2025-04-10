#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "dotrade.h"
#include <QtSerialPort/QSerialPort>
#include <QMainWindow>
#include <QTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool transmit(char* out,int len);
    bool transmit(char* out,int len,int timeout);
    QByteArray HexStringToByteArray(QString HexString);
    QString ByteArrayToHexString(QByteArray data);

private slots:
    void on_connectButton_clicked();

    void on_posIDButton_clicked();

    void on_posInfoButton_clicked();

    void on_doTradeButton_clicked();

    void on_transactionTypeComboBox_currentIndexChanged(int index);
    void sleep(int ms);

   //void on_SetInfo_clicked();

    void on_pullCardButton_clicked();

    void on_verifyButton_clicked();

    void on_finishButton_clicked();

    void on_readCardButton_clicked();

    void on_writeCardButton_clicked();


    void on_bt_apdu_icc_clicked();

    void on_bt_apdu_nfc_clicked();

    void on_bt_set_mer_clicked();

    void on_bt_algorithm_clicked();
    void test_Buzzer();
	void test_ShutDown();
private:
    Ui::MainWindow *ui;
    DoTrade dt;
    QSerialPort serial;
    QString portName;
    QString response;
    QByteArray cardUid;

};

#endif // MAINWINDOW_H
