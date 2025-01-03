/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <tqapplication.h>
#include <tqcombobox.h>
#include <tqmap.h>
#include <tqpainter.h>
#include <tqsqldatabase.h>
#include <tqsqlcursor.h>
#include <tqsqleditorfactory.h>
#include <tqsqlpropertymap.h>
#include <tqdatatable.h>
#include "../connection.h"

class StatusPicker : public TQComboBox
{
    TQ_OBJECT
    TQ_PROPERTY( int statusid READ statusId WRITE setStatusId )
public:
    StatusPicker( TQWidget *parent=0, const char *name=0 );
    int statusId() const;
    void setStatusId( int id );
private:
    TQMap< int, int > index2id;
};


class CustomTable : public TQDataTable
{
    TQ_OBJECT
public:
    CustomTable( 
	    TQSqlCursor *cursor, bool autoPopulate = FALSE, 
	    TQWidget * parent = 0, const char * name = 0 ) : 
	TQDataTable( cursor, autoPopulate, parent, name ) {}
    void paintField(
	    TQPainter * p, const TQSqlField* field, const TQRect & cr, bool );

};


class CustomSqlEditorFactory : public TQSqlEditorFactory
{
    TQ_OBJECT
public:
    TQWidget *createEditor( TQWidget *parent, const TQSqlField *field );
};
