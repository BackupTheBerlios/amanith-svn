
include(../../config/settings.conf)

win32: {

    !contains(DEFINES, WIN32_MINGW) {
        TARGET = libzlib
    }

    # Windows MinGW support
    contains(DEFINES, WIN32_MINGW) {
        TARGET = zlib
    }
}

unix: TARGET = zlib

TEMPLATE = lib

INCLUDEPATH = ./

DEPENDPATH += ./

DESTDIR = $$(AMANITHDIR)/lib

SOURCES = adler32.c \
          compress.c \
          crc32.c \
          deflate.c \
          gzio.c \
          infback.c \
          inffast.c \
          inflate.c \
          inftrees.c \
          trees.c \
          uncompr.c \
          zutil.c

CONFIG += staticlib

# this library is framework independent, it's in pure C!
CONFIG -= qt stl
