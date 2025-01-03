/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "fontdisplayer.h"

#include <tqapplication.h>
#include <tqslider.h>
#include <tqpainter.h>
#include <tqstatusbar.h>



int main(int argc, char** argv)
{
    TQApplication app(argc,argv);

    FontDisplayer m;
    TQSize sh = m.centralWidget()->sizeHint();
    m.resize(sh.width(),
             sh.height()+3*m.statusBar()->height());
    app.setMainWidget(&m);
    m.setCaption("TQt Example - TQFD");
    m.show();

    return app.exec();
}
