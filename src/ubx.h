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
#ifndef UBX_H
#define UBX_H

#include <QList>
#include <QObject>
#include "ubxmessage.h"
#include "m8_sv_info.h"

class M8Device;
class QTimer;

class UBX : public QObject
{
    Q_OBJECT
public:
    explicit UBX(M8Device *device, QObject *parent = nullptr);

    bool crcCheck(const QByteArray &msg);
    void parse(const QByteArray &msg);
    void configureNMEA();
    void injectTimeAssistance();
    void setEngineState(bool on);
    void setPowerSave(bool on);
    void setAutonomousAssist(bool enabled);
    void requestSatelliteInfo();
    void requestNavigationDatabase();

public slots:
    void requestTime();

signals:
    void systemTimeDrift(qint64 offsetMilliseconds);
    void satelliteInfo(M8_SV_INFO info);
    void writeMessage(const QByteArray &msg);
    void saveNavigationEntry(QByteArray entry);

private slots:
    void addMessage(UBXMessage message, bool priority = false);
    void sendNext();
    void encodeAndSend(const QByteArray &message);
    void ack();
    void ackTimeout();

private:
    UBXMessage m_ackQueue;
    QList<UBXMessage> m_sendQueue;
    QTimer *m_ackTimer;
    QByteArray m_UbxCfgNavx5;
    bool m_autonomousAssist;
};

#endif // UBX_H
