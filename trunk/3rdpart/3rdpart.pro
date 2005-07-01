include(../config/settings.conf)

TEMPLATE     = subdirs
CONFIG       += ordered


contains(DEFINES, _JPEG_PLUGIN) {
    SUBDIRS += libjpeg
}

# NB: zlib must be included (built) for PNG and/or FONT plugins

contains(DEFINES, _PNG_PLUGIN) {
    SUBDIRS += zlib libpng
}

contains(DEFINES, _FONTS_PLUGIN) {
   !contains(SUBDIRS, zlib) {
       SUBDIRS += zlib
   }
   SUBDIRS += freetype2
}

