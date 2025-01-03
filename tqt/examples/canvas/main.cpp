#include <tqstatusbar.h>
#include <tqmessagebox.h>
#include <tqmenubar.h>
#include <tqapplication.h>
#include <tqimage.h>

#include "canvas.h"

#include <stdlib.h>

extern TQString butterfly_fn;
extern TQString logo_fn;

int main(int argc, char** argv)
{
    TQApplication app(argc,argv);
    
    if ( argc > 1 )
	butterfly_fn = argv[1];
    else
	butterfly_fn = "butterfly.png";
    
    if ( argc > 2 )
	logo_fn = argv[2];
    else
	logo_fn = "qtlogo.png";
    
    TQCanvas canvas(800,600);
    canvas.setAdvancePeriod(30);
    Main m(canvas);
    m.resize(m.sizeHint());
    m.setCaption("TQt Example - Canvas");
    if ( TQApplication::desktop()->width() > m.width() + 10
	&& TQApplication::desktop()->height() > m.height() +30 )
	m.show();
    else
	m.showMaximized();
    
    TQObject::connect( tqApp, TQ_SIGNAL(lastWindowClosed()), tqApp, TQ_SLOT(quit()) );
    
    return app.exec();
}

