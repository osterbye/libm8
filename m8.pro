QT -= gui

VERSION = 0.1.2
TEMPLATE = lib
DEFINES += M8_LIBRARY

QMAKE_CXXFLAGS += -Wall -Wextra
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
    include/

# Public headers
HEADERS += \
    include/m8_global.h \
    include/m8.h \
    include/m8_status.h \
    include/m8_sv_info.h

# Source
SOURCES += \
    src/m8.cpp \
    src/m8control.cpp \
    src/m8device.cpp \
    src/nmea.cpp \
    src/ubx.cpp

HEADERS += \
    src/m8control.h \
    src/m8device.h \
    src/nmea.h \
    src/ubx.h \
    src/ubxmessage.h


DESTDIR = $$_PRO_FILE_PWD_/bin/
OBJECTS_DIR = $$_PRO_FILE_PWD_/build/.obj
MOC_DIR = $$_PRO_FILE_PWD_/build/.moc
RCC_DIR = $$_PRO_FILE_PWD_/build/.qrc
UI_DIR = $$_PRO_FILE_PWD_/build/.ui
