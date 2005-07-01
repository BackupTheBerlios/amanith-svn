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

