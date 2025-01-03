# TQt opengl module

opengl {
	HEADERS += $$OPENGL_H/tqgl.h \
		   $$OPENGL_H/tqglcolormap.h
	SOURCES	+= $$OPENGL_CPP/tqgl.cpp \
		   $$OPENGL_CPP/tqglcolormap.cpp
	x11 {
		HEADERS += $$OPENGL_H/tqgl_x11_p.h
		SOURCES += $$OPENGL_CPP/tqgl_x11.cpp
	}
	else:mac:SOURCES += $$OPENGL_CPP/qgl_mac.cpp
	else:win32:SOURCES += $$OPENGL_CPP/qgl_win.cpp

	dlopen_opengl:DEFINES+=QT_DLOPEN_OPENGL
}


