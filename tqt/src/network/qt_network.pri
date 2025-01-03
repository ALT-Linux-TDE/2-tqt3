# TQt network module

network {
	HEADERS += $$NETWORK_H/tqdns.h \
		    $$NETWORK_H/tqftp.h \
		    $$NETWORK_H/tqhttp.h \
		    $$NETWORK_H/tqhostaddress.h \
		    $$NETWORK_H/tqnetwork.h \
		    $$NETWORK_H/tqserversocket.h \
		    $$NETWORK_H/tqsocket.h \
		    $$NETWORK_H/tqsocketdevice.h
	NETWORK_SOURCES	= $$NETWORK_CPP/tqdns.cpp \
		    $$NETWORK_CPP/tqftp.cpp \
		    $$NETWORK_CPP/tqhttp.cpp \
		    $$NETWORK_CPP/tqhostaddress.cpp \
		    $$NETWORK_CPP/tqnetwork.cpp \
		    $$NETWORK_CPP/tqserversocket.cpp \
		    $$NETWORK_CPP/tqsocket.cpp \
		    $$NETWORK_CPP/tqsocketdevice.cpp
	unix:NETWORK_SOURCES += $$NETWORK_CPP/tqsocketdevice_unix.cpp
	win32:NETWORK_SOURCES += $$NETWORK_CPP/tqsocketdevice_win.cpp
	SOURCES    += $$NETWORK_SOURCES
}
