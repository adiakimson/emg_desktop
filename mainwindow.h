#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qserialport.h"
#include "ui_mainwindow.h"
#include <optional>
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
    QChart *chart;
    QChartView *chartView;
    QLineSeries *series;
    QString com;
    float x_point;
    int line_count=0; //counter for all lines
    int n=9; //number of bits to prescale the chart
    std::optional<int> currentChannel; //to switch channels
    Ui::MainWindow *ui;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    //void readChannel();

private slots:
    void onButton();
    void onRead();
    void stopMeasurement();
    void chartDraw();
    void on_comboBox_2_activated(int index);
    void on_comboBox_3_activated(int index);
    void on_radioButton_clicked();
    void on_radioButton_2_clicked();
    void on_radioButton_3_clicked();
    void on_radioButton_4_pressed();
};
#endif // MAINWINDOW_H
