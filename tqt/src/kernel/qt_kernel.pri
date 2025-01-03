# TQt kernel module

kernel {
	KERNEL_P	= kernel
	HEADERS += \
		  $$KERNEL_H/tqaccel.h \
		  $$KERNEL_P/tqucomextra_p.h \
		  $$KERNEL_H/tqapplication.h \
		  $$KERNEL_P/tqapplication_p.h \
		  $$KERNEL_H/tqasyncimageio.h \
		  $$KERNEL_H/tqasyncio.h \
		  $$KERNEL_H/tqbitmap.h \
		  $$KERNEL_H/tqbrush.h \
		  $$KERNEL_H/tqclipboard.h \
		  $$KERNEL_H/tqcolor.h \
		  $$KERNEL_P/tqcolor_p.h \
		  $$KERNEL_H/tqconnection.h \
		  $$KERNEL_H/tqcursor.h \
		  $$KERNEL_H/tqdesktopwidget.h \
		  $$KERNEL_H/tqdragobject.h \
		  $$KERNEL_H/tqdrawutil.h \
		  $$KERNEL_H/tqdropsite.h \
		  $$KERNEL_H/tqevent.h \
		  $$KERNEL_H/tqeventloop.h \
		  $$KERNEL_P/tqeventloop_p.h \
		  $$KERNEL_P/tqeventloop_glib_p.h \
		  $$KERNEL_H/tqfocusdata.h \
		  $$KERNEL_H/tqfont.h \
		  $$KERNEL_P/tqfontdata_p.h \
		  $$KERNEL_H/tqfontinfo.h \
		  $$KERNEL_H/tqfontmetrics.h \
		  $$KERNEL_H/tqguardedptr.h \
		  $$KERNEL_H/tqgif.h \
		  $$KERNEL_H/tqiconset.h \
		  $$KERNEL_H/tqimage.h \
		  $$KERNEL_P/tqimageformatinterface_p.h \
		  $$KERNEL_H/tqimageformatplugin.h \
		  $$KERNEL_H/tqkeycode.h \
		  $$KERNEL_H/tqkeysequence.h \
		  $$KERNEL_H/tqlayout.h \
		  $$KERNEL_P/tqlayoutengine_p.h \
		  $$KERNEL_H/tqtranslator.h \
		  $$KERNEL_H/tqmetaobject.h \
		  $$KERNEL_H/tqmime.h \
		  $$KERNEL_H/tqmovie.h \
		  $$KERNEL_H/tqnamespace.h \
		  $$KERNEL_H/tqnetworkprotocol.h \
		  $$KERNEL_H/tqobject.h \
 		  $$KERNEL_H/tqobjectcleanuphandler.h \
		  $$KERNEL_H/tqobjectdefs.h \
		  $$KERNEL_H/tqobjectdict.h \
		  $$KERNEL_H/tqobjectlist.h \
		  $$KERNEL_H/tqpaintdevice.h \
		  $$KERNEL_H/tqpainter.h \
		  $$KERNEL_P/tqpainter_p.h \
		  $$KERNEL_H/tqpalette.h \
		  $$KERNEL_H/tqpaintdevicemetrics.h \
		  $$KERNEL_H/tqpen.h \
		  $$KERNEL_H/tqpicture.h \
		  $$KERNEL_H/tqpixmap.h \
		  $$KERNEL_H/tqpixmapcache.h \
		  $$KERNEL_H/tqpointarray.h \
		  $$KERNEL_H/tqpoint.h \
		  $$KERNEL_H/tqpolygonscanner.h \
		  $$KERNEL_H/tqprinter.h \
		  $$KERNEL_H/tqprocess.h \
		  $$KERNEL_H/tqrect.h \
		  $$KERNEL_H/tqregion.h \
		  $$KERNEL_H/tqsessionmanager.h \
		  $$KERNEL_H/tqsignal.h \
		  $$KERNEL_H/tqsignalmapper.h \
		  $$KERNEL_H/tqsignalslotimp.h \
		  $$KERNEL_H/tqsize.h \
		  $$KERNEL_H/tqsizegrip.h \
		  $$KERNEL_H/tqsizepolicy.h \
		  $$KERNEL_H/tqsocketnotifier.h \
		  $$KERNEL_H/tqsound.h \
		  $$KERNEL_H/tqstyle.h \
		  $$KERNEL_H/tqstylesheet.h \
		  $$KERNEL_H/tqthread.h \
		  $$KERNEL_H/tqtimer.h \
		  $$KERNEL_H/tqurl.h \
		  $$KERNEL_H/tqlocalfs.h \
		  $$KERNEL_H/tqurloperator.h \
		  $$KERNEL_H/tqurlinfo.h \
		  $$KERNEL_H/tqwidget.h \
		  $$KERNEL_H/tqwidgetintdict.h \
		  $$KERNEL_H/tqwidgetlist.h \
		  $$KERNEL_H/tqwindowdefs.h \
		  $$KERNEL_H/tqwmatrix.h \
		  $$KERNEL_H/tqvariant.h \
		  $$KERNEL_P/tqrichtext_p.h \
		  $$KERNEL_P/tqinternal_p.h \
		  $$KERNEL_H/tqgplugin.h \
		  $$KERNEL_H/tqsimplerichtext.h \
		  $$KERNEL_CPP/tqscriptengine_p.h \
		  $$KERNEL_CPP/tqtextengine_p.h \
		  $$KERNEL_CPP/tqfontengine_p.h \
		  $$KERNEL_CPP/tqtextlayout_p.h

	inputmethod {
		unix:x11 {
			HEADERS += $$KERNEL_H/tqinputcontext.h
		} else {
			HEADERS += $$KERNEL_P/tqinputcontext_p.h
		}
	}

		  glibmainloop {
		  		HEADERS+=$$KERNEL_P/tqeventloop_glib_p.h
		  } 
		  
	win32:SOURCES += $$KERNEL_CPP/tqapplication_win.cpp \
		  $$KERNEL_CPP/tqclipboard_win.cpp \
		  $$KERNEL_CPP/tqcolor_win.cpp \
		  $$KERNEL_CPP/tqcursor_win.cpp \
		  $$KERNEL_CPP/tqdesktopwidget_win.cpp \
		  $$KERNEL_CPP/qdnd_win.cpp \
		  $$KERNEL_CPP/tqeventloop_win.cpp \
		  $$KERNEL_CPP/tqfont_win.cpp \
		  $$KERNEL_CPP/tqinputcontext_win.cpp \
		  $$KERNEL_CPP/tqmime_win.cpp \
		  $$KERNEL_CPP/tqpixmap_win.cpp \
		  $$KERNEL_CPP/tqprinter_win.cpp \
		  $$KERNEL_CPP/tqprocess_win.cpp \
		  $$KERNEL_CPP/tqpaintdevice_win.cpp \
		  $$KERNEL_CPP/tqpainter_win.cpp \
		  $$KERNEL_CPP/tqregion_win.cpp \
		  $$KERNEL_CPP/tqsound_win.cpp \
		  $$KERNEL_CPP/tqthread_win.cpp \
		  $$KERNEL_CPP/tqwidget_win.cpp \
		  $$KERNEL_CPP/qole_win.c \
		  $$KERNEL_CPP/tqfontengine_win.cpp

	unix:x11 {
	      SOURCES += $$KERNEL_CPP/tqapplication_x11.cpp \
		          $$KERNEL_CPP/tqclipboard_x11.cpp \
			  $$KERNEL_CPP/tqcolor_x11.cpp \
			  $$KERNEL_CPP/tqcursor_x11.cpp \
			  $$KERNEL_CPP/qdnd_x11.cpp \
			  $$KERNEL_CPP/tqdesktopwidget_x11.cpp \
			  $$KERNEL_CPP/tqfont_x11.cpp \
			  $$KERNEL_CPP/tqinputcontext.cpp \
			  $$KERNEL_CPP/tqinputcontext_x11.cpp \
			  $$KERNEL_CPP/tqmotifdnd_x11.cpp \
			  $$KERNEL_CPP/tqpixmap_x11.cpp \
			  $$KERNEL_CPP/tqpaintdevice_x11.cpp \
			  $$KERNEL_CPP/tqpainter_x11.cpp \
			  $$KERNEL_CPP/tqregion_x11.cpp \
			  $$KERNEL_CPP/tqsound_x11.cpp \
			  $$KERNEL_CPP/tqwidget_x11.cpp \
			  $$KERNEL_CPP/tqwidgetcreate_x11.cpp \
		          $$KERNEL_CPP/tqfontengine_x11.cpp
             glibmainloop {
                  SOURCES += $$KERNEL_CPP/tqeventloop_x11_glib.cpp
             } else {
                  SOURCES += $$KERNEL_CPP/tqeventloop_x11.cpp
             }

	}

	!x11:mac {
	    exists(tqsound_mac.cpp):SOURCES += $$KERNEL_CPP/tqsound_mac.cpp
	}
        !embedded:!x11:mac {
	      SOURCES += $$KERNEL_CPP/tqapplication_mac.cpp \
		          $$KERNEL_CPP/tqclipboard_mac.cpp \
			  $$KERNEL_CPP/tqcolor_mac.cpp \
			  $$KERNEL_CPP/tqcursor_mac.cpp \
			  $$KERNEL_CPP/tqmime_mac.cpp \
			  $$KERNEL_CPP/qdnd_mac.cpp \
			  $$KERNEL_CPP/tqdesktopwidget_mac.cpp \
			  $$KERNEL_CPP/tqpixmap_mac.cpp \
			  $$KERNEL_CPP/tqprinter_mac.cpp \
			  $$KERNEL_CPP/tqpaintdevice_mac.cpp \
			  $$KERNEL_CPP/tqpainter_mac.cpp \
			  $$KERNEL_CPP/tqregion_mac.cpp \
			  $$KERNEL_CPP/tqwidget_mac.cpp \
			  $$KERNEL_CPP/tqeventloop_mac.cpp \
			  $$KERNEL_CPP/tqfont_mac.cpp \
			  $$KERNEL_CPP/tqfontengine_mac.cpp
             DEFINES += QMAC_ONE_PIXEL_LOCK
        } else:unix {
	   SOURCES += $$KERNEL_CPP/tqprinter_unix.cpp \
	              $$KERNEL_CPP/tqpsprinter.cpp
           glibmainloop {
              SOURCES += $$KERNEL_CPP/tqeventloop_unix_glib.cpp
           } else {
	      SOURCES += $$KERNEL_CPP/tqeventloop_unix.cpp
           }

        }
	unix:SOURCES += $$KERNEL_CPP/tqprocess_unix.cpp \
		        $$KERNEL_CPP/tqthread_unix.cpp

	SOURCES += $$KERNEL_CPP/tqabstractlayout.cpp \
		  $$KERNEL_CPP/tqucomextra.cpp \
		  $$KERNEL_CPP/tqaccel.cpp \
		  $$KERNEL_CPP/tqapplication.cpp \
		  $$KERNEL_CPP/tqasyncimageio.cpp \
		  $$KERNEL_CPP/tqasyncio.cpp \
		  $$KERNEL_CPP/tqbitmap.cpp \
		  $$KERNEL_CPP/tqclipboard.cpp \
		  $$KERNEL_CPP/tqcolor.cpp \
		  $$KERNEL_CPP/tqcolor_p.cpp \
		  $$KERNEL_CPP/tqconnection.cpp \
		  $$KERNEL_CPP/tqcursor.cpp \
		  $$KERNEL_CPP/tqdragobject.cpp \
		  $$KERNEL_CPP/tqdrawutil.cpp \
		  $$KERNEL_CPP/tqdropsite.cpp \
		  $$KERNEL_CPP/tqevent.cpp \
		  $$KERNEL_CPP/tqeventloop.cpp \
		  $$KERNEL_CPP/tqfocusdata.cpp \
		  $$KERNEL_CPP/tqfont.cpp \
		  $$KERNEL_CPP/tqfontdatabase.cpp \
		  $$KERNEL_CPP/tqguardedptr.cpp \
		  $$KERNEL_CPP/tqiconset.cpp \
		  $$KERNEL_CPP/tqimage.cpp \
		  $$KERNEL_CPP/tqimageformatplugin.cpp \
		  $$KERNEL_CPP/tqkeysequence.cpp \
		  $$KERNEL_CPP/tqlayout.cpp \
		  $$KERNEL_CPP/tqlayoutengine.cpp \
		  $$KERNEL_CPP/tqtranslator.cpp \
		  $$KERNEL_CPP/tqmetaobject.cpp \
		  $$KERNEL_CPP/tqmime.cpp \
		  $$KERNEL_CPP/tqmovie.cpp \
		  $$KERNEL_CPP/tqnetworkprotocol.cpp \
		  $$KERNEL_CPP/tqobject.cpp \
		  $$KERNEL_CPP/tqobjectcleanuphandler.cpp \
		  $$KERNEL_CPP/tqpainter.cpp \
		  $$KERNEL_CPP/tqpalette.cpp \
		  $$KERNEL_CPP/tqpaintdevicemetrics.cpp \
		  $$KERNEL_CPP/tqpicture.cpp \
		  $$KERNEL_CPP/tqpixmap.cpp \
		  $$KERNEL_CPP/tqpixmapcache.cpp \
		  $$KERNEL_CPP/tqpointarray.cpp \
		  $$KERNEL_CPP/tqpoint.cpp \
		  $$KERNEL_CPP/tqpolygonscanner.cpp \
		  $$KERNEL_CPP/tqprinter.cpp \
		  $$KERNEL_CPP/tqprocess.cpp \
		  $$KERNEL_CPP/tqrect.cpp \
		  $$KERNEL_CPP/tqregion.cpp \
		  $$KERNEL_CPP/tqsignal.cpp \
		  $$KERNEL_CPP/tqsignalmapper.cpp \
		  $$KERNEL_CPP/tqsize.cpp \
		  $$KERNEL_CPP/tqsizegrip.cpp \
		  $$KERNEL_CPP/tqstyle.cpp \
		  $$KERNEL_CPP/tqsocketnotifier.cpp \
		  $$KERNEL_CPP/tqsound.cpp \
		  $$KERNEL_CPP/tqstylesheet.cpp \
		  $$KERNEL_CPP/tqthread.cpp \
		  $$KERNEL_CPP/tqtimer.cpp \
		  $$KERNEL_CPP/tqurl.cpp \
		  $$KERNEL_CPP/tqlocalfs.cpp \
		  $$KERNEL_CPP/tqurloperator.cpp \
		  $$KERNEL_CPP/tqurlinfo.cpp \
		  $$KERNEL_CPP/tqwidget.cpp \
		  $$KERNEL_CPP/tqwmatrix.cpp \
		  $$KERNEL_CPP/tqvariant.cpp \
		  $$KERNEL_CPP/tqrichtext.cpp \
		  $$KERNEL_CPP/tqinternal.cpp \
		  $$KERNEL_CPP/tqrichtext_p.cpp \
		  $$KERNEL_CPP/tqgplugin.cpp \
		  $$KERNEL_CPP/tqsimplerichtext.cpp \
		  $$KERNEL_CPP/tqscriptengine.cpp \
		  $$KERNEL_CPP/tqtextlayout.cpp \
		  $$KERNEL_CPP/tqtextengine.cpp

	unix:HEADERS   += $$KERNEL_P/tqpsprinter_p.h \
			  $$KERNEL_H/tqfontdatabase.h

	accessibility {
	      HEADERS += $$KERNEL_H/tqaccessible.h
	      SOURCES += $$KERNEL_CPP/tqaccessible.cpp

	      !embedded:!x11:mac:SOURCES += $$KERNEL_CPP/tqaccessible_mac.cpp
	      else:win32:SOURCES += $$KERNEL_CPP/tqaccessible_win.cpp
	}
}
