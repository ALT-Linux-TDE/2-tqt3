/****************************************************************************
**
** Implementation of TQListView widget class
**
** Created : 970809
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the TQt GUI Toolkit.
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
** This file may be used under the terms of the Q Public License as
** defined by Trolltech ASA and appearing in the file LICENSE.TQPL
** included in the packaging of this file.  Licensees holding valid TQt
** Commercial licenses may use this file in accordance with the TQt
** Commercial License Agreement provided with the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include "ntqlistview.h"
#ifndef QT_NO_LISTVIEW
#include "ntqtimer.h"
#include "ntqheader.h"
#include "ntqpainter.h"
#include "ntqcursor.h"
#include "ntqptrstack.h"
#include "ntqptrlist.h"
#include "ntqstrlist.h"
#include "ntqapplication.h"
#include "ntqbitmap.h"
#include "ntqdatetime.h"
#include "ntqptrdict.h"
#include "ntqptrvector.h"
#include "ntqiconset.h"
#include "ntqcleanuphandler.h"
#include "ntqpixmapcache.h"
#include "ntqpopupmenu.h"
#include "ntqtl.h"
#include "ntqdragobject.h"
#include "ntqlineedit.h"
#include "ntqvbox.h"
#include "ntqtooltip.h"
#include "ntqstyle.h"
#include "ntqstylesheet.h"
#include "../kernel/qinternal_p.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "ntqaccessible.h"
#endif

const int Unsorted = 16383;

static TQCleanupHandler<TQBitmap> qlv_cleanup_bitmap;

struct TQListViewItemIteratorPrivate
{
    TQListViewItemIteratorPrivate( uint f ) : flags( f )
	{
	    // nothing yet
	}

    uint flags;
};

static TQPtrDict<TQListViewItemIteratorPrivate> *qt_iteratorprivate_dict = 0;

struct TQListViewPrivate
{
    // classes that are here to avoid polluting the global name space

    // the magical hidden mother of all items
    class Root: public TQListViewItem {
    public:
	Root( TQListView * parent );

	void setHeight( int );
	void invalidateHeight();
	void setup();
	TQListView * theListView() const;

	TQListView * lv;
    };

    // for the stack used in drawContentsOffset()
    class Pending {
    public:
	Pending( int level, int ypos, TQListViewItem * item)
	    : l(level), y(ypos), i(item) {};

	int l; // level of this item; root is -1 or 0
	int y; // level of this item in the tree
	TQListViewItem * i; // the item itself
    };

    // to remember what's on screen
    class DrawableItem {
    public:
	DrawableItem( Pending * pi ) { y = pi->y; l = pi->l; i = pi->i; };
	int y;
	int l;
	TQListViewItem * i;
    };

    // for sorting
    class SortableItem {
    public:
	/*
	  We could be smarter and keep a pointer to the TQListView
	  item instead of numCols, col and asc. This would then allow
	  us to use the physical ordering of columns rather than the
	  logical. Microsoft uses the logical ordering, so there is
	  some virtue in doing so, although it prevents the user from
	  chosing the secondary key.
	*/
	TQListViewItem * item;
	int numCols;
	int col;
	bool asc;

	int cmp( const SortableItem& i ) const {
	    int diff = item->compare( i.item, col, asc );
	    if ( diff == 0 && numCols != 1 ) {
		for ( int j = 0; j < numCols; j++ ) {
		    if ( j != col ) {
			diff = item->compare( i.item, j, asc );
			if ( diff != 0 )
			    break;
		    }
		}
	    }
	    return diff;
	}
	bool operator<( const SortableItem& i ) const { return cmp( i ) < 0; }
	bool operator<=( const SortableItem& i ) const { return cmp( i ) <= 0; }
	bool operator>( const SortableItem& i ) const { return cmp( i ) > 0; }
    };

    class ItemColumnInfo {
    public:
	ItemColumnInfo(): pm( 0 ), next( 0 ), truncated( FALSE ), dirty( FALSE ), allow_rename( FALSE ), width( 0 ) {}
	~ItemColumnInfo() { delete pm; delete next; }
	TQString text, tmpText;
	TQPixmap * pm;
	ItemColumnInfo * next;
	uint truncated : 1;
	uint dirty : 1;
	uint allow_rename : 1;
	int width;
    };

    class ViewColumnInfo {
    public:
	ViewColumnInfo(): align(TQt::AlignAuto), sortable(TRUE), next( 0 ) {}
	~ViewColumnInfo() { delete next; }
	int align;
	bool sortable;
	ViewColumnInfo * next;
    };

    // private variables used in TQListView
    ViewColumnInfo * vci;
    TQHeader * h;
    Root * r;
    uint rootIsExpandable : 1;
    int margin;

    TQListViewItem * focusItem, *highlighted, *oldFocusItem;

    TQTimer * timer;
    TQTimer * dirtyItemTimer;
    TQTimer * visibleTimer;
    int levelWidth;

    // the list of drawables, and the range drawables covers entirely
    // (it may also include a few items above topPixel)
    TQPtrList<DrawableItem> * drawables;
    int topPixel;
    int bottomPixel;

    TQPtrDict<void> * dirtyItems;

    TQListView::SelectionMode selectionMode;

    // Per-column structure for information not in the TQHeader
    struct Column {
	TQListView::WidthMode wmode;
    };
    TQPtrVector<Column> column;

    // suggested height for the items
    int fontMetricsHeight;
    int minLeftBearing, minRightBearing;
    int ellipsisWidth;

    // currently typed prefix for the keyboard interface, and the time
    // of the last key-press
    TQString currentPrefix;
    TQTime currentPrefixTime;

    // holds a list of iterators
    TQPtrList<TQListViewItemIterator> *iterators;
    TQListViewItem *pressedItem, *selectAnchor;

    TQTimer *scrollTimer;
    TQTimer *renameTimer;
    TQTimer *autoopenTimer;

    // sort column and order   #### may need to move to TQHeader [subclass]
    int sortcolumn;
    bool ascending		:1;
    bool sortIndicator		:1;
    // whether to select or deselect during this mouse press.
    bool allColumnsShowFocus	:1;
    bool select			:1;

    // TRUE if the widget should take notice of mouseReleaseEvent
    bool buttonDown		:1;
    // TRUE if the widget should ignore a double-click
    bool ignoreDoubleClick	:1;

    bool clearing		:1;
    bool pressedSelected	:1;
    bool pressedEmptyArea 	:1;

    bool useDoubleBuffer	:1;
    bool toolTips		:1;
    bool fullRepaintOnComlumnChange:1;
    bool updateHeader		:1;

    bool startEdit : 1;
    bool ignoreEditAfterFocus : 1;
    bool inMenuMode :1;

    TQListView::RenameAction defRenameAction;

    TQListViewItem *startDragItem;
    TQPoint dragStartPos;
    TQListViewToolTip *toolTip;
    int pressedColumn;
    TQListView::ResizeMode resizeMode;
};

#ifndef QT_NO_TOOLTIP
class TQListViewToolTip : public TQToolTip
{
public:
    TQListViewToolTip( TQWidget *parent, TQListView *lv );
    virtual ~TQListViewToolTip();

    void maybeTip( const TQPoint &pos );

private:
    TQListView *view;

};

TQListViewToolTip::TQListViewToolTip( TQWidget *parent, TQListView *lv )
    : TQToolTip( parent ), view( lv )
{
}

TQListViewToolTip::~TQListViewToolTip()
{
}

void TQListViewToolTip::maybeTip( const TQPoint &pos )
{
    if ( !parentWidget() || !view || !view->showToolTips() )
	return;

    TQListViewItem *item = view->itemAt( pos );
    TQPoint contentsPos = view->viewportToContents( pos );
    if ( !item || !item->columns )
	return;
    int col = view->header()->sectionAt( contentsPos.x() );
    TQListViewPrivate::ItemColumnInfo *ci = (TQListViewPrivate::ItemColumnInfo*)item->columns;
    for ( int i = 0; ci && ( i < col ); ++i )
	ci = ci->next;

    if ( !ci || !ci->truncated )
	return;

    TQRect r = view->itemRect( item );
    int headerPos = view->header()->sectionPos( col );
    r.setLeft( headerPos );
    r.setRight( headerPos + view->header()->sectionSize( col ) );
    tip( r, TQStyleSheet::escape(item->text( col ) ) );
}
#endif

// these should probably be in TQListViewPrivate, for future thread safety
static bool activatedByClick;

// Holds the position of activation. The position is relative to the top left corner in the row minus the root decoration and its branches/depth.
// Used to track the point clicked for CheckBoxes and RadioButtons.
static TQPoint activatedP;

#if defined(QT_ACCESSIBILITY_SUPPORT)
static int indexOfItem( TQListViewItem *item )
{
    if ( !TQAccessible::isActive() )
	return 0;

    static TQListViewItem *lastItem = 0;
    static int lastIndex = 0;

    if ( !item || !item->listView() )
	return 0;

    if ( item == lastItem )
	return lastIndex;

    lastItem = item;
    int index = 1;

    TQListViewItemIterator it( item->listView() );
    while ( it.current() ) {
	if ( it.current() == item ) {
	    lastIndex = index;
	    return index;
	}
	++it;
	++index;
    }
    lastIndex = 0;
    return 0;
}
#endif

/*!
  \internal
  Creates a string with ... like "Trollte..." or "...olltech" depending on the alignment
*/

static TQString qEllipsisText( const TQString &org, const TQFontMetrics &fm, int width, int align )
{
    int ellWidth = fm.width( "..." );
    TQString text = TQString::fromLatin1("");
    int i = 0;
    int len = org.length();
    int offset = (align & TQt::AlignRight) ? (len-1) - i : i;
    while ( i < len && fm.width( text + org[ offset ] ) + ellWidth < width ) {
	if ( align & TQt::AlignRight )
	    text.prepend( org[ offset ] );
	else
	    text += org[ offset ];
	offset = (align & TQt::AlignRight) ? (len-1) - ++i : ++i;
    }
    if ( text.isEmpty() )
	text = ( align & TQt::AlignRight ) ? org.right( 1 ) : text = org.left( 1 );
    if ( align & TQt::AlignRight )
	text.prepend( "..." );
    else
	text += "...";
    return text;
}

/*!
    \class TQListViewItem
    \brief The TQListViewItem class implements a list view item.

    \ingroup advanced

    A list view item is a multi-column object capable of displaying
    itself in a TQListView.

    The easiest way to use TQListViewItem is to construct one with a
    few constant strings, and either a TQListView or another
    TQListViewItem as parent.
    \code
	(void) new TQListViewItem( listView, "Column 1", "Column 2" );
	(void) new TQListViewItem( listView->firstChild(), "A", "B", "C" );
    \endcode
    We've discarded the pointers to the items since we can still access
    them via their parent \e listView. By default, TQListView sorts its
    items; this can be switched off with TQListView::setSorting(-1).

    The parent must be another TQListViewItem or a TQListView. If the
    parent is a TQListView, the item becomes a top-level item within
    that TQListView. If the parent is another TQListViewItem, the item
    becomes a child of that list view item.

    If you keep the pointer, you can set or change the texts using
    setText(), add pixmaps using setPixmap(), change its mode using
    setSelectable(), setSelected(), setOpen() and setExpandable().
    You'll also be able to change its height using setHeight(), and
    traverse its sub-items. You don't have to keep the pointer since
    you can get a pointer to any TQListViewItem in a TQListView using
    TQListView::selectedItem(), TQListView::currentItem(),
    TQListView::firstChild(), TQListView::lastItem() and
    TQListView::findItem().

    If you call \c delete on a list view item, it will be deleted as
    expected, and as usual for \l{TQObject}s, if it has any child items
    (to any depth), all these will be deleted too.

    \l{TQCheckListItem}s are list view items that have a checkbox or
    radio button and can be used in place of plain TQListViewItems.

    You can traverse the tree as if it were a doubly-linked list using
    itemAbove() and itemBelow(); they return pointers to the items
    directly above and below this item on the screen (even if none of
    them are actually visible at the moment).

    Here's how to traverse all of an item's children (but not its
    children's children, etc.):
    Example:
    \code
	TQListViewItem * myChild = myItem->firstChild();
	while( myChild ) {
	    doSomething( myChild );
	    myChild = myChild->nextSibling();
	}
    \endcode

    If you want to iterate over every item, to any level of depth use
    an iterator. To iterate over the entire tree, initialize the
    iterator with the list view itself; to iterate starting from a
    particular item, initialize the iterator with the item:

    \code
	TQListViewItemIterator it( listview );
	while ( it.current() ) {
	    TQListViewItem *item = it.current();
	    doSomething( item );
	    ++it;
	}
    \endcode

    Note that the order of the children will change when the sorting
    order changes and is undefined if the items are not visible. You
    can, however, call enforceSortOrder() at any time; TQListView will
    always call it before it needs to show an item.

    Many programs will need to reimplement TQListViewItem. The most
    commonly reimplemented functions are:
    \table
    \header \i Function \i Description
    \row \i \l text()
	 \i Returns the text in a column. Many subclasses will compute
	    this on the fly.
    \row \i \l key()
	 \i Used for sorting. The default key() simply calls
	    text(), but judicious use of key() can give you fine
	    control over sorting; for example, TQFileDialog
	    reimplements key() to sort by date.
    \row \i \l setup()
	 \i Called before showing the item and whenever the list
	    view's font changes, for example.
    \row \i \l activate()
	 \i Called whenever the user clicks on the item or presses
	    Space when the item is the current item.
    \endtable

    Some subclasses call setExpandable(TRUE) even when they have no
    children, and populate themselves when setup() or setOpen(TRUE) is
    called. The \c dirview/dirview.cpp example program uses this
    technique to start up quickly: The files and subdirectories in a
    directory aren't inserted into the tree until they're actually
    needed.

    \img qlistviewitems.png List View Items

    \sa TQCheckListItem TQListView
*/

/*!
    \fn int TQCheckListItem::rtti() const

    Returns 1.

    Make your derived classes return their own values for rtti(), and
    you can distinguish between list view items. You should use values
    greater than 1000, to allow for extensions to this class.
*/

/*!
    Constructs a new top-level list view item with no parent
*/

TQListViewItem::TQListViewItem( )
{
    init();
}

/*!
    Constructs a new top-level list view item in the TQListView \a
    parent.
*/

TQListViewItem::TQListViewItem( TQListView * parent )
{
    init();
    parent->insertItem( this );
}


/*!
    Constructs a new list view item that is a child of \a parent and
    first in the parent's list of children.
*/

TQListViewItem::TQListViewItem( TQListViewItem * parent )
{
    init();
    parent->insertItem( this );
}




/*!
    Constructs an empty list view item that is a child of \a parent
    and is after item \a after in the parent's list of children. Since
    \a parent is a TQListView the item will be a top-level item.
*/

TQListViewItem::TQListViewItem( TQListView * parent, TQListViewItem * after )
{
    init();
    parent->insertItem( this );
    moveToJustAfter( after );
}


/*!
    Constructs an empty list view item that is a child of \a parent
    and is after item \a after in the parent's list of children.
*/

TQListViewItem::TQListViewItem( TQListViewItem * parent, TQListViewItem * after )
{
    init();
    parent->insertItem( this );
    moveToJustAfter( after );
}



/*!
    Constructs a new top-level list view item in the TQListView \a
    parent, with up to eight constant strings, \a label1, \a label2, \a
    label3, \a label4, \a label5, \a label6, \a label7 and \a label8
    defining its columns' contents.

    \sa setText()
*/

TQListViewItem::TQListViewItem( TQListView * parent,
			      TQString label1,
			      TQString label2,
			      TQString label3,
			      TQString label4,
			      TQString label5,
			      TQString label6,
			      TQString label7,
			      TQString label8 )
{
    init();
    parent->insertItem( this );

    setText( 0, label1 );
    setText( 1, label2 );
    setText( 2, label3 );
    setText( 3, label4 );
    setText( 4, label5 );
    setText( 5, label6 );
    setText( 6, label7 );
    setText( 7, label8 );
}


/*!
    Constructs a new list view item as a child of the TQListViewItem \a
    parent with up to eight constant strings, \a label1, \a label2, \a
    label3, \a label4, \a label5, \a label6, \a label7 and \a label8
    as columns' contents.

    \sa setText()
*/

TQListViewItem::TQListViewItem( TQListViewItem * parent,
			      TQString label1,
			      TQString label2,
			      TQString label3,
			      TQString label4,
			      TQString label5,
			      TQString label6,
			      TQString label7,
			      TQString label8 )
{
    init();
    parent->insertItem( this );

    setText( 0, label1 );
    setText( 1, label2 );
    setText( 2, label3 );
    setText( 3, label4 );
    setText( 4, label5 );
    setText( 5, label6 );
    setText( 6, label7 );
    setText( 7, label8 );
}

/*!
    Constructs a new list view item in the TQListView \a parent that is
    included after item \a after and that has up to eight column
    texts, \a label1, \a label2, \a label3, \a label4, \a label5, \a
    label6, \a label7 and\a label8.

    Note that the order is changed according to TQListViewItem::key()
    unless the list view's sorting is disabled using
    TQListView::setSorting(-1).

    \sa setText()
*/

TQListViewItem::TQListViewItem( TQListView * parent, TQListViewItem * after,
			      TQString label1,
			      TQString label2,
			      TQString label3,
			      TQString label4,
			      TQString label5,
			      TQString label6,
			      TQString label7,
			      TQString label8 )
{
    init();
    parent->insertItem( this );
    moveToJustAfter( after );

    setText( 0, label1 );
    setText( 1, label2 );
    setText( 2, label3 );
    setText( 3, label4 );
    setText( 4, label5 );
    setText( 5, label6 );
    setText( 6, label7 );
    setText( 7, label8 );
}


/*!
    Constructs a new list view item as a child of the TQListViewItem \a
    parent. It is inserted after item \a after and may contain up to
    eight strings, \a label1, \a label2, \a label3, \a label4, \a
    label5, \a label6, \a label7 and \a label8 as column entries.

    Note that the order is changed according to TQListViewItem::key()
    unless the list view's sorting is disabled using
    TQListView::setSorting(-1).

    \sa setText()
*/

TQListViewItem::TQListViewItem( TQListViewItem * parent, TQListViewItem * after,
			      TQString label1,
			      TQString label2,
			      TQString label3,
			      TQString label4,
			      TQString label5,
			      TQString label6,
			      TQString label7,
			      TQString label8 )
{
    init();
    parent->insertItem( this );
    moveToJustAfter( after );

    setText( 0, label1 );
    setText( 1, label2 );
    setText( 2, label3 );
    setText( 3, label4 );
    setText( 4, label5 );
    setText( 5, label6 );
    setText( 6, label7 );
    setText( 7, label8 );
}

/*!
    Sorts all this item's child items using the current sorting
    configuration (sort column and direction).

    \sa enforceSortOrder()
*/

void TQListViewItem::sort()
{
    if ( !listView() )
	 return;
    lsc = Unsorted;
    enforceSortOrder();
    listView()->triggerUpdate();
}

int TQListViewItem::RTTI = 0;

/*!
    Returns 0.

    Make your derived classes return their own values for rtti(), so
    that you can distinguish between different kinds of list view
    items. You should use values greater than 1000 to allow for
    extensions to this class.
*/

int TQListViewItem::rtti() const
{
    return RTTI;
}

/*
    Performs the initializations that's common to the constructors.
*/

void TQListViewItem::init()
{
    ownHeight = 0;
    maybeTotalHeight = -1;
    open = FALSE;

    nChildren = 0;
    parentItem = 0;
    siblingItem = childItem = 0;

    columns = 0;

    selected = 0;

    lsc = Unsorted;
    lso = TRUE; // unsorted in ascending order :)
    configured = FALSE;
    expandable = FALSE;
    selectable = TRUE;
    is_root = FALSE;
    allow_drag = FALSE;
    allow_drop = FALSE;
    visible = TRUE;
    renameBox = 0;
    enabled = TRUE;
    mlenabled = FALSE;
}

/*!
    If \a b is TRUE, the item is made visible; otherwise it is hidden.

    If the item is not visible, itemAbove() and itemBelow() will never
    return this item, although you still can reach it by using e.g.
    TQListViewItemIterator.
*/

void TQListViewItem::setVisible( bool b )
{
    if ( b == (bool)visible )
	return;
    TQListView *lv = listView();
    if ( !lv )
	return;
    if ( b && parent() && !parent()->isVisible() )
	return;
    visible = b;
    configured = FALSE;
    setHeight( 0 );
    invalidateHeight();
    if ( parent() )
	parent()->invalidateHeight();
    else
	lv->d->r->invalidateHeight();
    for ( TQListViewItem *i = childItem; i; i = i->siblingItem )
	i->setVisible( b );
    if ( lv )
	lv->triggerUpdate();
}

/*!
    Returns TRUE if the item is visible; otherwise returns FALSE.

    \sa setVisible()
*/

bool TQListViewItem::isVisible() const
{
    return (bool)visible;
}

/*!
    If \a b is TRUE, this item can be in-place renamed in the column
    \a col by the user; otherwise it cannot be renamed in-place.
*/

void TQListViewItem::setRenameEnabled( int col, bool b )
{
    TQListViewPrivate::ItemColumnInfo * l = (TQListViewPrivate::ItemColumnInfo*)columns;
    if ( !l ) {
	l = new TQListViewPrivate::ItemColumnInfo;
	columns = (void*)l;
    }
    for( int c = 0; c < col; c++ ) {
	if ( !l->next )
	    l->next = new TQListViewPrivate::ItemColumnInfo;
	l = l->next;
    }

    if ( !l )
	return;
    l->allow_rename = b;
}

/*!
    Returns TRUE if this item can be in-place renamed in column \a
    col; otherwise returns FALSE.
*/

bool TQListViewItem::renameEnabled( int col ) const
{
    TQListViewPrivate::ItemColumnInfo * l = (TQListViewPrivate::ItemColumnInfo*)columns;
    if ( !l )
	return FALSE;

    while( col && l ) {
	l = l->next;
	col--;
    }

    if ( !l )
	return FALSE;
    return (bool)l->allow_rename;
}

/*!
    If \a b is TRUE the item is enabled; otherwise it is disabled.
    Disabled items are drawn differently (e.g. grayed-out) and are not
    accessible by the user.
*/

void TQListViewItem::setEnabled( bool b )
{
    if ( (bool)enabled == b )
	return;
    enabled = b;
    if ( !enabled )
	selected = FALSE;
    TQListView *lv = listView();
    if ( lv ) {
	lv->triggerUpdate();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    TQAccessible::updateAccessibility( lv->viewport(), indexOfItem( this ), TQAccessible::StateChanged );
#endif
    }
}

/*!
    Returns TRUE if this item is enabled; otherwise returns FALSE.

    \sa setEnabled()
*/

bool TQListViewItem::isEnabled() const
{
    return (bool)enabled;
}

/*!
    If in-place renaming of this item is enabled (see
    renameEnabled()), this function starts renaming the item in column
    \a col, by creating and initializing an edit box.
*/

void TQListViewItem::startRename( int col )
{
    if ( !renameEnabled( col ) )
	return;
    if ( renameBox )
	cancelRename( col );
    TQListView *lv = listView();
    if ( !lv )
	return;

    if ( lv->d->renameTimer )
	lv->d->renameTimer->stop();

    lv->ensureItemVisible( this );

    if ( lv->d->timer->isActive() ) {
	// make sure that pending calculations get finished
	lv->d->timer->stop();
	lv->updateContents();
    }

    if ( lv->currentItem() && lv->currentItem()->renameBox ) {
	if ( lv->d->defRenameAction == TQListView::Reject )
	    lv->currentItem()->cancelRename( lv->currentItem()->renameCol );
	else
	    lv->currentItem()->okRename( lv->currentItem()->renameCol );
    }

    if ( this != lv->currentItem() )
	lv->setCurrentItem( this );

    TQRect r = lv->itemRect( this );
    r = TQRect( lv->viewportToContents( r.topLeft() ), r.size() );
    r.setLeft( lv->header()->sectionPos( col ) );
    r.setWidth(TQMIN(lv->header()->sectionSize(col) - 1,
                    lv->contentsX() + lv->visibleWidth() - r.left()));
    if ( col == 0 )
	r.setLeft( r.left() + lv->itemMargin() + ( depth() + ( lv->rootIsDecorated() ? 1 : 0 ) ) * lv->treeStepSize() - 1 );
    if ( pixmap( col ) )
	r.setLeft( r.left() + pixmap( col )->width() );
    if ( r.x() - lv->contentsX() < 0 ) {
	lv->scrollBy( r.x() - lv->contentsX(), 0 );
	r.setX( lv->contentsX() );
    } else if ( ( lv->contentsX() + lv->visibleWidth() ) < ( r.x() + r.width() ) ) {
	lv->scrollBy( ( r.x() + r.width() ) - ( lv->contentsX() + lv->visibleWidth() ), 0 );
    }
    if ( r.width() > lv->visibleWidth() )
	r.setWidth( lv->visibleWidth() );
    renameBox = new TQLineEdit( lv->viewport(), "qt_renamebox" );
    renameBox->setFrameStyle( TQFrame::Box | TQFrame::Plain );
    renameBox->setLineWidth( 1 );
    renameBox->setText( text( col ) );
    renameBox->selectAll();
    renameBox->installEventFilter( lv );
    lv->addChild( renameBox, r.x(), r.y() );
    renameBox->resize( r.size() );
    lv->viewport()->setFocusProxy( renameBox );
    renameBox->setFocus();
    renameBox->show();
    renameCol = col;
}

/*!
    This function removes the rename box.
*/

void TQListViewItem::removeRenameBox()
{
    // Sanity, it should be checked by the functions calling this first anyway
    TQListView *lv = listView();
    if ( !lv || !renameBox )
	return;
    const bool resetFocus = lv->viewport()->focusProxy() == renameBox;
    const bool renameBoxHadFocus = renameBox->hasFocus();
    delete renameBox;
    renameBox = 0;
    if ( resetFocus )
	lv->viewport()->setFocusProxy( lv );
    if (renameBoxHadFocus)
	lv->setFocus();
}

/*!
    This function is called if the user presses Enter during in-place
    renaming of the item in column \a col.

    \sa cancelRename()
*/

void TQListViewItem::okRename( int col )
{
    TQListView *lv = listView();
    if ( !lv || !renameBox )
	return;
    setText( col, renameBox->text() );
    removeRenameBox();

    // we set the parent lsc to Unsorted if that column is the sorted one
    if ( parent() && (int)parent()->lsc == col )
	parent()->lsc = Unsorted;

    emit lv->itemRenamed( this, col );
    emit lv->itemRenamed( this, col, text( col ) );
}

/*!
    This function is called if the user cancels in-place renaming of
    this item in column \a col (e.g. by pressing Esc).

    \sa okRename()
*/

void TQListViewItem::cancelRename( int )
{
    TQListView *lv = listView();
    if ( !lv || !renameBox )
	return;
    removeRenameBox();
}

/*!
    Destroys the item, deleting all its children and freeing up all
    allocated resources.
*/

TQListViewItem::~TQListViewItem()
{
    if ( renameBox ) {
	delete renameBox;
	renameBox = 0;
    }

    TQListView *lv = listView();

    if ( lv ) {
	if ( lv->d->oldFocusItem == this )
	    lv->d->oldFocusItem = 0;
	if ( lv->d->iterators ) {
	    TQListViewItemIterator *i = lv->d->iterators->first();
	    while ( i ) {
		if ( i->current() == this )
		    i->currentRemoved();
		i = lv->d->iterators->next();
	    }
	}
    }

    if ( parentItem )
	parentItem->takeItem( this );
    TQListViewItem * i = childItem;
    childItem = 0;
    while ( i ) {
	i->parentItem = 0;
	TQListViewItem * n = i->siblingItem;
	delete i;
	i = n;
    }
    delete (TQListViewPrivate::ItemColumnInfo *)columns;
}


/*!
    If \a b is TRUE each of the item's columns may contain multiple
    lines of text; otherwise each of them may only contain a single
    line.
*/

void TQListViewItem::setMultiLinesEnabled( bool b )
{
    mlenabled = b;
}

/*!
    Returns TRUE if the item can display multiple lines of text in its
    columns; otherwise returns FALSE.
*/

bool TQListViewItem::multiLinesEnabled() const
{
    return mlenabled;
}

/*!
    If \a allow is TRUE, the list view starts a drag (see
    TQListView::dragObject()) when the user presses and moves the mouse
    on this item.
*/


void TQListViewItem::setDragEnabled( bool allow )
{
    allow_drag = (uint)allow;
}

/*!
    If \a allow is TRUE, the list view accepts drops onto the item;
    otherwise drops are not allowed.
*/

void TQListViewItem::setDropEnabled( bool allow )
{
    allow_drop = (uint)allow;
}

/*!
    Returns TRUE if this item can be dragged; otherwise returns FALSE.

    \sa setDragEnabled()
*/

bool TQListViewItem::dragEnabled() const
{
    return (bool)allow_drag;
}

/*!
    Returns TRUE if this item accepts drops; otherwise returns FALSE.

    \sa setDropEnabled(), acceptDrop()
*/

bool TQListViewItem::dropEnabled() const
{
    return (bool)allow_drop;
}

/*!
    Returns TRUE if the item can accept drops of type TQMimeSource \a
    mime; otherwise returns FALSE.

    The default implementation does nothing and returns FALSE. A
    subclass must reimplement this to accept drops.
*/

bool TQListViewItem::acceptDrop( const TQMimeSource * ) const
{
    return FALSE;
}

#ifndef QT_NO_DRAGANDDROP

/*!
    This function is called when something was dropped on the item. \a e
    contains all the information about the drop.

    The default implementation does nothing, subclasses may need to
    reimplement this function.
*/

void TQListViewItem::dropped( TQDropEvent *e )
{
    Q_UNUSED( e );
}

#endif

/*!
    This function is called when a drag enters the item's bounding
    rectangle.

    The default implementation does nothing, subclasses may need to
    reimplement this function.
*/

void TQListViewItem::dragEntered()
{
}

/*!
    This function is called when a drag leaves the item's bounding
    rectangle.

    The default implementation does nothing, subclasses may need to
    reimplement this function.
*/

void TQListViewItem::dragLeft()
{
}

/*!
    Inserts \a newChild into this list view item's list of children.
    You should not need to call this function; it is called
    automatically by the constructor of \a newChild.

    \warning If you are using \c Single selection mode, then you
    should only insert unselected items.
*/

void TQListViewItem::insertItem( TQListViewItem * newChild )
{
    TQListView *lv = listView();
    if ( lv && lv->currentItem() && lv->currentItem()->renameBox ) {
	if ( lv->d->defRenameAction == TQListView::Reject )
	    lv->currentItem()->cancelRename( lv->currentItem()->renameCol );
	else
	    lv->currentItem()->okRename( lv->currentItem()->renameCol );
    }

    if ( !newChild || newChild->parentItem == this )
	return;
    if ( newChild->parentItem )
	newChild->parentItem->takeItem( newChild );
    if ( open )
	invalidateHeight();
    newChild->siblingItem = childItem;
    childItem = newChild;
    nChildren++;
    newChild->parentItem = this;
    lsc = Unsorted;
    newChild->ownHeight = 0;
    newChild->configured = FALSE;

    if ( lv && !lv->d->focusItem ) {
	lv->d->focusItem = lv->firstChild();
	lv->d->selectAnchor = lv->d->focusItem;
	lv->repaintItem( lv->d->focusItem );
    }
}


/*!
  \fn void TQListViewItem::removeItem( TQListViewItem * )
  \obsolete

  This function has been renamed takeItem().
*/


/*!
    Removes \a item from this object's list of children and causes an
    update of the screen display. The item is not deleted. You should
    not normally need to call this function because
    TQListViewItem::~TQListViewItem() calls it.

    The normal way to delete an item is to use \c delete.

    If you need to move an item from one place in the hierarchy to
    another you can use takeItem() to remove the item from the list
    view and then insertItem() to put the item back in its new
    position.

    If a taken item is part of a selection in \c Single selection
    mode, it is unselected and selectionChanged() is emitted. If a
    taken item is part of a selection in \c Multi or \c Extended
    selection mode, it remains selected.

    \warning This function leaves \a item and its children in a state
    where most member functions are unsafe. Only a few functions work
    correctly on an item in this state, most notably insertItem(). The
    functions that work on taken items are explicitly documented as
    such.

    \sa TQListViewItem::insertItem()
*/

void TQListViewItem::takeItem( TQListViewItem * item )
{
    if ( !item )
	return;

    TQListView *lv = listView();
    if ( lv && lv->currentItem() && lv->currentItem()->renameBox ) {
	if ( lv->d->defRenameAction == TQListView::Reject )
	    lv->currentItem()->cancelRename( lv->currentItem()->renameCol );
	else
	    lv->currentItem()->okRename( lv->currentItem()->renameCol );
    }
    bool emit_changed = FALSE;
    if ( lv && !lv->d->clearing ) {
	if ( lv->d->oldFocusItem == this )
	    lv->d->oldFocusItem = 0;

	if ( lv->d->iterators ) {
	    TQListViewItemIterator *i = lv->d->iterators->first();
	    while ( i ) {
		if ( i->current() == item )
		    i->currentRemoved();
		i = lv->d->iterators->next();
	    }
	}

	invalidateHeight();

	if ( lv->d && lv->d->drawables ) {
	    delete lv->d->drawables;
	    lv->d->drawables = 0;
	}

	if ( lv->d->dirtyItems ) {
	    if ( item->childItem ) {
		delete lv->d->dirtyItems;
		lv->d->dirtyItems = 0;
		lv->d->dirtyItemTimer->stop();
		lv->triggerUpdate();
	    } else {
		lv->d->dirtyItems->take( (void *)item );
	    }
	}

	if ( lv->d->focusItem ) {
	    const TQListViewItem * c = lv->d->focusItem;
	    while( c && c != item )
		c = c->parentItem;
	    if ( c == item ) {
		if ( lv->selectedItem() ) {
		    // for Single, setSelected( FALSE ) when selectedItem() is taken
		    lv->selectedItem()->setSelected( FALSE );
		    // we don't emit selectionChanged( 0 )
		    emit lv->selectionChanged();
		}
		if ( item->nextSibling() )
		    lv->d->focusItem = item->nextSibling();
		else if ( item->itemAbove() )
		    lv->d->focusItem = item->itemAbove();
		else
		    lv->d->focusItem = 0;
		emit_changed = TRUE;
	    }
	}

        // reset anchors etc. if they are set to this or any child
        // items
        const TQListViewItem *ptr = lv->d->selectAnchor;
        while (ptr && ptr != item)
            ptr = ptr->parentItem;
	if (ptr == item)
	    lv->d->selectAnchor = lv->d->focusItem;

        ptr = lv->d->startDragItem;
        while (ptr && ptr != item)
            ptr = ptr->parentItem;
	if (ptr == item)
	    lv->d->startDragItem = 0;

        ptr = lv->d->pressedItem;
        while (ptr && ptr != item)
            ptr = ptr->parentItem;
	if (ptr == item)
	    lv->d->pressedItem = 0;

        ptr = lv->d->highlighted;
        while (ptr && ptr != item)
            ptr = ptr->parentItem;
	if (ptr == item)
	    lv->d->highlighted = 0;
    }

    nChildren--;

    TQListViewItem ** nextChild = &childItem;
    while( nextChild && *nextChild && item != *nextChild )
	nextChild = &((*nextChild)->siblingItem);

    if ( nextChild && item == *nextChild )
	*nextChild = (*nextChild)->siblingItem;
    item->parentItem = 0;
    item->siblingItem = 0;
    item->ownHeight = 0;
    item->maybeTotalHeight = -1;
    item->configured = FALSE;

    if ( emit_changed ) {
	emit lv->currentChanged( lv->d->focusItem );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	TQAccessible::updateAccessibility( lv->viewport(), 0, TQAccessible::Focus );
#endif
    }
}


/*!
    \fn TQString TQListViewItem::key( int column, bool ascending ) const

    Returns a key that can be used for sorting by column \a column.
    The default implementation returns text(). Derived classes may
    also incorporate the order indicated by \a ascending into this
    key, although this is not recommended.

    If you want to sort on non-alphabetical data, e.g. dates, numbers,
    etc., it is more efficient to reimplement compare().

    \sa compare(), sortChildItems()
*/

TQString TQListViewItem::key( int column, bool ) const
{
    return text( column );
}


/*!
    Compares this list view item to \a i using the column \a col in \a
    ascending order. Returns \< 0 if this item is less than \a i, 0 if
    they are equal and \> 0 if this item is greater than \a i.

    This function is used for sorting.

    The default implementation compares the item keys (key()) using
    TQString::localeAwareCompare(). A reimplementation can use
    different values and a different comparison function. Here is a
    reimplementation that uses plain Unicode comparison:

    \code
    int MyListViewItem::compare( TQListViewItem *i, int col,
				 bool ascending ) const
    {
	return key( col, ascending ).compare( i->key( col, ascending) );
    }
    \endcode
    We don't recommend using \a ascending so your code can safely
    ignore it.

    \sa key() TQString::localeAwareCompare() TQString::compare()
*/

int TQListViewItem::compare( TQListViewItem *i, int col, bool ascending ) const
{
    return key( col, ascending ).localeAwareCompare( i->key( col, ascending ) );
}

/*!
    Sorts this item's children using column \a column. This is done in
    ascending order if \a ascending is TRUE and in descending order if
    \a ascending is FALSE.

    Asks some of the children to sort their children. (TQListView and
    TQListViewItem ensure that all on-screen objects are properly
    sorted but may avoid or defer sorting other objects in order to be
    more responsive.)

    \sa key() compare()
*/

void TQListViewItem::sortChildItems( int column, bool ascending )
{
    // we try HARD not to sort.  if we're already sorted, don't.
    if ( column == (int)lsc && ascending == (bool)lso )
	return;

    if ( column < 0 )
	return;

    lsc = column;
    lso = ascending;

    const int nColumns = ( listView() ? listView()->columns() : 0 );

    // only sort if the item have more than one child.
    if ( column > nColumns || childItem == 0 || (childItem->siblingItem == 0 && childItem->childItem == 0))
	return;

    // make an array for qHeapSort()
    TQListViewPrivate::SortableItem * siblings
	= new TQListViewPrivate::SortableItem[nChildren];
    TQListViewItem * s = childItem;
    int i = 0;
    while ( s && i < nChildren ) {
	siblings[i].numCols = nColumns;
	siblings[i].col = column;
	siblings[i].asc = ascending;
	siblings[i].item = s;
	s = s->siblingItem;
	i++;
    }

    // and sort it.
    qHeapSort( siblings, siblings + nChildren );

    // build the linked list of siblings, in the appropriate
    // direction, and finally set this->childItem to the new top
    // child.
    if ( ascending ) {
	for( i = 0; i < nChildren - 1; i++ )
	    siblings[i].item->siblingItem = siblings[i+1].item;
	siblings[nChildren-1].item->siblingItem = 0;
	childItem = siblings[0].item;
    } else {
	for( i = nChildren - 1; i > 0; i-- )
	    siblings[i].item->siblingItem = siblings[i-1].item;
	siblings[0].item->siblingItem = 0;
	childItem = siblings[nChildren-1].item;
    }
    for ( i = 0; i < nChildren; i++ ) {
	if ( siblings[i].item->isOpen() )
	    siblings[i].item->sort();
    }
    delete[] siblings;
}


/*!
    Sets this item's height to \a height pixels. This implicitly
    changes totalHeight(), too.

    Note that a font change causes this height to be overwritten
    unless you reimplement setup().

    For best results in Windows style we suggest using an even number
    of pixels.

    \sa height() totalHeight() isOpen();
*/

void TQListViewItem::setHeight( int height )
{
    if ( ownHeight != height ) {
	if ( visible )
	    ownHeight = height;
	else
	    ownHeight = 0;
	invalidateHeight();
    }
}


/*!
    Invalidates the cached total height of this item, including all
    open children.

    \sa setHeight() height() totalHeight()
*/

void TQListViewItem::invalidateHeight()
{
    if ( maybeTotalHeight < 0 )
	return;
    maybeTotalHeight = -1;
    if ( parentItem && parentItem->isOpen() )
	parentItem->invalidateHeight();
}


/*!
    Opens or closes an item, i.e. shows or hides an item's children.

    If \a o is TRUE all child items are shown initially. The user can
    hide them by clicking the <b>-</b> icon to the left of the item.
    If \a o is FALSE, the children of this item are initially hidden.
    The user can show them by clicking the <b>+</b> icon to the left
    of the item.

    \sa height() totalHeight() isOpen()
*/

void TQListViewItem::setOpen( bool o )
{
    if ( o == (bool)open || !enabled )
	return;
    open = o;

    // If no children to show simply emit signals and return
    if ( !nChildren ) {
        TQListView *lv = listView();
	if ( lv && this != lv->d->r ) {
	    if ( o )
		emit lv->expanded( this );
	    else
		emit lv->collapsed( this );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    TQAccessible::updateAccessibility( lv->viewport(), indexOfItem( this ), TQAccessible::StateChanged );
#endif
	}
	return;
    }
    invalidateHeight();

    if ( !configured ) {
	TQListViewItem * l = this;
	TQPtrStack<TQListViewItem> s;
	while( l ) {
	    if ( l->open && l->childItem ) {
		s.push( l->childItem );
	    } else if ( l->childItem ) {
		// first invisible child is unconfigured
		TQListViewItem * c = l->childItem;
		while( c ) {
		    c->configured = FALSE;
		    c = c->siblingItem;
		}
	    }
	    l->configured = TRUE;
	    l->setup();
	    l = (l == this) ? 0 : l->siblingItem;
	    if ( !l && !s.isEmpty() )
		l = s.pop();
	}
    }

    TQListView *lv = listView();

    if ( open && lv)
	enforceSortOrder();

    if ( isVisible() && lv && lv->d && lv->d->drawables ) {
	lv->d->drawables->clear();
	lv->buildDrawableList();
    }

    if ( lv && this != lv->d->r ) {
	if ( o )
	    emit lv->expanded( this );
	else
	    emit lv->collapsed( this );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	TQAccessible::updateAccessibility( lv->viewport(), indexOfItem( this ), TQAccessible::StateChanged );
#endif
    }
}


/*!
    This virtual function is called before the first time TQListView
    needs to know the height or any other graphical attribute of this
    object, and whenever the font, GUI style, or colors of the list
    view change.

    The default calls widthChanged() and sets the item's height to the
    height of a single line of text in the list view's font. (If you
    use icons, multi-line text, etc., you will probably need to call
    setHeight() yourself or reimplement it.)
*/

void TQListViewItem::setup()
{
    widthChanged();
    TQListView *lv = listView();

    int ph = 0;
    int h = 0;
    if ( lv ) {
	for ( uint i = 0; i < lv->d->column.size(); ++i ) {
	    if ( pixmap( i ) )
		ph = TQMAX( ph, pixmap( i )->height() );
	}

	if ( mlenabled ) {
	    h = ph;
	    for ( int c = 0; c < lv->columns(); ++c ) {
		int lines = text( c ).contains( TQChar('\n') ) + 1;
		int tmph = lv->d->fontMetricsHeight
			   + lv->fontMetrics().lineSpacing() * ( lines - 1 );
		h = TQMAX( h, tmph );
	    }
	    h += 2*lv->itemMargin();
	} else {
	    h = TQMAX( lv->d->fontMetricsHeight, ph ) + 2*lv->itemMargin();
	}
    }

    h = TQMAX( h, TQApplication::globalStrut().height());

    if ( h % 2 > 0 )
	h++;
    setHeight( h );
}




/*!
    This virtual function is called whenever the user presses the mouse
    on this item or presses Space on it.

    \sa activatedPos()
*/

void TQListViewItem::activate()
{
}


/*!
    When called from a reimplementation of activate(), this function
    gives information on how the item was activated. Otherwise the
    behavior is undefined.

    If activate() was caused by a mouse press, the function sets \a
    pos to where the user clicked and returns TRUE; otherwise it
    returns FALSE and does not change \a pos.

    \a pos is relative to the top-left corner of this item.

    \warning We recommend that you ignore this function; it is
    scheduled to become obsolete.

    \sa activate()
*/

bool TQListViewItem::activatedPos( TQPoint &pos )
{
    if ( activatedByClick )
	pos = activatedP;
    return activatedByClick;
}


/*!
    \fn bool TQListViewItem::isSelectable() const

    Returns TRUE if the item is selectable (as it is by default);
    otherwise returns FALSE

    \sa setSelectable()
*/


/*!
    Sets this item to be selectable if \a enable is TRUE (the
    default) or not to be selectable if \a enable is FALSE.

    The user is not able to select a non-selectable item using either
    the keyboard or the mouse. This also applies for the application
    programmer (e.g. setSelected() respects this value).

    \sa isSelectable()
*/

void TQListViewItem::setSelectable( bool enable )
{
    selectable = enable;
}


/*!
    \fn bool TQListViewItem::isExpandable() const

    Returns TRUE if this item is expandable even when it has no
    children; otherwise returns FALSE.
*/

/*!
    Sets this item to be expandable even if it has no children if \a
    enable is TRUE, and to be expandable only if it has children if \a
    enable is FALSE (the default).

    The dirview example uses this in the canonical fashion. It checks
    whether the directory is empty in setup() and calls
    setExpandable(TRUE) if not; in setOpen() it reads the contents of
    the directory and inserts items accordingly. This strategy means
    that dirview can display the entire file system without reading
    very much at startup.

    Note that root items are not expandable by the user unless
    TQListView::setRootIsDecorated() is set to TRUE.

    \sa setSelectable()
*/

void TQListViewItem::setExpandable( bool enable )
{
    expandable = enable;
}


/*!
    Makes sure that this object's children are sorted appropriately.

    This only works if every item from the root item down to this item
    is already sorted.

    \sa sortChildItems()
*/

void TQListViewItem::enforceSortOrder() const
{
    TQListView *lv = listView();
    if ( !lv || (lv && (lv->d->clearing || lv->d->sortcolumn == Unsorted)) )
	return;
    if ( parentItem &&
	 (parentItem->lsc != lsc || parentItem->lso != lso) )
	((TQListViewItem *)this)->sortChildItems( (int)parentItem->lsc,
						 (bool)parentItem->lso );
    else if ( !parentItem &&
	      ( (int)lsc != lv->d->sortcolumn || (bool)lso != lv->d->ascending ) )
	((TQListViewItem *)this)->sortChildItems( lv->d->sortcolumn, lv->d->ascending );
}


/*!
    \fn bool TQListViewItem::isSelected() const

    Returns TRUE if this item is selected; otherwise returns FALSE.

    \sa setSelected() TQListView::setSelected() TQListView::selectionChanged()
*/


/*!
    If \a s is TRUE this item is selected; otherwise it is deselected.

    This function does not maintain any invariants or repaint anything
    -- TQListView::setSelected() does that.

    \sa height() totalHeight()
*/

void TQListViewItem::setSelected( bool s )
{
    bool old = selected;

    TQListView *lv = listView();
    if ( lv && lv->selectionMode() != TQListView::NoSelection) {
	if ( s && isSelectable() )
	    selected = TRUE;
	else
	    selected = FALSE;

#if defined(QT_ACCESSIBILITY_SUPPORT)
	if ( old != (bool)selected ) {
	    int ind = indexOfItem( this );
	    TQAccessible::updateAccessibility( lv->viewport(), ind, TQAccessible::StateChanged );
	    TQAccessible::updateAccessibility( lv->viewport(), ind, selected ? TQAccessible::SelectionAdd : TQAccessible::SelectionRemove );
	}
#else
	Q_UNUSED( old );
#endif
    }
}

/*!
    Returns the total height of this object, including any visible
    children. This height is recomputed lazily and cached for as long
    as possible.

    Functions which can affect the total height are, setHeight() which
    is used to set an item's height, setOpen() to show or hide an
    item's children, and invalidateHeight() to invalidate the cached
    height.

    \sa height()
*/

int TQListViewItem::totalHeight() const
{
    if ( !visible )
	return 0;
    if ( maybeTotalHeight >= 0 )
	return maybeTotalHeight;
    TQListViewItem * that = (TQListViewItem *)this;
    if ( !that->configured ) {
	that->configured = TRUE;
	that->setup(); // ### virtual non-const function called in const
    }
    that->maybeTotalHeight = that->ownHeight;

    if ( !that->isOpen() || !that->childCount() )
	return that->ownHeight;

    TQListViewItem * child = that->childItem;
    while ( child != 0 ) {
	that->maybeTotalHeight += child->totalHeight();
	child = child->siblingItem;
    }
    return that->maybeTotalHeight;
}


/*!
    Returns the text in column \a column, or TQString::null if there is
    no text in that column.

    \sa key() paintCell()
*/

TQString TQListViewItem::text( int column ) const
{
    TQListViewPrivate::ItemColumnInfo * l
	= (TQListViewPrivate::ItemColumnInfo*) columns;

    while( column && l ) {
	l = l->next;
	column--;
    }

    return l ? l->text : TQString::null;
}


/*!
    Sets the text in column \a column to \a text, if \a column is a
    valid column number and \a text is different from the existing
    text.

    If \a text() has been reimplemented, this function may be a no-op.

    \sa text() key()
*/

void TQListViewItem::setText( int column, const TQString &text )
{
    if ( column < 0 )
	return;

    TQListViewPrivate::ItemColumnInfo * l
	= (TQListViewPrivate::ItemColumnInfo*) columns;
    if ( !l ) {
	l = new TQListViewPrivate::ItemColumnInfo;
	columns = (void*)l;
    }
    for( int c = 0; c < column; c++ ) {
	if ( !l->next )
	    l->next = new TQListViewPrivate::ItemColumnInfo;
	l = l->next;
    }
    if ( l->text == text )
	return;

    int oldLc = 0;
    int newLc = 0;
    if ( mlenabled ) {
	if ( !l->text.isEmpty() )
	    oldLc = l->text.contains( TQChar( '\n' ) ) + 1;
	if ( !text.isEmpty() )
	    newLc = text.contains( TQChar( '\n' ) ) + 1;
    }

    l->dirty = TRUE;
    l->text = text;
    if ( column == (int)lsc )
	lsc = Unsorted;

    if ( mlenabled && oldLc != newLc )
	setup();
    else
	widthChanged( column );

    TQListView * lv = listView();
    if ( lv ) {
	lv->d->useDoubleBuffer = TRUE;
	lv->triggerUpdate();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	if ( lv->isVisible() )
	    TQAccessible::updateAccessibility( lv->viewport(), indexOfItem(this), TQAccessible::NameChanged );
#endif
    }
}


/*!
    Sets the pixmap in column \a column to \a pm, if \a pm is non-null
    and different from the current pixmap, and if \a column is
    non-negative.

    \sa pixmap() setText()
*/

void TQListViewItem::setPixmap( int column, const TQPixmap & pm )
{
    if ( column < 0 )
	return;

    int oldW = 0;
    int oldH = 0;
    if ( pixmap( column ) ) {
	oldW = pixmap( column )->width();
	oldH = pixmap( column )->height();
    }

    TQListViewPrivate::ItemColumnInfo * l
	= (TQListViewPrivate::ItemColumnInfo*) columns;
    if ( !l ) {
	l = new TQListViewPrivate::ItemColumnInfo;
	columns = (void*)l;
    }

    for( int c = 0; c < column; c++ ) {
	if ( !l->next )
	    l->next = new TQListViewPrivate::ItemColumnInfo;
	l = l->next;
    }

    if ( ( pm.isNull() && ( !l->pm || l->pm->isNull() ) ) ||
	 ( l->pm && pm.serialNumber() == l->pm->serialNumber() ) )
	return;

    if ( pm.isNull() ) {
	delete l->pm;
	l->pm = 0;
    } else {
	if ( l->pm )
	    *(l->pm) = pm;
	else
	    l->pm = new TQPixmap( pm );
    }

    int newW = 0;
    int newH = 0;
    if ( pixmap( column ) ) {
	newW = pixmap( column )->width();
	newH = pixmap( column )->height();
    }

    if ( oldW != newW || oldH != newH ) {
	setup();
	widthChanged( column );
	invalidateHeight();
    }
    TQListView *lv = listView();
    if ( lv ) {
	lv->d->useDoubleBuffer = TRUE;
	lv->triggerUpdate();
    }
}


/*!
    Returns the pixmap for \a column, or 0 if there is no pixmap for
    \a column.

    \sa setText() setPixmap()
*/

const TQPixmap * TQListViewItem::pixmap( int column ) const
{
    TQListViewPrivate::ItemColumnInfo * l
    = (TQListViewPrivate::ItemColumnInfo*) columns;

    while( column && l ) {
	l = l->next;
	column--;
    }

    return (l && l->pm) ? l->pm : 0;
}


/*!
    This virtual function paints the contents of one column of an item
    and aligns it as described by \a align.

    \a p is a TQPainter open on the relevant paint device. \a p is
    translated so (0, 0) is the top-left pixel in the cell and \a
    width-1, height()-1 is the bottom-right pixel \e in the cell. The
    other properties of \a p (pen, brush, etc) are undefined. \a cg is
    the color group to use. \a column is the logical column number
    within the item that is to be painted; 0 is the column which may
    contain a tree.

    This function may use TQListView::itemMargin() for readability
    spacing on the left and right sides of data such as text, and
    should honor isSelected() and TQListView::allColumnsShowFocus().

    If you reimplement this function, you should also reimplement
    width().

    The rectangle to be painted is in an undefined state when this
    function is called, so you \e must draw on all the pixels. The
    painter \a p has the right font on entry.

    \sa paintBranches(), TQListView::drawContentsOffset()
*/

void TQListViewItem::paintCell( TQPainter * p, const TQColorGroup & cg,
			       int column, int width, int align )
{
    // Change width() if you change this.

    if ( !p )
	return;

    TQListView *lv = listView();
    if ( !lv )
	return;
    TQFontMetrics fm( p->fontMetrics() );

    // had, but we _need_ the column info for the ellipsis thingy!!!
    if ( !columns ) {
	for ( uint i = 0; i < lv->d->column.size(); ++i ) {
	    setText( i, text( i ) );
	}
    }

    TQString t = text( column );

    if ( columns ) {
	TQListViewPrivate::ItemColumnInfo *ci = 0;
	// try until we have a column info....
	while ( !ci ) {
	    ci = (TQListViewPrivate::ItemColumnInfo*)columns;
	    for ( int i = 0; ci && (i < column); ++i )
		ci = ci->next;

	    if ( !ci ) {
		setText( column, t );
		ci = 0;
	    }
	}

	// if the column width changed and this item was not painted since this change
	if ( ci && ( ci->width != width || ci->text != t || ci->dirty ) ) {
	    ci->text = t;
	    ci->dirty = FALSE;
	    ci->width = width;
	    ci->truncated = FALSE;
	    // if we have to do the ellipsis thingy calc the truncated text
	    int pw = lv->itemMargin()*2 - lv->d->minLeftBearing - lv->d->minRightBearing;
	    pw += pixmap( column ) ? pixmap( column )->width() + lv->itemMargin() : 0;
	    if ( !mlenabled && fm.width( t ) + pw > width ) {
		// take care of arabic shaping in width calculation (lars)
		ci->truncated = TRUE;
		ci->tmpText = qEllipsisText( t, fm, width - pw, align );
	    } else if ( mlenabled && fm.width( t ) + pw > width ) {
#ifndef QT_NO_STRINGLIST
		TQStringList list = TQStringList::split( TQChar('\n'), t, TRUE );
		for ( TQStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
		    TQString z = *it;
		    if ( fm.width( z ) + pw > width ) {
			ci->truncated = TRUE;
			*it = qEllipsisText( z, fm, width - pw, align );
		    }
		}

		if ( ci->truncated )
		    ci->tmpText = list.join( TQString("\n") );
#endif
	    }
	}

	// if we have to draw the ellipsis thingy, use the truncated text
	if ( ci && ci->truncated )
	    t = ci->tmpText;
    }

    int marg = lv->itemMargin();
    int r = marg;
    const TQPixmap * icon = pixmap( column );

    const BackgroundMode bgmode = lv->viewport()->backgroundMode();
    const TQColorGroup::ColorRole crole = TQPalette::backgroundRoleFromMode( bgmode );
    if ( cg.brush( crole ) != lv->colorGroup().brush( crole ) )
	p->fillRect( 0, 0, width, height(), cg.brush( crole ) );
    else
	lv->paintEmptyArea( p, TQRect( 0, 0, width, height() ) );


    // (lars) what does this do???
#if 0 // RS: ####
    if ( align != AlignLeft )
	marg -= lv->d->minRightBearing;
#endif
    if ( isSelected() &&
	 (column == 0 || lv->allColumnsShowFocus()) ) {
	p->fillRect( r - marg, 0, width - r + marg, height(),
		     cg.brush( TQColorGroup::Highlight ) );
	if ( enabled || !lv )
	    p->setPen( cg.highlightedText() );
	else if ( !enabled && lv)
	    p->setPen( lv->palette().disabled().highlightedText() );
    } else {
	if ( enabled || !lv )
	    p->setPen( cg.text() );
	else if ( !enabled && lv)
	    p->setPen( lv->palette().disabled().text() );
    }


#if 0
    bool reverse = TQApplication::reverseLayout();
#else
    bool reverse = FALSE;
#endif
    int iconWidth = 0;

    if ( icon ) {
        iconWidth = icon->width() + lv->itemMargin();
	int xo = r;
	// we default to AlignVCenter.
	int yo = ( height() - icon->height() ) / 2;

	// I guess we may as well always respect vertical alignment.
        if ( align & AlignBottom )
    	    yo = height() - icon->height();
	else if ( align & AlignTop )
	    yo = 0;

	// respect horizontal alignment when there is no text for an item.
	if ( text(column).isEmpty() ) {
	    if ( align & AlignRight )
		xo = width - 2 * marg - iconWidth;
	    else if ( align & AlignHCenter )
		xo = ( width - iconWidth ) / 2;
	}
	if ( reverse )
		xo = width - 2 * marg - iconWidth;
	p->drawPixmap( xo, yo, *icon );
    }

    if ( !t.isEmpty() ) {
	if ( !mlenabled ) {
	    if ( !(align & AlignTop || align & AlignBottom) )
		align |= AlignVCenter;
	} else {
	    if ( !(align & AlignVCenter || align & AlignBottom) )
		align |= AlignTop;
	}
	if ( !reverse )
	    r += iconWidth;

	if ( !mlenabled ) {
	    p->drawText( r, 0, width-marg-r, height(), align, t );
	} else {
	    p->drawText( r, marg, width-marg-r, height(), align, t );
	}
    }

    if ( mlenabled && column == 0 && isOpen() && childCount() ) {
	int textheight = fm.size( align, t ).height() + 2 * lv->itemMargin();
	textheight = TQMAX( textheight, TQApplication::globalStrut().height() );
	if ( textheight % 2 > 0 )
	    textheight++;
	if ( textheight < height() ) {
	    int w = lv->treeStepSize() / 2;
	    lv->style().drawComplexControl( TQStyle::CC_ListView, p, lv,
					    TQRect( 0, textheight, w + 1, height() - textheight + 1 ), cg,
					    lv->isEnabled() ? TQStyle::Style_Enabled : TQStyle::Style_Default,
					    TQStyle::SC_ListViewExpand,
					    (uint)TQStyle::SC_All, TQStyleOption( this ) );
	}
    }
}

/*!
    Returns the number of pixels of width required to draw column \a c
    of list view \a lv, using the metrics \a fm without cropping. The
    list view containing this item may use this information depending
    on the TQListView::WidthMode settings for the column.

    The default implementation returns the width of the bounding
    rectangle of the text of column \a c.

    \sa listView() widthChanged() TQListView::setColumnWidthMode()
    TQListView::itemMargin()
*/
int TQListViewItem::width( const TQFontMetrics& fm,
			  const TQListView* lv, int c ) const
{
    int w;
    if ( mlenabled )
	w = fm.size( AlignVCenter, text( c ) ).width() + lv->itemMargin() * 2
	    - lv->d->minLeftBearing - lv->d->minRightBearing;
    else
	w = fm.width( text( c ) ) + lv->itemMargin() * 2
	    - lv->d->minLeftBearing - lv->d->minRightBearing;
    const TQPixmap * pm = pixmap( c );
    if ( pm )
	w += pm->width() + lv->itemMargin(); // ### correct margin stuff?
    return TQMAX( w, TQApplication::globalStrut().width() );
}


/*!
    Paints a focus indicator on the rectangle \a r using painter \a p
    and colors \a cg.

    \a p is already clipped.

    \sa paintCell() paintBranches() TQListView::setAllColumnsShowFocus()
*/

void TQListViewItem::paintFocus( TQPainter *p, const TQColorGroup &cg,
				const TQRect & r )
{
    TQListView *lv = listView();
    if ( lv )
	lv->style().drawPrimitive( TQStyle::PE_FocusRect, p, r, cg,
				   (isSelected() ?
				    TQStyle::Style_FocusAtBorder :
				    TQStyle::Style_Default),
				   TQStyleOption(isSelected() ? cg.highlight() : cg.base() ));
}


/*!
    Paints a set of branches from this item to (some of) its children.

    Painter \a p is set up with clipping and translation so that you
    can only draw in the rectangle that needs redrawing; \a cg is the
    color group to use; the update rectangle is at (0, 0) and has size
    width \a w by height \a h. The top of the rectangle you own is at
    \a y (which is never greater than 0 but can be outside the window
    system's allowed coordinate range).

    The update rectangle is in an undefined state when this function
    is called; this function must draw on \e all of the pixels.

    \sa paintCell(), TQListView::drawContentsOffset()
*/

void TQListViewItem::paintBranches( TQPainter * p, const TQColorGroup & cg,
				   int w, int y, int h )
{
    TQListView *lv = listView();
    if ( lv )
	lv->paintEmptyArea( p, TQRect( 0, 0, w, h ) );
    if ( !visible || !lv )
	return;
    lv->style().drawComplexControl( TQStyle::CC_ListView, p, lv,
				    TQRect( 0, y, w, h ),
				    cg,
				    lv->isEnabled() ? TQStyle::Style_Enabled :
				    TQStyle::Style_Default,
				    (TQStyle::SC_ListViewBranch |
				     TQStyle::SC_ListViewExpand),
				    TQStyle::SC_None, TQStyleOption(this) );
}


TQListViewPrivate::Root::Root( TQListView * parent )
    : TQListViewItem( parent )
{
    lv = parent;
    setHeight( 0 );
    setOpen( TRUE );
}


void TQListViewPrivate::Root::setHeight( int )
{
    TQListViewItem::setHeight( 0 );
}


void TQListViewPrivate::Root::invalidateHeight()
{
    TQListViewItem::invalidateHeight();
    lv->triggerUpdate();
}


TQListView * TQListViewPrivate::Root::theListView() const
{
    return lv;
}


void TQListViewPrivate::Root::setup()
{
    // explicitly nothing
}



/*!
\internal
If called after a mouse click, tells the list view to ignore a
following double click. This state is reset after the next mouse click.
*/

void TQListViewItem::ignoreDoubleClick()
{
    TQListView *lv = listView();
    if ( lv )
	lv->d->ignoreDoubleClick = TRUE;
}



/*!
    \fn void  TQListView::onItem( TQListViewItem *i )

    This signal is emitted when the user moves the mouse cursor onto
    item \a i, similar to the TQWidget::enterEvent() function.
*/

// ### bug here too? see qiconview.cppp onItem/onViewport

/*!
    \fn void  TQListView::onViewport()

    This signal is emitted when the user moves the mouse cursor from
    an item to an empty part of the list view.
*/

/*!
    \enum TQListView::SelectionMode

    This enumerated type is used by TQListView to indicate how it
    reacts to selection by the user.

    \value Single  When the user selects an item, any already-selected
    item becomes unselected. The user can unselect the selected
    item by clicking on some empty space within the view.

    \value Multi  When the user selects an item in the usual way, the
    selection status of that item is toggled and the other items are
    left alone. Also, multiple items can be selected by dragging the
    mouse while the left mouse button stays pressed.

    \value Extended When the user selects an item in the usual way,
    the selection is cleared and the new item selected. However, if
    the user presses the Ctrl key when clicking on an item, the
    clicked item gets toggled and all other items are left untouched.
    And if the user presses the Shift key while clicking on an item,
    all items between the current item and the clicked item get
    selected or unselected, depending on the state of the clicked
    item. Also, multiple items can be selected by dragging the mouse
    over them.

    \value NoSelection  Items cannot be selected.

    In other words, \c Single is a real single-selection list view, \c
    Multi a real multi-selection list view, \c Extended is a list view
    where users can select multiple items but usually want to select
    either just one or a range of contiguous items, and \c NoSelection
    is a list view where the user can look but not touch.
*/

/*!
    \enum TQListView::ResizeMode

    This enum describes how the list view's header adjusts to resize
    events which affect the width of the list view.

    \value NoColumn The columns do not get resized in resize events.

    \value AllColumns All columns are resized equally to fit the width
    of the list view.

    \value LastColumn The last column is resized to fit the width of
    the list view.
*/

/*!
    \enum TQListView::RenameAction

    This enum describes whether a rename operation is accepted if the
    rename editor loses focus without the user pressing Enter.

    \value Accept Rename if Enter is pressed or focus is lost.

    \value Reject Discard the rename operation if focus is lost (and
    Enter has not been pressed).
*/

/*!
    \class TQListView
    \brief The TQListView class implements a list/tree view.

    \ingroup advanced
    \mainclass

    It can display and control a hierarchy of multi-column items, and
    provides the ability to add new items at any time. The user may
    select one or many items (depending on the \c SelectionMode) and
    sort the list in increasing or decreasing order by any column.

    The simplest pattern of use is to create a TQListView, add some
    column headers using addColumn() and create one or more
    TQListViewItem or TQCheckListItem objects with the TQListView as
    parent:

    \quotefile xml/tagreader-with-features/structureparser.h
    \skipto TQListView * table
    \printline
    \quotefile xml/tagreader-with-features/structureparser.cpp
    \skipto addColumn
    \printline addColumn
    \printline
    \skipto new TQListViewItem( table
    \printline

    Further nodes can be added to the list view object (the root of the
    tree) or as child nodes to TQListViewItems:

    \skipto for ( int i = 0 ; i < attributes.length();
    \printuntil }

    (From \link xml/tagreader-with-features/structureparser.cpp
    xml/tagreader-with-features/structureparser.cpp\endlink)

    The main setup functions are:
    \table
    \header \i Function \i Action
    \row \i \l addColumn()
	 \i Adds a column with a text label and perhaps width. Columns
	    are counted from the left starting with column 0.
    \row \i \l setColumnWidthMode()
	 \i Sets the column to be resized automatically or not.
    \row \i \l setAllColumnsShowFocus()
	 \i Sets whether items should show keyboard focus using all
	    columns or just column 0. The default is to show focus
	    just using column 0.
    \row \i \l setRootIsDecorated()
	 \i Sets whether root items should show open/close decoration to their left.
	    The default is FALSE.
    \row \i \l setTreeStepSize()
	 \i Sets how many pixels an item's children are indented
	    relative to their parent. The default is 20. This is
	    mostly a matter of taste.
    \row \i \l setSorting()
	 \i Sets whether the items should be sorted, whether it should
	    be in ascending or descending order, and by what column
	    they should be sorted. By default the list view is sorted
	    by the first column; to switch this off call setSorting(-1).
    \endtable

    To handle events such as mouse presses on the list view, derived
    classes can reimplement the TQScrollView functions:
    \link TQScrollView::contentsMousePressEvent() contentsMousePressEvent\endlink,
    \link TQScrollView::contentsMouseReleaseEvent() contentsMouseReleaseEvent\endlink,
    \link TQScrollView::contentsMouseDoubleClickEvent() contentsMouseDoubleClickEvent\endlink,
    \link TQScrollView::contentsMouseMoveEvent() contentsMouseMoveEvent\endlink,
    \link TQScrollView::contentsDragEnterEvent() contentsDragEnterEvent\endlink,
    \link TQScrollView::contentsDragMoveEvent() contentsDragMoveEvent\endlink,
    \link TQScrollView::contentsDragLeaveEvent() contentsDragLeaveEvent\endlink,
    \link TQScrollView::contentsDropEvent() contentsDropEvent\endlink, and
    \link TQScrollView::contentsWheelEvent() contentsWheelEvent\endlink.

    There are also several functions for mapping between items and
    coordinates. itemAt() returns the item at a position on-screen,
    itemRect() returns the rectangle an item occupies on the screen,
    and itemPos() returns the position of any item (whether it is
    on-screen or not). firstChild() returns the list view's first item
    (not necessarily visible on-screen).

    You can iterate over visible items using
    TQListViewItem::itemBelow(); over a list view's top-level items
    using TQListViewItem::firstChild() and
    TQListViewItem::nextSibling(); or every item using a
    TQListViewItemIterator. See
    the TQListViewItem documentation for examples of traversal.

    An item can be moved amongst its siblings using
    TQListViewItem::moveItem(). To move an item in the hierarchy use
    takeItem() and insertItem(). Item's (and all their child items)
    are deleted with \c delete; to delete all the list view's items
    use clear().

    There are a variety of selection modes described in the
    TQListView::SelectionMode documentation. The default is \c Single
    selection, which you can change using setSelectionMode().

    Because TQListView offers multiple selection it must display
    keyboard focus and selection state separately. Therefore there are
    functions both to set the selection state of an item
    (setSelected()) and to set which item displays keyboard focus
    (setCurrentItem()).

    TQListView emits two groups of signals; one group signals changes
    in selection/focus state and one indicates selection. The first
    group consists of selectionChanged() (applicable to all list
    views), selectionChanged(TQListViewItem*) (applicable only to a
    \c Single selection list view), and currentChanged(TQListViewItem*).
    The second group consists of doubleClicked(TQListViewItem*),
    returnPressed(TQListViewItem*),
    rightButtonClicked(TQListViewItem*, const TQPoint&, int), etc.

    Note that changing the state of the list view in a slot connected
    to a list view signal may cause unexpected side effects. If you
    need to change the list view's state in response to a signal, use
    a \link TQTimer::singleShot() single shot timer\endlink with a
    time out of 0, and connect this timer to a slot that modifies the
    list view's state.

    In Motif style, TQListView deviates fairly strongly from the look
    and feel of the Motif hierarchical tree view. This is done mostly
    to provide a usable keyboard interface and to make the list view
    look better with a white background.

    If selectionMode() is \c Single (the default) the user can select
    one item at a time, e.g. by clicking an item with the mouse, see
    \l TQListView::SelectionMode for details.

    The list view can be navigated either using the mouse or the
    keyboard. Clicking a <b>-</b> icon closes an item (hides its
    children) and clicking a <b>+</b> icon opens an item (shows its
    children). The keyboard controls are these:
    \table
    \header \i Keypress \i Action
    \row \i Home
	 \i Make the first item current and visible.
    \row \i End
	 \i Make the last item current and visible.
    \row \i Page Up
	 \i Make the item above the top visible item current and visible.
    \row \i Page Down
	 \i Make the item below the bottom visible item current and visible.
    \row \i Up Arrow
	 \i Make the item above the current item current and visible.
    \row \i Down Arrow
	 \i Make the item below the current item current and visible.
    \row \i Left Arrow
	 \i If the current item is closed (<b>+</b> icon) or has no
	    children, make its parent item current and visible. If the
	    current item is open (<b>-</b> icon) close it, i.e. hide its
	    children. Exception: if the current item is the first item
	    and is closed and the horizontal scrollbar is offset to
	    the right the list view will be scrolled left.
    \row \i Right Arrow
	 \i If the current item is closed (<b>+</b> icon) and has
	    children, the item is opened. If the current item is
	    opened (<b>-</b> icon) and has children the item's first
	    child is made current and visible. If the current item has
	    no children the list view is scrolled right.
    \endtable

    If the user starts typing letters with the focus in the list view
    an incremental search will occur. For example if the user types
    'd' the current item will change to the first item that begins
    with the letter 'd'; if they then type 'a', the current item will
    change to the first item that begins with 'da', and so on. If no
    item begins with the letters they type the current item doesn't
    change.

    \warning The list view assumes ownership of all list view items
    and will delete them when it does not need them any more.

    <img src=qlistview-m.png> <img src=qlistview-w.png>

    \sa TQListViewItem TQCheckListItem
*/

/*!
    \fn void TQListView::itemRenamed( TQListViewItem * item, int col )

    \overload

    This signal is emitted when \a item has been renamed, e.g. by
    in-place renaming, in column \a col.

    \sa TQListViewItem::setRenameEnabled()
*/

/*!
    \fn void TQListView::itemRenamed( TQListViewItem * item, int col, const TQString &text)

    This signal is emitted when \a item has been renamed to \a text,
    e.g. by in in-place renaming, in column \a col.

    \sa TQListViewItem::setRenameEnabled()
*/

/*!
    Constructs a new empty list view called \a name with parent \a
    parent.

    Performance is boosted by modifying the widget flags \a f so that
    only part of the TQListViewItem children is redrawn. This may be
    unsuitable for custom TQListViewItem classes, in which case \c
    WStaticContents and \c WNoAutoErase should be cleared.

    \sa TQWidget::clearWFlags() TQt::WidgetFlags
*/
TQListView::TQListView( TQWidget * parent, const char *name, WFlags f )
    : TQScrollView( parent, name, f | WStaticContents | WNoAutoErase )
{
    init();
}

void TQListView::init()
{
    d = new TQListViewPrivate;
    d->vci = 0;
    d->timer = new TQTimer( this );
    d->levelWidth = 20;
    d->r = 0;
    d->rootIsExpandable = 0;
    d->h = new TQHeader( this, "list view header" );
    d->h->installEventFilter( this );
    d->focusItem = 0;
    d->oldFocusItem = 0;
    d->drawables = 0;
    d->dirtyItems = 0;
    d->dirtyItemTimer = new TQTimer( this );
    d->visibleTimer = new TQTimer( this );
    d->renameTimer = new TQTimer( this );
    d->autoopenTimer = new TQTimer( this );
    d->margin = 1;
    d->selectionMode = TQListView::Single;
    d->sortcolumn = 0;
    d->ascending = TRUE;
    d->allColumnsShowFocus = FALSE;
    d->fontMetricsHeight = fontMetrics().height();
    d->h->setTracking(TRUE);
    d->buttonDown = FALSE;
    d->ignoreDoubleClick = FALSE;
    d->column.setAutoDelete( TRUE );
    d->iterators = 0;
    d->scrollTimer = 0;
    d->sortIndicator = FALSE;
    d->clearing = FALSE;
    d->minLeftBearing = fontMetrics().minLeftBearing();
    d->minRightBearing = fontMetrics().minRightBearing();
    d->ellipsisWidth = fontMetrics().width( "..." ) * 2;
    d->highlighted = 0;
    d->pressedItem = 0;
    d->selectAnchor = 0;
    d->select = TRUE;
    d->useDoubleBuffer = FALSE;
    d->startDragItem = 0;
    d->toolTips = TRUE;
#ifndef QT_NO_TOOLTIP
    d->toolTip = new TQListViewToolTip( viewport(), this );
#endif
    d->updateHeader = FALSE;
    d->fullRepaintOnComlumnChange = FALSE;
    d->resizeMode = NoColumn;
    d->defRenameAction = Reject;
    d->pressedEmptyArea = FALSE;
    d->startEdit = TRUE;
    d->ignoreEditAfterFocus = FALSE;
    d->inMenuMode = FALSE;
    d->pressedSelected = FALSE;

    setMouseTracking( TRUE );
    viewport()->setMouseTracking( TRUE );

    connect( d->timer, SIGNAL(timeout()),
	     this, SLOT(updateContents()) );
    connect( d->dirtyItemTimer, SIGNAL(timeout()),
	     this, SLOT(updateDirtyItems()) );
    connect( d->visibleTimer, SIGNAL(timeout()),
	     this, SLOT(makeVisible()) );
    connect( d->renameTimer, SIGNAL(timeout()),
	     this, SLOT(startRename()) );
    connect( d->autoopenTimer, SIGNAL(timeout()),
	     this, SLOT(openFocusItem()) );

    connect( d->h, SIGNAL(sizeChange(int,int,int)),
	     this, SLOT(handleSizeChange(int,int,int)) );
    connect( d->h, SIGNAL(indexChange(int,int,int)),
	     this, SLOT(handleIndexChange()) );
    connect( d->h, SIGNAL(sectionClicked(int)),
	     this, SLOT(changeSortColumn(int)) );
    connect( d->h, SIGNAL(sectionHandleDoubleClicked(int)),
	     this, SLOT(adjustColumn(int)) );
    connect( horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	     d->h, SLOT(setOffset(int)) );
    connect( horizontalScrollBar(), SIGNAL(valueChanged(int)),
	     d->h, SLOT(setOffset(int)) );

    // will access d->r
    TQListViewPrivate::Root * r = new TQListViewPrivate::Root( this );
    r->is_root = TRUE;
    d->r = r;
    d->r->setSelectable( FALSE );

    viewport()->setFocusProxy( this );
    viewport()->setFocusPolicy( WheelFocus );
    viewport()->setBackgroundMode( PaletteBase );
    setBackgroundMode( PaletteBackground, PaletteBase );
}

/*!
    \property TQListView::showSortIndicator
    \brief whether the list view header should display a sort indicator.

    If this property is TRUE, an arrow is drawn in the header of the
    list view to indicate the sort order of the list view contents.
    The arrow will be drawn in the correct column and will point up or
    down, depending on the current sort direction. The default is
    FALSE (don't show an indicator).

    \sa TQHeader::setSortIndicator()
*/

void TQListView::setShowSortIndicator( bool show )
{
    if ( show == d->sortIndicator )
	return;

    d->sortIndicator = show;
    if ( d->sortcolumn != Unsorted && d->sortIndicator )
	d->h->setSortIndicator( d->sortcolumn, d->ascending );
    else
	d->h->setSortIndicator( -1 );
}

bool TQListView::showSortIndicator() const
{
    return d->sortIndicator;
}

/*!
    \property TQListView::showToolTips
    \brief whether this list view should show tooltips for truncated column texts

    The default is TRUE.
*/

void TQListView::setShowToolTips( bool b )
{
    d->toolTips = b;
}

bool TQListView::showToolTips() const
{
    return d->toolTips;
}

/*!
    \property TQListView::resizeMode
    \brief whether all, none or the only the last column should be resized

    Specifies whether all, none or only the last column should be
    resized to fit the full width of the list view. The values for this
    property can be one of: \c NoColumn (the default), \c AllColumns
    or \c LastColumn.

    \warning Setting the resize mode should be done after all necessary
    columns have been added to the list view, otherwise the behavior is
    undefined.

    \sa TQHeader, header()
*/

void TQListView::setResizeMode( ResizeMode m )
{
    d->resizeMode = m;
    if ( m == NoColumn )
	header()->setStretchEnabled( FALSE );
    else if ( m == AllColumns )
	header()->setStretchEnabled( TRUE );
    else
	header()->setStretchEnabled( TRUE, header()->count() - 1 );
}

TQListView::ResizeMode TQListView::resizeMode() const
{
    return d->resizeMode;
}

/*!
    Destroys the list view, deleting all its items, and frees up all
    allocated resources.
*/

TQListView::~TQListView()
{
    if ( d->iterators ) {
	TQListViewItemIterator *i = d->iterators->first();
	while ( i ) {
	    i->listView = 0;
	    i = d->iterators->next();
	}
	delete d->iterators;
	d->iterators = 0;
    }

    d->focusItem = 0;
    delete d->r;
    d->r = 0;
    delete d->dirtyItems;
    d->dirtyItems = 0;
    delete d->drawables;
    d->drawables = 0;
    delete d->vci;
    d->vci = 0;
#ifndef QT_NO_TOOLTIP
    delete d->toolTip;
    d->toolTip = 0;
#endif
    delete d;
    d = 0;
}


/*!
    Calls TQListViewItem::paintCell() and
    TQListViewItem::paintBranches() as necessary for all list view
    items that require repainting in the \a cw pixels wide and \a ch
    pixels high bounding rectangle starting at position \a cx, \a cy
    with offset \a ox, \a oy. Uses the painter \a p.
*/

void TQListView::drawContentsOffset( TQPainter * p, int ox, int oy,
				    int cx, int cy, int cw, int ch )
{
    if ( columns() == 0 ) {
	paintEmptyArea( p, TQRect( cx, cy, cw, ch ) );
	return;
    }

    if ( !d->drawables ||
	 d->drawables->isEmpty() ||
	 d->topPixel > cy ||
	 d->bottomPixel < cy + ch - 1 ||
	 d->r->maybeTotalHeight < 0 )
	buildDrawableList();

    if ( d->dirtyItems ) {
	TQRect br( cx - ox, cy - oy, cw, ch );
	TQPtrDictIterator<void> it( *(d->dirtyItems) );
	TQListViewItem * i;
	while( (i = (TQListViewItem *)(it.currentKey())) != 0 ) {
	    ++it;
	    TQRect ir = itemRect( i ).intersect( viewport()->rect() );
	    if ( ir.isEmpty() || br.contains( ir ) )
		// we're painting this one, or it needs no painting: forget it
		d->dirtyItems->remove( (void *)i );
	}
	if ( d->dirtyItems->count() ) {
	    // there are still items left that need repainting
	    d->dirtyItemTimer->start( 0, TRUE );
	} else {
	    // we're painting all items that need to be painted
	    delete d->dirtyItems;
	    d->dirtyItems = 0;
	    d->dirtyItemTimer->stop();
	}
    }

    p->setFont( font() );

    TQPtrListIterator<TQListViewPrivate::DrawableItem> it( *(d->drawables) );

    TQRect r;
    int fx = -1, x, fc = 0, lc = 0;
    int tx = -1;
    TQListViewPrivate::DrawableItem * current;

    while ( (current = it.current()) != 0 ) {
	++it;
	if ( !current->i->isVisible() )
	    continue;
	int ih = current->i->height();
	int ith = current->i->totalHeight();
	int c;
	int cs;

	// need to paint current?
	if ( ih > 0 && current->y < cy+ch && current->y+ih > cy ) {
	    if ( fx < 0 ) {
		// find first interesting column, once
		x = 0;
		c = 0;
		cs = d->h->cellSize( 0 );
		while ( x + cs <= cx && c < d->h->count() ) {
		    x += cs;
		    c++;
		    if ( c < d->h->count() )
			cs = d->h->cellSize( c );
		}
		fx = x;
		fc = c;
		while( x < cx + cw && c < d->h->count() ) {
		    x += cs;
		    c++;
		    if ( c < d->h->count() )
			cs = d->h->cellSize( c );
		}
		lc = c;
	    }

	    x = fx;
	    c = fc;
	    // draw to last interesting column

	    bool drawActiveSelection = hasFocus() || d->inMenuMode ||
			    !style().styleHint( TQStyle::SH_ItemView_ChangeHighlightOnFocus, this ) ||
			    ( currentItem() && currentItem()->renameBox && currentItem()->renameBox->hasFocus() );
	    const TQColorGroup &cg = ( drawActiveSelection ? colorGroup() : palette().inactive() );

	    while ( c < lc && d->drawables ) {
		int i = d->h->mapToLogical( c );
		cs = d->h->cellSize( c );
		r.setRect( x - ox, current->y - oy, cs, ih );
		if ( i == 0 && current->i->parentItem )
		    r.setLeft( r.left() + current->l * treeStepSize() );

		p->save();
		// No need to paint if the cell isn't technically visible
		if ( !( r.width() == 0 || r.height() == 0 ) ) {
		    p->translate( r.left(), r.top() );
		    int ac = d->h->mapToLogical( c );
		    // map to Left currently. This should change once we
		    // can really reverse the listview.
		    int align = columnAlignment( ac );
		    if ( align == AlignAuto ) align = AlignLeft;
		    if ( d->useDoubleBuffer ) {
			TQRect a( 0, 0, r.width(), current->i->height() );
			TQSharedDoubleBuffer buffer( p, a, TQSharedDoubleBuffer::Force );
			if ( buffer.isBuffered() )
			    paintEmptyArea( buffer.painter(), a );
			buffer.painter()->setFont( p->font() );
			buffer.painter()->setPen( p->pen() );
			buffer.painter()->setBrush( p->brush() );
			buffer.painter()->setBrushOrigin( -r.left(), -r.top() );
			current->i->paintCell( buffer.painter(), cg, ac, r.width(),
			    align );
		    } else {
			current->i->paintCell( p, cg, ac, r.width(),
			    align );
		    }
		}
		p->restore();
		x += cs;
		c++;
	    }

	    if ( current->i == d->focusItem && hasFocus() &&
		 !d->allColumnsShowFocus ) {
		p->save();
		int cell = d->h->mapToActual( 0 );
		TQRect r( d->h->cellPos( cell ) - ox, current->y - oy, d->h->cellSize( cell ), ih );
		if ( current->i->parentItem )
		    r.setLeft( r.left() + current->l * treeStepSize() );
		if ( r.left() < r.right() )
		    current->i->paintFocus( p, colorGroup(), r );
		p->restore();
	    }
	}

    	const int cell = d->h->mapToActual( 0 );

	// does current need focus indication?
	if ( current->i == d->focusItem && hasFocus() &&
	     d->allColumnsShowFocus ) {
	    p->save();
	    int x = -contentsX();
	    int w = header()->cellPos( header()->count() - 1 ) +
		    header()->cellSize( header()->count() - 1 );

	    r.setRect( x, current->y - oy, w, ih );
	    if ( d->h->mapToActual( 0 ) == 0 || ( current->l == 0 && !rootIsDecorated() ) ) {
		int offsetx = TQMIN( current->l * treeStepSize(), d->h->cellSize( cell ) );
		r.setLeft( r.left() + offsetx );
		current->i->paintFocus( p, colorGroup(), r );
	    } else {
		int xdepth = TQMIN( treeStepSize() * ( current->i->depth() + ( rootIsDecorated() ? 1 : 0) )
			     + itemMargin(), d->h->cellSize( cell ) );
		xdepth += d->h->cellPos( cell );
		TQRect r1( r );
		r1.setRight( d->h->cellPos( cell ) - 1 );
		TQRect r2( r );
		r2.setLeft( xdepth - 1 );
		current->i->paintFocus( p, colorGroup(), r1 );
		current->i->paintFocus( p, colorGroup(), r2 );
	    }
	    p->restore();
	}

	if ( tx < 0 )
	    tx = d->h->cellPos( cell );

	// do any children of current need to be painted?
	if ( ih != ith &&
	     (current->i != d->r || d->rootIsExpandable) &&
	     current->y + ith > cy &&
	     current->y + ih < cy + ch &&
	     tx + current->l * treeStepSize() < cx + cw &&
	     tx + (current->l+1) * treeStepSize() > cx ) {
	    // compute the clip rectangle the safe way

	    int rtop = current->y + ih;
	    int rbottom = current->y + ith;
	    int rleft = tx + current->l*treeStepSize();
	    int rright = rleft + treeStepSize();

	    int crtop = TQMAX( rtop, cy );
	    int crbottom = TQMIN( rbottom, cy+ch );
	    int crleft = TQMAX( rleft, cx );
	    int crright = TQMIN( rright, cx+cw );

	    r.setRect( crleft-ox, crtop-oy,
		       crright-crleft, crbottom-crtop );

	    if ( r.isValid() ) {
		p->save();
		p->translate( rleft-ox, crtop-oy );
		current->i->paintBranches( p, colorGroup(), treeStepSize(),
					   rtop - crtop, r.height() );
		p->restore();
	    }
	}
    }

    if ( d->r->totalHeight() < cy + ch )
	paintEmptyArea( p, TQRect( cx - ox, d->r->totalHeight() - oy,
				  cw, cy + ch - d->r->totalHeight() ) );

    int c = d->h->count()-1;
    if ( c >= 0 &&
	 d->h->cellPos( c ) + d->h->cellSize( c ) < cx + cw ) {
	c = d->h->cellPos( c ) + d->h->cellSize( c );
	paintEmptyArea( p, TQRect( c - ox, cy - oy, cx + cw - c, ch ) );
    }
}



/*!
    Paints \a rect so that it looks like empty background using
    painter \a p. \a rect is in widget coordinates, ready to be fed to
    \a p.

    The default function fills \a rect with the
    viewport()->backgroundBrush().
*/

void TQListView::paintEmptyArea( TQPainter * p, const TQRect & rect )
{
    TQStyleOption opt( d->sortcolumn, 0 ); // ### hack; in 3.1, add a property in TQListView and TQHeader
    TQStyle::SFlags how = TQStyle::Style_Default;
    if ( isEnabled() )
	how |= TQStyle::Style_Enabled;

    style().drawComplexControl( TQStyle::CC_ListView,
				p, this, rect, colorGroup(),
				how, TQStyle::SC_ListView, TQStyle::SC_None,
				opt );
}


/*
    Rebuilds the list of drawable TQListViewItems. This function is
    const so that const functions can call it without requiring
    d->drawables to be mutable.
*/

void TQListView::buildDrawableList() const
{
    d->r->enforceSortOrder();

    TQPtrStack<TQListViewPrivate::Pending> stack;
    stack.push( new TQListViewPrivate::Pending( ((int)d->rootIsExpandable)-1,
					       0, d->r ) );

    // could mess with cy and ch in order to speed up vertical
    // scrolling
    int cy = contentsY();
    int ch = ((TQListView *)this)->visibleHeight();
    d->topPixel = cy + ch; // one below bottom
    d->bottomPixel = cy - 1; // one above top

    TQListViewPrivate::Pending * cur;

    // used to work around lack of support for mutable
    TQPtrList<TQListViewPrivate::DrawableItem> * dl;

    dl = new TQPtrList<TQListViewPrivate::DrawableItem>;
    dl->setAutoDelete( TRUE );
    if ( d->drawables )
	delete ((TQListView *)this)->d->drawables;
    ((TQListView *)this)->d->drawables = dl;

    while ( !stack.isEmpty() ) {
	cur = stack.pop();

	int ih = cur->i->height();
	int ith = cur->i->totalHeight();

	// if this is not true, buildDrawableList has been called recursivly
	Q_ASSERT( dl == d->drawables );

	// is this item, or its branch symbol, inside the viewport?
	if ( cur->y + ith >= cy && cur->y < cy + ch ) {
	    dl->append( new TQListViewPrivate::DrawableItem(cur));
	    // perhaps adjust topPixel up to this item?  may be adjusted
	    // down again if any children are not to be painted
	    if ( cur->y < d->topPixel )
		d->topPixel = cur->y;
	    // bottompixel is easy: the bottom item drawn contains it
	    d->bottomPixel = cur->y + ih - 1;
	}

	// push younger sibling of cur on the stack?
	if ( cur->y + ith < cy+ch && cur->i->siblingItem )
	    stack.push( new TQListViewPrivate::Pending(cur->l,
						      cur->y + ith,
						      cur->i->siblingItem));

	// do any children of cur need to be painted?
	if ( cur->i->isOpen() && cur->i->childCount() &&
	     cur->y + ith > cy &&
	     cur->y + ih < cy + ch ) {
	    cur->i->enforceSortOrder();

	    TQListViewItem * c = cur->i->childItem;
	    int y = cur->y + ih;

	    // if any of the children are not to be painted, skip them
	    // and invalidate topPixel
	    while ( c && y + c->totalHeight() <= cy ) {
		y += c->totalHeight();
		c = c->siblingItem;
		d->topPixel = cy + ch;
	    }

	    // push one child on the stack, if there is at least one
	    // needing to be painted
	    if ( c && y < cy+ch )
		stack.push( new TQListViewPrivate::Pending( cur->l + 1,
							   y, c ) );
	}

	delete cur;
    }
}

/*!
    \property TQListView::treeStepSize
    \brief the number of pixels a child is offset from its parent

    The default is 20 pixels.

    Of course, this property is only meaningful for hierarchical list
    views.
*/

int TQListView::treeStepSize() const
{
    return d->levelWidth;
}

void TQListView::setTreeStepSize( int size )
{
    if ( size != d->levelWidth ) {
	d->levelWidth = size;
	viewport()->repaint( FALSE );
    }
}

/*!
    Inserts item \a i into the list view as a top-level item. You do
    not need to call this unless you've called takeItem(\a i) or
    TQListViewItem::takeItem(\a i) and need to reinsert \a i elsewhere.

    \sa TQListViewItem::takeItem() takeItem()
*/

void TQListView::insertItem( TQListViewItem * i )
{
    if ( d->r ) // not for d->r itself
	d->r->insertItem( i );
}


/*!
    Removes and deletes all the items in this list view and triggers
    an update.

    Note that the currentChanged() signal is not emitted when this slot is invoked.
    \sa triggerUpdate()
*/

void TQListView::clear()
{
    bool wasUpdatesEnabled = viewport()->isUpdatesEnabled();
    viewport()->setUpdatesEnabled( FALSE );
    setContentsPos( 0, 0 );
    viewport()->setUpdatesEnabled( wasUpdatesEnabled );
    bool block = signalsBlocked();
    blockSignals( TRUE );
    d->clearing = TRUE;
    clearSelection();
    if ( d->iterators ) {
	TQListViewItemIterator *i = d->iterators->first();
	while ( i ) {
	    i->curr = 0;
	    i = d->iterators->next();
	}
    }

    if ( d->drawables )
	d->drawables->clear();
    delete d->dirtyItems;
    d->dirtyItems = 0;
    d->dirtyItemTimer->stop();

    d->focusItem = 0;
    d->selectAnchor = 0;
    d->pressedItem = 0;
    d->highlighted = 0;
    d->startDragItem = 0;

    // if it's down its downness makes no sense, so undown it
    d->buttonDown = FALSE;

    TQListViewItem *c = (TQListViewItem *)d->r->firstChild();
    TQListViewItem *n;
    while( c ) {
	n = (TQListViewItem *)c->nextSibling();
	delete c;
	c = n;
    }
    resizeContents( d->h->sizeHint().width(), contentsHeight() );
    delete d->r;
    d->r = 0;
    TQListViewPrivate::Root * r = new TQListViewPrivate::Root( this );
    r->is_root = TRUE;
    d->r = r;
    d->r->setSelectable( FALSE );
    blockSignals( block );
    triggerUpdate();
    d->clearing = FALSE;
}

/*!
    \reimp
*/

void TQListView::setContentsPos( int x, int y )
{
    updateGeometries();
    TQScrollView::setContentsPos( x, y );
}

/*!
    Adds a \a width pixels wide column with the column header \a label
    to the list view, and returns the index of the new column.

    All columns apart from the first one are inserted to the right of
    the existing ones.

    If \a width is negative, the new column's \l WidthMode is set to
    \c Maximum instead of \c Manual.

    \sa setColumnText() setColumnWidth() setColumnWidthMode()
*/
int TQListView::addColumn( const TQString &label, int width )
{
    int c = d->h->addLabel( label, width );
    d->column.resize( c+1 );
    d->column.insert( c, new TQListViewPrivate::Column );
    d->column[c]->wmode = width >=0 ? Manual : Maximum;
    updateGeometries();
    updateGeometry();
    return c;
}

/*!
    \overload

    Adds a \a width pixels wide new column with the header \a label
    and the \a iconset to the list view, and returns the index of the
    column.

    If \a width is negative, the new column's \l WidthMode is set to
    \c Maximum, and to \c Manual otherwise.

    \sa setColumnText() setColumnWidth() setColumnWidthMode()
*/
int TQListView::addColumn( const TQIconSet& iconset, const TQString &label, int width )
{
    int c = d->h->addLabel( iconset, label, width );
    d->column.resize( c+1 );
    d->column.insert( c, new TQListViewPrivate::Column );
    d->column[c]->wmode = width >=0 ? Manual : Maximum;
    updateGeometries();
    updateGeometry();
    return c;
}

/*!
    \property TQListView::columns
    \brief the number of columns in this list view

    \sa addColumn(), removeColumn()
*/

int TQListView::columns() const
{
    return d->column.count();
}

/*!
    Removes the column at position \a index.

    If no columns remain after the column is removed, the
    list view will be cleared.

    \sa clear()
*/

void TQListView::removeColumn( int index )
{
    if ( index < 0 || index > (int)d->column.count() - 1 )
	return;

    if ( d->vci ) {
	TQListViewPrivate::ViewColumnInfo *vi = d->vci, *prev = 0, *next = 0;
	for ( int i = 0; i < index; ++i ) {
	    if ( vi ) {
		prev = vi;
		vi = vi->next;
	    }
	}
	if ( vi ) {
	    next = vi->next;
	    if ( prev )
		prev->next = next;
	    vi->next = 0;
	    delete vi;
	    if ( index == 0 )
		d->vci = next;
	}
    }

    TQListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	TQListViewPrivate::ItemColumnInfo *ci = (TQListViewPrivate::ItemColumnInfo*)it.current()->columns;
	if ( ci ) {
	    TQListViewPrivate::ItemColumnInfo *prev = 0, *next = 0;
	    for ( int i = 0; i < index; ++i ) {
		if ( ci ) {
		    prev = ci;
		    ci = ci->next;
		}
	    }
	    if ( ci ) {
		next = ci->next;
		if ( prev )
		    prev->next = next;
		ci->next = 0;
		delete ci;
		if ( index == 0 )
		    it.current()->columns = next;
	    }
	}
    }

    for ( int i = index; i < (int)d->column.size(); ++i ) {
	TQListViewPrivate::Column *c = d->column.take( i );
	if ( i == index )
	    delete c;
	if ( i < (int)d->column.size()-1 )
	    d->column.insert( i, d->column[ i + 1 ] );
    }
    d->column.resize( d->column.size() - 1 );

    d->h->removeLabel( index );
    if (d->resizeMode == LastColumn)
	d->h->setStretchEnabled( TRUE, d->h->count() - 1 );

    updateGeometries();
    if ( d->column.count() == 0 )
	clear();
    updateGeometry();
    viewport()->update();
}

/*!
    Sets the heading of column \a column to \a label.

    \sa columnText()
*/
void TQListView::setColumnText( int column, const TQString &label )
{
    if ( column < d->h->count() ) {
	d->h->setLabel( column, label );
	updateGeometries();
	updateGeometry();
    }
}

/*!
    \overload

    Sets the heading of column \a column to \a iconset and \a label.

    \sa columnText()
*/
void TQListView::setColumnText( int column, const TQIconSet& iconset, const TQString &label )
{
    if ( column < d->h->count() ) {
	d->h->setLabel( column, iconset, label );
	updateGeometries();
    }
}

/*!
    Sets the width of column \a column to \a w pixels. Note that if
    the column has a \c WidthMode other than \c Manual, this width
    setting may be subsequently overridden.

    \sa columnWidth()
*/
void TQListView::setColumnWidth( int column, int w )
{
    int oldw = d->h->sectionSize( column );
    if ( column < d->h->count() && oldw != w ) {
 	d->h->resizeSection( column, w );
	disconnect( d->h, SIGNAL(sizeChange(int,int,int)),
		 this, SLOT(handleSizeChange(int,int,int)) );
	emit d->h->sizeChange( column, oldw, w);
	connect( d->h, SIGNAL(sizeChange(int,int,int)),
		 this, SLOT(handleSizeChange(int,int,int)) );
	updateGeometries();
	viewport()->update();
    }
}


/*!
    Returns the text of column \a c.

    \sa setColumnText()
*/

TQString TQListView::columnText( int c ) const
{
    return d->h->label(c);
}

/*!
    Returns the width of column \a c.

    \sa setColumnWidth()
*/

int TQListView::columnWidth( int c ) const
{
    int actual = d->h->mapToActual( c );
    return d->h->cellSize( actual );
}


/*!
    \enum TQListView::WidthMode

    This enum type describes how the width of a column in the view
    changes.

    \value Manual the column width does not change automatically.

    \value Maximum  the column is automatically sized according to the
    widths of all items in the column. (Note: The column never shrinks
    in this case.) This means that the column is always resized to the
    width of the item with the largest width in the column.

    \sa setColumnWidth() setColumnWidthMode() columnWidth()
*/


/*!
    Sets column \a{c}'s width mode to \a mode. The default depends on
    the original width argument to addColumn().

    \sa TQListViewItem::width()
*/

void TQListView::setColumnWidthMode( int c, WidthMode mode )
{
    if ( c < d->h->count() )
	 d->column[c]->wmode = mode;
}


/*!
    Returns the \c WidthMode for column \a c.

    \sa setColumnWidthMode()
*/

TQListView::WidthMode TQListView::columnWidthMode( int c ) const
{
    if ( c < d->h->count() )
	return d->column[c]->wmode;
    else
	return Manual;
}


/*!
    Sets column \a{column}'s alignment to \a align. The alignment is
    ultimately passed to TQListViewItem::paintCell() for each item in
    the list view. For horizontally aligned text with TQt::AlignLeft or
    TQt::AlignHCenter the ellipsis (...) will be to the right, for
    TQt::AlignRight the ellipsis will be to the left.

    \sa TQt::AlignmentFlags
*/

void TQListView::setColumnAlignment( int column, int align )
{
    if ( column < 0 )
	return;
    if ( !d->vci )
	d->vci = new TQListViewPrivate::ViewColumnInfo;
    TQListViewPrivate::ViewColumnInfo * l = d->vci;
    while( column ) {
	if ( !l->next )
	    l->next = new TQListViewPrivate::ViewColumnInfo;
	l = l->next;
	column--;
    }
    if ( l->align == align )
	return;
    l->align = align;
    triggerUpdate();
}


/*!
    Returns the alignment of column \a column. The default is \c
    AlignAuto.

    \sa TQt::AlignmentFlags
*/

int TQListView::columnAlignment( int column ) const
{
    if ( column < 0 || !d->vci )
	return AlignAuto;
    TQListViewPrivate::ViewColumnInfo * l = d->vci;
    while( column ) {
	if ( !l->next )
	    l->next = new TQListViewPrivate::ViewColumnInfo;
	l = l->next;
	column--;
    }
    return l ? l->align : AlignAuto;
}



/*!
    \reimp
 */
void TQListView::show()
{
    // Reimplemented to setx the correct background mode and viewed
    // area size.
    if ( !isVisible() ) {
	reconfigureItems();
	updateGeometries();
    }
    TQScrollView::show();
}


/*!
    Updates the sizes of the viewport, header, scroll bars and so on.

    \warning Don't call this directly; call triggerUpdate() instead.
*/

void TQListView::updateContents()
{
    if ( d->updateHeader )
	header()->adjustHeaderSize();
    d->updateHeader = FALSE;
    if ( !isVisible() ) {
	// Not in response to a setText/setPixmap any more.
	d->useDoubleBuffer = FALSE;

	return;
    }
    if ( d->drawables ) {
	delete d->drawables;
	d->drawables = 0;
    }
    viewport()->setUpdatesEnabled( FALSE );
    updateGeometries();
    viewport()->setUpdatesEnabled( TRUE );
    viewport()->repaint( FALSE );
    d->useDoubleBuffer = FALSE;
}


void TQListView::updateGeometries()
{
    int th = d->r->totalHeight();
    int tw = d->h->headerWidth();
    if ( d->h->offset() &&
	 tw < d->h->offset() + d->h->width() )
	horizontalScrollBar()->setValue( tw - TQListView::d->h->width() );
#if 0
    if ( TQApplication::reverseLayout() && d->h->offset() != horizontalScrollBar()->value() )
	horizontalScrollBar()->setValue( d->h->offset() );
#endif
    verticalScrollBar()->raise();
    resizeContents( tw, th );
    if ( d->h->isHidden() ) {
	setMargins( 0, 0, 0, 0 );
    } else {
	TQSize hs( d->h->sizeHint() );
	setMargins( 0, hs.height(), 0, 0 );
	d->h->setGeometry( viewport()->x(), viewport()->y()-hs.height(),
			   visibleWidth(), hs.height() );
    }
}


/*!
    Updates the display when the section \a section has changed size
    from the old size, \a os, to the new size, \a ns.
*/

void TQListView::handleSizeChange( int section, int os, int ns )
{
    bool upe = viewport()->isUpdatesEnabled();
    viewport()->setUpdatesEnabled( FALSE );
    int sx = horizontalScrollBar()->value();
    bool sv = horizontalScrollBar()->isVisible();
    updateGeometries();
    bool fullRepaint = d->fullRepaintOnComlumnChange || sx != horizontalScrollBar()->value()
		       || sv != horizontalScrollBar()->isVisible();
    d->fullRepaintOnComlumnChange = FALSE;
    viewport()->setUpdatesEnabled( upe );

    if ( fullRepaint ) {
	viewport()->repaint( FALSE );
	return;
    }

    int actual = d->h->mapToActual( section );
    int dx = ns - os;
    int left = d->h->cellPos( actual ) - contentsX() + d->h->cellSize( actual );
    if ( dx > 0 )
	left -= dx;
    if ( left < visibleWidth() )
	viewport()->scroll( dx, 0, TQRect( left, 0, visibleWidth() - left, visibleHeight() ) );
    viewport()->repaint( left - 4 - d->ellipsisWidth, 0, 4 + d->ellipsisWidth,
			 visibleHeight(), FALSE ); // border between the items and ellipses width

    // map auto to left for now. Need to fix this once we support
    // reverse layout on the listview.
    int align = columnAlignment( section );
    if ( align == AlignAuto ) align = AlignLeft;
    if ( align != AlignAuto && align != AlignLeft )
	viewport()->repaint( d->h->cellPos( actual ) - contentsX(), 0,
			     d->h->cellSize( actual ), visibleHeight() );

    if ( currentItem() && currentItem()->renameBox ) {
	TQRect r = itemRect( currentItem() );
	r = TQRect( viewportToContents( r.topLeft() ), r.size() );
	r.setLeft( header()->sectionPos( currentItem()->renameCol ) );
	r.setWidth( header()->sectionSize( currentItem()->renameCol ) - 1 );
	if ( currentItem()->renameCol == 0 )
	    r.setLeft( r.left() + itemMargin() + ( currentItem()->depth() +
						   ( rootIsDecorated() ? 1 : 0 ) ) * treeStepSize() - 1 );
	if ( currentItem()->pixmap( currentItem()->renameCol ) )
	    r.setLeft( r.left() + currentItem()->pixmap( currentItem()->renameCol )->width() );
	if ( r.x() - contentsX() < 0 )
	    r.setX( contentsX() );
	if ( r.width() > visibleWidth() )
	    r.setWidth( visibleWidth() );
	addChild( currentItem()->renameBox, r.x(), r.y() );
	currentItem()->renameBox->resize( r.size() );
    }
}


/*
    Very smart internal slot that repaints \e only the items that need
    to be repainted. Don't use this directly; call repaintItem()
    instead.
*/

void TQListView::updateDirtyItems()
{
    if ( d->timer->isActive() || !d->dirtyItems )
	return;
    TQRect ir;
    TQPtrDictIterator<void> it( *(d->dirtyItems) );
    TQListViewItem * i;
    while( (i = (TQListViewItem *)(it.currentKey())) != 0 ) {
	++it;
	ir = ir.unite( itemRect(i) );
    }
    if ( !ir.isEmpty() )  {		      // rectangle to be repainted
	if ( ir.x() < 0 )
	    ir.moveBy( -ir.x(), 0 );
	viewport()->repaint( ir, FALSE );
    }
}


void TQListView::makeVisible()
{
    if ( d->focusItem )
	ensureItemVisible( d->focusItem );
}


/*!
    Ensures that the header is correctly sized and positioned when the
    resize event \a e occurs.
*/

void TQListView::resizeEvent( TQResizeEvent *e )
{
    TQScrollView::resizeEvent( e );
    d->fullRepaintOnComlumnChange = TRUE;
    d->h->resize( visibleWidth(), d->h->height() );
}

/*! \reimp */

void TQListView::viewportResizeEvent( TQResizeEvent *e )
{
    TQScrollView::viewportResizeEvent( e );
    d->h->resize( visibleWidth(), d->h->height() );
    if ( resizeMode() != NoColumn && currentItem() && currentItem()->renameBox ) {
	TQRect r = itemRect( currentItem() );
	r = TQRect( viewportToContents( r.topLeft() ), r.size() );
	r.setLeft( header()->sectionPos( currentItem()->renameCol ) );
	r.setWidth( header()->sectionSize( currentItem()->renameCol ) - 1 );
	if ( currentItem()->renameCol == 0 )
	    r.setLeft( r.left() + itemMargin() + ( currentItem()->depth() +
						   ( rootIsDecorated() ? 1 : 0 ) ) * treeStepSize() - 1 );
	if ( currentItem()->pixmap( currentItem()->renameCol ) )
	    r.setLeft( r.left() + currentItem()->pixmap( currentItem()->renameCol )->width() );
	if ( r.x() - contentsX() < 0 )
	    r.setX( contentsX() );
	if ( r.width() > visibleWidth() )
	    r.setWidth( visibleWidth() );
	addChild( currentItem()->renameBox, r.x(), r.y() );
	currentItem()->renameBox->resize( r.size() );
    }
}

/*!
    Triggers a size, geometry and content update during the next
    iteration of the event loop. Ensures that there'll be just one
    update to avoid flicker.
*/

void TQListView::triggerUpdate()
{
    if ( !isVisible() || !isUpdatesEnabled() ) {
	// Not in response to a setText/setPixmap any more.
	d->useDoubleBuffer = FALSE;

	return; // it will update when shown, or something.
    }

    d->timer->start( 0, TRUE );
}


/*!
    Redirects the event \a e relating to object \a o, for the viewport
    to mousePressEvent(), keyPressEvent() and friends.
*/

bool TQListView::eventFilter( TQObject * o, TQEvent * e )
{
    if ( o == d->h &&
	 e->type() >= TQEvent::MouseButtonPress &&
	 e->type() <= TQEvent::MouseMove ) {
	TQMouseEvent * me = (TQMouseEvent *)e;
	TQMouseEvent me2( me->type(),
			 TQPoint( me->pos().x(),
				 me->pos().y() - d->h->height() ),
			 me->button(), me->state() );
	switch( me2.type() ) {
	case TQEvent::MouseButtonDblClick:
	    if ( me2.button() == RightButton )
		return TRUE;
	    break;
	case TQEvent::MouseMove:
	    if ( me2.state() & RightButton ) {
		viewportMouseMoveEvent( &me2 );
		return TRUE;
	    }
	    break;
	default:
	    break;
	}
    } else if ( o == viewport() ) {
	TQFocusEvent * fe = (TQFocusEvent *)e;

	switch( e->type() ) {
	case TQEvent::FocusIn:
	    focusInEvent( fe );
	    return TRUE;
	case TQEvent::FocusOut:
	    focusOutEvent( fe );
	    return TRUE;
	default:
	    // nothing
	    break;
	}
    } else if ( ::tqt_cast<TQLineEdit*>(o) ) {
	if ( currentItem() && currentItem()->renameBox ) {
	    if ( e->type() == TQEvent::KeyPress ) {
		TQKeyEvent *ke = (TQKeyEvent*)e;
		if ( ke->key() == Key_Return ||
		     ke->key() == Key_Enter ) {
		    currentItem()->okRename( currentItem()->renameCol );
		    return TRUE;
		} else if ( ke->key() == Key_Escape ) {
		    currentItem()->cancelRename( currentItem()->renameCol );
		    return TRUE;
		}
	    } else if ( e->type() == TQEvent::FocusOut ) {
		if ( ( (TQFocusEvent*)e )->reason() != TQFocusEvent::Popup ) {
		    TQCustomEvent *e = new TQCustomEvent( 9999 );
		    TQApplication::postEvent( o, e );
		    return TRUE;
		}
	    } else if ( e->type() == 9999 ) {
		if ( d->defRenameAction == Reject )
		    currentItem()->cancelRename( currentItem()->renameCol );
		else
		    currentItem()->okRename( currentItem()->renameCol );
		return TRUE;
	    }
	}
    }

    return TQScrollView::eventFilter( o, e );
}


/*!
    Returns a pointer to the list view containing this item.

    Note that this function traverses the items to the root to find the
    listview. This function will return 0 for taken items - see
    TQListViewItem::takeItem()
*/

TQListView * TQListViewItem::listView() const
{
    const TQListViewItem* c = this;
    while ( c && !c->is_root )
	c = c->parentItem;
    if ( !c )
	return 0;
    return ((TQListViewPrivate::Root*)c)->theListView();
}


/*!
    Returns the depth of this item.
*/
int TQListViewItem::depth() const
{
    return parentItem ? parentItem->depth()+1 : -1; // -1 == the hidden root
}


/*!
    Returns a pointer to the item immediately above this item on the
    screen. This is usually the item's closest older sibling, but it
    may also be its parent or its next older sibling's youngest child,
    or something else if anyoftheabove->height() returns 0. Returns 0
    if there is no item immediately above this item.

    This function assumes that all parents of this item are open (i.e.
    that this item is visible, or can be made visible by scrolling).

    This function might be relatively slow because of the tree
    traversions needed to find the correct item.

    \sa itemBelow() TQListView::itemRect()
*/

TQListViewItem * TQListViewItem::itemAbove()
{
    if ( !parentItem )
	return 0;

    TQListViewItem * c = parentItem;
    if ( c->childItem != this ) {
	c = c->childItem;
	while( c && c->siblingItem != this )
	    c = c->siblingItem;
	if ( !c )
	    return 0;
	while( c->isOpen() && c->childItem ) {
	    c = c->childItem;
	    while( c->siblingItem )
		c = c->siblingItem;		// assign c's sibling to c
	}
    }
    if ( c && ( !c->height() || !c->isEnabled() ) )
	return c->itemAbove();
    return c;
}


/*!
    Returns a pointer to the item immediately below this item on the
    screen. This is usually the item's eldest child, but it may also
    be its next younger sibling, its parent's next younger sibling,
    grandparent's, etc., or something else if anyoftheabove->height()
    returns 0. Returns 0 if there is no item immediately below this
    item.

    This function assumes that all parents of this item are open (i.e.
    that this item is visible or can be made visible by scrolling).

    \sa itemAbove() TQListView::itemRect()
*/

TQListViewItem * TQListViewItem::itemBelow()
{
    TQListViewItem * c = 0;
    if ( isOpen() && childItem ) {
	c = childItem;
    } else if ( siblingItem ) {
	c = siblingItem;
    } else if ( parentItem ) {
	c = this;
	do {
	    c = c->parentItem;
	} while( c->parentItem && !c->siblingItem );
	if ( c )
	    c = c->siblingItem;
    }
    if ( c && ( !c->height() || !c->isEnabled() ) )
	return c->itemBelow();
    return c;
}


/*!
    \fn bool TQListViewItem::isOpen () const

    Returns TRUE if this list view item has children \e and they are
    not explicitly hidden; otherwise returns FALSE.

    \sa setOpen()
*/

/*!
    Returns the first (top) child of this item, or 0 if this item has
    no children.

    Note that the children are not guaranteed to be sorted properly.
    TQListView and TQListViewItem try to postpone or avoid sorting to
    the greatest degree possible, in order to keep the user interface
    snappy.

    \sa nextSibling() sortChildItems()
*/

TQListViewItem* TQListViewItem::firstChild() const
{
    enforceSortOrder();
    return childItem;
}


/*!
    Returns the parent of this item, or 0 if this item has no parent.

    \sa firstChild(), nextSibling()
*/

TQListViewItem* TQListViewItem::parent() const
{
    if ( !parentItem || parentItem->is_root ) return 0;
    return parentItem;
}


/*!
    \fn TQListViewItem* TQListViewItem::nextSibling() const

    Returns the sibling item below this item, or 0 if there is no
    sibling item after this item.

    Note that the siblings are not guaranteed to be sorted properly.
    TQListView and TQListViewItem try to postpone or avoid sorting to
    the greatest degree possible, in order to keep the user interface
    snappy.

    \sa firstChild() sortChildItems()
*/

/*!
    \fn int TQListViewItem::childCount () const

    Returns how many children this item has. The count only includes
    the item's immediate children.
*/


/*!
    Returns the height of this item in pixels. This does not include
    the height of any children; totalHeight() returns that.
*/
int TQListViewItem::height() const
{
    TQListViewItem * that = (TQListViewItem *)this;
    if ( !that->configured ) {
	that->configured = TRUE;
	that->setup(); // ### virtual non-const function called in const
    }

    return visible ? ownHeight : 0;
}

/*!
    Call this function when the value of width() may have changed for
    column \a c. Normally, you should call this if text(c) changes.
    Passing -1 for \a c indicates that all columns may have changed.
    It is more efficient to pass -1 if two or more columns have
    changed than to call widthChanged() separately for each one.

    \sa width()
*/
void TQListViewItem::widthChanged( int c ) const
{
    TQListView *lv = listView();
    if ( lv )
	lv->widthChanged( this, c );
}

/*!
    \fn void  TQListView::dropped ( TQDropEvent * e )

    This signal is emitted, when a drop event occurred on the
    viewport (not onto an item).

    \a e provides all the information about the drop.
*/

/*!
    \fn void TQListView::selectionChanged()

    This signal is emitted whenever the set of selected items has
    changed (normally before the screen update). It is available in
    \c Single, \c Multi, and \c Extended selection modes, but is most
    useful in \c Multi selection mode.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.

    \sa setSelected() TQListViewItem::setSelected()
*/


/*!
    \fn void TQListView::pressed( TQListViewItem *item )

    This signal is emitted whenever the user presses the mouse button
    in a list view. \a item is the list view item on which the user
    pressed the mouse button, or 0 if the user didn't press the mouse
    on an item.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.
*/

/*!
    \fn void TQListView::pressed( TQListViewItem *item, const TQPoint &pnt, int c )

    \overload

    This signal is emitted whenever the user presses the mouse button
    in a list view. \a item is the list view item on which the user
    pressed the mouse button, or 0 if the user didn't press the mouse
    on an item. \a pnt is the position of the mouse cursor in global
    coordinates, and \a c is the column where the mouse cursor was
    when the user pressed the mouse button.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.
*/

/*!
    \fn void TQListView::clicked( TQListViewItem *item )

    This signal is emitted whenever the user clicks (mouse pressed \e
    and mouse released) in the list view. \a item is the clicked list
    view item, or 0 if the user didn't click on an item.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.
*/

/*!
    \fn void TQListView::mouseButtonClicked(int button, TQListViewItem * item, const TQPoint & pos, int c)

    This signal is emitted whenever the user clicks (mouse pressed \e
    and mouse released) in the list view at position \a pos. \a button
    is the mouse button that the user pressed, \a item is the clicked
    list view item or 0 if the user didn't click on an item. If \a
    item is not 0, \a c is the list view column into which the user
    pressed; if \a item is 0 \a{c}'s value is undefined.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.
*/

/*!
    \fn void TQListView::mouseButtonPressed(int button, TQListViewItem * item, const TQPoint & pos, int c)

    This signal is emitted whenever the user pressed the mouse button
    in the list view at position \a pos. \a button is the mouse button
    which the user pressed, \a item is the pressed list view item or 0
    if the user didn't press on an item. If \a item is not 0, \a c is
    the list view column into which the user pressed; if \a item is 0
    \a{c}'s value is undefined.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.
*/

/*!
    \fn void TQListView::clicked( TQListViewItem *item, const TQPoint &pnt, int c )

    \overload

    This signal is emitted whenever the user clicks (mouse pressed \e
    and mouse released) in the list view. \a item is the clicked list
    view item, or 0 if the user didn't click on an item. \a pnt is the
    position where the user has clicked in global coordinates. If \a
    item is not 0, \a c is the list view column into which the user
    pressed; if \a item is 0 \a{c}'s value is undefined.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.
*/

/*!
    \fn void TQListView::selectionChanged( TQListViewItem * )

    \overload

    This signal is emitted whenever the selected item has changed in
    \c Single selection mode (normally after the screen update). The
    argument is the newly selected item. If the selection is cleared
    (when, for example, the user clicks in the unused area of the list
    view) then this signal will not be emitted.

    In \c Multi selection mode, use the no argument overload of this
    signal.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.

    \sa setSelected() TQListViewItem::setSelected() currentChanged()
*/


/*!
    \fn void TQListView::currentChanged( TQListViewItem * )

    This signal is emitted whenever the current item has changed
    (normally after the screen update). The current item is the item
    responsible for indicating keyboard focus.

    The argument is the newly current item, or 0 if the change made no
    item current. This can happen, for example, if all items in the
    list view are deleted.

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.

    \sa setCurrentItem() currentItem()
*/


/*!
    \fn void TQListView::expanded( TQListViewItem *item )

    This signal is emitted when \a item has been expanded, i.e. when
    the children of \a item are shown.

    \sa setOpen() collapsed()
*/

/*!
    \fn void TQListView::collapsed( TQListViewItem *item )

    This signal is emitted when the \a item has been collapsed, i.e.
    when the children of \a item are hidden.

    \sa setOpen() expanded()
*/

/*!
    Processes the mouse press event \a e on behalf of the viewed widget.
*/
void TQListView::contentsMousePressEvent( TQMouseEvent * e )
{
    contentsMousePressEventEx( e );
}

void TQListView::contentsMousePressEventEx( TQMouseEvent * e )
{
    if ( !e )
	return;

    if ( !d->ignoreEditAfterFocus )
	d->startEdit = TRUE;
    d->ignoreEditAfterFocus = FALSE;

    if ( currentItem() && currentItem()->renameBox &&
	 !itemRect( currentItem() ).contains( e->pos() ) ) {
	d->startEdit = FALSE;
	if ( d->defRenameAction == Reject )
	    currentItem()->cancelRename( currentItem()->renameCol );
	else
	    currentItem()->okRename( currentItem()->renameCol );
    }

    d->startDragItem = 0;
    d->dragStartPos = e->pos();
    TQPoint vp = contentsToViewport( e->pos() );

    d->ignoreDoubleClick = FALSE;
    d->buttonDown = TRUE;

    TQListViewItem * i = itemAt( vp );
    d->pressedEmptyArea = e->y() > contentsHeight();
    if ( i && !i->isEnabled() )
	return;
    if ( d->startEdit && ( i != currentItem() || (i && !i->isSelected()) ) )
	d->startEdit = FALSE;
    TQListViewItem *oldCurrent = currentItem();

    if ( e->button() == RightButton && (e->state() & ControlButton ) )
	goto emit_signals;

    if ( !i ) {
	if ( !( e->state() & ControlButton ) )
	    clearSelection();
	goto emit_signals;
    } else {
	// No new anchor when using shift
	if ( !(e->state() & ShiftButton) )
 	    d->selectAnchor = i;
    }

    if ( (i->isExpandable() || i->childCount()) &&
	 d->h->mapToLogical( d->h->cellAt( vp.x() ) ) == 0 ) {
	int x1 = vp.x() +
		 d->h->offset() -
		 d->h->cellPos( d->h->mapToActual( 0 ) );
	TQPtrListIterator<TQListViewPrivate::DrawableItem> it( *(d->drawables) );
	while( it.current() && it.current()->i != i )
	    ++it;

	if ( it.current() ) {
            x1 -= treeStepSize() * (it.current()->l - 1);
	    TQStyle::SubControl ctrl =
		style().querySubControl( TQStyle::CC_ListView,
					 this, TQPoint(x1, e->pos().y()),
					 TQStyleOption(i) );
	    if( ctrl == TQStyle::SC_ListViewExpand &&
		e->type() == style().styleHint(TQStyle::SH_ListViewExpand_SelectMouseType, this)) {
		d->buttonDown = FALSE;
		if ( e->button() == LeftButton ) {
		    bool close = i->isOpen();
		    setOpen( i, !close );
		    // ### Looks dangerous, removed because of reentrance problems
		    // tqApp->processEvents();
		    if ( !d->focusItem ) {
			d->focusItem = i;
			repaintItem( d->focusItem );
			emit currentChanged( d->focusItem );
		    }
		    if ( close ) {
			bool newCurrent = FALSE;
			TQListViewItem *ci = d->focusItem;
			while ( ci ) {
			    if ( ci->parent() && ci->parent() == i ) {
				newCurrent = TRUE;
				break;
			    }
			    ci = ci->parent();
			}
			if ( newCurrent ) {
			    setCurrentItem( i );
			}
		    }
		}
		d->ignoreDoubleClick = TRUE;
		d->buttonDown = FALSE;
		goto emit_signals;
	    }
	}
    }

    d->select = d->selectionMode == Multi ? !i->isSelected() : TRUE;

    {// calculate activatedP
	activatedByClick = TRUE;
	TQPoint topLeft = itemRect( i ).topLeft(); //### inefficient?
	activatedP = vp - topLeft;
	int xdepth = treeStepSize() * (i->depth() + (rootIsDecorated() ? 1 : 0))
		     + itemMargin();

        // This returns the position of the first visual section?!? Shouldn't that always be 0? Keep it just in case we have missed something.
	xdepth += d->h->sectionPos( d->h->mapToSection( 0 ) );
	activatedP.rx() -= xdepth;
    }
    i->activate();
    activatedByClick = FALSE;

    if ( i != d->focusItem )
	setCurrentItem( i );
    else
	repaintItem( i );

    d->pressedSelected = i && i->isSelected();

    if ( i->isSelectable() && selectionMode() != NoSelection ) {
	if ( selectionMode() == Single )
	    setSelected( i, TRUE );
	else if ( selectionMode() == Multi )
	    setSelected( i, d->select );
	else if ( selectionMode() == Extended ) {
	    bool changed = FALSE;
	    if ( !(e->state() & (ControlButton | ShiftButton)) ) {
		if ( !i->isSelected() ) {
		    bool blocked = signalsBlocked();
		    blockSignals( TRUE );
		    clearSelection();
		    blockSignals( blocked );
		    i->setSelected( TRUE );
		    changed = TRUE;
		}
	    } else {
		if ( e->state() & ShiftButton )
		    d->pressedSelected = FALSE;
		if ( (e->state() & ControlButton) && !(e->state() & ShiftButton) && i ) {
		    i->setSelected( !i->isSelected() );
		    changed = TRUE;
		    d->pressedSelected = FALSE;
		} else if ( !oldCurrent || !i || oldCurrent == i ) {
		    if ( (bool)i->selected != d->select ) {
			changed = TRUE;
			i->setSelected( d->select );
		    }
		// Shift pressed in Extended mode ---
		} else {
		    changed = selectRange( i, oldCurrent, d->selectAnchor );
		}
	    }
	    if ( changed ) {
		d->useDoubleBuffer = TRUE;
		triggerUpdate();
		emit selectionChanged();

#if defined(QT_ACCESSIBILITY_SUPPORT)
		TQAccessible::updateAccessibility( viewport(), 0, TQAccessible::Selection );
#endif
	    }
	}
    }

 emit_signals:

    if ( i && !d->buttonDown &&
	 vp.x() + contentsX() < itemMargin() + ( i->depth() + ( rootIsDecorated() ? 1 : 0 ) ) * treeStepSize() )
	i = 0;
    d->pressedItem = i;

    int c = i ? d->h->mapToLogical( d->h->cellAt( vp.x() ) ) : -1;
    if ( !i || ( i && i->isEnabled() ) ) {
	emit pressed( i );
	emit pressed( i, viewport()->mapToGlobal( vp ), c );
    }
    emit mouseButtonPressed( e->button(), i, viewport()->mapToGlobal( vp ), c );

    if ( e->button() == RightButton && i == d->pressedItem ) {
	if ( !i && !(e->state() & ControlButton) )
	    clearSelection();

	emit rightButtonPressed( i, viewport()->mapToGlobal( vp ), c );
    }
}

/*!
    \reimp
*/

void TQListView::contentsContextMenuEvent( TQContextMenuEvent *e )
{
    if ( !receivers( SIGNAL(contextMenuRequested(TQListViewItem*,const TQPoint&,int)) ) ) {
	e->ignore();
	return;
    }
    if ( e->reason() == TQContextMenuEvent::Keyboard ) {
	TQListViewItem *item = currentItem();
	if ( item ) {
	    TQRect r = itemRect( item );
	    TQPoint p = r.topLeft();
	    if ( allColumnsShowFocus() )
		p += TQPoint( width() / 2, ( r.height() / 2 ) );
	    else
		p += TQPoint( columnWidth( 0 ) / 2, ( r.height() / 2 ) );
	    p.rx() = TQMAX( 0, p.x() );
	    p.rx() = TQMIN( visibleWidth(), p.x() );
	    emit contextMenuRequested( item, viewport()->mapToGlobal( p ), -1 );
	}
    } else {
	TQPoint vp = contentsToViewport( e->pos() );
	TQListViewItem * i = itemAt( vp );
	int c = i ? d->h->mapToLogical( d->h->cellAt( vp.x() ) ) : -1;
	emit contextMenuRequested( i, viewport()->mapToGlobal( vp ), c );
    }
}

/*!
    Processes the mouse release event \a e on behalf of the viewed widget.
*/
void TQListView::contentsMouseReleaseEvent( TQMouseEvent * e )
{
    contentsMouseReleaseEventEx( e );
}

void TQListView::contentsMouseReleaseEventEx( TQMouseEvent * e )
{
    d->startDragItem = 0;
    bool emitClicked = !d->pressedItem || d->buttonDown;
    d->buttonDown = FALSE;
    // delete and disconnect autoscroll timer, if we have one
    if ( d->scrollTimer ) {
	disconnect( d->scrollTimer, SIGNAL(timeout()),
		    this, SLOT(doAutoScroll()) );
	d->scrollTimer->stop();
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }

    if ( !e )
	return;

    if ( d->selectionMode == Extended &&
	 d->focusItem == d->pressedItem &&
	 d->pressedSelected && d->focusItem &&
	 e->button() == LeftButton) {
	bool block = signalsBlocked();
	blockSignals( TRUE );
	clearSelection();
	blockSignals( block );
	d->focusItem->setSelected( TRUE );
	emit selectionChanged();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	TQAccessible::updateAccessibility( viewport(), 0, TQAccessible::Selection );
#endif
    }

    TQPoint vp = contentsToViewport(e->pos());
    TQListViewItem *i = itemAt( vp );
    if ( i && !i->isEnabled() )
	return;

    if ( i && i == d->pressedItem && (i->isExpandable() || i->childCount()) &&
	 !d->h->mapToLogical( d->h->cellAt( vp.x() ) ) && e->button() == LeftButton &&
	 e->type() == style().styleHint(TQStyle::SH_ListViewExpand_SelectMouseType, this)) {
	TQPtrListIterator<TQListViewPrivate::DrawableItem> it( *(d->drawables) );
	while( it.current() && it.current()->i != i )
	    ++it;
	if ( it.current() ) {
	    int x1 = vp.x() + d->h->offset() - d->h->cellPos( d->h->mapToActual( 0 ) ) -
		     (treeStepSize() * (it.current()->l - 1));
	    TQStyle::SubControl ctrl = style().querySubControl( TQStyle::CC_ListView,
							       this, TQPoint(x1, e->pos().y()),
							       TQStyleOption(i) );
	    if( ctrl == TQStyle::SC_ListViewExpand ) {
		bool close = i->isOpen();
		setOpen( i, !close );
		// ### Looks dangerous, removed because of reentrance problems
		// tqApp->processEvents();
		if ( !d->focusItem ) {
		    d->focusItem = i;
		    repaintItem( d->focusItem );
		    emit currentChanged( d->focusItem );
		}
		if ( close ) {
		    bool newCurrent = FALSE;
		    TQListViewItem *ci = d->focusItem;
		    while ( ci ) {
			if ( ci->parent() && ci->parent() == i ) {
			    newCurrent = TRUE;
			    break;
			}
			ci = ci->parent();
		    }
		    if ( newCurrent )
			setCurrentItem( i );
		    d->ignoreDoubleClick = TRUE;
		}
	    }
	}
    }

    if ( i == d->pressedItem && i && i->isSelected() && e->button() == LeftButton && d->startEdit ) {
	TQRect r = itemRect( currentItem() );
	r = TQRect( viewportToContents( r.topLeft() ), r.size() );
	d->pressedColumn = header()->sectionAt(  e->pos().x() );
	r.setLeft( header()->sectionPos( d->pressedColumn ) );
	r.setWidth( header()->sectionSize( d->pressedColumn ) - 1 );
	if ( d->pressedColumn == 0 )
	    r.setLeft( r.left() + itemMargin() + ( currentItem()->depth() +
						   ( rootIsDecorated() ? 1 : 0 ) ) * treeStepSize() - 1 );
	if ( r.contains( e->pos() ) &&
	     !( e->state() & ( ShiftButton | ControlButton ) ) )
	    d->renameTimer->start( TQApplication::doubleClickInterval(), TRUE );
    }
    // Check if we are clicking at the root decoration.
    int rootColumnPos = d->h->sectionPos( 0 );
    if ( i && vp.x() + contentsX() < rootColumnPos + itemMargin() + ( i->depth() + ( rootIsDecorated() ? 1 : 0 ) ) * treeStepSize()
         && vp.x() + contentsX() > rootColumnPos ) {
        i = 0;
    }
    emitClicked = emitClicked && d->pressedItem == i;
    d->pressedItem = 0;
    d->highlighted = 0;

    if ( emitClicked ) {
	if ( !i || ( i && i->isEnabled() ) ) {
	    emit clicked( i );
	    emit clicked( i, viewport()->mapToGlobal( vp ), d->h->mapToLogical( d->h->cellAt( vp.x() ) ) );
	}
	emit mouseButtonClicked( e->button(), i, viewport()->mapToGlobal( vp ),
				 i ? d->h->mapToLogical( d->h->cellAt( vp.x() ) ) : -1 );

	if ( e->button() == RightButton ) {
	    if ( !i ) {
 		if ( !(e->state() & ControlButton) )
		    clearSelection();
		emit rightButtonClicked( 0, viewport()->mapToGlobal( vp ), -1 );
		return;
	    }

	    int c = d->h->mapToLogical( d->h->cellAt( vp.x() ) );
	    emit rightButtonClicked( i, viewport()->mapToGlobal( vp ), c );
	}
    }
}


/*!
    Processes the mouse double-click event \a e on behalf of the viewed widget.
*/
void TQListView::contentsMouseDoubleClickEvent( TQMouseEvent * e )
{
    d->renameTimer->stop();
    d->startEdit = FALSE;
    if ( !e || e->button() != LeftButton )
	return;

    // ensure that the following mouse moves and eventual release is
    // ignored.
    d->buttonDown = FALSE;

    if ( d->ignoreDoubleClick ) {
	d->ignoreDoubleClick = FALSE;
	return;
    }

    TQPoint vp = contentsToViewport(e->pos());

    TQListViewItem * i = itemAt( vp );

    // we emit doubleClicked when the item is null (or enabled) to be consistent with
    // rightButtonClicked etc.
    if ( !i || i->isEnabled() ) {
	int c = d->h->mapToLogical( d->h->cellAt( vp.x() ) );
	emit doubleClicked( i, viewport()->mapToGlobal( vp ), c );
    }

    if ( !i || !i->isEnabled() )
	return;

    if ( !i->isOpen() ) {
	if ( i->isExpandable() || i->childCount() )
	    setOpen( i, TRUE );
    } else {
	setOpen( i, FALSE );
    }

    // we emit the 'old' obsolete doubleClicked only if the item is not null and enabled
    emit doubleClicked( i );
}


/*!
    Processes the mouse move event \a e on behalf of the viewed widget.
*/
void TQListView::contentsMouseMoveEvent( TQMouseEvent * e )
{
    if ( !e )
	return;

    bool needAutoScroll = FALSE;

    TQPoint vp = contentsToViewport(e->pos());

    TQListViewItem * i = itemAt( vp );
    if ( i && !i->isEnabled() )
	return;
    if ( i != d->highlighted &&
	 !(d->pressedItem &&
	   ( d->pressedItem->isSelected() || d->selectionMode == NoSelection ) &&
	   d->pressedItem->dragEnabled() )) {

	if ( i ) {
	    emit onItem( i );
	} else {
	    emit onViewport();
	}
	d->highlighted = i;
    }

    if ( d->startDragItem )
	i = d->startDragItem;

    if ( !d->buttonDown ||
	 ( ( e->state() & LeftButton ) != LeftButton &&
	   ( e->state() & MidButton ) != MidButton &&
	   ( e->state() & RightButton ) != RightButton ) )
	return;

    if ( d->pressedItem &&
	 ( d->pressedItem->isSelected() || d->selectionMode == NoSelection ) &&
	 d->pressedItem->dragEnabled() ) {

	if ( !d->startDragItem ) {
	    setSelected( d->pressedItem, TRUE );
	    d->startDragItem = d->pressedItem;
	}
	if ( ( d->dragStartPos - e->pos() ).manhattanLength() > TQApplication::startDragDistance() ) {
	    d->buttonDown = FALSE;
#ifndef QT_NO_DRAGANDDROP
	    startDrag();
#endif
	}
	return;
    }

    // check, if we need to scroll
    if ( vp.y() > visibleHeight() || vp.y() < 0 )
	needAutoScroll = TRUE;

    // if we need to scroll and no autoscroll timer is started,
    // connect the timer
    if ( needAutoScroll && !d->scrollTimer ) {
	d->scrollTimer = new TQTimer( this );
	connect( d->scrollTimer, SIGNAL(timeout()),
		 this, SLOT(doAutoScroll()) );
	d->scrollTimer->start( 100, FALSE );
	// call it once manually
	doAutoScroll( vp );
    }

    // if we don't need to autoscroll
    if ( !needAutoScroll ) {
	// if there is a autoscroll timer, delete it
	if ( d->scrollTimer ) {
	    disconnect( d->scrollTimer, SIGNAL(timeout()),
			this, SLOT(doAutoScroll()) );
	    d->scrollTimer->stop();
	    delete d->scrollTimer;
	    d->scrollTimer = 0;
	}
	// call this to select an item ( using the pos from the event )
	doAutoScroll( vp );
    }
}


/*!
    This slot handles auto-scrolling when the mouse button is pressed
    and the mouse is outside the widget.
*/
void TQListView::doAutoScroll()
{
    doAutoScroll( TQPoint() );
}

/*
  Handles auto-scrolling when the mouse button is pressed
  and the mouse is outside the widget.

  If cursorPos is (0,0) (isNull == TRUE) it uses the current TQCursor::pos, otherwise it uses cursorPos
*/
void TQListView::doAutoScroll( const TQPoint &cursorPos )
{
    TQPoint pos = cursorPos.isNull() ? viewport()->mapFromGlobal( TQCursor::pos() ) :  cursorPos;
    if ( !d->focusItem || ( d->pressedEmptyArea && pos.y() > contentsHeight() ) )
	return;

    bool down = pos.y() > itemRect( d->focusItem ).y();

    int g = pos.y() + contentsY();

    if ( down && pos.y() > height()  )
	g = height() + contentsY();
    else if ( pos.y() < 0 )
	g = contentsY();

    TQListViewItem *c = d->focusItem, *old = 0;
    TQListViewItem *oldCurrent = c;
    if ( down ) {
	int y = itemRect( d->focusItem ).y() + contentsY();
	while( c && y + c->height() <= g ) {
	    y += c->height();
	    old = c;
	    c = c->itemBelow();
	}
	if ( !c && old )
	    c = old;
    } else {
	int y = itemRect( d->focusItem ).y() + contentsY();
	while( c && y >= g ) {
	    old = c;
	    c = c->itemAbove();
	    if ( c )
		y -= c->height();
	}
	if ( !c && old )
	    c = old;
    }

    if ( !c || c == d->focusItem )
	return;

    if ( d->focusItem ) {
	if ( d->selectionMode == Multi ) {
	    // also (de)select the ones in between
	    TQListViewItem * b = d->focusItem;
	    bool down = ( itemPos( c ) > itemPos( b ) );
	    while( b && b != c ) {
		if ( b->isSelectable() )
		    setSelected( b, d->select );
		b = down ? b->itemBelow() : b->itemAbove();
	    }
	    if ( c->isSelectable() )
		setSelected( c, d->select );
	} else if ( d->selectionMode == Extended ) {
	    if ( selectRange( c, oldCurrent, d->selectAnchor ) ) {
		d->useDoubleBuffer = TRUE;
		triggerUpdate();
		emit selectionChanged();
	    }
	}
    }

    setCurrentItem( c );
    d->visibleTimer->start( 1, TRUE );
}

/*!
    \reimp
*/

void TQListView::focusInEvent( TQFocusEvent* )
{
    d->inMenuMode = FALSE;
    if ( d->focusItem ) {
	repaintItem( d->focusItem );
    } else if ( firstChild() && TQFocusEvent::reason() != TQFocusEvent::Mouse ) {
	d->focusItem = firstChild();
	emit currentChanged( d->focusItem );
	repaintItem( d->focusItem );
    }
    if ( TQFocusEvent::reason() == TQFocusEvent::Mouse ) {
	d->ignoreEditAfterFocus = TRUE;
	d->startEdit = FALSE;
    }
    if ( style().styleHint( TQStyle::SH_ItemView_ChangeHighlightOnFocus, this ) ) {
	bool db = d->useDoubleBuffer;
	d->useDoubleBuffer = TRUE;
	viewport()->repaint( FALSE );
	d->useDoubleBuffer = db;
    }

    TQRect mfrect = itemRect( d->focusItem );
    if ( mfrect.isValid() ) {
	if ( header() && header()->isVisible() )
	    setMicroFocusHint( mfrect.x(), mfrect.y()+header()->height(), mfrect.width(), mfrect.height(), FALSE );
	else
	    setMicroFocusHint( mfrect.x(), mfrect.y(), mfrect.width(), mfrect.height(), FALSE );
    }
}


/*!
    \reimp
*/

void TQListView::focusOutEvent( TQFocusEvent* )
{
    if ( TQFocusEvent::reason() == TQFocusEvent::Popup && d->buttonDown )
	d->buttonDown = FALSE;
    if ( style().styleHint( TQStyle::SH_ItemView_ChangeHighlightOnFocus, this ) ) {
	d->inMenuMode =
	    TQFocusEvent::reason() == TQFocusEvent::Popup
	    || (tqApp->focusWidget() && tqApp->focusWidget()->inherits("TQMenuBar"));
	if ( !d->inMenuMode ) {
	    bool db = d->useDoubleBuffer;
	    d->useDoubleBuffer = TRUE;
	    viewport()->repaint( FALSE );
	    d->useDoubleBuffer = db;
	}
    }

    if ( d->focusItem )
	repaintItem( d->focusItem );
}


/*!
    \reimp
*/

void TQListView::keyPressEvent( TQKeyEvent * e )
{
    if (currentItem() && currentItem()->renameBox)
        return;
    if (!firstChild()) {
	e->ignore();
	return; // subclass bug
    }

    TQListViewItem* oldCurrent = currentItem();
    if ( !oldCurrent ) {
	setCurrentItem( firstChild() );
	if ( d->selectionMode == Single )
	    setSelected( firstChild(), TRUE );
	return;
    }

    TQListViewItem * i = currentItem();
    TQListViewItem *old = i;

    TQRect r( itemRect( i ) );
    TQListViewItem * i2;

    bool singleStep = FALSE;
    bool selectCurrent = TRUE;
    bool wasNavigation = TRUE;

    switch( e->key() ) {
    case Key_Backspace:
    case Key_Delete:
	d->currentPrefix.truncate( 0 );
	break;
    case Key_Enter:
    case Key_Return:
	d->currentPrefix.truncate( 0 );
	if ( i && !i->isSelectable() && i->isEnabled() &&
	     ( i->childCount() || i->isExpandable() || i->isOpen() ) ) {
	    i->setOpen( !i->isOpen() );
	    return;
	}
	e->ignore();
	if ( currentItem() && !currentItem()->isEnabled() )
	    break;
	emit returnPressed( currentItem() );
	// do NOT accept.  TQDialog.
	return;
    case Key_Down:
	selectCurrent = FALSE;
	i = i->itemBelow();
	d->currentPrefix.truncate( 0 );
	singleStep = TRUE;
	break;
    case Key_Up:
	selectCurrent = FALSE;
	i = i->itemAbove();
	d->currentPrefix.truncate( 0 );
	singleStep = TRUE;
	break;
    case Key_Home:
	selectCurrent = FALSE;
	i = firstChild();
        if (!i->height() || !i->isEnabled())
            i = i->itemBelow();
	d->currentPrefix.truncate( 0 );
	break;
    case Key_End:
	selectCurrent = FALSE;
	i = firstChild();
        while (i->nextSibling() && i->nextSibling()->height() && i->nextSibling()->isEnabled())
	    i = i->nextSibling();
	while ( i->itemBelow() )
	    i = i->itemBelow();
	d->currentPrefix.truncate( 0 );
	break;
    case Key_Next:
	selectCurrent = FALSE;
	i2 = itemAt( TQPoint( 0, visibleHeight()-1 ) );
	if ( i2 == i || !r.isValid() ||
	     visibleHeight() <= itemRect( i ).bottom() ) {
	    if ( i2 )
		i = i2;
	    int left = visibleHeight();
	    while( (i2 = i->itemBelow()) != 0 && left > i2->height() ) {
		left -= i2->height();
		i = i2;
	    }
	} else {
	    if ( !i2 ) {
		// list is shorter than the view, goto last item
		while( (i2 = i->itemBelow()) != 0 )
		    i = i2;
	    } else {
		i = i2;
	    }
	}
	d->currentPrefix.truncate( 0 );
	break;
    case Key_Prior:
	selectCurrent = FALSE;
	i2 = itemAt( TQPoint( 0, 0 ) );
	if ( i == i2 || !r.isValid() || r.top() <= 0 ) {
	    if ( i2 )
		i = i2;
	    int left = visibleHeight();
	    while( (i2 = i->itemAbove()) != 0 && left > i2->height() ) {
		left -= i2->height();
		i = i2;
	    }
	} else {
	    i = i2;
	}
	d->currentPrefix.truncate( 0 );
	break;
    case Key_Plus:
	d->currentPrefix.truncate( 0 );
	if (  !i->isOpen() && (i->isExpandable() || i->childCount()) )
	    setOpen( i, TRUE );
	else
	    return;
	break;
    case Key_Right:
	d->currentPrefix.truncate( 0 );
	if ( i->isOpen() && i->childItem) {
	    TQListViewItem *childItem = i->childItem;
	    while (childItem && !childItem->isVisible())
		childItem = childItem->nextSibling();
	    if (childItem)
		i = childItem;
	} else if ( !i->isOpen() && (i->isExpandable() || i->childCount()) ) {
	    setOpen( i, TRUE );
	} else if ( contentsX() + visibleWidth() < contentsWidth() ) {
	    horizontalScrollBar()->addLine();
	    return;
	} else {
	    return;
	}
	break;
    case Key_Minus:
	d->currentPrefix.truncate( 0 );
	if ( i->isOpen() )
	    setOpen( i, FALSE );
	else
	    return;
	break;
    case Key_Left:
	d->currentPrefix.truncate( 0 );
	if ( i->isOpen() ) {
	    setOpen( i, FALSE );
	} else if ( i->parentItem && i->parentItem != d->r ) {
	    i = i->parentItem;
	} else if ( contentsX() ) {
	    horizontalScrollBar()->subtractLine();
	    return;
	} else {
	    return;
	}
	break;
    case Key_Space:
	activatedByClick = FALSE;
	d->currentPrefix.truncate( 0 );
	if ( currentItem() && !currentItem()->isEnabled() )
	    break;
	i->activate();
	if ( i->isSelectable() && ( d->selectionMode == Multi || d->selectionMode == Extended ) ) {
	    setSelected( i, !i->isSelected() );
	    d->currentPrefix.truncate( 0 );
	}
	emit spacePressed( currentItem() );
	break;
    case Key_Escape:
	e->ignore(); // For TQDialog
	return;
    case Key_F2:
	if ( currentItem() && currentItem()->renameEnabled( 0 ) )
	    currentItem()->startRename( 0 );
    default:
	if ( e->text().length() > 0 && e->text()[ 0 ].isPrint() ) {
	    selectCurrent = FALSE;
	    wasNavigation = FALSE;
	    TQString input( d->currentPrefix );
	    TQListViewItem * keyItem = i;
	    TQTime now( TQTime::currentTime() );
	    bool tryFirst = TRUE;
	    while( keyItem ) {
		// try twice, first with the previous string and this char
		if ( d->currentPrefixTime.msecsTo( now ) <= 400 )
		    input = input + e->text().lower();
		else
		    input = e->text().lower();
		if ( input.length() == e->text().length() ) {
		    if ( keyItem->itemBelow() ) {
			keyItem = keyItem->itemBelow();
			tryFirst = TRUE;
		    } else {
			keyItem = firstChild();
			tryFirst = FALSE;
		    }
		}
                TQString keyItemKey;
		TQString prefix;
		while( keyItem ) {
                    keyItemKey = TQString::null;
		    // Look first in the sort column, then left to right
		    if (d->sortcolumn != Unsorted)
			keyItemKey = keyItem->text(d->sortcolumn);
		    for ( int col = 0; col < d->h->count() && keyItemKey.isNull(); ++col )
			keyItemKey = keyItem->text( d->h->mapToSection(col) );
		    if ( !keyItemKey.isEmpty() ) {
			prefix = keyItemKey;
			prefix.truncate( input.length() );
			prefix = prefix.lower();
			if ( prefix == input ) {
			    d->currentPrefix = input;
			    d->currentPrefixTime = now;
			    i = keyItem;
				// nonoptimal double-break...
			    keyItem = 0;
			    input.truncate( 0 );
			    tryFirst = FALSE;
			}
		    }
		    if ( keyItem )
			keyItem = keyItem->itemBelow();
		    if ( !keyItem && tryFirst ) {
			keyItem = firstChild();
			tryFirst = FALSE;
		    }
		}
		// then, if appropriate, with just this character
		if ( input.length() > e->text().length() ) {
		    input.truncate(0);
		    keyItem = i;
		}
	    }
	} else {
	    d->currentPrefix.truncate( 0 );
	    if ( e->state() & ControlButton ) {
		d->currentPrefix = TQString::null;
		switch ( e->key() ) {
		case Key_A:
		    selectAll( TRUE );
		    break;
		}
	    }
	    e->ignore();
	    return;
	}
    }

    if ( !i )
	return;

    if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
	d->selectAnchor = i;

    setCurrentItem( i );
    if ( i->isSelectable() )
	handleItemChange( old, wasNavigation && (e->state() & ShiftButton),
			  wasNavigation && (e->state() & ControlButton) );

    if ( d->focusItem && !d->focusItem->isSelected() && d->selectionMode == Single && selectCurrent )
	setSelected( d->focusItem, TRUE );

    if ( singleStep )
	d->visibleTimer->start( 1, TRUE );
    else
	ensureItemVisible( i );
}


/*!
    Returns the list view item at \a viewPos. Note that \a viewPos is
    in the viewport()'s coordinate system, not in the list view's own,
    much larger, coordinate system.

    itemAt() returns 0 if there is no such item.

    Note that you also get the pointer to the item if \a viewPos
    points to the root decoration (see setRootIsDecorated()) of the
    item. To check whether or not \a viewPos is on the root decoration
    of the item, you can do something like this:

    \code
    TQListViewItem *i = itemAt( p );
    if ( i ) {
	if ( p.x() > header()->sectionPos( header()->mapToIndex( 0 ) ) +
		treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() ||
		p.x() < header()->sectionPos( header()->mapToIndex( 0 ) ) ) {
	    ; // p is not on root decoration
	else
	    ; // p is on the root decoration
    }
    \endcode

    This might be interesting if you use this function to find out
    where the user clicked and if you want to start a drag (which you
    do not want to do if the user clicked onto the root decoration of
    an item).

    \sa itemPos() itemRect() viewportToContents()
*/

TQListViewItem * TQListView::itemAt( const TQPoint & viewPos ) const
{
    if ( viewPos.x() > contentsWidth() - contentsX() ) {
	return 0;
    }

    if ( !d->drawables || d->drawables->isEmpty() ) {
	buildDrawableList();
    }

    TQListViewPrivate::DrawableItem * c = d->drawables->first();
    int g = viewPos.y() + contentsY();

    while( c && c->i && ( c->y + c->i->height() <= g ||
			  !c->i->isVisible() ||
			  ( c->i->parent() && !c->i->parent()->isVisible() ) ) )
	c = d->drawables->next();

    TQListViewItem *i = (c && c->y <= g) ? c->i : 0;
    return i;
}


/*!
    Returns the y-coordinate of \a item in the list view's coordinate
    system. This function is normally much slower than itemAt() but it
    works for all items, whereas itemAt() normally works only for
    items on the screen.

    This is a thin wrapper around TQListViewItem::itemPos().

    \sa itemAt() itemRect()
*/

int TQListView::itemPos( const TQListViewItem * item )
{
    return item ? item->itemPos() : 0;
}


/*! \obsolete
    \property TQListView::multiSelection
    \brief whether the list view is in multi-selection or extended-selection mode

    If you enable multi-selection, \c Multi, mode, it is possible to
    specify whether or not this mode should be extended. \c Extended
    means that the user can select multiple items only when pressing
    the Shift or Ctrl key at the same time.

    The default selection mode is \c Single.

    \sa selectionMode()
*/

void TQListView::setMultiSelection( bool enable )
{
    if ( !enable )
	d->selectionMode = TQListView::Single;
    else if (  d->selectionMode != Multi && d->selectionMode != Extended )
	d->selectionMode = TQListView::Multi;
}

bool TQListView::isMultiSelection() const
{
    return d->selectionMode == TQListView::Extended || d->selectionMode == TQListView::Multi;
}

/*!
    \property TQListView::selectionMode
    \brief the list view's selection mode

    The mode can be \c Single (the default), \c Extended, \c Multi or
    \c NoSelection.

    \sa multiSelection
*/

void TQListView::setSelectionMode( SelectionMode mode )
{
    if ( d->selectionMode == mode )
	return;

    if ( ( d->selectionMode == Multi || d->selectionMode == Extended ) &&
	 ( mode == TQListView::Single || mode == TQListView::NoSelection ) ){
	clearSelection();
	if ( ( mode == TQListView::Single ) && currentItem() )
	    currentItem()->selected = TRUE;
    }

    d->selectionMode = mode;
}

TQListView::SelectionMode TQListView::selectionMode() const
{
    return d->selectionMode;
}


/*!
    If \a selected is TRUE the \a item is selected; otherwise it is
    unselected.

    If the list view is in \c Single selection mode and \a selected is
    TRUE, the currently selected item is unselected and \a item is
    made current. Unlike TQListViewItem::setSelected(), this function
    updates the list view as necessary and emits the
    selectionChanged() signals.

    \sa isSelected() setMultiSelection() isMultiSelection()
    setCurrentItem(), setSelectionAnchor()
*/

void TQListView::setSelected( TQListViewItem * item, bool selected )
{
    if ( !item || item->isSelected() == selected ||
	 !item->isSelectable() || selectionMode() == NoSelection )
	return;

    bool emitHighlighted = FALSE;
    if ( selectionMode() == Single && d->focusItem != item ) {
	TQListViewItem *o = d->focusItem;
	if ( d->focusItem && d->focusItem->selected )
	    d->focusItem->setSelected( FALSE );
	d->focusItem = item;
	if ( o )
	    repaintItem( o );
	emitHighlighted = TRUE;
    }

    item->setSelected( selected );

    repaintItem( item );

    if ( d->selectionMode == Single && selected )
	emit selectionChanged( item );
    emit selectionChanged();

    if ( emitHighlighted )
	emit currentChanged( d->focusItem );
}

/*!
    Sets the selection anchor to \a item, if \a item is selectable.

    The selection anchor is the item that remains selected when
    Shift-selecting with either mouse or keyboard in \c Extended
    selection mode.

    \sa setSelected()
*/

void TQListView::setSelectionAnchor( TQListViewItem *item )
{
    if ( item && item->isSelectable() )
	d->selectAnchor = item;
}

/*!
    Sets all the items to be not selected, updates the list view as
    necessary, and emits the selectionChanged() signals. Note that for
    \c Multi selection list views this function needs to iterate over
    \e all items.

    \sa setSelected(), setMultiSelection()
*/

void TQListView::clearSelection()
{
    selectAll( FALSE );
}

/*!
    If \a select is TRUE, all the items get selected; otherwise all
    the items get unselected. This only works in the selection modes \c
    Multi and \c Extended. In \c Single and \c NoSelection mode the
    selection of the current item is just set to \a select.
*/

void TQListView::selectAll( bool select )
{
    if ( d->selectionMode == Multi || d->selectionMode == Extended ) {
	bool b = signalsBlocked();
	blockSignals( TRUE );
	bool anything = FALSE;
	TQListViewItemIterator it( this );
	while ( it.current() ) {
	    TQListViewItem *i = it.current();
	    if ( (bool)i->selected != select ) {
		i->setSelected( select );
		anything = TRUE;
	    }
	    ++it;
	}
	blockSignals( b );
	if ( anything ) {
	    emit selectionChanged();
	    d->useDoubleBuffer = TRUE;
	    triggerUpdate();
	}
    } else if ( d->focusItem ) {
	TQListViewItem * i = d->focusItem;
	setSelected( i, select );
    }
}

/*!
    Inverts the selection. Only works in \c Multi and \c Extended
    selection modes.
*/

void TQListView::invertSelection()
{
    if ( d->selectionMode == Single ||
	 d->selectionMode == NoSelection )
	return;

    bool b = signalsBlocked();
    blockSignals( TRUE );
    TQListViewItemIterator it( this );
    for ( ; it.current(); ++it )
	it.current()->setSelected( !it.current()->isSelected() );
    blockSignals( b );
    emit selectionChanged();
    triggerUpdate();
}


/*!
    Returns TRUE if the list view item \a i is selected; otherwise
    returns FALSE.

    \sa TQListViewItem::isSelected()
*/

bool TQListView::isSelected( const TQListViewItem * i ) const
{
    return i ? i->isSelected() : FALSE;
}


/*!
    Returns the selected item if the list view is in \c Single
    selection mode and an item is selected.

    If no items are selected or the list view is not in \c Single
    selection mode this function returns 0.

    \sa setSelected() setMultiSelection()
*/

TQListViewItem * TQListView::selectedItem() const
{
    if ( d->selectionMode != Single )
	return 0;
    if ( d->focusItem && d->focusItem->isSelected() )
	return d->focusItem;
    return 0;
}


/*!
    Sets item \a i to be the current item and repaints appropriately
    (i.e. highlights the item). The current item is used for keyboard
    navigation and focus indication; it is independent of any selected
    items, although a selected item can also be the current item.

    This function does not set the selection anchor. Use
    setSelectionAnchor() instead.

    \sa currentItem() setSelected()
*/

void TQListView::setCurrentItem( TQListViewItem * i )
{
    if ( !i || d->focusItem == i || !i->isEnabled() )
	return;

    if ( currentItem() && currentItem()->renameBox ) {
	if ( d->defRenameAction == Reject )
	    currentItem()->cancelRename( currentItem()->renameCol );
	else
	    currentItem()->okRename( currentItem()->renameCol );
    }

    TQListViewItem * prev = d->focusItem;
    d->focusItem = i;

    TQRect mfrect = itemRect( i );
    if ( mfrect.isValid() ) {
	if ( header() && header()->isVisible() )
	    setMicroFocusHint( mfrect.x(), mfrect.y()+header()->height(), mfrect.width(), mfrect.height(), FALSE );
	else
	    setMicroFocusHint( mfrect.x(), mfrect.y(), mfrect.width(), mfrect.height(), FALSE );
    }

    if ( i != prev ) {
	if ( i && d->selectionMode == Single ) {
	    bool changed = FALSE;
	    if ( prev && prev->selected ) {
		changed = TRUE;
		prev->setSelected( FALSE );
	    }
	    if ( i && !i->selected && d->selectionMode != NoSelection && i->isSelectable() ) {
		i->setSelected( TRUE );
		changed = TRUE;
		emit selectionChanged( i );
	    }
	    if ( changed )
		emit selectionChanged();
	}

	if ( i )
	    repaintItem( i );
	if ( prev )
	    repaintItem( prev );
	emit currentChanged( i );

#if defined(QT_ACCESSIBILITY_SUPPORT)
	TQAccessible::updateAccessibility( viewport(), indexOfItem( i ), TQAccessible::Focus );
#endif
    }
}


/*!
    Returns the current item, or 0 if there isn't one.

    \sa setCurrentItem()
*/

TQListViewItem * TQListView::currentItem() const
{
    return d->focusItem;
}


/*!
    Returns the rectangle on the screen that item \a i occupies in
    viewport()'s coordinates, or an invalid rectangle if \a i is 0 or
    is not currently visible.

    The rectangle returned does not include any children of the
    rectangle (i.e. it uses TQListViewItem::height(), rather than
    TQListViewItem::totalHeight()). If you want the rectangle to
    include children you can use something like this:

    \code
    TQRect r( listView->itemRect( item ) );
    r.setHeight( (TQCOORD)(TQMIN( item->totalHeight(),
				listView->viewport->height() - r.y() ) ) )
    \endcode

    Note the way it avoids too-high rectangles. totalHeight() can be
    much larger than the window system's coordinate system allows.

    itemRect() is comparatively slow. It's best to call it only for
    items that are probably on-screen.
*/

TQRect TQListView::itemRect( const TQListViewItem * i ) const
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    TQListViewPrivate::DrawableItem * c = d->drawables->first();

    while( c && c->i && c->i != i )
	c = d->drawables->next();

    if ( c && c->i == i ) {
	int y = c->y - contentsY();
	if ( y + c->i->height() >= 0 &&
	     y < ((TQListView *)this)->visibleHeight() ) {
	    TQRect r( -contentsX(), y, d->h->width(), i->height() );
	    return r;
	}
    }

    return TQRect( 0, 0, -1, -1 );
}


/*!
    \fn void TQListView::doubleClicked( TQListViewItem *item )

    \obsolete (use doubleClicked( TQListViewItem *, const TQPoint&, int ))

    This signal is emitted whenever an item is double-clicked. It's
    emitted on the second button press, not the second button release.
    \a item is the list view item on which the user did the
    double-click.
*/

/*!
    \fn void TQListView::doubleClicked( TQListViewItem *, const TQPoint&, int  )

    This signal is emitted whenever an item is double-clicked. It's
    emitted on the second button press, not the second button release.
    The arguments are the relevant TQListViewItem (may be 0), the point
    in global coordinates and the relevant column (or -1 if the click
    was outside the list).

    \warning Do not delete any TQListViewItem objects in slots
    connected to this signal.
*/


/*!
    \fn void TQListView::returnPressed( TQListViewItem * )

    This signal is emitted when Enter or Return is pressed. The
    argument is the currentItem().
*/

/*!
    \fn void TQListView::spacePressed( TQListViewItem * )

    This signal is emitted when Space is pressed. The argument is
    the currentItem().
*/


/*!
    Sets the list view to be sorted by column \a column in ascending
    order if \a ascending is TRUE or descending order if it is FALSE.

    If \a column is -1, sorting is disabled and the user cannot sort
    columns by clicking on the column headers. If \a column is larger
    than the number of columns the user must click on a column
    header to sort the list view.
*/

void TQListView::setSorting( int column, bool ascending )
{
    if ( column == -1 )
	column = Unsorted;

    if ( d->sortcolumn == column && d->ascending == ascending )
	return;

    d->ascending = ascending;
    d->sortcolumn = column;
    if ( d->sortcolumn != Unsorted && d->sortIndicator )
	d->h->setSortIndicator( d->sortcolumn, d->ascending );
    else
	d->h->setSortIndicator( -1 );

    triggerUpdate();

#if defined(QT_ACCESSIBILITY_SUPPORT)
    TQAccessible::updateAccessibility( viewport(), 0, TQAccessible::ObjectReorder );
#endif
}

/*!
    \enum TQt::SortOrder

    This enum describes how the items in a widget are sorted.

    \value Ascending The items are sorted ascending e.g. starts with
    'AAA' ends with 'ZZZ' in Latin-1 locales

    \value Descending The items are sorted descending e.g. starts with
    'ZZZ' ends with 'AAA' in Latin-1 locales
*/

/*!
    Sets the \a column the list view is sorted by.

    Sorting is triggered by choosing a header section.
*/

void TQListView::changeSortColumn( int column )
{
    if ( isRenaming() ) {
	if ( d->defRenameAction == TQListView::Reject ) {
	    currentItem()->cancelRename( currentItem()->renameCol );
	} else {
	    currentItem()->okRename( currentItem()->renameCol );
	}
    }
    if ( d->sortcolumn != Unsorted ) {
	int lcol = d->h->mapToLogical( column );
	setSorting( lcol, d->sortcolumn == lcol ? !d->ascending : TRUE);
    }
}

/*!
  \internal
  Handles renaming when sections are being swapped by the user.
*/

void TQListView::handleIndexChange()
{
    if ( isRenaming() ) {
        if ( d->defRenameAction == TQListView::Reject ) {
            currentItem()->cancelRename( currentItem()->renameCol );
        } else {
            currentItem()->okRename( currentItem()->renameCol );
        }
    }
    triggerUpdate();
}

/*!
    Returns the column by which the list view is sorted, or -1 if
    sorting is disabled.

    \sa sortOrder()
*/

int TQListView::sortColumn() const
{
    return d->sortcolumn == Unsorted ? -1 : d->sortcolumn;
}

/*!
    Sets the sorting column for the list view.

    If \a column is -1, sorting is disabled and the user cannot sort
    columns by clicking on the column headers. If \a column is larger
    than the number of columns the user must click on a column header
    to sort the list view.

    \sa setSorting()
*/
void TQListView::setSortColumn( int column )
{
    setSorting( column, d->ascending );
}

/*!
    Returns the sorting order of the list view items.

    \sa sortColumn()
*/
TQt::SortOrder TQListView::sortOrder() const
{
    if ( d->ascending )
	return Ascending;
    return Descending;
}

/*!
    Sets the sort order for the items in the list view to \a order.

    \sa setSorting()
*/
void TQListView::setSortOrder( SortOrder order )
{
    setSorting( d->sortcolumn, order == Ascending ? TRUE : FALSE );
}

/*!
    Sorts the list view using the last sorting configuration (sort
    column and ascending/descending).
*/

void TQListView::sort()
{
    if ( d->r )
	d->r->sort();
}

/*!
    \property TQListView::itemMargin
    \brief the advisory item margin that list items may use

    The item margin defaults to one pixel and is the margin between
    the item's edges and the area where it draws its contents.
    TQListViewItem::paintFocus() draws in the margin.

    \sa TQListViewItem::paintCell()
*/

void TQListView::setItemMargin( int m )
{
    if ( d->margin == m )
	return;
    d->margin = m;
    if ( isVisible() ) {
	if ( d->drawables )
	    d->drawables->clear();
	triggerUpdate();
    }
}

int TQListView::itemMargin() const
{
    return d->margin;
}


/*!
    \fn void TQListView::rightButtonClicked( TQListViewItem *, const TQPoint&, int )

    This signal is emitted when the right button is clicked (i.e. when
    it's released). The arguments are the relevant TQListViewItem (may
    be 0), the point in global coordinates and the relevant column (or
    -1 if the click was outside the list).
*/


/*!
    \fn void TQListView::rightButtonPressed (TQListViewItem *, const TQPoint &, int)

    This signal is emitted when the right button is pressed. The
    arguments are the relevant TQListViewItem (may be 0), the point in
    global coordinates and the relevant column (or -1 if the click was
    outside the list).
*/

/*!
    \fn void TQListView::contextMenuRequested( TQListViewItem *item, const TQPoint & pos, int col )

    This signal is emitted when the user invokes a context menu with
    the right mouse button or with special system keys. If the
    keyboard was used \a item is the current item; if the mouse was
    used, \a item is the item under the mouse pointer or 0 if there is
    no item under the mouse pointer. If no item is clicked, the column
    index emitted is -1.

    \a pos is the position for the context menu in the global
    coordinate system.

    \a col is the column on which the user pressed, or -1 if the
    signal was triggered by a key event.
*/

/*!
    \reimp
*/
void TQListView::styleChange( TQStyle& old )
{
    TQScrollView::styleChange( old );
    reconfigureItems();
}


/*!
    \reimp
*/
void TQListView::setFont( const TQFont & f )
{
    TQScrollView::setFont( f );
    reconfigureItems();
}


/*!
    \reimp
*/
void TQListView::setPalette( const TQPalette & p )
{
    TQScrollView::setPalette( p );
    reconfigureItems();
}


/*!
    Ensures that setup() is called for all currently visible items,
    and that it will be called for currently invisible items as soon
    as their parents are opened.

    (A visible item, here, is an item whose parents are all open. The
    item may happen to be off-screen.)

    \sa TQListViewItem::setup()
*/

void TQListView::reconfigureItems()
{
    d->fontMetricsHeight = fontMetrics().height();
    d->minLeftBearing = fontMetrics().minLeftBearing();
    d->minRightBearing = fontMetrics().minRightBearing();
    d->ellipsisWidth = fontMetrics().width( "..." ) * 2;
    d->r->setOpen( FALSE );
    d->r->configured = FALSE;
    d->r->setOpen( TRUE );
}

/*!
    Ensures that the width mode of column \a c is updated according to
    the width of \a item.
*/

void TQListView::widthChanged( const TQListViewItem* item, int c )
{
    if ( c >= d->h->count() )
	return;


    TQFontMetrics fm = fontMetrics();
    int col = c < 0 ? 0 : c;
    while ( col == c || ( c < 0 && col < d->h->count() ) ) {
	if ( d->column[col]->wmode == Maximum ) {
	    int w = item->width( fm, this, col );
	    if ( showSortIndicator() ) {
            int tw = d->h->sectionSizeHint( col, fm ).width();
            tw += 40; //add space for the sort indicator
            w = TQMAX( w, tw );
	    }
	    if ( col == 0 ) {
		int indent = treeStepSize() * item->depth();
		if ( rootIsDecorated() )
		    indent += treeStepSize();
		w += indent;
	    }
	    if ( w > columnWidth( col ) && !d->h->isStretchEnabled() && !d->h->isStretchEnabled( col ) ) {
		d->updateHeader = TRUE;
		setColumnWidth( col, w );
	    }
	}
	col++;
    }
}

/*!
    \property TQListView::allColumnsShowFocus
    \brief whether items should show keyboard focus using all columns

    If this property is TRUE all columns will show focus and selection
    states, otherwise only column 0 will show focus.

    The default is FALSE.

    Setting this to TRUE if it's not necessary may cause noticeable
    flicker.
*/

void TQListView::setAllColumnsShowFocus( bool enable )
{
    d->allColumnsShowFocus = enable;
}

bool TQListView::allColumnsShowFocus() const
{
    return d->allColumnsShowFocus;
}


/*!
    Returns the first item in this TQListView. Returns 0 if there is no
    first item.

    A list view's items can be traversed using firstChild()
    and nextSibling() or using a TQListViewItemIterator.

    \sa itemAt() TQListViewItem::itemBelow() TQListViewItem::itemAbove()
*/

TQListViewItem * TQListView::firstChild() const
{
    if ( !d->r )
	return 0;

    d->r->enforceSortOrder();
    return d->r->childItem;
}

/*!
    Returns the last item in the list view tree. Returns 0 if there
    are no items in the TQListView.

    This function is slow because it traverses the entire tree to find
    the last item.
*/

TQListViewItem* TQListView::lastItem() const
{
    TQListViewItem* item = firstChild();
    if ( item ) {
	while ( item->nextSibling() || item->firstChild() ) {
	    if ( item->nextSibling() )
		item = item->nextSibling();
	    else
		item = item->firstChild();
	}
    }
    return item;
}

/*!
    Repaints this item on the screen if it is currently visible.
*/

void TQListViewItem::repaint() const
{
    TQListView *lv = listView();
    if ( lv )
	lv->repaintItem( this );
}


/*!
    Repaints \a item on the screen if \a item is currently visible.
    Takes care to avoid multiple repaints.
*/

void TQListView::repaintItem( const TQListViewItem * item ) const
{
    if ( !item )
	return;
    d->dirtyItemTimer->start( 0, TRUE );
    if ( !d->dirtyItems )
	d->dirtyItems = new TQPtrDict<void>();
    d->dirtyItems->replace( (void *)item, (void *)item );
}


struct TQCheckListItemPrivate
{
    TQCheckListItemPrivate():
	exclusive( 0 ),
	currentState( TQCheckListItem::Off ),
	statesDict( 0 ),
	tristate( FALSE ) {}

    TQCheckListItem *exclusive;
    TQCheckListItem::ToggleState currentState;
    TQPtrDict<TQCheckListItem::ToggleState> *statesDict;
    bool tristate;
};


/*!
    \class TQCheckListItem
    \brief The TQCheckListItem class provides checkable list view items.

    \ingroup advanced

    TQCheckListItems are used in \l{TQListView}s to provide
    \l{TQListViewItem}s that are checkboxes, radio buttons or
    controllers.

    Checkbox and controller check list items may be inserted at any
    level in a list view. Radio button check list items must be
    children of a controller check list item.

    The item can be checked or unchecked with setOn(). Its type can be
    retrieved with type() and its text retrieved with text().

    \img qlistviewitems.png List View Items

    \sa TQListViewItem TQListView
*/

// ### obscenity is warranted.

/*!
    \enum TQCheckListItem::Type

    This enum type specifies a TQCheckListItem's type:

    \value RadioButton
    \value CheckBox
    \value Controller \e obsolete (use \c RadioButtonController instead)
    \value RadioButtonController
    \value CheckBoxController
*/

/*!
    \enum TQCheckListItem::ToggleState

    This enum specifies a TQCheckListItem's toggle state.

    \value Off
    \value NoChange
    \value On
*/


/*!
    Constructs a checkable item with parent \a parent, text \a text
    and of type \a tt. Note that a \c RadioButton must be the child of a
    \c RadioButtonController, otherwise it will not toggle.
*/
TQCheckListItem::TQCheckListItem( TQCheckListItem *parent, const TQString &text,
				Type tt )
    : TQListViewItem( parent, text, TQString::null )
{
    myType = tt;
    init();
    if ( myType == RadioButton ) {
	if ( parent->type() != RadioButtonController )
	    tqWarning( "TQCheckListItem::TQCheckListItem(), radio button must be "
		      "child of a controller" );
	else
	    d->exclusive = parent;
    }
}

/*!
    Constructs a checkable item with parent \a parent, which is after
    \a after in the parent's list of children, and with text \a text
    and of type \a tt. Note that a \c RadioButton must be the child of
    a \c RadioButtonController, otherwise it will not toggle.
*/
TQCheckListItem::TQCheckListItem( TQCheckListItem *parent, TQListViewItem *after,
 				const TQString &text, Type tt )
    : TQListViewItem( parent, after, text )
{
    myType = tt;
    init();
    if ( myType == RadioButton ) {
 	if ( parent->type() != RadioButtonController )
 	    tqWarning( "TQCheckListItem::TQCheckListItem(), radio button must be "
		      "child of a controller" );
 	else
 	    d->exclusive = parent;
    }
}

/*!
    Constructs a checkable item with parent \a parent, text \a text
    and of type \a tt. Note that this item must \e not be a \c
    RadioButton. Radio buttons must be children of a \c
    RadioButtonController.
*/
TQCheckListItem::TQCheckListItem( TQListViewItem *parent, const TQString &text,
				Type tt )
    : TQListViewItem( parent, text, TQString::null )
{
    myType = tt;
    if ( myType == RadioButton ) {
      tqWarning( "TQCheckListItem::TQCheckListItem(), radio button must be "
	       "child of a TQCheckListItem" );
    }
    init();
}

/*!
    Constructs a checkable item with parent \a parent, which is after
    \a after in the parent's list of children, with text \a text and
    of type \a tt. Note that this item must \e not be a \c
    RadioButton. Radio buttons must be children of a \c
    RadioButtonController.
*/
TQCheckListItem::TQCheckListItem( TQListViewItem *parent, TQListViewItem *after,
 				const TQString &text, Type tt )
    : TQListViewItem( parent, after, text )
{
    myType = tt;
    if ( myType == RadioButton ) {
 	tqWarning( "TQCheckListItem::TQCheckListItem(), radio button must be "
 		  "child of a TQCheckListItem" );
    }
    init();
}


/*!
    Constructs a checkable item with parent \a parent, text \a text
    and of type \a tt. Note that \a tt must \e not be \c RadioButton.
    Radio buttons must be children of a \c RadioButtonController.
*/
TQCheckListItem::TQCheckListItem( TQListView *parent, const TQString &text,
				Type tt )
    : TQListViewItem( parent, text )
{
    myType = tt;
    if ( tt == RadioButton )
	tqWarning( "TQCheckListItem::TQCheckListItem(), radio button must be "
		 "child of a TQCheckListItem" );
    init();
}

/*!
    Constructs a checkable item with parent \a parent, which is after
    \a after in the parent's list of children, with text \a text and
    of type \a tt. Note that \a tt must \e not be \c RadioButton.
    Radio buttons must be children of a \c RadioButtonController.
*/
TQCheckListItem::TQCheckListItem( TQListView *parent, TQListViewItem *after,
 				const TQString &text, Type tt )
    : TQListViewItem( parent, after, text )
{
    myType = tt;
    if ( tt == RadioButton )
 	tqWarning( "TQCheckListItem::TQCheckListItem(), radio button must be "
 		  "child of a TQCheckListItem" );
    init();
}


int TQCheckListItem::RTTI = 1;

/* \reimp */

int TQCheckListItem::rtti() const
{
    return RTTI;
}

/*!
    Constructs a \c RadioButtonController item with parent \a parent,
    text \a text and pixmap \a p.
*/
TQCheckListItem::TQCheckListItem( TQListView *parent, const TQString &text,
				const TQPixmap & p )
    : TQListViewItem( parent, text )
{
    myType = RadioButtonController;
    setPixmap( 0, p );
    init();
}

/*!
    Constructs a \c RadioButtonController item with parent \a parent,
    text \a text and pixmap \a p.
*/
TQCheckListItem::TQCheckListItem( TQListViewItem *parent, const TQString &text,
				const TQPixmap & p )
    : TQListViewItem( parent, text )
{
    myType = RadioButtonController;
    setPixmap( 0, p );
    init();
}

void TQCheckListItem::init()
{
    d = new TQCheckListItemPrivate();
    on = FALSE; // ### remove on ver 4
    if ( myType == CheckBoxController || myType == CheckBox ) {
	d->statesDict = new TQPtrDict<ToggleState>(101);
	d->statesDict->setAutoDelete( TRUE );
    }
    // CheckBoxControllers by default have tristate set to TRUE
    if ( myType == CheckBoxController )
	setTristate( TRUE );
}

/*!
    Destroys the item, and all its children to any depth, freeing up
    all allocated resources.
*/
TQCheckListItem::~TQCheckListItem()
{
    if ( myType == RadioButton
	 && d->exclusive && d->exclusive->d
	 && d->exclusive->d->exclusive == this )
	d->exclusive->turnOffChild();
    d->exclusive = 0; // so the children won't try to access us.
    if ( d->statesDict )
	delete d->statesDict;
    delete d;
    d = 0;
}

/*!
    \fn TQCheckListItem::Type TQCheckListItem::type() const

    Returns the type of this item.
*/

/*!
   \fn  bool TQCheckListItem::isOn() const

    Returns TRUE if the item is toggled on; otherwise returns FALSE.
*/

/*!
   Sets tristate to \a b if the \c Type is either a \c CheckBoxController or
   a \c CheckBox.

   \c CheckBoxControllers are tristate by default.

   \sa state() isTristate()
*/
void TQCheckListItem::setTristate( bool b )
{
    if ( ( myType != CheckBoxController ) && ( myType != CheckBox ) ) {
	tqWarning( "TQCheckListItem::setTristate(), has no effect on RadioButton "
		  "or RadioButtonController." );
    	return;
    }
    d->tristate = b;
}

/*!
   Returns TRUE if the item is tristate; otherwise returns FALSE.

   \sa setTristate()
*/
bool TQCheckListItem::isTristate() const
{
    return d->tristate;
}

/*!
    Returns the state of the item.

    \sa TQCheckListItem::ToggleState
*/
TQCheckListItem::ToggleState TQCheckListItem::state() const
{
    if ( !isTristate() && internalState() == NoChange )
	return Off;
    else
	return d->currentState;
}

/*
  Same as the public state() except this one does not mask NoChange into Off
  when tristate is disabled.
*/
TQCheckListItem::ToggleState TQCheckListItem::internalState() const
{
    return d->currentState;
}




/*!
    Sets the toggle state of the checklistitem to \a s. \a s can be
    \c Off, \c NoChange or \c On.

    Tristate can only be enabled for \c CheckBox or \c CheckBoxController,
    therefore the \c NoChange only applies to them.

    Setting the state to \c On or \c Off on a \c CheckBoxController
    will recursivly set the states of its children to the same state.

    Setting the state to \c NoChange on a \c CheckBoxController will
    make it recursivly recall the previous stored state of its
    children. If there was no previous stored state the children are
    all set to \c On.
*/
void TQCheckListItem::setState( ToggleState s )
{
    if ( myType == CheckBoxController && state() == NoChange )
	updateStoredState( (void*) this );
    setState( s, TRUE, TRUE );
}

/*
  Sets the toggle state of the checklistitems. \a update tells if the
  controller / parent controller should be aware of these changes, \a store
  tells if the parent should store its children if certain conditions arise
*/
void TQCheckListItem::setState( ToggleState s, bool update, bool store)
{

    if ( s == internalState() )
	return;

    if ( myType == CheckBox ) {
	setCurrentState( s );
	stateChange( state() );
  	if ( update && parent() && parent()->rtti() == 1
  	     && ((TQCheckListItem*)parent())->type() == CheckBoxController )
  	    ((TQCheckListItem*)parent())->updateController( update, store );
    } else if ( myType == CheckBoxController ) {
	if ( s == NoChange && childCount()) {
	    restoreState( (void*) this );
	} else {
	    TQListViewItem *item = firstChild();
	    int childCount = 0;
	    while( item ) {
		if ( item->rtti() == 1 &&
		     ( ((TQCheckListItem*)item)->type() == CheckBox ||
		       ((TQCheckListItem*)item)->type() == CheckBoxController ) ) {
		    TQCheckListItem *checkItem = (TQCheckListItem*)item;
		    checkItem->setState( s, FALSE, FALSE );
		    childCount++;
		}
		item = item->nextSibling();
	    }
	    if ( update ) {
		if ( childCount > 0 ) {
		    ToggleState oldState = internalState();
		    updateController( FALSE, FALSE );
		    if ( oldState != internalState() &&
			 parent() && parent()->rtti() == 1 &&
			 ((TQCheckListItem*)parent())->type() == CheckBoxController )
			((TQCheckListItem*)parent())->updateController( update, store );

			updateController( update, store );
		} else {
		    // if there are no children we simply set the CheckBoxController and update its parent
		    setCurrentState( s );
		    stateChange( state() );
		    if ( parent() && parent()->rtti() == 1
			 && ((TQCheckListItem*)parent())->type() == CheckBoxController )
			((TQCheckListItem*)parent())->updateController( update, store );
		}
	    } else {
		setCurrentState( s );
		stateChange( state() );
	    }

	}
    } else if ( myType == RadioButton ) {
	if ( s == On ) {
	    if ( d->exclusive && d->exclusive->d->exclusive != this )
		d->exclusive->turnOffChild();
	    setCurrentState( s );
	    if ( d->exclusive )
		d->exclusive->d->exclusive = this;
	} else {
	    if ( d->exclusive && d->exclusive->d->exclusive == this )
		d->exclusive->d->exclusive = 0;
	    setCurrentState( Off );
	}
	stateChange( state() );
    }
    repaint();
}

/*
  this function is needed becase we need to update "on" everytime
  we update d->currentState. In order to retain binary compatibility
  the inline function isOn() needs the "on" bool
  ### should be changed in ver 4
*/
void TQCheckListItem::setCurrentState( ToggleState s )
{
    ToggleState old = d->currentState;
    d->currentState = s;
    if (d->currentState == On)
	on = TRUE;
    else
	on = FALSE;

#if defined(QT_ACCESSIBILITY_SUPPORT)
    if ( old != d->currentState && listView() )
	TQAccessible::updateAccessibility( listView()->viewport(), indexOfItem( this ), TQAccessible::StateChanged );
#else
    Q_UNUSED( old );
#endif
}



/*
  updates the internally stored state of this item for the parent (key)
*/
void TQCheckListItem::setStoredState( ToggleState newState, void *key )
{
    if ( myType == CheckBox || myType == CheckBoxController )
	d->statesDict->replace( key, new ToggleState(newState) );
}

/*
  Returns the stored state for this item for the given key.
  If the key is not found it returns Off.
*/
TQCheckListItem::ToggleState TQCheckListItem::storedState( void *key ) const
{
    if ( !d->statesDict )
	return Off;

    ToggleState *foundState = d->statesDict->find( key );
    if ( foundState )
	return ToggleState( *foundState );
    else
	return Off;
}


/*!
    \fn TQString TQCheckListItem::text() const

    Returns the item's text.
*/


/*!
    If this is a \c RadioButtonController that has \c RadioButton
    children, turn off the child that is on.
*/
void TQCheckListItem::turnOffChild()
{
    if ( myType == RadioButtonController && d->exclusive )
	d->exclusive->setOn( FALSE );
}

/*!
    Toggle check box or set radio button to on.
*/
void TQCheckListItem::activate()
{
    TQListView * lv = listView();

    if ( ( lv && !lv->isEnabled() ) || !isEnabled() )
	return;

    TQPoint pos;
    int boxsize = lv->style().pixelMetric(TQStyle::PM_CheckListButtonSize, lv);
    if ( activatedPos( pos ) ) {
	bool parentControl = FALSE;
	if ( parent() && parent()->rtti() == 1  &&
	    ((TQCheckListItem*) parent())->type() == RadioButtonController )
	    parentControl = TRUE;

	int x = parentControl ? 0 : 3;
	int align = lv->columnAlignment( 0 );
	int marg = lv->itemMargin();
	int y = 0;

	if ( align & AlignVCenter )
	    y = ( ( height() - boxsize ) / 2 ) + marg;
	else
	    y = (lv->fontMetrics().height() + 2 + marg - boxsize) / 2;

	TQRect r( x, y, boxsize-3, boxsize-3 );
	// columns might have been swapped
	r.moveBy( lv->header()->sectionPos( 0 ), 0 );
	if ( !r.contains( pos ) )
	    return;
    }
    if ( ( myType == CheckBox ) || ( myType == CheckBoxController) )  {
	switch ( internalState() ) {
	case On:
	    setState( Off );
	    break;
	case Off:
	    if ( (!isTristate() && myType == CheckBox) ||
                 (myType == CheckBoxController && !childCount()) ) {
		setState( On );
	    } else {
		setState( NoChange );
		if ( myType == CheckBoxController && internalState() != NoChange )
		    setState( On );
	    }
	    break;
	case NoChange:
	    setState( On );
	    break;
	}
	ignoreDoubleClick();
    } else if ( myType == RadioButton ) {
	setOn( TRUE );
	ignoreDoubleClick();
    }
}

/*!
    Sets the button on if \a b is TRUE, otherwise sets it off.
    Maintains radio button exclusivity.
*/
void TQCheckListItem::setOn( bool b  )
{
    if ( b )
	setState( On , TRUE, TRUE );
    else
	setState( Off , TRUE, TRUE );
}


/*!
    This virtual function is called when the item changes its state.
    \c NoChange (if tristate is enabled and the type is either \c
    CheckBox or \c CheckBoxController) reports the same as \c Off, so
    use state() to determine if the state is actually \c Off or \c
    NoChange.
*/
void TQCheckListItem::stateChange( bool )
{
}

/*
  Calls the public virtual function if the state is changed to either On, NoChange or Off.
  NoChange reports the same as Off - ### should be fixed in ver4
*/
void TQCheckListItem::stateChange( ToggleState s )
{
    stateChange( s == On );
}

/*
  sets the state of the CheckBox and CheckBoxController back to
  previous stored state
*/
void TQCheckListItem::restoreState( void *key, int depth )
{
    switch ( type() ) {
    case CheckBox:
	setCurrentState( storedState( key ) );
	stateChange( state() );
	repaint();
	break;
    case CheckBoxController: {
	TQListViewItem *item = firstChild();
	int childCount = 0;
	while ( item ) {
	    // recursively calling restoreState for children of type CheckBox and CheckBoxController
	    if ( item->rtti() == 1 &&
		 ( ((TQCheckListItem*)item)->type() == CheckBox ||
		   ((TQCheckListItem*)item)->type() == CheckBoxController ) ) {
		((TQCheckListItem*)item)->restoreState( key , depth+1 );
		childCount++;
	    }
	    item = item->nextSibling();
	}
	if ( childCount > 0 ) {
	    if ( depth == 0 )
		updateController( TRUE );
	    else
		updateController( FALSE );
	} else {
	    // if there are no children we retrieve the CheckBoxController state directly.
	    setState( storedState( key ), TRUE, FALSE );
	}
    }
	break;
    default:
	break;
    }
}


/*
  Checks the childrens state and updates the controllers state
  if necessary. If the controllers state change, then his parent again is
  called to update itself.
*/
void TQCheckListItem::updateController( bool update , bool store )
{
    if ( myType != CheckBoxController )
	return;

    TQCheckListItem *controller = 0;
    // checks if this CheckBoxController has another CheckBoxController as parent
    if ( parent() && parent()->rtti() == 1
	 && ((TQCheckListItem*)parent())->type() == CheckBoxController )
	controller = (TQCheckListItem*)parent();

    ToggleState theState = Off;
    bool first = TRUE;
    TQListViewItem *item = firstChild();
    while( item && theState != NoChange ) {
	if ( item->rtti() == 1 &&
	     ( ((TQCheckListItem*)item)->type() == CheckBox ||
	       ((TQCheckListItem*)item)->type() == CheckBoxController ) ) {
	    TQCheckListItem *checkItem = (TQCheckListItem*)item;
	    if ( first ) {
		theState = checkItem->internalState();
		first = FALSE;
	    } else {
		if ( checkItem->internalState() == NoChange ||
		     theState != checkItem->internalState() )
		    theState = NoChange;
		else
		    theState = checkItem->internalState();
	    }
	}
	item = item->nextSibling();
    }
    if ( internalState() != theState ) {
	setCurrentState( theState );
	if ( store && ( internalState() == On || internalState() == Off ) )
	    updateStoredState( (void*) this );
	stateChange( state() );
	if ( update && controller ) {
	    controller->updateController( update, store );
	}
	repaint();
    }
}


/*
  Makes all the children CheckBoxes update their storedState
*/
void TQCheckListItem::updateStoredState( void *key )
{
    if ( myType != CheckBoxController )
	return;

    TQListViewItem *item = firstChild();
    while( item ) {
	if ( item->rtti() == 1 ) {
	    TQCheckListItem *checkItem = (TQCheckListItem*)item;
	    if ( checkItem->type() == CheckBox )
		checkItem->setStoredState( checkItem->internalState(), key );
	    else if (checkItem->type() == CheckBoxController )
		checkItem->updateStoredState( key );
	}
	item = item->nextSibling();
    }
    // this state is only needed if the CheckBoxController has no CheckBox / CheckBoxController children.
    setStoredState( internalState() , key );
}


/*!
    \reimp
*/
void TQCheckListItem::setup()
{
    TQListViewItem::setup();
    int h = height();
    TQListView *lv = listView();
    if ( lv )
	h = TQMAX( lv->style().pixelMetric(TQStyle::PM_CheckListButtonSize, lv),
		  h );
    h = TQMAX( h, TQApplication::globalStrut().height() );
    setHeight( h );
}

/*!
    \reimp
*/

int TQCheckListItem::width( const TQFontMetrics& fm, const TQListView* lv, int column) const
{
    int r = TQListViewItem::width( fm, lv, column );
    if ( column == 0 ) {
	r += lv->itemMargin();
	if ( myType == RadioButtonController && pixmap( 0 ) ) {
	    //	     r += 0;
	} else {
	    r +=  lv->style().pixelMetric(TQStyle::PM_CheckListButtonSize, lv) + 4;
	}
    }
    return TQMAX( r, TQApplication::globalStrut().width() );
}

/*!
    Paints the item using the painter \a p and the color group \a cg.
    The item is in column \a column, has width \a width and has
    alignment \a align. (See TQt::AlignmentFlags for valid alignments.)
*/
void TQCheckListItem::paintCell( TQPainter * p, const TQColorGroup & cg,
			       int column, int width, int align )
{
    if ( !p )
	return;

    TQListView *lv = listView();
    if ( !lv )
	return;

    const BackgroundMode bgmode = lv->viewport()->backgroundMode();
    const TQColorGroup::ColorRole crole = TQPalette::backgroundRoleFromMode( bgmode );
    if ( cg.brush( crole ) != lv->colorGroup().brush( crole ) )
	p->fillRect( 0, 0, width, height(), cg.brush( crole ) );
    else
	lv->paintEmptyArea( p, TQRect( 0, 0, width, height() ) );

    if ( column != 0 ) {
	// The rest is text, or for subclasses to change.
	TQListViewItem::paintCell( p, cg, column, width, align );
	return;
    }

    bool parentControl = FALSE;
    if ( parent() && parent()->rtti() == 1  &&
	 ((TQCheckListItem*) parent())->type() == RadioButtonController )
	parentControl = TRUE;

    TQFontMetrics fm( lv->fontMetrics() );
    int boxsize = lv->style().pixelMetric( myType == RadioButtonController ? TQStyle::PM_CheckListControllerSize :
					   TQStyle::PM_CheckListButtonSize, lv);
    int marg = lv->itemMargin();
    int r = marg;

    // Draw controller / checkbox / radiobutton ---------------------
    int styleflags = TQStyle::Style_Default;
    if ( internalState() == On ) {
	styleflags |= TQStyle::Style_On;
    } else if ( internalState() == NoChange ) {
	if ( myType == CheckBoxController && !isTristate() )
	    styleflags |= TQStyle::Style_Off;
	else
	    styleflags |= TQStyle::Style_NoChange;
    } else {
	styleflags |= TQStyle::Style_Off;
    }
    if ( isSelected() )
	styleflags |= TQStyle::Style_Selected;
    if ( isEnabled() && lv->isEnabled() )
	styleflags |= TQStyle::Style_Enabled;

    if ( myType == RadioButtonController ) {
	int x = 0;
	if(!parentControl)
	    x += 3;
	if ( !pixmap( 0 ) ) {
	    lv->style().drawPrimitive(TQStyle::PE_CheckListController, p,
				  TQRect(x, 0, boxsize,
					fm.height() + 2 + marg),
				  cg, styleflags, TQStyleOption(this));
	    r += boxsize + 4;
	}
    } else {
	Q_ASSERT( lv ); //###
	int x = 0;
	int y = 0;
	if ( !parentControl )
	    x += 3;
	if ( align & AlignVCenter )
	    y = ( ( height() - boxsize ) / 2 ) + marg;
	else
	    y = (fm.height() + 2 + marg - boxsize) / 2;

	if ( ( myType == CheckBox ) || ( myType == CheckBoxController ) ) {
	    lv->style().drawPrimitive(TQStyle::PE_CheckListIndicator, p,
				      TQRect(x, y, boxsize,
					    fm.height() + 2 + marg),
				      cg, styleflags, TQStyleOption(this));
	} else { //radio button look
	    lv->style().drawPrimitive(TQStyle::PE_CheckListExclusiveIndicator,
					      p, TQRect(x, y, boxsize,
						       fm.height() + 2 + marg),
					      cg, styleflags, TQStyleOption(this));
	}
	r += boxsize + 4;
    }

    // Draw text ----------------------------------------------------
    p->translate( r, 0 );
    p->setPen( TQPen( cg.text() ) );
    TQListViewItem::paintCell( p, cg, column, width - r, align );
}

/*!
    Draws the focus rectangle \a r using the color group \a cg on the
    painter \a p.
*/
void TQCheckListItem::paintFocus( TQPainter *p, const TQColorGroup & cg,
				 const TQRect & r )
{
    bool intersect = TRUE;
    TQListView *lv = listView();
    if ( lv && lv->header()->mapToActual( 0 ) != 0 ) {
	int xdepth = lv->treeStepSize() * ( depth() + ( lv->rootIsDecorated() ? 1 : 0) ) + lv->itemMargin();
	int p = lv->header()->cellPos( lv->header()->mapToActual( 0 ) );
	xdepth += p;
	intersect = r.intersects( TQRect( p, r.y(), xdepth - p + 1, r.height() ) );
    }
    bool parentControl = FALSE;
    if ( parent() && parent()->rtti() == 1  &&
	 ((TQCheckListItem*) parent())->type() == RadioButtonController )
	parentControl = TRUE;
    if ( myType != RadioButtonController && intersect &&
	 (lv->rootIsDecorated() || myType == RadioButton ||
	  (myType == CheckBox && parentControl) ) ) {
	TQRect rect;
	int boxsize = lv->style().pixelMetric(TQStyle::PM_CheckListButtonSize, lv);
	if ( lv->columnAlignment(0) == AlignCenter ) {
	    TQFontMetrics fm( lv->font() );
	    int bx = (lv->columnWidth(0) - (boxsize + fm.width(text())))/2 + boxsize;
	    if ( bx < 0 ) bx = 0;
	    rect.setRect( r.x() + bx + 5, r.y(), r.width() - bx - 5,
			  r.height() );
	} else
	    rect.setRect( r.x() + boxsize + 5, r.y(), r.width() - boxsize - 5,
			  r.height() );
	TQListViewItem::paintFocus(p, cg, rect);
    } else {
	TQListViewItem::paintFocus(p, cg, r);
    }
}

/*!
    \reimp
*/
TQSize TQListView::sizeHint() const
{
    if ( cachedSizeHint().isValid() )
	return cachedSizeHint();

    constPolish();

    if ( !isVisible() && (!d->drawables || d->drawables->isEmpty()) )
	// force the column widths to sanity, if possible
	buildDrawableList();

    TQSize s( d->h->sizeHint() );
    if ( verticalScrollBar()->isVisible() )
	s.setWidth( s.width() + style().pixelMetric(TQStyle::PM_ScrollBarExtent) );
    s += TQSize(frameWidth()*2,frameWidth()*2);
    TQListViewItem * l = d->r;
    while( l && !l->height() )
	l = l->childItem ? l->childItem : l->siblingItem;

    if ( l && l->height() )
	s.setHeight( s.height() + 10 * l->height() );
    else
	s.setHeight( s.height() + 140 );

    if ( s.width() > s.height() * 3 )
	s.setHeight( s.width() / 3 );
    else if ( s.width() *3 < s.height() )
	s.setHeight( s.width() * 3 );

    setCachedSizeHint( s );

    return s;
}


/*!
    \reimp
*/

TQSize TQListView::minimumSizeHint() const
{
    return TQScrollView::minimumSizeHint();
}


/*!
    Sets \a item to be open if \a open is TRUE and \a item is
    expandable, and to be closed if \a open is FALSE. Repaints
    accordingly.

    \sa TQListViewItem::setOpen() TQListViewItem::setExpandable()
*/

void TQListView::setOpen( TQListViewItem * item, bool open )
{
    if ( !item ||
	item->isOpen() == open ||
	(open && !item->childCount() && !item->isExpandable()) )
	return;

    TQListViewItem* nextParent = 0;
    if ( open )
	nextParent = item->itemBelow();

    item->setOpen( open );

    if ( open ) {
	TQListViewItem* lastChild = item;
	TQListViewItem* tmp;
	while ( TRUE ) {
	    tmp = lastChild->itemBelow();
	    if ( !tmp || tmp == nextParent )
		break;
	    lastChild = tmp;
	}
	ensureItemVisible( lastChild );
	ensureItemVisible( item );
    }
    if ( d->drawables )
	d->drawables->clear();
    buildDrawableList();

    TQListViewPrivate::DrawableItem * c = d->drawables->first();

    while( c && c->i && c->i != item )
	c = d->drawables->next();

    if ( c && c->i == item ) {
	d->dirtyItemTimer->start( 0, TRUE );
	if ( !d->dirtyItems )
	    d->dirtyItems = new TQPtrDict<void>();
	while( c && c->i ) {
	    d->dirtyItems->insert( (void *)(c->i), (void *)(c->i) );
	    c = d->drawables->next();
	}
    }
}


/*!
    Identical to \a{item}->isOpen(). Provided for completeness.

    \sa setOpen()
*/

bool TQListView::isOpen( const TQListViewItem * item ) const
{
    return item->isOpen();
}


/*!
    \property TQListView::rootIsDecorated
    \brief whether the list view shows open/close signs on root items

    Open/close signs are small <b>+</b> or <b>-</b> symbols in windows
    style, or arrows in Motif style. The default is FALSE.
*/

void TQListView::setRootIsDecorated( bool enable )
{
    if ( enable != (bool)d->rootIsExpandable ) {
	d->rootIsExpandable = enable;
	if ( isVisible() )
	    triggerUpdate();
    }
}

bool TQListView::rootIsDecorated() const
{
    return d->rootIsExpandable;
}


/*!
    Ensures that item \a i is visible, scrolling the list view
    vertically if necessary and opening (expanding) any parent items
    if this is required to show the item.

    \sa itemRect() TQScrollView::ensureVisible()
*/

void TQListView::ensureItemVisible( const TQListViewItem * i )
{
    if ( !i || !i->isVisible() )
	return;

    TQListViewItem *parent = i->parent();
    while ( parent ) {
	if ( !parent->isOpen() )
	    parent->setOpen( TRUE );
	parent = parent->parent();
    }

    if ( d->r->maybeTotalHeight < 0 )
	updateGeometries();
    int y = itemPos( i );
    int h = i->height();
    if ( isVisible() && y + h > contentsY() + visibleHeight() )
	setContentsPos( contentsX(), y - visibleHeight() + h );
    else if ( !isVisible() || y < contentsY() )
	setContentsPos( contentsX(), y );
}


/*!
    \fn TQString TQCheckListItem::text( int n ) const

    \reimp
*/

/*!
    Returns the TQHeader object that manages this list view's columns.
    Please don't modify the header behind the list view's back.

    You may safely call TQHeader::setClickEnabled(),
    TQHeader::setResizeEnabled(), TQHeader::setMovingEnabled(),
    TQHeader::hide() and all the const TQHeader functions.
*/

TQHeader * TQListView::header() const
{
    return d->h;
}


/*!
    \property TQListView::childCount
    \brief the number of parentless (top-level) TQListViewItem objects in this TQListView

    Holds the current number of parentless (top-level) TQListViewItem
    objects in this TQListView.

    \sa TQListViewItem::childCount()
*/

int TQListView::childCount() const
{
    if ( d->r )
	return d->r->childCount();
    return 0;
}


/*
    Moves this item to just after \a olderSibling. \a olderSibling and
    this object must have the same parent.

    If you need to move an item in the hierarchy use takeItem() and
    insertItem().
*/

void TQListViewItem::moveToJustAfter( TQListViewItem * olderSibling )
{
    if ( parentItem && olderSibling &&
	 olderSibling->parentItem == parentItem && olderSibling != this ) {
	if ( parentItem->childItem == this ) {
	    parentItem->childItem = siblingItem;
	} else {
	    TQListViewItem * i = parentItem->childItem;
	    while( i && i->siblingItem != this )
		i = i->siblingItem;
	    if ( i )
		i->siblingItem = siblingItem;
	}
	siblingItem = olderSibling->siblingItem;
	olderSibling->siblingItem = this;
	parentItem->lsc = Unsorted;
    }
}

/*!
    Move the item to be after item \a after, which must be one of the
    item's siblings. To move an item in the hierarchy, use takeItem()
    and insertItem().

    Note that this function will have no effect if sorting is enabled
    in the list view.
*/

void TQListViewItem::moveItem( TQListViewItem *after )
{
    if ( !after || after == this )
	return;
    if ( parent() != after->parent() ) {
	if ( parentItem )
	    parentItem->takeItem( this );
	if ( after->parentItem ) {
	    int tmpLsc = after->parentItem->lsc;
	    after->parentItem->insertItem( this );
	    after->parentItem->lsc = tmpLsc;
	}
    }
    moveToJustAfter( after );
    TQListView *lv = listView();
    if ( lv )
	lv->triggerUpdate();
}

/*
    Recursively sorts items, from the root to this item.
    (enforceSortOrder() won't work the other way around, as
    documented.)
*/
void TQListViewItem::enforceSortOrderBackToRoot()
{
    if ( parentItem ) {
	parentItem->enforceSortOrderBackToRoot();
	parentItem->enforceSortOrder();
    }
}

/*!
    \reimp
*/
void TQListView::showEvent( TQShowEvent * )
{
    if ( d->drawables )
	d->drawables->clear();
    delete d->dirtyItems;
    d->dirtyItems = 0;
    d->dirtyItemTimer->stop();
    d->fullRepaintOnComlumnChange = TRUE;

    updateGeometries();
}


/*!
    Returns the y coordinate of this item in the list view's
    coordinate system. This function is normally much slower than
    TQListView::itemAt(), but it works for all items whereas
    TQListView::itemAt() normally only works for items on the screen.

    \sa TQListView::itemAt() TQListView::itemRect() TQListView::itemPos()
*/

int TQListViewItem::itemPos() const
{
    TQPtrStack<TQListViewItem> s;
    TQListViewItem * i = (TQListViewItem *)this;
    while( i ) {
	s.push( i );
	i = i->parentItem;
    }

    int a = 0;
    TQListViewItem * p = 0;
    while( s.count() ) {
	i = s.pop();
	if ( p ) {
	    if ( !p->configured ) {
		p->configured = TRUE;
		p->setup(); // ### virtual non-const function called in const
	    }
	    a += p->height();
	    TQListViewItem * s = p->firstChild();
	    while( s && s != i ) {
		a += s->totalHeight();
		s = s->nextSibling();
	    }
	}
	p = i;
    }
    return a;
}


/*!
  \fn void TQListView::removeItem( TQListViewItem * )
  \obsolete

  This function has been renamed takeItem().
*/

/*!
    Removes item \a i from the list view; \a i must be a top-level
    item. The warnings regarding TQListViewItem::takeItem() apply to
    this function, too.

    \sa insertItem()
*/
void TQListView::takeItem( TQListViewItem * i )
{
    if ( d->r )
	d->r->takeItem( i );
}


void TQListView::openFocusItem()
{
    d->autoopenTimer->stop();
    if ( d->focusItem && !d->focusItem->isOpen() ) {
	d->focusItem->setOpen( TRUE );
	d->focusItem->repaint();
    }
}

static const int autoopenTime = 750;

#ifndef QT_NO_DRAGANDDROP

/*! \reimp */

void TQListView::contentsDragEnterEvent( TQDragEnterEvent *e )
{
    d->oldFocusItem = d->focusItem;
    TQListViewItem *i = d->focusItem;
    d->focusItem = itemAt( contentsToViewport( e->pos() ) );
    if ( i )
	i->repaint();
    if ( d->focusItem ) {
	d->autoopenTimer->start( autoopenTime );
	d->focusItem->dragEntered();
	d->focusItem->repaint();
    }
    if ( ( i && i->dropEnabled() && i->acceptDrop( e ) ) || acceptDrops() )
	e->accept();
    else
	e->ignore();
}

/*! \reimp */

void TQListView::contentsDragMoveEvent( TQDragMoveEvent *e )
{
    TQListViewItem *i = d->focusItem;
    d->focusItem = itemAt( contentsToViewport( e->pos() ) );
    if ( i ) {
	if ( i != d->focusItem )
	    i->dragLeft();
	i->repaint();
    }
    if ( d->focusItem ) {
	if ( i != d->focusItem ) {
	    d->focusItem->dragEntered();
	    d->autoopenTimer->stop();
	    d->autoopenTimer->start( autoopenTime );
	}
	d->focusItem->repaint();
    } else {
	d->autoopenTimer->stop();
    }
    if ( ( i && i->dropEnabled() && i->acceptDrop( e ) ) || acceptDrops() )
	e->accept();
    else
	e->ignore();
}

/*! \reimp */

void TQListView::contentsDragLeaveEvent( TQDragLeaveEvent * )
{
    d->autoopenTimer->stop();

    if ( d->focusItem )
	d->focusItem->dragLeft();

    setCurrentItem( d->oldFocusItem );
    d->oldFocusItem = 0;
}

/*! \reimp */

void TQListView::contentsDropEvent( TQDropEvent *e )
{
    d->autoopenTimer->stop();

    setCurrentItem( d->oldFocusItem );
    TQListViewItem *i = itemAt( contentsToViewport( e->pos() ) );
    if ( i && i->dropEnabled() && i->acceptDrop( e ) ) {
	i->dropped( e );
	e->accept();
    } else if ( acceptDrops() ) {
	emit dropped( e );
	e->accept();
    }
}

/*!
    If the user presses the mouse on an item and starts moving the
    mouse, and the item allow dragging (see
    TQListViewItem::setDragEnabled()), this function is called to get a
    drag object and a drag is started unless dragObject() returns 0.

    By default this function returns 0. You should reimplement it and
    create a TQDragObject depending on the selected items.
*/

TQDragObject *TQListView::dragObject()
{
    return 0;
}

/*!
    Starts a drag.
*/

void TQListView::startDrag()
{
    if ( !d->startDragItem )
	return;

    d->startDragItem = 0;
    d->buttonDown = FALSE;

    TQDragObject *drag = dragObject();
    if ( !drag )
	return;

    drag->drag();
}

#endif // QT_NO_DRAGANDDROP

/*!
    \property TQListView::defaultRenameAction
    \brief What action to perform when the editor loses focus during renaming

    If this property is \c Accept, and the user renames an item and
    the editor loses focus (without the user pressing Enter), the
    item will still be renamed. If the property's value is \c Reject,
    the item will not be renamed unless the user presses Enter. The
    default is \c Reject.
*/

void TQListView::setDefaultRenameAction( RenameAction a )
{
    d->defRenameAction = a;
}

TQListView::RenameAction TQListView::defaultRenameAction() const
{
    return d->defRenameAction;
}

/*!
    Returns TRUE if an item is being renamed; otherwise returns FALSE.
*/

bool TQListView::isRenaming() const
{
    return currentItem() && currentItem()->renameBox;
}

/**********************************************************************
 *
 * Class TQListViewItemIterator
 *
 **********************************************************************/


/*!
    \class TQListViewItemIterator
    \brief The TQListViewItemIterator class provides an iterator for collections of TQListViewItems.

    \ingroup advanced

    Construct an instance of a TQListViewItemIterator, with either a
    TQListView* or a TQListViewItem* as argument, to operate on the tree
    of TQListViewItems, starting from the argument.

    A TQListViewItemIterator iterates over all the items from its
    starting point. This means that it always makes the first child of
    the current item the new current item. If there is no child, the
    next sibling becomes the new current item; and if there is no next
    sibling, the next sibling of the parent becomes current.

    The following example creates a list of all the items that have
    been selected by the user, storing pointers to the items in a
    TQPtrList:
    \code
    TQPtrList<TQListViewItem> lst;
    TQListViewItemIterator it( myListView );
    while ( it.current() ) {
	if ( it.current()->isSelected() )
	    lst.append( it.current() );
	++it;
    }
    \endcode

    An alternative approach is to use an \c IteratorFlag:
    \code
    TQPtrList<TQListViewItem> lst;
    TQListViewItemIterator it( myListView, TQListViewItemIterator::Selected );
    while ( it.current() ) {
	lst.append( it.current() );
	++it;
    }
    \endcode

    A TQListViewItemIterator provides a convenient and easy way to
    traverse a hierarchical TQListView.

    Multiple TQListViewItemIterators can operate on the tree of
    TQListViewItems. A TQListView knows about all iterators operating on
    its TQListViewItems. So when a TQListViewItem gets removed all
    iterators that point to this item are updated and point to the
    following item if possible, otherwise to a valid item before the
    current one or to 0. Note however that deleting the parent item of
    an item that an iterator points to is not safe.

    \sa TQListView, TQListViewItem
*/

/*!
    \enum TQListViewItemIterator::IteratorFlag

    These flags can be passed to a TQListViewItemIterator constructor
    (OR-ed together if more than one is used), so that the iterator
    will only iterate over items that match the given flags.

    \value Visible
    \value Invisible
    \value Selected
    \value Unselected
    \value Selectable
    \value NotSelectable
    \value DragEnabled
    \value DragDisabled
    \value DropEnabled
    \value DropDisabled
    \value Expandable
    \value NotExpandable
    \value Checked
    \value NotChecked
*/

/*!
    Constructs an empty iterator.
*/

TQListViewItemIterator::TQListViewItemIterator()
    :  curr( 0 ), listView( 0 )
{
    init( 0 );
}

/*!
    Constructs an iterator for the TQListView that contains the \a
    item. The current iterator item is set to point to the \a item.
*/

TQListViewItemIterator::TQListViewItemIterator( TQListViewItem *item )
    :  curr( item ), listView( 0 )
{
    init( 0 );

    if ( item ) {
	item->enforceSortOrderBackToRoot();
	listView = item->listView();
    }
    addToListView();
}

/*!
    Constructs an iterator for the TQListView that contains the \a item
    using the flags \a iteratorFlags. The current iterator item is set
    to point to \a item or the next matching item if \a item doesn't
    match the flags.

    \sa TQListViewItemIterator::IteratorFlag
*/

TQListViewItemIterator::TQListViewItemIterator( TQListViewItem *item, int iteratorFlags )
    :  curr( item ), listView( 0 )
{
    init( iteratorFlags );

    // go to next matching item if the current don't match
    if ( curr && !matchesFlags( curr ) )
	++( *this );

    if ( curr ) {
	curr->enforceSortOrderBackToRoot();
	listView = curr->listView();
    }
    addToListView();
}


/*!
    Constructs an iterator for the same TQListView as \a it. The
    current iterator item is set to point on the current item of \a
    it.
*/

TQListViewItemIterator::TQListViewItemIterator( const TQListViewItemIterator& it )
    : curr( it.curr ), listView( it.listView )
{
    init(it.d() ? it.d()->flags : 0);

    addToListView();
}

/*!
    Constructs an iterator for the TQListView \a lv. The current
    iterator item is set to point on the first child (TQListViewItem)
    of \a lv.
*/

TQListViewItemIterator::TQListViewItemIterator( TQListView *lv )
    : curr( lv->firstChild() ), listView( lv )
{
    init( 0 );

    addToListView();
}

/*!
    Constructs an iterator for the TQListView \a lv with the flags \a
    iteratorFlags. The current iterator item is set to point on the
    first child (TQListViewItem) of \a lv that matches the flags.

    \sa TQListViewItemIterator::IteratorFlag
*/

TQListViewItemIterator::TQListViewItemIterator( TQListView *lv, int iteratorFlags )
    : curr ( lv->firstChild() ), listView( lv )
{
    init( iteratorFlags );

    addToListView();
    if ( !matchesFlags( curr ) )
	++( *this );
}



/*!
    Assignment. Makes a copy of \a it and returns a reference to its
    iterator.
*/

TQListViewItemIterator &TQListViewItemIterator::operator=( const TQListViewItemIterator &it )
{
    if ( listView ) {
	if ( listView->d->iterators->removeRef( this ) ) {
	    if ( listView->d->iterators->count() == 0 ) {
		delete listView->d->iterators;
		listView->d->iterators = 0;
	    }
	}
    }

    listView = it.listView;
    addToListView();
    curr = it.curr;

    // sets flags to be the same as the input iterators flags
    if ( d() && it.d() )
	d()->flags = it.d()->flags;

    // go to next matching item if the current don't match
    if ( curr && !matchesFlags( curr ) )
	++( *this );

    return *this;
}

/*!
    Destroys the iterator.
*/

TQListViewItemIterator::~TQListViewItemIterator()
{
    if ( listView ) {
	if ( listView->d->iterators->removeRef( this ) ) {
	    if ( listView->d->iterators->count() == 0 ) {
		delete listView->d->iterators;
		listView->d->iterators = 0;
	    }
	}
    }
    // removs the d-ptr from the dict ( autodelete on), and deletes the
    // ptrdict if it becomes empty
    if ( qt_iteratorprivate_dict ) {
	qt_iteratorprivate_dict->remove( this );
	if ( qt_iteratorprivate_dict->isEmpty() ) {
	    delete qt_iteratorprivate_dict;
	    qt_iteratorprivate_dict = 0;
	}
    }
}

/*!
    Prefix ++. Makes the next item the new current item and returns
    it. Returns 0 if the current item is the last item or the
    TQListView is 0.
*/

TQListViewItemIterator &TQListViewItemIterator::operator++()
{
    do {
        if ( !curr )
	    return *this;

	TQListViewItem *item = curr->firstChild();
	if ( !item ) {
	    while ( (item = curr->nextSibling()) == 0  ) {
		curr = curr->parent();
		if ( curr == 0 )
		    break;
	    }
	}
	curr = item;
	// if the next one doesn't match the flags we try one more ahead
    } while  ( curr && !matchesFlags( curr ) );
    return *this;
}

/*!
    \overload

    Postfix ++. Makes the next item the new current item and returns
    the item that \e was the current item.
*/

const TQListViewItemIterator TQListViewItemIterator::operator++( int )
{
    TQListViewItemIterator oldValue = *this;
    ++( *this );
    return oldValue;
}

/*!
    Sets the current item to the item \a j positions after the current
    item. If that item is beyond the last item, the current item is
    set to 0. Returns the current item.
*/

TQListViewItemIterator &TQListViewItemIterator::operator+=( int j )
{
    while ( curr && j-- )
	++( *this );

    return *this;
}

/*!
    Prefix --. Makes the previous item the new current item and
    returns it. Returns 0 if the current item is the first item or the
    TQListView is 0.
*/

TQListViewItemIterator &TQListViewItemIterator::operator--()
{
    if ( !curr )
	return *this;

    if ( !curr->parent() ) {
	// we are in the first depth
       if ( curr->listView() ) {
	    if ( curr->listView()->firstChild() != curr ) {
		// go the previous sibling
		TQListViewItem *i = curr->listView()->firstChild();
		while ( i && i->siblingItem != curr )
		    i = i->siblingItem;

		curr = i;

		if ( i && i->firstChild() ) {
		    // go to the last child of this item
		    TQListViewItemIterator it( curr->firstChild() );
		    for ( ; it.current() && it.current()->parent(); ++it )
			curr = it.current();
		}

		if ( curr && !matchesFlags( curr ) )
		    --( *this );

		return *this;
	    } else {
		//we are already the first child of the list view, so it's over
		curr = 0;
		return *this;
	    }
	} else
	    return *this;
    } else {
	TQListViewItem *parent = curr->parent();

	if ( curr != parent->firstChild() ) {
	    // go to the previous sibling
	    TQListViewItem *i = parent->firstChild();
	    while ( i && i->siblingItem != curr )
		i = i->siblingItem;

	    curr = i;

	    if ( i && i->firstChild() ) {
		// go to the last child of this item
		TQListViewItemIterator it( curr->firstChild() );
		for ( ; it.current() && it.current()->parent() != parent; ++it )
		    curr = it.current();
	    }

	    if ( curr && !matchesFlags( curr ) )
		--( *this );

	    return *this;
	} else {
	    // make our parent the current item
	    curr = parent;

	    if ( curr && !matchesFlags( curr ) )
		--( *this );

	    return *this;
	}
    }
}

/*!
    \overload

    Postfix --. Makes the previous item the new current item and
    returns the item that \e was the current item.
*/

const TQListViewItemIterator TQListViewItemIterator::operator--( int )
{
    TQListViewItemIterator oldValue = *this;
    --( *this );
    return oldValue;
}

/*!
    Sets the current item to the item \a j positions before the
    current item. If that item is before the first item, the current
    item is set to 0. Returns the current item.
*/

TQListViewItemIterator &TQListViewItemIterator::operator-=( int j )
{
    while ( curr && j-- )
	--( *this );

    return *this;
}

/*!
    Dereference operator. Returns a reference to the current item. The
    same as current().
*/

TQListViewItem* TQListViewItemIterator::operator*()
{
    if ( curr != 0 && !matchesFlags( curr ) )
	tqWarning( "TQListViewItemIterator::operator*() curr out of sync" );
    return curr;
}

/*!
    Returns iterator's current item.
*/

TQListViewItem *TQListViewItemIterator::current() const
{
    if ( curr != 0 && !matchesFlags( curr ) )
	tqWarning( "TQListViewItemIterator::current() curr out of sync" );
    return curr;
}

TQListViewItemIteratorPrivate* TQListViewItemIterator::d() const
{
    return qt_iteratorprivate_dict ?
	qt_iteratorprivate_dict->find( (void *)this ) : 0;
}

void TQListViewItemIterator::init( int iteratorFlags )
{
    // makes new global ptrdict if it doesn't exist
    if ( !qt_iteratorprivate_dict ) {
	qt_iteratorprivate_dict = new TQPtrDict<TQListViewItemIteratorPrivate>;
	qt_iteratorprivate_dict->setAutoDelete( TRUE );
    }

    // sets flag, or inserts new TQListViewItemIteratorPrivate with flag
    if ( d() )
	d()->flags = iteratorFlags;
    else
	qt_iteratorprivate_dict->insert( this, new TQListViewItemIteratorPrivate( iteratorFlags ) );
}


/*
    Adds this iterator to its TQListView's list of iterators.
*/

void TQListViewItemIterator::addToListView()
{
    if ( listView ) {
	if ( !listView->d->iterators ) {
	    listView->d->iterators = new TQPtrList<TQListViewItemIterator>;
	    TQ_CHECK_PTR( listView->d->iterators );
	}
	listView->d->iterators->append( this );
    }
}

/*
    This function is called to notify the iterator that the current
    item has been deleted, and sets the current item point to another
    (valid) item or 0.
*/

void TQListViewItemIterator::currentRemoved()
{
    if ( !curr ) return;

    if ( curr->parent() )
	curr = curr->parent();
    else if ( curr->nextSibling() )
	curr = curr->nextSibling();
    else if ( listView && listView->firstChild() &&
	      listView->firstChild() != curr )
	curr = listView->firstChild();
    else
	curr = 0;
}

/*
  returns TRUE if the item \a item matches all of the flags set for the iterator
*/
bool TQListViewItemIterator::matchesFlags( const TQListViewItem *item ) const
{
    if ( !item )
	return FALSE;

    int flags = d() ? d()->flags : 0;

    if ( flags == 0 )
  	return TRUE;

    if ( flags & Visible && !item->isVisible() )
	return FALSE;
    if ( flags & Invisible && item->isVisible() )
	return FALSE;
    if ( flags & Selected && !item->isSelected() )
	return FALSE;
    if ( flags & Unselected && item->isSelected() )
	return FALSE;
    if ( flags & Selectable && !item->isSelectable() )
	return FALSE;
    if ( flags & NotSelectable && item->isSelectable() )
	return FALSE;
    if ( flags & DragEnabled && !item->dragEnabled() )
	return FALSE;
    if ( flags & DragDisabled && item->dragEnabled() )
	return FALSE;
    if ( flags & DropEnabled && !item->dropEnabled() )
	return FALSE;
    if ( flags & DropDisabled && item->dropEnabled() )
	return FALSE;
    if ( flags & Expandable && !item->isExpandable() )
	return FALSE;
    if ( flags & NotExpandable && item->isExpandable() )
	return FALSE;
    if ( flags & Checked && !isChecked( item ) )
	return FALSE;
    if ( flags & NotChecked && isChecked( item ) )
	return FALSE;

    return TRUE;
}

/*
  we want the iterator to check TQCheckListItems as well, so we provide this convenience function
  that checks if the rtti() is 1 which means TQCheckListItem and if isOn is TRUE, returns FALSE otherwise.
*/
bool TQListViewItemIterator::isChecked( const TQListViewItem *item ) const
{
    if ( item->rtti() == 1 )
	return ((const TQCheckListItem*)item)->isOn();
    else return FALSE;
}

void TQListView::handleItemChange( TQListViewItem *old, bool shift, bool control )
{
    if ( d->selectionMode == Single ) {
	// nothing
    } else if ( d->selectionMode == Extended ) {
	if ( shift ) {
	    selectRange( d->selectAnchor ? d->selectAnchor : old,
			 d->focusItem, FALSE, TRUE, (d->selectAnchor && !control) ? TRUE : FALSE );
	} else if ( !control ) {
	    bool block = signalsBlocked();
	    blockSignals( TRUE );
	    selectAll( FALSE );
	    blockSignals( block );
	    setSelected( d->focusItem, TRUE );
	}
    } else if ( d->selectionMode == Multi ) {
	if ( shift )
	    selectRange( old, d->focusItem, TRUE, FALSE );
    }
}

void TQListView::startRename()
{
    if ( !currentItem() )
	return;
    currentItem()->startRename( d->pressedColumn );
    d->buttonDown = FALSE;
}

/* unselects items from to, including children, returns TRUE if any items were unselected */
bool TQListView::clearRange( TQListViewItem *from, TQListViewItem *to, bool includeFirst )
{
    if ( !from || !to )
	return FALSE;

    // Swap
    if ( from->itemPos() > to->itemPos() ) {
	TQListViewItem *temp = from;
	from = to;
	to = temp;
    }

    // Start on second?
    if ( !includeFirst ) {
	TQListViewItem *below = (from == to) ? from : from->itemBelow();
	if ( below )
	    from = below;
    }

    // Clear items <from, to>
    bool changed = FALSE;

    TQListViewItemIterator it( from );
    while ( it.current() ) {
	if ( it.current()->isSelected() ) {
	    it.current()->setSelected( FALSE );
	    changed = TRUE;
	}
	if ( it.current() == to )
	    break;
	++it;
    }

    // NOTE! This function does _not_ emit
    // any signals about selection changed
    return changed;
}

void TQListView::selectRange( TQListViewItem *from, TQListViewItem *to, bool invert, bool includeFirst, bool clearSel )
{
    if ( !from || !to )
	return;
    if ( from == to && !includeFirst )
	return;
    bool swap = FALSE;
    if ( to == from->itemAbove() )
	swap = TRUE;
    if ( !swap && from != to && from != to->itemAbove() ) {
	TQListViewItemIterator it( from );
	bool found = FALSE;
	for ( ; it.current(); ++it ) {
	    if ( it.current() == to ) {
		found = TRUE;
		break;
	    }
	}
	if ( !found )
	    swap = TRUE;
    }
    if ( swap ) {
	TQListViewItem *i = from;
	from = to;
	to = i;
	if ( !includeFirst )
	    to = to->itemAbove();
    } else {
	if ( !includeFirst )
	    from = from->itemBelow();
    }

    bool changed = FALSE;
    if ( clearSel ) {
	TQListViewItemIterator it( firstChild() );
	for ( ; it.current(); ++it ) {
	    if ( it.current()->selected ) {
		it.current()->setSelected( FALSE );
		changed = TRUE;
	    }
	}
	it = TQListViewItemIterator( to );
	for ( ; it.current(); ++it ) {
	    if ( it.current()->selected ) {
		it.current()->setSelected( FALSE );
		changed = TRUE;
	    }
	}
    }

    for ( TQListViewItem *i = from; i; i = i->itemBelow() ) {
	if ( !invert ) {
	    if ( !i->selected && i->isSelectable() ) {
		i->setSelected( TRUE );
		changed = TRUE;
	    }
	} else {
	    bool sel = !i->selected;
	    if ( ( (bool)i->selected != sel && sel && i->isSelectable() ) || !sel ) {
		i->setSelected( sel );
		changed = TRUE;
	    }
	}
	if ( i == to )
	    break;
    }
    if ( changed ) {
	d->useDoubleBuffer = TRUE;
	triggerUpdate();
	emit selectionChanged();
    }
}

/* clears selection from anchor to old, selects from anchor to new, does not emit selectionChanged on change */
bool TQListView::selectRange( TQListViewItem *newItem, TQListViewItem *oldItem, TQListViewItem *anchorItem )
{
    if ( !newItem || !oldItem || !anchorItem )
	return FALSE;

    int  anchorPos = anchorItem ? anchorItem->itemPos() : 0,
	 oldPos    = oldItem ? oldItem->itemPos() : 0,
	 newPos    = newItem->itemPos();
    TQListViewItem *top=0, *bottom=0;
    if ( anchorPos > newPos ) {
	top = newItem;
	bottom = anchorItem;
    } else {
	top = anchorItem;
	bottom = newItem;
    }

    // removes the parts of the old selection that will no longer be selected
    bool changed = FALSE;
    int topPos    = top ? top->itemPos() : 0,
	bottomPos = bottom ? bottom->itemPos() : 0;
    if ( !(oldPos > topPos && oldPos < bottomPos) ) {
	if ( oldPos < topPos )
	    changed = clearRange( oldItem, top );
	else
	    changed = clearRange( bottom, oldItem );
    }

    // selects the new (not already selected) items
    TQListViewItemIterator lit( top );
    for ( ; lit.current(); ++lit ) {
	if ( (bool)lit.current()->selected != d->select ) {
	    lit.current()->setSelected( d->select );
	    changed = TRUE;
	}
	// Include bottom, then break
	if ( lit.current() == bottom )
	    break;
    }

    return changed;
}


/*!
    Finds the first list view item in column \a column, that matches
    \a text and returns the item, or returns 0 of no such item could
    be found.
    The search starts from the current item if the current item exists,
    otherwise it starts from the first list view item. After reaching
    the last item the search continues from the first item.
    Pass OR-ed together \l TQt::StringComparisonMode values
    in the \a compare flag, to control how the matching is performed.
    The default comparison mode is case-sensitive, exact match.
*/

TQListViewItem *TQListView::findItem( const TQString& text, int column,
				    ComparisonFlags compare ) const
{
    if (text.isEmpty() && !(compare & ExactMatch))
	return 0;

    if ( compare == CaseSensitive || compare == 0 )
	compare |= ExactMatch;

    TQString itmtxt;
    TQString comtxt = text;
    if ( !(compare & CaseSensitive) )
	comtxt = comtxt.lower();

    TQListViewItemIterator it( d->focusItem ? d->focusItem : firstChild() );
    TQListViewItem *sentinel = 0;
    TQListViewItem *item;
    TQListViewItem *beginsWithItem = 0;
    TQListViewItem *endsWithItem = 0;
    TQListViewItem *containsItem = 0;

    for ( int pass = 0; pass < 2; pass++ ) {
	while ( (item = it.current()) != sentinel ) {
	    itmtxt = item->text( column );
	    if ( !(compare & CaseSensitive) )
		itmtxt = itmtxt.lower();

	    if ( compare & ExactMatch && itmtxt == comtxt )
		return item;
	    if ( compare & BeginsWith && !beginsWithItem && itmtxt.startsWith( comtxt ) )
		beginsWithItem = containsItem = item;
	    if ( compare & EndsWith && !endsWithItem && itmtxt.endsWith( comtxt ) )
		endsWithItem = containsItem = item;
	    if ( compare & Contains && !containsItem && itmtxt.contains( comtxt ) )
		containsItem = item;
	    ++it;
	}

	it = TQListViewItemIterator( firstChild() );
	sentinel = d->focusItem ? d->focusItem : firstChild();
    }

    // Obey the priorities
    if ( beginsWithItem )
	return beginsWithItem;
    else if ( endsWithItem )
	return endsWithItem;
    else if ( containsItem )
	return containsItem;
    return 0;
}

/*! \reimp */
void TQListView::windowActivationChange( bool oldActive )
{
    if ( oldActive && d->scrollTimer )
	d->scrollTimer->stop();
    if ( palette().active() != palette().inactive() )
	viewport()->update();
    TQScrollView::windowActivationChange( oldActive );
}

/*!
    Hides the column specified at \a column. This is a convenience
    function that calls setColumnWidth( \a column, 0 ).

    Note: The user may still be able to resize the hidden column using
    the header handles. To prevent this, call setResizeEnabled(FALSE,
    \a column) on the list views header.

    \sa setColumnWidth()
*/

void TQListView::hideColumn( int column )
{
    setColumnWidth( column, 0 );
}

/*! Adjusts the column \a col to its preferred width */

void TQListView::adjustColumn( int col )
{
    if ( col < 0 || col > (int)d->column.count() - 1 || d->h->isStretchEnabled( col ) )
	return;

    int oldw = d->h->sectionSize( col );

    int w = d->h->sectionSizeHint( col, fontMetrics() ).width();
    if ( d->h->iconSet( col ) )
	w += d->h->iconSet( col )->pixmap().width();
    w = TQMAX( w, 20 );
    TQFontMetrics fm( fontMetrics() );
    TQListViewItem* item = firstChild();
    int rootDepth = rootIsDecorated() ? treeStepSize() : 0;
    while ( item ) {
	int iw = item->width( fm, this, col );
	if ( 0 == col )
	    iw += itemMargin() + rootDepth + item->depth()*treeStepSize() - 1;
	w = TQMAX( w, iw );
	item = item->itemBelow();
    }
    w = TQMAX( w, TQApplication::globalStrut().width() );

    d->h->adjustHeaderSize( oldw - w );
    if (oldw != w) {
        d->fullRepaintOnComlumnChange = TRUE;
        d->h->resizeSection( col, w );
	emit d->h->sizeChange( col, oldw, w);
    }
}

#endif // QT_NO_LISTVIEW
