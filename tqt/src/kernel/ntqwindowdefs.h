/****************************************************************************
**
** Definition of general window system dependent functions, types and
** constants
**
** Created : 931029
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

#ifndef TQWINDOWDEFS_H
#define TQWINDOWDEFS_H

#ifndef QT_H
#include "ntqobjectdefs.h"
#include "ntqstring.h"
#include "ntqnamespace.h"
#endif // QT_H

// Class forward definitions

class TQPaintDevice;
class TQPaintDeviceMetrics;
class TQWidget;
class TQWidgetMapper;
class TQDialog;
class TQColor;
class TQColorGroup;
class TQPalette;
class TQCursor;
class TQPoint;
class TQSize;
class TQRect;
class TQPointArray;
class TQPainter;
class TQRegion;
class TQFont;
class TQFontMetrics;
class TQFontInfo;
class TQPen;
class TQBrush;
class TQWMatrix;
class TQPixmap;
class TQBitmap;
class TQMovie;
class TQImage;
class TQImageIO;
class TQPicture;
class TQPrinter;
class TQAccel;
class TQTimer;
class TQTime;
class TQClipboard;


// Widget list (defined in ntqwidgetlist.h)

class TQWidgetList;
class TQWidgetListIt;


// Window system dependent definitions

#if defined(Q_WS_MAC)
#if QT_MACOSX_VERSION < 0x1020
typedef struct OpaqueEventLoopTimerRef* EventLoopTimerRef;
typedef struct OpaqueMenuHandle *MenuRef;
#else
typedef struct __EventLoopTimer*        EventLoopTimerRef;
typedef struct OpaqueMenuRef*        MenuRef;
#endif

#ifndef Q_WS_MACX
typedef struct CGContext *CGContextRef;
#endif
typedef struct OpaqueWindowGroupRef *WindowGroupRef;
typedef struct OpaqueGrafPtr *CGrafPtr;
typedef struct OpaquePMPrintSession *PMPrintSession;
typedef struct OpaquePMPrintSettings *PMPrintSettings;
typedef struct OpaquePMPageFormat *PMPageFormat;
typedef struct Point Point;
typedef struct OpaqueEventHandlerRef*   EventHandlerRef;
typedef struct OpaqueEventHandlerCallRef*  EventHandlerCallRef;
typedef struct OpaqueEventRef*          EventRef;
typedef long int OSStatus;
typedef struct OpaqueScrapRef *ScrapRef;
typedef struct OpaqueRgnHandle *RgnHandle;
typedef struct OpaqueWindowPtr *WindowPtr;
typedef WindowPtr WindowRef;
typedef struct OpaqueGrafPtr *GWorldPtr;
typedef GWorldPtr GrafPtr;
typedef struct GDevice **GDHandle;
typedef struct ColorTable ColorTable;
typedef struct BitMap BitMap;
typedef struct EventRecord EventRecord;
typedef void * MSG;
typedef int WId;
typedef struct AEDesc AppleEvent;

#endif // Q_WS_MAC

#if defined(Q_WS_WIN)
#include "qwindowdefs_win.h"
#endif // Q_WS_WIN


#if defined(Q_OS_TEMP)
#include "qwinfunctions_wce.h"
#endif // Q_OS_TEMP

#if defined(Q_WS_X11)

typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;
typedef struct _XGC *GC;
typedef struct _XRegion *Region;
typedef unsigned long  WId;

Q_EXPORT Display *tqt_xdisplay();
Q_EXPORT int	 tqt_xscreen();
Q_EXPORT WId	 tqt_xrootwin(); // ### REMOVE 4.0
Q_EXPORT WId	 tqt_xrootwin( int scrn ); // ### 4.0 add default arg of -1
Q_EXPORT GC	 tqt_xget_readonly_gc( int scrn, bool monochrome );
Q_EXPORT GC	 tqt_xget_temp_gc( int scrn, bool monochrome );

Q_EXPORT const char *tqAppClass();		// get application class

#endif // Q_WS_X11

#if defined(Q_WS_QWS)

typedef unsigned long  WId;
struct TQWSEvent;
class TQGfx;

#endif // Q_WS_QWS

class TQApplication;

#if defined(NEEDS_QMAIN)
#define main qMain
#endif

// Global platform-independent types and functions

typedef TQ_INT32 TQCOORD;				// coordinate type
const TQCOORD TQCOORD_MAX =  2147483647;
const TQCOORD TQCOORD_MIN = -TQCOORD_MAX - 1;

typedef unsigned int TQRgb;			// RGB triplet

Q_EXPORT const char *tqAppName();		// get application name

// Misc functions

typedef void (*TQtCleanUpFunction)();
Q_EXPORT void tqAddPostRoutine( TQtCleanUpFunction );
Q_EXPORT void tqRemovePostRoutine( TQtCleanUpFunction );

#if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with TQt 2.x
typedef TQtCleanUpFunction Q_CleanUpFunction;
#endif


#endif // TQWINDOWDEFS_H
