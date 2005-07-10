include(../../../config/useamanith.conf)

TEMPLATE       = app
TARGET         = path2d
LANGUAGE       = C++

CONFIG        += qt warn_on opengl

SOURCES        = main.cpp \
                 drawpath.cpp

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += opengl qt3support
}
