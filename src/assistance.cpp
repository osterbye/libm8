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
#include "assistance.h"
#include "ubx.h"
#include <QDateTime>
#include <QFile>

#define ASST_DEBUG
#ifdef ASST_DEBUG
#include <QDebug>
#include <QStringList>
#define ASST_D(x) qDebug() << "[Assistance] " << x
#else
#define ASST_D(x)
#endif

#define JAN_1_2022 1640995200000LL

Assistance::Assistance(QByteArray configPath, UBX *ubx, QObject *parent)
    : QObject(parent), m_offlineDirectory("")
{
    readConfig(configPath);
    if (m_assistLevel > ASSIST_OFF && QDateTime::currentMSecsSinceEpoch() > JAN_1_2022)
        ubx->injectTimeAssistance();
}

void Assistance::readConfig(QByteArray configPath)
{
    QFile cfg(configPath);
    if (cfg.exists() && cfg.open(QIODevice::ReadOnly)) {
        QByteArray line = cfg.readLine();
        while (line.length() > 0) {
            if (line.startsWith("level:")) {
                m_assistLevel = static_cast<assist_lvl_t>(line.remove(0, 6).toInt());
            } else if (line.startsWith("offlinedir:")) {
                m_offlineDirectory = line.mid(11);
            }
            line = cfg.readLine();
        }
        cfg.close();
    }

    if (m_assistLevel > ASSIST_ONLINE)
        m_assistLevel = ASSIST_AUTONOMOUS;

    if ((ASSIST_OFFLINE == m_assistLevel || ASSIST_AUTONOMOUS == m_assistLevel)
        && m_offlineDirectory.isEmpty())
        m_assistLevel = ASSIST_BASIC;

#ifdef ASST_DEBUG
    QStringList levels = QStringList() << "Off"
                                       << "Basic"
                                       << "Autonomous"
                                       << "Offline"
                                       << "Online";
    ASST_D("Assist level:" << levels.at(m_assistLevel));
    ASST_D("Offline dir:" << m_offlineDirectory);
#endif
}
