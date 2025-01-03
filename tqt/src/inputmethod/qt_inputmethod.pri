# TQt inputmetod module

inputmethod {
	INPUTMETHOD_P = inputmethod
	HEADERS +=$$INPUTMETHOD_H/tqinputcontextfactory.h \
		  $$INPUTMETHOD_P/tqinputcontextinterface_p.h \
		  $$INPUTMETHOD_H/tqinputcontextplugin.h
	SOURCES +=$$INPUTMETHOD_CPP/tqinputcontextfactory.cpp \
		  $$INPUTMETHOD_CPP/tqinputcontextplugin.cpp
}
