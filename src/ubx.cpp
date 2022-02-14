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
#include "ubx.h"
#include "m8device.h"
#include <QDateTime>
#include <QTimer>

//#define UBX_DEBUG
#ifdef UBX_DEBUG
#include <QDebug>
#include <QStringList>
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
    connect(m_ackTimer, &QTimer::timeout, this, &UBX::ackTimeout);
    connect(this, &UBX::writeMessage, device, &M8Device::write);
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
    /*UBX_D("ck_a(" << QString::number(msg.at(len + 4)).toLatin1() << ", " << ck_a << "), ck_b("
                  << QString::number(msg.at(len + 5)).toLatin1() << ", " << ck_b << ")");*/
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
                if (((msg.at(23) & 0x04) > 0) || ((msg.at(23) & 0x03) == 0x03)) {
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
                        QTimer::singleShot(1000, this, &UBX::requestTime);
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
                    QTimer::singleShot(1000, this, &UBX::requestTime);
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
            UBX_D("nack");
            ack();
        }
        break;
    case 0x06:
        if (0x3E == msg.at(1)) {
#ifdef UBX_DEBUG
            UBX_D("GNSS configuration:");
            UBX_D("Version:\t\t" << QString::number(msg.at(4) & 0xFF).toLatin1());
            UBX_D("numTrkChHw:\t" << QString::number(msg.at(5) & 0xFF).toLatin1());
            UBX_D("numTrkChUse:\t" << QString::number(msg.at(6) & 0xFF).toLatin1());
            UBX_D("numConfigBlocks:\t" << QString::number(msg.at(7) & 0xFF).toLatin1());
            const QStringList gnssNames = (QStringList() << "GPS"
                                                         << "SBAS"
                                                         << "Galileo"
                                                         << "BeiDou"
                                                         << "IMES"
                                                         << "QZSS"
                                                         << "GLONASS"
                                                         << "IRNSS");
            int len = (msg.at(2) & 0xFF) | (msg.at(3) << 8);
            int i = 8;
            while ((i + 8) <= (len + 4) && (i + 8) < msg.size()) {
                if (msg.at(i) < gnssNames.size()) {
                    UBX_D("System:\t\t" << gnssNames.at(msg.at(i++)));
                } else {
                    UBX_D("System:\t\t" << QString::number(msg.at(i++) & 0xFF).toLatin1());
                }
                UBX_D("\tresTrkCh:\t" << QString::number(msg.at(i++) & 0xFF).toLatin1());
                UBX_D("\tmaxTrkCh:\t" << QString::number(msg.at(i++) & 0xFF).toLatin1());
                ++i; // Skip reserved byte
                UBX_D("\tEnabled:\t" << (msg.at(i++) & 0xFF));
                ++i; // Skip empty byte
                UBX_D("\tFlags (hex):\t" << QString::number(msg.at(i++) & 0xFF, 16).toLatin1());
                ++i; // Skip empty byte
            }
#endif
        }
        break;
    case 0x0A:
        if (0x09 == msg.at(1)) {
            UBX_D("Antenna status: " << QString::number(msg.at(24) & 0xFF).toLatin1());
            UBX_D("Antenna power:  " << QString::number(msg.at(25) & 0xFF).toLatin1());
        } else if (0x28 == msg.at(1)) {
            UBX_D("GNSS supported:\t\t" << QString::number(msg.at(5) & 0xFF).toLatin1());
            UBX_D("GNSS default:\t\t" << QString::number(msg.at(6) & 0xFF).toLatin1());
            UBX_D("GNSS enabled:\t\t" << QString::number(msg.at(7) & 0xFF).toLatin1());
            UBX_D("GNSS simultaneous:\t" << QString::number(msg.at(8) & 0xFF).toLatin1());
        }
        break;
    default:
        UBX_D("Unknown response class: " << QString::number(msg.at(0)).toLatin1());
        break;
    }
}

void UBX::configureNMEA()
{
    // Disable all NMEA messsages except GGA
    UBXMessage msgNMEAConf;
    msgNMEAConf.ack = true;
    msgNMEAConf.message.append(0x06); /* Message class */
    msgNMEAConf.message.append(0x01); /* Message id */
    msgNMEAConf.message.append(0x08); /* Payload size */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* Payload size */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* msgClass */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* msgID */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* rate port 0 */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* rate port 1 */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* rate port 2 */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* rate port 3 */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* rate port 4 */
    msgNMEAConf.message.append(static_cast<char>(0x00)); /* rate port 5 */

    /* Message: DTM */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x0A; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GBQ */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x44; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GBS */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x09; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GLL */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x01; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GLQ */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x43; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GNQ */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x42; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GNS */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x0D; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GPQ */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x40; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GRS */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x06; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GSA */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x02; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GST */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x07; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: GSV */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x03; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: RMC */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x04; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: THS */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x0E; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: TXT */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x41; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: VLW */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x0F; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: VTG */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x05; /* msgID */
    addMessage(msgNMEAConf);

    /* Message: ZDA */
    msgNMEAConf.message[4] = 0xF0; /* msgClass */
    msgNMEAConf.message[5] = 0x08; /* msgID */
    addMessage(msgNMEAConf);

#ifdef UBX_DEBUG
    /* Get enabled GNSS systems */
    UBXMessage msgGNSS;
    msgGNSS.ack = false;
    msgGNSS.message.append(0x0A); /* Message class */
    msgGNSS.message.append(0x28); /* Message id */
    msgGNSS.message.append(static_cast<char>(0x00)); /* Payload size */
    msgGNSS.message.append(static_cast<char>(0x00)); /* Payload size */
    addMessage(msgGNSS);

    /* Get individual GNSS configurations */
    msgGNSS.message[0] = 0x06;
    msgGNSS.message[1] = 0x3E;
    addMessage(msgGNSS);
#endif
}

void UBX::injectTimeAssistance()
{
    UBXMessage msgIniTime;
    msgIniTime.ack = false;
    msgIniTime.message.append(0x13); /* Message class */
    msgIniTime.message.append(0x40); /* Message id */
    msgIniTime.message.append(static_cast<char>(0x18)); /* Payload size */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Payload size */
    msgIniTime.message.append(0x10); /* Message type */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Message version */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Source (0: on receipt of message) */
    msgIniTime.message.append(0x80); /* Leap seconds since 1980 (or 0x80 = -128 if unknown) */
    QDateTime now = QDateTime::currentDateTimeUtc();
    msgIniTime.message.append(now.date().year() & 0xFF); /* Year */
    msgIniTime.message.append((now.date().year() >> 8) & 0xFF); /* Year */
    msgIniTime.message.append(now.date().month() & 0xFF); /* Month */
    msgIniTime.message.append(now.date().day() & 0xFF); /* Day */
    msgIniTime.message.append(now.time().hour() & 0xFF); /* Hour */
    msgIniTime.message.append(now.time().minute() & 0xFF); /* Minute */
    msgIniTime.message.append(now.time().second() & 0xFF); /* Seconds */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Reserved1 */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Nanoseconds */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Nanoseconds */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Nanoseconds */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Nanoseconds */
    msgIniTime.message.append(0x10); /* Seconds part of time accuracy */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Seconds part of time accuracy */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Reserved2 */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Reserved2 */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Nanoseconds part of time accuracy */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Nanoseconds part of time accuracy */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Nanoseconds part of time accuracy */
    msgIniTime.message.append(static_cast<char>(0x00)); /* Nanoseconds part of time accuracy */
    addMessage(msgIniTime, true);
}

void UBX::setEngineState(bool on)
{
    quint8 mode = (on) ? 0x09 : 0x08;
    UBXMessage msgRST;
    msgRST.ack = false;
    msgRST.message.append(0x06); /* Message class */
    msgRST.message.append(0x04); /* Message id */
    msgRST.message.append(0x04); /* Payload size */
    msgRST.message.append(static_cast<char>(0x00)); /* Payload size */
    msgRST.message.append(static_cast<char>(0x00)); /* navBbrMask */
    msgRST.message.append(static_cast<char>(0x00)); /* navBbrMask */
    msgRST.message.append(mode); /* resetMode */
    msgRST.message.append(static_cast<char>(0x00)); /* Reserved1 */
    addMessage(msgRST);
}

void UBX::setPowerSave(bool on)
{
    quint8 mode = 0;
    if (on) {
        mode = 1;
        UBXMessage msgPMS;
        msgPMS.ack = false;
        msgPMS.message.append(0x06); /* Message class */
        msgPMS.message.append(0x86); /* Message id */
        msgPMS.message.append(0x08); /* Payload size */
        msgPMS.message.append(static_cast<char>(0x00)); /* Payload size */
        msgPMS.message.append(static_cast<char>(0x00)); /* Message version */
        msgPMS.message.append(0x03); /* Aggressive with 1Hz */
        msgPMS.message.append(static_cast<char>(0x00)); /* Period, for interval mode */
        msgPMS.message.append(static_cast<char>(0x00)); /* Period, for interval mode */
        msgPMS.message.append(static_cast<char>(0x00)); /* onTime, for interval mode */
        msgPMS.message.append(static_cast<char>(0x00)); /* onTime, for interval mode */
        msgPMS.message.append(static_cast<char>(0x00)); /* Reserved1 */
        msgPMS.message.append(static_cast<char>(0x00)); /* Reserved1 */
        addMessage(msgPMS);
    }

    UBXMessage msgRXM;
    msgRXM.ack = false;
    msgRXM.message.append(0x06); /* Message class */
    msgRXM.message.append(0x11); /* Message id */
    msgRXM.message.append(0x02); /* Payload size */
    msgRXM.message.append(static_cast<char>(0x00)); /* Payload size */
    msgRXM.message.append(static_cast<char>(0x00)); /* Reserved1 */
    msgRXM.message.append(mode); /* lpMode */
    addMessage(msgRXM);
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

void UBX::addMessage(UBXMessage message, bool priority)
{
    if (priority) {
        m_sendQueue.prepend(message);
    } else {
        m_sendQueue.append(message);
    }
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
                QTimer::singleShot(10, this, &UBX::sendNext);
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
        // UBX_D("Ack/Nack received");
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
        // Only try resend once
        m_ackQueue.ack = false;
        m_sendQueue.prepend(m_ackQueue);
        m_ackQueue.message.clear();
        sendNext();
    }
}
