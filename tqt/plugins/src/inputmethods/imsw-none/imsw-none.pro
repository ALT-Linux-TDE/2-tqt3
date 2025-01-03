TEMPLATE = lib
TARGET   = qimsw-none
DESTDIR  = ../../../inputmethods

INCLUDEPATH += .
CONFIG      += qt warn_on debug plugin
target.path += $$plugins.path/inputmethods
INSTALLS    += target

# Input
HEADERS += qnoneinputcontextplugin.h
SOURCES += qnoneinputcontextplugin.cpp
