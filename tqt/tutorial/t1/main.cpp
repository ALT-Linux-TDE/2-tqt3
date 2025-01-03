/****************************************************************
**
** TQt tutorial 1
**
****************************************************************/

#include <tqapplication.h>
#include <tqpushbutton.h>


int main( int argc, char **argv )
{
    TQApplication a( argc, argv );

    TQPushButton hello( "Hello world!", 0 );
    hello.resize( 100, 30 );

    a.setMainWidget( &hello );
    hello.show();
    return a.exec();
}
