/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <tqapplication.h>
#include <tqdialog.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqsqldatabase.h>
#include <tqsqlcursor.h>
#include <tqsqlform.h>
#include <tqsqlpropertymap.h>
#include "../connection.h"

class CustomEdit : public TQLineEdit
{
    TQ_OBJECT
    TQ_PROPERTY( TQString upperLine READ upperLine WRITE setUpperLine )
    public:
	CustomEdit( TQWidget *parent=0, const char *name=0 );
	TQString upperLine() const;
	void setUpperLine( const TQString &line );
    public slots:
	void changed( const TQString &line );
    private:
	TQString upperLineText;
};


class FormDialog : public TQDialog
{
    TQ_OBJECT
    public:
	FormDialog();
	~FormDialog();
    public slots:
	void save();
    private:
	TQSqlCursor *staffCursor;
	TQSqlForm *sqlForm;
	TQSqlPropertyMap *propMap;
	TQSqlIndex idIndex;
};


