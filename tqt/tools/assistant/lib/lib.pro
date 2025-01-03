TEMPLATE        = lib
TARGET		= tqassistantclient
VERSION		= 1.0

CONFIG		+= qt warn_on release
CONFIG		+= dll

SOURCES		= tqassistantclient.cpp
HEADERS         += $$QT_SOURCE_TREE/include/tqassistantclient.h


DEFINES		+= QT_INTERNAL_NETWORK
include( ../../../src/qt_professional.pri )

DESTDIR		= ../../../lib

unix {
	target.path=$$libs.path
	QMAKE_CFLAGS += $$QMAKE_CFLAGS_SHLIB
	QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_SHLIB
	INSTALLS        += target
}
