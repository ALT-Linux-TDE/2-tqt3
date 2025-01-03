TEMPLATE	= app
CONFIG		+= qt console warn_on release professional
HEADERS	= tquic.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../shared/parser.h \
		  ../interfaces/widgetinterface.h

#HEADERS +=	  ../shared/ui2uib.h \
#		  ../shared/uib.h

SOURCES	= main.cpp tquic.cpp form.cpp object.cpp \
		   subclassing.cpp embed.cpp\
		  ../shared/widgetdatabase.cpp  \
		  ../shared/domtool.cpp \
		  ../shared/parser.cpp

#SOURCES +=	  ../shared/ui2uib.cpp \
#		  ../shared/uib.cpp

DEFINES		+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

TARGET		= tquic
INCLUDEPATH	+= ../shared
DEFINES 	+= TQUIC
DESTDIR		= ../../../bin

target.path=$$bins.path
INSTALLS        += target
