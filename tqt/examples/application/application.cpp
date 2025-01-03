/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "application.h"

#include <ntqimage.h>
#include <ntqpixmap.h>
#include <ntqtoolbar.h>
#include <ntqtoolbutton.h>
#include <ntqpopupmenu.h>
#include <ntqmenubar.h>
#include <ntqtextedit.h>
#include <ntqfile.h>
#include <ntqfiledialog.h>
#include <ntqstatusbar.h>
#include <ntqmessagebox.h>
#include <ntqprinter.h>
#include <ntqapplication.h>
#include <ntqaccel.h>
#include <ntqtextstream.h>
#include <ntqpainter.h>
#include <ntqpaintdevicemetrics.h>
#include <ntqwhatsthis.h>
#include <ntqsimplerichtext.h>

#include "filesave.xpm"
#include "fileopen.xpm"
#include "fileprint.xpm"

ApplicationWindow::ApplicationWindow()
    : TQMainWindow( 0, "example application main window", WDestructiveClose | WGroupLeader )
{
    printer = new TQPrinter( TQPrinter::HighResolution );
    TQPixmap openIcon, saveIcon, printIcon;

    TQToolBar * fileTools = new TQToolBar( this, "file operations" );
    fileTools->setLabel( "File Operations" );

    openIcon = TQPixmap( fileopen );
    TQToolButton * fileOpen
	= new TQToolButton( openIcon, "Open File", TQString::null,
			   this, SLOT(choose()), fileTools, "open file" );

    saveIcon = TQPixmap( filesave );
    TQToolButton * fileSave
	= new TQToolButton( saveIcon, "Save File", TQString::null,
			   this, SLOT(save()), fileTools, "save file" );

    printIcon = TQPixmap( fileprint );
    TQToolButton * filePrint
	= new TQToolButton( printIcon, "Print File", TQString::null,
			   this, SLOT(print()), fileTools, "print file" );


    (void)TQWhatsThis::whatsThisButton( fileTools );

    const char * fileOpenText = "<p><img source=\"fileopen\"> "
	         "Click this button to open a <em>new file</em>.<br>"
                 "You can also select the <b>Open</b> command "
                 "from the <b>File</b> menu.</p>";

    TQWhatsThis::add( fileOpen, fileOpenText );

    TQMimeSourceFactory::defaultFactory()->setPixmap( "document-open", openIcon );

    const char * fileSaveText = "<p>Click this button to save the file you "
                 "are editing. You will be prompted for a file name.\n"
                 "You can also select the <b>Save</b> command "
                 "from the <b>File</b> menu.</p>";

    TQWhatsThis::add( fileSave, fileSaveText );

    const char * filePrintText = "Click this button to print the file you "
                 "are editing.\n"
		 "You can also select the Print command "
		 "from the File menu.";

    TQWhatsThis::add( filePrint, filePrintText );


    TQPopupMenu * file = new TQPopupMenu( this );
    menuBar()->insertItem( "&File", file );


    file->insertItem( "&New", this, SLOT(newDoc()), CTRL+Key_N );

    int id;
    id = file->insertItem( openIcon, "&Open...",
			   this, SLOT(choose()), CTRL+Key_O );
    file->setWhatsThis( id, fileOpenText );

    id = file->insertItem( saveIcon, "&Save",
			   this, SLOT(save()), CTRL+Key_S );
    file->setWhatsThis( id, fileSaveText );

    id = file->insertItem( "Save &As...", this, SLOT(saveAs()) );
    file->setWhatsThis( id, fileSaveText );

    file->insertSeparator();

    id = file->insertItem( printIcon, "&Print...",
			   this, SLOT(print()), CTRL+Key_P );
    file->setWhatsThis( id, filePrintText );

    file->insertSeparator();

    file->insertItem( "&Close", this, SLOT(close()), CTRL+Key_W );

    file->insertItem( "&Quit", tqApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

    menuBar()->insertSeparator();

    TQPopupMenu * help = new TQPopupMenu( this );
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1 );
    help->insertItem( "About &TQt", this, SLOT(aboutTQt()) );
    help->insertSeparator();
    help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1 );

    e = new TQTextEdit( this, "editor" );
    e->setFocus();
    setCentralWidget( e );
    statusBar()->message( "Ready", 2000 );

    resize( 450, 600 );
}


ApplicationWindow::~ApplicationWindow()
{
    delete printer;
}



void ApplicationWindow::newDoc()
{
    ApplicationWindow *ed = new ApplicationWindow;
    ed->setCaption("TQt Example - Application");
    ed->show();
}

void ApplicationWindow::choose()
{
    TQString fn = TQFileDialog::getOpenFileName( TQString::null, TQString::null,
					       this);
    if ( !fn.isEmpty() )
	load( fn );
    else
	statusBar()->message( "Loading aborted", 2000 );
}


void ApplicationWindow::load( const TQString &fileName )
{
    TQFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
	return;

    TQTextStream ts( &f );
    e->setText( ts.read() );
    e->setModified( FALSE );
    setCaption( fileName );
    statusBar()->message( "Loaded document " + fileName, 2000 );
}


void ApplicationWindow::save()
{
    if ( filename.isEmpty() ) {
	saveAs();
	return;
    }

    TQString text = e->text();
    TQFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
	statusBar()->message( TQString("Could not write to %1").arg(filename),
			      2000 );
	return;
    }

    TQTextStream t( &f );
    t << text;
    f.close();

    e->setModified( FALSE );

    setCaption( filename );

    statusBar()->message( TQString( "File %1 saved" ).arg( filename ), 2000 );
}


void ApplicationWindow::saveAs()
{
    TQString fn = TQFileDialog::getSaveFileName( TQString::null, TQString::null,
					       this );
    if ( !fn.isEmpty() ) {
	filename = fn;
	save();
    } else {
	statusBar()->message( "Saving aborted", 2000 );
    }
}


void ApplicationWindow::print()
{
    printer->setFullPage( TRUE );
    if ( printer->setup(this) ) {		// printer dialog
	statusBar()->message( "Printing..." );
	TQPainter p;
	if( !p.begin( printer ) ) {               // paint on printer
	    statusBar()->message( "Printing aborted", 2000 );
	    return;
	}

	TQPaintDeviceMetrics metrics( p.device() );
	int dpiy = metrics.logicalDpiY();
	int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
	TQRect view( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
	TQSimpleRichText richText( TQStyleSheet::convertFromPlainText(e->text()),
				  TQFont(),
				  e->context(),
				  e->styleSheet(),
				  e->mimeSourceFactory(),
				  view.height() );
	richText.setWidth( &p, view.width() );
	int page = 1;
	do {
	    richText.draw( &p, margin, margin, view, colorGroup() );
	    view.moveBy( 0, view.height() );
	    p.translate( 0 , -view.height() );
	    p.drawText( view.right() - p.fontMetrics().width( TQString::number( page ) ),
			view.bottom() + p.fontMetrics().ascent() + 5, TQString::number( page ) );
	    if ( view.top() - margin >= richText.height() )
		break;
	    printer->newPage();
	    page++;
	} while (TRUE);

	statusBar()->message( "Printing completed", 2000 );
    } else {
	statusBar()->message( "Printing aborted", 2000 );
    }
}

void ApplicationWindow::closeEvent( TQCloseEvent* ce )
{
    if ( !e->isModified() ) {
	ce->accept();
	return;
    }

    switch( TQMessageBox::information( this, "TQt Application Example",
				      "Do you want to save the changes"
				      " to the document?",
				      "Yes", "No", "Cancel",
				      0, 1 ) ) {
    case 0:
	save();
	ce->accept();
	break;
    case 1:
	ce->accept();
	break;
    case 2:
    default: // just for sanity
	ce->ignore();
	break;
    }
}


void ApplicationWindow::about()
{
    TQMessageBox::about( this, "TQt Application Example",
			"This example demonstrates simple use of "
			"TQMainWindow,\nTQMenuBar and TQToolBar.");
}


void ApplicationWindow::aboutTQt()
{
    TQMessageBox::aboutTQt( this, "TQt Application Example" );
}
