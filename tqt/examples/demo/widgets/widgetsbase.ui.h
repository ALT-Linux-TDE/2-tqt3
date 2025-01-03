#include <ntqobjectlist.h>

void WidgetsBase::init()
{
    timeEdit->setTime( TQTime::currentTime() );
    dateEdit->setDate( TQDate::currentDate() );    
}

void WidgetsBase::destroy()
{

}

void WidgetsBase::resetColors()
{
    groupBox->setPalette( palette(), FALSE );
    TQObjectList *chldn = groupBox->queryList();
    if ( chldn ) {
	for(TQObject *obj=chldn->first(); obj; obj = chldn->next()) {
	    if(obj->isWidgetType()) {
		TQWidget *w = (TQWidget *)obj;
		if(!w->isTopLevel())
		    w->setPalette(palette(), FALSE);
	    }
	}
    }
}

void WidgetsBase::setColor( const TQString & color )
{
    groupBox->setPalette( TQColor( color ), FALSE );
    TQObjectList *chldn = groupBox->queryList();
    if ( chldn ) {
	for(TQObject *obj=chldn->first(); obj; obj = chldn->next()) {
	    if(obj->isWidgetType()) {
		TQWidget *w = (TQWidget *)obj;
		if(!w->isTopLevel())
		    w->setPalette(TQColor(color), FALSE);
	    }
	}
    }
}

void WidgetsBase::setColor()
{
    setColor( lineEdit->text() );
}

void WidgetsBase::updateClock()
{
    clock->setTime( timeEdit->time() );
}

void WidgetsBase::updateColorTest( const TQString & color )
{
    colorTest->setPalette( TQColor( color ), TRUE);
}

void WidgetsBase::updateDateTimeString()
{
    TQDateTime dt;
    dt.setDate( dateEdit->date() );
    dt.setTime( timeEdit->time() );
    dateTimeLabel->setText( dt.toString() );
}

