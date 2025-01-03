TEMPLATE = lib
TARGET	 = tqplatinumstyle

CONFIG  += qt warn_off release plugin
DESTDIR  = ../../../styles

HEADERS		= ../../../../include/tqplatinumstyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/tqplatinumstyle.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/tqwindowsstyle.h
	SOURCES += ../../../../src/styles/tqwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
