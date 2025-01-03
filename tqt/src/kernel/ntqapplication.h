/****************************************************************************
**
** Definition of TQApplication class
**
** Created : 931107
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the TQt GUI Toolkit.
**
** This file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the files LICENSE.GPL2
** and LICENSE.GPL3 included in the packaging of this file.
** Alternatively you may (at your option) use any later version
** of the GNU General Public License if such license has been
** publicly approved by Trolltech ASA (or its successors, if any)
** and the KDE Free TQt Foundation.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/.
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** This file may be used under the terms of the Q Public License as
** defined by Trolltech ASA and appearing in the file LICENSE.TQPL
** included in the packaging of this file.  Licensees holding valid TQt
** Commercial licenses may use this file in accordance with the TQt
** Commercial License Agreement provided with the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#ifndef TQAPPLICATION_H
#define TQAPPLICATION_H

#ifndef QT_H
#include "ntqdesktopwidget.h"
#include "ntqasciidict.h"
#include "ntqpalette.h"
#include "ntqtranslator.h"
#include "ntqstrlist.h"
#include "ntqstringlist.h"
#endif // QT_H

class TQSessionManager;
class TQStyle;
class TQTranslator;
class TQEventLoop;
#if defined(TQ_WS_X11)
class TQIMEvent;
#endif
#if defined(TQ_WS_QWS)
class TQWSDecoration;
#endif

#ifdef TQT_THREAD_SUPPORT
class TQMutex;
class TQThread;
#endif // TQT_THREAD_SUPPORT


class TQApplication;
extern TQ_EXPORT TQApplication *tqApp;		// global application object


class TQ_EXPORT TQApplication : public TQObject
{
    TQ_OBJECT
public:
    TQApplication( int &argc, char **argv );
    TQApplication( int &argc, char **argv, bool GUIenabled );
    TQApplication( int &argc, char **argv, bool GUIenabled, bool SMenabled );
    enum Type { Tty, GuiClient, GuiServer };
    TQApplication( int &argc, char **argv, Type );
#if defined(TQ_WS_X11)
    TQApplication( Display* dpy, HANDLE visual = 0, HANDLE cmap = 0 );
    TQApplication( Display *dpy, int argc, char **argv,
		  HANDLE visual = 0, HANDLE cmap= 0 );
#endif
    virtual ~TQApplication();

    int		    argc()	const;
    char	  **argv()	const;

    Type type() const;

#ifndef TQT_NO_STYLE
    static TQStyle  &style();
    static void	    setStyle( TQStyle* );
    static TQStyle*  setStyle( const TQString& );
#endif
#ifndef Q_QDOC
    enum ColorMode { NormalColors, CustomColors };
    static ColorMode colorMode();
    static void      setColorMode( TQApplication::ColorMode );
#endif

    enum ColorSpec { NormalColor=0, CustomColor=1, ManyColor=2 };
    static int	     colorSpec();
    static void      setColorSpec( int );
#ifndef TQT_NO_CURSOR
    static TQCursor  *overrideCursor();
    static void	     setOverrideCursor( const TQCursor &, bool replace=FALSE );
    static void	     restoreOverrideCursor();
#endif
    static bool	     hasGlobalMouseTracking();
    static void	     setGlobalMouseTracking( bool enable );
#ifndef TQT_NO_PALETTE
    static TQPalette  palette( const TQWidget* = 0 );
    static TQPalette  palette( TQStringList );
    static void	     setPalette( const TQPalette &, bool informWidgets=FALSE,
				 const char* className = 0 );
#endif
    static TQFont     font( const TQWidget* = 0 );
    static void	     setFont( const TQFont &, bool informWidgets=FALSE,
			      const char* className = 0 );
    static TQFontMetrics fontMetrics();

    TQWidget	    *mainWidget()  const;
    virtual void     setMainWidget( TQWidget * );
    virtual void     polish( TQWidget * );

    static TQWidgetList *allWidgets();
    static TQWidgetList *topLevelWidgets();

    static TQDesktopWidget   *desktop();

    static TQWidget     *activePopupWidget();
    static TQWidget     *activeModalWidget();
#ifndef TQT_NO_CLIPBOARD
    static TQClipboard  *clipboard();
#endif
    TQWidget	       *focusWidget() const;
    TQWidget	       *activeWindow() const;

    static TQWidget  *widgetAt( int x, int y, bool child=FALSE );
    static TQWidget  *widgetAt( const TQPoint &, bool child=FALSE );

    static TQEventLoop *eventLoop();

    int		     exec();
    void	     processEvents();
    void	     processEvents( int maxtime );
    void	     processOneEvent();
    bool	     hasPendingEvents();
    int		     enter_loop();
    void	     exit_loop();
    int		     loopLevel() const;
    static void	     exit( int retcode=0 );

    static bool	     sendEvent( TQObject *receiver, TQEvent *event );
    static void	     postEvent( TQObject *receiver, TQEvent *event );
    static void	     sendPostedEvents( TQObject *receiver, int event_type );
    static void	     sendPostedEvents();

    static void      removePostedEvents( TQObject *receiver );

    virtual bool     notify( TQObject *, TQEvent * );

    static bool	     startingUp();
    static bool	     closingDown();

    static void	     flushX();
    static void flush();
    static void	     syncX();

    static void	     beep();

#ifndef TQT_NO_TRANSLATION
# ifndef TQT_NO_TEXTCODEC
    void	     setDefaultCodec( TQTextCodec * );
    TQTextCodec*	     defaultCodec() const;
# endif
    void	     installTranslator( TQTranslator * );
    void	     removeTranslator( TQTranslator * );
#endif
    enum Encoding { DefaultCodec, UnicodeUTF8 };
    TQString	     translate( const char * context,
				const char * key,
				const char * comment = 0,
				Encoding encoding = DefaultCodec ) const;
#ifndef TQT_NO_DIR
    TQString   applicationDirPath();
    TQString   applicationFilePath();
#endif
#ifndef TQT_NO_PALETTE
    // obsolete functions
    static void      setWinStyleHighlightColor( const TQColor &c ) {
	TQPalette p( palette() );
	p.setColor( TQColorGroup::Highlight, c );
	setPalette( p, TRUE);
    }
    static const TQColor &winStyleHighlightColor() {
	return palette().active().highlight();
    }
#endif
    static void      setDesktopSettingsAware( bool );
    static bool      desktopSettingsAware();

    static void      setCursorFlashTime( int );
    static int       cursorFlashTime();

    static void      setDoubleClickInterval( int );
    static int       doubleClickInterval();
#ifndef TQT_NO_WHEELEVENT
    static void      setWheelScrollLines( int );
    static int       wheelScrollLines();
#endif
    static void	     setGlobalStrut( const TQSize & );
    static TQSize     globalStrut();

#ifndef TQT_NO_COMPONENT
    static void      setLibraryPaths( const TQStringList & );
    static TQStringList libraryPaths();
    static void      addLibraryPath( const TQString & );
    static void      removeLibraryPath( const TQString & );
#endif // TQT_NO_COMPONENT
    static void setStartDragTime( int ms );
    static int startDragTime();
    static void setStartDragDistance( int l );
    static int startDragDistance();

    static void setReverseLayout( bool b );
    static bool reverseLayout();

    static int horizontalAlignment( int align );

    static bool	    isEffectEnabled( TQt::UIEffect );
    static void	    setEffectEnabled( TQt::UIEffect, bool enable = TRUE );

#if defined(TQ_WS_MAC)
    virtual bool     macEventFilter( EventHandlerCallRef, EventRef );
#endif
#if defined(TQ_WS_WIN)
    virtual bool     winEventFilter( MSG * );
#endif
#if defined(TQ_WS_X11)
    virtual bool     x11EventFilter( XEvent * );
    virtual int	     x11ClientMessage( TQWidget*, XEvent*, bool passive_only);
    int              x11ProcessEvent( XEvent* );
#endif
#if defined(TQ_WS_QWS)
    virtual bool     qwsEventFilter( TQWSEvent * );
    int              qwsProcessEvent( TQWSEvent* );
    void             qwsSetCustomColors( TQRgb *colortable, int start, int numColors );
/*!
  \internal
    Returns true if the process is GUI server
*/
    bool          qwsIsGUIServer();
#ifndef TQT_NO_QWS_MANAGER
    static TQWSDecoration &qwsDecoration();
    static void      qwsSetDecoration( TQWSDecoration *);
#endif
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_CYGWIN)
    static WindowsVersion winVersion();
#elif defined(Q_OS_MAC)
    static MacintoshVersion macVersion();
#endif
#if defined(TQ_WS_WIN)
    void	     winFocus( TQWidget *, bool );
    static void	     winMouseButtonUp();
#endif

#ifndef TQT_NO_SESSIONMANAGER
    // session management
    bool	     isSessionRestored() const;
    TQString 	sessionId() const;
    TQString 	sessionKey() const;
    virtual void     commitData( TQSessionManager& sm );
    virtual void     saveState( TQSessionManager& sm );
#endif
#if defined(TQ_WS_X11)
#if !defined(TQT_NO_IM_EXTENSIONS)
    virtual TQWidget *locateICHolderWidget( TQWidget *w );
    virtual TQWidgetList *icHolderWidgets();
    static void create_im();
    static void close_im();
#else
    TQWidget *locateICHolderWidget( TQWidget *w );
    TQWidgetList *icHolderWidgets();
    static void create_xim();
    static void close_xim();
#endif
    static TQString defaultInputMethod();
    void	changeAllInputContext( const TQString & );
    static bool x11_apply_settings();
#endif
    void	     wakeUpGuiThread();
#if defined(TQT_THREAD_SUPPORT)
    void	     lock();
    void	     unlock(bool wakeUpGui = TRUE);
    bool	     locked();
    bool             tryLock();
#endif

signals:
    void	     lastWindowClosed();
    void	     aboutToQuit();
    void	     guiThreadAwake();

public slots:
    void	     quit();
    void	     closeAllWindows();
    void	     aboutTQt();

#if defined(TQ_WS_QWS)
protected:
    void setArgs(int, char **);
#endif

protected:
    bool event(TQEvent *);

private:
    void construct( int &argc, char **argv, Type, bool enable_sm );
    void initialize( int, char **, bool enable_sm = true );
    void init_precmdline();
    void process_cmdline( int* argcptr, char ** argv );
    bool internalNotify( TQObject *, TQEvent * );
#if defined(TQ_WS_QWS)
    static TQWidget *findChildWidget( const TQWidget *p, const TQPoint &pos );
    static TQWidget *findWidget( const TQObjectList&, const TQPoint &, bool rec );
#endif

#if defined(TQ_WS_MAC)
    bool do_mouse_down(Point *, bool *);
    static TQMAC_PASCAL OSStatus globalEventProcessor(EventHandlerCallRef,  EventRef, void *);
    static TQMAC_PASCAL OSStatus globalAppleEventProcessor(const AppleEvent *, AppleEvent *, long);
    static TQMAC_PASCAL void qt_context_timer_callbk(EventLoopTimerRef, void *);
    static TQMAC_PASCAL void qt_select_timer_callbk(EventLoopTimerRef, void *);
    static bool qt_mac_apply_settings();
    friend class TQMacInputMethod;
    friend TQMAC_PASCAL OSStatus qt_window_event(EventHandlerCallRef, EventRef, void *);
    friend void qt_mac_update_os_settings();
    friend bool qt_set_socket_handler( int, int, TQObject *, bool);
    friend void qt_mac_destroy_widget(TQWidget *);
    friend void tqt_init(int *, char **, TQApplication::Type);
#endif

#if defined(TQ_WS_X11)
private slots:
    void postIMEvent( TQObject *receiver, TQIMEvent *event );
#endif

public:
#ifdef TQT_THREAD_SUPPORT
    static TQMutex   *tqt_mutex;
#endif // TQT_THREAD_SUPPORT

private:
    int		     app_argc;
    char	   **app_argv;
    bool	     quit_now;
    int		     quit_code;
    static TQStyle   *app_style;
    static int	     app_cspec;
#ifndef TQT_NO_PALETTE
    static TQPalette *app_pal;
#endif
    static TQFont    *app_font;
#ifndef TQT_NO_CURSOR
    static TQCursor  *app_cursor;
#endif
#ifndef TQT_THREAD_SUPPORT
    static TQEventLoop* eventloop;
#endif
    static int	     app_tracking;
    static bool	     is_app_running;
    static bool	     is_app_closing;
    static bool	     app_exit_loop;
    static int	     loop_level;
    static TQWidget  *main_widget;
    static TQWidget  *focus_widget;
    static TQWidget  *active_window;
    static bool	     obey_desktop_settings;
    static int	     cursor_flash_time;
    static int	     mouse_double_click_time;
    static int	     wheel_scroll_lines;
    static int	     composedUnicode; // Value, meta-composed character

    static bool	     animate_ui;
    static bool	     animate_menu;
    static bool	     animate_tooltip;
    static bool	     animate_combo;
    static bool	     fade_menu;
    static bool	     fade_tooltip;
    static bool	     animate_toolbox;
    static bool	     widgetCount; // Coupled with -widgetcount switch
    static bool	     metaComposeUnicode;

    TQValueList<TQTranslator*> *translators;
#ifndef TQT_NO_SESSIONMANAGER
    TQSessionManager *session_manager;
    TQString	     session_id;
    static TQString* session_key;
    bool	     is_session_restored;
#endif
#if defined(TQ_WS_X11)
#if !defined (TQT_NO_STYLE)
    static void x11_initialize_style();
#endif
    static TQString defaultIM; // default input method's name in this application.
#endif

    static TQSize     app_strut;
#ifndef TQT_NO_COMPONENT
    static TQStringList *app_libpaths;
#endif
    static TQAsciiDict<TQPalette> *app_palettes;
    static TQAsciiDict<TQFont>    *app_fonts;

    static TQWidgetList *popupWidgets;
    bool	     inPopupMode() const;
    void	     closePopup( TQWidget *popup );
    void	     openPopup( TQWidget *popup );
    void	     setActiveWindow( TQWidget* act );

    static bool      sendSpontaneousEvent( TQObject *receiver, TQEvent *event );
    static void      removePostedEvent( TQEvent * );
    static void      removePostedEvents( TQObject *receiver, int event_type );

    friend class TQObject;
    friend class TQWidget;
    friend class TQETWidget;
    friend class TQDialog;
    friend class TQAccelManager;
    friend class TQEvent;
#ifdef TQT_THREAD_SUPPORT
    friend class TQThread;
#endif // TQT_THREAD_SUPPORT
    friend class TQTranslator;
    friend class TQEventLoop;
    friend TQ_EXPORT void tqt_ucm_initialize( TQApplication * );
#if defined(TQ_WS_WIN)
    friend bool qt_sendSpontaneousEvent( TQObject*, TQEvent* );
#endif
#if defined(TQ_WS_QWS)
    friend class TQInputContext;
#endif
private: // Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    TQApplication( const TQApplication & );
    TQApplication &operator=( const TQApplication & );
#endif

private:
    static TQEventLoop* currentEventLoop();

public:
#ifdef TQT_THREAD_SUPPORT
    static TQThread* guiThread();
    static void threadTerminationHandler( TQThread * );
#endif
    static bool isGuiThread();
};

inline int TQApplication::argc() const
{
    return app_argc;
}

inline char **TQApplication::argv() const
{
    return app_argv;
}

#if defined(TQ_WS_QWS)
inline void TQApplication::setArgs(int c, char **v)
{
        app_argc = c;
	    app_argv = v;
}
#endif

#ifndef TQT_NO_CURSOR
inline TQCursor *TQApplication::overrideCursor()
{
    return app_cursor;
}
#endif
inline bool TQApplication::hasGlobalMouseTracking()
{
    return app_tracking > 0;
}

inline TQWidget *TQApplication::mainWidget() const
{
    return main_widget;
}

inline TQWidget *TQApplication::focusWidget() const
{
    return focus_widget;
}

inline TQWidget *TQApplication::activeWindow() const
{
    return active_window;
}

inline TQWidget *TQApplication::widgetAt( const TQPoint &p, bool child )
{
    return widgetAt( p.x(), p.y(), child );
}

inline bool TQApplication::inPopupMode() const
{
    return popupWidgets != 0;
}
#ifndef TQT_NO_SESSIONMANAGER
inline bool TQApplication::isSessionRestored() const
{
    return is_session_restored;
}

inline TQString TQApplication::sessionId() const
{
    return session_id;
}

inline TQString TQApplication::sessionKey() const
{
    return session_key ? *session_key : TQString::null;
}
#endif
inline TQSize TQApplication::globalStrut()
{
    return app_strut;
}

#ifdef TQT_NO_TRANSLATION
// Simple versions
inline TQString TQApplication::translate( const char *, const char *sourceText,
					const char *, Encoding encoding ) const
{
#ifndef TQT_NO_TEXTCODEC
    if ( encoding == UnicodeUTF8 )
	return TQString::fromUtf8( sourceText );
    else
#endif
	return TQString::fromLatin1( sourceText );
}
#endif

inline int TQApplication::horizontalAlignment( int align )
{
    align &= AlignHorizontal_Mask;
    if ( align == AlignAuto ) {
	if ( reverseLayout() )
	    align = AlignRight;
	else
	    align = AlignLeft;
    }
    return align;
}

#endif // TQAPPLICATION_H

