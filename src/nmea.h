#ifndef NMEA_H
#define NMEA_H

#include <QObject>

class NMEA : public QObject
{
    Q_OBJECT
public:
    explicit NMEA(QObject *parent = nullptr);

    bool crcCheck(const QByteArray &nmea);
    void parse(const QByteArray &nmea);

signals:
    void newPosition(double latitude, double longitude, float altitude, quint8 satellites);
};

#endif // NMEA_H
