# TQt styles module

styles {
	STYLES_P = styles
	HEADERS +=$$STYLES_H/tqstylefactory.h \
		  $$STYLES_P/tqstyleinterface_p.h \
		  $$STYLES_H/tqstyleplugin.h \
		  $$STYLES_H/tqcommonstyle.h
	SOURCES +=$$STYLES_CPP/tqstylefactory.cpp \
		  $$STYLES_CPP/tqstyleplugin.cpp \
		  $$STYLES_CPP/tqcommonstyle.cpp

	contains( styles, all ) {
		styles += mac cde motifplus sgi platinum compact interlace windows motif
	}

	x11|embedded|!macx-*:styles -= mac
	contains( styles, mac ) {
		HEADERS +=$$STYLES_H/tqmacstyle_mac.h \
                          $$STYLES_H/tqmacstylepixmaps_mac_p.h
		SOURCES +=$$STYLES_CPP/tqmacstyle_mac.cpp
		HEADERS *= $$STYLES_CPP/qaquastyle_p.h 
		SOURCES *= $$STYLES_CPP/qaquastyle_p.cpp 

		!contains( styles, windows ) {
			message( mac requires windows )
			styles += windows
		}
        }
	else:DEFINES += TQT_NO_STYLE_MAC

	#embedded|!macx-*:styles -= aqua
	contains( styles, aqua ) {
		HEADERS += $$STYLES_H/qaquastyle.h 
		SOURCES += $$STYLES_CPP/qaquastyle.cpp 
		HEADERS *= $$STYLES_CPP/qaquastyle_p.h 
		SOURCES *= $$STYLES_CPP/qaquastyle_p.cpp 

		!contains( styles, windows ) {
			message( aqua requires windows )
			styles += windows
		}
	}
	else:DEFINES += TQT_NO_STYLE_AQUA

	contains( styles, cde ) {
		HEADERS +=$$STYLES_H/tqcdestyle.h
		SOURCES +=$$STYLES_CPP/tqcdestyle.cpp

		!contains( styles, motif ) {
			message( cde requires motif )
			styles += motif
		}
	}
	else:DEFINES += TQT_NO_STYLE_CDE

	contains( styles, motifplus ) {
		HEADERS +=$$STYLES_H/tqmotifplusstyle.h
		SOURCES +=$$STYLES_CPP/tqmotifplusstyle.cpp
		!contains( styles, motif ) {
			message( motifplus requires motif )
			styles += motif
		}
	}
	else:DEFINES += TQT_NO_STYLE_MOTIFPLUS

	contains( styles, interlace ) {
		HEADERS +=$$STYLES_H/tqinterlacestyle.h
		SOURCES +=$$STYLES_CPP/tqinterlacestyle.cpp
		!contains( styles, windows ) {
			message( interlace requires windows )
			styles += windows
		}
	}
	else:DEFINES += TQT_NO_STYLE_INTERLACE

	contains( styles, platinum ) {
		HEADERS +=$$STYLES_H/tqplatinumstyle.h
		SOURCES +=$$STYLES_CPP/tqplatinumstyle.cpp
		!contains( styles, windows ) {
			message( platinum requires windows )
			styles += windows
		}
	}
	else:DEFINES += TQT_NO_STYLE_PLATINUM

	contains( styles, windowsxp ) {
		HEADERS +=$$STYLES_H/tqwindowsxpstyle.h
		SOURCES +=$$STYLES_CPP/tqwindowsxpstyle.cpp
		!contains( styles, windowsxp ) {
			message( windowsxp requires windows )
			styles += windows
		}
        }
        else:DEFINES += TQT_NO_STYLE_WINDOWSXP

	contains( styles, sgi ) {
		HEADERS +=$$STYLES_H/tqsgistyle.h
		SOURCES +=$$STYLES_CPP/tqsgistyle.cpp
		!contains( styles, motif ) {
			message( sgi requires motif )
			styles += motif
		}
	}
	else:DEFINES += TQT_NO_STYLE_SGI

	contains( styles, compact ) {
		HEADERS +=$$STYLES_H/tqcompactstyle.h
		SOURCES +=$$STYLES_CPP/tqcompactstyle.cpp
		!contains( styles, windows ) {
			message( compact requires windows )
			styles += windows
		}
	}
	else:DEFINES += TQT_NO_STYLE_COMPACT

        wince-*:styles += pocketpc
	contains( styles, pocketpc ) {
		HEADERS +=$$STYLES_H/qpocketpcstyle_wce.h
		SOURCES +=$$STYLES_CPP/qpocketpcstyle_wce.cpp
		
		!contains( styles, windows ) {
			message( pocketpc requires windows )
			styles += windows
		}
	}
        else:DEFINES += TQT_NO_STYLE_POCKETPC
        
	contains( styles, windows ) {
		HEADERS +=$$STYLES_H/tqwindowsstyle.h
		SOURCES +=$$STYLES_CPP/tqwindowsstyle.cpp
	}
	else:DEFINES += TQT_NO_STYLE_WINDOWS

	contains( styles, motif ) {
		HEADERS +=$$STYLES_H/tqmotifstyle.h
		SOURCES +=$$STYLES_CPP/tqmotifstyle.cpp
	}
	else:DEFINES += TQT_NO_STYLE_MOTIF
}
