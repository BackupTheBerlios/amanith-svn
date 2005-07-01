include(../../config/useamanith.conf)
include(./build.conf)

TARGET = fontimpexp

TEMPLATE = lib

CONFIG += dll

DESTDIR = $$(AMANITHDIR)/plugins

DEFINES += G_MAKE_PLUGIN

SOURCES = gfontsimpexp.cpp


