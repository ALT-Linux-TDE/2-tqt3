/****************************************************************
**
** Implementation of LCDRange class, TQt tutorial 12
**
****************************************************************/

#include "lcdrange.h"

#include <ntqslider.h>
#include <ntqlcdnumber.h>
#include <ntqlabel.h>


LCDRange::LCDRange( TQWidget *parent, const char *name )
        : TQVBox( parent, name )
{
    init();
}


LCDRange::LCDRange( const char *s, TQWidget *parent,
		    const char *name )
        : TQVBox( parent, name )
{
    init();
    setText( s );
}


void LCDRange::init()
{
    TQLCDNumber *lcd  = new TQLCDNumber( 2, this, "lcd"  );
    slider = new TQSlider( Horizontal, this, "slider" );
    slider->setRange( 0, 99 );
    slider->setValue( 0 );

    label = new TQLabel( " ", this, "label"  );
    label->setAlignment( AlignCenter );

    connect( slider, SIGNAL(valueChanged(int)),
	     lcd, SLOT(display(int)) );
    connect( slider, SIGNAL(valueChanged(int)),
	     SIGNAL(valueChanged(int)) );

    setFocusProxy( slider );
}


int LCDRange::value() const
{
    return slider->value();
}


const char *LCDRange::text() const
{
    return label->text();
}


void LCDRange::setValue( int value )
{
    slider->setValue( value );
}


void LCDRange::setRange( int minVal, int maxVal )
{
    if ( minVal < 0 || maxVal > 99 || minVal > maxVal ) {
	tqWarning( "LCDRange::setRange(%d,%d)\n"
		  "\tRange must be 0..99\n"
		  "\tand minVal must not be greater than maxVal",
		  minVal, maxVal );
	return;
    }
    slider->setRange( minVal, maxVal );
}


void LCDRange::setText( const char *s )
{
    label->setText( s );
}
