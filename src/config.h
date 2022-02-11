#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>

typedef enum {
    ASSIST_OFF,
    ASSIST_BASIC,
    ASSIST_AUTONOMOUS,
    ASSIST_OFFLINE, /* Not supported yet */
    ASSIST_ONLINE /* Not supported yet */
} ASSIST_LEVEL;

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QByteArray configPath, QObject *parent = nullptr);

    ASSIST_LEVEL assistLevel();
    QByteArray offlineDir();
    bool powerSave();

private:
    ASSIST_LEVEL m_assistLevel;
    QByteArray m_offlineDirectory;
    bool m_powerSave;
};

#endif // CONFIG_H
