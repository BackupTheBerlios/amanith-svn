include(../../../config/useamanith.conf)

TEMPLATE       = app
TARGET         = testshaders
LANGUAGE       = C++

CONFIG        += qt warn_on opengl

SOURCES        = main.cpp \
                 testshaders.cpp
