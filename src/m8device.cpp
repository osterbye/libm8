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
#include "m8device.h"
#include <qplatformdefs.h>
#include <QSocketNotifier>

//#define M8DEVICE_DEBUG
#ifdef M8DEVICE_DEBUG
#include <QDebug>
#define M8DEVICE_D(x) qDebug() << "[M8Device] " << x
#else
#define M8DEVICE_D(x)
#endif

#define MAX_WRITE_DATA 512
#define MAX_READ_DATA 512

M8Device::M8Device(QString device, QObject *parent) : QObject(parent), m_socketNotifier(nullptr)
{
    m_deviceFD = QT_OPEN(device.toUtf8().constData(), O_RDWR);
    if (m_deviceFD < 0) {
        qWarning("[M8Device] Could not open %s", device.toUtf8().constData());
    } else {
        m_socketNotifier = new QSocketNotifier(m_deviceFD, QSocketNotifier::Read, this);
        connect(m_socketNotifier, &QSocketNotifier::activated, this, &M8Device::readDeviceData);
    }
}

M8Device::~M8Device()
{
    if (m_socketNotifier) {
        disconnect(m_socketNotifier, &QSocketNotifier::activated, this, &M8Device::readDeviceData);
        m_socketNotifier->deleteLater();
    }

    if (m_deviceFD >= 0)
        QT_CLOSE(m_deviceFD);
}

bool M8Device::isAvailable()
{
    return (m_deviceFD >= 0);
}

void M8Device::write(QByteArray message)
{
    if (m_deviceFD < 0)
        return;

    while (!message.isEmpty()) {
        int bytesToSend = qMin(message.count(), MAX_WRITE_DATA);
        ssize_t bytesSent =
                QT_WRITE(m_deviceFD, message.left(bytesToSend), static_cast<size_t>(bytesToSend));
        M8DEVICE_D("writeMessage() - actual size: " << bytesToSend << ", sent: " << bytesSent);
        if (bytesSent > 0) {
            message.remove(0, static_cast<int>(bytesSent));
        } else {
            message.clear();
            M8DEVICE_D("writeMessage() - ERROR");
        }
    }
}

void M8Device::readDeviceData()
{
    static char buffer[MAX_READ_DATA];
    ssize_t bytesRead = QT_READ(m_deviceFD, buffer, sizeof(buffer));
    M8DEVICE_D("Read " << bytesRead << " bytes");
    if (bytesRead > 0) {
        QByteArray newData(buffer, static_cast<int>(bytesRead));
        // M8DEVICE_D(newData);
        emit data(newData);
    }
}
