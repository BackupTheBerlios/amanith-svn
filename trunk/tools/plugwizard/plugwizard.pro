include(../../config/useamanith.conf)

TEMPLATE    = app

TARGET      = plugwizard

CONFIG      += qt warn_on

HEADERS     += mainform.h \
               classtreeform.h

SOURCES     += main.cpp \
               mainform.cpp \
               classtreeform.cpp

FORMS        = mainformbase.ui \
               classtreeformbase.ui

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += qt3support
    CONFIG += uic3
}
