include(../config/settings.conf)

TEMPLATE = subdirs
CONFIG	+= ordered
SUBDIRS = others


contains(DEFINES, _OPENGLEXT_PLUGIN) {
    SUBDIRS += opengl
}
