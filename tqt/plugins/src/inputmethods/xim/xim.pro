TEMPLATE = lib
TARGET = qxim
DESTDIR  = ../../../inputmethods

INCLUDEPATH += .
CONFIG      += qt warn_on debug plugin
target.path += $$plugins.path/inputmethods
INSTALLS    += target
DEFINES     += QT_NO_XINERAMA

# Input
HEADERS += qximinputcontext.h \
           qximinputcontextplugin.h
SOURCES += qximinputcontext_x11.cpp \
           qximinputcontextplugin.cpp
