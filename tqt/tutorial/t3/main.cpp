/****************************************************************
**
** TQt tutorial 3
**
****************************************************************/

#include <tqapplication.h>
#include <tqpushbutton.h>
#include <tqfont.h>
#include <tqvbox.h>

int main( int argc, char **argv )
{
    TQApplication a( argc, argv );

    TQVBox box;
    box.resize( 200, 120 );

    TQPushButton quit( "Quit", &box );
    quit.setFont( TQFont( "Times", 18, TQFont::Bold ) );

    TQObject::connect( &quit, TQ_SIGNAL(clicked()), &a, TQ_SLOT(quit()) );

    a.setMainWidget( &box );
    box.show();

    return a.exec();
}
