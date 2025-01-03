TEMPLATE	= app
TARGET		= tqdir

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config

HEADERS		= tqdir.h ../dirview/dirview.h
SOURCES		= tqdir.cpp ../dirview/dirview.cpp
