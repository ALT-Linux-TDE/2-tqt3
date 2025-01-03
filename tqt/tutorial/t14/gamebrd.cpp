/****************************************************************
**
** Implementation of GameBoard class, TQt tutorial 14
**
****************************************************************/

#include "gamebrd.h"

#include <ntqfont.h>
#include <ntqapplication.h>
#include <ntqlabel.h>
#include <ntqaccel.h>
#include <ntqpushbutton.h>
#include <ntqlcdnumber.h>
#include <ntqlayout.h>
#include <ntqvbox.h>

#include "lcdrange.h"
#include "cannon.h"

GameBoard::GameBoard( TQWidget *parent, const char *name )
        : TQWidget( parent, name )
{
    TQPushButton *quit = new TQPushButton( "&Quit", this, "quit" );
    quit->setFont( TQFont( "Times", 18, TQFont::Bold ) );

    connect( quit, SIGNAL(clicked()), tqApp, SLOT(quit()) );

    LCDRange *angle  = new LCDRange( "ANGLE", this, "angle" );
    angle->setRange( 5, 70 );

    LCDRange *force  = new LCDRange( "FORCE", this, "force" );
    force->setRange( 10, 50 );

    TQVBox *box = new TQVBox( this, "cannonFrame" );
    box->setFrameStyle( TQFrame::WinPanel | TQFrame::Sunken );

    cannonField = new CannonField( box, "cannonField" );

    connect( angle, SIGNAL(valueChanged(int)),
	     cannonField, SLOT(setAngle(int)) );
    connect( cannonField, SIGNAL(angleChanged(int)),
	     angle, SLOT(setValue(int)) );

    connect( force, SIGNAL(valueChanged(int)),
	     cannonField, SLOT(setForce(int)) );
    connect( cannonField, SIGNAL(forceChanged(int)),
	     force, SLOT(setValue(int)) );

    connect( cannonField, SIGNAL(hit()),
	     this, SLOT(hit()) );
    connect( cannonField, SIGNAL(missed()),
	     this, SLOT(missed()) );

    TQPushButton *shoot = new TQPushButton( "&Shoot", this, "shoot" );
    shoot->setFont( TQFont( "Times", 18, TQFont::Bold ) );

    connect( shoot, SIGNAL(clicked()), SLOT(fire()) );

    connect( cannonField, SIGNAL(canShoot(bool)),
             shoot, SLOT(setEnabled(bool)) );

    TQPushButton *restart
	= new TQPushButton( "&New Game", this, "newgame" );
    restart->setFont( TQFont( "Times", 18, TQFont::Bold ) );

    connect( restart, SIGNAL(clicked()), this, SLOT(newGame()) );

    hits = new TQLCDNumber( 2, this, "hits" );
    shotsLeft = new TQLCDNumber( 2, this, "shotsleft" );
    TQLabel *hitsL = new TQLabel( "HITS", this, "hitsLabel" );
    TQLabel *shotsLeftL
	= new TQLabel( "SHOTS LEFT", this, "shotsleftLabel" );

    TQAccel *accel = new TQAccel( this );
    accel->connectItem( accel->insertItem( Key_Enter ),
			this, SLOT(fire()) );
    accel->connectItem( accel->insertItem( Key_Return ),
			this, SLOT(fire()) );
    accel->connectItem( accel->insertItem( CTRL+Key_Q ),
			tqApp, SLOT(quit()) );

    TQGridLayout *grid = new TQGridLayout( this, 2, 2, 10 );
    grid->addWidget( quit, 0, 0 );
    grid->addWidget( box, 1, 1 );
    grid->setColStretch( 1, 10 );

    TQVBoxLayout *leftBox = new TQVBoxLayout;
    grid->addLayout( leftBox, 1, 0 );
    leftBox->addWidget( angle );
    leftBox->addWidget( force );

    TQHBoxLayout *topBox = new TQHBoxLayout;
    grid->addLayout( topBox, 0, 1 );
    topBox->addWidget( shoot );
    topBox->addWidget( hits );
    topBox->addWidget( hitsL );
    topBox->addWidget( shotsLeft );
    topBox->addWidget( shotsLeftL );
    topBox->addStretch( 1 );
    topBox->addWidget( restart );

    angle->setValue( 60 );
    force->setValue( 25 );
    angle->setFocus();

    newGame();
}


void GameBoard::fire()
{
    if ( cannonField->gameOver() || cannonField->isShooting() )
	return;
    shotsLeft->display( shotsLeft->intValue() - 1 );
    cannonField->shoot();
}


void GameBoard::hit()
{
    hits->display( hits->intValue() + 1 );
    if ( shotsLeft->intValue() == 0 )
	cannonField->setGameOver();
    else
	cannonField->newTarget();
}


void GameBoard::missed()
{
    if ( shotsLeft->intValue() == 0 )
	cannonField->setGameOver();
}


void GameBoard::newGame()
{
    shotsLeft->display( 15 );
    hits->display( 0 );
    cannonField->restartGame();
    cannonField->newTarget();
}
