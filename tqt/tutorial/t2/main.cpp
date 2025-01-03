/****************************************************************
**
** TQt tutorial 2
**
****************************************************************/

#include <tqapplication.h>
#include <tqpushbutton.h>
#include <tqfont.h>


int main( int argc, char **argv )
{
    TQApplication a( argc, argv );

    TQPushButton quit( "Quit", 0 );
    quit.resize( 75, 30 );
    quit.setFont( TQFont( "Times", 18, TQFont::Bold ) );

    TQObject::connect( &quit, TQ_SIGNAL(clicked()), &a, TQ_SLOT(quit()) );

    a.setMainWidget( &quit );
    quit.show();
    return a.exec();
}
