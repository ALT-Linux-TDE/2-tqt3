#include <ntqapplication.h>

#include "startup.h"

int main( int argc, char **argv )
{
    TQApplication a( argc, argv );
    StartUp s;
    TQObject::connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
