/*
*/

#ifndef STRUCTUREPARSER_H
#define STRUCTUREPARSER_H

#include <tqxml.h>
#include <tqptrstack.h>

class TQListView;
class TQListViewItem;
class TQString;

class StructureParser: public TQXmlDefaultHandler
{
public:
    StructureParser( TQListView * );
    bool startElement( const TQString&, const TQString&, const TQString& ,
                       const TQXmlAttributes& );
    bool endElement( const TQString&, const TQString&, const TQString& );

    void setListView( TQListView * );

private:
    TQPtrStack<TQListViewItem> stack;
    TQListView * table;
};

#endif
