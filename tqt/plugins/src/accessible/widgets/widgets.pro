TEMPLATE = lib
TARGET  += qtwidgets

CONFIG  += qt dll plugin
DESTDIR  = ../../../accessible
VERSION  = 1.0.0

REQUIRES += accessibility

SOURCES  += main.cpp \
	    tqaccessiblewidget.cpp \
	    tqaccessiblemenu.cpp

HEADERS  += tqaccessiblewidget.h \
	    tqaccessiblemenu.h
