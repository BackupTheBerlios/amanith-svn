include(../../../config/useamanith.conf)

TEMPLATE   = app
LANGUAGE   = C++
TARGET     = lowlevel
CONFIG     += warn_on console
SOURCES    = main.cpp

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += qt3support
}