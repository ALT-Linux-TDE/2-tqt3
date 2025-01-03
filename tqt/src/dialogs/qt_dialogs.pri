# TQt dialogs module

dialogs {
	DIALOGS_P	= dialogs

	HEADERS	+= $$DIALOGS_H/tqcolordialog.h \
		  $$DIALOGS_H/tqdialog.h \
		  $$DIALOGS_H/tqerrormessage.h \
		  $$DIALOGS_H/tqfiledialog.h \
		  $$DIALOGS_H/tqfontdialog.h \
		  $$DIALOGS_H/tqmessagebox.h \
		  $$DIALOGS_H/tqprogressdialog.h \
		  $$DIALOGS_H/tqtabdialog.h \
		  $$DIALOGS_H/tqwizard.h \
		  $$DIALOGS_H/tqinputdialog.h

        !embedded:mac:SOURCES  += $$DIALOGS_CPP/tqfiledialog_mac.cpp $$DIALOGS_CPP/tqcolordialog_mac.cpp
	win32:SOURCES += $$DIALOGS_CPP/tqfiledialog_win.cpp
	unix:SOURCES += $$DIALOGS_CPP/tqprintdialog.cpp
	unix:HEADERS   += $$DIALOGS_H/tqprintdialog.h 

	SOURCES += $$DIALOGS_CPP/tqcolordialog.cpp \
		  $$DIALOGS_CPP/tqdialog.cpp \
		  $$DIALOGS_CPP/tqerrormessage.cpp \
		  $$DIALOGS_CPP/tqfiledialog.cpp \
		  $$DIALOGS_CPP/tqfontdialog.cpp \
		  $$DIALOGS_CPP/tqmessagebox.cpp \
		  $$DIALOGS_CPP/tqprogressdialog.cpp \
		  $$DIALOGS_CPP/tqtabdialog.cpp \
		  $$DIALOGS_CPP/tqwizard.cpp \
		  $$DIALOGS_CPP/tqinputdialog.cpp
}
