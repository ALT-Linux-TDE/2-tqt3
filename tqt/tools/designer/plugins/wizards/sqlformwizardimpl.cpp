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

#include "sqlformwizardimpl.h"

#include <ntqlistbox.h>
#include <ntqwidget.h>
#include <ntqcheckbox.h>
#include <ntqlineedit.h>

#include <ntqlabel.h>
#include <ntqgroupbox.h>
#include <ntqlayout.h>
#include <ntqregexp.h>
#include <ntqpushbutton.h>
#include <ntqmultilineedit.h>
#include <ntqlistview.h>
#include <ntqfeatures.h>
#include <ntqradiobutton.h>
#include <ntqspinbox.h>
#include <limits.h>

#ifndef TQT_NO_SQL
#include <ntqdatatable.h>
#include <ntqdatabrowser.h>
#include <ntqdataview.h>
#include <ntqsqleditorfactory.h>
#include <ntqsqlindex.h>
#include <ntqsqlcursor.h>
#endif

SqlFormWizard::SqlFormWizard( TQUnknownInterface *aIface, TQWidget *w,
			      TQWidget* parent, DesignerFormWindow *fw, const char* name, bool modal, WFlags fl )
    : SqlFormWizardBase( parent, name, modal, fl ), widget( w ), appIface( aIface ),
     mode( None )
{
    appIface->addRef();
    formWindow = fw;
    setFinishEnabled( finishPage, TRUE );

    /* set mode of operation */
    if ( ::tqt_cast<TQDataTable*>(widget) ) {
	setCaption( "Data Table Wizard" );
	mode = Table;
	setAppropriate( navigPage, FALSE );
	setAppropriate( layoutPage, FALSE );
	checkBoxAutoEdit->setChecked( FALSE );
    } else if ( ::tqt_cast<TQDataBrowser*>(widget) ) {
	setCaption( "Data Browser Wizard" );
	setAppropriate( tablePropertiesPage, FALSE );
	mode = Browser;
	checkBoxAutoEdit->setChecked( TRUE );
    } else if ( ::tqt_cast<TQDataView*>(widget) ) {
	setCaption( "Data View Wizard" );
	setAppropriate( tablePropertiesPage, FALSE );
	setAppropriate( navigPage, FALSE );
	setAppropriate( sqlPage, FALSE);
	checkCreateFieldLayout->hide();
	checkCreateButtonLayout->hide();
	checkBoxAutoEdit->hide();
	mode = View;
    }

    connect( nextButton(), TQ_SIGNAL(clicked()), TQ_SLOT(nextPageClicked()) );
    setupPage1();
}


SqlFormWizard::~SqlFormWizard()
{
    appIface->release();
}

void SqlFormWizard::nextPageClicked()
{
    if ( currentPage() == populatePage ) {
	autoPopulate( TRUE );
    }
}

void SqlFormWizard::connectionSelected( const TQString &c )
{
    if ( !appIface )
	return;

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;

    listBoxTable->clear();
    TQPtrList<DesignerDatabase> databases = proIface->databaseConnections();
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	if ( d->name() == c  || ( ( d->name() == "(default)" || d->name().isEmpty() ) && c == "(default)") )
	    listBoxTable->insertStringList( d->tables() );
    }
    setNextEnabled( databasePage, ( listBoxTable->currentItem() >= 0 ) );
}

void SqlFormWizard::tableSelected( const TQString & )
{
    if ( listBoxTable->currentItem() >= 0 ) {
	setNextEnabled( databasePage, TRUE );
    } else {
	setNextEnabled( databasePage, FALSE );
    }

}

void SqlFormWizard::autoPopulate( bool populate )
{
    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;
    TQPtrList<DesignerDatabase> databases = proIface->databaseConnections();
    listBoxField->clear();
    listBoxSortField->clear();
    listBoxSelectedField->clear();
    if ( populate ) {
	for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	    if ( d->name() == listBoxConnection->currentText() ||
		 ( ( d->name() == "(default)" || d->name().isEmpty() ) &&
		 listBoxConnection->currentText() == "(default)" ) ) {
		TQStringList lst = *d->fields().find( listBoxTable->currentText() );
		// remove primary index fields, if any
		listBoxSortField->insertStringList( lst );
		d->open( FALSE );
#ifndef TQT_NO_SQL
		TQSqlCursor tab( listBoxTable->currentText(), TRUE, d->connection() );
		TQSqlIndex pIdx = tab.primaryIndex();
		for ( uint i = 0; i < pIdx.count(); i++ ) {
		    listBoxField->insertItem( pIdx.field( i )->name() );
		    lst.remove( pIdx.field( i )->name() );
		}
#endif
		d->close();
		listBoxSelectedField->insertStringList( lst );
	    }
	}
    }
}

void SqlFormWizard::fieldDown()
{
    if ( listBoxSelectedField->currentItem() == -1 ||
	 listBoxSelectedField->currentItem() == (int)listBoxSelectedField->count() - 1 ||
	 listBoxSelectedField->count() < 2 )
	return;
    int index = listBoxSelectedField->currentItem() + 1;
    TQListBoxItem *i = listBoxSelectedField->item( listBoxSelectedField->currentItem() );
    listBoxSelectedField->takeItem( i );
    listBoxSelectedField->insertItem( i, index );
    listBoxSelectedField->setCurrentItem( i );
}

void SqlFormWizard::fieldUp()
{
    if ( listBoxSelectedField->currentItem() <= 0 ||
	 listBoxSelectedField->count() < 2 )
	return;
    int index = listBoxSelectedField->currentItem() - 1;
    TQListBoxItem *i = listBoxSelectedField->item( listBoxSelectedField->currentItem() );
    listBoxSelectedField->takeItem( i );
    listBoxSelectedField->insertItem( i, index );
    listBoxSelectedField->setCurrentItem( i );
}

void SqlFormWizard::removeField()
{
    int i = listBoxSelectedField->currentItem();
    if ( i != -1 ) {
	listBoxField->insertItem( listBoxSelectedField->currentText() );
	listBoxSelectedField->removeItem( i );
    }
}

void SqlFormWizard::addField()
{
    int i = listBoxField->currentItem();
    if ( i == -1 )
	return;
    TQString f = listBoxField->currentText();
    if ( !f.isEmpty() )
	listBoxSelectedField->insertItem( f );
    listBoxField->removeItem( i );
}

void SqlFormWizard::addSortField()
{
    int i = listBoxSortField->currentItem();
    if ( i == -1 )
	return;
    TQString f = listBoxSortField->currentText();
    if ( !f.isEmpty() )
	listBoxSortedField->insertItem( f + " ASC" );
}

void SqlFormWizard::reSortSortField()
{
    int i = listBoxSortedField->currentItem();
    if ( i != -1 ) {
	TQString text = listBoxSortedField->currentText();
	if ( text.mid( text.length() - 3 ) == "ASC" )
	    text = text.mid( 0, text.length() - 3 ) + "DESC";
	else if ( text.mid( text.length() - 4 ) == "DESC" )
	    text = text.mid( 0, text.length() - 4 ) + "ASC";
	listBoxSortedField->removeItem( i );
	listBoxSortedField->insertItem( text, i );
	listBoxSortedField->setCurrentItem( i );
    }
}

void SqlFormWizard::removeSortField()
{
    int i = listBoxSortedField->currentItem();
    if ( i != -1 ) {
	listBoxSortedField->removeItem( i );
    }
}

void SqlFormWizard::sortFieldUp()
{
    if ( listBoxSortedField->currentItem() <= 0 ||
	 listBoxSortedField->count() < 2 )
	return;
    int index = listBoxSortedField->currentItem() - 1;
    TQListBoxItem *i = listBoxSortedField->item( listBoxSortedField->currentItem() );
    listBoxSortedField->takeItem( i );
    listBoxSortedField->insertItem( i, index );
    listBoxSortedField->setCurrentItem( i );
}

void SqlFormWizard::sortFieldDown()
{
    if ( listBoxSortedField->currentItem() == -1 ||
	 listBoxSortedField->currentItem() == (int)listBoxSortedField->count() - 1 ||
	 listBoxSortedField->count() < 2 )
	return;
    int index = listBoxSortedField->currentItem() + 1;
    TQListBoxItem *i = listBoxSortedField->item( listBoxSortedField->currentItem() );
    listBoxSortedField->takeItem( i );
    listBoxSortedField->insertItem( i, index );
    listBoxSortedField->setCurrentItem( i );
}

void SqlFormWizard::setupDatabaseConnections()
{
    if ( !appIface )
	return;

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;
    proIface->setupDatabases();
    raise();
    setupPage1();
}

void SqlFormWizard::setupPage1()
{
    if ( !appIface )
	return;

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;

    listBoxTable->clear();
    listBoxConnection->clear();
    TQPtrList<DesignerDatabase> databases = proIface->databaseConnections();
    TQStringList lst;
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() )
	lst << d->name();
    listBoxConnection->insertStringList( lst );
    if ( lst.count() )
	listBoxConnection->setCurrentItem( 0 );

    setNextEnabled( databasePage, FALSE );
}

static TQPushButton *create_widget( TQWidget *parent, const char *name,
				   const TQString &txt, const TQRect &r, DesignerFormWindow *fw )
{
    TQPushButton *pb = (TQPushButton*)fw->create( "TQPushButton", parent, name );
    pb->setText( txt );
    pb->setGeometry( r );
    fw->setPropertyChanged( pb, "text", TRUE );
    fw->setPropertyChanged( pb, "geometry", TRUE );
    return pb;
}

void SqlFormWizard::accept()
{
    if ( !appIface || mode == None )
	return;

#ifndef TQT_NO_SQL
    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !widget || !proIface ) {
	SqlFormWizardBase::accept();
	return;
    }

    TQString conn = listBoxConnection->currentText();
    TQString table = listBoxTable->currentText();
    TQStringList lst;
    lst << conn << table;

    if ( !conn.isEmpty() && !table.isEmpty() ) {
	formWindow->setProperty( widget, "database", lst );
	formWindow->setPropertyChanged( widget, "database", TRUE );
    }

    if ( !editFilter->text().isEmpty() ) {
	widget->setProperty( "filter", editFilter->text() );
	formWindow->setPropertyChanged( widget, "filter", TRUE );
    }

    if ( listBoxSortedField->count() ) {
	TQStringList lst;
	for ( uint i = 0; i < listBoxSortedField->count(); ++i )
	    lst << listBoxSortedField->text( i );
	widget->setProperty( "sort", lst );
	formWindow->setPropertyChanged( widget, "sort", TRUE );
    }

    TQPtrList<DesignerDatabase> databases = proIface->databaseConnections();
    DesignerDatabase *database = 0;
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	if ( d->name() == listBoxConnection->currentText() || ( ( d->name() == "(default)" || d->name().isEmpty() ) && listBoxConnection->currentText() == "(default)" ) ) {
	    database = d;
	    d->open( FALSE );
	    break;
	}
    }

    if (!database) {
	return;
    }
    TQSqlCursor tab( listBoxTable->currentText(), TRUE, database->connection() );
    int columns = 2;

    TQSqlEditorFactory * f = TQSqlEditorFactory::defaultFactory();

    TQWidget * editorDummy;
    TQWidget * editor;
    TQLabel * label;

    int visibleFields = listBoxSelectedField->count();
    int numPerColumn = visibleFields / columns;
    if( (visibleFields % columns) > 0)
	numPerColumn++;

    int row = 0;
    const int SPACING = 25;
    const int COL_SPACING = SPACING*9;

    uint j;
    switch ( mode ) {
    case None:
	break;
    case View:
    case Browser: {

	if ( mode == Browser && !checkBoxAutoEdit->isChecked() ) {
	    ((TQDataBrowser*)widget)->setAutoEdit( FALSE );
	    formWindow->setPropertyChanged( widget, "autoEdit", TRUE );
	}

	formWindow->clearSelection();
	bool createFieldLayout = checkCreateFieldLayout->isChecked();
	bool createButtonLayout = checkCreateButtonLayout->isChecked();
	bool createLayouts = checkCreateLayouts->isChecked();
	bool labelAbove = radioLabelsTop->isOn();
	uint numCols = spinNumberOfColumns->text().toInt();
	uint currentCol = 0;
	uint fieldsPerCol = listBoxSelectedField->count();
	uint fieldsInCol = 0;
	if ( listBoxSelectedField->count() )
	    fieldsPerCol = listBoxSelectedField->count() / numCols;
	/* labels and data field editors */
	for( j = 0; j < listBoxSelectedField->count(); j++ ){

	    TQSqlField* field = tab.field( listBoxSelectedField->text( j ) );
	    if ( !field )
		continue;

	    /* label */
	    TQString labelName = field->name();
	    labelName = labelName.mid(0,1).upper() + labelName.mid(1);
	    label = (TQLabel*)formWindow->create( "TQLabel", widget,
						 TQString( "label" + labelName ) );
	    label->setText( labelName );
	    label->setGeometry( SPACING + currentCol*COL_SPACING, row+SPACING,
				SPACING*3, SPACING );

	    formWindow->setPropertyChanged( label, "geometry", TRUE );
	    formWindow->setPropertyChanged( label, "text", TRUE );

	    /* editor */
	    editorDummy = f->createEditor( widget, field );
	    editor = formWindow->create( editorDummy->className(), widget,
					 TQString( TQString( editorDummy->className() )
						  + labelName) );
	    delete editorDummy;
	    if ( labelAbove ) {
		row += SPACING;
		editor->setGeometry(SPACING + currentCol*COL_SPACING, row+SPACING,
				    SPACING*3, SPACING );
	    } else {
		editor->setGeometry(SPACING * 5 + currentCol*COL_SPACING, row+SPACING,
				    SPACING*3, SPACING );
	    }
	    formWindow->setPropertyChanged( editor, "geometry", TRUE );
	    if ( TQString(editor->className()) == "TQLineEdit" &&
		 (field->type() == TQVariant::Double ||
		  field->type() == TQVariant::Int ||
		  field->type() == TQVariant::UInt ) ) {
		/* default right-align numerics */
		//##
		((TQLineEdit*)editor)->setAlignment( TQt::AlignRight );
		formWindow->setPropertyChanged( editor, "alignment", TRUE );
	    }
	    if ( ::tqt_cast<TQSpinBox*>(editor) ) {
		( (TQSpinBox*)editor )->setMaxValue( INT_MAX );
		formWindow->setPropertyChanged( editor, "maxValue", TRUE );
	    }
	    TQStringList lst;
	    lst << conn << table << field->name();
	    formWindow->setProperty( editor, "database", lst );
	    formWindow->setPropertyChanged( editor, "database", TRUE );

	    /* geometry */
	    if ( createFieldLayout ) {
		formWindow->selectWidget( label );
		formWindow->selectWidget( editor );
	    }

	    row += SPACING + 5;
	    fieldsInCol++;
	    if ( ( fieldsInCol >= fieldsPerCol ) && ( currentCol < numCols-1 ) ) {
		currentCol++;
		fieldsInCol = 0;
		row = 0;
	    }
	}

	if ( listBoxSelectedField->count() ) {
	    if ( createFieldLayout )
		formWindow->layoutG();
	    row += SPACING;
	}

	if ( mode == Browser ) {
	    if ( checkBoxNavig->isChecked() ) {
		formWindow->clearSelection();
		currentCol = 0;
		if ( checkBoxFirst->isChecked() ) {
		    TQPushButton *pb = create_widget( widget, "PushButtonFirst",
						     "|< &First",
						     TQRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ),
						     formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "first()" );
		    formWindow->addConnection( widget, "firstRecordAvailable( bool )",
					       pb, "setEnabled( bool )" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxPrev->isChecked() ) {
		    TQPushButton *pb = create_widget( widget, "PushButtonPrev",
						     "<< &Prev",
						     TQRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ),
						     formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "prev()" );
		    formWindow->addConnection( widget, "prevRecordAvailable( bool )",
					       pb, "setEnabled( bool )" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxNext->isChecked() ) {
		    TQPushButton *pb = create_widget( widget, "PushButtonNext",
						     "&Next >>",
						     TQRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ),
						     formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "next()" );
		    formWindow->addConnection( widget, "nextRecordAvailable( bool )", pb,
					       "setEnabled( bool )" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxLast->isChecked() ) {
		    TQPushButton *pb = create_widget( widget, "PushButtonLast", "&Last >|",
						     TQRect( 3 * SPACING * currentCol, row+SPACING, SPACING*3, SPACING ), formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "last()" );
		    formWindow->addConnection( widget, "lastRecordAvailable( bool )", pb,
					       "setEnabled( bool )" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( createButtonLayout )
		    formWindow->layoutH();
	    }
	    if ( checkBoxEdit->isChecked() ) {
		formWindow->clearSelection();
		row += SPACING;
		currentCol = 0;
		if ( checkBoxInsert->isChecked() ) {
		    TQPushButton *pb = create_widget( widget, "PushButtonInsert", "&Insert",
						     TQRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ), formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "insert()" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxUpdate->isChecked() ) {
		    TQPushButton *pb = create_widget( widget, "PushButtonUpdate", "&Update",
						     TQRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ), formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "update()" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxDelete->isChecked() ) {
		    TQPushButton *pb = create_widget( widget, "PushButtonDelete", "&Delete",
						     TQRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ), formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "del()" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( createButtonLayout )
		    formWindow->layoutH();
	    }
	}
	if ( createLayouts )
	    formWindow->layoutGContainer( widget );
	formWindow->clearSelection();
	break;
    }
    case Table:
	{
	    TQDataTable* sqlTable = ((TQDataTable*)widget);
	    if ( checkBoxAutoEdit->isChecked() ) {
		sqlTable->setAutoEdit( TRUE );
		formWindow->setPropertyChanged( sqlTable, "autoEdit", TRUE );
	    }

	    if ( checkBoxReadOnly->isChecked() ) {
		sqlTable->setReadOnly( TRUE );
		formWindow->setPropertyChanged( sqlTable, "readOnly", TRUE );
	    } else {
		if ( checkBoxConfirmInserts->isChecked() ) {
		    sqlTable->setConfirmInsert( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmInsert", TRUE );
		}
		if ( checkBoxConfirmUpdates->isChecked() ) {
		    sqlTable->setConfirmUpdate( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmUpdate", TRUE );
		}
		if ( checkBoxConfirmDeletes->isChecked() ) {
		    sqlTable->setConfirmDelete( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmDelete", TRUE );
		}
		if ( checkBoxConfirmCancels->isChecked() ) {
		    sqlTable->setConfirmCancels( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmCancels", TRUE );
		}
	    }
	    if ( checkBoxSorting->isChecked() ) {
		sqlTable->setSorting( TRUE );
		formWindow->setPropertyChanged( sqlTable, "sorting", TRUE );
	    }

	    TQMap<TQString, TQString> columnFields;
	    sqlTable->setNumCols( listBoxSelectedField->count() ); // no need to change property through mdbIface here, since TQDataTable doesn't offer that through Designer
	    for( j = 0; j < listBoxSelectedField->count(); j++ ){

		TQSqlField* field = tab.field( listBoxSelectedField->text( j ) );
		if ( !field )
		    continue;

		TQString labelName = field->name();
		labelName = labelName.mid(0,1).upper() + labelName.mid(1);

		((TQTable*)widget)->horizontalHeader()->setLabel( j, labelName );

		columnFields.insert( labelName, field->name() );
	    }
	    formWindow->setColumnFields( widget, columnFields );
	    break;
	}
    }

    database->close();
#endif

    SqlFormWizardBase::accept();
}
