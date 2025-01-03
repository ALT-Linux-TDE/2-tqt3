/****************************************************************
**
** Implementation of MainWindow class, translation tutorial 2
**
****************************************************************/

#include "arrowpad.h"
#include "mainwindow.h"

#include <tqaccel.h>
#include <tqapplication.h>
#include <tqmenubar.h>
#include <tqpopupmenu.h>

MainWindow::MainWindow( TQWidget *parent, const char *name )
    : TQMainWindow( parent, name )
{
    ArrowPad *ap = new ArrowPad( this, "arrow pad" );
    setCentralWidget( ap );

    TQPopupMenu *file = new TQPopupMenu( this );
    file->insertItem( tr("E&xit"), tqApp, TQ_SLOT(quit()),
		      tr("Ctrl+Q", "Quit") );
    menuBar()->insertItem( tr("&File"), file );
    menuBar()->setSeparator( TQMenuBar::InWindowsStyle );
}
