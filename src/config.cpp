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
#include "config.h"
#include <QFile>

#define CFG_DEBUG
#ifdef CFG_DEBUG
#include <QDebug>
#include <QStringList>
#define CFG_D(x) qDebug() << "[Assistance] " << x
#else
#define CFG_D(x)
#endif

Config::Config(QByteArray configPath, QObject *parent)
    : QObject(parent), m_assistLevel(ASSIST_BASIC), m_offlineDirectory(""), m_powerSave(false)
{
    QFile cfg(configPath);
    if (cfg.exists() && cfg.open(QIODevice::ReadOnly)) {
        QByteArray line = cfg.readLine();
        while (line.length() > 0) {
            if (line.startsWith("level:")) {
                m_assistLevel = static_cast<ASSIST_LEVEL>(line.remove(0, 6).trimmed().toInt());
            } else if (line.startsWith("offlinedir:")) {
                m_offlineDirectory = line.mid(11).trimmed();
            } else if (line.startsWith("powersave:")) {
                m_powerSave = static_cast<bool>(line.remove(0, 10).trimmed().toInt());
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

#ifdef CFG_DEBUG
    QStringList levels = QStringList() << "Off"
                                       << "Basic"
                                       << "Autonomous"
                                       << "Offline"
                                       << "Online";
    CFG_D("Assist level:" << levels.at(m_assistLevel));
    CFG_D("Offline dir:" << m_offlineDirectory);
    CFG_D("Power Save:" << m_powerSave);
#endif
}

ASSIST_LEVEL Config::assistLevel()
{
    return m_assistLevel;
}

QString Config::offlineDir()
{
    return m_offlineDirectory;
}

bool Config::powerSave()
{
    return m_powerSave;
}
