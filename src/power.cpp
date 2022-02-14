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
