/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <ntqtextview.h>
#include <ntqpushbutton.h>
#include <ntqtextstream.h>
#include <ntqapplication.h>
#include <ntqmessagebox.h>
#include <stdlib.h>

#include "server.h"



ServerInfo::ServerInfo( TQ_UINT16 port, TQWidget *parent, const char *name ) :
    ServerInfoBase( parent, name )
{
    SimpleServer *server = new SimpleServer( port, this, "simple server" );
    connect( server, SIGNAL(newConnect()), SLOT(newConnect()) );
    connect( btnQuit, SIGNAL(clicked()), tqApp, SLOT(quit()) );
}

void ServerInfo::newConnect()
{
    infoText->append( tr( "New connection\n" ) );
}


SimpleServer::SimpleServer( TQ_UINT16 port, TQObject* parent, const char *name ) :
    TQServerSocket( port, 1, parent, name )
{
    if ( !ok() ) {
	TQMessageBox::critical( 0, tr( "Error" ), tr( "Failed to bind to port %1" ).arg( port ) );
	exit(1);
    }
}

void SimpleServer::newConnection( int socket )
{
    (void)new ClientSocket( socket, &info, this, "client socket" );
    emit newConnect();
}


ClientSocket::ClientSocket( int sock, InfoData *i, TQObject *parent, const char *name ) :
    TQSocket( parent, name ), info( i )
{
    connect( this, SIGNAL(readyRead()), SLOT(readClient()) );
    connect( this, SIGNAL(connectionClosed()), SLOT(connectionClosed()) );
    setSocket( sock );
}

void ClientSocket::readClient()
{
    TQTextStream stream( this );
    TQStringList answer;
    while ( canReadLine() ) {
	stream << processCommand( stream.readLine() );
    }
}

TQString ClientSocket::processCommand( const TQString& command )
{
    TQString answer;
    TQString com = command.simplifyWhiteSpace ();
    if ( com.startsWith( "LIST" ) ) {
	bool ok;
	TQStringList nodes = info->list( com.mid( 5 ), &ok );
	if ( ok ) {
	    for ( TQStringList::Iterator it = nodes.begin(); it != nodes.end(); ++it ) 
		answer += "212+" + *it + "\r\n";
	    answer += "212 \r\n";
	} else 
	    answer += "550 Invalid path\r\n";	
    } else if ( com.startsWith( "GET " ) ) {
	bool ok;
	TQStringList data = TQStringList::split( '\n', info->get( com.mid( 4 ), &ok ), TRUE );
	if ( ok ) {
	    for ( TQStringList::Iterator it = data.begin(); it != data.end(); ++it ) 
		answer += "213+" + *it + "\r\n";
	    answer += "213 \r\n";
	} else 
	    answer += "550 Info not found\r\n";	
    } else 
	answer += "500 Syntax error\r\n";	

    return answer;
}

void ClientSocket::connectionClosed()
{
    delete this;
}
