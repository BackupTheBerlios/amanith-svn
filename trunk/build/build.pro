include(../config/settings.conf)

TEMPLATE = lib

TARGET = amanith

INCLUDEPATH += $$(AMANITHDIR)/include

DESTDIR = $$(AMANITHDIR)/lib

DEPENDPATH += $$(AMANITHDIR)/src


#*********************************************************
#
#                       KERNEL SOURCES
#
#*********************************************************
SOURCES = gelement.cpp \
          gproperty.cpp \
          gmultiproperty.cpp \
          gkernel.cpp \
          gerror.cpp \
          gimpexp.cpp \
          gpluglib.cpp \
          1d/gcurve1d.cpp \
          1d/gmulticurve1d.cpp \
          1d/gbeziercurve1d.cpp \
          1d/gbsplinecurve1d.cpp \
          1d/gpolylinecurve1d.cpp \
          1d/ghermitecurve1d.cpp \
          2d/gcurve2d.cpp \
          2d/gmulticurve2d.cpp \
          2d/gbeziercurve2d.cpp \
          2d/gbsplinecurve2d.cpp \
          2d/gpolylinecurve2d.cpp \
          2d/ghermitecurve2d.cpp \
          2d/gellipsecurve2d.cpp \
          2d/gmesh2d.cpp \
          2d/gpath2d.cpp \
          2d/gtracer2d.cpp \
          2d/gpixelmap.cpp \
          2d/gfont2d.cpp \
          2d/gtesselator2d.cpp \
          2d/ganimtrsnode2d.cpp \
          geometry/gaffineparts.cpp \
          numerics/geigen.cpp \
          numerics/gintegration.cpp \
          rendering/gdrawstyle.cpp \
          rendering/gdrawboard.cpp \
          rendering/gopenglboard.cpp \
          rendering/gopenglstyles.cpp \
          rendering/gopenglmasks.cpp \
          rendering/gopenglgroups.cpp \
          rendering/gopenglradialgrad.cpp \
          rendering/gopenglconicalgrad.cpp \
          rendering/gopenglstroke.cpp \
          rendering/gopenglgeometries.cpp \
          support/gblowfish.cpp \
          support/gavltree.cpp \
          support/gutilities.cpp


#*********************************************************
#
#                 OPENGL EXTENSIONS PLUGIN
#
#*********************************************************
contains(DEFINES, _OPENGLEXT_PLUGIN) {
    SOURCES += support/glew.c gopenglext.cpp
    CONFIG += opengl

    # Qt4 syntax
    contains(DEFINES, USE_QT4) {
        QT += opengl
    }
}



#*********************************************************
#
#                     JPEG PLUGIN
#
#*********************************************************

contains(DEFINES, _JPEG_PLUGIN) {
    contains(JPEG_PLUGIN_OPTIONS, INTERNAL) {
        include($$(AMANITHDIR)/plugins/jpeg/build.conf)
        SOURCES += $$(AMANITHDIR)/plugins/jpeg/gjpegimpexp.cpp
    }
}

#*********************************************************
#
#                     PNG PLUGIN
#
#*********************************************************

contains(DEFINES, _PNG_PLUGIN) {
    contains(PNG_PLUGIN_OPTIONS, INTERNAL) {
        include($$(AMANITHDIR)/plugins/png/build.conf)
        SOURCES += $$(AMANITHDIR)/plugins/png/gpngimpexp.cpp
    }
}

#*********************************************************
#
#                    FONTS PLUGIN
#
#*********************************************************
contains(DEFINES, _FONTS_PLUGIN) {
    contains(FONTS_PLUGIN_OPTIONS, INTERNAL) {
        include($$(AMANITHDIR)/plugins/fonts/build.conf)
        SOURCES += $$(AMANITHDIR)/plugins/fonts/gfontsimpexp.cpp
    }
}


#*********************************************************
#
#  BUILD AND LINK SPECIFICATION: PLEASE DO NOT TOUCH !!!
#
#*********************************************************


# in this case we use stl, so remove qt dependencies!
!contains(DEFINES, _USE_QT) {
    CONFIG -= qt
    CONFIG += stl
 }

contains(BUILD_MODE, dynamic) {
    CONFIG -= staticlib
    CONFIG += dll
    DEFINES += G_MAKE_DLL GLEW_BUILD G_MAKE_PLUGIN
    DEFINES -= G_USE_DLL G_NO_DLL GLEW_STATIC

    # Windows MinGW support
    contains(DEFINES, WIN32_MINGW) {
        win32: LIBS += -L$$(AMANITHDIR)/lib
    }
    unix: LIBS += -L$$(AMANITHDIR)/lib
}

contains(BUILD_MODE, static) {
    CONFIG -= dll
    CONFIG += staticlib
    DEFINES += G_NO_DLL GLEW_STATIC
    DEFINES -= G_MAKE_DLL G_USE_DLL GLEW_BUILD
}

# link options for Windows (no MinGW!)
!contains(DEFINES, WIN32_MINGW) {
    win32: LIBS += wbemuuid.lib
}
