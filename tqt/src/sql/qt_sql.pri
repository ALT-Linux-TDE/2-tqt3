# TQt sql module

sql {

	!table {
		message(table must be enabled for sql support)
		REQUIRES += table
	}
	
	SQL_P	    = sql
	HEADERS     += $$SQL_H/tqsql.h \
		    $$SQL_H/tqsqlquery.h \
		    $$SQL_H/tqsqldatabase.h \
		    $$SQL_H/tqsqlfield.h \
		    $$SQL_H/tqsqlrecord.h \
		    $$SQL_H/tqsqlcursor.h \
		    $$SQL_H/tqsqlform.h \
		    $$SQL_H/tqeditorfactory.h \
		    $$SQL_H/tqsqleditorfactory.h \
		    $$SQL_H/tqsqldriver.h \
		    $$SQL_P/tqsqldriverinterface_p.h \
		    $$SQL_P/tqsqlextension_p.h \
		    $$SQL_H/tqsqldriverplugin.h \
		    $$SQL_H/tqsqlerror.h \
		    $$SQL_H/tqsqlresult.h \
		    $$SQL_H/tqsqlindex.h \
		    $$SQL_H/tqsqlpropertymap.h \
		    $$SQL_P/tqsqlmanager_p.h \
		    $$SQL_H/tqdatatable.h \
		    $$SQL_H/tqdataview.h \
		    $$SQL_H/tqdatabrowser.h \
		    $$SQL_H/tqsqlselectcursor.h 

	SOURCES     += $$SQL_CPP/tqsqlquery.cpp \
		    $$SQL_CPP/tqsqldatabase.cpp \
		    $$SQL_CPP/tqsqlfield.cpp \
		    $$SQL_CPP/tqsqlrecord.cpp \
		    $$SQL_CPP/tqsqlform.cpp \
		    $$SQL_CPP/tqsqlcursor.cpp \
		    $$SQL_CPP/tqeditorfactory.cpp \
		    $$SQL_CPP/tqsqleditorfactory.cpp \
		    $$SQL_CPP/tqsqldriver.cpp \
		    $$SQL_CPP/tqsqlextension_p.cpp \
		    $$SQL_CPP/tqsqldriverplugin.cpp \
		    $$SQL_CPP/tqsqlerror.cpp \
		    $$SQL_CPP/tqsqlresult.cpp \
		    $$SQL_CPP/tqsqlindex.cpp \
		    $$SQL_CPP/tqsqlpropertymap.cpp \
		    $$SQL_CPP/tqsqlmanager_p.cpp \
		    $$SQL_CPP/tqdatatable.cpp \
		    $$SQL_CPP/tqdataview.cpp \
		    $$SQL_CPP/tqdatabrowser.cpp \
		    $$SQL_CPP/tqsqlselectcursor.cpp \
		    $$SQL_CPP/drivers/cache/tqsqlcachedresult.cpp

	contains(sql-drivers, all ) {
		sql-driver += psql mysql odbc oci tds db2 sqlite ibase
	}			

	contains(sql-drivers, psql) {
		HEADERS += $$SQL_CPP/drivers/psql/tqsql_psql.h
		SOURCES += $$SQL_CPP/drivers/psql/tqsql_psql.cpp
		DEFINES += QT_SQL_POSTGRES
		unix {
			!contains( LIBS, .*pq.* ) {
				LIBS *= -lpq
			}
		}
		win32 {
			!contains( LIBS, .*libpq.* ) {
				LIBS *= libpqdll.lib
			}
#			win32-msvc: { 
#				LIBS *= delayimp.lib
#				QMAKE_LFLAGS += /DELAYLOAD:libpqdll.dll
#			}
		}
	}

	contains(sql-drivers, mysql) {
		HEADERS += $$SQL_CPP/drivers/mysql/tqsql_mysql.h
		SOURCES += $$SQL_CPP/drivers/mysql/tqsql_mysql.cpp
		DEFINES += QT_SQL_MYSQL
		unix {
			!contains( LIBS, .*mysql.* ) {
				LIBS    *= -lmysqlclient
			}
		}
		win32 {
			!contains( LIBS, .*mysql.* ) {
				LIBS    *= libmysql.lib
			}
#			win32-msvc: { 
#				LIBS *= delayimp.lib
#				QMAKE_LFLAGS += /DELAYLOAD:libmysql.dll
#			}
		}
	}
	
	contains(sql-drivers, odbc) {
		HEADERS += $$SQL_CPP/drivers/odbc/tqsql_odbc.h
		SOURCES += $$SQL_CPP/drivers/odbc/tqsql_odbc.cpp
		DEFINES += QT_SQL_ODBC

		mac {
			!contains( LIBS, .*odbc.* ) {
				LIBS        *= -lodbc
			}
		}

		unix {
			!contains( LIBS, .*odbc.* ) {
				LIBS        *= -lodbc
			}
		}

		win32 {
			LIBS     *= odbc32.lib
		}

	}

	contains(sql-drivers, oci) {
		HEADERS += $$SQL_CPP/drivers/oci/tqsql_oci.h
		SOURCES += $$SQL_CPP/drivers/oci/tqsql_oci.cpp
		DEFINES += QT_SQL_OCI
		unix {
			!contains( LIBS, .*clnts.* ) {
			    LIBS += -lclntsh -lwtc8
			}
		}
		win32 {
			LIBS += oci.lib
#			win32-msvc: { 
#				LIBS *= delayimp.lib
#				QMAKE_LFLAGS += /DELAYLOAD:oci.dll
#			}
		}
	}

	contains(sql-drivers, tds) {
		HEADERS += $$SQL_CPP/drivers/tds/tqsql_tds.h \
			   $$SQL_CPP/drivers/shared/tqsql_result.h
		SOURCES += $$SQL_CPP/drivers/tds/tqsql_tds.cpp \
			   $$SQL_CPP/drivers/shared/tqsql_result.cpp
		DEFINES += QT_SQL_TDS
		unix {
			LIBS += -L$SYBASE/lib -lsybdb
		}
		win32 {
			LIBS += NTWDBLIB.LIB
#			win32-msvc: { 
#				LIBS *= delayimp.lib
#				QMAKE_LFLAGS += /DELAYLOAD:ntwdblib.dll
#			}
		}
	}

	contains(sql-drivers, db2) {
		HEADERS += $$SQL_CPP/drivers/db2/tqsql_db2.h
		SOURCES += $$SQL_CPP/drivers/db2/tqsql_db2.cpp
		DEFINES += QT_SQL_DB2
		unix {
			LIBS += -ldb2
		}
		win32 {
			LIBS += db2cli.lib
		}
	}

	contains(sql-drivers, ibase) {
                HEADERS += $$SQL_CPP/drivers/ibase/tqsql_ibase.h
                SOURCES += $$SQL_CPP/drivers/ibase/tqsql_ibase.cpp
                DEFINES += QT_SQL_IBASE
                unix {
                        LIBS *= -lfbclient
                }
                win32 {
			LIBS *= gds32_ms.lib
                }
	}

        contains(sql-drivers, sqlite) {
	    !contains( LIBS, .*sqlite.* ) {

                INCLUDEPATH += $$SQL_CPP/../3rdparty/sqlite/

                HEADERS += $$SQL_CPP/../3rdparty/sqlite/btree.h \
                $$SQL_CPP/../3rdparty/sqlite/config.h \
                $$SQL_CPP/../3rdparty/sqlite/hash.h \
                $$SQL_CPP/../3rdparty/sqlite/opcodes.h \
                $$SQL_CPP/../3rdparty/sqlite/os.h \
                $$SQL_CPP/../3rdparty/sqlite/pager.h \
                $$SQL_CPP/../3rdparty/sqlite/parse.h \
                $$SQL_CPP/../3rdparty/sqlite/sqlite.h \
                $$SQL_CPP/../3rdparty/sqlite/sqliteInt.h \
                $$SQL_CPP/../3rdparty/sqlite/vdbe.h \
		$$SQL_CPP/../3rdparty/sqlite/vdbeInt.h

                SOURCES += $$SQL_CPP/../3rdparty/sqlite/attach.c \
                $$SQL_CPP/../3rdparty/sqlite/auth.c \
                $$SQL_CPP/../3rdparty/sqlite/btree.c \
                $$SQL_CPP/../3rdparty/sqlite/btree_rb.c \
                $$SQL_CPP/../3rdparty/sqlite/build.c \
                $$SQL_CPP/../3rdparty/sqlite/copy.c \
		$$SQL_CPP/../3rdparty/sqlite/date.c \
                $$SQL_CPP/../3rdparty/sqlite/delete.c \
                $$SQL_CPP/../3rdparty/sqlite/expr.c \
                $$SQL_CPP/../3rdparty/sqlite/func.c \
                $$SQL_CPP/../3rdparty/sqlite/hash.c \
                $$SQL_CPP/../3rdparty/sqlite/insert.c \
                $$SQL_CPP/../3rdparty/sqlite/main.c \
                $$SQL_CPP/../3rdparty/sqlite/opcodes.c \
                $$SQL_CPP/../3rdparty/sqlite/os.c \
                $$SQL_CPP/../3rdparty/sqlite/pager.c \
                $$SQL_CPP/../3rdparty/sqlite/parse.c \
                $$SQL_CPP/../3rdparty/sqlite/pragma.c \
                $$SQL_CPP/../3rdparty/sqlite/printf.c \
                $$SQL_CPP/../3rdparty/sqlite/random.c \
                $$SQL_CPP/../3rdparty/sqlite/select.c \
                $$SQL_CPP/../3rdparty/sqlite/shell.c \
                $$SQL_CPP/../3rdparty/sqlite/table.c \
                $$SQL_CPP/../3rdparty/sqlite/tokenize.c \
                $$SQL_CPP/../3rdparty/sqlite/trigger.c \
                $$SQL_CPP/../3rdparty/sqlite/update.c \
                $$SQL_CPP/../3rdparty/sqlite/util.c \
                $$SQL_CPP/../3rdparty/sqlite/vacuum.c \
                $$SQL_CPP/../3rdparty/sqlite/vdbe.c \
		$$SQL_CPP/../3rdparty/sqlite/vdbeaux.c \
                $$SQL_CPP/../3rdparty/sqlite/where.c
	    }

            HEADERS += $$SQL_CPP/drivers/sqlite/tqsql_sqlite.h
            SOURCES += $$SQL_CPP/drivers/sqlite/tqsql_sqlite.cpp
            DEFINES += QT_SQL_SQLITE
        }

        contains(sql-drivers, sqlite3) {
            HEADERS += $$SQL_CPP/drivers/sqlite3/tqsql_sqlite3.h
            SOURCES += $$SQL_CPP/drivers/sqlite3/tqsql_sqlite3.cpp
            DEFINES += QT_SQL_SQLITE3
            unix {
                !contains( LIBS, .*sqlite3.* ) {
                    LIBS *= -lsqlite3
                }
            }
        }
}

