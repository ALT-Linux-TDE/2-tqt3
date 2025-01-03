/****************************************************************************
**
** Definition of TQToolButton class
**
** Created : 979899
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

#ifndef TQTOOLBUTTON_H
#define TQTOOLBUTTON_H

#ifndef QT_H
#include "tqbutton.h"
#include "tqstring.h"
#include "tqpixmap.h"
#include "tqiconset.h"
#endif // QT_H

#ifndef TQT_NO_TOOLBUTTON

class TQToolButtonPrivate;
class TQToolBar;
class TQPopupMenu;

class TQ_EXPORT TQToolButton : public TQButton
{
    TQ_OBJECT
    TQ_ENUMS( TextPosition )

    TQ_PROPERTY( TQIconSet iconSet READ iconSet WRITE setIconSet )
    TQ_PROPERTY( bool usesBigPixmap READ usesBigPixmap WRITE setUsesBigPixmap )
    TQ_PROPERTY( bool usesTextLabel READ usesTextLabel WRITE setUsesTextLabel )
    TQ_PROPERTY( TQString textLabel READ textLabel WRITE setTextLabel )
    TQ_PROPERTY( int popupDelay READ popupDelay WRITE setPopupDelay )
    TQ_PROPERTY( bool autoRaise READ autoRaise WRITE setAutoRaise )
    TQ_PROPERTY( TextPosition textPosition READ textPosition WRITE setTextPosition )

    TQ_OVERRIDE( bool toggleButton WRITE setToggleButton )
    TQ_OVERRIDE( bool on WRITE setOn )
    TQ_OVERRIDE( TQPixmap pixmap DESIGNABLE false STORED false )
    TQ_OVERRIDE( BackgroundMode backgroundMode DESIGNABLE true)

public:
    enum TextPosition {
	BesideIcon,
	BelowIcon,
	Right = BesideIcon, // obsolete
	Under = BelowIcon // obsolete
    };
    TQToolButton( TQWidget * parent, const char* name=0 );
#ifndef TQT_NO_TOOLBAR
    TQToolButton( const TQIconSet& s, const TQString &textLabel,
		 const TQString& grouptext,
		 TQObject * receiver, const char* slot,
		 TQToolBar * parent, const char* name=0 );
#endif
    TQToolButton( ArrowType type, TQWidget *parent, const char* name=0 );
    ~TQToolButton();

    TQSize sizeHint() const;
    TQSize minimumSizeHint() const;

    virtual void setIconSet( const TQIconSet & );
    TQIconSet iconSet() const;

    bool usesBigPixmap() const { return ubp; }
    bool usesTextLabel() const { return utl; }
    TQString textLabel() const { return tl; }

#ifndef TQT_NO_POPUPMENU
    void setPopup( TQPopupMenu* popup );
    TQPopupMenu* popup() const;

    void setPopupDelay( int delay );
    int popupDelay() const;

    void openPopup();
#endif

    void setAutoRaise( bool enable );
    bool autoRaise() const;
    TextPosition textPosition() const;

    void setText( const TQString &txt );

public slots:
    virtual void setUsesBigPixmap( bool enable );
    virtual void setUsesTextLabel( bool enable );
    virtual void setTextLabel( const TQString &, bool );

    virtual void setToggleButton( bool enable );

    virtual void setOn( bool enable );
    void toggle();
    void setTextLabel( const TQString & );
    void setTextPosition( TextPosition pos );

protected:
    void mousePressEvent( TQMouseEvent * );
    void drawButton( TQPainter * );
    void drawButtonLabel(TQPainter *);

    void enterEvent( TQEvent * );
    void leaveEvent( TQEvent * );
    void moveEvent( TQMoveEvent * );

    // ### Make virtual in 4.0, maybe act like TQPushButton with
    // regards to setFlat() instead?  Andy
    bool uses3D() const;

    bool eventFilter( TQObject *o, TQEvent *e );

#ifndef TQT_NO_PALETTE
    void paletteChange( const TQPalette & );
#endif

private slots:
    void popupTimerDone();
    void popupPressed();

private:
    void init();

    TQPixmap bp;
    int bpID;
    TQPixmap sp;
    int spID;

    TQString tl;

    TQToolButtonPrivate *d;
    TQIconSet *s;

    uint utl : 1;
    uint ubp : 1;
    uint hasArrow : 1;

private:	// Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    TQToolButton( const TQToolButton & );
    TQToolButton& operator=( const TQToolButton & );
#endif
};

#endif // TQT_NO_TOOLBUTTON

#endif // TQTOOLBUTTON_H
