
include(../../config/settings.conf)

TEMPLATE = lib

unix: TARGET = jpeg
win32: {
    !contains(DEFINES, WIN32_MINGW) {
        TARGET = libjpeg
    }

    # Windows MinGW support
    contains(DEFINES, WIN32_MINGW) {
        TARGET = jpeg
    }
}

INCLUDEPATH = ./

DEPENDPATH += ./

DESTDIR = $$(AMANITHDIR)/lib


SOURCES = jerror.c jfdctflt.c jfdctfst.c \
          jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c \
          jquant2.c jutils.c jmemmgr.c jcomapi.c


contains(JPEG_PLUGIN_OPTIONS, READ) {
    SOURCES += jdapimin.c jdapistd.c \
               jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
               jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
               jdpostct.c jdsample.c jdtrans.c
}

contains(JPEG_PLUGIN_OPTIONS, WRITE) {
    SOURCES += jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
               jcinit.c jcmainct.c jcmarker.c jcmaster.c jcparam.c \
               jcphuff.c jcprepct.c jcsample.c jctrans.c jdatadst.c
}

# if read and write weren't specified, default behavior is a read and write enabled library
!contains(JPEG_PLUGIN_OPTIONS, WRITE) {
    !contains(JPEG_PLUGIN_OPTIONS, READ) {

        SOURCES += jdapimin.c jdapistd.c \
                   jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
                   jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
                   jdpostct.c jdsample.c jdtrans.c


        SOURCES += jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
                   jcinit.c jcmainct.c jcmarker.c jcmaster.c jcparam.c \
                   jcphuff.c jcprepct.c jcsample.c jctrans.c jdatadst.c

    }
}


# memory models options. Just one of these files must be included:
#
# jmemansi.c: This version uses the ANSI-standard library routine tmpfile(),
#             which not all non-ANSI systems have.  On some systems
#             tmpfile() may put the temporary file in a non-optimal
#             location; if you don't like what it does, use jmemname.c
#
# jmemname.c: This version creates named temporary files.  For anything
#             except a Unix machine, you'll need to configure the
#             select_file_name() routine appropriately; see the comments
#             near the head of jmemname.c.  If you use this version, define
#             NEED_SIGNAL_CATCHER in jconfig.h to make sure the temp files
#             are removed if the program is aborted.
#
# jmemnobs.c: (That stands for No Backing Store :-).)  This will compile on
#              almost any system, but it assumes you have enough main memory
#              or virtual memory to hold the biggest images you work with.
#
# jmemdos.c: This should be used with most 16-bit MS-DOS compilers.
#            See the system-specific notes about MS-DOS for more info.
#            IMPORTANT: if you use this, define USE_MSDOS_MEMMGR in
#            jconfig.h, and include the assembly file jmemdosa.asm in the
#            programs.  The supplied makefiles and jconfig files for
#            16-bit MS-DOS compilers already do both.
#
# jmemmac.c: Custom version for Apple Macintosh; see the system-specific
#            notes for Macintosh for more info.

SOURCES += jmemnobs.c

CONFIG += staticlib

# this library is framework independent, it's in pure C!
CONFIG -= qt stl
