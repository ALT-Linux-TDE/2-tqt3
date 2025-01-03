TEMPLATE = lib
TARGET	 = tqwindowsstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/tqwindowsstyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/tqwindowsstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
