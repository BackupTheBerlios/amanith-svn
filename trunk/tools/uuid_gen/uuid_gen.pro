include(../../config/settings.conf)

TEMPLATE = app
LANGUAGE = C++

CONFIG += qt warn_on

HEADERS	+= uuidgenfrm.h

SOURCES	+= main.cpp

FORMS = uuidgenfrmbase.ui

TARGET = uuid_gen

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += qt3support
    CONFIG += uic3
}
