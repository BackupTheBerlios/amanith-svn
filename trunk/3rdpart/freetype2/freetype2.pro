include(../../config/settings.conf)

TEMPLATE = lib

INCLUDEPATH = ./include ../zlib

DEPENDPATH += ./src

DESTDIR = $$(AMANITHDIR)/lib

SOURCES =  base/ftsystem.c \
           base/ftinit.c \
           base/ftdebug.c \
           base/ftbase.c \
           base/ftglyph.c \
           base/ftbitmap.c \
           base/ftmm.c \
           base/ftpfr.c \
           sfnt/sfnt.c \
           psnames/psnames.c\ 
           psaux/psaux.c \
           pshinter/pshinter.c \
           gzip/ftgzip.c

# MacOS X and Tiger support
unix: DEFINES += DARWIN_NO_CARBON

# take into account compilation mode (debug or release)
DEFINES -= FT_DEBUG_LEVEL_ERROR FT_DEBUG_LEVEL_TRACE
contains(BUILD_MODE, debug) {
#    DEFINES += FT_DEBUG_LEVEL_ERROR FT_DEBUG_LEVEL_TRACE
}

#select every used drivers
contains(FONTS_PLUGIN_OPTIONS, TRUETYPE) {
    DEFINES += _FONTS_TRUETYPE_MODULE
    SOURCES += truetype/truetype.c
}

contains(FONTS_PLUGIN_OPTIONS, TYPE1) {
    DEFINES += _FONTS_TYPE1_MODULE
    SOURCES += type1/type1.c
}
contains(FONTS_PLUGIN_OPTIONS, TYPE42) {
    DEFINES += _FONTS_TYPE42_MODULE
    SOURCES += type42/type42.c
}
contains(FONTS_PLUGIN_OPTIONS, CFF) {
    DEFINES += _FONTS_CFF_MODULE
    SOURCES += cff/cff.c
}
contains(FONTS_PLUGIN_OPTIONS, PFR) {
    DEFINES += _FONTS_PFR_MODULE
    SOURCES += pfr/pfr.c
}
contains(FONTS_PLUGIN_OPTIONS, CID) {
    DEFINES += _FONTS_CID_MODULE
    SOURCES += cid/type1cid.c
}
#contains(FONTS_PLUGIN_OPTIONS, MAC) {
#    DEFINES += _FONTS_MAC_MODULE
#    SOURCES += base/ftmac_wrap.c
#}

CONFIG += staticlib

# this library is framework independent, it's in pure C!
CONFIG -= qt stl

unix: {
  TARGET = freetype2
  LIBS += $$AMANITH_DIR/lib/libzlib.a
}

win32: {
  TARGET = libfreetype2
  LIBS += $$AMANITH_DIR/lib/zlib.lib
}
