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
                m_assistLevel = static_cast<ASSIST_LEVEL>(line.remove(0, 6).toInt());
            } else if (line.startsWith("offlinedir:")) {
                m_offlineDirectory = line.mid(11);
            } else if (line.startsWith("powersave:")) {
                m_powerSave = static_cast<bool>(line.remove(0, 10).toInt());
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

QByteArray Config::offlineDir()
{
    return m_offlineDirectory;
}

bool Config::powerSave()
{
    return m_powerSave;
}
