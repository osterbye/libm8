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
#include "m8.h"
#include "m8control.h"

M8::M8(QString device, QObject *parent) : QObject(parent)
{
    m_control = new M8Control(device, this);
    connect(m_control, SIGNAL(statusChange(M8_STATUS)), this, SIGNAL(statusChange(M8_STATUS)));
    connect(m_control, SIGNAL(nmea(QByteArray)), this, SIGNAL(nmea(QByteArray)));
    connect(m_control, SIGNAL(newPosition(double, double, float, quint8)), this,
            SIGNAL(newPosition(double, double, float, quint8)));
}

M8_STATUS M8::status()
{
    return m_control->status();
}
