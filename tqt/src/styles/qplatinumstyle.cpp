/****************************************************************************
**
** Implementation of Platinum-like style class
**
** Created : 981231
**
** Copyright (C) 1998-2008 Trolltech ASA.  All rights reserved.
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

#include "ntqplatinumstyle.h"

#if !defined(TQT_NO_STYLE_PLATINUM) || defined(QT_PLUGIN)

#include "ntqapplication.h"
#include "ntqcombobox.h"
#include "ntqdrawutil.h"
#include "ntqpainter.h"
#include "ntqpalette.h"
#include "ntqpixmap.h"
#include "ntqpushbutton.h"
#include "ntqscrollbar.h"
#include "ntqslider.h"
#include <limits.h>

/*!
    \class TQPlatinumStyle ntqplatinumstyle.h
    \brief The TQPlatinumStyle class provides Mac/Platinum look and feel.

    \ingroup appearance

    This class implements the Platinum look and feel. It's an
    experimental class that tries to resemble a Macinosh-like GUI
    style with the TQStyle system. The emulation is currently far from
    perfect.

    \sa TQAquaStyle
*/


/*!
    Constructs a TQPlatinumStyle
*/
TQPlatinumStyle::TQPlatinumStyle()
{
}

/*!\reimp
*/
TQPlatinumStyle::~TQPlatinumStyle()
{
}


/*!\reimp
 */
void TQPlatinumStyle::drawPrimitive( PrimitiveElement pe,
				    TQPainter *p,
				    const TQStyleControlElementData &ceData,
				    ControlElementFlags elementFlags,
				    const TQRect &r,
				    const TQColorGroup &cg,
				    SFlags flags,
				    const TQStyleOption& opt ) const
{
    switch (pe) {
    case PE_HeaderSection:
	{
	    // adjust the sunken flag, otherwise headers are drawn
	    // sunken...
	    if ( flags & Style_Sunken )
		flags ^= Style_Sunken;
	    drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, r, cg, flags, opt );
	    break;
	}
    case PE_ButtonTool:
	{
	    // tool buttons don't change color when pushed in platinum,
	    // so we need to make the mid and button color the same
	    TQColorGroup myCG = cg;
	    TQBrush fill;

	    // quick trick to make sure toolbuttons drawn sunken
	    // when they are activated...
	    if ( flags & Style_On )
		flags |= Style_Sunken;

	    fill = myCG.brush( TQColorGroup::Button );
	    myCG.setBrush( TQColorGroup::Mid, fill );
	    drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, r, myCG, flags, opt );
	    break;
	}
    case PE_ButtonBevel:
	{
	    int x,
		y,
		w,
		h;
	    r.rect( &x, &y, &w, &h );

	    TQPen oldPen = p->pen();
	    if ( w * h < 1600 ||
		 TQABS(w - h) > 10 ) {
		// small buttons

		if ( !(flags & (Style_Sunken | Style_Down)) ) {
		    p->fillRect( x + 2, y + 2, w - 4, h - 4,
				 cg.brush(TQColorGroup::Button) );
		    // the bright side
		    p->setPen( cg.dark() );
		    // the bright side
		    p->setPen( cg.dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.light() );
		    p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
		    p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );

		    // the dark side
		    p->setPen( cg.mid() );
		    p->drawLine( x + 2, y + h - 2, x + w - 2, y + h - 2 );
		    p->drawLine( x + w - 2, y + 2, x + w - 2, y + h - 3 );

		    p->setPen( cg.dark().dark() );
		    p->drawLine( x + 1, y + h - 1, x + w - 1,
				 y + h - 1 );
		    p->drawLine( x + w - 1, y + 1,
				 x + w - 1,
				 y + h - 2 );
		} else {
		    p->fillRect(x + 2, y + 2,
				w - 4, h - 4,
				cg.brush( TQColorGroup::Mid ));

		    // the dark side
		    p->setPen( cg.dark().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.mid().dark());
		    p->drawLine( x + 1, y + 1,
				 x + w-2, y + 1);
		    p->drawLine( x + 1, y + 1,
				 x + 1, y + h - 2 );


		    // the bright side!

		    p->setPen(cg.button());
		    p->drawLine( x + 1, y + h - 2,
				 x + w - 2,
				 y + h - 2 );
		    p->drawLine( x + w - 2, y + 1,
				 x + w - 2,
				 y + h - 2 );
		    p->setPen(cg.dark());
		    p->drawLine(x, y + h - 1,
				x + w - 1,
				y + h - 1 );
		    p->drawLine(x + w - 1, y,
				x + w - 1,
				y + h - 1 );
		}
	    } else {
		// big ones
		if ( !(flags & (Style_Sunken | Style_Down)) ) {
		    p->fillRect( x + 3, y + 3, w - 6,
				 h - 6,
				 cg.brush(TQColorGroup::Button) );

		    // the bright side
		    p->setPen( cg.button().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.button() );
		    p->drawLine( x + 1, y + 1,
				 x + w - 2, y + 1 );
		    p->drawLine( x + 1, y + 1,
				 x + 1, y + h - 2 );

		    p->setPen( cg.light() );
		    p->drawLine( x + 2, y + 2,
				 x + 2, y + h - 2 );
		    p->drawLine( x + 2, y + 2,
				 x + w - 2, y + 2 );
		    // the dark side!

		    p->setPen( cg.mid() );
		    p->drawLine( x + 3, y + h - 3,
				 x + w - 3,
				 y + h - 3 );
		    p->drawLine( x + w - 3, y + 3,
				 x + w - 3,
				 y + h - 3 );
		    p->setPen( cg.dark() );
		    p->drawLine( x + 2, y + h - 2,
				 x + w - 2,
				 y + h - 2 );
		    p->drawLine( x + w - 2, y + 2,
				 x + w - 2,
				 y + h - 2 );

		    p->setPen( cg.dark().dark() );
		    p->drawLine( x + 1, y + h - 1,
				 x + w - 1,
				 y + h - 1 );
		    p->drawLine( x + w - 1, y + 1,
				 x + w - 1,
				 y + h - 1 );
		} else {
		    p->fillRect( x + 3, y + 3, w - 6,
				 h - 6,
				 cg.brush( TQColorGroup::Mid ) );

		    // the dark side
		    p->setPen( cg.dark().dark().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.dark().dark() );
		    p->drawLine( x + 1, y + 1,
				 x + w - 2, y + 1 );
		    p->drawLine( x + 1, y + 1,
				 x + 1, y + h - 2 );

		    p->setPen( cg.mid().dark() );
		    p->drawLine( x + 2, y + 2,
				 x + 2, y + w - 2 );
		    p->drawLine( x + 2, y + 2,
				 x + w - 2, y + 2 );


		    // the bright side!

		    p->setPen( cg.button() );
		    p->drawLine( x + 2, y + h - 3,
				 x + w - 3,
				 y + h - 3 );
		    p->drawLine( x + w - 3, y + 3,
				 x + w - 3,
				 y + h - 3 );

		    p->setPen( cg.midlight() );
		    p->drawLine( x + 1, y + h - 2,
				 x + w - 2,
				 y + h - 2 );
		    p->drawLine( x + w - 2, y + 1,
				 x + w - 2,
				 y + h - 2 );

		    p->setPen( cg.dark() );
		    p->drawLine( x, y + h - 1,
				 x + w - 1,
				 y + h - 1 );
		    p->drawLine( x + w - 1, y,
				 x + w - 1,
				 y + h - 1 );


		    // corners
		    p->setPen( mixedColor(cg.dark().dark().dark(),
					  cg.dark()) );
		    p->drawPoint( x, y + h - 1 );
		    p->drawPoint( x + w - 1, y );

		    p->setPen( mixedColor(cg.dark().dark(), cg.midlight()) );
		    p->drawPoint( x + 1, y + h - 2 );
		    p->drawPoint( x + w - 2, y + 1 );

		    p->setPen( mixedColor(cg.mid().dark(), cg.button() ) );
		    p->drawPoint( x + 2, y + h - 3 );
		    p->drawPoint( x + w - 3, y + 2 );
		}
	    }
	    p->setPen( oldPen );
	    break;
	}
    case PE_ButtonCommand:
	{
	    TQPen oldPen = p->pen();
	    int x,
		y,
		w,
		h;
	    r.rect( &x, &y, &w, &h);

	    if ( !(flags & (Style_Down | Style_On)) ) {
		p->fillRect( x+3, y+3, w-6, h-6,
			     cg.brush( TQColorGroup::Button ));
		// the bright side
		p->setPen( cg.shadow() );
		p->drawLine( x, y, x+w-1, y );
		p->drawLine( x, y, x, y + h - 1 );

		p->setPen( cg.button() );
		p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
		p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );

		p->setPen( cg.light() );
		p->drawLine( x + 2, y + 2, x + 2, y + h - 2 );
		p->drawLine( x + 2, y + 2, x + w - 2, y + 2 );


		// the dark side!

		p->setPen( cg.mid() );
		p->drawLine( x + 3, y + h - 3 ,x + w - 3, y + h - 3 );
		p->drawLine( x + w - 3, y + 3, x + w - 3, y + h - 3 );

		p->setPen( cg.dark() );
		p->drawLine( x + 2, y + h - 2, x + w - 2, y + h - 2 );
		p->drawLine( x + w - 2, y + 2, x + w - 2, y + h - 2 );

		p->setPen( cg.shadow() );
		p->drawLine( x + 1, y + h - 1, x + w - 1, y + h - 1 );
		p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );


		// top left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y );
		p->drawPoint( x + 1, y );
		p->drawPoint( x, y+1 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + 1, y + 1 );
		p->setPen( cg.button() );
		p->drawPoint( x + 2, y + 2 );
		p->setPen( white );
		p->drawPoint( x + 3, y + 3 );
		// bottom left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y + h - 1 );
		p->drawPoint( x + 1, y + h - 1 );
		p->drawPoint( x, y + h - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + 1, y + h - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( x + 2, y + h - 3 );
		// top right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w -1, y );
		p->drawPoint( x + w - 2, y );
		p->drawPoint( x + w - 1, y + 1 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + w - 2, y + 1 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 3, y + 2 );
		// bottom right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y + h - 1 );
		p->drawPoint( x + w - 2, y + h - 1 );
		p->drawPoint( x + w - 1, y + h - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + w - 2, y + h - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 3, y + h - 3 );
		p->setPen( cg.mid() );
		p->drawPoint( x + w - 4, y + h - 4 );

	    } else {
		p->fillRect( x + 2, y + 2, w - 4, h - 4,
			     cg.brush(TQColorGroup::Dark) );

		// the dark side
		p->setPen( cg.shadow() );
		p->drawLine( x, y, x + w - 1, y );
		p->drawLine( x, y, x, y + h - 1 );

		p->setPen( cg.dark().dark() );
		p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
		p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );

		// the bright side!

		p->setPen( cg.button() );
		p->drawLine( x + 1, y + h - 2, x + w - 2, y + h - 2 );
		p->drawLine( x + w - 2, y + 1, x + w - 2, y + h - 2 );

		p->setPen( cg.dark() );
		p->drawLine( x, y + h - 1, x + w - 1, y + h - 1 );
		p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );

		// top left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y );
		p->drawPoint( x + 1, y );
		p->drawPoint( x, y + 1 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + 1, y + 1 );
		p->setPen( cg.dark().dark() );
		p->drawPoint( x + 3, y + 3 );
		// bottom left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y + h - 1 );
		p->drawPoint( x + 1, y + h - 1 );
		p->drawPoint( x, y + h - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + 1, y + h - 2 );
		// top right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y );
		p->drawPoint( x + w - 2, y );
		p->drawPoint( x + w - 1, y + 1 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + w - 2, y + 1 );
		// bottom right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y + h - 1 );
		p->drawPoint( x + w - 2, y + h - 1 );
		p->drawPoint( x + w - 1, y + h - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + w - 2, y + h - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 3, y + h - 3 );
		p->setPen( cg.mid() );
		p->drawPoint( x + w - 4, y + h - 4 );
	    }
	    p->setPen( oldPen );
	    break;
	}
    case PE_Indicator:
	{
	    drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, TQRect(r.x(), r.y(),
						    r.width() - 2, r.height()),
			   cg, flags );
	    p->fillRect( r.x() + r.width() - 2, r.y(), 2, r.height(),
			 cg.brush( TQColorGroup::Background ) );
	    p->setPen( cg.shadow() );
	    p->drawRect( r.x(), r.y(), r.width() - 2, r.height() );

	    static const TQCOORD nochange_mark[] = { 3,5, 9,5,  3,6, 9,6 };
	    static const TQCOORD check_mark[] = {
		3,5, 5,5,  4,6, 5,6,  5,7, 6,7,  5,8, 6,8,      6,9, 9,9,
		6,10, 8,10, 7,11, 8,11,  7,12, 7,12,  8,8, 9,8,  8,7, 10,7,
		9,6, 10,6, 9,5, 11,5,  10,4, 11,4,  10,3, 12,3,
		11,2, 12,2, 11,1, 13,1,  12,0, 13,0 };
	    if ( !(flags & Style_Off) ) {
		TQPen oldPen = p->pen();
		int x1 = r.x();
		int y1 = r.y();
		if ( flags & Style_Down ) {
		    x1++;
		    y1++;
		}
		TQPointArray amark;
		if ( flags & Style_On ) {
		    amark = TQPointArray( sizeof(check_mark)/(sizeof(TQCOORD)*2),
					 check_mark );
		    // ### KLUDGE!!
		    flags ^= Style_On;
		    flags ^= Style_Down;
		} else if ( flags & Style_NoChange ) {
		    amark = TQPointArray( sizeof(nochange_mark)
					 / (sizeof(TQCOORD) * 2),
					 nochange_mark );
		}

		amark.translate( x1 + 1, y1 + 1 );
		p->setPen( cg.dark() );
		p->drawLineSegments( amark );
		amark.translate( -1, -1 );
		p->setPen( cg.foreground() );
		p->drawLineSegments( amark );
		p->setPen( oldPen );
	    }
	    break;
	}
    case PE_IndicatorMask:
	{
	    int x,
		y,
		w,
		h;
	    r.rect( &x, &y, &w, &h );
	    p->fillRect( x, y, w - 2, h, color1);
	    if ( flags & Style_Off ) {
		TQPen oldPen = p->pen();
		p->setPen ( TQPen(color1, 2));
		p->drawLine( x + 2, y + h / 2 - 1,
			     x + w / 2 - 1, y + h - 4 );
		p->drawLine( x + w / 2 - 1, y + h - 4,
			     x + w, 0);
		p->setPen( oldPen );
	    }
	    break;
	}
    case PE_ExclusiveIndicator:
	{
#define TQCOORDARRLEN(x) sizeof(x) / (sizeof(TQCOORD) * 2 )
	    bool down = flags & Style_Down;
	    bool on = flags & Style_On;

	    static const TQCOORD pts1[] = {		// normal circle
		5,0, 8,0, 9,1, 10,1, 11,2, 12,3, 12,4, 13,5,
		13,8, 12,9, 12,10, 11,11, 10,12, 9,12, 8,13,
		5,13, 4,12, 3,12, 2,11, 1,10, 1,9, 0,8, 0,5,
		1,4, 1,3, 2,2, 3,1, 4,1 };
	    static const TQCOORD pts2[] = {		// top left shadow
		5,1, 8,1, 3,2, 7,2, 2,3, 5,3,  2,4, 4,4,
		1,5, 3,5, 1,6, 1,8, 2,6, 2,7 };
	    static const TQCOORD pts3[] = {		// bottom right, dark
		5,12, 8,12, 7,11, 10,11, 8,10, 11,10,
		9,9, 11,9, 10,8, 12,8, 11,7, 11,7,
		12,5, 12,7 };
	    static const TQCOORD pts4[] = {		// bottom right, light
		5,12, 8,12, 7,11, 10,11, 9,10, 11,10,
		10,9, 11,9, 11,7, 11,8, 12,5, 12,8 };
	    static const TQCOORD pts5[] = {		// check mark
		6,4, 8,4, 10,6, 10,8, 8,10, 6,10, 4,8, 4,6 };
	    static const TQCOORD pts6[] = {		// check mark extras
		4,5, 5,4, 9,4, 10,5, 10,9, 9,10, 5,10, 4,9 };
	    int x, y;
	    x = r.x();
	    y = r.y();
	    p->eraseRect( r );
	    p->setBrush( (down||on) ? cg.brush( TQColorGroup::Dark )
			 : cg.brush( TQColorGroup::Button) );
	    p->setPen( NoPen );
	    p->drawEllipse( x, y, 13, 13 );
	    p->setPen( cg.shadow() );
	    TQPointArray a( TQCOORDARRLEN(pts1), pts1 );
	    a.translate( x, y );
	    p->drawPolyline( a );	// draw normal circle
	    TQColor tc, bc;
	    const TQCOORD *bp;
	    int	bl;
	    if ( down || on ) {			// pressed down or on
		tc = cg.dark().dark();
		bc = cg.light();
		bp = pts4;
		bl = TQCOORDARRLEN(pts4);
	    } else {					// released
		tc = cg.light();
		bc = cg.dark();
		bp = pts3;
		bl = TQCOORDARRLEN(pts3);
	    }
	    p->setPen( tc );
	    a.setPoints( TQCOORDARRLEN(pts2), pts2 );
	    a.translate( x, y );
	    p->drawLineSegments( a );		// draw top shadow
	    p->setPen( bc );
	    a.setPoints( bl, bp );
	    a.translate( x, y );
	    p->drawLineSegments( a );
	    if ( on ) {				// draw check mark
		int x1 = x,
		    y1 = y;
		p->setBrush( cg.foreground() );
		p->setPen( cg.foreground() );
		a.setPoints( TQCOORDARRLEN(pts5), pts5 );
		a.translate( x1, y1 );
		p->drawPolygon( a );
		p->setBrush( NoBrush );
		p->setPen( cg.dark() );
		a.setPoints( TQCOORDARRLEN(pts6), pts6 );
		a.translate( x1, y1 );
		p->drawLineSegments( a );
	    }
	    break;
	}

    case PE_ExclusiveIndicatorMask:
	{
	    static const TQCOORD pts1[] = {		// normal circle
		5,0, 8,0, 9,1, 10,1, 11,2, 12,3, 12,4, 13,5,
		13,8, 12,9, 12,10, 11,11, 10,12, 9,12, 8,13,
		5,13, 4,12, 3,12, 2,11, 1,10, 1,9, 0,8, 0,5,
		1,4, 1,3, 2,2, 3,1, 4,1 };
	    TQPointArray a(TQCOORDARRLEN(pts1), pts1);
	    a.translate(r.x(), r.y());
	    p->setPen(color1);
	    p->setBrush(color1);
	    p->drawPolygon(a);
	    break;
	}
    case PE_ScrollBarAddLine:
	{
	    drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, r, cg,
			   (flags & Style_Enabled) | ((flags & Style_Down)
						      ? Style_Sunken
						      : Style_Raised) );
	    p->setPen( cg.shadow() );
	    p->drawRect( r );
	    drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowRight
			    : PE_ArrowDown), p, ceData, elementFlags, TQRect(r.x() + 2,
						      r.y() + 2,
						      r.width() - 4,
						      r.height() - 4),
			   cg, flags );
	    break;
	}
    case PE_ScrollBarSubLine:
	{
	    drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, r, cg,
			   (flags & Style_Enabled) | ((flags & Style_Down)
						      ? Style_Sunken
						      : Style_Raised) );
	    p->setPen( cg.shadow() );
	    p->drawRect( r );
	    drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowLeft
			    : PE_ArrowUp ), p, ceData, elementFlags, TQRect(r.x() + 2,
						     r.y() + 2,
						     r.width() - 4,
						     r.height() - 4),
			   cg, flags );
	    break;
	}
    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage:
	{
 	    TQPen oldPen = p->pen();
	    if ( r.width() < 3 || r.height() < 3 ) {
		p->fillRect( r, cg.brush(TQColorGroup::Mid) );
		p->setPen( cg.shadow() );
		p->drawRect( r );
		p->setPen( oldPen );
	    } else {
		int x,
		    y,
		    w,
		    h;
		r.rect( &x, &y, &w, &h );
		if ( flags & Style_Horizontal ) {
		    p->fillRect( x + 2, y + 2, w - 2,
				 h - 4,
				 cg.brush(TQColorGroup::Mid) );
		    // the dark side
		    p->setPen( cg.dark().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->setPen( cg.shadow());
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.mid().dark());
		    p->drawLine( x + 1, y + 1, x + w - 1,
				 y + 1 );
		    p->drawLine( x + 1, y + 1, x + 1,
				 y + h - 2 );

		    // the bright side!

		    p->setPen( cg.button());
		    p->drawLine( x + 1, y + h - 2,
				 x + w - 1,
				 y + h - 2 );
		    p->setPen( cg.shadow());
		    p->drawLine( x, y + h - 1,
				 x + w - 1,
				 y + h - 1 );

		} else {
		    p->fillRect( x + 2, y + 2, w - 4,
				 h - 2,
				 cg.brush(TQColorGroup::Mid) );

		    // the dark side
		    p->setPen( cg.dark().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->setPen( cg.shadow() );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.mid().dark() );
		    p->drawLine( x + 1, y + 1, x + w - 2,
				 y + 1 );
		    p->drawLine( x + 1, y + 1, x + 1,
				 y + h - 1 );

		    // the bright side!
		    p->setPen( cg.button() );
		    p->drawLine( x + w - 2, y + 1,
				 x + w - 2,
				 y + h - 1 );

		    p->setPen( cg.shadow() );
		    p->drawLine( x + w - 1, y,
				 x + w - 1,
				 y + h - 1 );

		}
	    }
	    p->setPen( oldPen );
	    break;
	}
    case PE_ScrollBarSlider:
	{
	    TQPoint bo = p->brushOrigin();
	    p->setBrushOrigin( r.topLeft() );
	    drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, r, cg, Style_Raised );
	    p->setBrushOrigin( bo );
	    drawRiffles( p, r.x(), r.y(), r.width(), r.height(), cg,
			 flags & Style_Horizontal );
	    p->setPen( cg.shadow() );
	    p->drawRect( r );
	    if ( flags & Style_HasFocus ) {
		drawPrimitive( PE_FocusRect, p, ceData, elementFlags, TQRect(r.x() + 2, r.y() + 2,
						      r.width() - 5,
						      r.height() - 5 ),
			       cg, flags );
	    }
	    break;
	}
    default:
	TQWindowsStyle::drawPrimitive( pe, p, ceData, elementFlags, r, cg, flags, opt );
	break;
    }

}

/*!\reimp
 */
void TQPlatinumStyle::drawControl( ControlElement element,
				  TQPainter *p,
				  const TQStyleControlElementData &ceData,
				  ControlElementFlags elementFlags,
				  const TQRect &r,
				  const TQColorGroup &cg,
				  SFlags how,
				  const TQStyleOption& opt,
				  const TQWidget *widget ) const
{
    switch( element ) {
    case CE_PushButton:
	{
#ifndef TQT_NO_PUSHBUTTON
	    TQColorGroup myCg( cg );
	    int x1, y1, x2, y2;
	    bool useBevelButton;
	    SFlags flags;
	    flags = Style_Default;
	    p->setBrushOrigin( -ceData.bgOffset.x(),
			       -ceData.bgOffset.y() );

	    // take care of the flags based on what we know...
	    if ( elementFlags & CEF_IsDown )
		flags |= Style_Down;
	    if ( elementFlags & CEF_IsOn )
		flags |= Style_On;
	    if ( elementFlags & CEF_IsEnabled )
		flags |= Style_Enabled;
	    if ( elementFlags & CEF_IsDefault )
		flags |= Style_Default;
 	    if (! ( elementFlags & CEF_IsFlat ) && !(flags & Style_Down))
 		flags |= Style_Raised;

	    r.coords( &x1, &y1, &x2, &y2 );

	    p->setPen( cg.foreground() );
	    p->setBrush( TQBrush(cg.button(), NoBrush) );

	    TQBrush fill;
	    if ( elementFlags & CEF_IsDown ) {
		fill = cg.brush( TQColorGroup::Dark );
		// this could be done differently, but this
		// makes a down Bezel drawn correctly...
		myCg.setBrush( TQColorGroup::Mid, fill );
	    } else if ( elementFlags & CEF_IsOn ) {
		fill = TQBrush( cg.mid(), Dense4Pattern );
		myCg.setBrush( TQColorGroup::Mid, fill );
	    }
	    // to quote the old TQPlatinumStlye drawPushButton...
	    // small or square image buttons as well as toggle buttons are
	    // bevel buttons (what a heuristic....)
	    if ( ( elementFlags & CEF_BiState )
		 || ( (!ceData.fgPixmap.isNull()) &&
		      (ceData.rect.width() * ceData.rect.height() < 1600 ||
		       TQABS( ceData.rect.width() - ceData.rect.height()) < 10 )) )
		useBevelButton = TRUE;
	    else
		useBevelButton = FALSE;

	    int diw = pixelMetric( PM_ButtonDefaultIndicator, ceData, elementFlags, widget );
	    if ( elementFlags & CEF_IsDefault ) {
		x1 += 1;
		y1 += 1;
		x2 -= 1;
		y2 -= 1;
		TQColorGroup cg2( myCg );
		SFlags myFlags = flags;
		// don't draw the default button sunken, unless it's necessary.
		if ( myFlags & Style_Down )
		    myFlags ^= Style_Down;
		if ( myFlags & Style_Sunken )
		    myFlags ^= Style_Sunken;

		cg2.setColor( TQColorGroup::Button, cg.mid() );
		if ( useBevelButton ) {
		    drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, TQRect( x1, y1,
							     x2 - x1 + 1,
							     y2 - y1 + 1 ),
				   myCg, myFlags, opt );
		} else {
		    drawPrimitive( PE_ButtonCommand, p, ceData, elementFlags, TQRect( x1, y1,
							       x2 - x1 + 1,
							       y2 - y1 + 1 ),
				   cg2, myFlags, opt );
		}
	    }

	    if ( ( elementFlags & CEF_IsDefault ) || ( elementFlags & CEF_AutoDefault ) ) {
		x1 += diw;
		y1 += diw;
		x2 -= diw;
		y2 -= diw;
	    }

	    if ( (!( elementFlags & CEF_IsFlat )) || ( elementFlags & CEF_IsOn ) || ( elementFlags & CEF_IsDown ) ) {
		if ( useBevelButton ) {
		    // fix for toggle buttons...
		    if ( flags & (Style_Down | Style_On) )
			flags |= Style_Sunken;
		    drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, TQRect( x1, y1,
							     x2 - x1 + 1,
							     y2 - y1 + 1 ),
				   myCg, flags, opt );
		} else {

		    drawPrimitive( PE_ButtonCommand, p, ceData, elementFlags, TQRect( x1, y1,
							       x2 - x1 + 1,
							       y2 - y1 + 1 ),
				   myCg, flags, opt );
		}
	    }


	    if ( p->brush().style() != NoBrush )
		p->setBrush( NoBrush );
	    break;
#endif
	}
    case CE_PushButtonLabel:
	{
#ifndef TQT_NO_PUSHBUTTON
	    bool on;
	    int x, y, w, h;
	    SFlags flags;
	    flags = Style_Default;
	    on = (( elementFlags & CEF_IsDown ) || ( elementFlags & CEF_IsOn ));
	    r.rect( &x, &y, &w, &h );
	    if ( elementFlags & CEF_IsMenuWidget ) {
		int dx = pixelMetric( PM_MenuButtonIndicator, ceData, elementFlags, widget );

		TQColorGroup g = cg;
		int xx = x + w - dx - 4;
		int yy = y - 3;
		int hh = h + 6;

		if ( !on ) {
		    p->setPen( g.mid() );
		    p->drawLine( xx, yy + 2, xx, yy + hh - 3 );
		    p->setPen( g.button() );
		    p->drawLine( xx + 1, yy + 1, xx + 1, yy + hh - 2 );
		    p->setPen( g.light() );
		    p->drawLine( xx + 2, yy + 2, xx + 2, yy + hh - 2 );
		}
		if ( elementFlags & CEF_IsEnabled )
		    flags |= Style_Enabled;
		drawPrimitive( PE_ArrowDown, p, ceData, elementFlags, TQRect(x + w - dx - 1, y + 2,
						      dx, h - 4),
			       g, flags, opt );
		w -= dx;
	    }
#ifndef TQT_NO_ICONSET
	    if ( !ceData.iconSet.isNull() ) {
		TQIconSet::Mode mode = ( elementFlags & CEF_IsEnabled )
				      ? TQIconSet::Normal : TQIconSet::Disabled;
		if ( mode == TQIconSet::Normal && ( elementFlags & CEF_HasFocus ) )
		    mode = TQIconSet::Active;
		TQIconSet::State state = TQIconSet::Off;
		if ( ( elementFlags & CEF_BiState ) && ( elementFlags & CEF_IsOn ) )
		    state = TQIconSet::On;
		TQPixmap pixmap = ceData.iconSet.pixmap( TQIconSet::Small,
							 mode, state );
		int pixw = pixmap.width();
		int pixh = pixmap.height();
		p->drawPixmap( x + 2, y + h / 2 - pixh / 2, pixmap );
		x += pixw + 4;
		w -= pixw + 4;
	    }
#endif
	    drawItem( p, TQRect( x, y, w, h ),
		      AlignCenter | ShowPrefix,
		      ceData.colorGroup, ( elementFlags & CEF_IsEnabled ),
		      (ceData.fgPixmap.isNull())?NULL:&ceData.fgPixmap, ceData.text, -1,
		      on ? &ceData.colorGroup.brightText()
		      : &ceData.colorGroup.buttonText() );
	    if ( elementFlags & CEF_HasFocus )
		drawPrimitive( PE_FocusRect, p, ceData, elementFlags,
			       subRect(SR_PushButtonFocusRect, ceData, elementFlags, widget),
			       cg, flags );
	    break;
#endif
	}
    default:
	TQWindowsStyle::drawControl( element, p, ceData, elementFlags, r, cg, how, opt, widget );
	break;
    }
}

/*!\reimp
 */
void TQPlatinumStyle::drawComplexControl( ComplexControl control,
					 TQPainter *p,
					 const TQStyleControlElementData &ceData,
					 ControlElementFlags elementFlags,
					 const TQRect &r,
					 const TQColorGroup &cg,
					 SFlags how,
					 SCFlags sub,
					 SCFlags subActive,
					 const TQStyleOption& opt,
					 const TQWidget *widget ) const
{
    switch ( control ) {
    case CC_ComboBox:
	{
	    int x,
		y,
		w,
		h;
	    r.rect( &x, &y, &w, &h );
	    p->fillRect( x + 2,  y + 2, w - 4,
			 h - 4, cg.brush(TQColorGroup::Button) );
	    // the bright side
	    p->setPen(cg.shadow());
	    p->drawLine( x, y, x + w - 1, y );
	    p->drawLine( x, y, x, y + h - 1 );

	    p->setPen( cg.light() );
	    p->drawLine( x + 1, y + 1,
			 x + w - 2, y + 1 );
	    p->drawLine( x + 1, y + 1,
			 x + 1, y + h - 2 );

	    // the dark side!


	    p->setPen( cg.mid() );
	    p->drawLine( x + 2, y + h - 2,
			 x + w - 2, y + h - 2 );
	    p->drawLine( x + w - 2, y + 2,
			 x + w - 2, y + h - 2 );

	    p->setPen (cg.shadow() );
	    p->drawLine( x + 1, y + h - 1,
			 x + w - 1, y + h - 1 );
	    p->drawLine( x + w - 1, y,
			 x + w - 1, y + h - 1 );

	    // top left corner:
	    p->setPen( cg.background() );
	    p->drawPoint( x, y );
	    p->drawPoint( x + 1, y );
	    p->drawPoint( x, y + 1 );
	    p->setPen( cg.shadow() );
	    p->drawPoint( x + 1, y + 1 );
	    p->setPen( white );
	    p->drawPoint( x + 3, y + 3 );
	    // bottom left corner:
	    p->setPen( cg.background() );
	    p->drawPoint( x, y + h - 1 );
	    p->drawPoint( x + 1, y + h - 1 );
	    p->drawPoint( x, y + h - 2 );
	    p->setPen( cg.shadow() );
	    p->drawPoint( x + 1, y + h - 2 );
	    // top right corner:
	    p->setPen( cg.background() );
	    p->drawPoint( x + w - 1, y );
	    p->drawPoint( x + w - 2, y );
	    p->drawPoint( x + w - 1, y + 1 );
	    p->setPen( cg.shadow() );
	    p->drawPoint( x + w - 2, y + 1 );
	    // bottom right corner:
	    p->setPen( cg.background() );
	    p->drawPoint( x + w - 1, y + h - 1 );
	    p->drawPoint( x + w - 2, y + h - 1 );
	    p->drawPoint( x + w - 1, y + h - 2 );
	    p->setPen( cg.shadow() );
	    p->drawPoint( x + w - 2, y + h - 2 );
	    p->setPen( cg.dark() );
	    p->drawPoint( x + w - 3, y + h - 3 );

	    if ( sub & SC_ComboBoxArrow ) {
		TQRect rTmp = querySubControlMetrics( CC_ComboBox, ceData, elementFlags,
						     SC_ComboBoxArrow, opt, widget );
		int xx = rTmp.x(),
		    yy = rTmp.y(),
		    ww = rTmp.width(),
		    hh = rTmp.height();
		// the bright side

		p->setPen( cg.mid() );
		p->drawLine( xx, yy+2, xx, yy+hh-3 );

		p->setPen( cg.button() );
		p->drawLine( xx+1, yy+1, xx+ww-2, yy+1 );
		p->drawLine( xx+1, yy+1, xx+1, yy+hh-2 );

		p->setPen( cg.light() );
		p->drawLine( xx+2, yy+2, xx+2, yy+hh-2 );
		p->drawLine( xx+2, yy+2, xx+ww-2, yy+2 );


		// the dark side!

		p->setPen( cg.mid() );
		p->drawLine( xx+3, yy+hh-3 ,xx+ww-3, yy+hh-3 );
		p->drawLine( xx+ww-3, yy+3, xx+ww-3, yy+hh-3 );

		p->setPen( cg.dark() );
		p->drawLine( xx+2, yy+hh-2 ,xx+ww-2, yy+hh-2 );
		p->drawLine( xx+ww-2, yy+2, xx+ww-2, yy+hh-2 );

		p->setPen( cg.shadow() );
		p->drawLine( xx+1, yy+hh-1,xx+ww-1, yy+hh-1 );
		p->drawLine( xx+ww-1, yy, xx+ww-1, yy+hh-1 );

		// top right corner:
		p->setPen( cg.background() );
		p->drawPoint( xx + ww - 1, yy );
		p->drawPoint( xx + ww - 2, yy );
		p->drawPoint( xx + ww - 1, yy + 1 );
		p->setPen( cg.shadow() );
		p->drawPoint( xx + ww - 2, yy + 1 );
		// bottom right corner:
		p->setPen( cg.background() );
		p->drawPoint( xx + ww - 1, yy + hh - 1 );
		p->drawPoint( xx + ww - 2, yy + hh - 1 );
		p->drawPoint( xx + ww - 1, yy + hh - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( xx + ww - 2, yy + hh - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( xx + ww - 3, yy + hh - 3 );
		p->setPen( cg.mid() );
		p->drawPoint( xx + ww - 4, yy + hh - 4 );

		// and the arrows
		p->setPen( cg.foreground() );
		TQPointArray a ;
		a.setPoints(  7, -3,1, 3,1, -2,0, 2,0, -1,-1, 1,-1, 0,-2  );
		a.translate( xx + ww / 2, yy + hh / 2 - 3 );
		p->drawLineSegments( a, 0, 3 );		// draw arrow
		p->drawPoint( a[6] );
		a.setPoints( 7, -3,-1, 3,-1, -2,0, 2,0, -1,1, 1,1, 0,2 );
		a.translate( xx + ww / 2, yy + hh / 2 + 2 );
		p->drawLineSegments( a, 0, 3 );		// draw arrow
		p->drawPoint( a[6] );

	    }
#ifndef TQT_NO_COMBOBOX
	    if ( sub & SC_ComboBoxEditField ) {
		// sadly this is pretty much the windows code, except
		// for the first fillRect call...
		TQRect re =
		    TQStyle::visualRect( querySubControlMetrics( CC_ComboBox,
								ceData, elementFlags,
								SC_ComboBoxEditField, TQStyleOption::Default, widget ),
					ceData, elementFlags );
		if ( ( elementFlags & CEF_HasFocus ) && (!( elementFlags & CEF_IsEditable )) )
		    p->fillRect( re.x() + 1, re.y() + 1,
				 re.width() - 2, re.height() - 2,
				 cg.brush( TQColorGroup::Highlight ) );

		if ( elementFlags & CEF_HasFocus ) {
		    p->setPen( cg.highlightedText() );
		    p->setBackgroundColor( cg.highlight() );
		} else {
		    p->setPen( cg.text() );
		    p->setBackgroundColor( cg.background() );
		}

		if ( ( elementFlags & CEF_HasFocus ) && (!( elementFlags & CEF_IsEditable )) ) {
		    TQRect re =
			TQStyle::visualRect( subRect( SR_ComboBoxFocusRect, ceData, elementFlags,
						     widget ),
					    ceData, elementFlags );
		    drawPrimitive( PE_FocusRect, p, ceData, elementFlags, re, cg,
				   Style_FocusAtBorder,
				   TQStyleOption(cg.highlight()));
		}
		if ( elementFlags & CEF_IsEditable ) {
		    // need this for the moment...
		    // was the code in comboButton rect
		    TQRect ir( x + 3, y + 3,
			      w - 6 - 16, h - 6 );
		    if ( TQApplication::reverseLayout() )
			ir.moveBy( 16, 0 );
		    // end comboButtonRect...
		    ir.setRect( ir.left() - 1, ir.top() - 1, ir.width() + 2,
				ir.height() + 2 );
		    qDrawShadePanel( p, ir, cg, TRUE, 2, 0 );
		}
	    }
#endif
	    break;
	}
    case CC_Slider:
	{
#ifndef TQT_NO_SLIDER
	    int thickness = pixelMetric( PM_SliderControlThickness, ceData, elementFlags, widget );
	    int len = pixelMetric( PM_SliderLength, ceData, elementFlags, widget );
	    int ticks = ceData.tickMarkSetting;

	    TQRect groove = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderGroove,
						  opt, widget),
		  handle = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderHandle,
						  opt, widget);

	    if ((sub & SC_SliderGroove) && groove.isValid()) {
		p->fillRect( groove, cg.brush(TQColorGroup::Background) );

		int x, y, w, h;
		int mid = thickness / 2;

		if ( ticks & TQSlider::Above )
		    mid += len / 8;
		if ( ticks & TQSlider::Below )
		    mid -= len / 8;

		if ( ceData.orientation == Horizontal ) {
		    x = 0;
		    y = groove.y() + mid - 3;
		    w = ceData.rect.width();
		    h = 7;
		} else {
		    x = groove.x() + mid - 3;
		    y = 0;
		    w = 7;
		    h = ceData.rect.height();
		}

		p->fillRect( x, y, w, h, cg.brush( TQColorGroup::Dark ));
		// the dark side
		p->setPen( cg.dark() );
		p->drawLine( x, y, x + w - 1, y );
		p->drawLine( x, y, x, y + h - 1);
		p->setPen( cg.shadow() );
		p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
		p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );
		// the bright side!
		p->setPen(cg.shadow());
		p->drawLine( x + 1,  y + h - 2, x + w - 2,  y + h - 2 );
		p->drawLine( x + w - 2, y + 1, x + w - 2, y + h - 2 );
		p->setPen( cg.light() );
		p->drawLine( x, y + h - 1, x + w - 1, y + h - 1 );
		p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );
		// top left corner:
		p->setPen(cg.background());
		p->drawPoint( x, y );
		p->drawPoint( x + 1, y );
		p->drawPoint( x, y + 1 );
		p->setPen(cg.shadow());
		p->drawPoint( x + 1, y + 1 );
		// bottom left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y + h - 1 );
		p->drawPoint( x + 1, y + h - 1 );
		p->drawPoint( x, y + h - 2 );
		p->setPen( cg.light() );
		p->drawPoint( x + 1, y + h - 2 );
		// top right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y );
		p->drawPoint( x + w - 2, y );
		p->drawPoint( x + w - 1, y + 1 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 2, y + 1 );
		// bottom right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y + h - 1 );
		p->drawPoint( x + w - 2, y + h - 1 );
		p->drawPoint( x + w - 1, y + h - 2 );
		p->setPen( cg.light() );
		p->drawPoint( x + w - 2, y + h - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 3, y + h - 3 );
		// ### end slider groove

		if ( how & Style_HasFocus )
		    drawPrimitive( PE_FocusRect, p, ceData, elementFlags, groove, cg );
	    }

	    if ((sub & SC_SliderHandle) && handle.isValid()) {
		const TQColor c0 = cg.shadow();
		const TQColor c1 = cg.dark();
		const TQColor c3 = cg.light();

		int x1 = handle.x();
		int x2 = handle.x() + handle.width() - 1;
		int y1 = handle.y();
		int y2 = handle.y() + handle.height() - 1;
		int mx = handle.width() / 2;
		int my = handle.height() / 2;

		if ( ceData.orientation == Vertical ) {
		    // Background
		    TQBrush oldBrush = p->brush();
		    p->setBrush( cg.brush( TQColorGroup::Button ) );
		    p->setPen( NoPen );
		    TQPointArray a(6);
		    a.setPoint( 0, x1 + 1, y1 + 1 );
		    a.setPoint( 1, x2 - my + 2, y1 + 1 );
		    a.setPoint( 2, x2 - 1, y1 + my - 1 );
		    a.setPoint( 3, x2 - 1, y2 - my + 1 );
		    a.setPoint( 4, x2 - my + 2, y2 - 1 );
		    a.setPoint( 5, x1 + 1, y2 - 1 );
		    p->drawPolygon( a );
		    p->setBrush( oldBrush );

		    // shadow border
		    p->setPen( c0 );
		    p->drawLine( x1, y1 + 1, x1,y2 - 1 );
		    p->drawLine( x2 - my + 2, y1, x2, y1 + my - 2 );
		    p->drawLine( x2 - my + 2, y2, x2, y1 + my + 2 );
		    p->drawLine( x2, y1 + my - 2, x2, y1 + my + 2 );
		    p->drawLine( x1 + 1, y1, x2 - my + 2, y1 );
		    p->drawLine( x1 + 1, y2, x2 - my + 2, y2 );

		    // light shadow
		    p->setPen( c3 );
		    p->drawLine( x1 + 1, y1 + 2, x1 + 1, y2 - 2 );
		    p->drawLine( x1 + 1, y1 + 1, x2 - my + 2, y1 + 1 );
		    p->drawLine( x2 - my + 2, y1 + 1, x2 - 1, y1 + my - 2 );

		    // dark shadow
		    p->setPen(c1);
		    p->drawLine( x2 - 1, y1 + my - 2, x2 - 1, y1 + my + 2 );
		    p->drawLine( x2 - my + 2, y2 - 1, x2 - 1, y1 + my + 2 );
		    p->drawLine( x1 + 1, y2 - 1, x2 -my + 2, y2 - 1 );

		    drawRiffles( p, handle.x(), handle.y() + 2, handle.width() - 3,
				 handle.height() - 4, cg, TRUE );
		} else {  // Horizontal
		    TQBrush oldBrush = p->brush();
		    p->setBrush( cg.brush( TQColorGroup::Button ) );
		    p->setPen( NoPen );
		    TQPointArray a(6);
		    a.setPoint( 0, x2 - 1, y1 + 1 );
		    a.setPoint( 1, x2 - 1, y2 - mx + 2 );
		    a.setPoint( 2, x2 - mx + 1, y2 - 1 );
		    a.setPoint( 3, x1 + mx - 1, y2 - 1 );
		    a.setPoint( 4, x1 + 1, y2 - mx + 2 );
		    a.setPoint( 5, x1 + 1, y1 + 1 );
		    p->drawPolygon( a );
		    p->setBrush( oldBrush );

		    // shadow border
		    p->setPen( c0 );
		    p->drawLine( x1 + 1, y1, x2 - 1, y1 );
		    p->drawLine( x1, y2 - mx + 2, x1 + mx - 2, y2 );
		    p->drawLine( x2, y2 - mx + 2, x1 + mx + 2, y2 );
		    p->drawLine( x1 + mx - 2, y2, x1 + mx + 2, y2 );
		    p->drawLine( x1, y1 + 1, x1, y2 - mx + 2 );
		    p->drawLine( x2, y1 + 1, x2, y2 - mx + 2 );

		    // light shadow
		    p->setPen(c3);
		    p->drawLine( x1 + 1, y1 + 1, x2 - 1, y1 + 1 );
		    p->drawLine( x1 + 1, y1 + 1, x1 + 1, y2 - mx + 2 );

		    // dark shadow
		    p->setPen(c1);
		    p->drawLine( x2 - 1, y1 + 1, x2 - 1, y2 - mx + 2 );
		    p->drawLine( x1 + 1, y2 - mx + 2, x1 + mx - 2, y2 - 1 );
		    p->drawLine( x2 - 1, y2 - mx + 2, x1 + mx + 2, y2 - 1 );
		    p->drawLine( x1 + mx - 2, y2 - 1, x1 + mx + 2, y2 - 1 );

		    drawRiffles( p, handle.x() + 2, handle.y(), handle.width() - 4,
				 handle.height() - 5, cg, FALSE );
		}
	    }

	    if ( sub & SC_SliderTickmarks )
		TQCommonStyle::drawComplexControl( control, p, ceData, elementFlags, r,
						  cg, how, SC_SliderTickmarks,
						  subActive, opt, widget );
#endif
	    break;
	}
    default:
	TQWindowsStyle::drawComplexControl( control, p, ceData, elementFlags, r, cg,
					   how, sub, subActive, opt, widget );
	break;
    }
}



/*!\reimp
 */
TQRect TQPlatinumStyle::querySubControlMetrics( ComplexControl control,
					      const TQStyleControlElementData &ceData,
					      ControlElementFlags elementFlags,
					      SubControl sc,
					      const TQStyleOption& opt,
					      const TQWidget *widget ) const
{
    switch( control ) {
#ifndef TQT_NO_COMBOBOX
    case CC_ComboBox:
	switch( sc ) {
	case SC_ComboBoxArrow: {
	    TQRect ir = ceData.rect;
	    int xx;
	    if( TQApplication::reverseLayout() )
		xx = ir.x();
	    else
		xx = ir.x() + ir.width() - 20;
	    return TQRect( xx, ir.y(), 20, ir.height()); }
	default:
	    break;
	}
	break;
#endif
#ifndef TQT_NO_SCROLLBAR
    case CC_ScrollBar: {
	int sliderStart = ceData.startStep;
	int sbextent = pixelMetric( PM_ScrollBarExtent, ceData, elementFlags, widget );
	int maxlen = ((ceData.orientation == TQt::Horizontal) ?
		      ceData.rect.width() : ceData.rect.height()) - ( sbextent * 2 );

	int sliderlen;

	// calculate length
	if ( ceData.maxSteps != ceData.minSteps ) {
	    uint range = ceData.maxSteps - ceData.minSteps;
	    sliderlen = ( ceData.pageStep * maxlen ) /
			( range + ceData.pageStep );

	    int slidermin = pixelMetric( PM_ScrollBarSliderMin, ceData, elementFlags, widget );
	    if ( sliderlen < slidermin || range > INT_MAX / 2 )
		sliderlen = slidermin;
	    if ( sliderlen > maxlen )
		sliderlen = maxlen;
	} else {
	    sliderlen = maxlen;
	}

	switch ( sc ) {
	case SC_ScrollBarSubLine:
	    if ( ceData.orientation == TQt::Horizontal ) {
		int buttonw = TQMIN( ceData.rect.width() / 2, sbextent );
		return TQRect( ceData.rect.width() - 2 * buttonw, 0, buttonw, sbextent );
	    } else {
		int buttonh = TQMIN( ceData.rect.height() / 2, sbextent );
		return TQRect( 0, ceData.rect.height() - 2 * buttonh, sbextent, buttonh );
	    }
	case SC_ScrollBarAddLine:
	    if ( ceData.orientation == TQt::Horizontal ) {
		int buttonw = TQMIN( ceData.rect.width() / 2, sbextent );
		return TQRect( ceData.rect.width() - buttonw, 0, sbextent, buttonw );
	    } else {
		int buttonh = TQMIN( ceData.rect.height() / 2, sbextent );
		return TQRect(0, ceData.rect.height() - buttonh, sbextent, buttonh );
	    }
	case SC_ScrollBarSubPage:
	    if ( ceData.orientation == TQt::Horizontal )
		return TQRect( 1, 0, sliderStart, sbextent );
	    return TQRect( 0, 1, sbextent, sliderStart );
	case SC_ScrollBarAddPage:
	    if ( ceData.orientation == TQt::Horizontal )
		return TQRect( sliderStart + sliderlen, 0, maxlen - sliderStart - sliderlen, sbextent );
	    return TQRect( 0, sliderStart + sliderlen, sbextent, maxlen - sliderStart - sliderlen );
	case SC_ScrollBarGroove:
	    if ( ceData.orientation == TQt::Horizontal )
		return TQRect( 1, 0, ceData.rect.width() - sbextent * 2, ceData.rect.height() );
	    return TQRect( 0, 1, ceData.rect.width(), ceData.rect.height() - sbextent * 2 );
	default:
	    break;
	}
	break; }
#endif
#ifndef TQT_NO_SLIDER
    case CC_Slider: {
	int tickOffset = pixelMetric( PM_SliderTickmarkOffset, ceData, elementFlags, widget);
	int thickness = pixelMetric( PM_SliderControlThickness, ceData, elementFlags, widget);
	int mid = thickness / 2;
	int ticks = ceData.tickMarkSetting;
	int len = pixelMetric( PM_SliderLength, ceData, elementFlags, widget );

	switch ( sc ) {
	case SC_SliderGroove:
	    if ( ticks & TQSlider::Above )
		mid += len / 8;
	    if ( ticks & TQSlider::Below )
		mid -= len / 8;
	    if ( ceData.orientation == TQSlider::Horizontal )
		return TQRect( 0, tickOffset, ceData.rect.width(), thickness );
	    return TQRect( tickOffset, 0, thickness, ceData.rect.height() );
	default:
	    break;
	}
	break; }
#endif
    default:
	break;
    }
    return TQWindowsStyle::querySubControlMetrics( control, ceData, elementFlags, sc, opt, widget );
}


/*!\reimp
 */
int TQPlatinumStyle::pixelMetric( PixelMetric metric, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags,
				 const TQWidget *widget ) const
{
    int ret;
    switch( metric ) {
    case PM_ButtonDefaultIndicator:
	ret = 3;
	break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;
    case PM_IndicatorWidth:
	ret = 15;
	break;
    case PM_IndicatorHeight:
	ret = 13;
	break;
    case PM_ExclusiveIndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
	ret = 15;
	break;
    case PM_SliderLength:
	ret = 17;
	break;
    case PM_MaximumDragDistance:
	ret = -1;
	break;
    default:
	ret = TQWindowsStyle::pixelMetric( metric, ceData, elementFlags, widget );
	break;
    }
    return ret;
}

/*!\reimp
 */
TQRect TQPlatinumStyle::subRect( SubRect r, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget *widget ) const
{
    TQRect rect;
    switch ( r ) {
    case SR_ComboBoxFocusRect:
	{
	    TQRect tmpR = ceData.rect;
	    rect = TQRect( tmpR.x() + 4, tmpR.y() + 4, tmpR.width() - 8 - 16,
			  tmpR.height() - 8 );
	    break;
	}
    default:
	rect = TQWindowsStyle::subRect( r, ceData, elementFlags, widget );
	break;
    }
    return rect;
}

/*!
    Mixes two colors \a c1 and \a c2 to a new color.
*/
TQColor TQPlatinumStyle::mixedColor(const TQColor &c1, const TQColor &c2) const
{
    int h1,s1,v1,h2,s2,v2;
    c1.hsv(&h1,&s1,&v1);
    c2.hsv(&h2,&s2,&v2);
    return TQColor( (h1+h2)/2, (s1+s2)/2, (v1+v2)/2, TQColor::Hsv );
}

/*!
    Draws the nifty Macintosh decoration used on sliders using painter
    \a p and colorgroup \a g. \a x, \a y, \a w, \a h and \a horizontal
    specify the geometry and orientation of the riffles.
*/
void TQPlatinumStyle::drawRiffles( TQPainter* p,  int x, int y, int w, int h,
				  const TQColorGroup &g, bool horizontal ) const
{
    if (!horizontal) {
	if (h > 20) {
	    y += (h-20)/2 ;
	    h = 20;
	}
	if (h > 8) {
	    int n = h / 4;
	    int my = y+h/2-n;
	    int i ;
	    p->setPen(g.light());
	    for (i=0; i<n; i++) {
		p->drawLine(x+3, my+2*i, x+w-5, my+2*i);
	    }
	    p->setPen(g.dark());
	    my++;
	    for (i=0; i<n; i++) {
		p->drawLine(x+4, my+2*i, x+w-4, my+2*i);
	    }
	}
    }
    else {
	if (w > 20) {
	    x += (w-20)/2 ;
	    w = 20;
	}
	if (w > 8) {
	    int n = w / 4;
	    int mx = x+w/2-n;
	    int i ;
	    p->setPen(g.light());
	    for (i=0; i<n; i++) {
		p->drawLine(mx+2*i, y+3, mx + 2*i, y+h-5);
	    }
	    p->setPen(g.dark());
	    mx++;
	    for (i=0; i<n; i++) {
		p->drawLine(mx+2*i, y+4, mx + 2*i, y+h-4);
	    }
	}
    }
}


#endif
