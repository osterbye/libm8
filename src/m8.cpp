/*
MIT License

Copyright (c) 2020-2021 Nikolaj Due Østerbye

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
    connect(m_control, &M8Control::statusChange, this, &M8::statusChange);
    connect(m_control, &M8Control::nmea, this, &M8::nmea);
    connect(m_control, &M8Control::newPosition, this, &M8::newPosition);
    connect(m_control, &M8Control::systemTimeDrift, this, &M8::systemTimeDrift);
    connect(m_control, &M8Control::satelliteInfo, this, &M8::satelliteInfo);
}

M8_STATUS M8::status()
{
    return m_control->status();
}

void M8::requestTime()
{
    m_control->requestTime();
}

void M8::requestSatelliteInfo()
{
    m_control->requestSatelliteInfo();
}
