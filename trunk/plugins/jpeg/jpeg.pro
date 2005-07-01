include(../../config/useamanith.conf)
include(./build.conf)

TARGET = jpegimpexp

TEMPLATE = lib

CONFIG += dll

DESTDIR = $$(AMANITHDIR)/plugins

DEFINES += G_MAKE_PLUGIN

SOURCES = gjpegimpexp.cpp


