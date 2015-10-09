#include <QCoreApplication>
#include "tachycontrol.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    tc::TachyControl tachy;

    qDebug() << "conConfig";

    tc::ConnectionConfig conConfig;

    conConfig.baudRate = QSerialPort::Baud19200;
    conConfig.comPort = "com4";
    conConfig.dataBits = QSerialPort::Data8;
    conConfig.flowControl = QSerialPort::NoFlowControl;
    conConfig.parity =  QSerialPort::NoParity;
    conConfig.stopBits = QSerialPort::OneStop;

    qDebug() << "measConfig";

    tc::MeasurementConfig measConfig;
    measConfig.iterations = 1;
    measConfig.measMode = tc::MeasureModes::ePrecise;
    measConfig.reflectorless = false;
    measConfig.twoSide = true;
    measConfig.withDistance = true;

    qDebug() << "connect";
    if(tachy.connectSensor(conConfig)){
        tachy.measure(true,measConfig);
    }
    return a.exec();
}

