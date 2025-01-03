/****************************************************************
**
** Translation tutorial 1
**
****************************************************************/

#include <tqapplication.h>
#include <tqpushbutton.h>
#include <tqtranslator.h>


int main( int argc, char **argv )
{
    TQApplication app( argc, argv );

    TQTranslator translator( 0 );
    translator.load( "tt1_la", "." );
    app.installTranslator( &translator );

    TQPushButton hello( TQPushButton::tr("Hello world!"), 0 );

    app.setMainWidget( &hello );
    hello.show();
    return app.exec();
}
