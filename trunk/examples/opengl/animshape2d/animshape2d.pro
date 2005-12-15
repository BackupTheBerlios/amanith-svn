include(../../../config/useamanith.conf)

TEMPLATE = app
TARGET = drawanim
LANGUAGE = C++

CONFIG += qt warn_on opengl

DEFINES += TIXML_USE_STL

SOURCES = main.cpp \
          drawanim.cpp \
          tinystr.cpp \
          tinyxml.cpp \
          tinyxmlerror.cpp \
          tinyxmlparser.cpp

win32: RC_FILE = example.rc

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += opengl
}
