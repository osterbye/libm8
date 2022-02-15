/*
MIT License

Copyright (c) 2020-2022 Nikolaj Due Østerbye

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef M8CONTROL_H
#define M8CONTROL_H

#include <QObject>
#include "m8_status.h"
#include "m8_sv_info.h"

class Assistance;
class Config;
class M8Device;
class NMEA;
class Power;
class QThread;
class QTimer;
class UBX;

class M8Control : public QObject
{
    Q_OBJECT
public:
    explicit M8Control(QString device, QByteArray configPath, QObject *parent = nullptr);
    ~M8Control();

    void setPower(bool on);
    void saveAutonomousAssistData();
    M8_STATUS status();
    void requestTime();
    void requestSatelliteInfo();

signals:
    void statusChange(M8_STATUS status);
    void nmea(const QByteArray &nmea);
    void newPosition(double latitude, double longitude, float altitude, quint8 satellites);
    void systemTimeDrift(qint64 offsetMilliseconds);
    void satelliteInfo(M8_SV_INFO info);

private slots:
    void deviceData(QByteArray ba);
    void chipTimeout();

private:
    void setStatus(M8_STATUS status);

private:
    M8Device *m_m8Device;
    QThread *m_m8DeviceThread;
    M8_STATUS m_status;
    QTimer *m_statusTimer;
    QByteArray m_input;
    Assistance *m_assistance;
    Config *m_config;
    Power *m_power;
    NMEA *m_nmea;
    bool m_chipConfirmationDone;
    UBX *m_ubx;
};

#endif // M8CONTROL_H
