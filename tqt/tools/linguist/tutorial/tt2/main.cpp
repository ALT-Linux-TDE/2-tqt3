/****************************************************************
**
** Translation tutorial 2
**
****************************************************************/

#include "mainwindow.h"

#include <tqapplication.h>
#include <tqstring.h>
#include <tqtextcodec.h>
#include <tqtranslator.h>

int main( int argc, char **argv )
{
    TQApplication app( argc, argv );

    TQTranslator translator( 0 );
    translator.load( TQString("tt2_") + TQTextCodec::locale(), "." );
    app.installTranslator( &translator );

    MainWindow *mw = new MainWindow;
    app.setMainWidget( mw );
    mw->show();
    return app.exec();
}
