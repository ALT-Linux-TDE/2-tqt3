/****************************************************************************
**
** Implementation of TQDesktopWidget class.
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

#include "ntqdesktopwidget.h"
#include "ntqapplication.h"
#include "ntqobjectlist.h"
#include "qt_x11_p.h"
#include <cstdlib>

// defined in qwidget_x11.cpp
extern int qt_x11_create_desktop_on_screen;

// defined in qapplication_x11.cpp
extern Atom qt_net_workarea;
extern bool qt_net_supports(Atom atom);

// function to update the workarea of the screen
static bool qt_desktopwidget_workarea_dirty = TRUE;
void qt_desktopwidget_update_workarea()
{
    qt_desktopwidget_workarea_dirty = TRUE;
}


class TQSingleDesktopWidget : public TQWidget
{
public:
    TQSingleDesktopWidget();
    ~TQSingleDesktopWidget();
};

TQSingleDesktopWidget::TQSingleDesktopWidget()
    : TQWidget( 0, "desktop", WType_Desktop )
{
}

TQSingleDesktopWidget::~TQSingleDesktopWidget()
{
    while ( children() )
        removeChild( children()->getFirst() );
}


class TQDesktopWidgetPrivate
{
public:
    TQDesktopWidgetPrivate();
    ~TQDesktopWidgetPrivate();

    void init();

    bool use_xinerama;
    int defaultScreen;
    int screenCount;

    TQWidget **screens;
    TQRect *rects;
    TQRect *workareas;
};

TQDesktopWidgetPrivate::TQDesktopWidgetPrivate()
    : use_xinerama(FALSE), defaultScreen(0), screenCount(1),
      screens( 0 ), rects( 0 ), workareas( 0 )
{
}

TQDesktopWidgetPrivate::~TQDesktopWidgetPrivate()
{
    if ( screens ) {
	for ( int i = 0; i < screenCount; ++i ) {
	    if (i == defaultScreen) continue;
	    delete screens[i];
	    screens[i] = 0;
	}

	free(screens);
    }

    if ( rects )     delete [] rects;
    if ( workareas ) delete [] workareas;
}

void TQDesktopWidgetPrivate::init()
{
    // get the screen count
    int newScreenCount;

#ifndef QT_NO_XINERAMA
    XineramaScreenInfo *xinerama_screeninfo = 0;
    int unused;
    use_xinerama = (XineramaQueryExtension(TQPaintDevice::x11AppDisplay(),
					   &unused, &unused) &&
		    XineramaIsActive(TQPaintDevice::x11AppDisplay()));

    if (use_xinerama) {
	xinerama_screeninfo =
	    XineramaQueryScreens(TQPaintDevice::x11AppDisplay(), &newScreenCount);

    if (xinerama_screeninfo)
	defaultScreen = 0;
    } else
#endif // QT_NO_XINERAMA
    {
	defaultScreen = DefaultScreen(TQPaintDevice::x11AppDisplay());
	newScreenCount = ScreenCount(TQPaintDevice::x11AppDisplay());
        use_xinerama = false;
    }

    delete [] rects;
    rects     = new TQRect[ newScreenCount ];
    delete [] workareas;
    workareas = new TQRect[ newScreenCount ];

    // get the geometry of each screen
    int i, j, x, y, w, h;
    for ( i = 0, j = 0; i < newScreenCount; i++ ) {

#ifndef QT_NO_XINERAMA
	if (use_xinerama) {
	    x = xinerama_screeninfo[i].x_org;
	    y = xinerama_screeninfo[i].y_org;
	    w = xinerama_screeninfo[i].width;
	    h = xinerama_screeninfo[i].height;
	} else
#endif // QT_NO_XINERAMA
	    {
		x = 0;
		y = 0;
		w = WidthOfScreen(ScreenOfDisplay(TQPaintDevice::x11AppDisplay(), i));
		h = HeightOfScreen(ScreenOfDisplay(TQPaintDevice::x11AppDisplay(), i));
	    }

	workareas[i] = TQRect();
	rects[j].setRect(x, y, w, h);

        // overlapping?
        if (j > 0 && rects[j-1].intersects(rects[j])) {
            // pick the bigger one, ignore the other
            if ((rects[j].width()*rects[j].height()) >
                (rects[j-1].width()*rects[j-1].height()))
            rects[j-1] = rects[j];
        }
        else
            j++;
    }

    if (screens) {
        // leaks TQWidget* pointers on purpose, can't delete them as pointer escapes
        screens = (TQWidget**) realloc(screens, j * sizeof(TQWidget*));
        if (j > screenCount)
            memset(&screens[screenCount], 0, (j-screenCount) * sizeof(TQWidget*));
    }

    screenCount = j;

#ifndef QT_NO_XINERAMA
    if (use_xinerama && screenCount == 1)
        use_xinerama = false;

    if (xinerama_screeninfo)
	XFree(xinerama_screeninfo);
#endif // QT_NO_XINERAMA

}

// the TQDesktopWidget itself will be created on the default screen
// as qt_x11_create_desktop_on_screen defaults to -1
TQDesktopWidget::TQDesktopWidget()
    : TQWidget( 0, "desktop", WType_Desktop )
{
    d = new TQDesktopWidgetPrivate();

    /*
      we don't call d->init() here, since the initial resize event
      will end up calling init() a second time, which is inefficient.
      instead, for the sending of all posted event to the desktop
      widget (including the initial resize event, which calls
      d->init()).
    */
    TQApplication::sendPostedEvents( this, 0 );
}

TQDesktopWidget::~TQDesktopWidget()
{
    delete d;
}

bool TQDesktopWidget::isVirtualDesktop() const
{
    return d->use_xinerama;
}

int TQDesktopWidget::primaryScreen() const
{
    return d->defaultScreen;
}

int TQDesktopWidget::numScreens() const
{
    return d->screenCount;
}

TQWidget *TQDesktopWidget::screen( int screen )
{
    if (d->use_xinerama)
	return this;

    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    if ( ! d->screens ) {
	d->screens = (TQWidget**) calloc( d->screenCount, sizeof(TQWidget*));
	d->screens[ d->defaultScreen ] = this;
    }

    if ( ! d->screens[screen] ||               // not created yet
	 ! d->screens[screen]->isDesktop() ) { // reparented away
	qt_x11_create_desktop_on_screen = screen;
	d->screens[screen] = new TQSingleDesktopWidget;
	qt_x11_create_desktop_on_screen = -1;
    }

    return d->screens[screen];
}

const TQRect& TQDesktopWidget::availableGeometry( int screen ) const
{
    if ( qt_desktopwidget_workarea_dirty ) {
	// the workareas are dirty, invalidate them
	for ( int i = 0; i < d->screenCount; ++i )
	    d->workareas[i] = TQRect();
	qt_desktopwidget_workarea_dirty = FALSE;
    }

    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    if ( d->workareas[screen].isValid() )
	return d->workareas[screen];

    if ( ! isVirtualDesktop() && qt_net_supports( qt_net_workarea ) ) {
	Atom ret;
	int format, e;
	unsigned char *data = 0;
	unsigned long nitems, after;

	e = XGetWindowProperty( TQPaintDevice::x11AppDisplay(),
				TQPaintDevice::x11AppRootWindow( screen ),
				qt_net_workarea, 0, 4, False, XA_CARDINAL,
				&ret, &format, &nitems, &after, &data );

	if (e == Success && ret == XA_CARDINAL &&
	    format == 32 && nitems == 4) {
	    long *workarea = (long *) data;
	    d->workareas[screen].setRect( workarea[0], workarea[1],
					  workarea[2], workarea[3] );
	} else {
	    d->workareas[screen] = screenGeometry(screen);
	}
	if ( data )
	    XFree( data );
    } else {
	d->workareas[screen] = screenGeometry(screen);
    }

    return d->workareas[screen];
}

const TQRect& TQDesktopWidget::screenGeometry( int screen ) const
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    return d->rects[ screen ];
}

int TQDesktopWidget::screenNumber( TQWidget *widget ) const
{
    if ( !widget )
	return d->defaultScreen;

#ifndef QT_NO_XINERAMA
    if (d->use_xinerama) {
	// this is how we do it for xinerama
	TQRect frame = widget->frameGeometry();
	if ( !widget->isTopLevel() )
	    frame.moveTopLeft( widget->mapToGlobal( TQPoint( 0, 0 ) ) );

	int maxSize = -1;
	int maxScreen = -1;

	for ( int i = 0; i < d->screenCount; ++i ) {
	    TQRect sect = d->rects[i].intersect( frame );
	    int size = sect.width() * sect.height();
	    if ( size > maxSize && sect.width() > 0 && sect.height() > 0 ) {
		maxSize = size;
		maxScreen = i;
	    }
	}
	return maxScreen;
    }
#endif // QT_NO_XINERAMA

    return widget->x11Screen();
}

int TQDesktopWidget::screenNumber( const TQPoint &point ) const
{
    for ( int i = 0; i < d->screenCount; ++i ) {
	if ( d->rects[i].contains( point ) )
	    return i;
    }
    return -1;
}

void TQDesktopWidget::resizeEvent( TQResizeEvent *event )
{
    d->init();
    qt_desktopwidget_workarea_dirty = TRUE;
    TQWidget::resizeEvent( event );
}
