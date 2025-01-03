# TQt codecs module

!bigcodecs:DEFINES += TQT_NO_BIG_CODECS
tools {
	CODECS_P		= codecs
	HEADERS += \
#$$CODECS_H/qasmocodec.h \
		$$CODECS_H/tqbig5codec.h \
		  $$CODECS_H/tqeucjpcodec.h \
		  $$CODECS_H/tqeuckrcodec.h \
		  $$CODECS_P/tqisciicodec_p.h \
		  $$CODECS_H/tqgb18030codec.h \
		  $$CODECS_H/tqjiscodec.h \
		  $$CODECS_H/tqjpunicode.h \
		  $$CODECS_H/tqhebrewcodec.h \
		  $$CODECS_H/tqsjiscodec.h \
		  $$CODECS_H/tqtextcodec.h \
		  $$CODECS_H/tqtsciicodec.h \
		  $$CODECS_H/tqutfcodec.h \
		  $$CODECS_P/tqtextcodecinterface_p.h \
		  $$CODECS_H/tqtextcodecfactory.h \
		  $$CODECS_H/tqtextcodecplugin.h

	SOURCES += \
#$$CODECS_CPP/qasmocodec.cpp \
		$$CODECS_CPP/tqbig5codec.cpp \
		  $$CODECS_CPP/tqeucjpcodec.cpp \
		  $$CODECS_CPP/tqeuckrcodec.cpp \
		  $$CODECS_CPP/tqisciicodec.cpp \
		  $$CODECS_CPP/tqgb18030codec.cpp \
		  $$CODECS_CPP/tqhebrewcodec.cpp \
		  $$CODECS_CPP/tqjiscodec.cpp \
		  $$CODECS_CPP/tqjpunicode.cpp \
		  $$CODECS_CPP/tqsjiscodec.cpp \
		  $$CODECS_CPP/tqtextcodec.cpp \
		  $$CODECS_CPP/tqtsciicodec.cpp \
		  $$CODECS_CPP/tqutfcodec.cpp \
		  $$CODECS_CPP/tqtextcodecfactory.cpp \
		  $$CODECS_CPP/tqtextcodecplugin.cpp

	x11:SOURCES += $$CODECS_CPP/tqfontcncodec.cpp  \
		  $$CODECS_CPP/tqfonthkcodec.cpp  \
		  $$CODECS_CPP/tqfontjpcodec.cpp  \
		  $$CODECS_CPP/tqfontkrcodec.cpp  \
		  $$CODECS_CPP/tqfontlaocodec.cpp \
		  $$CODECS_CPP/tqfonttwcodec.cpp
	x11:HEADERS += $$CODECS_P/tqfontcodecs_p.h
}
