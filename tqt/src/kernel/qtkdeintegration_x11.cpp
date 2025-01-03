/* This file is licensed under the terms of the GPL v2 or v3, as it has been publicly released by
   OpenSUSE as part of their GPLed TQt library disribution */

#define QT_CLEAN_NAMESPACE
#include "qtkdeintegration_x11_p.h"

#include <ntqcolordialog.h>
#include <ntqfiledialog.h>
#include <ntqfontdialog.h>
#include <ntqlibrary.h>
#include <ntqregexp.h>
#include <ntqmessagebox.h>
#include <stdlib.h>

bool TQKDEIntegration::inited = false;
bool TQKDEIntegration::enable = false;

bool TQKDEIntegration::enabled()
    {
    if( !inited )
        initLibrary();
    return enable;
    }

static TQCString findLibrary()
    {
    if( getenv( "QT_NO_KDE_INTEGRATION" ) == NULL
        || getenv( "QT_NO_KDE_INTEGRATION" )[ 0 ] == '0' )
        {
#ifdef USE_LIB64_PATHES
        return "/opt/kde3/lib64/kde3/plugins/integration/libqtkde";
#else
        return "/opt/kde3/lib/kde3/plugins/integration/libqtkde";
#endif
        }
    return "";
    }

inline static long widgetToWinId( const TQWidget* w )
    {
    return w != NULL ? w->winId() : 0;
    }

inline static TQFont fontPtrToFontRef( const TQFont* f )
    {
    return f != NULL ? *f : TQFont();
    }

// ---
static bool (*qtkde_initializeIntegration)( );
static TQStringList (*qtkde_getOpenFileNames)( const TQString& filter, TQString* workingDirectory,
    long parent, const TQCString& name, const TQString& caption, TQString* selectedFilter,
    bool multiple );
static TQString (*qtkde_getSaveFileName)( const TQString& initialSelection, const TQString& filter,
    TQString* workingDirectory, long parent, const TQCString& name, const TQString& caption,
    TQString* selectedFilter );
static TQString (*qtkde_getExistingDirectory)( const TQString& initialDirectory, long parent,
    const TQCString& name, const TQString& caption );
static TQColor (*qtkde_getColor)( const TQColor& color, long parent, const TQCString& name );
static TQFont (*qtkde_getFont)( bool* ok, const TQFont& def, long parent, const TQCString& name );
static int (*qtkde_messageBox1)( int type, long parent, const TQString& caption, const TQString& text,
    int button0, int button1, int button2 );
static int (*qtkde_messageBox2)( int type, long parent, const TQString& caption, const TQString& text,
    const TQString& button0Text, const TQString& button1Text, const TQString& button2Text,
    int defaultButton, int escapeButton );

void TQKDEIntegration::initLibrary()
    {
    if( !inited )
        {
        enable = false;
        inited = true;
        TQString libpath = findLibrary();
        if( libpath.isEmpty())
            return;
        TQLibrary lib( libpath );
        lib.setAutoUnload( false );
        qtkde_initializeIntegration = (
            bool (*)( )
            )
            lib.resolve("initializeIntegration");
        if( qtkde_initializeIntegration == NULL )
            return;
        qtkde_getOpenFileNames = (
            TQStringList (*)( const TQString& filter, TQString* workingDirectory, long parent,
                const TQCString& name, const TQString& caption, TQString* selectedFilter,
                bool multiple )
            )
            lib.resolve("getOpenFileNames");
        if( qtkde_getOpenFileNames == NULL )
            return;
        qtkde_getSaveFileName = (
            TQString (*)( const TQString& initialSelection, const TQString& filter, TQString* workingDirectory,
                long parent, const TQCString& name, const TQString& caption, TQString* selectedFilter )
            )
            lib.resolve("getSaveFileName");
        if( qtkde_getSaveFileName == NULL )
            return;
        qtkde_getExistingDirectory = (
            TQString (*)( const TQString& initialDirectory, long parent, const TQCString& name,
                const TQString& caption )
            )
            lib.resolve("getExistingDirectory");
        if( qtkde_getExistingDirectory == NULL )
            return;
        qtkde_getColor = (
            TQColor (*)( const TQColor& color, long parent, const TQCString& name )
            )
            lib.resolve("getColor");
        if( qtkde_getColor == NULL )
            return;
        qtkde_getFont = (
            TQFont (*)( bool* ok, const TQFont& def, long parent, const TQCString& name )
            )
            lib.resolve("getFont");
        if( qtkde_getFont == NULL )
            return;
        qtkde_messageBox1 = (
            int (*)( int type, long parent, const TQString& caption, const TQString& text,
                int button0, int button1, int button2 )
            )
            lib.resolve("messageBox1");
        if( qtkde_messageBox1 == NULL )
            return;
        qtkde_messageBox2 = (
            int (*)( int type, long parent, const TQString& caption, const TQString& text,
                const TQString& button0Text, const TQString& button1Text, const TQString& button2Text,
                int defaultButton, int escapeButton )
            )
            lib.resolve("messageBox2");
        if( qtkde_messageBox2 == NULL )
            return;
        enable = qtkde_initializeIntegration();
        }
    }

bool TQKDEIntegration::initializeIntegration( )
    {
    return qtkde_initializeIntegration(
         );
    }
TQStringList TQKDEIntegration::getOpenFileNames( const TQString& filter, TQString* workingDirectory,
    TQWidget* parent, const char* name, const TQString& caption, TQString* selectedFilter,
    bool multiple )
    {
    return qtkde_getOpenFileNames(
        filter, workingDirectory, widgetToWinId( parent ), name, caption, selectedFilter, multiple );
    }
TQString TQKDEIntegration::getSaveFileName( const TQString& initialSelection, const TQString& filter,
    TQString* workingDirectory, TQWidget* parent, const char* name, const TQString& caption,
    TQString* selectedFilter )
    {
    return qtkde_getSaveFileName(
        initialSelection, filter, workingDirectory, widgetToWinId( parent ), name, caption, selectedFilter );
    }
TQString TQKDEIntegration::getExistingDirectory( const TQString& initialDirectory, TQWidget* parent,
    const char* name, const TQString& caption )
    {
    return qtkde_getExistingDirectory(
        initialDirectory, widgetToWinId( parent ), name, caption );
    }
TQColor TQKDEIntegration::getColor( const TQColor& color, TQWidget* parent, const char* name )
    {
    return qtkde_getColor(
        color, widgetToWinId( parent ), name );
    }
TQFont TQKDEIntegration::getFont( bool* ok, const TQFont* def, TQWidget* parent, const char* name )
    {
    return qtkde_getFont(
        ok, fontPtrToFontRef( def ), widgetToWinId( parent ), name );
    }
int TQKDEIntegration::messageBox1( int type, TQWidget* parent, const TQString& caption,
    const TQString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        type, widgetToWinId( parent ), caption, text, button0, button1, button2 );
    }
int TQKDEIntegration::messageBox2( int type, TQWidget* parent, const TQString& caption,
    const TQString& text, const TQString& button0Text, const TQString& button1Text, const TQString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        type, widgetToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }
// ---

int TQKDEIntegration::information( TQWidget* parent, const TQString& caption,
    const TQString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        TQMessageBox::Information, widgetToWinId( parent ), caption, text, button0, button1, button2 );
    }

int TQKDEIntegration::question( TQWidget* parent, const TQString& caption,
    const TQString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        TQMessageBox::Question, widgetToWinId( parent ), caption, text, button0, button1, button2 );
    }

int TQKDEIntegration::warning( TQWidget* parent, const TQString& caption,
    const TQString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        TQMessageBox::Warning, widgetToWinId( parent ), caption, text, button0, button1, button2 );
    }

int TQKDEIntegration::critical( TQWidget* parent, const TQString& caption,
    const TQString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        TQMessageBox::Critical, widgetToWinId( parent ), caption, text, button0, button1, button2 );
    }

int TQKDEIntegration::information( TQWidget* parent, const TQString& caption,
    const TQString& text, const TQString& button0Text, const TQString& button1Text, const TQString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        TQMessageBox::Information, widgetToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }

int TQKDEIntegration::question( TQWidget* parent, const TQString& caption,
    const TQString& text, const TQString& button0Text, const TQString& button1Text, const TQString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        TQMessageBox::Question, widgetToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }

int TQKDEIntegration::warning( TQWidget* parent, const TQString& caption,
    const TQString& text, const TQString& button0Text, const TQString& button1Text, const TQString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        TQMessageBox::Warning, widgetToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }

int TQKDEIntegration::critical( TQWidget* parent, const TQString& caption,
    const TQString& text, const TQString& button0Text, const TQString& button1Text, const TQString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        TQMessageBox::Critical, widgetToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }
