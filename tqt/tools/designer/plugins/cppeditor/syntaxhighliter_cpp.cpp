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

#include "syntaxhighliter_cpp.h"
#include "paragdata.h"
#include "ntqstring.h"
#include "ntqstringlist.h"
#include "ntqmap.h"
#include "ntqapplication.h"
#include "ntqregexp.h"

const char * const SyntaxHighlighter_CPP::keywords[] = {
    // C++ keywords
    "and",
    "and_eq",
    "asm",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "class",
    "compl",
    "const",
    "const_cast",
    "continue",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "FALSE",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "not",
    "not_eq",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "register",
    "reinterpret_cast",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_cast",
    "struct",
    "switch",
    "template",
    "this",
    "throw",
    "true",
    "TRUE",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
    // additional "keywords" intoduced by TQt
    "slots",
    "signals",
    "uint",
    "ushort",
    "ulong",
    "emit",
    // end of array
    0
};

static TQMap<int, TQMap<TQString, int > > *wordMap = 0;

SyntaxHighlighter_CPP::SyntaxHighlighter_CPP()
    : TQTextPreProcessor(), lastFormat( 0 ), lastFormatId( -1 )
{
    TQFont f( tqApp->font() );

    addFormat( Standard, new TQTextFormat( f, TQt::black ) );
    addFormat( Number, new TQTextFormat( f, TQt::darkBlue ) );
    addFormat( String, new TQTextFormat( f, TQt::darkGreen ) );
    addFormat( Type, new TQTextFormat( f, TQt::darkMagenta ) );
    addFormat( Keyword, new TQTextFormat( f, TQt::darkYellow ) );
    addFormat( PreProcessor, new TQTextFormat( f, TQt::darkBlue ) );
    addFormat( Label, new TQTextFormat( f, TQt::darkRed ) );
    f.setFamily( "times" );
    addFormat( Comment, new TQTextFormat( f, TQt::red ) );

    if ( wordMap )
	return;

    wordMap = new TQMap<int, TQMap<TQString, int> >;
    int len;
    for ( int i = 0; keywords[ i ]; ++i ) {
	len = (int)strlen( keywords[ i ] );
	if ( !wordMap->contains( len ) )
	    wordMap->insert( len, TQMap<TQString, int >() );
	TQMap<TQString, int> &map = wordMap->operator[]( len );
	map[ keywords[ i ] ] = Keyword;
    }
}

SyntaxHighlighter_CPP::~SyntaxHighlighter_CPP()
{
}

static int string2Id( const TQString &s )
{
    if ( s == "Standard" )
	return SyntaxHighlighter_CPP::Standard;
    if ( s == "Comment" )
	return SyntaxHighlighter_CPP::Comment;
    if ( s == "Number" )
	return SyntaxHighlighter_CPP::Number;
    if ( s == "String" )
	return SyntaxHighlighter_CPP::String;
    if ( s == "Type" )
	return SyntaxHighlighter_CPP::Type;
    if ( s == "Preprocessor" )
	return SyntaxHighlighter_CPP::PreProcessor;
    if ( s == "Label" )
	return SyntaxHighlighter_CPP::Label;
    if ( s == "Keyword" )
	return SyntaxHighlighter_CPP::Keyword;
    return SyntaxHighlighter_CPP::Standard;
}

void SyntaxHighlighter_CPP::updateStyles( const TQMap<TQString, ConfigStyle> &styles )
{
    for ( TQMap<TQString, ConfigStyle>::ConstIterator it = styles.begin(); it != styles.end(); ++it ) {
	int id = string2Id( it.key() );
	TQTextFormat *f = format( id );
	if ( !f )
	    continue;
	f->setFont( (*it).font );
	f->setColor( (*it).color );
    }
}

void SyntaxHighlighter_CPP::process( TQTextDocument *doc, TQTextParagraph *string, int, bool invalidate )
{

    TQTextFormat *formatStandard = format( Standard );
    TQTextFormat *formatComment = format( Comment );
    TQTextFormat *formatNumber = format( Number );
    TQTextFormat *formatString = format( String );
    TQTextFormat *formatType = format( Type );
    TQTextFormat *formatPreProcessor = format( PreProcessor );
    TQTextFormat *formatLabel = format( Label );

    // states
    const int StateStandard = 0;
    const int StateCommentStart1 = 1;
    const int StateCCommentStart2 = 2;
    const int StateCppCommentStart2 = 3;
    const int StateCComment = 4;
    const int StateCppComment = 5;
    const int StateCCommentEnd1 = 6;
    const int StateCCommentEnd2 = 7;
    const int StateStringStart = 8;
    const int StateString = 9;
    const int StateStringEnd = 10;
    const int StateString2Start = 11;
    const int StateString2 = 12;
    const int StateString2End = 13;
    const int StateNumber = 14;
    const int StatePreProcessor = 15;

    // tokens
    const int InputAlpha = 0;
    const int InputNumber = 1;
    const int InputAsterix = 2;
    const int InputSlash = 3;
    const int InputParen = 4;
    const int InputSpace = 5;
    const int InputHash = 6;
    const int InputQuotation = 7;
    const int InputApostrophe = 8;
    const int InputSep = 9;

    static uchar table[ 16 ][ 10 ] = {
	{ StateStandard,      StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateStandard
	{ StateStandard,      StateNumber,   StateCCommentStart2, StateCppCommentStart2, StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateCommentStart1
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCComment,         StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCCommentStart2
	{ StateCppComment,    StateCppComment, StateCppComment,     StateCppComment,       StateCppComment, StateCppComment, StateCppComment,   StateCppComment,  StateCppComment,   StateCppComment }, // CppCommentStart2
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCComment,         StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCComment
	{ StateCppComment,    StateCppComment, StateCppComment,     StateCppComment,       StateCppComment, StateCppComment, StateCppComment,   StateCppComment,  StateCppComment,   StateCppComment }, // StateCppComment
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCCommentEnd2,     StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCCommentEnd1
	{ StateStandard,      StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateCCommentEnd2
	{ StateString,        StateString,     StateString,         StateString,           StateString,     StateString,     StateString,       StateStringEnd,   StateString,       StateString }, // StateStringStart
	{ StateString,        StateString,     StateString,         StateString,           StateString,     StateString,     StateString,       StateStringEnd,   StateString,       StateString }, // StateString
	{ StateStandard,      StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateStringEnd
	{ StateString2,       StateString2,    StateString2,        StateString2,          StateString2,    StateString2,    StateString2,      StateString2,     StateString2End,   StateString2 }, // StateString2Start
	{ StateString2,       StateString2,    StateString2,        StateString2,          StateString2,    StateString2,    StateString2,      StateString2,     StateString2End,   StateString2 }, // StateString2
	{ StateStandard,      StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateString2End
	{ StateNumber,        StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateNumber
	{ StatePreProcessor,  StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard } // StatePreProcessor
    };

    TQString buffer;

    int state = StateStandard;
    if ( string->prev() ) {
	if ( string->prev()->endState() == -1 )
	    process( doc, string->prev(), 0, FALSE );
	state = string->prev()->endState();
    }
    int input = -1;
    int i = 0;
    bool lastWasBackSlash = FALSE;
    bool makeLastStandard = FALSE;

    ParagData *paragData = (ParagData*)string->extraData();
    if ( paragData )
	paragData->parenList.clear();
    else
	paragData = new ParagData;

    TQString alphabeth = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    TQString mathChars = "xXeE";
    TQString numbers = "0123456789";
    bool questionMark = FALSE;
    TQChar lastChar;
    for (;;) {
	TQChar c = string->at( i )->c;

	if ( lastWasBackSlash ) {
	    input = InputSep;
	} else {
	    switch ( c ) {
	    case '*':
		input = InputAsterix;
		break;
	    case '/':
		input = InputSlash;
		break;
	    case '(': case '[': case '{':
		input = InputParen;
		if ( state == StateStandard ||
		     state == StateNumber ||
		     state == StatePreProcessor ||
		     state == StateCCommentEnd2 ||
		     state == StateCCommentEnd1 ||
		     state == StateString2End ||
		     state == StateStringEnd )
		    paragData->parenList << Paren( Paren::Open, c, i );
		break;
	    case ')': case ']': case '}':
		input = InputParen;
		if ( state == StateStandard ||
		     state == StateNumber ||
		     state == StatePreProcessor ||
		     state == StateCCommentEnd2 ||
		     state == StateCCommentEnd1 ||
		     state == StateString2End ||
		     state == StateStringEnd )
		    paragData->parenList << Paren( Paren::Closed, c, i );
		break;
	    case '#':
		input = InputHash;
		break;
	    case '"':
		input = InputQuotation;
		break;
	    case '\'':
		input = InputApostrophe;
		break;
	    case ' ':
		input = InputSpace;
		break;
	    case '1': case '2': case '3': case '4': case '5':
	    case '6': case '7': case '8': case '9': case '0':
		if ( alphabeth.find( lastChar ) != -1 &&
		     ( mathChars.find( lastChar ) == -1 || numbers.find( string->at( i - 1 )->c ) == -1 ) ) {
		    input = InputAlpha;
		} else {
		    if ( input == InputAlpha && numbers.find( lastChar ) != -1 )
			input = InputAlpha;
		    else
			input = InputNumber;
		}
		break;
	    case ':': {
		input = InputAlpha;
		TQChar nextChar = ' ';
		if ( i < string->length() - 1 )
		    nextChar = string->at( i + 1 )->c;
		if ( state == StateStandard && !questionMark && lastChar != ':' && nextChar != ':' ) {
		    for ( int j = 0; j < i; ++j ) {
			if ( string->at( j )->format() == formatStandard )
			    string->setFormat( j, 1, formatLabel, FALSE );
		    }
		}
	    } break;
	    default: {
		if ( !questionMark && c == '?' )
		    questionMark = TRUE;
		if ( c.isLetter() || c == '_' )
		    input = InputAlpha;
		else
		    input = InputSep;
	    } break;
	    }
	}

	lastWasBackSlash = !lastWasBackSlash && c == '\\';

	if ( input == InputAlpha )
	    buffer += c;

    	state = table[ state ][ input ];

	switch ( state ) {
	case StateStandard: {
	    int len = buffer.length();
	    string->setFormat( i, 1, formatStandard, FALSE );
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    if ( buffer.length() > 0 && input != InputAlpha ) {
		if ( buffer[ 0 ] == 'Q' ) {
		    string->setFormat( i - buffer.length(), buffer.length(), formatType, FALSE );
		} else {
		    TQMap<int, TQMap<TQString, int > >::Iterator it = wordMap->find( len );
		    if ( it != wordMap->end() ) {
			TQMap<TQString, int >::Iterator it2 = ( *it ).find( buffer );
			if ( it2 != ( *it ).end() )
			    string->setFormat( i - buffer.length(), buffer.length(), format( ( *it2 ) ), FALSE );
		    }
		}
		buffer = TQString::null;
	    }
	} break;
	case StateCommentStart1:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = TRUE;
	    buffer = TQString::null;
	    break;
	case StateCCommentStart2:
	    string->setFormat( i - 1, 2, formatComment, FALSE );
	    makeLastStandard = FALSE;
	    buffer = TQString::null;
	    break;
	case StateCppCommentStart2:
	    string->setFormat( i - 1, 2, formatComment, FALSE );
	    makeLastStandard = FALSE;
	    buffer = TQString::null;
	    break;
	case StateCComment:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = TQString::null;
	    break;
	case StateCppComment:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = TQString::null;
	    break;
	case StateCCommentEnd1:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = TQString::null;
	    break;
	case StateCCommentEnd2:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = TQString::null;
	    break;
	case StateStringStart:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = TQString::null;
	    break;
	case StateString:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatString, FALSE );
	    buffer = TQString::null;
	    break;
	case StateStringEnd:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = TQString::null;
	    break;
	case StateString2Start:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = TQString::null;
	    break;
	case StateString2:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatString, FALSE );
	    buffer = TQString::null;
	    break;
	case StateString2End:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = TQString::null;
	    break;
	case StateNumber:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatNumber, FALSE );
	    buffer = TQString::null;
	    break;
	case StatePreProcessor:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatPreProcessor, FALSE );
	    buffer = TQString::null;
	    break;
	}

	lastChar = c;
	i++;
	if ( i >= string->length() )
	    break;
    }

    string->setExtraData( paragData );

    int oldEndState = string->endState();
    if ( state == StateCComment ||
	 state == StateCCommentEnd1 ) {
	string->setEndState( StateCComment );
    } else if ( state == StateString ) {
	string->setEndState( StateString );
    } else if ( state == StateString2 ) {
	string->setEndState( StateString2 );
    } else {
	string->setEndState( StateStandard );
    }

    string->setFirstPreProcess( FALSE );

    TQTextParagraph *p = string->next();
    if ( (!!oldEndState || !!string->endState()) && oldEndState != string->endState() &&
	 invalidate && p && !p->firstPreProcess() && p->endState() != -1 ) {
	while ( p ) {
	    if ( p->endState() == -1 )
		return;
	    p->setEndState( -1 );
	    p = p->next();
	}
    }
}

TQTextFormat *SyntaxHighlighter_CPP::format( int id )
{
    if ( lastFormatId == id  && lastFormat )
	return lastFormat;

    TQTextFormat *f = formats[ id ];
    lastFormat = f ? f : formats[ 0 ];
    lastFormatId = id;
    return lastFormat;
}

void SyntaxHighlighter_CPP::addFormat( int id, TQTextFormat *f )
{
    formats.insert( id, f );
}

void SyntaxHighlighter_CPP::removeFormat( int id )
{
    formats.remove( id );
}

