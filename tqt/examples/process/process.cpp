/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <tqobject.h>
#include <tqprocess.h>
#include <tqvbox.h>
#include <tqtextview.h>
#include <tqpushbutton.h>
#include <tqapplication.h>
#include <tqmessagebox.h>

#include <stdlib.h>

class UicManager : public TQVBox
{
    TQ_OBJECT

public:
    UicManager();
    ~UicManager() {}

public slots:
    void readFromStdout();
    void scrollToTop();

private:
    TQProcess *proc;
    TQTextView *output;
    TQPushButton *quitButton;
};

UicManager::UicManager()
{
    // Layout
    output = new TQTextView( this );
    quitButton = new TQPushButton( tr("Quit"), this );
    connect( quitButton, TQ_SIGNAL(clicked()),
	    tqApp, TQ_SLOT(quit()) );
    resize( 500, 500 );

    // TQProcess related code
    proc = new TQProcess( this );

    // Set up the command and arguments.
    // On the command line you would do:
    //   tquic -tr i18n "small_dialog.ui"
    proc->addArgument( "tquic" );
    proc->addArgument( "-tr" );
    proc->addArgument( "i18n" );
    proc->addArgument( "small_dialog.ui" );

    connect( proc, TQ_SIGNAL(readyReadStdout()),
	    this, TQ_SLOT(readFromStdout()) );
    connect( proc, TQ_SIGNAL(processExited()),
	    this, TQ_SLOT(scrollToTop()) );

    if ( !proc->start() ) {
	// error handling
	TQMessageBox::critical( 0,
		tr("Fatal error"),
		tr("Could not start the tquic command."),
		tr("Quit") );
	exit( -1 );
    }
}

void UicManager::readFromStdout()
{
    // Read and process the data.
    // Bear in mind that the data might be output in chunks.
    output->append( proc->readStdout() );
}

void UicManager::scrollToTop()
{
    output->setContentsPos( 0, 0 );
}

int main( int argc, char **argv )
{
    TQApplication a( argc, argv );
    UicManager manager;
    a.setMainWidget( &manager );
    manager.show();
    return a.exec();
}

#include "process.moc"
