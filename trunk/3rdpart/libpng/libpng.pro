include(../../config/settings.conf)

TEMPLATE = lib

INCLUDEPATH = ./ ../zlib

DEPENDPATH += ./

DESTDIR = $$(AMANITHDIR)/lib


SOURCES = png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c\
          pngrio.c pngrtran.c pngrutil.c pngset.c pngtrans.c \
          pngwio.c pngwrite.c pngwtran.c pngwutil.c


# read options
!contains(PNG_PLUGIN_OPTIONS, READ) {
    DEFINES += PNG_NO_READ_SUPPORTED
}


# write options
!contains(PNG_PLUGIN_OPTIONS, WRITE) {
    DEFINES += PNG_NO_WRITE_SUPPORTED
}

# we force a static libpng
DEFINES += PNG_STATIC

CONFIG += staticlib

# this library is framework independent, it's in pure C!
CONFIG -= qt stl

unix: {
    TARGET = png
    LIBS += $$AMANITH_DIR/lib/libzlib.a
}

win32: {

    !contains(DEFINES, WIN32_MINGW) {
        TARGET = libpng
        LIBS += $$AMANITH_DIR/lib/libzlib.lib
    }

    # Windows MinGW support
    contains(DEFINES, WIN32_MINGW) {
        TARGET = png
        LIBS += $$AMANITH_DIR/lib/libzlib.a
    }
}


