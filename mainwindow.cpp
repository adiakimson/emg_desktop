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
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(800,400);
       connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::onButton);
       connect(&serial_port,&QSerialPort::readyRead,this,&MainWindow::onRead);
       connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::stopMeasurement);
       connect(ui->radioButton,&QRadioButton::pressed,this,&MainWindow::readCh1);
       connect(ui->radioButton_2,&QRadioButton::pressed,this,&MainWindow::readCh2);
       connect(ui->radioButton_3,&QRadioButton::pressed,this,&MainWindow::readCh3);
       foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
               {
               ui->comboBox->addItem(serialPortInfo.portName());
               com = serialPortInfo.portName();
       }
      ui->comboBox_2->addItem("9600");
      ui->comboBox_2->addItem("19200");
      ui->comboBox_2->addItem("38400");
      ui->comboBox_2->addItem("115200");
      chartDraw(series);
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
           qDebug() << byte_array.size();
           QBuffer buf(&byte_array);
           buf.open(QIODevice::ReadOnly);
           while (buf.canReadLine()) {
               QString s = buf.readLine();
                   ui->plainTextEdit->appendPlainText(s.trimmed());
                   QStringList stringList = s.mid(1, s.length() - 2).split(" ");
                   bool ok;
                   int channel = 0;
                   for(QString item : stringList)
                   {
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

void MainWindow::readCh1()
{
    while(serial_port.atEnd()==false)
    {
        if((line_count+3)%3==0)
        {
            byte_array.append(serial_port.readLine());
            qDebug()<<byte_array.size();
            QBuffer buf(&byte_array);
            buf.open(QIODevice::ReadOnly);
            while (buf.canReadLine()) {
                QString s = buf.readLine();
                ui->plainTextEdit->appendPlainText(s.trimmed());
                QStringList stringList = s.mid(1, s.length() - 2).split(" ");
                bool ok;
                int channel = 0;
                for(QString item : stringList)
                {
                    float itemNumber = item.toFloat(&ok);
                    if (ok)
                    {
                        if(channel == 0)
                        {
                            x_point = x_point+0.01;
                            series1->append(x_point, itemNumber/1024);
                            if(x_point>1)
                            {
                                x_point = 0;
                                series1->clear();
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
        line_count++;
    }
}

void MainWindow::readCh2()
{
    while(serial_port.atEnd()==false)
    {
        byte_array.append(serial_port.readAll());
           qDebug() << byte_array.size();
           QBuffer buf(&byte_array);
           buf.open(QIODevice::ReadOnly);
           while (buf.canReadLine()) {
               QString s = buf.readLine();
               if((line_count+3)%3==1)
               {
                   ui->plainTextEdit->appendPlainText(s.trimmed());
                   QStringList stringList = s.mid(1, s.length() - 2).split(" ");
                   bool ok;
                   int channel = 0;
                   for(QString item : stringList)
                   {
                       float itemNumber = item.toFloat(&ok);
                       if (ok)
                       {
                           if(channel == 0)
                           {
                               x_point = x_point+0.01;
                               series2->append(x_point, itemNumber/1024);
                               if(x_point>1)
                               {
                                   x_point = 0;
                                   series2->clear();
                               }
                           }
                           channel++;
                       }
                   }
               }
               line_count++;
           }
           int pos = buf.pos();
           buf.close();
           byte_array.remove(0,pos);
    }
}

void MainWindow::readCh3()
{
    while(serial_port.atEnd()==false)
    {
        byte_array.append(serial_port.readAll());
           qDebug() << byte_array.size();
           QBuffer buf(&byte_array);
           buf.open(QIODevice::ReadOnly);
           while (buf.canReadLine()) {
               QString s = buf.readLine();
               if((line_count+3)%3==2)
               {
                   ui->plainTextEdit->appendPlainText(s.trimmed());
                   QStringList stringList = s.mid(1, s.length() - 2).split(" ");
                   bool ok;
                   int channel = 0;
                   for(QString item : stringList)
                   {
                       float itemNumber = item.toFloat(&ok);
                       if (ok)
                       {
                           if(channel == 0)
                           {
                               x_point = x_point+0.01;
                               series3->append(x_point, itemNumber/1024);
                               if(x_point>1)
                               {
                                   x_point = 0;
                                   series3->clear();
                               }
                           }
                           channel++;
                       }
                   }
               }
           line_count++;
           }
           int pos = buf.pos();
           buf.close();
           byte_array.remove(0,pos);
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

void MainWindow::chartDraw(QLineSeries *data)
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


void MainWindow::on_radioButton_clicked()
{
    line_count=0;
    MainWindow::readCh1();
    delete ui->frame->layout();
    //MainWindow::chartDraw(series1);
    chart->setTitle("EMG signals Ch1");
    ui->plainTextEdit->appendPlainText("Channel 1");
}

void MainWindow::on_radioButton_2_clicked()
{
    line_count=1;
    MainWindow::readCh2();
    delete ui->frame->layout();
    //MainWindow::chartDraw(series2);
    chart->setTitle("EMG signals Ch2");
    ui->plainTextEdit->appendPlainText("Channel 2");
}

void MainWindow::on_radioButton_3_clicked()
{
    line_count=2;
    MainWindow::readCh3();
    delete ui->frame->layout();
    //MainWindow::chartDraw(series3);
    chart->setTitle("EMG signals Ch3");
    ui->plainTextEdit->appendPlainText("Channel 3");
}

