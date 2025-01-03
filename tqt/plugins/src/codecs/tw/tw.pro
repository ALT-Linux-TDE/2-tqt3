TEMPLATE = lib
TARGET   = qtwcodecs

CONFIG  += qt warn_on plugin
DESTDIR  = ../../../codecs

REQUIRES = !bigcodecs

HEADERS  = ../../../../include/tqbig5codec.h \
	   ../../../../include/private/tqfontcodecs_p.h
SOURCES  = ../../../../src/codecs/tqbig5codec.cpp \
	   ../../../../src/codecs/tqfonttwcodec.cpp \
	   main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target

