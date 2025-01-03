TEMPLATE = lib
TARGET	 = qkrcodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = ../../../codecs

REQUIRES = !bigcodecs

HEADERS		= ../../../../include/tqeuckrcodec.h \
		  ../../../../include/private/tqfontcodecs_p.h

SOURCES		= ../../../../src/codecs/tqeuckrcodec.cpp \
		  ../../../../src/codecs/tqfontkrcodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target

