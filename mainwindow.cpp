#include "mainwindow.h"
#include "qnamespace.h"
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
#include <QIODevice>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , line_count(0)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(850,550);
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::onButton);
    connect(&serial_port,&QSerialPort::readyRead,this, &MainWindow::onRead);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::stopMeasurement);
    connect(ui->radioButton,&QRadioButton::pressed,this,&MainWindow::onRead);
    connect(ui->radioButton_2,&QRadioButton::pressed,this,&MainWindow::onRead);
    connect(ui->radioButton_3,&QRadioButton::pressed,this,&MainWindow::onRead);
    connect(ui->radioButton_4,&QRadioButton::pressed,this,&MainWindow::onRead);
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->comboBox->addItem(serialPortInfo.portName());
        com = serialPortInfo.portName();
    }
    ui->comboBox_2->addItem("9600");
    ui->comboBox_2->addItem("19200");
    ui->comboBox_2->addItem("38400");
    ui->comboBox_2->addItem("115200");
    ui->comboBox_3->addItem("Chart prescaler: 8");
    ui->comboBox_3->addItem("Chart prescaler: 9");
    ui->comboBox_3->addItem("Chart prescaler: 10");
    ui->comboBox_3->addItem("Chart prescaler: 11");
    ui->comboBox_3->addItem("Chart prescaler: 12");
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
                     QStringList stringList = s.mid(1, s.length() - 2).split(" ");
                     int channel = 0;
                     for(const auto& item : stringList)
                     {
                         bool ok;
                         float itemNumber = item.toFloat(&ok);
                         if (ok)
                         {
                             channel=(line_count+3)%3;
                             if((channel==currentChannel)||(!currentChannel.has_value()))
                             {
                                 ui->plainTextEdit->appendPlainText(s.trimmed());
                                 ui->plainTextEdit_2->appendPlainText(ln.trimmed());
                                 x_point = x_point+0.01;
                                 series->append(x_point, itemNumber/(1 << n));
                                 if(x_point>1)
                                {
                                   x_point = 0;
                                   series->clear();
                                }
                             }
                             line_count++;
                         }
                     }
             int pos = buf.pos();
             buf.close();
             byte_array.remove(0,pos);
         }
     }
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

void MainWindow::on_comboBox_3_activated(int index)
{
    if(index==0)
    {
        n=8;
    }
    else if(index==1)
    {
        n=9;
    }
    else if(index==2)
    {
        n=10;
    }
    else if(index==3)
    {
        n=11;
    }
    else
    {
        n=12;
    }
}

void MainWindow::on_radioButton_clicked()
{
     currentChannel = 0;
     delete ui->frame->layout();
     chart->setTitle("EMG signals Ch1");
     series->setColor(Qt::blue);
     ui->plainTextEdit->appendPlainText("Channel 1");
}

void MainWindow::on_radioButton_2_clicked()
{
     currentChannel = 1;
     delete ui->frame->layout();
     chart->setTitle("EMG signals Ch2");
     series->setColor(Qt::green);
     ui->plainTextEdit->appendPlainText("Channel 2");
}

void MainWindow::on_radioButton_3_clicked()
{
     currentChannel = 2;
     delete ui->frame->layout();
     chart->setTitle("EMG signals Ch3");
     series->setColor(Qt::red);
     ui->plainTextEdit->appendPlainText("Channel 3");
}

void MainWindow::on_radioButton_4_pressed()
{
    currentChannel.reset();
    delete ui->frame->layout();
    chart->setTitle("EMG signals");
    series->setColor(Qt::black);
    ui->plainTextEdit->appendPlainText("All channels");
}


