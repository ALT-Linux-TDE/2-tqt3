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
    if(TQObjectList *chldn = groupBox->queryList()) {
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
    if(TQObjectList *chldn = groupBox->queryList()) {
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
	colorTest->setPalette( TQColor( color ) );
}

void WidgetsBase::updateDateTimeString()
{
	TQDateTime dt;
    	dt.setDate( dateEdit->date() );
    	dt.setTime( timeEdit->time() );
    	dateTimeLabel->setText( dt.toString() );
}

