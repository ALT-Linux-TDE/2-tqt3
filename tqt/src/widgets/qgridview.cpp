/****************************************************************************
**
** Implementation of TQGridView class
**
** Created : 010523
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


#include "ntqgridview.h"

#ifndef TQT_NO_GRIDVIEW

#include "ntqpainter.h"

/*!
    \class TQGridView ntqgridview.h
    \brief The TQGridView class provides an abstract base for
    fixed-size grids.

    \ingroup abstractwidgets

    A grid view consists of a number of abstract cells organized in
    rows and columns. The cells have a fixed size and are identified
    with a row index and a column index. The top-left cell is in row
    0, column 0. The bottom-right cell is in row numRows()-1, column
    numCols()-1.

    You can define \l numRows, \l numCols, \l cellWidth and \l
    cellHeight. Reimplement the pure virtual function paintCell() to
    draw the contents of a cell.

    With ensureCellVisible(), you can ensure a certain cell is
    visible. With rowAt() and columnAt() you can find a cell based on
    the given x- and y-coordinates.

    If you need to monitor changes to the grid's dimensions (i.e. when
    numRows or numCols is changed), reimplement the dimensionChange()
    change handler.

    Note: the row and column indices are always given in the order,
    row (vertical offset) then column (horizontal offset). This order
    is the opposite of all pixel operations, which are given in the
    order x (horizontal offset), y (vertical offset).

    TQGridView is a very simple abstract class based on TQScrollView. It
    is designed to simplify the task of drawing many cells of the same
    size in a potentially scrollable canvas. If you need rows and
    columns with different sizes, use a TQTable instead. If you need a
    simple list of items, use a TQListBox. If you need to present
    hierachical data use a TQListView, and if you need random objects
    at random positions, consider using either a TQIconView or a
    TQCanvas.
*/


/*!
    Constructs a grid view.

    The \a parent, \a name and widget flag, \a f, arguments are passed
    to the TQScrollView constructor.
*/
TQGridView::TQGridView( TQWidget *parent, const char *name, WFlags f )
    :TQScrollView( parent, name, f | WStaticContents ),
     nrows( 5 ), ncols( 5 ), cellw( 12 ), cellh( 12 )
{
    viewport()->setBackgroundMode( PaletteBase );
    setBackgroundMode( PaletteBackground, PaletteBase );
    viewport()->setFocusProxy( this );
}

/*!
    Destroys the grid view.
*/
TQGridView::~TQGridView()
{
}

void TQGridView::updateGrid()
{
    resizeContents( ncols * cellw, nrows * cellh );
}

/*!
    \property TQGridView::numRows
    \brief The number of rows in the grid

    \sa numCols
*/
void TQGridView::setNumRows( int numRows )
{
    int oldnrows = nrows;
    nrows = numRows;
    dimensionChange( oldnrows, ncols );
    updateGrid();
}

/*!
    \property TQGridView::numCols
    \brief The number of columns in the grid

    \sa numRows
*/
void TQGridView::setNumCols( int numCols )
{
    int oldncols = ncols;
    ncols = numCols;
    dimensionChange( nrows, oldncols );
    updateGrid();
}

/*!
    \property TQGridView::cellWidth
    \brief The width of a grid column

    All columns in a grid view have the same width.

    \sa cellHeight
*/
void TQGridView::setCellWidth( int cellWidth )
{
    cellw = cellWidth;
    updateGrid();
    updateContents();
}

/*!
    \property TQGridView::cellHeight
    \brief The height of a grid row

    All rows in a grid view have the same height.

    \sa cellWidth
*/
void TQGridView::setCellHeight( int cellHeight )
{
    cellh = cellHeight;
    updateGrid();
    updateContents();
}

/*!
    Returns the geometry of cell (\a row, \a column) in the content
    coordinate system.

    \sa cellRect()
 */
TQRect TQGridView::cellGeometry( int row, int column )
{
    TQRect r;
    if ( row >= 0 && row < nrows && column >= 0 && column < ncols )
	r.setRect( cellw * column, cellh * row, cellw, cellh );
    return r;
}

/*!
    Repaints cell (\a row, \a column).

    If \a erase is TRUE, TQt erases the area of the cell before the
    paintCell() call; otherwise no erasing takes place.

    \sa TQWidget::repaint()
*/
void TQGridView::repaintCell( int row, int column, bool erase )
{
    repaintContents( cellGeometry( row, column ), erase );
}

/*!
    Updates cell (\a row, \a column).

    \sa TQWidget::update()
*/
void TQGridView::updateCell( int row, int column )
{
    updateContents( cellGeometry( row, column ) );
}

/*!
    Ensures cell (\a row, \a column) is visible, scrolling the grid
    view if necessary.
*/
void TQGridView::ensureCellVisible( int row, int column )
{
    TQRect r = cellGeometry( row, column );
    ensureVisible( r.x(), r.y(), r.width(), r.height() );
}

/*!
    This function fills the \a cw pixels wide and \a ch pixels high
    rectangle starting at position (\a cx, \a cy) with the background
    color using the painter \a p.

    paintEmptyArea() is invoked by drawContents() to erase or fill
    unused areas.
*/

void TQGridView::paintEmptyArea( TQPainter *p, int cx ,int cy, int cw, int ch)
{
    if ( gridSize().width() >= contentsWidth() && gridSize().height() >= contentsHeight() )
	return;
    // Region of the rect we should draw
    contentsToViewport( cx, cy, cx, cy );
    TQRegion reg( TQRect( cx, cy, cw, ch ) );
    // Subtract the table from it
    reg = reg.subtract( TQRect( contentsToViewport( TQPoint( 0, 0 ) ), gridSize() ) );

    // And draw the rectangles (transformed as needed)
    TQMemArray<TQRect> r = reg.rects();
    const TQBrush &brush = backgroundBrush();
    for ( int i = 0; i < (int)r.count(); ++i)
	p->fillRect( r[ i ], brush );
}

/*!\reimp
 */
void TQGridView::drawContents( TQPainter *p, int cx, int cy, int cw, int ch )
{
    int colfirst = columnAt( cx );
    int collast = columnAt( cx + cw );
    int rowfirst = rowAt( cy );
    int rowlast = rowAt( cy + ch );

    if ( rowfirst == -1 || colfirst == -1 ) {
	paintEmptyArea( p, cx, cy, cw, ch );
	return;
    }

    if ( collast < 0 || collast >= ncols )
	collast = ncols-1;
    if ( rowlast < 0 || rowlast >= nrows )
	rowlast = nrows-1;

    // Go through the rows
    for ( int r = rowfirst; r <= rowlast; ++r ) {
	// get row position and height
	int rowp = r * cellh;

	// Go through the columns in the row r
	// if we know from where to where, go through [colfirst, collast],
	// else go through all of them
	for ( int c = colfirst; c <= collast; ++c ) {
	    // get position and width of column c
	    int colp = c * cellw;
	    // Translate painter and draw the cell
	    p->translate( colp, rowp );
	    paintCell( p, r, c );
	    p->translate( -colp, -rowp );
	}
    }

    // Paint empty rects
    paintEmptyArea( p, cx, cy, cw, ch );
}

/*!
    \reimp

    (Implemented to get rid of a compiler warning.)
*/
void TQGridView::drawContents( TQPainter * )
{
}

/*!
    \fn void TQGridView::dimensionChange( int oldNumRows, int oldNumCols )

    This change handler is called whenever any of the grid's
    dimensions change. \a oldNumRows and \a oldNumCols contain the
    old dimensions, numRows() and numCols() contain the new
    dimensions.
*/
void TQGridView::dimensionChange( int, int ) {}



/*!
    \fn int TQGridView::rowAt( int y ) const

    Returns the number of the row at position \a y. \a y must be given
    in content coordinates.

    \sa columnAt()
*/

/*!
    \fn int TQGridView::columnAt( int x ) const

    Returns the number of the column at position \a x. \a x must be
    given in content coordinates.

    \sa rowAt()
*/

/*!
    \fn void TQGridView::paintCell( TQPainter *p, int row, int col )

    This pure virtual function is called to paint the single cell at
    (\a row, \a col) using painter \a p. The painter must be open when
    paintCell() is called and must remain open.

    The coordinate system is \link TQPainter::translate() translated
    \endlink so that the origin is at the top-left corner of the cell
    to be painted, i.e. \e cell coordinates. Do not scale or shear
    the coordinate system (or if you do, restore the transformation
    matrix before you return).

    The painter is not clipped by default in order to get maximum
    efficiency. If you want clipping, use

    \code
    p->setClipRect( cellRect(), TQPainter::CoordPainter );
    //... your drawing code
    p->setClipping( FALSE );

    \endcode
*/

/*!
    \fn  TQRect TQGridView::cellRect() const

    Returns the geometry of a cell in a cell's coordinate system. This
    is a convenience function useful in paintCell(). It is equivalent
    to TQRect( 0, 0, cellWidth(), cellHeight() ).

    \sa cellGeometry()

*/

/*!
    \fn  TQSize TQGridView::gridSize() const

    Returns the size of the grid in pixels.

*/

#endif // TQT_NO_GRIDVIEW
