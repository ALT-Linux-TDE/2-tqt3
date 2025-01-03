/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "mainwindow.h"
#include "qfileiconview.h"
#include "../dirview/dirview.h"

#include <ntqsplitter.h>
#include <ntqprogressbar.h>
#include <ntqlabel.h>
#include <ntqstatusbar.h>
#include <ntqtoolbar.h>
#include <ntqcombobox.h>
#include <ntqpixmap.h>
#include <ntqtoolbutton.h>
#include <ntqdir.h>
#include <ntqfileinfo.h>

static const char* cdtoparent_xpm[]={
    "15 13 3 1",
    ". c None",
    "* c #000000",
    "a c #ffff99",
    "..*****........",
    ".*aaaaa*.......",
    "***************",
    "*aaaaaaaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaa***aaaaaaa*",
    "*aa*****aaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa******aaa*",
    "*aaaaaaaaaaaaa*",
    "*aaaaaaaaaaaaa*",
    "***************"};

static const char* newfolder_xpm[] = {
    "15 14 4 1",
    " 	c None",
    ".	c #000000",
    "+	c #FFFF00",
    "@	c #FFFFFF",
    "          .    ",
    "               ",
    "          .    ",
    "       .     . ",
    "  ....  . . .  ",
    " .+@+@.  . .   ",
    "..........  . .",
    ".@+@+@+@+@..   ",
    ".+@+@+@+@+. .  ",
    ".@+@+@+@+@.  . ",
    ".+@+@+@+@+.    ",
    ".@+@+@+@+@.    ",
    ".+@+@+@+@+.    ",
    "...........    "};

FileMainWindow::FileMainWindow()
    : TQMainWindow()
{
    setup();
}

void FileMainWindow::show()
{
    TQMainWindow::show();
}

void FileMainWindow::setup()
{
    TQSplitter *splitter = new TQSplitter( this );

    dirlist = new DirectoryView( splitter, "dirlist", TRUE );
    dirlist->addColumn( "Name" );
    dirlist->addColumn( "Type" );
    Directory *root = new Directory( dirlist, "/" );
    root->setOpen( TRUE );
    splitter->setResizeMode( dirlist, TQSplitter::KeepSize );

    fileview = new TQtFileIconView( "/", splitter );
    fileview->setSelectionMode( TQIconView::Extended );

    setCentralWidget( splitter );

    TQToolBar *toolbar = new TQToolBar( this, "toolbar" );
    setRightJustification( TRUE );

    (void)new TQLabel( tr( " Path: " ), toolbar );

    pathCombo = new TQComboBox( TRUE, toolbar );
    pathCombo->setAutoCompletion( TRUE );
    toolbar->setStretchableWidget( pathCombo );
    connect( pathCombo, SIGNAL( activated( const TQString & ) ),
	     this, SLOT ( changePath( const TQString & ) ) );

    toolbar->addSeparator();

    TQPixmap pix;

    pix = TQPixmap( cdtoparent_xpm );
    upButton = new TQToolButton( pix, "One directory up", TQString::null,
				this, SLOT( cdUp() ), toolbar, "cd up" );

    pix = TQPixmap( newfolder_xpm );
    mkdirButton = new TQToolButton( pix, "New Folder", TQString::null,
				   this, SLOT( newFolder() ), toolbar, "new folder" );

    connect( dirlist, SIGNAL( folderSelected( const TQString & ) ),
	     fileview, SLOT ( setDirectory( const TQString & ) ) );
    connect( fileview, SIGNAL( directoryChanged( const TQString & ) ),
	     this, SLOT( directoryChanged( const TQString & ) ) );
    connect( fileview, SIGNAL( startReadDir( int ) ),
	     this, SLOT( slotStartReadDir( int ) ) );
    connect( fileview, SIGNAL( readNextDir() ),
	     this, SLOT( slotReadNextDir() ) );
    connect( fileview, SIGNAL( readDirDone() ),
	     this, SLOT( slotReadDirDone() ) );

    setDockEnabled( DockLeft, FALSE );
    setDockEnabled( DockRight, FALSE );

    label = new TQLabel( statusBar() );
    statusBar()->addWidget( label, 2, TRUE );
    progress = new TQProgressBar( statusBar() );
    statusBar()->addWidget( progress, 1, TRUE );

    connect( fileview, SIGNAL( enableUp() ),
	     this, SLOT( enableUp() ) );
    connect( fileview, SIGNAL( disableUp() ),
	     this, SLOT( disableUp() ) );
    connect( fileview, SIGNAL( enableMkdir() ),
	     this, SLOT( enableMkdir() ) );
    connect( fileview, SIGNAL( disableMkdir() ),
	     this, SLOT( disableMkdir() ) );
}

void FileMainWindow::setPathCombo()
{
    TQString dir = caption();
    int i = 0;
    bool found = FALSE;
    for ( i = 0; i < pathCombo->count(); ++i ) {
	if ( pathCombo->text( i ) == dir) {
	    found = TRUE;
	    break;
	}
    }

    if ( found )
	pathCombo->setCurrentItem( i );
    else {
	pathCombo->insertItem( dir );
	pathCombo->setCurrentItem( pathCombo->count() - 1 );
    }

}

void FileMainWindow::directoryChanged( const TQString &dir )
{
    setCaption( dir );
    setPathCombo();
}

void FileMainWindow::slotStartReadDir( int dirs )
{
    label->setText( tr( " Reading Directory..." ) );
    progress->reset();
    progress->setTotalSteps( dirs );
}

void FileMainWindow::slotReadNextDir()
{
    int p = progress->progress();
    progress->setProgress( ++p );
}

void FileMainWindow::slotReadDirDone()
{
    label->setText( tr( " Reading Directory Done." ) );
    progress->setProgress( progress->totalSteps() );
}

void FileMainWindow::cdUp()
{
    TQDir dir = fileview->currentDir();
    dir.cd( ".." );
    fileview->setDirectory( dir );
}

void FileMainWindow::newFolder()
{
    fileview->newDirectory();
}

void FileMainWindow::changePath( const TQString &path )
{
    if ( TQFileInfo( path ).exists() )
	fileview->setDirectory( path );
    else
	setPathCombo();
}

void FileMainWindow::enableUp()
{
    upButton->setEnabled( TRUE );
}

void FileMainWindow::disableUp()
{
    upButton->setEnabled( FALSE );
}

void FileMainWindow::enableMkdir()
{
    mkdirButton->setEnabled( TRUE );
}

void FileMainWindow::disableMkdir()
{
    mkdirButton->setEnabled( FALSE );
}
