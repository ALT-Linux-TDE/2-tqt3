/**********************************************************************
**
** Definition of TQtMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file contains a class moved out of the TQt GUI Toolkit API. It
** may be used, distributed and modified without limitation.
**
**********************************************************************/

#ifndef TQTMULTILINEEDIT_H
#define TQTMULTILINEEDIT_H

#ifndef QT_H
#include "qttableview.h"
#include "ntqstring.h"
#include "ntqptrlist.h"
#endif // QT_H

#ifndef QT_NO_QTMULTILINEEDIT

struct TQtMultiLineData;
class TQtMultiLineEditCommand;
class TQValidator;

class TQtMultiLineEdit : public TQtTableView
{
    TQ_OBJECT
    TQ_ENUMS( EchoMode WordWrap WrapPolicy )
    TQ_PROPERTY( int numLines READ numLines )
    TQ_PROPERTY( bool atBeginning READ atBeginning )
    TQ_PROPERTY( bool atEnd READ atEnd )
    TQ_PROPERTY( int maxLineWidth READ maxLineWidth )
    TQ_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    TQ_PROPERTY( bool edited READ edited WRITE setEdited DESIGNABLE false )
    TQ_PROPERTY( EchoMode echoMode READ echoMode WRITE setEchoMode )
    TQ_PROPERTY( int maxLength READ maxLength WRITE setMaxLength )
    TQ_PROPERTY( int maxLines READ maxLines WRITE setMaxLines )
    TQ_PROPERTY( int hMargin READ hMargin WRITE setHMargin )
    TQ_PROPERTY( WordWrap wordWrap READ wordWrap WRITE setWordWrap )
    TQ_PROPERTY( int wrapColumnOrWidth READ wrapColumnOrWidth WRITE setWrapColumnOrWidth )
    TQ_PROPERTY( WrapPolicy wrapPolicy READ wrapPolicy WRITE setWrapPolicy )
    TQ_PROPERTY( bool autoUpdate READ autoUpdate WRITE setAutoUpdate DESIGNABLE false )
    TQ_PROPERTY( bool undoEnabled READ isUndoEnabled WRITE setUndoEnabled )
    TQ_PROPERTY( int undoDepth READ undoDepth WRITE setUndoDepth )
    TQ_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    TQ_PROPERTY( bool overWriteMode READ isOverwriteMode WRITE setOverwriteMode )
    TQ_PROPERTY( TQString text READ text WRITE setText )
    TQ_PROPERTY( int length READ length )

public:
    TQtMultiLineEdit( TQWidget *parent=0, const char *name=0 );
   ~TQtMultiLineEdit();

    TQString textLine( int line ) const;
    int numLines() const;

    TQSize sizeHint() const;
    TQSize minimumSizeHint() const;
    TQSizePolicy sizePolicy() const;

    virtual void setFont( const TQFont &font );

    virtual void insertLine( const TQString &s, int line = -1 );
    virtual void insertAt( const TQString &s, int line, int col, bool mark = FALSE );
    virtual void removeLine( int line );

    void cursorPosition( int *line, int *col ) const;
    virtual void setCursorPosition( int line, int col, bool mark = FALSE );
    void getCursorPosition( int *line, int *col ) const;
    bool atBeginning() const;
    bool atEnd() const;

    virtual void setFixedVisibleLines( int lines );

    int maxLineWidth() const;

    void setAlignment( int flags );
    int alignment() const;

    virtual void setValidator( const TQValidator * );
    const TQValidator * validator() const;

    void setEdited( bool );
    bool edited() const;

    void cursorWordForward( bool mark );
    void cursorWordBackward( bool mark );

    enum EchoMode { Normal, NoEcho, Password };
    virtual void setEchoMode( EchoMode );
    EchoMode echoMode() const;

    void setMaxLength(int);
    int maxLength() const;
    virtual void setMaxLineLength(int);
    int maxLineLength() const;
    virtual void setMaxLines(int);
    int maxLines() const;
    virtual void setHMargin(int);
    int hMargin() const;

    virtual void setSelection( int row_from, int col_from, int row_to, int col_t );

    enum WordWrap {
	NoWrap,
	WidgetWidth,
	FixedPixelWidth,
	FixedColumnWidth
    };
    void setWordWrap( WordWrap mode );
    WordWrap wordWrap() const;
    void setWrapColumnOrWidth( int );
    int wrapColumnOrWidth() const;

    enum WrapPolicy {
	AtWhiteSpace,
	Anywhere
    };
    void setWrapPolicy( WrapPolicy policy );
    WrapPolicy wrapPolicy() const;

    bool autoUpdate()	const;
    virtual void setAutoUpdate( bool );

    void setUndoEnabled( bool );
    bool isUndoEnabled() const;
    void setUndoDepth( int );
    int undoDepth() const;

    bool isReadOnly() const;
    bool isOverwriteMode() const;

    TQString text() const;

    int length() const;

    static void setDefaultTabStop( int ex );
    static int defaultTabStop();
public slots:
    virtual void       setText( const TQString &);
    virtual void       setReadOnly( bool );
    virtual void       setOverwriteMode( bool );

    void       clear();
    void       append( const TQString &);
    void       deselect();
    void       selectAll();
#ifndef QT_NO_CLIPBOARD
    void       paste();
    void       pasteSubType(const TQCString& subtype);
    void       copyText() const;
    void       copy() const;
    void       cut();
#endif
    void       insert( const TQString& );
    void       undo();
    void       redo();

signals:
    void	textChanged();
    void	returnPressed();
    void	undoAvailable( bool );
    void	redoAvailable( bool );
    void	copyAvailable( bool );

protected:
    void	paintCell( TQPainter *, int row, int col );
    bool	event( TQEvent * );

    void	mousePressEvent( TQMouseEvent * );
    void	mouseMoveEvent( TQMouseEvent * );
    void	mouseReleaseEvent( TQMouseEvent * );
    void	mouseDoubleClickEvent( TQMouseEvent * );
    void 	wheelEvent( TQWheelEvent * );
    void	keyPressEvent( TQKeyEvent * );
    void	focusInEvent( TQFocusEvent * );
    void	focusOutEvent( TQFocusEvent * );
    void	timerEvent( TQTimerEvent * );
    void	leaveEvent( TQEvent * );
    void	resizeEvent( TQResizeEvent * );

    bool	focusNextPrevChild( bool );

#ifndef QT_NO_DRAGANDDROP
    void	dragMoveEvent( TQDragMoveEvent* );
    void	dragEnterEvent( TQDragEnterEvent * );
    void	dropEvent( TQDropEvent* );
    void	dragLeaveEvent( TQDragLeaveEvent* );
#endif

    bool	hasMarkedText() const;
    TQString	markedText() const;
    int		textWidth( int );
    int		textWidth( const TQString &);

    TQPoint	cursorPoint() const;

protected:
    virtual void insert( const TQString&, bool mark );
    virtual void newLine();
    virtual void killLine();
    virtual void pageUp( bool mark=FALSE );
    virtual void pageDown( bool mark=FALSE );
    virtual void cursorLeft( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorRight( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorUp( bool mark=FALSE );
    virtual void cursorDown( bool mark=FALSE );
    virtual void backspace();
    virtual void del();
    virtual void home( bool mark=FALSE );
    virtual void end( bool mark=FALSE );

    bool getMarkedRegion( int *line1, int *col1,
			  int *line2, int *col2 ) const;
    int lineLength( int row ) const;
    TQString *getString( int row ) const;
    bool isEndOfParagraph( int row ) const;
    TQString stringShown( int row ) const;

protected:
    bool	cursorOn;
    void	insertChar( TQChar );

private slots:
    void	clipboardChanged();
    void	blinkTimerTimeout();
    void	scrollTimerTimeout();
    void	dndTimeout();

private:
#ifndef QT_NO_MIME
    TQCString pickSpecial(TQMimeSource* ms, bool always_ask, const TQPoint&);
#endif
#ifndef QT_NO_MIMECLIPBOARD
    void       pasteSpecial(const TQPoint&);
#endif
    struct TQtMultiLineEditRow {
	TQtMultiLineEditRow( TQString string, int width, bool nl = TRUE )
	    :s(string), w(width), newline( nl )
	{
	};
	TQString s;
	int w;
	bool newline;
    };
    TQPtrList<TQtMultiLineEditRow> *contents;
    TQtMultiLineData *d;

    bool	readOnly;
    bool	dummy;
    bool	markIsOn;
    bool	dragScrolling ;
    bool	dragMarking;
    bool	textDirty;
    bool	wordMark;
    bool	overWrite;

    int		cursorX;
    int		cursorY;
    int		markAnchorX;
    int		markAnchorY;
    int		markDragX;
    int		markDragY;
    int		curXPos;	// cell coord of cursor
    int		blinkTimer; // #### not used anymore - remove in 3.0
    int		scrollTimer; // #### not used anymore - remove in 3.0

    int		mapFromView( int xPos, int row );
    int		mapToView( int xIndex, int row );

    void	pixelPosToCursorPos(TQPoint p, int* x, int* y) const;
    void	setCursorPixelPosition(TQPoint p, bool clear_mark=TRUE);

    void	setWidth( int );
    void	updateCellWidth();
    bool 	partiallyInvisible( int row );
    void	makeVisible();
    void	setBottomCell( int row );

    void 	newMark( int posx, int posy, bool copy=TRUE );
    void 	markWord( int posx, int posy );
    void	extendSelectionWord( int &newX, int&newY);
    int 	charClass( TQChar );
    void	turnMark( bool on );
    bool	inMark( int posx, int posy ) const;
    bool	beforeMark( int posx, int posy ) const;
    bool	afterMark( int posx, int posy ) const;
    int		setNumRowsAndTruncate();

#ifndef QT_NO_DRAGANDDROP
    void	doDrag();
#endif
    void	startAutoScroll();
    void	stopAutoScroll();

    void	cursorLeft( bool mark, bool clear_mark, bool wrap );
    void	cursorRight( bool mark, bool clear_mark, bool wrap );
    void	cursorUp( bool mark, bool clear_mark );
    void	cursorDown( bool mark, bool clear_mark );

    void	wrapLine( int line, int removed = 0);
    void	rebreakParagraph( int line, int removed = 0 );
    void	rebreakAll();
    void	insertAtAux( const TQString &s, int line, int col, bool mark = FALSE );
    void	killLineAux();
    void	delAux();
    int	positionToOffsetInternal( int row, int col ) const;
    void	offsetToPositionInternal( int position, int *row, int *col ) const;
    void	deleteNextChar( int offset, int row, int col );

    void addUndoCmd( TQtMultiLineEditCommand* );
    void addRedoCmd( TQtMultiLineEditCommand* );
    void processCmd( TQtMultiLineEditCommand*, bool );

private:	// Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    TQtMultiLineEdit( const TQtMultiLineEdit & );
    TQtMultiLineEdit &operator=( const TQtMultiLineEdit & );
#endif
};

inline bool TQtMultiLineEdit::isReadOnly() const { return readOnly; }

inline bool TQtMultiLineEdit::isOverwriteMode() const { return overWrite; }

inline void TQtMultiLineEdit::setOverwriteMode( bool on )
{
    overWrite = on;
 }

inline int TQtMultiLineEdit::lineLength( int row ) const
{
    return contents->at( row )->s.length();
}

inline bool TQtMultiLineEdit::atEnd() const
{
    return cursorY == (int)contents->count() - 1
	&& cursorX == lineLength( cursorY ) ;
}

inline bool TQtMultiLineEdit::atBeginning() const
{
    return cursorY == 0 && cursorX == 0;
}

inline TQString *TQtMultiLineEdit::getString( int row ) const
{
    return &(contents->at( row )->s);
}

inline int TQtMultiLineEdit::numLines() const
{
    return contents->count();
}

#endif // QT_NO_QTMULTILINEEDIT

#endif // TQTMULTILINEDIT_H
