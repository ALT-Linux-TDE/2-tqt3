TEMPLATE = lib
TARGET	 = qcncodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = ../../../codecs

REQUIRES = !bigcodecs

HEADERS		= ../../../../include/tqgb18030codec.h \
		  ../../../../include/private/tqfontcodecs_p.h

SOURCES		= ../../../../src/codecs/tqgb18030codec.cpp \
		  ../../../../src/codecs/tqfontcncodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target
