include(../../../config/useamanith.conf)

TEMPLATE       = app
TARGET         = delaunay2d
LANGUAGE       = C++

CONFIG        += qt warn_on opengl

SOURCES        = main.cpp \
                 drawmesh.cpp

