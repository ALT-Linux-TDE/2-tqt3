/****************************************************************************
**
** Implementation of TQUrl class
**
** Created : 950429
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the TQt GUI Toolkit.
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
** This file may be used under the terms of the Q Public License as
** defined by Trolltech ASA and appearing in the file LICENSE.TQPL
** included in the packaging of this file.  Licensees holding valid TQt
** Commercial licenses may use this file in accordance with the TQt
** Commercial License Agreement provided with the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include "ntqurl.h"

#ifndef TQT_NO_URL

#include "ntqdir.h"

#include <stdlib.h>

class TQUrlPrivate
{
public:
    TQString protocol;
    TQString user;
    TQString pass;
    TQString host;
    TQString path, cleanPath;
    TQString refEncoded;
    TQString queryEncoded;
    bool isValid;
    int port;
    bool cleanPathDirty;
};

/*!
    Replaces backslashes with slashes and removes multiple occurrences
    of slashes or backslashes if \c allowMultiple is FALSE.
*/

static void slashify( TQString& s, bool allowMultiple = TRUE )
{
    bool justHadSlash = FALSE;
    for ( int i = 0; i < (int)s.length(); i++ ) {
	if ( !allowMultiple && justHadSlash &&
	     ( s[ i ] == '/' || s[ i ] == '\\' ) ) {
	    s.remove( i, 1 );
	    --i;
	    continue;
	}
	if ( s[ i ] == '\\' )
	    s[ i ] = '/';
#if defined (TQ_WS_MAC9)
	if ( s[ i ] == ':' && (i == (int)s.length()-1 || s[ i + 1 ] != '/' ) ) //mac colon's go away, unless after a protocol
		s[ i ] = '/';
#endif
	if ( s[ i ] == '/' )
	    justHadSlash = TRUE;
	else
	    justHadSlash = FALSE;
    }
}



/*!
    \class TQUrl ntqurl.h

    \brief The TQUrl class provides a URL parser and simplifies working with URLs.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">TQt Enterprise Edition</a>.
\endif

    \ingroup io
    \ingroup misc
    \mainclass

    \module network

    The TQUrl class is provided for simple work with URLs. It can
    parse, decode, encode, etc.

    TQUrl works with the decoded path and encoded query in turn.

    Example:

    <tt>http://www.trolltech.com:80/cgi-bin/test%20me.pl?cmd=Hello%20you</tt>

    \table
    \header \i Function	    \i Returns
    \row \i \l protocol()   \i "http"
    \row \i \l host()	    \i "www.trolltech.com"
    \row \i \l port()	    \i 80
    \row \i \l path()	    \i "/cgi-bin/test&nbsp;me.pl"
    \row \i \l fileName()   \i "test&nbsp;me.pl"
    \row \i \l query()	    \i "cmd=Hello%20you"
    \endtable

    Example:

    <tt>http://doc.trolltech.com/ntqdockarea.html#lines</tt>

    \table
    \header \i Function	    \i Returns
    \row \i \l protocol()   \i "http"
    \row \i \l host()	    \i "doc.trolltech.com"
    \row \i \l fileName()   \i "ntqdockarea.html"
    \row \i \l ref()	    \i "lines"
    \endtable

    The individual parts of a URL can be set with setProtocol(),
    setHost(), setPort(), setPath(), setFileName(), setRef() and
    setQuery(). A URL could contain, for example, an ftp address which
    requires a user name and password; these can be set with setUser()
    and setPassword().

    Because path is always encoded internally you must not use "%00"
    in the path, although this is okay (but not recommended) for the
    query.

    TQUrl is normally used like this:

    \code
    TQUrl url( "http://www.trolltech.com" );
    // or
    TQUrl url( "file:/home/myself/Mail", "Inbox" );
    \endcode

    You can then access and manipulate the various parts of the URL.

    To make it easy to work with TQUrls and TQStrings, TQUrl implements
    the necessary cast and assignment operators so you can do
    following:

    \code
    TQUrl url( "http://www.trolltech.com" );
    TQString s = url;
    // or
    TQString s( "http://www.trolltech.com" );
    TQUrl url( s );
    \endcode

    Use the static functions, encode() and decode() to encode or
    decode a URL in a string. (They operate on the string in-place.)
    The isRelativeUrl() static function returns TRUE if the given
    string is a relative URL.

    If you want to use a URL to work on a hierarchical structure (e.g.
    a local or remote filesystem), you might want to use the subclass
    TQUrlOperator.

    \sa TQUrlOperator
*/


/*!
    Constructs an empty URL that is invalid.
*/

TQUrl::TQUrl()
{
    d = new TQUrlPrivate;
    d->isValid = FALSE;
    d->port = -1;
    d->cleanPathDirty = TRUE;
}

/*!
    Constructs a URL by parsing the string \a url.

    If you pass a string like "/home/qt", the "file" protocol is
    assumed.
*/

TQUrl::TQUrl( const TQString& url )
{
    d = new TQUrlPrivate;
    d->protocol = "file";
    d->port = -1;
    parse( url );
}

/*!
    Copy constructor. Copies the data of \a url.
*/

TQUrl::TQUrl( const TQUrl& url )
{
    d = new TQUrlPrivate;
    *d = *url.d;
}

/*!
    Returns TRUE if \a url is relative; otherwise returns FALSE.
*/

bool TQUrl::isRelativeUrl( const TQString &url )
{
    int colon = url.find( ":" );
    int slash = url.find( "/" );

    return ( slash != 0 && ( colon == -1 || ( slash != -1 && colon > slash ) ) );
}

/*!
    Constructs an URL taking \a url as the base (context) and
    \a relUrl as a relative URL to \a url. If \a relUrl is not relative,
    \a relUrl is taken as the new URL.

    For example, the path of
    \code
    TQUrl url( "ftp://ftp.trolltech.com/qt/source", "qt-2.1.0.tar.gz" );
    \endcode
    will be "/qt/srource/qt-2.1.0.tar.gz".

    On the other hand,
    \code
    TQUrl url( "ftp://ftp.trolltech.com/qt/source", "/usr/local" );
    \endcode
    will result in a new URL, "ftp://ftp.trolltech.com/usr/local",
    because "/usr/local" isn't relative.

    Similarly,
    \code
    TQUrl url( "ftp://ftp.trolltech.com/qt/source", "file:/usr/local" );
    \endcode
    will result in a new URL, with "/usr/local" as the path
    and "file" as the protocol.

    Normally it is expected that the path of \a url points to a
    directory, even if the path has no slash at the end. But if you
    want the constructor to handle the last part of the path as a file
    name if there is no slash at the end, and to let it be replaced by
    the file name of \a relUrl (if it contains one), set \a checkSlash
    to TRUE.
*/

TQUrl::TQUrl( const TQUrl& url, const TQString& relUrl, bool checkSlash )
{
    d = new TQUrlPrivate;
    TQString rel = relUrl;
    slashify( rel );

    TQUrl urlTmp( url );
    if ( !urlTmp.isValid() ) {
	urlTmp.reset();
    }
    if ( isRelativeUrl( rel ) ) {
	if ( rel[ 0 ] == '#' ) {
	    *this = urlTmp;
	    rel.remove( (uint)0, 1 );
	    decode( rel );
	    setRef( rel );
	} else if ( rel[ 0 ] == '?' ) {
	    *this = urlTmp;
	    rel.remove( (uint)0, 1 );
	    setQuery( rel );
	} else {
	    decode( rel );
	    *this = urlTmp;
	    setRef( TQString::null );
	    if ( checkSlash && d->cleanPath[(int)path().length()-1] != '/' ) {
		if ( isRelativeUrl( path() ) )
		    setEncodedPathAndQuery( rel );
		else
		    setFileName( rel );
	    } else {
		TQString p = urlTmp.path();
		if ( p.isEmpty() ) {
		    // allow URLs like "file:foo"
		    if ( !d->host.isEmpty() && !d->user.isEmpty() && !d->pass.isEmpty() )
			p = "/";
		}
		if ( !p.isEmpty() && p.right(1)!="/" )
		    p += "/";
		p += rel;
		d->path = p;
		d->cleanPathDirty = TRUE;
	    }
	}
    } else {
	if ( rel[ 0 ] == TQChar( '/' ) ) {
	    *this = urlTmp;
	    setEncodedPathAndQuery( rel );
	} else {
	    *this = rel;
	}
    }
}

/*!
    Destructor.
*/

TQUrl::~TQUrl()
{
    delete d;
    d = 0;
}

/*!
    Returns the protocol of the URL. Typically, "file", "http", "ftp",
    etc.

    \sa setProtocol()
*/

TQString TQUrl::protocol() const
{
    return d->protocol;
}

/*!
    Sets the protocol of the URL to \a protocol. Typically, "file",
    "http", "ftp", etc.

    \sa protocol()
*/

void TQUrl::setProtocol( const TQString& protocol )
{
    d->protocol = protocol;
    if ( hasHost() )
	d->isValid = TRUE;
}

/*!
    Returns the username of the URL.

    \sa setUser() setPassword()
*/

TQString TQUrl::user() const
{
    return  d->user;
}

/*!
    Sets the username of the URL to \a user.

    \sa user() setPassword()
*/

void TQUrl::setUser( const TQString& user )
{
    d->user = user;
}

/*!
    Returns TRUE if the URL contains a username; otherwise returns
    FALSE.

    \sa setUser() setPassword()
*/

bool TQUrl::hasUser() const
{
    return !d->user.isEmpty();
}

/*!
    Returns the password of the URL.

    \warning Passwords passed in URLs are normally \e insecure; this
    is due to the mechanism, not because of TQt.

    \sa setPassword() setUser()
*/

TQString TQUrl::password() const
{
    return d->pass;
}

/*!
    Sets the password of the URL to \a pass.

    \warning Passwords passed in URLs are normally \e insecure; this
    is due to the mechanism, not because of TQt.

    \sa password() setUser()
*/

void TQUrl::setPassword( const TQString& pass )
{
    d->pass = pass;
}

/*!
    Returns TRUE if the URL contains a password; otherwise returns
    FALSE.

    \warning Passwords passed in URLs are normally \e insecure; this
    is due to the mechanism, not because of TQt.

    \sa setPassword() setUser()
*/

bool TQUrl::hasPassword() const
{
    return !d->pass.isEmpty();
}

/*!
    Returns the hostname of the URL.

    \sa setHost() hasHost()
*/

TQString TQUrl::host() const
{
    return d->host;
}

/*!
    Sets the hostname of the URL to \a host.

    \sa host() hasHost()
*/

void TQUrl::setHost( const TQString& host )
{
    d->host = host;
    if ( !d->protocol.isNull() && d->protocol != "file" )
	d->isValid = TRUE;
}

/*!
    Returns TRUE if the URL contains a hostname; otherwise returns
    FALSE.

    \sa setHost()
*/

bool TQUrl::hasHost() const
{
    return !d->host.isEmpty();
}

/*!
    Returns the port of the URL or -1 if no port has been set.

    \sa setPort()
*/

int TQUrl::port() const
{
    return d->port;
}

/*!
    Sets the port of the URL to \a port.

    \sa port()
*/

void TQUrl::setPort( int port )
{
    d->port = port;
}

/*!
    Returns TRUE if the URL contains a port; otherwise returns FALSE.

    \sa setPort()
*/

bool TQUrl::hasPort() const
{
    return d->port >= 0;
}

/*!
    Sets the path of the URL to \a path.

    \sa path() hasPath()
*/

void TQUrl::setPath( const TQString& path )
{
    d->path = path;
    slashify( d->path );
    d->cleanPathDirty = TRUE;
    d->isValid = TRUE;
}

/*!
    Returns TRUE if the URL contains a path; otherwise returns FALSE.

    \sa path() setPath()
*/

bool TQUrl::hasPath() const
{
    return !d->path.isEmpty();
}

/*!
    Sets the query of the URL to \a txt. \a txt must be encoded.

    \sa query() encode()
*/

void TQUrl::setQuery( const TQString& txt )
{
    d->queryEncoded = txt;
}

/*!
    Returns the (encoded) query of the URL.

    \sa setQuery() decode()
*/

TQString TQUrl::query() const
{
    return d->queryEncoded;
}

/*!
    Returns the (encoded) reference of the URL.

    \sa setRef() hasRef() decode()
*/

TQString TQUrl::ref() const
{
    return d->refEncoded;
}

/*!
    Sets the reference of the URL to \a txt. \a txt must be encoded.

    \sa ref() hasRef() encode()
*/

void TQUrl::setRef( const TQString& txt )
{
    d->refEncoded = txt;
}

/*!
    Returns TRUE if the URL has a reference; otherwise returns FALSE.

    \sa setRef()
*/

bool TQUrl::hasRef() const
{
    return !d->refEncoded.isEmpty();
}

/*!
    Returns TRUE if the URL is valid; otherwise returns FALSE. A URL
    is invalid if it cannot be parsed, for example.
*/

bool TQUrl::isValid() const
{
    return d->isValid;
}

/*!
    Resets all parts of the URL to their default values and
    invalidates it.
*/

void TQUrl::reset()
{
    d->protocol = "file";
    d->user = "";
    d->pass = "";
    d->host = "";
    d->path = "";
    d->queryEncoded = "";
    d->refEncoded = "";
    d->isValid = TRUE;
    d->port = -1;
    d->cleanPathDirty = TRUE;
}

/*!
    Parses the \a url.
*/

bool TQUrl::parse( const TQString& url )
{
    TQString url_( url );
    slashify( url_ );

    if ( url_.isEmpty() ) {
	d->isValid = FALSE;
	return FALSE;
    }

    d->cleanPathDirty = TRUE;
    d->isValid = TRUE;
    TQString oldProtocol = d->protocol;
    d->protocol = TQString::null;

    const int Init	= 0;
    const int Protocol	= 1;
    const int Separator1= 2; // :
    const int Separator2= 3; // :/
    const int Separator3= 4; // :// or more slashes
    const int User	= 5;
    const int Pass	= 6;
    const int Host	= 7;
    const int Path	= 8;
    const int Ref	= 9;
    const int Query	= 10;
    const int Port	= 11;
    const int Done	= 12;

    const int InputAlpha= 1;
    const int InputDigit= 2;
    const int InputSlash= 3;
    const int InputColon= 4;
    const int InputAt	= 5;
    const int InputHash = 6;
    const int InputQuery= 7;

    static uchar table[ 12 ][ 8 ] = {
     /* None       InputAlpha  InputDigit  InputSlash  InputColon  InputAt     InputHash   InputQuery */
	{ 0,       Protocol,   0,          Path,       0,          0,          0,          0,         }, // Init
	{ 0,       Protocol,   Protocol,   0,          Separator1, 0,          0,          0,         }, // Protocol
	{ 0,       Path,       Path,       Separator2, 0,          0,          0,          0,         }, // Separator1
	{ 0,       Path,       Path,       Separator3, 0,          0,          0,          0,         }, // Separator2
	{ 0,       User,       User,       Separator3, Pass,       Host,       0,          0,         }, // Separator3
	{ 0,       User,       User,       User,       Pass,       Host,       User,       User,      }, // User
	{ 0,       Pass,       Pass,       Pass,       Pass,       Host,       Pass,       Pass,      }, // Pass
	{ 0,       Host,       Host,       Path,       Port,       Host,       Ref,        Query,     }, // Host
	{ 0,       Path,       Path,       Path,       Path,       Path,       Ref,        Query,     }, // Path
	{ 0,       Ref,        Ref,        Ref,        Ref,        Ref,        Ref,        Query,     }, // Ref
	{ 0,       Query,      Query,      Query,      Query,      Query,      Query,      Query,     }, // Query
	{ 0,       0,          Port,       Path,       0,          0,          0,          0,         }  // Port
    };

    bool relPath = FALSE;

    relPath = FALSE;
    bool forceRel = FALSE;

    // If ':' is at pos 1, we have only one letter
    // before that separator => that's a drive letter!
    if ( url_.length() >= 2 && url_[1] == ':' )
	relPath = forceRel = TRUE;

    int hasNoHost = -1;
    int cs = url_.find( ":/" );
    if ( cs != -1 ) // if a protocol is there, find out if there is a host or directly the path after it
	hasNoHost = url_.find( "///", cs );
    table[ 4 ][ 1 ] = User;
    table[ 4 ][ 2 ] = User;
    if ( cs == -1 || forceRel ) { // we have a relative file
	if ( url.find( ':' ) == -1 || forceRel ) {
	    table[ 0 ][ 1 ] = Path;
	    // Filenames may also begin with a digit
	    table[ 0 ][ 2 ] = Path;
	} else {
	    table[ 0 ][ 1 ] = Protocol;
	}
	relPath = TRUE;
    } else { // some checking
	table[ 0 ][ 1 ] = Protocol;

	// find the part between the protocol and the path as the meaning
	// of that part is dependend on some chars
	++cs;
	while ( url_[ cs ] == '/' )
	    ++cs;
	int slash = url_.find( "/", cs );
	if ( slash == -1 )
	    slash = url_.length() - 1;
	TQString tmp = url_.mid( cs, slash - cs + 1 );

	if ( !tmp.isEmpty() ) { // if this part exists

	    // look for the @ in this part
	    int at = tmp.find( "@" );
	    if ( at != -1 )
		at += cs;
	    // we have no @, which means host[:port], so directly
	    // after the protocol the host starts, or if the protocol
	    // is file or there were more than 2 slashes, it�s the
	    // path
	    if ( at == -1 ) {
		if ( url_.left( 4 ) == "file" || hasNoHost != -1 )
		    table[ 4 ][ 1 ] = Path;
		else
		    table[ 4 ][ 1 ] = Host;
		table[ 4 ][ 2 ] = table[ 4 ][ 1 ];
	    }
	}
    }

    int state = Init; // parse state
    int input; // input token

    TQChar c = url_[ 0 ];
    int i = 0;
    TQString port;

    for ( ;; ) {
	switch ( c ) {
	case '?':
	    input = InputQuery;
	    break;
	case '#':
	    input = InputHash;
	    break;
	case '@':
	    input = InputAt;
	    break;
	case ':':
	    input = InputColon;
	    break;
	case '/':
	    input = InputSlash;
	    break;
	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9': case '0':
	    input = InputDigit;
	    break;
	default:
	    input = InputAlpha;
	}

	state = table[ state ][ input ];

	switch ( state ) {
	case Protocol:
	    d->protocol += c;
	    break;
	case User:
	    d->user += c;
	    break;
	case Pass:
	    d->pass += c;
	    break;
	case Host:
	    d->host += c;
	    break;
	case Path:
	    d->path += c;
	    break;
	case Ref:
	    d->refEncoded += c;
	    break;
	case Query:
	    d->queryEncoded += c;
	    break;
	case Port:
	    port += c;
	    break;
	default:
	    break;
	}

	++i;
	if ( i > (int)url_.length() - 1 || state == Done || state == 0 )
	    break;
	c = url_[ i ];

    }

    if ( !port.isEmpty() ) {
	port.remove( (uint)0, 1 );
	d->port = atoi( port.latin1() );
    }

    // error
    if ( i < (int)url_.length() - 1 ) {
	d->isValid = FALSE;
	return FALSE;
    }


    if ( d->protocol.isEmpty() )
	d->protocol = oldProtocol;

    if ( d->path.isEmpty() )
	d->path = "/";

    // hack for windows
    if ( d->path.length() == 2 && d->path[ 1 ] == ':' )
	d->path += "/";

    // #### do some corrections, should be done nicer too
    if ( !d->pass.isEmpty() ) {
	if ( d->pass[ 0 ] == ':' )
	    d->pass.remove( (uint)0, 1 );
	decode( d->pass );
    }
    if ( !d->user.isEmpty() ) {
	decode( d->user );
    }
    if ( !d->path.isEmpty() ) {
	if ( d->path[ 0 ] == '@' || d->path[ 0 ] == ':' )
	    d->path.remove( (uint)0, 1 );
	if ( d->path[ 0 ] != '/' && !relPath && d->path[ 1 ] != ':' )
	    d->path.prepend( "/" );
    }
    if ( !d->refEncoded.isEmpty() && d->refEncoded[ 0 ] == '#' )
	d->refEncoded.remove( (uint)0, 1 );
    if ( !d->queryEncoded.isEmpty() && d->queryEncoded[ 0 ] == '?' )
	d->queryEncoded.remove( (uint)0, 1 );
    if ( !d->host.isEmpty() && d->host[ 0 ] == '@' )
	d->host.remove( (uint)0, 1 );

#if defined(Q_OS_WIN32)
    // hack for windows file://machine/path syntax
    if ( d->protocol == "file" ) {
	if ( url.left( 7 ) == "file://" &&
	     d->path.length() > 1 && d->path[ 1 ] != ':' )
		 d->path.prepend( "/" );
    }
#endif

    decode( d->path );
    d->cleanPathDirty = TRUE;

#if 0
    tqDebug( "URL: %s", url.latin1() );
    tqDebug( "protocol: %s", d->protocol.latin1() );
    tqDebug( "user: %s", d->user.latin1() );
    tqDebug( "pass: %s", d->pass.latin1() );
    tqDebug( "host: %s", d->host.latin1() );
    tqDebug( "path: %s", path().latin1() );
    tqDebug( "ref: %s", d->refEncoded.latin1() );
    tqDebug( "query: %s", d->queryEncoded.latin1() );
    tqDebug( "port: %d\n\n----------------------------\n\n", d->port );
#endif

    return TRUE;
}

/*!
    \overload

    Parses \a url and assigns the resulting data to this class.

    If you pass a string like "/home/qt" the "file" protocol will be
    assumed.
*/

TQUrl& TQUrl::operator=( const TQString& url )
{
    reset();
    parse( url );

    return *this;
}

/*!
    Assigns the data of \a url to this class.
*/

TQUrl& TQUrl::operator=( const TQUrl& url )
{
    *d = *url.d;
    return *this;
}

/*!
    Compares this URL with \a url and returns TRUE if they are equal;
    otherwise returns FALSE.
*/

bool TQUrl::operator==( const TQUrl& url ) const
{
    if ( !isValid() || !url.isValid() )
	return FALSE;

    if ( d->protocol == url.d->protocol &&
	 d->user == url.d->user &&
	 d->pass == url.d->pass &&
	 d->host == url.d->host &&
	 d->path == url.d->path &&
	 d->queryEncoded == url.d->queryEncoded &&
	 d->refEncoded == url.d->refEncoded &&
	 d->isValid == url.d->isValid &&
	 d->port == url.d->port )
	return TRUE;

    return FALSE;
}

/*!
    \overload

    Compares this URL with \a url. \a url is parsed first. Returns
    TRUE if \a url is equal to this url; otherwise returns FALSE.
*/

bool TQUrl::operator==( const TQString& url ) const
{
    TQUrl u( url );
    return ( *this == u );
}

/*!
    Sets the file name of the URL to \a name. If this URL contains a
    fileName(), the original file name is replaced by \a name.

    See the documentation of fileName() for a more detailed discussion
    of what is handled as file name and what is handled as a directory
    path.

    \sa fileName()
*/

void TQUrl::setFileName( const TQString& name )
{
    TQString fn( name );
    slashify( fn );

    while ( fn[ 0 ] == '/' )
	fn.remove( (uint)0, 1 );

    TQString p;
    if ( path().isEmpty() ) {
	p = "/";
    } else {
	p = path();
	int slash = p.findRev( TQChar( '/' ) );
	if ( slash == -1 ) {
	    p = "/";
	} else if ( p[ (int)p.length() - 1 ] != '/' ) {
	    p.truncate( slash + 1 );
	}
    }

    p += fn;
    if ( !d->queryEncoded.isEmpty() )
	p += "?" + d->queryEncoded;
    setEncodedPathAndQuery( p );
}

/*!
    Returns the encoded path and query.

    \sa decode()
*/

TQString TQUrl::encodedPathAndQuery()
{
    TQString p = path();
    if ( p.isEmpty() )
	p = "/";

    encode( p );

    if ( !d->queryEncoded.isEmpty() ) {
	p += "?";
	p += d->queryEncoded;
    }

    return p;
}

/*!
    Parses \a pathAndQuery for a path and query and sets those values.
    The whole string must be encoded.

    \sa encode()
*/

void TQUrl::setEncodedPathAndQuery( const TQString& pathAndQuery )
{
    d->cleanPathDirty = TRUE;
    int pos = pathAndQuery.find( '?' );
    if ( pos == -1 ) {
	d->path = pathAndQuery;
	d->queryEncoded = "";
    } else {
	d->path = pathAndQuery.left( pos );
	d->queryEncoded = pathAndQuery.mid( pos + 1 );
    }

    decode( d->path );
    d->cleanPathDirty = TRUE;
}

extern bool tqt_resolve_symlinks; // defined in qapplication.cpp

/*!
    Returns the path of the URL. If \a correct is TRUE, the path is
    cleaned (deals with too many or too few slashes, cleans things
    like "/../..", etc). Otherwise path() returns exactly the path
    that was parsed or set.

    \sa setPath() hasPath()
*/
TQString TQUrl::path( bool correct ) const
{
    if ( !correct )
	return d->path;

    if ( d->cleanPathDirty ) {
	bool check = TRUE;
	if ( TQDir::isRelativePath( d->path ) ) {
	    d->cleanPath = d->path;
	} else if ( isLocalFile() ) {
#if defined(Q_OS_WIN32)
	    // hack for stuff like \\machine\path and //machine/path on windows
	    if ( ( d->path.left( 1 ) == "/" || d->path.left( 1 ) == "\\" ) &&
		 d->path.length() > 1 ) {
		d->cleanPath = d->path;
		bool share = (d->cleanPath[0] == '\\' && d->cleanPath[1] == '\\') ||
		             (d->cleanPath[0] == '/' && d->cleanPath[1] == '/');
		slashify( d->cleanPath, FALSE );
		d->cleanPath = TQDir::cleanDirPath( d->cleanPath );
		if ( share ) {
		    check = FALSE;
		    while (d->cleanPath.at(0) != '/' || d->cleanPath.at(1) != '/')
			d->cleanPath.prepend("/");
		}
	    }
#endif
	    if ( check ) {
		TQFileInfo fi( d->path );
		if ( !fi.exists() )
		    d->cleanPath = d->path;
		else if ( fi.isDir() ) {
                    TQString canPath = TQDir( d->path ).canonicalPath();
                    TQString dir;
                    if ( tqt_resolve_symlinks && !canPath.isNull() )
			dir = TQDir::cleanDirPath( canPath );
                    else
			dir = TQDir::cleanDirPath( TQDir( d->path ).absPath() );
		    dir += "/";
		    if ( dir == "//" )
			d->cleanPath = "/";
		    else
			d->cleanPath = dir;
		} else {
		    TQString p =
			TQDir::cleanDirPath( (tqt_resolve_symlinks ?
					    fi.dir().canonicalPath() :
					    fi.dir().absPath()) );
		    d->cleanPath = p + "/" + fi.fileName();
		}
	    }
	} else {
	    if ( d->path != "/" && d->path[ (int)d->path.length() - 1 ] == '/' )
		d->cleanPath = TQDir::cleanDirPath( d->path ) + "/";
	    else
		d->cleanPath = TQDir::cleanDirPath( d->path );
	}

	if ( check )
	    slashify( d->cleanPath, FALSE );
	d->cleanPathDirty = FALSE;
    }

    return d->cleanPath;
}

/*!
    Returns TRUE if the URL is a local file; otherwise returns FALSE.
*/

bool TQUrl::isLocalFile() const
{
    return d->protocol == "file";
}

/*!
    Returns the file name of the URL. If the path of the URL doesn't
    have a slash at the end, the part between the last slash and the
    end of the path string is considered to be the file name. If the
    path has a slash at the end, an empty string is returned here.

    \sa setFileName()
*/

TQString TQUrl::fileName() const
{
    if ( d->path.isEmpty() || d->path.endsWith( "/" )
#ifdef TQ_WS_WIN
	|| d->path.endsWith( "\\" )
#endif
	)
	return TQString::null;

    return TQFileInfo( d->path ).fileName();
}

/*!
    Adds the path \a pa to the path of the URL.

    \sa setPath() hasPath()
*/

void TQUrl::addPath( const TQString& pa )
{
    if ( pa.isEmpty() )
	return;

    TQString p( pa );
    slashify( p );

    if ( path().isEmpty() ) {
	if ( p[ 0 ] != TQChar( '/' ) )
	    d->path = "/" + p;
	else
	    d->path = p;
    } else {
	if ( p[ 0 ] != TQChar( '/' ) && d->path[ (int)d->path.length() - 1 ] != '/' )
	    d->path += "/" + p;
	else
	    d->path += p;
    }
    d->cleanPathDirty = TRUE;
}

/*!
    Returns the directory path of the URL. This is the part of the
    path of the URL without the fileName(). See the documentation of
    fileName() for a discussion of what is handled as file name and
    what is handled as directory path.

    \sa setPath() hasPath()
*/

TQString TQUrl::dirPath() const
{
    if ( path().isEmpty() )
	return TQString::null;

    TQString s = path();
    int pos = s.findRev( '/' );
    if ( pos == -1 ) {
	return TQString::fromLatin1(".");
    } else {
	if ( pos == 0 )
	    return TQString::fromLatin1( "/" );
	return s.left( pos );
    }
}

/*!
    Encodes the \a url in-place into UTF-8.  For example

    \code
	TQString url = http://www.trolltech.com
	TQUrl::encode( url );
	// url is now "http%3A//www%20trolltech%20com"
    \endcode

  \sa decode()
*/

void TQUrl::encode( TQString& url )
{
    if ( url.isEmpty() )
	return;

    TQCString curl = url.utf8();
    int oldlen = curl.length();

    const TQCString special( "+<>#@\"&%$:,;?={}|^~[]\'`\\ \n\t\r" );
    TQString newUrl;
    int newlen = 0;

    for ( int i = 0; i < oldlen ;++i ) {
	uchar inCh = (uchar)curl[ i ];

	if ( inCh >= 128 || special.contains(inCh) ) {
	    newUrl[ newlen++ ] = TQChar( '%' );

	    ushort c = inCh / 16;
	    c += c > 9 ? 'A' - 10 : '0';
	    newUrl[ newlen++ ] = c;

	    c = inCh % 16;
	    c += c > 9 ? 'A' - 10 : '0';
	    newUrl[ newlen++ ] = c;
	} else {
	    newUrl[ newlen++ ] = inCh;
	}
    }

    url = newUrl;
}

static uchar hex_to_int( uchar c )
{
    if ( c >= 'A' && c <= 'F' )
	return c - 'A' + 10;
    if ( c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    if ( c >= '0' && c <= '9')
	return c - '0';
    return 0;
}

/*!
    Decodes the \a url in-place into UTF-8.  For example

    \code
	TQString url = "http%3A//www%20trolltech%20com"
	TQUrl::decode( url );
	// url is now "http://www.trolltech.com"
    \endcode

    \sa encode()
*/

void TQUrl::decode( TQString& url )
{
    if ( url.isEmpty() )
	return;

    int newlen = 0;
    TQCString curl = url.utf8();
    int oldlen = curl.length();

    TQCString newUrl(oldlen);

    int i = 0;
    while ( i < oldlen ) {
	uchar c = (uchar)curl[ i++ ];
	if ( c == '%' && i <= oldlen - 2 ) {
	    c = hex_to_int( (uchar)curl[ i ] ) * 16 + hex_to_int( (uchar)curl[ i + 1 ] );
	    i += 2;
	}
	newUrl [ newlen++ ] = c;
    }
    newUrl.truncate( newlen );

    url = TQString::fromUtf8(newUrl.data());
}


/*!
    Composes a string version of the URL and returns it. If \a
    encodedPath is TRUE the path in the returned string is encoded. If
    \a forcePrependProtocol is TRUE and \a encodedPath looks like a
    local filename, the "file:/" protocol is also prepended.

    \sa encode() decode()
*/

TQString TQUrl::toString( bool encodedPath, bool forcePrependProtocol ) const
{
    TQString res, p = path();
    if ( encodedPath )
	encode( p );

    if ( isLocalFile() ) {
	if ( forcePrependProtocol )
	    res = d->protocol + ":" + p;
	else
	    res = p;
    } else if ( d->protocol == "mailto" ) {
	res = d->protocol + ":" + p;
    } else {
	res = d->protocol + "://";
	if ( !d->user.isEmpty() || !d->pass.isEmpty() ) {
	    TQString tmp;
	    if ( !d->user.isEmpty() ) {
		tmp = d->user;
		encode( tmp );
		res += tmp;
	    }
	    if ( !d->pass.isEmpty() ) {
		tmp = d->pass;
		encode( tmp );
		res += ":" + tmp;
	    }
	    res += "@";
	}
	res += d->host;
	if ( d->port != -1 )
	    res += ":" + TQString( "%1" ).arg( d->port );
	if ( !p.isEmpty() ) {
	    if ( !d->host.isEmpty() && p[0]!='/' )
		res += "/";
	    res += p;
	}
    }

    if ( !d->refEncoded.isEmpty() )
	res += "#" + d->refEncoded;
    if ( !d->queryEncoded.isEmpty() )
	res += "?" + d->queryEncoded;

    return res;
}

/*!
    Composes a string version of the URL and returns it.

    \sa TQUrl::toString()
*/

TQUrl::operator TQString() const
{
    return toString();
}

/*!
    Changes the directory to one directory up.

    \sa setPath()
*/

bool TQUrl::cdUp()
{
    d->path += "/..";
    d->cleanPathDirty = TRUE;
    return TRUE;
}

#endif // TQT_NO_URL
