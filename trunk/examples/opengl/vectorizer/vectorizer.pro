include(../../../config/useamanith.conf)

TEMPLATE       = app
TARGET         = vectorizer
LANGUAGE       = C++

CONFIG        += qt warn_on opengl

HEADERS        = drawer.h
SOURCES        = main.cpp \
                 drawer.cpp

