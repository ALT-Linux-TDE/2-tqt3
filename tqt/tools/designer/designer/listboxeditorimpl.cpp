/**********************************************************************
** Copyright (C) 2005-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of TQt Designer.
**
** This file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the files LICENSE.GPL2
** and LICENSE.GPL3 included in the packaging of this file.
** Alternatively you may (at your option) use any later version
** of the GNU General Public License if such license has been
** publicly approved by Trolltech ASA (or its successors, if any)
** and the KDE Free TQt Foundation.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/.
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** Licensees holding valid TQt Commercial licenses may use this file in
** accordance with the TQt Commercial License Agreement provided with
** the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include "listboxeditorimpl.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "pixmapchooser.h"
#include "command.h"
#include "listboxdnd.h"
#include "listboxrename.h"

#include <ntqlineedit.h>
#include <ntqlabel.h>
#include <ntqlistbox.h>
#include <ntqpushbutton.h>
#include <ntqcheckbox.h>

ListBoxEditor::ListBoxEditor( TQWidget *parent, TQWidget *editWidget, FormWindow *fw )
    : ListBoxEditorBase( parent, 0, TRUE ), formwindow( fw )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    listbox = (TQListBox*)editWidget;

    itemText->setText( "" );
    itemText->setEnabled( FALSE );
    itemPixmap->setText( "" );
    itemChoosePixmap->setEnabled( FALSE );
    itemDeletePixmap->setEnabled( FALSE );

    TQListBoxItem *i = 0;
    for ( i = listbox->firstItem(); i; i = i->next() ) {
	if ( i->pixmap() )
	    (void)new TQListBoxPixmap( preview, *i->pixmap(), i->text() );
	else
	    (void)new TQListBoxText( preview, i->text() );
    }

    if ( preview->firstItem() )
	preview->setCurrentItem( preview->firstItem() );

    ListBoxDnd *editorDnd = new ListBoxDnd( preview );
    editorDnd->setDragMode( ListBoxDnd::Internal | ListBoxDnd::Move );
    TQObject::connect( editorDnd, SIGNAL( dropped( TQListBoxItem * ) ),
		      editorDnd, SLOT( confirmDrop( TQListBoxItem * ) ) );

    ListBoxRename *editorRename = new ListBoxRename( preview );

    TQObjectList *l = parent->queryList( "TQLineEdit", "itemText" );
    TQObjectListIt it( *l );
    TQObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
	TQObject::connect( editorRename,
			  SIGNAL( itemTextChanged( const TQString & ) ),
			  obj,
			  SLOT( setText( const TQString & ) ) );
    }
    delete l;
    
}

void ListBoxEditor::insertNewItem()
{
    TQListBoxItem *i = new TQListBoxText( preview, tr("New Item") );
    preview->setCurrentItem( i );
    preview->setSelected( i, TRUE );
    itemText->setFocus();
    itemText->selectAll();
}

void ListBoxEditor::deleteCurrentItem()
{
    delete preview->item( preview->currentItem() );
    if ( preview->currentItem() != -1 )
	preview->setSelected( preview->currentItem(), TRUE );
}

void ListBoxEditor::currentItemChanged( TQListBoxItem *i )
{
    itemText->blockSignals( TRUE );
    itemText->setText( "" );
    itemPixmap->setText( "" );
    itemText->blockSignals( FALSE );

    if ( !i ) {
	itemText->setEnabled( FALSE );
	itemChoosePixmap->setEnabled( FALSE );
	itemDeletePixmap->setEnabled( FALSE );
	return;
    }

    itemText->blockSignals( TRUE );
    itemText->setEnabled( TRUE );
    itemChoosePixmap->setEnabled( TRUE );
    itemDeletePixmap->setEnabled( i->pixmap() && !i->pixmap()->isNull() );

    itemText->setText( i->text() );
    if ( i->pixmap() )
	itemPixmap->setPixmap( *i->pixmap() );
    itemText->blockSignals( FALSE );
}

void ListBoxEditor::currentTextChanged( const TQString &txt )
{
    if ( preview->currentItem() == -1 )
	return;

    preview->blockSignals( TRUE );
    if ( preview->item( preview->currentItem() )->pixmap() )
	preview->changeItem( *preview->item( preview->currentItem() )->pixmap(), txt, preview->currentItem() );
    else
	preview->changeItem( txt, preview->currentItem() );
    preview->blockSignals( FALSE );
}

void ListBoxEditor::okClicked()
{
    applyClicked();
    accept();
}

void ListBoxEditor::cancelClicked()
{
    reject();
}

void ListBoxEditor::applyClicked()
{
    TQListBoxItem *i = 0;
    TQValueList<PopulateListBoxCommand::Item> items;
    for ( i = preview->firstItem(); i; i = i->next() ) {
	PopulateListBoxCommand::Item item;
	if ( i->pixmap() )
	    item.pix = *i->pixmap();
	item.text = i->text();
	items.append( item );
    }

    PopulateListBoxCommand *cmd = new PopulateListBoxCommand( tr( "Edit the Items of '%1'" ).arg( listbox->name() ),
							      formwindow, listbox, items );
    cmd->execute();
    formwindow->commandHistory()->addCommand( cmd );
}

void ListBoxEditor::choosePixmap()
{
    if ( preview->currentItem() == -1 )
	return;

    TQPixmap pix;
    if ( preview->item( preview->currentItem() )->pixmap() )
    	pix = qChoosePixmap( this, formwindow, *preview->item( preview->currentItem() )->pixmap() );
    else
    	pix = qChoosePixmap( this, formwindow, TQPixmap() );

    if ( pix.isNull() )
	return;

    TQString txt = preview->item( preview->currentItem() )->text();
    preview->changeItem( pix, txt, preview->currentItem() );
    itemDeletePixmap->setEnabled( TRUE );
}

void ListBoxEditor::moveItemUp()
{
    if ( preview->currentItem() < 1 )
	return;

    TQListBoxItem *i = preview->item( preview->currentItem() );
    bool hasPix = (i->pixmap() != 0);
    TQPixmap pix;
    if ( hasPix )
	pix = *i->pixmap();
    TQString txt = i->text();

    TQListBoxItem *p = i->prev();
    if ( p->pixmap() )
	preview->changeItem( *p->pixmap(), p->text(), preview->currentItem() );
    else
	preview->changeItem( p->text(), preview->currentItem() );

    if ( hasPix )
	preview->changeItem( pix, txt, preview->currentItem() - 1 );
    else
	preview->changeItem( txt, preview->currentItem() - 1 );
}

void ListBoxEditor::moveItemDown()
{
    if ( preview->currentItem() == -1 || preview->currentItem() > (int)preview->count() - 2 )
	return;

    TQListBoxItem *i = preview->item( preview->currentItem() );
    bool hasPix = (i->pixmap() != 0);
    TQPixmap pix;
    if ( hasPix )
	pix = *i->pixmap();
    TQString txt = i->text();

    TQListBoxItem *n = i->next();
    if ( n->pixmap() )
	preview->changeItem( *n->pixmap(), n->text(), preview->currentItem() );
    else
	preview->changeItem( n->text(), preview->currentItem() );

    if ( hasPix )
	preview->changeItem( pix, txt, preview->currentItem() + 1 );
    else
	preview->changeItem( txt, preview->currentItem() + 1 );
}

void ListBoxEditor::deletePixmap()
{
    if ( preview->currentItem() == -1 )
	return;

    TQListBoxItem *i = preview->item( preview->currentItem() );
    preview->changeItem( i->text(), preview->currentItem() );
    itemDeletePixmap->setEnabled( FALSE );
}
