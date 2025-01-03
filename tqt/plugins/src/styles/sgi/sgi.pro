TEMPLATE = lib
TARGET	 = tqsgistyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/tqsgistyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/tqsgistyle.cpp

!contains(styles, motif) {
	HEADERS += ../../../../include/tqmotifstyle.h
	SOURCES += ../../../../src/styles/tqmotifstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
