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
#include <QtBluetooth>
#include <QDateTime>
#include <cstdint>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , line_count(0)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(850,600);
    //serialport data
    //connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::onButton);
    //connect(&serial_port,&QSerialPort::readyRead,this, &MainWindow::onRead);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::stopMeasurement);
    connect(ui->radioButton,&QRadioButton::pressed,this,&MainWindow::onRead);
    connect(ui->radioButton_2,&QRadioButton::pressed,this,&MainWindow::onRead);
    connect(ui->radioButton_3,&QRadioButton::pressed,this,&MainWindow::onRead);
    connect(ui->radioButton_4,&QRadioButton::pressed,this,&MainWindow::onRead);
    //BT data
    this->discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(this->discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(captureDeviceProperties(QBluetoothDeviceInfo)));
    connect(this->discoveryAgent, SIGNAL(finished()), this, SLOT(searchingFinished()));
    this->socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(ui->connection, &QPushButton::clicked, this, &MainWindow::readMessageFromDevice);
    connect(this->socket, SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(this->socket, SIGNAL(disconnected()), this, SLOT(connectionInterrupted()));
    connect(this->socket, SIGNAL(readyRead()), this, SLOT(readMessageFromDevice()));
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

//reading via serialport
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
                     QStringList stringList = s.trimmed().split(" ");
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
                                 x_point++;
                                 series->append(x_point, itemNumber);//(1 << n));
                                 chartView->repaint();
                                 if(x_point>1000)
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
    // baud rate domyślny
    serial_port.setBaudRate(38400);
    serial_port.setFlowControl(QSerialPort::NoFlowControl);
    if (!serial_port.open(QIODevice::ReadWrite)) {
        QMessageBox::warning(this,"Error","Port open");
    }
}

void MainWindow::stopMeasurement()
{
    serial_port.close();
    this->socket->close();
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
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    auto lay = new QVBoxLayout();
    ui->frame->setLayout(lay);
    lay->addWidget(chartView);
    qDebug() << "S:" << series->points().size();
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

void MainWindow::on_search_clicked()
{
    qDebug() << "Szukanie...";
    ui->plainTextEdit->appendPlainText("Searching for device...");
    ui->comboBox->clear();
    ui->search->setEnabled(false);
    this->discoveryAgent->start(); // rozpoczęcie wyszukiwania
}

void MainWindow::on_connection_clicked()
{
    qDebug() << "Łączenie...";
    ui->plainTextEdit->appendPlainText("Connecting...");
    QString comboBoxQString = ui->comboBox->currentText();
    QStringList portList = comboBoxQString.split(" ");
    QString deviceAddres = portList.last();
    static const QString serviceUuid(QStringLiteral("00001101-0000-1000-8000-00805F9B34FB"));
    this->socket->connectToService(QBluetoothAddress(deviceAddres),QBluetoothUuid(serviceUuid),QIODevice::ReadWrite);
    ui->plainTextEdit->appendPlainText("Starting connection with device: " + portList.first() + " and address: " + deviceAddres);
}

void MainWindow::on_disconnect_clicked()
{
    qDebug() << "Kończenie połączenia...";
    ui->plainTextEdit->appendPlainText("Connection ended.");
}

void MainWindow::captureDeviceProperties(const QBluetoothDeviceInfo &device)
{
    ui->comboBox->addItem(device.name() + " " + device.address().toString());
}

void MainWindow::searchingFinished()
{
    ui->search->setEnabled(true);
}

void MainWindow::connectionEstablished()
{
    qDebug() << "Połączono.";
    ui->plainTextEdit->appendPlainText("Connected.");
}

void MainWindow::connectionInterrupted()
{
    ui->plainTextEdit->appendPlainText("Connection interrupted.");
}

void MainWindow::socketReadyToRead()
{
    qDebug() << "New data...";
    while(this->socket->isOpen() && this->socket->isReadable())
    {
        bt_array.append(this->socket->readAll());
        qDebug() << "Size of bt array";
        qDebug() << bt_array.size();
        QBuffer buffer(&bt_array);
        buffer.open(QIODevice::ReadOnly);
         while (buffer.canReadLine())
         {
             QString s = buffer.readLine();
             qDebug() << s;
             QString ln = QString::number(line_count);
             qDebug() << ln;
             QStringList stringList = s.trimmed().split(" ");
             for(const auto& item : stringList)
             {
                 bool ok;
                 uint8_t data8bit = item.toUInt(&ok);
                 qDebug() << data8bit;
                 if(data8bit>127)
                 {
                     //?
                 }
             }
         }
    }
}

// odczyt przez bluetooth
void MainWindow::readMessageFromDevice()
{
    qDebug() << "Reading data...";
    if(this->socket->isOpen() && this->socket->isReadable())
    {
        while(true) //na potrzeby debuggowania
        {
            bt_array.append(this->socket->readAll());
            // qDebug() << "Size of bt array";
            // qDebug() << bt_array.size();
            QBuffer buffer(&bt_array);
            buffer.open(QIODevice::ReadOnly);
            while (buffer.canReadLine())
            {
                    // załadować dwie liczby
                    QString s1 = buffer.readLine();
                    QString s2 = buffer.readLine();
                    QString ln = QString::number(line_count);
                    QStringList stringList1 = s1.trimmed().split(" ");
                    // domyślny kanał pierwszy
                    int channel = 0;          
                        for(const auto& item : stringList1)
                        {
                            bool ok;
                            uint8_t itemNumber1;
                            uint8_t itemNumber2;
                            itemNumber1 = item.toUInt(&ok);
                            itemNumber2 = item.toUInt(&ok);
                            // jeżeli pierwsza liczba większa od 127
                            if (ok&&(itemNumber1>127))
                            {
                                channel=(line_count+3)%3;
                                // wczytać dwa uint8_t i scalić w uint16_t
                                uint16_t newItemNumber = (uint16_t)((itemNumber1 << 8) + itemNumber2);
                                // uint16_t załadować do kontenera dla wykresu
                                if((channel==currentChannel)||(!currentChannel.has_value()))
                                {
                                    ui->plainTextEdit->appendPlainText(s1.trimmed());
                                    ui->plainTextEdit->appendPlainText(s2.trimmed());
                                    ui->plainTextEdit_2->appendPlainText(ln.trimmed());
                                    x_point++;
                                }
                                // obsługa preskalera
                                series->append(x_point, newItemNumber/(1 << n));
                                chartView->repaint();
                                // 1000 informuje o tym jak długi będzie wykres
                                if(x_point>1000)
                               {
                                  x_point = 0;
                                  series->clear();
                               }
                            }
                            line_count++;
                        }

            int pos = buffer.pos();
            buffer.close();
            bt_array.remove(0,pos);
        }
        }
    }
    else
    {
        ui->plainTextEdit->appendPlainText("Reading data is currently unavailable");
    }
}



