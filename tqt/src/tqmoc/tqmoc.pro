TEMPLATE	= app
TARGET		= tqmoc

CONFIG 		= console release qtinc yacc lex_included yacc_no_name_mangle
DEFINES	       += QT_MOC TQT_NO_CODECS QT_LITE_UNICODE TQT_NO_COMPONENT \
		  TQT_NO_STL TQT_NO_COMPRESS
win32:DEFINES  += QT_NODLL
DESTDIR         = ../../bin


INCLUDEPATH	+= $$QT_SOURCE_TREE/include ../tools .
DEPENDPATH	+= $$QT_SOURCE_TREE/include ../tools .
LIBS		=
OBJECTS_DIR	= .
SOURCES		= ../tools/tqbuffer.cpp	    \
		  ../tools/tqptrcollection.cpp  \
		  ../tools/tqcstring.cpp	    \
		  ../tools/tqdatastream.cpp  \
		  ../tools/tqdatetime.cpp    \
		  ../tools/tqfile.cpp	    \
		  ../tools/tqdir.cpp	    \
		  ../tools/tqfileinfo.cpp    \
		  ../tools/tqgarray.cpp	    \
		  ../tools/tqgdict.cpp	    \
		  ../tools/tqglist.cpp	    \
		  ../tools/tqglobal.cpp	    \
		  ../tools/tqgvector.cpp	    \
		  ../tools/tqiodevice.cpp    \
		  ../tools/tqregexp.cpp	    \
		  ../tools/tqstring.cpp	    \
		  ../tools/tqlocale.cpp	    \
                  ../tools/tqunicodetables.cpp \
		  ../tools/tqstringlist.cpp  \
		  ../tools/tqtextstream.cpp  \
		  ../tools/tqbitarray.cpp    \
		  ../tools/tqmap.cpp         \
		  ../tools/tqgcache.cpp      \
		  ../codecs/tqtextcodec.cpp \
		  ../codecs/tqutfcodec.cpp

isEmpty(QT_PRODUCT)|contains(QT_PRODUCT, qt-internal) {
    LEXSOURCES  = tqmoc.l
    YACCSOURCES = tqmoc.y
} else {
    SOURCES   += tqmoc_yacc.cpp
}

unix:SOURCES	+= ../tools/tqfile_unix.cpp ../tools/tqdir_unix.cpp ../tools/tqfileinfo_unix.cpp
win32:SOURCES	+= ../tools/tqfile_win.cpp ../tools/tqdir_win.cpp ../tools/tqfileinfo_win.cpp
macx:LIBS	+= -framework Carbon

target.path=$$bins.path
INSTALLS += target
