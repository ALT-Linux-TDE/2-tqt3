TEMPLATE = lib
TARGET	 = tqjpcodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = ../../../codecs

REQUIRES	= !bigcodecs

HEADERS		= ../../../../include/tqeucjpcodec.h \
		  ../../../../include/tqjiscodec.h \
		  ../../../../include/tqsjiscodec.h \
		  ../../../../include/tqjpunicode.h \
		  ../../../../include/private/tqfontcodecs_p.h

SOURCES		= ../../../../src/codecs/tqeucjpcodec.cpp \
		  ../../../../src/codecs/tqjiscodec.cpp \
		  ../../../../src/codecs/tqsjiscodec.cpp \
		  ../../../../src/codecs/tqjpunicode.cpp \
		  ../../../../src/codecs/tqfontjpcodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target

