include(../../../config/useamanith.conf)

TEMPLATE       = app
TARGET         = vectorizer
LANGUAGE       = C++

CONFIG        += qt warn_on opengl

HEADERS        = drawer.h
SOURCES        = main.cpp \
                 drawer.cpp

win32: RC_FILE = example.rc

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += opengl qt3support
}
