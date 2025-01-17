#include <ntqapplication.h>
#include "hello.h"

int main( int argc, char **argv )
{
    TQApplication a( argc, argv );

    MyPushButton* hello = new MyPushButton( "Hello world!" );
    hello->resize( 100, 30 );

    a.setMainWidget( hello );
    hello->show();
    return a.exec();
}
