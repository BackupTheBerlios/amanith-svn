include(../../../config/useamanith.conf)

TEMPLATE = app
TARGET = drawbezier
LANGUAGE = C++

CONFIG += qt warn_on opengl

SOURCES = main.cpp \
          drawbezier.cpp

win32: RC_FILE = example.rc

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += opengl
}
