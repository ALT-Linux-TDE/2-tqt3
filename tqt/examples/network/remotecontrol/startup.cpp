#include "startup.h"
#include "remotectrlimpl.h"
#include "maindialog.h"
#include "ipcserver.h"

#include <ntqsocket.h>
#include <ntqlabel.h>

static const TQ_UINT16 ipcPort = 54923;

StartUp::StartUp()
{
    remoteCtrl = 0;
    mainDialog = 0;

    socket = new TQSocket( this );
    connect( socket, SIGNAL(connected()), SLOT(startRemoteCtrl()) );
    connect( socket, SIGNAL(error(int)), SLOT(startMainDialog()) );
    socket->connectToHost( "localhost", ipcPort );
}

StartUp::~StartUp()
{
    delete remoteCtrl;
    delete mainDialog;
}

void StartUp::startRemoteCtrl()
{
    remoteCtrl = new RemoteCtrlImpl( socket );
    remoteCtrl->show();
}

void StartUp::startMainDialog()
{
    mainDialog = new MainDialog();
    mainDialog->show();

    IpcServer *server = new IpcServer( ipcPort, this );

    connect( server, SIGNAL(receivedText(const TQString&)),
	    mainDialog->description, SLOT(setText(const TQString&)) );
    connect( server, SIGNAL(receivedPixmap(const TQPixmap&)),
	    mainDialog->image, SLOT(setPixmap(const TQPixmap&)) );
}
