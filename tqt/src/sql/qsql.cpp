/****************************************************************************
**
** Implementation of TQSql class
**
** Created : 2000-11-03
**
** Copyright (C) 2000-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the sql module of the TQt GUI Toolkit.
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


/*!
    \class TQSql ntqsql.h
    \brief The TQSql class is a namespace for TQt SQL identifiers that
    need to be global-like.

    \ingroup database
    \mainclass
    \module sql

    Normally, you can ignore this class. Several TQt SQL classes
    inherit it, so all the identifiers in the TQt SQL namespace are
    visible without qualification.
*/

/*!
    \enum TQSql::Confirm

    This enum type describes edit confirmations.

    \value Yes
    \value No
    \value Cancel
*/

/*!
    \enum TQSql::Op

    This enum type describes edit operations.

    \value None
    \value Insert
    \value Update
    \value Delete
*/


/*!
    \enum TQSql::Location

    This enum type describes SQL navigation locations.

    \value BeforeFirst
    \value AfterLast
*/

/*!
    \enum TQSql::ParameterType

    This enum is used to set the type of a bind parameter

    \value In  the bind parameter is used to put data into the database
    \value Out  the bind parameter is used to receive data from the database
    \value InOut  the bind parameter is used to put data into the
	database; it will be overwritten with output data on executing
	a query.
*/

/*!
    \enum TQSql::TableType

    This enum type describes types of tables

    \value Tables  All the tables visible to the user
    \value SystemTables  Internal tables used by the DBMS
    \value Views  All the views visible to the user
    \value AllTables  All of the above
*/

/*!
    \fn TQSql::TQSql()

    Constructs a TQt SQL namespace class
*/
