/****************************************************************************
**
** Implementation of TQHButtonGroup class
**
** Created : 990602
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

#include "ntqhbuttongroup.h"
#ifndef QT_NO_HBUTTONGROUP

/*!
    \class TQHButtonGroup ntqhbuttongroup.h
    \brief The TQHButtonGroup widget organizes TQButton widgets in a
    group with one horizontal row.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance

    TQHButtonGroup is a convenience class that offers a thin layer on
    top of TQButtonGroup. From a layout point of view it is effectively
    a TQHBox that offers a frame with a title and is specifically
    designed for buttons. From a functionality point of view it is a
    TQButtonGroup.

    \img qbuttongroup-h.png TQButtonGroup

    \sa TQVButtonGroup
*/

/*!
    Constructs a horizontal button group with no title.

    The \a parent and \a name arguments are passed to the TQWidget
    constructor.
*/
TQHButtonGroup::TQHButtonGroup( TQWidget *parent, const char *name )
    : TQButtonGroup( 1, Vertical /* sic! */, parent, name )
{
}

/*!
    Constructs a horizontal button group with the title \a title.

    The \a parent and \a name arguments are passed to the TQWidget
    constructor.
*/

TQHButtonGroup::TQHButtonGroup( const TQString &title, TQWidget *parent,
			    const char *name )
    : TQButtonGroup( 1, Vertical /* sic! */, title, parent, name )
{
}

/*!
    Destroys the horizontal button group, deleting its child widgets.
*/
TQHButtonGroup::~TQHButtonGroup()
{
}
#endif
