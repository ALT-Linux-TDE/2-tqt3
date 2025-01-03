/****************************************************************************
**
** Definition of the TQWizard class.
**
** Created : 990101
**
** Copyright (C) 1999-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the dialogs module of the TQt GUI Toolkit.
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

#ifndef TQWIZARDDIALOG_H
#define TQWIZARDDIALOG_H


#ifndef QT_H
#include "ntqdialog.h"
#endif // QT_H

#ifndef QT_NO_WIZARD

class TQHBoxLayout;
class TQWizardPrivate;

class Q_EXPORT TQWizard : public TQDialog
{
    TQ_OBJECT
    TQ_PROPERTY( TQFont titleFont READ titleFont WRITE setTitleFont )

public:
    TQWizard( TQWidget* parent=0, const char* name=0, bool modal=FALSE,
	     WFlags f=0 );
    ~TQWizard();

    void show();

    void setFont( const TQFont & font );

    virtual void addPage( TQWidget *, const TQString & );
    virtual void insertPage( TQWidget*, const TQString&, int );
    virtual void removePage( TQWidget * );

    TQString title( TQWidget * ) const;
    void setTitle( TQWidget *, const TQString & );
    TQFont titleFont() const;
    void setTitleFont( const TQFont & );

    virtual void showPage( TQWidget * );

    TQWidget * currentPage() const;

    TQWidget* page( int ) const;
    int pageCount() const;
    int indexOf( TQWidget* ) const;

    virtual bool appropriate( TQWidget * ) const;
    virtual void setAppropriate( TQWidget *, bool );

    TQPushButton * backButton() const;
    TQPushButton * nextButton() const;
    TQPushButton * finishButton() const;
    TQPushButton * cancelButton() const;
    TQPushButton * helpButton() const;

    bool eventFilter( TQObject *, TQEvent * );

public slots:
    virtual void setBackEnabled( TQWidget *, bool );
    virtual void setNextEnabled( TQWidget *, bool );
    virtual void setFinishEnabled( TQWidget *, bool );

    virtual void setHelpEnabled( TQWidget *, bool );

    // obsolete
    virtual void setFinish(  TQWidget *, bool ) {}

protected slots:
    virtual void back();
    virtual void next();
    virtual void help();

signals:
    void helpClicked();
    void selected( const TQString& );

protected:
    virtual void layOutButtonRow( TQHBoxLayout * );
    virtual void layOutTitleRow( TQHBoxLayout *, const TQString & );

private:
    void setBackEnabled( bool );
    void setNextEnabled( bool );

    void setHelpEnabled( bool );

    void setNextPage( TQWidget * );

    void updateButtons();

    void layOut();

    TQWizardPrivate *d;

private:	// Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    TQWizard( const TQWizard & );
    TQWizard& operator=( const TQWizard & );
#endif
};

#endif // QT_NO_WIZARD

#endif // TQWIZARD_H
