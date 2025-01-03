TEMPLATE = lib
TARGET	 = tqmotifstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/tqmotifstyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/tqmotifstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
