include(../config/settings.conf)

TEMPLATE = subdirs
CONFIG	+= ordered
SUBDIRS = others viewimage


contains(DEFINES, _OPENGLEXT_PLUGIN) {
    SUBDIRS += opengl
}

#contains(DEFINES, _NETWORK_PLUGIN) {
#    SUBDIRS += network 
#}
