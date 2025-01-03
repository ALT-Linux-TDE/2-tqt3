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
#include "../connection.h"

class FormDialog : public TQDialog
{
    TQ_OBJECT
    public:
	FormDialog();
	~FormDialog();
    public slots:
	void save();
    private:
	TQSqlCursor staffCursor;
	TQSqlForm *sqlForm;
	TQSqlIndex idIndex;
};
