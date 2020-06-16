#include "nmea.h"

NMEA::NMEA(QObject *parent) : QObject(parent) {}

bool NMEA::crcCheck(const QByteArray &nmea)
{
    bool ok;
    quint8 crcStr = static_cast<quint8>(nmea.mid(nmea.length() - 3, 2).toInt(&ok, 16));
    if (ok) {
        quint8 crcCalc = 0;
        for (int i = 1; i < (nmea.size() - 4); ++i) {
            crcCalc ^= nmea.at(i);
        }
        return (crcStr == crcCalc);
    }
    return false;
}

void NMEA::parse(const QByteArray &nmea)
{
    if (nmea.size() >= 6 && nmea.at(3) == 'G' && nmea.at(4) == 'G' && nmea.at(5) == 'A') {
        const QList<QByteArray> nmeaFields = nmea.split(',');
        if (nmeaFields.count() >= 10 && nmeaFields.at(6).toInt() > 0) {
            /*          time       lat         lon
             * $GPGGA,130153.00,5538.937814,N,01232.581883,E,1,05,1.4,61.7,M,40.5,M,,*5E
             *    0       1          2      3       4      5 6  7   8   9  10  11 12  13
             */
            double latitude =
                    (nmeaFields.at(2).left(2).toInt() + ((nmeaFields.at(2).mid(2).toDouble()) / 60))
                    * ((nmeaFields.at(3) == "S") ? -1 : 1);
            double longitude =
                    (nmeaFields.at(4).left(3).toInt() + ((nmeaFields.at(4).mid(3).toDouble()) / 60))
                    * ((nmeaFields.at(5) == "W") ? -1 : 1);
            float altitude = nmeaFields.at(9).toFloat();
            quint8 satellites = static_cast<quint8>(nmeaFields.at(7).toUInt());
            emit newPosition(latitude, longitude, altitude, satellites);
        }
    }
}
