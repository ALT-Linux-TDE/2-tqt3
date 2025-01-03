# TQt tools module

tools {
	TOOLS_P		= tools
	HEADERS +=  $$TOOLS_H/tqmemarray.h \
		  $$TOOLS_H/tqasciicache.h \
		  $$TOOLS_H/tqasciidict.h \
		  $$TOOLS_H/tqbitarray.h \
		  $$TOOLS_H/tqbuffer.h \
		  $$TOOLS_H/tqcache.h \
		  $$TOOLS_H/tqcleanuphandler.h \
		  $$TOOLS_P/tqcomponentfactory_p.h \
		  $$TOOLS_P/tqcomlibrary_p.h \
		  $$TOOLS_H/tqcstring.h \
		  $$TOOLS_H/tqdatastream.h \
		  $$TOOLS_H/tqdatetime.h \
		  $$TOOLS_H/tqdeepcopy.h \
		  $$TOOLS_H/tqdict.h \
		  $$TOOLS_H/tqdir.h \
		  $$TOOLS_P/tqdir_p.h \
		  $$TOOLS_H/tqfile.h \
		  $$TOOLS_P/tqfiledefs_p.h \
		  $$TOOLS_H/tqfileinfo.h \
		  $$TOOLS_H/tqgarray.h \
		  $$TOOLS_H/tqgcache.h \
		  $$TOOLS_H/tqgdict.h \
		  $$TOOLS_H/tqglist.h \
		  $$TOOLS_H/tqglobal.h \
		  $$TOOLS_P/tqgpluginmanager_p.h \
		  $$TOOLS_H/tqgvector.h \
		  $$TOOLS_H/tqintcache.h \
		  $$TOOLS_H/tqintdict.h \
		  $$TOOLS_H/tqiodevice.h \
		  $$TOOLS_H/tqlibrary.h \
		  $$TOOLS_P/tqlibrary_p.h \
		  $$TOOLS_H/tqlocale.h \
		  $$TOOLS_P/tqlocale_p.h \
		  $$TOOLS_H/tqptrlist.h \
		  $$TOOLS_H/tqmap.h \
		  $$TOOLS_H/tqmutex.h \
		  $$TOOLS_P/tqmutex_p.h \
		  $$TOOLS_P/tqmutexpool_p.h \
		  $$TOOLS_P/tqpluginmanager_p.h \
		  $$TOOLS_H/tqptrcollection.h \
		  $$TOOLS_H/tqptrdict.h \
		  $$TOOLS_H/tqptrqueue.h \
		  $$TOOLS_H/tqregexp.h \
		  $$TOOLS_H/tqsemaphore.h \
		  $$TOOLS_H/tqsettings.h \
		  $$TOOLS_P/tqsettings_p.h \
		  $$TOOLS_H/tqshared.h \
		  $$TOOLS_H/tqptrstack.h \
		  $$TOOLS_H/tqstring.h \
		  $$TOOLS_H/tqstringlist.h \
		  $$TOOLS_H/tqstrlist.h \
		  $$TOOLS_H/tqstrvec.h \
		  $$TOOLS_H/tqtextstream.h \
		  $$TOOLS_P/tqthreadinstance_p.h \
		  $$TOOLS_H/tqthreadstorage.h\
		  $$TOOLS_P/tqunicodetables_p.h \
		  $$TOOLS_H/tqptrvector.h \
	          $$TOOLS_H/tqvaluelist.h \
		  $$TOOLS_H/tqvaluestack.h \
		  $$TOOLS_H/tqvaluevector.h \
		  $$TOOLS_H/tqwaitcondition.h \
		  $$TOOLS_P/tqcom_p.h \
		  $$TOOLS_P/tqucom_p.h \
		  $$TOOLS_H/tquuid.h

	win32:SOURCES += $$TOOLS_CPP/tqdir_win.cpp \
	 	  $$TOOLS_CPP/tqfile_win.cpp \
		  $$TOOLS_CPP/tqfileinfo_win.cpp \
		  $$TOOLS_CPP/tqlibrary_win.cpp \
		  $$TOOLS_CPP/tqsettings_win.cpp \
		  $$TOOLS_CPP/tqmutex_win.cpp \
		  $$TOOLS_CPP/tqwaitcondition_win.cpp \
		  $$TOOLS_CPP/tqthreadstorage_win.cpp \
		  $$TOOLS_CPP/tqcriticalsection_p.cpp

	wince-* {
		SOURCES -= $$TOOLS_CPP/tqdir_win.cpp \
			   $$TOOLS_CPP/tqfile_win.cpp \
			   $$TOOLS_CPP/tqfileinfo_win.cpp
		SOURCES += $$TOOLS_CPP/tqdir_wce.cpp \
			   $$TOOLS_CPP/tqfile_wce.cpp \
			   $$TOOLS_CPP/tqfileinfo_wce.cpp
	}

        offmac:SOURCES += $$TOOLS_CPP/tqdir_mac.cpp \
		  $$TOOLS_CPP/tqfile_mac.cpp \
		  $$TOOLS_CPP/tqfileinfo_mac.cpp
	else:unix:SOURCES += $$TOOLS_CPP/tqdir_unix.cpp \
		  $$TOOLS_CPP/tqfile_unix.cpp \
		  $$TOOLS_CPP/tqfileinfo_unix.cpp \
		  $$TOOLS_CPP/tqmutex_unix.cpp \
		  $$TOOLS_CPP/tqthreadstorage_unix.cpp \
		  $$TOOLS_CPP/tqwaitcondition_unix.cpp

        mac:!x11:!embedded:SOURCES += $$TOOLS_CPP/tqsettings_mac.cpp
	mac {
		SOURCES+=3rdparty/dlcompat/dlfcn.c
		INCLUDEPATH+=3rdparty/dlcompat
	    }
	unix:SOURCES += $$TOOLS_CPP/tqlibrary_unix.cpp

	SOURCES += $$TOOLS_CPP/tqbitarray.cpp \
		  $$TOOLS_CPP/tqbuffer.cpp \
		  $$TOOLS_CPP/tqcomponentfactory.cpp \
		  $$TOOLS_CPP/tqcomlibrary.cpp \
		  $$TOOLS_CPP/tqcstring.cpp \
		  $$TOOLS_CPP/tqdatastream.cpp \
		  $$TOOLS_CPP/tqdatetime.cpp \
		  $$TOOLS_CPP/tqdeepcopy.cpp \
		  $$TOOLS_CPP/tqdir.cpp \
		  $$TOOLS_CPP/tqfile.cpp \
		  $$TOOLS_CPP/tqfileinfo.cpp \
		  $$TOOLS_CPP/tqgarray.cpp \
		  $$TOOLS_CPP/tqgcache.cpp \
		  $$TOOLS_CPP/tqgdict.cpp \
		  $$TOOLS_CPP/tqglist.cpp \
		  $$TOOLS_CPP/tqglobal.cpp \
		  $$TOOLS_CPP/tqgpluginmanager.cpp \
		  $$TOOLS_CPP/tqgvector.cpp \
		  $$TOOLS_CPP/tqiodevice.cpp \
		  $$TOOLS_CPP/tqlibrary.cpp \
		  $$TOOLS_CPP/tqlocale.cpp \
		  $$TOOLS_CPP/tqmap.cpp \
		  $$TOOLS_CPP/tqmutexpool.cpp \
		  $$TOOLS_CPP/tqptrcollection.cpp \
		  $$TOOLS_CPP/tqregexp.cpp \
		  $$TOOLS_CPP/tqstring.cpp \
		  $$TOOLS_CPP/tqsemaphore.cpp \
		  $$TOOLS_CPP/tqsettings.cpp \
		  $$TOOLS_CPP/tqstringlist.cpp \
		  $$TOOLS_CPP/tqtextstream.cpp \
		  $$TOOLS_CPP/tqunicodetables.cpp \
		  $$TOOLS_CPP/tqucom.cpp \
		  $$TOOLS_CPP/tquuid.cpp

        irix-cc* {
                CXXFLAGS_PRELINK = $$QMAKE_CXXFLAGS
                CXXFLAGS_PRELINK -= -O2
                QMAKE_PRE_LINK          = $(CXX) -c $$CXXFLAGS_PRELINK -O1 $(INCPATH) -o $(OBJECTS_DIR)/tqlocale.o tools/tqlocale.cpp
        }
}

