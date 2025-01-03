TEMPLATE = lib
TARGET	 = qsqlite3

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/sqlite3/qsql_sqlite3.h
SOURCES		= smain.cpp \
		  ../../../../src/sql/drivers/sqlite3/qsql_sqlite3.cpp

unix {
	OBJECTS_DIR = .obj
	!contains( LIBS, .*sqlite3.* ) {
	    LIBS	*= -lsqlite3
	}
}

win32 {
	OBJECTS_DIR = obj
	LIBS	*= sqlite3.lib
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:sqlite3.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dsqlite3.dll
#	}
}

REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
