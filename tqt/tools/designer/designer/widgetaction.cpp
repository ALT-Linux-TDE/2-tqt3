/**********************************************************************
** Copyright (C) 2000-2008 Trolltech ASA.  All rights reserved.
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

#include "widgetaction.h"
#include <ntqwidget.h>
#include <ntqtoolbutton.h>
#include "mainwindow.h"
#include <ntqstylefactory.h>
#include <ntqstyle.h>

static TQStyle *windowsStyle = 0;

void WidgetAction::addedTo( TQWidget *w, TQWidget *container )
{
    if ( ::tqt_cast<TQToolButton*>(w) && ::tqt_cast<TQScrollView*>(container->parent()->parent()) ) {
	if ( !windowsStyle )
	    windowsStyle = TQStyleFactory::create( "windows" );
	w->setStyle( windowsStyle );
	( (TQToolButton*)w )->setUsesTextLabel( TRUE );
	( (TQToolButton*)w )->setTextPosition( TQToolButton::Right );
	w->setBackgroundMode( container->backgroundMode() );
    }
}

WidgetAction::~WidgetAction()
{
    if ( MainWindow::self )
	MainWindow::self->toolActions.removeRef( this );
}

void WidgetAction::init( const TQString &g )
{
    MainWindow::self->toolActions.append( this );
    grp = g;
}
