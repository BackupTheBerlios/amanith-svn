include(../../../config/useamanith.conf)

TEMPLATE = app
TARGET = drawboard
LANGUAGE = C++

CONFIG += qt warn_on opengl

SOURCES = main.cpp \
          drawboard.cpp \
          test_color.cpp \
          test_lineargradient.cpp \
          test_radialgradientin.cpp \
          test_radialgradientout.cpp \
          test_conicalgradientin.cpp \
          test_conicalgradientout.cpp \
          test_pattern.cpp \
          test_stroking.cpp \
          test_masks.cpp

win32: RC_FILE = example.rc

# Qt4 support
contains(DEFINES, USE_QT4) {
    QT += opengl qt3support
}
