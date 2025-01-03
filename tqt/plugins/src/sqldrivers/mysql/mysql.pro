TEMPLATE = lib
TARGET	 = qsqlmysql

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/mysql/tqsql_mysql.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/mysql/tqsql_mysql.cpp

unix {
	OBJECTS_DIR = .obj

	!contains( LIBS, .*mysql.* ) {
	    LIBS    *= -lmysqlclient
	}
}
win32 {
	OBJECTS_DIR = obj
	LIBS	*= libmysql.lib
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libmysql.dll
#	}
}

REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
