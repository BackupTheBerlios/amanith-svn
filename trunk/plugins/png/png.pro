include(../../config/useamanith.conf)
include(./build.conf)

TARGET = pngimpexp

TEMPLATE = lib

CONFIG += dll

DESTDIR = $$(AMANITHDIR)/plugins

DEFINES += G_MAKE_PLUGIN

SOURCES = gpngimpexp.cpp


