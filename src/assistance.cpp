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
#include "config.h"
#include "ubx.h"
#include <QDateTime>
#include <QDir>
#include <QStringBuilder>
#include <QStringList>

//#define ASST_DEBUG
#ifdef ASST_DEBUG
#include <QDebug>
#define ASST_D(x) qDebug() << "[Assistance] " << x
#else
#define ASST_D(x)
#endif

#define JAN_1_2022 1640995200000LL

Assistance::Assistance(UBX *ubx, Config *cfg, QObject *parent)
    : QObject(parent), p_cfg(cfg), p_ubx(ubx), m_entryNumber(0)
{
    if (cfg->assistLevel() > ASSIST_OFF && QDateTime::currentMSecsSinceEpoch() > JAN_1_2022)
        ubx->injectTimeAssistance();

    if (ASSIST_AUTONOMOUS == cfg->assistLevel()) {
        ubx->setAutonomousAssist(true);
        uploadAutonomousAssistData();
    }

    connect(ubx, &UBX::saveNavigationEntry, this, &Assistance::saveNavigationEntry);
}

void Assistance::saveAutonomousAssistData()
{
    if (!p_cfg->offlineDir().isEmpty()) {
        m_entryNumber = 0;
        p_ubx->requestNavigationDatabase();
    }
}

void Assistance::uploadAutonomousAssistData()
{
    QDir d(p_cfg->offlineDir(), { "*.dbd" });
    for (QString &filename : d.entryList()) {
        QFile f(p_cfg->offlineDir() % '/' % filename);
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray payload = f.readAll();
            f.close();
            p_ubx->uploadNavigationDatabase(payload);
            ASST_D("Upload " << filename << ":\n\t" << payload);
        }
    }
}

void Assistance::saveNavigationEntry(QByteArray entry)
{
    if (0 == m_entryNumber) {
        QDir d(p_cfg->offlineDir(), { "*.dbd" });
        for (QString &filename : d.entryList()) {
            d.remove(filename);
        }
    }

    QString fpath = QString("%1/mga%2.dbd")
                            .arg(p_cfg->offlineDir())
                            .arg(m_entryNumber, 3, 10, QLatin1Char('0'));
    QFile f(fpath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(entry);
        f.close();
        ASST_D("Save " << f.fileName() << ":\n\t" << entry);
    }
    ++m_entryNumber;
}
