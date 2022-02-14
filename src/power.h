#ifndef POWER_H
#define POWER_H

#include <QObject>

class Config;
class NMEA;
class UBX;

class Power : public QObject
{
    Q_OBJECT
public:
    explicit Power(NMEA *nmea, UBX *ubx, Config *cfg, QObject *parent = nullptr);

    void setPower(bool on);

private slots:
    void newPosition(double latitude, double longitude, float altitude, quint8 satellites);

private:
    UBX *p_ubx;
    Config *p_config;
    bool m_gnssActiveRequested;
    bool m_psmActive;
};

#endif // POWER_H
