include(../../config/useamanith.conf)

TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on

SOURCES	+= main.cpp \
	drawer.cpp

FORMS	= canvasresizeform.ui \
	resizeform.ui

IMAGES	= ./images/amanith32.png

TARGET = viewimage

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += qt3support
    CONFIG += uic3
    HEADERS += drawer_qt4.h
}

!contains(DEFINES, USE_QT4) {
    HEADERS += drawer.h
}
