/**********************************************************************
** Copyright (C) 2000-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the TQt Assistant.
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

#include "docuparser.h"
#include "profile.h"

#include <ntqdir.h>
#include <ntqfile.h>
#include <ntqfileinfo.h>
#include <ntqregexp.h>
#include <ntqstring.h>
#include <ntqxml.h>

TQDataStream &operator>>( TQDataStream &s, ContentItem &ci )
{
    s >> ci.title;
    s >> ci.reference;
    s >> ci.depth;
    return s;
}

TQDataStream &operator<<( TQDataStream &s, const ContentItem &ci )
{
    s << ci.title;
    s << ci.reference;
    s << ci.depth;
    return s;
}

const TQString DocuParser::DocumentKey = "/TQt Assistant/" + TQString(TQT_VERSION_STR) + "/";

DocuParser *DocuParser::createParser( const TQString &fileName )
{
    TQFile file( fileName );
    if( !file.open( IO_ReadOnly ) ) {	
	return 0;	
    }
    
    TQString str;
    int read = 0;
    int maxlen = 1024;
    int majVer = 0, minVer = 0, serVer = 0;
    static TQRegExp re( "assistantconfig +version=\"(\\d)\\.(\\d)\\.(\\d)\"", FALSE );
    Q_ASSERT( re.isValid() );
    while( read != -1 ) {
 	read = file.readLine( str, maxlen );
	if( re.search( str ) >= 0 ) {
	    majVer = re.cap( 1 ).toInt();
	    minVer = re.cap( 2 ).toInt();
	    serVer = re.cap( 3 ).toInt();
	}
    }

    if( majVer == 3 && minVer >= 2 ) 
	return new DocuParser320;
    
    return new DocuParser310;	
}


bool DocuParser::parse( TQFile *file )
{
    TQXmlInputSource source( file );
    TQXmlSimpleReader reader;
    reader.setContentHandler( this );
    reader.setErrorHandler( this );
    setFileName( TQFileInfo( *file ).absFilePath() );
    return reader.parse( source );
}


TQString DocuParser::errorProtocol() const
{
    return errorProt;
}


TQValueList<ContentItem> DocuParser::getContentItems()
{
    return contentList;
}


TQPtrList<IndexItem> DocuParser::getIndexItems()
{
    return indexList;
}

TQString DocuParser::absolutify( const TQString &name ) const
{
    TQFileInfo orgPath( name );
    if( orgPath.isRelative() )
	return TQFileInfo( fname ).dirPath() + TQDir::separator() + name;
    return name;
}


void DocuParser310::addTo( Profile *p )
{
    p->addDCFTitle( fname, docTitle );
    p->addDCFIcon( docTitle, iconName );
    p->addDCFIndexPage( docTitle, conURL );
}


bool DocuParser310::startDocument()
{
    state = StateInit;
    errorProt = "";

    contentRef = "";
    indexRef = "";
    depth = 0;
    contentList.clear();
    indexList.clear();

    return TRUE;
}


bool DocuParser310::startElement( const TQString &, const TQString &,
			       const TQString &qname,
			       const TQXmlAttributes &attr )
{
    if (qname == "DCF" && state == StateInit) {
	state = StateContent;
	contentRef = absolutify( attr.value( "ref" ) );
	conURL = contentRef;
	docTitle = attr.value( "title" );
	iconName = absolutify( attr.value( "icon" ) );
	contentList.append( ContentItem( docTitle, contentRef, depth ) );
    } else if (qname == "section" && (state == StateContent || state == StateSect)) {
	state = StateSect;
	contentRef = absolutify( attr.value( "ref" ) );
	title = attr.value( "title" );
	depth++;
	contentList.append( ContentItem( title, contentRef, depth ) );
    } else if (qname == "keyword" && state == StateSect) {
	state = StateKeyword;
	indexRef = absolutify( attr.value( "ref" ) );
    } else
	return FALSE;
    return TRUE;
}

bool DocuParser310::endElement( const TQString &nameSpace, const TQString &localName,
			     const TQString &qName )
{
    switch( state ) {
    case StateInit:
	break;
    case StateContent:
	state = StateInit;
	break;
    case StateSect:
	state = --depth ? StateSect : StateContent;
	break;
    case StateKeyword:
	state = StateSect;
	break;
    }
    return TRUE;
}


bool DocuParser310::characters( const TQString& ch )
{
    TQString str = ch.simplifyWhiteSpace();
    if ( str.isEmpty() )
	return TRUE;

    switch ( state ) {
	case StateInit:
        case StateContent:
        case StateSect:
            return FALSE;
	    break;
        case StateKeyword:
	    indexList.append( new IndexItem( str, indexRef ) );
	    break;
	default:
            return FALSE;
    }
    return TRUE;
}


bool DocuParser310::fatalError( const TQXmlParseException& exception )
{
    errorProt += TQString( "fatal parsing error: %1 in line %2, column %3\n" )
        .arg( exception.message() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() );

    return TQXmlDefaultHandler::fatalError( exception );
}


DocuParser320::DocuParser320()
    : prof( new Profile )
{
}


void DocuParser320::addTo( Profile *p )
{
    TQMap<TQString,TQString>::ConstIterator it;

    for (it = prof->dcfTitles.begin(); it != prof->dcfTitles.end(); ++it)
	p->dcfTitles[it.key()] = *it;

    for (it = prof->icons.begin(); it != prof->icons.end(); ++it)
	p->icons[it.key()] = *it;

    for (it = prof->indexPages.begin(); it != prof->indexPages.end(); ++it)
	p->indexPages[it.key()] = *it;
}


bool DocuParser320::startDocument()
{
    state = StateInit;
    errorProt = "";

    contentRef = "";
    indexRef = "";
    depth = 0;
    contentList.clear();
    indexList.clear();

    prof->addDCF( fname );

    return TRUE;
}

bool DocuParser320::startElement( const TQString &, const TQString &,
			       const TQString &qname,
			       const TQXmlAttributes &attr )
{
    TQString lower = qname.lower();

    switch( state ) {
	
    case StateInit:
	if( lower == "assistantconfig" )	    
	    state = StateDocRoot;
	break;

    case StateDocRoot:
	if( lower == "dcf" ) {
	    state = StateContent;
	    contentRef = absolutify( attr.value( "ref" ) );
	    conURL = contentRef;
	    docTitle = attr.value( "title" );
	    iconName = absolutify( attr.value( "icon" ) );
	    contentList.append( ContentItem( docTitle, contentRef, depth ) );
	} else if( lower == "profile" ) {
	    state = StateProfile;
	}
	break;

    case StateSect:
	if ( lower == "keyword" && state == StateSect ) {
	    state = StateKeyword;
	    indexRef = absolutify( attr.value( "ref" ) );
	    break;
	} // else if (lower == "section")
    case StateContent:
	if( lower == "section" ) {
	    state = StateSect;
	    contentRef = absolutify( attr.value( "ref" ) );
	    title = attr.value( "title" );
	    depth++;
	    contentList.append( ContentItem( title, contentRef, depth ) );
	}
	break;

    case StateProfile:
	if( lower == "property" ) {
	    state = StateProperty;
	    propertyName = attr.value( "name" );
	}
	break;

    case StateProperty:
	break;
    }    
    
    return TRUE;
}

bool DocuParser320::endElement( const TQString &nameSpace,
				const TQString &localName,
				const TQString &qName )
{
    switch( state ) {
    case StateInit:
	break;
    case StateDocRoot:
	state = StateInit;
	break;
    case StateProfile:
	state = StateDocRoot;
	break;
    case StateProperty:
	state = StateProfile;
	if( propertyName.isEmpty() || propertyValue.isEmpty() )
	    return FALSE;
	{
	    static const TQStringList lst = TQStringList() << "startpage" << "abouturl"
							 << "applicationicon" << "assistantdocs";
	    if (lst.contains(propertyName))
		propertyValue = absolutify( propertyValue );
	}
	prof->addProperty( propertyName, propertyValue );
	break;
    case StateContent:
	if( !iconName.isEmpty() ) prof->addDCFIcon( docTitle, iconName );
	if( contentRef.isEmpty() )
	    return FALSE;
	prof->addDCFIndexPage( docTitle, conURL );
	prof->addDCFTitle( fname, docTitle );
	state = StateDocRoot;
	break;
    case StateSect:
	state = --depth ? StateSect : StateContent;
	break;
    case StateKeyword:
	state = StateSect;
	break;
    }
    return TRUE;
}

bool DocuParser320::characters( const TQString& ch )
{
    TQString str = ch.simplifyWhiteSpace();
    if ( str.isEmpty() )
	return TRUE;

    switch ( state ) {
    case StateInit:
    case StateContent:
    case StateSect:
	return FALSE;
	break;
    case StateKeyword:
	indexList.append( new IndexItem( str, indexRef ) );
	break;
    case StateProperty:
        propertyValue = ch;	
	break;
    default:
	return FALSE;
    }
    return TRUE;
}

bool DocuParser320::fatalError( const TQXmlParseException& exception )
{
    errorProt += TQString( "fatal parsing error: %1 in line %2, column %3\n" )
        .arg( exception.message() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() );

    return TQXmlDefaultHandler::fatalError( exception );
}
