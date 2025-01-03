#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrlist.h>
#include <tqiconview.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqfiledialog.h>
#include <tqtabwidget.h>
#include <tqpoint.h>
#include <tqobjectlist.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqfileinfo.h>
#include <tqmap.h>
#include <tqdatetime.h>
#include <tqapplication.h>
#include <tqcheckbox.h>
#include <tqurl.h>

#include "mainwindow.h"
#include "startdialogimpl.h"
#include "designerapp.h"

FileDialog::FileDialog( const TQString &dir, TQWidget *parent )
    : TQFileDialog( dir, TQString::null, parent )
{
}

void FileDialog::accept()
{
    emit fileSelected();
}


StartDialog::StartDialog( TQWidget *parent, const TQString &templatePath )
    : StartDialogBase( parent, 0 )
{
    newForm = new NewForm( templateView, templatePath );
    recentFiles.clear();
    initFileOpen();
    showInFuture = TRUE;

    connect( buttonHelp, TQ_SIGNAL( clicked() ),
	     MainWindow::self, TQ_SLOT( showDialogHelp() ) );
    connect( recentView, TQ_SIGNAL( doubleClicked(TQIconViewItem*) ),
	     this, TQ_SLOT( accept() ) );
    connect( recentView, TQ_SIGNAL( returnPressed(TQIconViewItem*) ),
	     this, TQ_SLOT( accept() ) );
    connect( fd, TQ_SIGNAL( fileSelected() ), this, TQ_SLOT( accept() ) );
}

void StartDialog::accept()
{
    hide();
    showInFuture = !checkShowInFuture->isChecked(); // means don't show it anymore

    int tabindex = tabWidget->currentPageIndex();
    TQString filename;
    if( !tabindex ) {
	if ( !templateView->currentItem() )
	    return;
	Project *pro = MainWindow::self->findProject( tr( "<No Project>" ) );
	if ( !pro )
	    return;
	MainWindow::self->setCurrentProject( pro );
	( (NewItem*)templateView->currentItem() )->insert( pro );
    } else if ( tabindex == 1 ) {
	filename = fd->selectedFile();
    } else if ( tabindex == 2 ) {
	filename = recentFiles[recentView->currentItem()->index()];
    }
    if ( tabindex ) {
	if ( !filename.isEmpty() ) {
	    TQFileInfo fi( filename );
	    if ( fi.extension() == "pro" )
		MainWindow::self->openProject( filename );
	    else
		MainWindow::self->fileOpen( "", "", filename );
	}
    }

    done( Accepted );
}

void StartDialog::reject()
{
    done( Rejected );
}

void StartDialog::recentItemChanged( TQIconViewItem *item )
{
    TQString msg( recentFiles[item->index()] );
    TQFileInfo f( recentFiles[item->index()] );
    uint s = f.size();
    TQString unit( "B" );
    if( s > 1024 ) {
	s /= 1024;
	unit = "KB";
    }
    if ( s > 1024 ) {
	s /= 1024;
	unit = "MB";
    }
    TQDateTime dt( f.lastModified() );
    TQString date( dt.toString( "MMMM dd hh:mm" ));
    msg = TQString( "%1 (%2 %3)  %4" ).arg(msg).arg(s).arg(unit).arg(date);
    fileInfoLabel->setText( msg );
}

void StartDialog::clearFileInfo()
{
    fileInfoLabel->clear();
}

void StartDialog::setRecentlyFiles( TQStringList &files )
{
    insertRecentItems( files, FALSE );
}

void StartDialog::setRecentlyProjects( TQStringList &projects )
{
    insertRecentItems( projects, TRUE );
}

void StartDialog::insertRecentItems( TQStringList &files, bool isProject )
{
    TQString iconName = "designer_newform.png";
    if ( isProject )
	iconName = "designer_project.png";
    TQIconViewItem *item;
    TQStringList::iterator it = files.begin();
    for( ; it != files.end(); ++it ) {
	TQFileInfo fi( *it );
	item = new TQIconViewItem( recentView, fi.fileName() );
	recentFiles[recentView->index( item )] = *it;
	item->setPixmap( TQPixmap::fromMimeSource( iconName ) );
	item->setDragEnabled( FALSE );
    }
}

void StartDialog::initFileOpen()
{
    TQString encode = TQDir::currentDirPath();
    TQUrl::encode( encode );
    fd = new FileDialog( encode, this );
    TQPoint point( 0, 0 );
    fd->reparent( tab, point );

    TQObjectList *l = fd->queryList( "TQPushButton" );
    TQObjectListIt it( *l );
    TQObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
	((TQPushButton*)obj)->hide();
    }
    delete l;

    fd->setSizeGripEnabled ( FALSE );
    tabLayout->addWidget( fd );

    TQPluginManager<ImportFilterInterface> manager( IID_ImportFilter, TQApplication::libraryPaths(),
						   MainWindow::self->pluginDirectory() );
    TQStringList filterlist;
    filterlist << tr( "Designer Files (*.ui *.pro)" );
    filterlist << tr( "TQt User-Interface Files (*.ui)" );
    filterlist << tr( "QMAKE Project Files (*.pro)" );

    TQStringList list = manager.featureList();
    for ( TQStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 )
	filterlist << *it2;

    filterlist << tr( "All Files (*)" );
    TQString filters = filterlist.join( ";;" );
    fd->setFilters( filters );

    fd->show();
}
