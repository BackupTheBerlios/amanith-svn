include(../../../config/useamanith.conf)

TEMPLATE = app
TARGET = drawanim
LANGUAGE = C++

CONFIG += qt warn_on opengl

SOURCES = main.cpp \
          drawanim.cpp

win32: RC_FILE = example.rc

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += opengl qt3support
}
