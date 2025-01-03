#include <ntqapplication.h>
#include "helpdemo.h"

int main( int argc, char ** argv )
{
    TQApplication a( argc, argv );
    HelpDemo help;
    help.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
