TEMPLATE	= app
TARGET		= fileiconview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES        = iconview full-config

HEADERS		= mainwindow.h \
		  tqfileiconview.h \
		  ../dirview/dirview.h
SOURCES		= main.cpp \
		  mainwindow.cpp \
		  tqfileiconview.cpp \
		  ../dirview/dirview.cpp
