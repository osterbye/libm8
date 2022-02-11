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
#ifndef M8_H
#define M8_H

#include "m8_global.h"
#include "m8_status.h"
#include "m8_sv_info.h"
#include <QObject>

class M8Control;

class M8_EXPORT M8 : public QObject
{
    Q_OBJECT
public:
    M8(QString device, QObject *parent = nullptr);
    M8(QString device, QByteArray configPath, QObject *parent = nullptr);

    M8_STATUS status();
    void requestTime();
    void requestSatelliteInfo();

signals:
    void statusChange(M8_STATUS status);
    void nmea(const QByteArray &nmea);
    void newPosition(double latitude, double longitude, float altitude, quint8 satellites);
    void systemTimeDrift(qint64 offsetMilliseconds);
    void satelliteInfo(M8_SV_INFO info);

private:
    M8Control *m_control;
};

#endif // M8_H
