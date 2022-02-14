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
