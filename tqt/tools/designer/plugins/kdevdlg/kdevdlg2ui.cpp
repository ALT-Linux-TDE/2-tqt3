/**********************************************************************
**
** Copyright (C) 2005-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of TQt Designer.
**
** This file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the files LICENSE.GPL2
** and LICENSE.GPL3 included in the packaging of this file.
** Alternatively you may (at your option) use any later version
** of the GNU General Public License if such license has been
** publicly approved by Trolltech ASA (or its successors, if any)
** and the KDE Free TQt Foundation.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/.
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** Licensees holding valid TQt Commercial licenses may use this file in
** accordance with the TQt Commercial License Agreement provided with
** the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include "kdevdlg2ui.h"
#include <ntqdir.h>
#include <ntqstring.h>
#include <ntqptrstack.h>

/// some little helpers ///

void KDEVDLG2UI::wi()
{
    for ( int i = 0; i < indentation; i++ )
	*out << "    ";
}

void KDEVDLG2UI::indent()
{
    indentation++;
}

void KDEVDLG2UI::undent()
{
    indentation--;
}

void KDEVDLG2UI::writeClass( const TQString& name )
{
    wi(); *out << "<class>" << name << "</class>" << endl;
}

void KDEVDLG2UI::writeWidgetStart( const TQString& qclass )
{
    wi(); *out << "<widget class=\"" << qclass << "\">" << endl;
}

void KDEVDLG2UI::writeWidgetEnd()
{
    wi(); *out << "</widget>" << endl;
}

void KDEVDLG2UI::writeCString( const TQString& name, const TQString& value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<cstring>" << value << "</cstring>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeString( const TQString& name, const TQString& value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<string>" << value << "</string>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeRect( const TQString& name, int x, int y, int w, int h )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<rect>" << endl; indent();
    wi(); *out << "<x>" << x << "</x>" << endl;
    wi(); *out << "<y>" << y << "</y>" << endl;
    wi(); *out << "<width>" << w << "</width>" << endl;
    wi(); *out << "<height>" << h << "</height>" << endl; undent();
    wi(); *out << "</rect>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeFont( const TQString& family, int pointsize )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>font</name>" << endl;
    wi(); *out << "<font>" << endl; indent();
    wi(); *out << "<family>" << family << "</family>" << endl;
    wi(); *out << "<pointsize>" << pointsize << "</pointsize>" << endl; undent();
    wi(); *out << "</font>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeBool( const TQString& name, bool value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<bool>" << (value ? "true" : "false") << "</bool>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeNumber( const TQString& name, int value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<number>" << value << "</number>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeEnum( const TQString& name, const TQString& value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<enum>" << value << "</enum>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeSet( const TQString& name, const TQString& value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<set>" << value << "</set>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeItem( const TQString& name, const TQString& value )
{
    wi(); *out << "<item>" << endl; indent();
    writeString( name, value ); undent();
    wi(); *out << "</item>" << endl;
}

void KDEVDLG2UI::writeColumn( const TQString& name, const TQString& value )
{
    wi(); *out << "<column>" << endl; indent();
    writeString( name, value ); undent();
    wi(); *out << "</column>" << endl;
}

void KDEVDLG2UI::writeColor( const TQString& name, const TQString& value )
{
    int color = value.toInt();

    int r = color & 0x00ff0000 >> 16;
    int g = color & 0x0000ff00 >> 8;
    int b = color & 0x000000ff;
    
    wi(); *out << "<property>" << endl; indent(); //###FIX
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<color>" << endl; indent();
    wi(); *out << "<red>" << r << "</red>" << endl;
    wi(); *out << "<green>" << g << "</green>" << endl;
    wi(); *out << "<blue>" << b << "</blue>" << endl; undent();
    wi(); *out << "</color>" << endl;
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeStyles( const TQStringList styles, bool isFrame )
{
    if ( isFrame ) {
	bool defineFrame = FALSE;
	TQString shadow = "NoFrame";
	TQString shape = "StyledPanel";
	int width = 2;
	if ( styles.contains( "WS_EX_STATICEDGE" ) ) {
	    shadow = "Plain";
	    width = 1;
	    defineFrame = TRUE;
	}
	if ( styles.contains( "WS_EX_CLIENTEDGE" ) ) {
	    shadow = "Sunken";
	    defineFrame = TRUE;
	}
	if ( styles.contains( "WS_EX_DLGMODALFRAME" ) ) {
	    shadow = "Raised";
	    defineFrame = TRUE;
	}
	if ( !styles.contains( "WS_BORDER" ) ) {
	    shape = "NoFrame";
	    defineFrame = TRUE;
	}

	if ( defineFrame ) {
	    writeEnum( "frameShape", "StyledPanel" );
	    writeEnum( "frameShadow", shadow );
	    writeNumber( "lineWidth", width );
	}
    }

    if ( styles.contains("WS_DISABLED") )
	writeBool("enabled", FALSE );
    if ( styles.contains("WS_EX_ACCEPTFILES") )
	writeBool("acceptDrops", TRUE );
    if ( styles.contains("WS_EX_TRANSPARENT") )
	writeBool("autoMask", TRUE );
    if ( !styles.contains("WS_TABSTOP") )
	writeEnum("focusPolicy", "NoFocus");
}

/*!
  Constructs a KDEVDLG2UI object
*/

KDEVDLG2UI::KDEVDLG2UI( TQTextStream* input, const TQString& name )
{
    className = name;
    writeToFile = TRUE;
    in = input;
    indentation = 0;
    out = 0;
}

/*!
  Destructs the KDEVDLG2UI object
*/

KDEVDLG2UI::~KDEVDLG2UI()
{
}

/*!
  Parses the input stream and writes the output to files.
*/

bool KDEVDLG2UI::parse()
{
    TQFile fileOut;
    TQString buffer;
	
    if ( writeToFile ) {

	TQString outputFile = TQString( className ) + ".ui";
	fileOut.setName( outputFile );
	if (!fileOut.open( IO_WriteOnly ) )
	    tqFatal( "kdevdlg2ui: Could not open output file '%s'", outputFile.latin1() );
	out = new TQTextStream( &fileOut );
	targetFiles.append( outputFile );
    } else {
	out = new TQTextStream( &buffer, IO_WriteOnly );
    }
    
    writeDialog( className );

    delete out;
    out = 0;

    return TRUE;
}

/*!
  Parses the input stream and writes the output in \a get.
*/
bool KDEVDLG2UI::parse( TQStringList& get )
{
    writeToFile = FALSE;
    bool result = parse();
    get = target;
    return result;
}

/*!
  Replaces characters like '&', '<' and '>' with the proper encoding.
*/
void KDEVDLG2UI::cleanString( TQString * text )
{
    if ( !text ) return;
    text->replace( "\\n", "\n" );
    text->replace( "\\t", "\t" );
    text->replace( "&", "&amp;" );
    text->replace( "<", "&lt;" );
    text->replace( ">", "&gt;" );
}

/*!
  Builds a number of UI dialog out of the current input stream
*/
bool KDEVDLG2UI::writeDialog( const TQString& name )
{  
    *out << "<!DOCTYPE UI><UI>" << endl;
    writeClass( name );
    
    while ( !in->eof() ) {
	
	line = in->readLine().simplifyWhiteSpace();
	
	if ( line.left( 4 ) == "data" ) {
	    // ignore data section
	    while ( line.left( 1 ) != "}" && !in->eof() ) {
		line = in->readLine();
	    }
	} else if ( line.left( 4 ) == "item" ) {
	    writeWidgetStart( line.section( "//", 0, 0 ).section( " ", 1, 1 ) );
	} else if ( line.left( 1 ) == "{" ) {
	    indent();
	} else if ( line.left( 1 ) == "}" ) {
	    undent();
	    writeWidgetEnd();
	} else if ( line.left( 4 ) == "Name" ) {
	    TQString name = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "name", name );
	} else if ( line.left( 4 ) == "Font" ) {
	    TQString font = line.section( "//", 0, 0 ).section( "\"", 1 );
	    TQString family = font.section("\"", 1, 1 );
	    int pointSize = font.section("\"", 3, 3 ).toInt();
	    //int weight = font.section("\"", 5, 5 ).toInt();
	    //bool italic = ( font.section("\"", 7, 7 ) == "TRUE" );
	    writeFont( family, pointSize ); // weight, italic ?
	} else if ( line.left( 9 ) == "IsEnabled" ) {
	    bool isEnabled =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "enabled", isEnabled );
	} else if ( line.left( 12 ) == "AcceptsDrops" ) {
	    bool acceptDrops =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "acceptDrops", acceptDrops );
	} else if ( line.left( 12 ) == "isAutoResize" ) {
	    bool isAutoResize =
	    	( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "autoResize", isAutoResize ); //###FIX: obsolete
	} else if ( line.left( 12 ) == "isAutoRepeat" ) {
	    bool isAutoRepeat =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "autoRepeat", isAutoRepeat );
	} else if ( line.left( 9 ) == "isDefault" ) {
	    bool isDefault =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "default", isDefault );
	} else if ( line.left( 13 ) == "isAutoDefault" ) {
	    bool isAutoDefault =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "autoDefault", isAutoDefault );
	} else if ( line.left( 14 ) == "isToggleButton" ) {
	    bool isToggleButton =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "toggleButton", isToggleButton );
	} else if ( line.left( 11 ) == "isToggledOn" ) {
	    bool isToggledOn =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "on", isToggledOn );
	} else if ( line.left( 8 ) == "hasFrame" ) {
	    bool hasFrame =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "frame", hasFrame );
	} else if ( line.left( 10 ) == "isReadOnly" ) {
	    bool isReadOnly =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "readOnly", isReadOnly );
	} else if ( line.left( 9 ) == "isChecked" ) {
	    bool isChecked =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "checked", isChecked );
	} else if ( line.left( 16 ) == "isAutoCompletion" ) {
	    bool isAutoCompl =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "autoCompletion", isAutoCompl );
	} else if ( line.left( 8 ) == "EditText" ) {
	    bool editText =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "editable", editText );
	} else if ( line.left( 10 ) == "isTracking" ) {
	    bool isTracking =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "tracking", isTracking );
	} else if ( line.left( 16 ) == "isMultiSelection" ) {
	    bool isMultiSel =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "multiSelection", isMultiSel );
	} else if ( line.left( 21 ) == "isAllColumnsShowFocus" ) {
	    bool isAllColsShowFocus =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "allColumnsShowFocus", isAllColsShowFocus );
	} else if ( line.left( 16 ) == "isRootDecorated" ) {
	    bool isRootDec =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "rootIsDecorated", isRootDec );
	} else if ( line.left( 1 ) == "X" ) {
	    int x = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    line = in->readLine().stripWhiteSpace();
	    int y = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    line = in->readLine().stripWhiteSpace();
	    int w = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    line = in->readLine().stripWhiteSpace();
	    int h = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeRect( "geometry", x, y, w, h );
	} else if ( line.left( 8 ) == "MinWidth" ) {
	    int minw = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "minimumWidth", minw );
	} else if ( line.left( 9 ) == "MinHeight" ) {
	    int minh = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "minimumHeight", minh );
	} else if ( line.left( 8 ) == "MaxWidth" ) {
	    int maxw = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maximumWidth", maxw );
	} else if ( line.left( 9 ) == "MaxHeight" ) {
	    int maxh = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maximumHeight", maxh );
	} else if ( line.left( 4 ) == "Text" ) {
	    TQString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & text );
	    writeString( "text", text );
	} else if ( line.left( 5 ) == "Title" ) {
	    TQString title = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & title );
	    writeString( "title", title );
	} else if ( line.left( 5 ) == "Buddy" ) {
	    TQString buddy = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "buddy", buddy );
	} else if ( line.left( 14 ) == "SpecialValText" ) {
	    TQString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "specialValueText", text );
	} else if ( line.left( 6 ) == "Prefix" ) {
	    TQString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "prefix", text );
	}  else if ( line.left( 6 ) == "Suffix" ) {
	    TQString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "suffix", text );
	} else if ( line.left( 5 ) == "Value" ) {
	    int v = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "value", v );
	} else if ( line.left( 8 ) == "MinValue" ) {
	    int minv = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "minValue", minv );
	} else if ( line.left( 8 ) == "MaxValue" ) {
	    int maxv = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maxValue", maxv );
	} else if ( line.left( 9 ) == "SizeLimit" ) {
	    int limit = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "sizeLimit", limit );
	} else if ( line.left( 9 ) == "MaxLength" ) {
	    int maxl = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maxLength", maxl );
	} else if ( line.left( 8 ) == "MaxCount" ) {
	    int maxc = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maxCount", maxc );
	} else if ( line.left( 14 ) == "CursorPosition" ) {
	    int pos = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "cursorPosition", pos );
	} else if ( line.left( 9 ) == "NumDigits" ) {
	    int digits = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "numDigits", digits );
	} else if ( line.left( 10 ) == "TotalSteps" ) {
	    int steps = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "totalSteps", steps );
	} else if ( line.left( 12 ) == "TreeStepSize" ) {
	    int stepSize = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "treeStepSize", stepSize );
	} else if ( line.left( 10 ) == "ItemMargin" ) {
	    int margin = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "itemMargin", margin );
	} else if ( line.left( 7 ) == "ToolTip" ) {
	    TQString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "toolTip", text );
	} else if ( line.left( 9 ) == "QuickHelp" ) {
	    TQString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & text );
	    writeString( "whatsThis", text );
	} else if ( line.left( 15 ) == "InsertionPolicy" ) {
	    TQString policy = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & policy );
	    writeEnum( "insertionPolicy", policy ); //###FIX: TQComboBox::
	} else if ( line.left( 11 ) == "Orientation" ) {
	    TQString orientation =
		line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & orientation );
	    if ( orientation == "V" )
		writeEnum( "orientation", "TQt::Vertical"  );
	    else if ( orientation == "H" )
		writeEnum( "orientation", "TQt::Horizontal"  );
	} else if ( line.left( 14 ) == "vScrollBarMode" ) {
	    TQString mode = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & mode );
	    writeEnum( "vScrollBarMode", mode );
	} else if ( line.left( 14 ) == "hScrollBarMode" ) {
	    TQString mode = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & mode );
	    writeEnum( "hScrollBarMode", mode );
	} else if ( line.left( 7 ) == "Entries" ) {
	    TQString entries = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & entries );
	    TQStringList l = TQStringList::split( '\n', entries );
	    for ( TQStringList::Iterator it = l.begin(); it != l.end(); ++it )
		writeItem( "text", *it );
	} else if ( line.left( 7 ) == "Columns" ) {
	    TQString columns = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & columns );
	    TQStringList l = TQStringList::split( '\n', columns );
	    for ( TQStringList::Iterator it = l.begin(); it != l.end(); ++it )
		writeColumn( "text", *it );
	} else if ( line.left( 6 ) == "BgMode" ) {
	   TQString mode = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	   cleanString( & mode );
	   writeString( "backgroundMode", mode ); //###FIX: TQWidget:::
	} else if ( line.left( 10 ) == "BgPalColor" ) {
	   TQString color = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	   cleanString( & color );
	   writeColor( "paletteBackgroundColor", color );
	} //else {
	    //if ( line.length() )
	    //tqDebug( "IGNORED: %s", line.latin1() );
	//}
    }
    *out << "</UI>" << endl;
    return TRUE;
}

