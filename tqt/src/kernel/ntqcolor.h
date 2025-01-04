/****************************************************************************
**
** Definition of TQColor class
**
** Created : 940112
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the TQt GUI Toolkit.
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

#ifndef TQCOLOR_H
#define TQCOLOR_H

#ifndef QT_H
#include "ntqwindowdefs.h"
#include "ntqstringlist.h"
#endif // QT_H

const TQRgb  TQT_RGB_MASK    = 0x00ffffff;		// masks RGB values

TQ_EXPORT inline int tqRed( TQRgb rgb )		// get red part of RGB
{ return (int)((rgb >> 16) & 0xff); }

TQ_EXPORT inline int tqGreen( TQRgb rgb )		// get green part of RGB
{ return (int)((rgb >> 8) & 0xff); }

TQ_EXPORT inline int tqBlue( TQRgb rgb )		// get blue part of RGB
{ return (int)(rgb & 0xff); }

TQ_EXPORT inline int tqAlpha( TQRgb rgb )		// get alpha part of RGBA
{ return (int)((rgb >> 24) & 0xff); }

TQ_EXPORT inline TQRgb tqRgb( int r, int g, int b )// set RGB value
{ return (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

TQ_EXPORT inline TQRgb tqRgba( int r, int g, int b, int a )// set RGBA value
{ return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

TQ_EXPORT inline int tqGray( int r, int g, int b )// convert R,G,B to gray 0..255
{ return (r*11+g*16+b*5)/32; }

TQ_EXPORT inline int tqGray( TQRgb rgb )		// convert RGB to gray 0..255
{ return tqGray( tqRed(rgb), tqGreen(rgb), tqBlue(rgb) ); }


class TQ_EXPORT TQColor
{
public:
    enum Spec { Rgb, Hsv };

    TQColor();
    TQColor( int r, int g, int b );
    TQColor( int x, int y, int z, Spec );
    TQColor( TQRgb rgb, uint pixel=0xffffffff);
    TQColor( const TQString& name );
    TQColor( const char *name );
    TQColor( const TQColor & );
    TQColor &operator=( const TQColor & );

    bool   isValid() const;
    bool   isDirty() const;
    TQString name() const;
    void   setNamedColor( const TQString& name );

    TQRgb   rgb()    const;
    void   setRgb( int r, int g, int b );
    void   setRgb( TQRgb rgb );
    void   getRgb( int *r, int *g, int *b ) const { rgb( r, g, b ); }
    void   rgb( int *r, int *g, int *b ) const; // obsolete

    int	   red()    const;
    int	   green()  const;
    int	   blue()   const;

    void   setHsv( int h, int s, int v );
    void   getHsv( int *h, int *s, int *v ) const { hsv( h, s, v ); }
    void   hsv( int *h, int *s, int *v ) const; // obsolete
    void   getHsv( int &h, int &s, int &v ) const { hsv( &h, &s, &v ); } // obsolete

    TQColor light( int f = 150 ) const;
    TQColor dark( int f = 200 )	const;

    bool   operator==( const TQColor &c ) const;
    bool   operator!=( const TQColor &c ) const;

    uint   alloc();
    uint   pixel()  const;

#if defined(TQ_WS_X11)
    // ### in 4.0, make this take a default argument of -1 for default screen?
    uint alloc( int screen );
    uint pixel( int screen ) const;
#endif

    static int  maxColors();
    static int  numBitPlanes();

    static int  enterAllocContext();
    static void leaveAllocContext();
    static int  currentAllocContext();
    static void destroyAllocContext( int );

#if defined(TQ_WS_WIN)
    static const TQRgb* palette( int* numEntries = 0 );
    static int setPaletteEntries( const TQRgb* entries, int numEntries,
				  int base = -1 );
    static HPALETTE hPal()  { return hpal; }
    static uint	realizePal( TQWidget * );
#endif

    static void initialize();
    static void cleanup();
#ifndef TQT_NO_STRINGLIST
    static TQStringList colorNames();
#endif
    enum { Dirt = 0x44495254, Invalid = 0x49000000 };

private:
    void setSystemNamedColor( const TQString& name );
    void setPixel( uint pixel );
    static void initGlobalColors();
    static uint argbToPix32(TQRgb);
    static TQColor* globalColors();
    static bool color_init;
    static bool globals_init;
#if defined(TQ_WS_WIN)
    static HPALETTE hpal;
#endif
    static enum ColorModel { d8, d32 } colormodel;
    union {
	TQRgb argb;
	struct D8 {
	    TQRgb argb;
	    uchar pix;
	    uchar invalid;
	    uchar dirty;
	    uchar direct;
	} d8;
	struct D32 {
	    TQRgb argb;
	    uint pix;
	    bool invalid() const { return argb == TQColor::Invalid && pix == TQColor::Dirt; }
	    bool probablyDirty() const { return pix == TQColor::Dirt; }
	} d32;
    } d;
};


inline TQColor::TQColor()
{ d.d32.argb = Invalid; d.d32.pix = Dirt; }

inline TQColor::TQColor( int r, int g, int b )
{
    d.d32.argb = Invalid;
    d.d32.pix = Dirt;
    setRgb( r, g, b );
}

inline TQRgb TQColor::rgb() const
{ return d.argb; }

inline int TQColor::red() const
{ return tqRed(d.argb); }

inline int TQColor::green() const
{ return tqGreen(d.argb); }

inline int TQColor::blue() const
{ return tqBlue(d.argb); }

inline bool TQColor::isValid() const
{
    if ( colormodel == d8 )
	return !d.d8.invalid;
    else
	return !d.d32.invalid();
}

inline bool TQColor::operator==( const TQColor &c ) const
{
    return d.argb == c.d.argb && isValid() == c.isValid();
}

inline bool TQColor::operator!=( const TQColor &c ) const
{
    return !operator==(c);
}


/*****************************************************************************
  TQColor stream functions
 *****************************************************************************/

#ifndef TQT_NO_DATASTREAM
TQ_EXPORT TQDataStream &operator<<( TQDataStream &, const TQColor & );
TQ_EXPORT TQDataStream &operator>>( TQDataStream &, TQColor & );
#endif

#endif // TQCOLOR_H