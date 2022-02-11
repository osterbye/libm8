#include "power.h"
#include "config.h"
#include "nmea.h"
#include "ubx.h"

Power::Power(NMEA *nmea, UBX *ubx, Config *cfg, QObject *parent) : QObject(parent), p_ubx(ubx), p_config(cfg)
{
    if (cfg->powerSave())
        connect(nmea, &NMEA::newPosition, this, &Power::newPosition);

}

void Power::setPower(bool on)
{
/*
UBX-CFG-RST
    - Controlled GNSS start/stop
*/
}

void Power::newPosition(double latitude, double longitude, float altitude, quint8 satellites)
{
    Q_UNUSED(latitude)
    Q_UNUSED(longitude)
    Q_UNUSED(altitude)

    if (satellites >= 4)
        p_ubx->setPowerSave(true);
}
