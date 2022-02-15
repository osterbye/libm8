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
#include "power.h"
#include "config.h"
#include "nmea.h"
#include "ubx.h"

Power::Power(NMEA *nmea, UBX *ubx, Config *cfg, QObject *parent)
    : QObject(parent), p_ubx(ubx), p_config(cfg), m_gnssActiveRequested(true), m_psmActive(false)
{
    if (cfg->powerSave())
        connect(nmea, &NMEA::newPosition, this, &Power::newPosition);
}

void Power::setPower(bool on)
{
    if (on != m_gnssActiveRequested) {
        p_ubx->setEngineState(on);
        m_gnssActiveRequested = on;
    }
}

void Power::newPosition(double latitude, double longitude, float altitude, quint8 satellites)
{
    Q_UNUSED(latitude)
    Q_UNUSED(longitude)
    Q_UNUSED(altitude)

    if (!m_psmActive && satellites >= 10) {
        m_psmActive = true;
        p_ubx->setPowerSave(true);
    } else if (m_psmActive && satellites <= 6) {
        m_psmActive = false;
        p_ubx->setPowerSave(false);
    }
}
