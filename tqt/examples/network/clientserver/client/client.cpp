/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <ntqsocket.h>
#include <ntqapplication.h>
#include <ntqvbox.h>
#include <ntqhbox.h>
#include <ntqtextview.h>
#include <ntqlineedit.h>
#include <ntqlabel.h>
#include <ntqpushbutton.h>
#include <ntqtextstream.h>


class Client : public TQVBox
{
    TQ_OBJECT
public:
    Client( const TQString &host, TQ_UINT16 port )
    {
	// GUI layout
	infoText = new TQTextView( this );
	TQHBox *hb = new TQHBox( this );
	inputText = new TQLineEdit( hb );
	TQPushButton *send = new TQPushButton( tr("Send") , hb );
	TQPushButton *close = new TQPushButton( tr("Close connection") , this );
	TQPushButton *quit = new TQPushButton( tr("Quit") , this );

	connect( send, SIGNAL(clicked()), SLOT(sendToServer()) );
	connect( close, SIGNAL(clicked()), SLOT(closeConnection()) );
	connect( quit, SIGNAL(clicked()), tqApp, SLOT(quit()) );

	// create the socket and connect various of its signals
	socket = new TQSocket( this );
	connect( socket, SIGNAL(connected()),
		SLOT(socketConnected()) );
	connect( socket, SIGNAL(connectionClosed()),
		SLOT(socketConnectionClosed()) );
	connect( socket, SIGNAL(readyRead()),
		SLOT(socketReadyRead()) );
	connect( socket, SIGNAL(error(int)),
		SLOT(socketError(int)) );

	// connect to the server
	infoText->append( tr("Trying to connect to the server\n") );
	socket->connectToHost( host, port );
    }

    ~Client()
    {
    }

private slots:
    void closeConnection()
    {
	socket->close();
	if ( socket->state() == TQSocket::Closing ) {
	    // We have a delayed close.
	    connect( socket, SIGNAL(delayedCloseFinished()),
		    SLOT(socketClosed()) );
	} else {
	    // The socket is closed.
	    socketClosed();
	}
    }

    void sendToServer()
    {
	// write to the server
	TQTextStream os(socket);
	os << inputText->text() << "\n";
	inputText->setText( "" );
    }

    void socketReadyRead()
    {
	// read from the server
	while ( socket->canReadLine() ) {
	    infoText->append( socket->readLine() );
	}
    }

    void socketConnected()
    {
	infoText->append( tr("Connected to server\n") );
    }

    void socketConnectionClosed()
    {
	infoText->append( tr("Connection closed by the server\n") );
    }

    void socketClosed()
    {
	infoText->append( tr("Connection closed\n") );
    }

    void socketError( int e )
    {
	infoText->append( tr("Error number %1 occurred\n").arg(e) );
    }

private:
    TQSocket *socket;
    TQTextView *infoText;
    TQLineEdit *inputText;
};


int main( int argc, char** argv )
{
    TQApplication app( argc, argv );
    Client client( argc<2 ? "localhost" : argv[1], 4242 );
    app.setMainWidget( &client );
    client.show();
    return app.exec();
}

#include "client.moc"
