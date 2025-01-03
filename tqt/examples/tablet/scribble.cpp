/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "canvas.h"
#include "scribble.h"

#include <tqapplication.h>
#include <tqevent.h>
#include <tqpainter.h>
#include <tqtoolbar.h>
#include <tqtoolbutton.h>
#include <tqspinbox.h>
#include <tqtooltip.h>
#include <tqrect.h>
#include <tqpoint.h>
#include <tqcolordialog.h>
#include <tqfiledialog.h>
#include <tqcursor.h>
#include <tqimage.h>
#include <tqstrlist.h>
#include <tqpopupmenu.h>
#include <tqintdict.h>



Scribble::Scribble( TQWidget *parent, const char *name )
    : TQMainWindow( parent, name )
{
    canvas = new Canvas( this );
    setCentralWidget( canvas );

    TQToolBar *tools = new TQToolBar( this );

    bSave = new TQToolButton( TQPixmap(), "Save", "Save as PNG image", this, TQ_SLOT( slotSave() ), tools );
    bSave->setText( "Save as..." );

    tools->addSeparator();

    bPColor = new TQToolButton( TQPixmap(), "Choose Pen Color", "Choose Pen Color", this, TQ_SLOT( slotColor() ), tools );
    bPColor->setText( "Choose Pen Color..." );

    tools->addSeparator();

    bPWidth = new TQSpinBox( 1, 20, 1, tools );
    TQToolTip::add( bPWidth, "Choose Pen Width" );
    connect( bPWidth, TQ_SIGNAL( valueChanged( int ) ), this, TQ_SLOT( slotWidth( int ) ) );
    bPWidth->setValue( 3 );

    tools->addSeparator();

    bClear = new TQToolButton( TQPixmap(), "Clear Screen", "Clear Screen", this, TQ_SLOT( slotClear() ), tools );
    bClear->setText( "Clear Screen" );
}

/*

Scribble::Scribble( TQWidget *parent, const char *name )
    : TQMainWindow( parent, name )
{
    canvas = new Canvas( this );
    setCentralWidget( canvas );

    TQToolBar *tools = new TQToolBar( this );

    bSave = new TQPushButton( "Save as...", tools );

    tools->addSeparator();

    bPColor = new TQPushButton( "Choose Pen Color...", tools );
    //    bPColor->setText( "Choose Pen Color..." );

    tools->addSeparator();

    bPWidth = new TQSpinBox( 1, 20, 1, tools );
    TQToolTip::add( bPWidth, "Choose Pen Width" );
    connect( bPWidth, TQ_SIGNAL( valueChanged( int ) ), this, TQ_SLOT( slotWidth( int ) ) );
    bPWidth->setValue( 3 );

    tools->addSeparator();

    bClear = new TQPushButton( "Clear Screen", tools );
    TQObject::connect( bSave, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotSave() ) );
    TQObject::connect( bPColor, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotColor() ) );
    TQObject::connect( bClear, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotClear() ) );
		
}

  */
void Scribble::slotSave()
{
    TQPopupMenu *menu = new TQPopupMenu( 0 );
    TQIntDict<TQString> formats;
    formats.setAutoDelete( TRUE );

    for ( unsigned int i = 0; i < TQImageIO::outputFormats().count(); i++ ) {
	TQString str = TQString( TQImageIO::outputFormats().at( i ) );
	formats.insert( menu->insertItem( TQString( "%1..." ).arg( str ) ), new TQString( str ) );
    }

    menu->setMouseTracking( TRUE );
    int id = menu->exec( bSave->mapToGlobal( TQPoint( 0, bSave->height() + 1 ) ) );

    if ( id != -1 ) {
	TQString format = *formats[ id ];

	TQString filename = TQFileDialog::getSaveFileName( TQString::null, TQString( "*.%1" ).arg( format.lower() ), this );
	if ( !filename.isEmpty() )
	    canvas->save( filename, format );
    }

    delete menu;
}

void Scribble::slotColor()
{
    TQColor c = TQColorDialog::getColor( canvas->penColor(), this );
    canvas->setPenColor( c );
}

void Scribble::slotWidth( int w )
{
    canvas->setPenWidth( w );
}

void Scribble::slotClear()
{
    canvas->clearScreen();
}
