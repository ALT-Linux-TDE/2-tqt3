
#include "viewer.h"

#include <tqapplication.h>
 
int main( int argc, char **argv )
{
    TQApplication app( argc, argv );
    Viewer * textViewer = new Viewer();
    textViewer->setCaption( "TQt Example - Simple TQFont Demo" );
    app.setMainWidget( textViewer );
    textViewer->show();
    return app.exec();
}                  
