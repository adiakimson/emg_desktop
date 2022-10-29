#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qlist.h"
#include "qserialport.h"
#include <QMainWindow>
#include <QSerialPort>
#include <QBuffer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QMessageBox>
#include <QComboBox>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QSerialPort serial_port;
    QByteArray byte_array; //for all data
    //QByteArray byte_array1; //ch1
    //QByteArray byte_array2; //ch2
    //QByteArray byte_array3; //ch3
    QChart *chart;
    QChartView *chartView;
    QLineSeries *series;
    QLineSeries *series1;
    QLineSeries *series2;
    QLineSeries *series3;
    QString com;
    float x_point;
    int line_count=0;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onButton();
    void onRead(); //to read all
    void readCh1();
    void readCh2();
    void readCh3();
    void stopMeasurement();
    void chartDraw(QLineSeries *data);
    void on_comboBox_2_activated(int index);
    void on_radioButton_clicked();
    void on_radioButton_2_clicked();
    void on_radioButton_3_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
