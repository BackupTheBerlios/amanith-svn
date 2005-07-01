include(../../config/useamanith.conf)

TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on

HEADERS	+= drawer.h

SOURCES	+= main.cpp \
	drawer.cpp

FORMS	= canvasresizeform.ui \
	resizeform.ui

IMAGES	= ./images/amanith32.png

TARGET = viewimage
