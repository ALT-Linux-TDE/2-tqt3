TEMPLATE = lib
TARGET	 = qsqlpsql

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/psql/tqsql_psql.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/psql/tqsql_psql.cpp
unix {
	OBJECTS_DIR	= .obj
	!contains( LIBS, .*pq.* ) {
	    LIBS	*= -lpq
	}
}

win32 {
	OBJECTS_DIR	= obj
	LIBS	*= libpqdll.lib
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libpq.dll
#	}
}

REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
