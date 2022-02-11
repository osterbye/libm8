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
#ifndef M8_SV_INFO_H
#define M8_SV_INFO_H

#include <QtCore/qglobal.h>
#include <QList>

/**
 * @brief Satellite signal information
 */
struct M8_SV {
    quint8 gnssId; /* GNSS identifier */
    quint8 svId; /* Satellite identifier */
    quint8 cno; /* Carrier to noise ratio (signal strength) [dBHz] */
    qint8 elev; /* Elevation (range: +/-90), unknown if out of range [deg] */
    qint16 azim; /* Azimuth (range 0-360), unknown if elevation is out of range [deg] */
    qint16 prRes; /* Pseudorange residual [m] */
    quint32 flags; /* Bitmask (see u-blox M8 protocol specification) */
};

/**
 * @brief UBX-NAV-SAT tracked satellites information
 */
struct M8_SV_INFO {
    quint32 iTOW; /* GPS time of week of the navigation epoch. [ms]  */
    quint8 version; /* Message version */
    quint8 numSvs; /* Number of satellites */
    QList<M8_SV> satellites;
};

#endif // M8_SV_INFO_H
