#include "mainwindow.h"
#include "qpushbutton.h"
#include "qradiobutton.h"
#include "qserialportinfo.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QValueAxis>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , line_count(0)
    , ch1(0)
    , ch2(0)
    , ch3(0)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(800,400);
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::onButton);
    connect(&serial_port,&QSerialPort::readyRead,this, &MainWindow::onRead);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::stopMeasurement);
    connect(ui->radioButton,&QRadioButton::pressed,this,&MainWindow::onRead);
    connect(ui->radioButton_2,&QRadioButton::pressed,this,&MainWindow::onRead);
    connect(ui->radioButton_3,&QRadioButton::pressed,this,&MainWindow::onRead);
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->comboBox->addItem(serialPortInfo.portName());
        com = serialPortInfo.portName();
    }
    ui->comboBox_2->addItem("9600");
    ui->comboBox_2->addItem("19200");
    ui->comboBox_2->addItem("38400");
    ui->comboBox_2->addItem("115200");
    chartDraw();
}

MainWindow::~MainWindow()
{
    delete ui;
}

 void MainWindow::onRead()
 {
     while(serial_port.atEnd()==false)
     {
             byte_array.append(serial_port.readAll());
             qDebug() << byte_array.size(); // DEBUG
             QBuffer buf(&byte_array);
             buf.open(QIODevice::ReadOnly);
             while (buf.canReadLine()) {
                 QString s = buf.readLine();
                 QString ln = QString::number(line_count);
                 if((currentChannel.has_value() && (line_count+3)%3==currentChannel) || !currentChannel.has_value())
                 {
                 ui->plainTextEdit->appendPlainText(s.trimmed());
                 ui->plainTextEdit_2->appendPlainText(ln.trimmed());
                 QStringList stringList = s.mid(1, s.length() - 2).split(" ");
                 int channel = 0;
                 for(const auto& item : stringList)
                 {
                     bool ok;
                     float itemNumber = item.toFloat(&ok);
                     if (ok)
                     {
                         if(channel == 0)
                         {
                             x_point = x_point+0.01;
                             series->append(x_point, itemNumber/1024);
                             if(x_point>1)
                             {
                                 x_point = 0;
                                 series->clear();
                             }
                         }
                         channel++;
                     }
                 }
             }
             int pos = buf.pos();
             buf.close();
             byte_array.remove(0,pos);
         }
     }
     line_count++;
 }

void MainWindow::onButton()
{
    serial_port.setPortName(com);
    serial_port.setBaudRate(38400);
    serial_port.setFlowControl(QSerialPort::NoFlowControl);
    if (!serial_port.open(QIODevice::ReadWrite)) {
        QMessageBox::warning(this,"Error","Port open");
    }
}

void MainWindow::stopMeasurement()
{
    serial_port.close();
    QMessageBox::warning(this,"Warning","Measurement stopped");
}

void MainWindow::chartDraw()
{
    chart = new QChart();
    chart->legend()->hide();
    series = new QLineSeries();
    chart->addSeries(series);
    chart->setTitle("EMG signals");
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time");
    axisX->setRange(0, 1000);
    axisX->setLabelFormat("%d");
    chart->addAxis(axisX, Qt::AlignBottom);
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("mV");
    axisY->setRange(0, 3300);
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    auto lay = new QVBoxLayout();
    ui->frame->setLayout(lay);
    lay->addWidget(chartView);
}

void MainWindow::on_comboBox_2_activated(int index)
{
    if(index==0)
    {
        serial_port.setBaudRate(9600);
    }
    else if(index==1)
    {
        serial_port.setBaudRate(19200);
    }
    else if(index==2)
    {
        serial_port.setBaudRate(38400);
    }
    else
    {
        serial_port.setBaudRate(115200);
    }
}

//popracować nad przełączaniem
void MainWindow::on_radioButton_clicked()
{
    flag1=true;
    flag2=flag3=false;
    currentChannel = 0;
    QString s;
    while(flag1==true&&line_count%3==0)
    {
        MainWindow::onRead();
        delete ui->frame->layout();
        chart->setTitle("EMG signals Ch1");
        ui->plainTextEdit->appendPlainText("Channel 1");
        ui->plainTextEdit_2->appendPlainText("Channel line");
        s=QString::number(ch1);
        ui->plainTextEdit_2->appendPlainText(s);
        ch1++;
    }
}

void MainWindow::on_radioButton_2_clicked()
{
    flag2=true;
    flag1=flag3=false;
    currentChannel = 1;
    QString s;
    while(flag2==true&&line_count%3==1)
    {
        MainWindow::onRead();
        delete ui->frame->layout();
        chart->setTitle("EMG signals Ch2");
        ui->plainTextEdit->appendPlainText("Channel 2");
        ui->plainTextEdit_2->appendPlainText("Channel line");
        s=QString::number(ch2);
        ui->plainTextEdit_2->appendPlainText(s);
        ch2++;
    }
}

void MainWindow::on_radioButton_3_clicked()
{
    flag3=true;
    flag1=flag2=false;
    currentChannel = 2;
    QString s;
    while(flag3==true&&line_count%3==2)
    {
        MainWindow::onRead();
        delete ui->frame->layout();
        chart->setTitle("EMG signals Ch3");
        ui->plainTextEdit->appendPlainText("Channel 3");
        ui->plainTextEdit_2->appendPlainText("Channel line");
        s=QString::number(ch3);
        ui->plainTextEdit_2->appendPlainText(s);
        ch3++;
    }
}

