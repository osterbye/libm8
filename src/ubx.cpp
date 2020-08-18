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
#include "ubx.h"
#include "m8device.h"
#include <QDateTime>
#include <QTimer>

//#define UBX_DEBUG
#ifdef UBX_DEBUG
#include <QDebug>
#define UBX_D(x) qDebug() << "[ubx] " << x
#else
#define UBX_D(x)
#endif

UBX::UBX(M8Device *device, QObject *parent) : QObject(parent)
{
    UBX_D("constructor");
    m_ackQueue.message.clear();
    m_ackQueue.ack = false;
    m_ackTimer = new QTimer(this);
    m_ackTimer->setInterval(3000);
    m_ackTimer->setSingleShot(true);
    connect(m_ackTimer, SIGNAL(timeout()), this, SLOT(ackTimeout()));
    connect(this, SIGNAL(writeMessage(QByteArray)), device, SLOT(write(QByteArray)));
}

bool UBX::crcCheck(const QByteArray &msg)
{
    quint8 ck_a = 0;
    quint8 ck_b = 0;
    int len = msg.at(2) | (msg.at(3) << 8);

    for (int i = 0; i < (msg.size() - 2); ++i) {
        ck_a += msg.at(i);
        ck_b += ck_a;
    }
    UBX_D("ck_a(" << QString::number(msg.at(len + 4)).toLatin1() << ", " << ck_a << "), ck_b("
                  << QString::number(msg.at(len + 5)).toLatin1() << ", " << ck_b << ")");
    if ((ck_a == msg.at(len + 4)) && (ck_b == msg.at(len + 5)))
        return true;

    return false;
}

void UBX::parse(const QByteArray &msg)
{
    switch (msg.at(0)) {
    case 0x01:
        if (0x21 == msg.at(1)) {
            if ((msg.size() >= 24)) {
                if (((msg.at(23) & 0x04) > 0)) {
                    QTime t(msg.at(20) & 0xFF, msg.at(21) & 0xFF, msg.at(22) & 0xFF,
                            (static_cast<qint32>((msg.at(12) & 0xFF) | ((msg.at(13) & 0xFF) << 8)
                                                 | ((msg.at(14) & 0xFF) << 16)
                                                 | ((msg.at(15) & 0xFF) << 24)))
                                    / 1000000);
                    QDate d((msg.at(16) & 0xFF) | ((msg.at(17) & 0xFF) << 8), msg.at(18) & 0xFF,
                            msg.at(19) & 0xFF);
                    if (t.isValid() && d.isValid()) {
                        QDateTime dt(d, t);
                        emit systemTimeDrift(QDateTime::currentDateTimeUtc().msecsTo(dt));
                        UBX_D("New UTC time: " << dt);
                    } else {
                        UBX_D("Time not valid yet: " << QDateTime(d, t) << "\tt: " << t.isValid()
                                                     << ",\td: " << d.isValid());
                        QTimer::singleShot(1000, this, SLOT(requestTime()));
                    }
                } else {
#ifdef UBX_DEBUG
                    QTime t(msg.at(20) & 0xFF, msg.at(21) & 0xFF, msg.at(22) & 0xFF,
                            ((qint32)((msg.at(12) & 0xFF) | ((msg.at(13) & 0xFF) << 8)
                                      | ((msg.at(14) & 0xFF) << 16) | ((msg.at(15) & 0xFF) << 24)))
                                    / 1000000);
                    QDate d((msg.at(16) & 0xFF) | ((msg.at(17) & 0xFF) << 8), msg.at(18) & 0xFF,
                            msg.at(19) & 0xFF);
                    QDateTime dt(d, t);
                    UBX_D("Still waiting for accurate time. Invalid time is: "
                          << dt << "\t\tvalidity flags: "
                          << QString::number((uint)(msg.at(23) & 0xFF), 16).toLatin1());
#endif
                    QTimer::singleShot(1000, this, SLOT(requestTime()));
                }
            }
        } else if (0x35 == msg.at(1)) {
            UBX_D("UBX-NAV-SAT");
            int payloadLen = msg.at(2) | (msg.at(3) << 8);
            if (msg.size() >= (payloadLen + 6)) {
                M8_SV_INFO info;
                info.iTOW = static_cast<quint32>(msg.at(4) | (msg.at(5) << 8) | (msg.at(6) << 16)
                                                 | (msg.at(7) << 24));
                info.version = static_cast<quint8>(msg.at(8));
                info.numSvs = static_cast<quint8>(msg.at(9));
                for (int i = 12; i <= (msg.size() - 14); i += 12) {
                    M8_SV sat;
                    sat.gnssId = static_cast<quint8>(msg.at(i));
                    sat.svId = static_cast<quint8>(msg.at(i + 1));
                    sat.cno = static_cast<quint8>(msg.at(i + 2));
                    sat.elev = static_cast<qint8>(msg.at(i + 3));
                    sat.azim = static_cast<qint16>(msg.at(i + 4) | (msg.at(i + 5) << 8));
                    sat.prRes = static_cast<qint16>(msg.at(i + 6) | (msg.at(i + 7) << 8));
                    sat.flags =
                            static_cast<quint32>(msg.at(i + 8) | (msg.at(i + 9) << 8)
                                                 | (msg.at(i + 10) << 16) | (msg.at(i + 11) << 24));
                    info.satellites.append(sat);
                }
                emit satelliteInfo(info);
            } else {
                UBX_D("Error: wrong message size for UBX-NAV-SAT");
            }
        } else if (0x60 == msg.at(1)) {
            UBX_D("Autonomous assist enabled: " << QString::number(msg.at(8)).toLatin1());
            UBX_D("Autonomous assist active:  " << QString::number(msg.at(9)).toLatin1());
        }
        break;
    case 0x05:
        if (0x01 == msg.at(1)) {
            UBX_D("ack");
            ack();
        } else if (0x00 == msg.at(1)) {
            UBX_D("!!! NACK !!!");
        }
        break;
    case 0x0A:
        if (0x09 == msg.at(1)) {
            UBX_D("Antenna status: " << QString::number(msg.at(24) & 0xFF).toLatin1());
            UBX_D("Antenna power:  " << QString::number(msg.at(25) & 0xFF).toLatin1());
        }
        break;
    default:
        UBX_D("Unknown response class: " << QString::number(msg.at(0)).toLatin1());
        break;
    }
}

void UBX::requestSatelliteInfo()
{
    UBXMessage msgReqSvInfo;
    msgReqSvInfo.ack = false;
    msgReqSvInfo.message.append(0x01); /* Message class */
    msgReqSvInfo.message.append(0x35); /* Message id */
    msgReqSvInfo.message.append(static_cast<char>(0x00)); /* Payload size */
    msgReqSvInfo.message.append(static_cast<char>(0x00)); /* Payload size */
    addMessage(msgReqSvInfo);
}

void UBX::requestTime()
{
    UBXMessage msgReqTime;
    msgReqTime.ack = false;
    msgReqTime.message.append(0x01); /* Message class */
    msgReqTime.message.append(0x21); /* Message id */
    msgReqTime.message.append(static_cast<char>(0x00)); /* Payload size */
    msgReqTime.message.append(static_cast<char>(0x00)); /* Payload size */
    addMessage(msgReqTime);
}

void UBX::addMessage(UBXMessage message)
{
    m_sendQueue.append(message);
    sendNext();
}

void UBX::sendNext()
{
    if (m_ackTimer->isActive()) {
        UBX_D("sendNext(): Waiting for ACK");
    } else {
        if (!m_sendQueue.isEmpty()) {
            // UBX_D("Sending next mesage");
            UBXMessage ubxMessage = m_sendQueue.takeFirst();
            encodeAndSend(ubxMessage.message);
            if (ubxMessage.ack) {
                // Ack requested, so we wait for the ack before sending next message
                UBX_D("Ack requested");
                m_ackQueue = ubxMessage;
                m_ackTimer->start();
            } else {
                QTimer::singleShot(10, this, SLOT(sendNext()));
            }
        }
    }
}

void UBX::encodeAndSend(const QByteArray &message)
{
    quint8 ck_a = 0;
    quint8 ck_b = 0;
    for (int i = 0; i < message.size(); ++i) {
        ck_a += (message.at(i) & 0xFF);
        ck_a = ck_a & 0xFF;
        ck_b += ck_a;
        ck_b = ck_b & 0xFF;
    }
    QByteArray data;
    data.append(static_cast<char>(0xB5));
    data.append(static_cast<char>(0x62));
    data.append(message);
    data.append(static_cast<char>(ck_a));
    data.append(static_cast<char>(ck_b));

#ifdef UBX_DEBUG
    /*UBX_D("encodeAndSend(");
    for (int i = 0; i < data.size(); ++i) {
        qDebug() << "0x" << QString::number(data.at(i) & 0xFF, 16).toLatin1();
    }
    UBX_D(")");*/
#endif
    emit writeMessage(data);
}

void UBX::ack()
{
    if (m_ackTimer->isActive()) {
        m_ackTimer->stop();
        UBX_D("Ack received");
        m_ackQueue.message.clear();
        m_ackQueue.ack = false;
        sendNext();
    } else {
        UBX_D("Ack received when not expected");
    }
}

void UBX::ackTimeout()
{
    UBX_D("Ack timeout. Resending message");
    if (!m_ackQueue.message.isEmpty()) {
        m_sendQueue.prepend(m_ackQueue);
        m_ackQueue.message.clear();
        m_ackQueue.ack = false;
        sendNext();
    }
}
