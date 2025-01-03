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

#include "editor.h"
#include "parenmatcher.h"
#include <ntqfile.h>
#include <private/qrichtext_p.h>
#include "conf.h"
#include <ntqapplication.h>
#include <ntqpopupmenu.h>
#include <ntqaccel.h>

Editor::Editor( const TQString &fn, TQWidget *parent, const char *name )
    : TQTextEdit( parent, name ), hasError( FALSE )
{
    document()->setFormatter( new TQTextFormatterBreakInWords );
    if ( !fn.isEmpty() )
	load( fn );
    setHScrollBarMode( TQScrollView::AlwaysOff );
    setVScrollBarMode( TQScrollView::AlwaysOn );
    document()->setUseFormatCollection( FALSE );
    parenMatcher = new ParenMatcher;
    connect( this, SIGNAL( cursorPositionChanged( TQTextCursor * ) ),
	     this, SLOT( cursorPosChanged( TQTextCursor * ) ) );
    cfg = new Config;
    document()->addSelection( Error );
    document()->addSelection( Step );
    document()->setSelectionColor( Error, red );
    document()->setSelectionColor( Step, yellow );
    document()->setInvertSelectionText( Error, FALSE );
    document()->setInvertSelectionText( Step, FALSE );
    document()->addSelection( ParenMatcher::Match );
    document()->addSelection( ParenMatcher::Mismatch );
    document()->setSelectionColor( ParenMatcher::Match, TQColor( 204, 232, 195 ) );
    document()->setSelectionColor( ParenMatcher::Mismatch, TQt::magenta );
    document()->setInvertSelectionText( ParenMatcher::Match, FALSE );
    document()->setInvertSelectionText( ParenMatcher::Mismatch, FALSE );

    accelComment = new TQAccel( this );
    accelComment->connectItem( accelComment->insertItem( ALT + Key_C ),
			       this, SLOT( commentSelection() ) );
    accelUncomment = new TQAccel( this );
    accelUncomment->connectItem( accelUncomment->insertItem( ALT + Key_U ),
				 this, SLOT( uncommentSelection() ) );
    editable = TRUE;
}

Editor::~Editor()
{
    delete cfg;
    delete parenMatcher;
}

void Editor::cursorPosChanged( TQTextCursor *c )
{
    if ( parenMatcher->match( c ) )
	repaintChanged();
    if ( hasError ) {
	emit clearErrorMarker();
	hasError = FALSE;
    }
}

void Editor::load( const TQString &fn )
{
    filename = fn;
    TQFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
	return;
    TQCString txt;
    txt.resize( f.size() );
    f.readBlock( txt.data(), f.size() );
    TQString s( TQString::fromLatin1( txt ) );
    setText( s );
}

void Editor::save( const TQString &fn )
{
    if ( !filename.isEmpty() )
	filename = fn;
}

void Editor::configChanged()
{
    document()->invalidate();
    viewport()->repaint( FALSE );
}

void Editor::setErrorSelection( int line )
{
    TQTextParagraph *p = document()->paragAt( line );
    if ( !p )
	return;
    TQTextCursor c( document() );
    c.setParagraph( p );
    c.setIndex( 0 );
    document()->removeSelection( Error );
    document()->setSelectionStart( Error, c );
    c.gotoLineEnd();
    document()->setSelectionEnd( Error, c );
    hasError = TRUE;
    viewport()->repaint( FALSE );
}

void Editor::setStepSelection( int line )
{
    TQTextParagraph *p = document()->paragAt( line );
    if ( !p )
	return;
    TQTextCursor c( document() );
    c.setParagraph( p );
    c.setIndex( 0 );
    document()->removeSelection( Step );
    document()->setSelectionStart( Step, c );
    c.gotoLineEnd();
    document()->setSelectionEnd( Step, c );
    viewport()->repaint( FALSE );
}

void Editor::clearStepSelection()
{
    document()->removeSelection( Step );
    viewport()->repaint( FALSE );
}

void Editor::doChangeInterval()
{
    emit intervalChanged();
    TQTextEdit::doChangeInterval();
}

void Editor::commentSelection()
{
    TQTextParagraph *start = document()->selectionStartCursor( TQTextDocument::Standard ).paragraph();
    TQTextParagraph *end = document()->selectionEndCursor( TQTextDocument::Standard ).paragraph();
    if ( !start || !end )
	start = end = textCursor()->paragraph();
    while ( start ) {
	if ( start == end && textCursor()->index() == 0 )
	    break;
	start->insert( 0, "//" );
	if ( start == end )
	    break;
	start = start->next();
    }
    document()->removeSelection( TQTextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}

void Editor::uncommentSelection()
{
    TQTextParagraph *start = document()->selectionStartCursor( TQTextDocument::Standard ).paragraph();
    TQTextParagraph *end = document()->selectionEndCursor( TQTextDocument::Standard ).paragraph();
    if ( !start || !end )
	start = end = textCursor()->paragraph();
    while ( start ) {
	if ( start == end && textCursor()->index() == 0 )
	    break;
	while ( start->at( 0 )->c == '/' )
	    start->remove( 0, 1 );
	if ( start == end )
	    break;
	start = start->next();
    }
    document()->removeSelection( TQTextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}

TQPopupMenu *Editor::createPopupMenu( const TQPoint &p )
{
    TQPopupMenu *menu = TQTextEdit::createPopupMenu( p );
    menu->insertSeparator();
    menu->insertItem( tr( "C&omment Code\tAlt+C" ), this, SLOT( commentSelection() ) );
    menu->insertItem( tr( "Unco&mment Code\tAlt+U" ), this, SLOT( uncommentSelection() ) );
    return menu;
}

bool Editor::eventFilter( TQObject *o, TQEvent *e )
{
    if ( ( e->type() == TQEvent::FocusIn || e->type() == TQEvent::FocusOut ) &&
	 ( o == this || o == viewport() ) ) {
	accelUncomment->setEnabled( e->type() == TQEvent::FocusIn );
	accelComment->setEnabled( e->type() == TQEvent::FocusIn );
    }
    return TQTextEdit::eventFilter( o, e );
}

void Editor::doKeyboardAction( KeyboardAction action )
{
    if ( !editable )
	return;
    TQTextEdit::doKeyboardAction( action );
}

void Editor::keyPressEvent( TQKeyEvent *e )
{
    if ( editable ) {
	TQTextEdit::keyPressEvent( e );
	return;
    }

    switch ( e->key() ) {
    case Key_Left:
    case Key_Right:
    case Key_Up:
    case Key_Down:
    case Key_Home:
    case Key_End:
    case Key_Prior:
    case Key_Next:
    case Key_Direction_L:
    case Key_Direction_R:
	TQTextEdit::keyPressEvent( e );
	break;
    default:
	e->accept();
	break;
    }
}
