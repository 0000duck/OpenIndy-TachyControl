#include "tachycontrol.h"

using namespace tc;

/*!
 * \brief TachyControl::TachyControl
 */
TachyControl::TachyControl()
{
    this->serial = new QSerialPort();
}

/*!
 * \brief TachyControl::connectSensor
 * \param ConnectConfig
 * \return
 */
bool TachyControl::connectSensor(ConnectionConfig ConnectConfig)
{
    //set port
    this->serial->setPortName(ConnectConfig.comPort);

    //open com port and set parameters
    if(this->serial->open(QIODevice::ReadWrite)){
        if(this->serial->setBaudRate(ConnectConfig.baudRate)
                && this->serial->setDataBits(ConnectConfig.dataBits)
                && this->serial->setParity(ConnectConfig.parity)
                && this->serial->setFlowControl(ConnectConfig.flowControl)
                && this->serial->setStopBits(ConnectConfig.stopBits)){

            return true;
        }
    }
    return false;
}

/*!
 * \brief TachyControl::disconnectSensor
 * \return
 */
bool TachyControl::disconnectSensor()
{
    if(this->serial->isOpen()){
        this->serial->close();
    }
    return true;
}

/*!
 * \brief TachyControl::move
 * \param azimuth
 * \param zenith
 * \param distance
 * \param isRelative
 * \return
 */
bool TachyControl::move(bool mathematic, const double &azimuth, const double &zenith, const double &distance, const bool &isRelative)
{
    if(isRelative){

    }else{

        if( this->serial->isOpen()){

            double az = azimuth;
            double ze = zenith;

            if(az <= 0.0){
                az = 2*PI + az;
            }

            //correct the values depending on specified sense of rotation
            if(mathematic){
                    az = 2*PI - az;
            }

            QString command="%R1Q,9027:";
            command.append(QString::number(az));
            command.append(",");
            command.append(QString::number(ze));
            command.append(",0,0,0\r\n");

            if(executeCommand(command)){
                this->receive();
            }
        }
    }
    return true;
}

/*!
 * \brief TachyControl::toggleSightOrientation
 * \return
 */
bool TachyControl::toggleSightOrientation()
{
    if(this->serial->isOpen()){
        QString command = "%R1Q,9028:0,0,0\r\n";
        if(executeCommand(command)){
            this->receive();
            return true;
        }
    }
    return false;
}

/*!
 * \brief TachyControl::measure returns a list of vectors containing azimuth, zenith, distance
 * \param measConfig
 * \return
 */
QList<std::vector<double> > TachyControl::measure(bool mathematical, MeasurementConfig measConfig)
{
    QList<std::vector<double> > measurements;

    int faceCount = 1;

    //check for two face measurement
    if(measConfig.twoSide){
        faceCount = 2;
    }

    if(this->serial->isOpen()){

        //number of iterations
        for(int i=0;i<measConfig.iterations;i++){

            for(int k=0; k<faceCount;k++){

                if(measConfig.withDistance){
                    //execute distance measurement
                    this->executeEDM(measConfig.reflectorless);
                }

                //get angles
                QString command = "%R1Q,2108:5000,1\r\n";

                if(this->executeCommand(command)){

                    QString measureData = this->receive();

                    QStringList polarElements = measureData.split(",");

                    //split receive code and get polar elements to save in vector
                    std::vector<double> polarResults;
                    polarResults.push_back(polarElements.at(polarElements.size()-3).toDouble());
                    polarResults.push_back(polarElements.at(polarElements.size()-2).toDouble());
                    polarResults.push_back(polarElements.at(polarElements.size()-1).toDouble());

                    //convert the values depending on specified sense of rotation
                    if(mathematical){
                        polarResults.at(0) = 2* PI - polarResults.at(0);
                    }
                    //add all measurements to list and return at the end
                    measurements.append(polarResults);

                    if(faceCount == 2){
                        this->toggleSightOrientation();
                    }
                }
            }
        }
    }
    return measurements;
}

/*!
 * \brief TachyControl::dataStream for watch window
 * \return
 */
QVariantMap TachyControl::dataStream()
{
    //currently not active at tachymeter
    QVariantMap data;
    return data;
}

/*!
 * \brief TachyControl::receive return code of geom com function call
 * \return
 */
QString TachyControl::receive()
{
    //receive response
    QByteArray responseData = this->serial->readAll();
    while (this->serial->waitForReadyRead(1000))
        responseData += this->serial->readAll();

    //parse and return response
    QString response = QString(responseData);
    return response;
}

/*!
 * \brief TachyControl::executeCommand executes the geom com command
 * \return
 */
bool TachyControl::executeCommand(QString command)
{
    QByteArray data = command.toLocal8Bit();

    this->serial->write(data, data.length());

    if(this->serial->waitForBytesWritten(10000)){

        if(this->serial->waitForReadyRead(10000)){
            return true;
        }else{
            QString test = this->receive();
            qDebug() << test;
            qDebug() << this->serial->errorString();
        }
    }else{
        QString test = this->receive();
        qDebug() << test;
        qDebug() << this->serial->errorString();
        return true; //check if this is ok here, or not !!!!
    }

    return false;
}

/*!
 * \brief TachyControl::executeEDM for distance measurement
 * \return
 */
bool TachyControl::executeEDM(bool reflectorless)
{
    QString edmCommand = "";

    if(reflectorless){
        edmCommand = "%R1Q,2008:6,1\r\n"; //edm measurement without reflector
    }else{
        edmCommand = "%R1Q,2008:1,1\r\n"; //edm measurement with reflector
    }

    //execute measurement and check return code if measurement is valid
    if(this->executeCommand(edmCommand)){
        QString receive = this->receive();
        if(receive.contains("%R1P,0,0:0\r\n")){
            return true;
        }
    }
    return false;
}

/*!
 * \brief TachyControl::setMeasureMode defines the current measure mode depending on settings
 * \param measConfig
 * \return
 */
bool TachyControl::setMeasureMode(MeasurementConfig measConfig)
{
    //get current setting if IR or RL standard or tracking
    QString command = "%R1Q,17018:\r\n";

    if(this->executeCommand(command)){
        QString receive = this->receive();

        if(!measConfig.reflectorless){ //IR
            //check if fast mode is active, else activate
            if(measConfig.measMode == MeasureModes::eFast){
                //R1Q,17019:1\r\n = IR fast
                if(!receive.contains("%R1P,0,0:0,1\r\n")){
                    return this->switchToMeasMode("%R1Q,17019:1\r\n");
                }
                return true;
            //check if precise mode is active, else activate
            }else if(measConfig.measMode == MeasureModes::ePrecise){
                //R1Q,17019:11\r\n = IR precise
                if(!receive.contains("%R1P,0,0:0,11\r\n")){
                    return this->switchToMeasMode("%R1Q,17019:11\r\n");
                }
                return true;
            }
            return false;

        }else{ //RL
            //%R1P,0,0:0,3\r\n == RL standard
            if(!receive.contains("%R1P,0,0:0,3\r\n")){ //if not, switch
                return this->switchToMeasMode("%R1Q,17019:3\r\n");
            }
            return true;
        }
    }
}

/*!
 * \brief TachyControl::switchToMeasMode switches to the specified measure mode
 * \param modeCommand
 * \return
 */
bool TachyControl::switchToMeasMode(QString modeCommand)
{
    if(this->executeCommand(modeCommand)){
        QString receive = this->receive();
        return true;
    }
    return false;
}
