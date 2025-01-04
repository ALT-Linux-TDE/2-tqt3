/**********************************************************************
** Copyright (C) 2005-2008 Trolltech ASA.  All rights reserved.
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

#ifndef BROWSER_H
#define BROWSER_H

#include <ntqobject.h>

class Editor;
class TQTextCursor;
class TQTextParagraph;
class TQTextFormat;

class EditorBrowser : public TQObject
{
    TQ_OBJECT

public:
    EditorBrowser( Editor *e );
    ~EditorBrowser();

    bool eventFilter( TQObject *o, TQEvent *e );
    virtual void setCurrentEdior( Editor *e );
    virtual void addEditor( Editor *e );
    virtual bool findCursor( const TQTextCursor &c, TQTextCursor &from, TQTextCursor &to );
    virtual void showHelp( const TQString & ) {}

protected:
    Editor *curEditor;
    TQTextParagraph *oldHighlightedParag;
    TQString lastWord;
    TQTextFormat *highlightedFormat;

};

#endif