/*
MIT License

Copyright (c) 2020-2021 Nikolaj Due Østerbye

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
#include "ubx.h"
#include <QThread>
#include <QTimer>

//#define M8C_DEBUG
#ifdef M8C_DEBUG
#include <QDebug>
#define M8C_D(x) qDebug() << "[m8control] " << x
#else
#define M8C_D(x)
#endif

M8Control::M8Control(QString device, QObject *parent)
    : QObject(parent), m_status(M8_STATUS_INITIALIZING), m_chipConfirmationDone(false)
{
    m_m8Device = new M8Device(device);
    if (m_m8Device->isAvailable()) {
        m_m8DeviceThread = new QThread();
        m_m8Device->moveToThread(m_m8DeviceThread);
        m_m8DeviceThread->start();
        m_nmea = new NMEA(this);
        connect(m_nmea, &NMEA::newPosition, this, &M8Control::newPosition);
        m_ubx = new UBX(m_m8Device, this);
        connect(m_ubx, &UBX::systemTimeDrift, this, &M8Control::systemTimeDrift);
        connect(m_ubx, &UBX::satelliteInfo, this, &M8Control::satelliteInfo);
        connect(m_m8Device, &M8Device::data, this, &M8Control::deviceData);
        QTimer::singleShot(5000, this, &M8Control::chipTimeout);
    } else {
        delete m_m8Device;
        setStatus(M8_STATUS_ERROR_DRIVER);
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

void M8Control::requestTime()
{
    m_ubx->requestTime();
}

void M8Control::requestSatelliteInfo()
{
    m_ubx->requestSatelliteInfo();
}

/**
 * @brief M8Control::deviceData
 * @param ba
 *
 * Either NMEA or UBX message (disregarding CRC) will confirm chip presence.
 */
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
                m_chipConfirmationDone = true;
                if (M8_STATUS_ON != m_status)
                    setStatus(M8_STATUS_ON);
            } else {
                M8C_D("Incomplete nmea string. Wait for more data. " << m_input);
                break;
            }
        } else if (m_input.startsWith(static_cast<char>(0xB5))) {
            if (m_input.count() >= 8) {
                if (0x62 != m_input.at(1)) {
                    M8C_D("Incorrect sync char for ubx message: " << m_input.at(1));
                    m_input.remove(0, 1);
                } else {
                    int payloadLen = m_input.at(4) | (m_input.at(5) << 8);
                    if (m_input.size() >= (payloadLen + 8)) {
                        QByteArray ubxMessage = m_input.mid(2, payloadLen + 6);
                        if (m_ubx->crcCheck(ubxMessage))
                            m_ubx->parse(ubxMessage);

                        m_input.remove(0, payloadLen + 8);
                    } else {
                        M8C_D("Incomplete ubx message. Wait for more data.");
                        break;
                    }
                }
                m_chipConfirmationDone = true;
            } else {
                M8C_D("Incomplete ubx message. Wait for more data.");
                break;
            }
        } else {
            m_input.remove(0, 1);
        }
    }
}

void M8Control::chipTimeout()
{
    if (!m_chipConfirmationDone)
        setStatus(M8_STATUS_ERROR_CHIP);

    m_chipConfirmationDone = true;
}

void M8Control::setStatus(M8_STATUS status)
{
    if (M8_STATUS_ON != m_status && M8_STATUS_ON == status)
        m_ubx->configureNMEA();

    m_status = status;
    emit statusChange(m_status);
}
