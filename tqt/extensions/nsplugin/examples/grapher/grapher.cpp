// Include TQt Netscape Plugin classes.
#include "ntqnp.h"

// Include other TQt classes.
#include <ntqpainter.h>
#include <ntqtextstream.h>
#include <ntqbuffer.h>
#include <ntqpixmap.h>
#include <ntqmenubar.h>
#include <ntqpushbutton.h>
#include <ntqptrlist.h>
#include <ntqmessagebox.h>

// Include some C library functions.
#include <math.h>
#include <stdlib.h>

#ifndef M_PI // Some math.h don't include this.
#define M_PI 3.14159265358979323846264338327950288
#endif



//
// GraphModel is a simple abstract class that describes
// a table of numeric and text data.
//

class GraphModel {
public:
    enum ColType { Numeric, Label };

    union Datum {
	double dbl;
	TQString* str;
    };

    virtual TQPtrList<Datum>& graphData()=0;
    virtual ColType colType(int col) const=0;
    virtual int nCols() const=0;
};


//
// Graph is a widget subclass that displays a GraphModel.
// Since the widget is a TQNPWidget, it can be used as a plugin window,
// returned by Grapher::newWindow() below.
//

class Graph : public TQNPWidget {
    TQ_OBJECT
public:
    // Constructs a Graph to display a GraphModel
    //
    Graph(GraphModel&);
    ~Graph();

    // Two styles are available - Pie and Bar graph
    //
    enum Style { Pie, Bar };
    static const char* styleName[];
    void setStyle(Style);
    void setStyle(const char*);

    // Timer event processing rotates the pie graph
    //
    void timerEvent(TQTimerEvent*);

    // These functions are provided by TQNPWidget - we override
    // them to hide and show the plugin menubar.
    //
    void enterInstance();
    void leaveInstance();

    // Paint the graph...
    //
    void paintEvent(TQPaintEvent*);
    //
    // ... as either a "Loading" message, a Bar graph, a Pie graph,
    // or an error message.
    //
    void paintWait(TQPaintEvent*);
    void paintBar(TQPaintEvent*);
    void paintPie(TQPaintEvent*);
    void paintError(const char*);

signals:
    // Signals emitted when the Help menus are selected.
    void aboutPlugin();
    void aboutData();

private:
    GraphModel& model;
    TQMenuBar *menubar;
    Style style;
    TQPopupMenu* stylemenu;
    int pieRotationTimer;
    int pieRotation;
    TQPixmap pm;

private slots:
    void setStyleFromMenu(int id);
};


Graph::Graph( GraphModel& mdl ) :
    model(mdl),
    style(Bar),
    pieRotationTimer(0),
    pieRotation(0)
{
    // Create a menubar for the widget
    //
    menubar = new TQMenuBar( this );
    stylemenu = new TQPopupMenu;
    stylemenu->setCheckable(TRUE);
    for ( Style s = Pie; styleName[s]; s = Style(s+1)) {
	stylemenu->insertItem(styleName[s], s+100);
    }
    connect(stylemenu, TQ_SIGNAL(activated(int)),
	this, TQ_SLOT(setStyleFromMenu(int)));
    setStyle(Pie);

    menubar->insertItem("Style", stylemenu);
    menubar->insertSeparator();

    TQPopupMenu* help = new TQPopupMenu;
    help->insertItem( "About plugin...", this, TQ_SIGNAL(aboutPlugin()) );
    help->insertItem( "About data...", this, TQ_SIGNAL(aboutData()) );
    menubar->insertItem("Help", help);
    menubar->hide();
}

Graph::~Graph()
{
}

void Graph::setStyle(Style s)
{
    if (style != s) {
	if (pieRotationTimer)
	    killTimer(pieRotationTimer);
	stylemenu->setItemChecked(100+style, FALSE);
	style = s;
	if ( style == Pie )
	    pieRotationTimer = startTimer( 80 );
	else
	    pieRotationTimer = 0;
	stylemenu->setItemChecked(100+style, TRUE);
	update();
    }
}

void Graph::timerEvent(TQTimerEvent*)
{
    pieRotation = ( pieRotation + 6 ) % 360; repaint(FALSE);
}

void Graph::setStyle(const char* stext)
{
    for ( Style s = Pie; styleName[s]; s = Style(s+1) ) {
	if ( tqstricmp(stext,styleName[s])==0 ) {
	    setStyle(s);
	    return;
	}
    }
}

void Graph::enterInstance()
{
    menubar->show();
}

void Graph::leaveInstance()
{
    menubar->hide();
}

void Graph::paintError(const char* e)
{
    TQPainter p(this);
    int w = width();
    p.drawText(w/8, 0, w-w/4, height(), AlignCenter|WordBreak, e);
}

void Graph::paintBar(TQPaintEvent* event)
{
    if ( model.colType(0) != GraphModel::Numeric ) {
	paintError("First column not numeric, cannot draw bar graph\n");
	return;
    }

    TQPtrList<GraphModel::Datum>& data = model.graphData();

    double max = 0.0;

    for (GraphModel::Datum* rowdata = data.first();
	rowdata; rowdata = data.next())
    {
	if (rowdata[0].dbl > max) max = rowdata[0].dbl;
    }

    const uint w = width();
    const uint h = height();

    TQPainter p(this);

    p.setClipRect(event->rect());

    if ( w > data.count() ) {
	// More pixels than data
	int x = 0;
	int i = 0;
	TQFontMetrics fm=fontMetrics();
	int fh = fm.height();

	for (GraphModel::Datum* rowdata = data.first();
	    rowdata; rowdata = data.next())
	{
	    TQColor c;
	    c.setHsv( (i * 255)/data.count(), 255, 255 );// rainbow effect
	    p.setBrush(c);
	    int bw = (w-w/4-x)/(data.count()-i);
	    int bh = int((h-h/4-1)*rowdata[0].dbl/max);
	    p.drawRect( w/8+x, h-h/8-1-bh, bw, bh );

	    i++;
	    x+=bw;
	}
    } else {
	// More data than pixels
	int x = 0;
	int i = 0;
	double av = 0.0;
	int n = 0;
	for (GraphModel::Datum* rowdata = data.first(); rowdata;
	    rowdata = data.next())
	{
	    int bx = i*w/data.count();

	    if (bx > x) {
		TQColor c;
		c.setHsv( (x * 255)/w, 255, 255 );// rainbow effect
		p.setPen(c);
		int bh = int(h*av/n/max);

		p.drawLine(x,h-1,x,h-bh);

		av = 0.0;
		n = 0;
		x = bx;
	    }

	    av += rowdata[0].dbl;
	    n++;

	    i++;
	}
    }
}

void Graph::paintPie(TQPaintEvent* event)
{
    if ( model.colType(0) != GraphModel::Numeric ) {
	paintError("First column not numeric, cannot draw pie graph\n");
	return;
    }

    TQPtrList<GraphModel::Datum>& data = model.graphData();

    double total = 0.0;

    GraphModel::Datum* rowdata;

    for (rowdata = data.first();
	rowdata; rowdata = data.next())
    {
	total += rowdata[0].dbl;
    }

    // Only use first column for pie chart
    if ( !total ) return;

    int apos = (pieRotation-90)*16;

    const int w = width();
    const int h = height();

    const int xd = w - w/5;
    const int yd = h - h/5;

    pm.resize(width(),height());
    pm.fill(backgroundColor());
    TQPainter p(&pm);
    p.setFont(font());

    p.setClipRect(event->rect());

    int i = 0;

    for (rowdata = data.first();
	rowdata; rowdata = data.next())
    {
	TQColor c;

	c.setHsv( ( i * 255)/data.count(), 255, 255 );// rainbow effect
	p.setBrush( c );			// solid fill with color c

	int a = int(( rowdata[0].dbl * 360.0 ) / total * 16.0 + 0.5);
	p.drawPie( w/10, h/10, xd, yd, -apos, -a );
	apos += a;
	i++;
    }

    if (model.colType(1) == GraphModel::Label) {
	double apos = (pieRotation-90)*M_PI/180;

	for (rowdata = data.first();
	    rowdata; rowdata = data.next())
	{
	    double a = rowdata[0].dbl * 360 / total * M_PI / 180;
	    int x = int(cos(apos+a/2)*w*5/16 + w/2 + 0.5);
	    int y = int(sin(apos+a/2)*h*5/16 + h/2 + 0.5);

	    // ### This causes a crash, so comment out for now
	    /*p.drawText(x-w/8, y-h/8, w/4, h/4,
		WordBreak|AlignCenter,
		*rowdata[1].str);*/
	    apos += a;
	}
    }

    TQPainter p2(this);
    p2.setClipRect(event->rect());
    p2.drawPixmap(0,0,pm);
}

void Graph::paintWait(TQPaintEvent*)
{
    TQPainter p(this);
    p.drawText(rect(), AlignCenter, "Loading...");
}

void Graph::paintEvent(TQPaintEvent* event)
{
    if (!model.nCols()) {
	paintWait(event);
    } else {
	switch (style) {
	  case Pie:
	    paintPie(event);
	    break;
	  case Bar:
	    paintBar(event);
	    break;
	}
    }
}

void Graph::setStyleFromMenu(int id)
{
    setStyle(Style(id-100));
}

const char* Graph::styleName[] = { "Pie", "Bar", 0 };


//
// Grapher is a subclass of TQNPInstance, and so it can be returned
// by GrapherPlugin::newInstance().  A TQNPInstance represents the
// plugin, distinctly from the plugin window.
//
// Grapher is also a GraphModel, because it loads graph data from
// the net.  When Grapher creates a window in newWindow(), it creates
// a Graph widget to display the GraphModel that is the Grapher itself.
//

class Grapher : public TQNPInstance, GraphModel {
    TQ_OBJECT
public:
    // Create a Grapher - all Grapher plugins are created
    // by one GrapherPlugin object.
    //
    Grapher();
    ~Grapher();

    // We override this TQNPInstance function to create our
    // own subclass of TQNPWidget, a Graph widget.
    //
    TQNPWidget* newWindow();

    // We override this TQNPInstance function to process the
    // incoming graph data.
    //
    int write(TQNPStream* /*str*/, int /*offset*/, int len, void* buffer);

private:
    // Grapher is a GraphModel, so it implements the pure virtual
    // functions of that class.
    //
    TQPtrList<Datum>& graphData();
    ColType colType(int col) const;
    int nCols() const;

    void consumeLine();
    TQPtrList<Datum> data;
    TQBuffer line;
    int ncols;
    ColType *coltype;

private slots:
    // Slots that are connected to the Graph menu items.
    //
    void aboutPlugin();
    void aboutData();
};

Grapher::Grapher()
{
    data.setAutoDelete(TRUE);
    ncols = 0;
    line.open(IO_WriteOnly|IO_Truncate);
}

Grapher::~Grapher()
{
}

TQPtrList<GraphModel::Datum>& Grapher::graphData()
{
    return data;
}

GraphModel::ColType Grapher::colType(int col) const
{
    return coltype[col];
}

int Grapher::nCols() const
{
    return ncols;
}


TQNPWidget* Grapher::newWindow()
{
    // Create a Graph - our subclass of TQNPWidget.
    Graph *graph = new Graph(*this);

    // Look at the arguments from the EMBED tag.
    //   GRAPHSTYLE chooses pie or bar
    //   FONTFAMILY and FONTSIZE choose the font
    //
    const char* style = arg("GRAPHSTYLE");
    if ( style ) graph->setStyle(style);

    const char* fontfamily = arg("FONTFAMILY");
    const char* fontsize = arg("FONTSIZE");
    int ptsize = fontsize ? atoi(fontsize) : graph->font().pointSize();
    if (fontfamily) graph->setFont(TQFont(fontfamily, ptsize));

    connect(graph, TQ_SIGNAL(aboutPlugin()), this, TQ_SLOT(aboutPlugin()));
    connect(graph, TQ_SIGNAL(aboutData()), this, TQ_SLOT(aboutData()));

    return graph;
}

void Grapher::consumeLine()
{
    line.close();
    line.open(IO_ReadOnly);

    TQTextStream ts( &line );

    if (ncols == 0 ) {
	ncols=0;
	TQPtrList<ColType> typelist;
	typelist.setAutoDelete(TRUE);
	do {
	    TQString typestr;
	    ts >> typestr >> ws;
	    ColType* t = 0;
	    if ( typestr == "num" ) {
		t = new ColType(Numeric);
	    } else if ( typestr == "label" ) {
		t = new ColType(Label);
	    }
	    if (t) typelist.append(t);
	} while (!ts.atEnd());
	coltype = new ColType[ncols];
	for (ColType* t = typelist.first(); t; t = typelist.next()) {
	    coltype[ncols++] = *t;
	}
    } else {
	int col=0;
	Datum *rowdata = new Datum[ncols];
	while ( col < ncols && !ts.atEnd() ) {
	    switch (coltype[col]) {
	      case Numeric: {
		double value;
		ts >> value >> ws;
		rowdata[col].dbl = value;
		break;
	      }
	      case Label: {
		TQString* value = new TQString;
		ts >> *value >> ws;
		rowdata[col].str = value;
		break;
	      }
	    }
	    col++;
	}

	data.append(rowdata);
    }

    line.close();
    line.open(IO_WriteOnly|IO_Truncate);
}

int Grapher::write(TQNPStream* /*str*/, int /*offset*/, int len, void* buffer)
{
    // The browser calls this function when data is available on one
    // of the streams the plugin has requested.  Since we are only
    // processing one stream - the URL in the SRC argument of the EMBED
    // tag, we assume the TQNPStream is that one.  Also, since we do not
    // override TQNPInstance::writeReady(), we must accepts ALL the data
    // that is sent to this function.
    //
    char* txt = (char*)buffer;
    for (int i=0; i<len; i++) {
	char ch = txt[i];
	switch ( ch ) {
	  case '\n':
	    consumeLine();
	    break;
	  case '\r': // ignore;
	    break;
	  default:
	    line.putch(ch);
	}
    }
    if ( widget() )
	widget()->update();

    return len;
}

void Grapher::aboutPlugin()
{
    getURL( "http://doc.trolltech.com/netscape-plugin.html", "_blank" );
}

void Grapher::aboutData()
{
    const char* page = arg("DATAPAGE");
    if (page)
	getURL( page, "_blank" );
    else
	TQMessageBox::message("Help", "No help for this data");
}


//
// GrapherPlugin is the start of everything.  It is a TQNPlugin subclass,
// and it is responsible for describing the plugin to the browser, and
// creating instances of the plugin when it appears in web page.
//

class GrapherPlugin : public TQNPlugin {
public:
    GrapherPlugin()
    {
    }

    TQNPInstance* newInstance()
    {
	// Make a new Grapher, our subclass of TQNPInstance.
	return new Grapher;
    }

    const char* getMIMEDescription() const
    {
	// Describe the MIME types which this plugin can
	// process.  Just the concocted "application/x-graphable"
	// type, with the "g1n" filename extension.
	//
	return "application/x-graphable:g1n:Graphable ASCII numeric data";
    }

    const char * getPluginNameString() const
    {
	// The name of the plugin.  This is the title string used in
	// the "About Plugins" page of the browser.
	//
	return "TQt-based Graph Plugin";
    }

    const char * getPluginDescriptionString() const
    {
	// A longer description of the plugin.
	//
	return "A TQt-based LiveConnected plug-in that graphs numeric data";
    }

};

//
// Finally, we provide the implementation of TQNPlugin::create(), to
// provide our subclass of TQNPlugin.
//

TQNPlugin* TQNPlugin::create()
{
    return new GrapherPlugin;
}

#include "grapher.moc"
