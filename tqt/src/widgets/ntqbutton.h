/****************************************************************************
**
** Definition of TQButton widget class
**
** Created : 940206
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

#ifndef TQBUTTON_H
#define TQBUTTON_H

#ifndef QT_H
#include "ntqwidget.h"
#include "ntqkeysequence.h"
#endif // QT_H

#ifndef QT_NO_BUTTON


class TQButtonGroup;
class TQToolBar;
class TQButtonData;

class Q_EXPORT TQButton : public TQWidget
{
    TQ_OBJECT
    TQ_ENUMS( ToggleType ToggleState )
    TQ_PROPERTY( TQString text READ text WRITE setText )
    TQ_PROPERTY( TQPixmap pixmap READ pixmap WRITE setPixmap )
    TQ_PROPERTY( TQKeySequence accel READ accel WRITE setAccel )
    TQ_PROPERTY( bool toggleButton READ isToggleButton )
    TQ_PROPERTY( ToggleType toggleType READ toggleType )
    TQ_PROPERTY( bool down READ isDown WRITE setDown DESIGNABLE false  )
    TQ_PROPERTY( bool on READ isOn )
    TQ_PROPERTY( ToggleState toggleState READ state )
    TQ_PROPERTY( bool autoResize READ autoResize WRITE setAutoResize DESIGNABLE false )
    TQ_PROPERTY( bool autoRepeat READ autoRepeat WRITE setAutoRepeat )
    TQ_PROPERTY( bool exclusiveToggle READ isExclusiveToggle )

public:
    TQButton( TQWidget* parent=0, const char* name=0, WFlags f=0 );
    ~TQButton();

    TQString text() const;
    virtual void setText( const TQString &);
    const TQPixmap *pixmap() const;
    virtual void setPixmap( const TQPixmap & );

#ifndef QT_NO_ACCEL
    TQKeySequence		accel()	const;
    virtual void	setAccel( const TQKeySequence& );
#endif

    bool	isToggleButton() const;

    enum ToggleType { SingleShot, Toggle, Tristate };
    ToggleType	toggleType() const;

    virtual void setDown( bool );
    bool	isDown() const;

    bool	isOn() const;

    enum ToggleState { Off, NoChange, On };
    ToggleState	state() const;

#ifndef QT_NO_COMPAT
    bool	autoResize() const;
    void	setAutoResize( bool );
#endif

    bool	autoRepeat() const;
    virtual void setAutoRepeat( bool );
    bool	isExclusiveToggle() const;

    TQButtonGroup *group() const;

public slots:
    void	animateClick();
    void	toggle();

signals:
    void	pressed();
    void	released();
    void	clicked();
    void	toggled( bool );
    void	stateChanged( int );

protected:
    void	setToggleButton( bool );
    virtual void	setToggleType( ToggleType );
    void	setOn( bool );
    virtual void	setState( ToggleState );

    virtual bool hitButton( const TQPoint &pos ) const;
    virtual void drawButton( TQPainter * );
    virtual void drawButtonLabel( TQPainter * );

    void	keyPressEvent( TQKeyEvent *);
    void	keyReleaseEvent( TQKeyEvent *);
    void	mousePressEvent( TQMouseEvent * );
    void	mouseReleaseEvent( TQMouseEvent * );
    void	mouseMoveEvent( TQMouseEvent * );
    void	paintEvent( TQPaintEvent * );
    void	focusInEvent( TQFocusEvent * );
    void	focusOutEvent( TQFocusEvent * );

    void	enabledChange( bool );

private slots:
    void	animateTimeout();
    void	autoRepeatTimeout();
    void	emulateClick();

private:
    TQString	btext;
    TQPixmap    *bpixmap;
    uint	toggleTyp	: 2;
    uint	buttonDown	: 1;
    uint	stat		: 2;
    uint	mlbDown		: 1;
    uint	autoresize	: 1;
    uint	animation	: 1;
    uint	repeat		: 1;
    TQButtonData *d;

    friend class TQButtonGroup;
    friend class TQToolBar;
    void          ensureData();
    virtual void setGroup( TQButtonGroup* );
    TQTimer	 *timer();
    void	nextState();

private:	// Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    TQButton( const TQButton & );
    TQButton &operator=( const TQButton & );
#endif
};


inline TQString TQButton::text() const
{
    return btext;
}

inline const TQPixmap *TQButton::pixmap() const
{
    return bpixmap;
}

inline bool TQButton::isToggleButton() const
{
    return ToggleType(toggleTyp) != SingleShot;
}

inline  bool TQButton::isDown() const
{
    return buttonDown;
}

inline bool TQButton::isOn() const
{
    return ToggleState(stat) != Off;
}

#ifndef QT_NO_COMPAT
inline bool TQButton::autoResize() const
{
    return autoresize;
}
#endif

inline bool TQButton::autoRepeat() const
{
    return repeat;
}

inline TQButton::ToggleState TQButton::state() const
{
    return ToggleState(stat);
}

inline void TQButton::setToggleButton( bool b )
{
    setToggleType( b ? Toggle : SingleShot );
}

inline void TQButton::setOn( bool y )
{
    setState( y ? On : Off );
}

inline TQButton::ToggleType TQButton::toggleType() const
{
    return ToggleType(toggleTyp);
}


#endif // QT_NO_BUTTON

#endif // TQBUTTON_H
