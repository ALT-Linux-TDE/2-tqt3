/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <stdlib.h>
#include <ntqsocket.h>
#include <ntqregexp.h>
#include <ntqserversocket.h>
#include <ntqapplication.h>
#include <ntqmainwindow.h>
#include <ntqtextstream.h>
#include <ntqvbox.h>
#include <ntqlabel.h>
#include <ntqtextview.h>
#include <ntqpushbutton.h>

// HttpDaemon is the the class that implements the simple HTTP server.
class HttpDaemon : public TQServerSocket
{
    TQ_OBJECT
public:
    HttpDaemon( TQObject* parent=0 ) :
	TQServerSocket(8080,1,parent)
    {
	if ( !ok() ) {
	    tqWarning("Failed to bind to port 8080");
	    exit( 1 );
	}
    }

    void newConnection( int socket )
    {
	// When a new client connects, the server constructs a TQSocket and all
	// communication with the client is done over this TQSocket. TQSocket
	// works asynchronouslyl, this means that all the communication is done
	// in the two slots readClient() and discardClient().
	TQSocket* s = new TQSocket( this );
	connect( s, SIGNAL(readyRead()), this, SLOT(readClient()) );
	connect( s, SIGNAL(delayedCloseFinished()), this, SLOT(discardClient()) );
	s->setSocket( socket );
	emit newConnect();
    }

signals:
    void newConnect();
    void endConnect();
    void wroteToClient();

private slots:
    void readClient()
    {
	// This slot is called when the client sent data to the server. The
	// server looks if it was a get request and sends a very simple HTML
	// document back.
	TQSocket* socket = (TQSocket*)sender();
	if ( socket->canReadLine() ) {
	    TQStringList tokens = TQStringList::split( TQRegExp("[ \r\n][ \r\n]*"), socket->readLine() );
	    if ( tokens[0] == "GET" ) {
		TQTextStream os( socket );
		os.setEncoding( TQTextStream::UnicodeUTF8 );
		os << "HTTP/1.0 200 Ok\r\n"
		    "Content-Type: text/html; charset=\"utf-8\"\r\n"
		    "\r\n"
		    "<h1>Nothing to see here</h1>\n";
		socket->close();
		emit wroteToClient();
	    }
	}
    }
    void discardClient()
    {
	TQSocket* socket = (TQSocket*)sender();
	delete socket;
	emit endConnect();
    }
};


// HttpInfo provides a simple graphical user interface to the server and shows
// the actions of the server.
class HttpInfo : public TQVBox
{
    TQ_OBJECT
public:
    HttpInfo()
    {
	HttpDaemon *httpd = new HttpDaemon( this );

	TQString itext = TQString(
		"This is a small httpd example.\n"
		"You can connect with your\n"
		"web browser to port %1"
	    ).arg( httpd->port() );
	TQLabel *lb = new TQLabel( itext, this );
	lb->setAlignment( AlignHCenter );
	infoText = new TQTextView( this );
	TQPushButton *quit = new TQPushButton( "quit" , this );

	connect( httpd, SIGNAL(newConnect()), SLOT(newConnect()) );
	connect( httpd, SIGNAL(endConnect()), SLOT(endConnect()) );
	connect( httpd, SIGNAL(wroteToClient()), SLOT(wroteToClient()) );
	connect( quit, SIGNAL(pressed()), tqApp, SLOT(quit()) );
    }

    ~HttpInfo()
    {
    }

private slots:
    void newConnect()
    {
	infoText->append( "New connection" );
    }
    void endConnect()
    {
	infoText->append( "Connection closed\n\n" );
    }
    void wroteToClient()
    {
	infoText->append( "Wrote to client" );
    }

private:
    TQTextView *infoText;
};


int main( int argc, char** argv )
{
    TQApplication app( argc, argv );
    HttpInfo info;
    app.setMainWidget( &info );
    info.show();
    return app.exec();
}

#include "httpd.moc"
