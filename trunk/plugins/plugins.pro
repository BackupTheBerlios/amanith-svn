include(../config/settings.conf)

TEMPLATE	= subdirs
CONFIG	+= ordered

contains(DEFINES, _JPEG_PLUGIN) {
    contains(JPEG_PLUGIN_OPTIONS, EXTERNAL) {
        SUBDIRS += jpeg
    }
}

contains(DEFINES, _PNG_PLUGIN) {
    contains(PNG_PLUGIN_OPTIONS, EXTERNAL) {
        SUBDIRS += png
    }
}

contains(DEFINES, _FONTS_PLUGIN) {
    contains(FONTS_PLUGIN_OPTIONS, EXTERNAL) {
        SUBDIRS += fonts
    }
}

contains(DEFINES, _NETWORK_PLUGIN) {
    contains(NETWORK_PLUGIN_OPTIONS, EXTERNAL) {
        SUBDIRS += network
    }
}
