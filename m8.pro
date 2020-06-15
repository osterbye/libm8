QT -= gui

VERSION = 0.1.0
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
    include/m8_status.h


# Source
SOURCES += \
    src/m8.cpp \
    src/m8control.cpp \
    src/M8Device.cpp

HEADERS += \
    src/m8control.h \
    src/M8Device.h


DESTDIR = $$_PRO_FILE_PWD_/bin/
OBJECTS_DIR = $$_PRO_FILE_PWD_/build/.obj
MOC_DIR = $$_PRO_FILE_PWD_/build/.moc
RCC_DIR = $$_PRO_FILE_PWD_/build/.qrc
UI_DIR = $$_PRO_FILE_PWD_/build/.ui
