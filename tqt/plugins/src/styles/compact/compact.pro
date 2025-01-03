TEMPLATE = lib
TARGET	 = tqcompactstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/tqcompactstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/tqcompactstyle.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/tqwindowsstyle.h
	SOURCES += ../../../../src/styles/tqwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
