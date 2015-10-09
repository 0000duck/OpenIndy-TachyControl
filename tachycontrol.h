#ifndef TACHYCONTROL_H
#define TACHYCONTROL_H

#include <QtGlobal>
#include <QDateTime>
#include <QObject>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QRegExp>
#include <QThread>
#include <QStringList>
#include <QVariantMap>
#include <QPointer>

namespace tc{

#define PI 3.141592653589793
#define RHO_DEGREE  (180.0/PI)
#define RHO_GON (200.0/PI)

/*!
 * \brief The MeasureModes enum
 */
enum MeasureModes{
    eFast,
    ePrecise
};

/*!
 * \brief The ConnectionConfig struct
 */
struct ConnectionConfig{

    QString comPort;
    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::FlowControl flowControl;
    QSerialPort::StopBits stopBits;
};

/*!
 * \brief The MeasurementConfig struct
 */
struct MeasurementConfig{
    bool reflectorless;
    MeasureModes measMode;
    int iterations;
    bool twoSide;
    bool withDistance;
};

/*!
 * \brief The TachyControl class
 */
class TachyControl
{
public:
    TachyControl();

    //connect and disconnect
    bool connectSensor(ConnectionConfig ConnectConfig);
    bool disconnectSensor();

    //special total station methods
    bool move(bool mathematic, const double &azimuth, const double &zenith, const double &distance, const bool &isRelative);

    bool toggleSightOrientation();

    //returns a list of vectors<azimuth, zenith, distance> depending on count of measurements
    QList<std::vector<double> > measure(bool mathematical, MeasurementConfig measConfig);

    //Data Stream
    QVariantMap dataStream();

private:

    QPointer<QSerialPort> serial;

    //methods to execute commands and receive data
    QString receive();
    bool executeCommand(QString command);
    //execute distance measurement
    bool executeEDM(bool reflectorless);


    //function to set measure mode
    bool setMeasureMode(MeasurementConfig measConfig);

    //switch measure mode
    bool switchToMeasMode(QString modeCommand);
};

}
#endif // TACHYCONTROL_H
