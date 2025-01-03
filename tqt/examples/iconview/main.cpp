/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <tqiconview.h>
#include <tqapplication.h>
#include <tqdragobject.h>
#include <tqpixmap.h>
#include <tqiconset.h>

#include <tqmime.h>
#include <stdio.h>

class ListenDND : public TQObject
{
    TQ_OBJECT

public:
    ListenDND( TQWidget *w )
        : view( w )
    {}

public slots:
    void dropped( TQDropEvent *mime ) {
        tqDebug( "Dropped Mimesource %p into the view %p", mime, view );
        tqDebug( "  Formats:" );
        int i = 0;
        const char *str = mime->format( i );
        tqDebug( "    %s", str );
        while ( str ) {
            tqDebug( "    %s", str );
            str = mime->format( ++i );
        }
    };
    void moved() {
        tqDebug( "All selected items were moved to another widget" );
    }

protected:
    TQWidget *view;

};

int main( int argc, char **argv )
{
    TQApplication a( argc, argv );

    TQIconView tqiconview;
    tqiconview.setSelectionMode( TQIconView::Extended );

    for ( unsigned int i = 0; i < 3000; i++ ) {
	TQIconViewItem *item = new TQIconViewItem( &tqiconview, TQString( "Item %1" ).arg( i + 1 ) );
	item->setRenameEnabled( TRUE );
    }

    tqiconview.setCaption( "TQt Example - Iconview" );

    ListenDND listen_dnd( &tqiconview );
    TQObject::connect( &tqiconview, TQ_SIGNAL( dropped( TQDropEvent *, const TQValueList<TQIconDragItem> & ) ),
		      &listen_dnd, TQ_SLOT( dropped( TQDropEvent * ) ) );
    TQObject::connect( &tqiconview, TQ_SIGNAL( moved() ), &listen_dnd, TQ_SLOT( moved() ) );

    a.setMainWidget( &tqiconview );
    tqiconview.show();
    tqiconview.resize( tqiconview.sizeHint() );

    return a.exec();
}

#include "main.moc"
