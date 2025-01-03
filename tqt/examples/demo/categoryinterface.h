#ifndef CATEGORYINTERFACE_H
#define CATEGORYINTERFACE_H

#include <tqstring.h>
#include <tqiconset.h>
#include <tqobject.h>

class TQWidgetStack;

class CategoryInterface : public TQObject
{
    TQ_OBJECT

public:
    CategoryInterface( TQWidgetStack *s ) : stack( s ) {}
    virtual ~CategoryInterface() {}
    virtual TQString name() const = 0;
    virtual TQIconSet icon() const = 0;
    virtual int numCategories() const = 0;
    virtual TQString categoryName( int i ) const = 0;
    virtual TQIconSet categoryIcon( int i ) const = 0;
    virtual int categoryOffset() const = 0;

public slots:
    virtual void setCurrentCategory( int i ) = 0;

protected:
    TQWidgetStack *stack;

};

#endif
