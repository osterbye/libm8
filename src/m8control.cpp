/*
MIT License

Copyright (c) 2020 Nikolaj Due Osterbye

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
#include "m8control.h"
#include "m8device.h"
#include "nmea.h"
#include <QThread>

#define M8C_DEBUG
#ifdef M8C_DEBUG
#include <QDebug>
#define M8C_D(x) qDebug() << "[m8control] " << x
#else
#define M8C_D(x)
#endif

#define BYTE(x) (x & 0xFF)

M8Control::M8Control(QString device, QObject *parent)
    : QObject(parent), m_status(M8_STATUS_INITIALIZING)
{
    m_m8Device = new M8Device(device);
    if (m_m8Device->isAvailable()) {
        m_m8DeviceThread = new QThread();
        m_m8Device->moveToThread(m_m8DeviceThread);
        m_m8DeviceThread->start();
        m_nmea = new NMEA(this);
        connect(m_nmea, SIGNAL(newPosition(double, double, float, quint8)), this,
                SIGNAL(newPosition(double, double, float, quint8)));
        connect(m_m8Device, SIGNAL(data(QByteArray)), this, SLOT(deviceData(QByteArray)));
        setStatus(M8_STATUS_ON);
    } else {
        delete m_m8Device;
        setStatus(M8_STATUS_ERROR);
    }
}

M8Control::~M8Control()
{
    if (m_m8DeviceThread) {
        m_m8DeviceThread->quit();
        m_m8DeviceThread->wait(2000);
        m_m8DeviceThread->deleteLater();
        delete m_m8Device;
    }
}

M8_STATUS M8Control::status()
{
    return m_status;
}

void M8Control::deviceData(QByteArray ba)
{
    m_input.append(ba);
    while (!m_input.isEmpty()) {
        if (m_input.startsWith('$')) {
            int nmeaEnd = m_input.indexOf('\n');
            if (nmeaEnd >= 0) {
                QByteArray nmeaStr = m_input.left(nmeaEnd);
                if (m_nmea->crcCheck(nmeaStr)) {
                    M8C_D("NMEA: " << nmeaStr);
                    emit nmea(nmeaStr);
                    m_nmea->parse(nmeaStr);
                } else {
                    M8C_D("NMEA checksum error: " << nmeaStr);
                }
                m_input.remove(0, nmeaEnd);
            } else {
                M8C_D("Incomplete nmea string. Wait for more data. " << m_input);
                break;
            }
        } else if (m_input.startsWith(static_cast<char>(0xB5))) {
            if (m_input.count() >= 8) {
                if (0x62 != BYTE(m_input.at(1))) {
                    M8C_D("Incorrect sync char for ubx message: " << m_input.at(1));
                    m_input.remove(0, 1);
                } else {
                    int payloadLen = BYTE(m_input.at(4)) | (BYTE(m_input.at(5)) << 8);
                    if (m_input.count() >= (payloadLen + 8)) {
                        m_input.remove(0, payloadLen + 8);
                    } else {
                        M8C_D("Incomplete ubx message. Wait for more data.");
                        break;
                    }
                }
            } else {
                M8C_D("Incomplete ubx message. Wait for more data.");
                break;
            }
        } else {
            m_input.remove(0, 1);
        }
    }
}

void M8Control::setStatus(M8_STATUS status)
{
    m_status = status;
    emit statusChange(m_status);
}
