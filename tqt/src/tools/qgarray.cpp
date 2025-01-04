/****************************************************************************
**
** Implementation of TQGArray class
**
** Created : 930906
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the tools module of the TQt GUI Toolkit.
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

#include "ntqglobal.h"
#if defined(Q_CC_BOR)
    // needed for qsort() because of a std namespace problem on Borland
#   include "qplatformdefs.h"
#elif defined(TQ_WS_WIN)
    // needed for bsearch on some platforms
#   include "qt_windows.h"
#endif

#define	 TQGARRAY_CPP
#include "ntqgarray.h"
#include <stdlib.h>
#include <string.h>

#ifdef TQT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // TQT_THREAD_SUPPORT

/*
  If USE_MALLOC isn't defined, we use new[] and delete[] to allocate
  memory. The documentation for TQMemArray<T>::assign() explicitly
  mentions that the array is freed using free(), so don't mess around
  with USE_MALLOC unless you know what you're doing.
*/
#define USE_MALLOC

#undef NEW
#undef DELETE

#if defined(USE_MALLOC)
#define NEW(type,size)	((type*)malloc(size*sizeof(type)))
#define DELETE(array)	(free((char*)array))
#else
#define NEW(type,size)	(new type[size])
#define DELETE(array)	(delete[] array)
#define DONT_USE_REALLOC			// comment to use realloc()
#endif

/*!
  \class TQShared ntqshared.h
  \reentrant
  \ingroup shared
  \brief The TQShared class is used internally for implementing shared classes.

  \internal

  It only contains a reference count and member functions to increment and
  decrement it.

  Shared classes normally have internal classes that inherit TQShared and
  add the shared data.

  \sa \link shclass.html Shared Classes\endlink
*/

/*!
  \class TQGArray ntqgarray.h
  \reentrant
  \ingroup shared
  \ingroup collection
  \brief The TQGArray class is an internal class for implementing the TQMemArray class.

  \internal

  TQGArray is a strictly internal class that acts as base class for the
  TQMemArray template array.

  It contains an array of bytes and has no notion of an array element.
*/


/*!
  Constructs a null array.
*/

TQGArray::TQGArray()
{
    shd = newData();
    TQ_CHECK_PTR( shd );
}

/*!
  Dummy constructor; does not allocate any data.

  This constructor does not initialize any array data so subclasses
  must do it. The intention is to make the code more efficient.
*/

TQGArray::TQGArray( int, int )
{
}

/*!
  Constructs an array with room for \a size bytes.
*/

TQGArray::TQGArray( int size )
{
    if ( size < 0 ) {
#if defined(QT_CHECK_RANGE)
	tqWarning( "TQGArray: Cannot allocate array with negative length" );
#endif
	size = 0;
    }
    shd = newData();
    TQ_CHECK_PTR( shd );
    if ( size == 0 )				// zero length
	return;
    shd->data = NEW(char,size);
    TQ_CHECK_PTR( shd->data );
    shd->len =
#ifdef QT_QGARRAY_SPEED_OPTIM
	shd->maxl =
#endif
	size;
}

/*!
  Constructs a shallow copy of \a a.
*/

TQGArray::TQGArray( const TQGArray &a )
{
    shd = a.shd;
    shd->ref();
}

/*!
  Dereferences the array data and deletes it if this was the last
  reference.
*/

TQGArray::~TQGArray()
{
    if ( shd && shd->deref() ) {		// delete when last reference
	if ( shd->data )			// is lost
	    DELETE(shd->data);
	deleteData( shd );
	shd = 0;
    }
}


/*!
  \fn TQGArray &TQGArray::operator=( const TQGArray &a )

  Assigns a shallow copy of \a a to this array and returns a reference to
  this array.  Equivalent to assign().
*/

/*!
  \fn void TQGArray::detach()

  Detaches this array from shared array data.
*/

/*!
  \fn char *TQGArray::data() const

  Returns a pointer to the actual array data.
*/

/*!
  \fn uint TQGArray::nrefs() const

  Returns the reference count.
*/

/*!
  \fn uint TQGArray::size() const

  Returns the size of the array, in bytes.
*/


/*!
  Returns TRUE if this array is equal to \a a, otherwise FALSE.
  The comparison is bitwise, of course.
*/

bool TQGArray::isEqual( const TQGArray &a ) const
{
    if ( size() != a.size() )			// different size
	return FALSE;
    if ( data() == a.data() )			// has same data
	return TRUE;
    return (size() ? memcmp( data(), a.data(), size() ) : 0) == 0;
}


/*!
  Resizes the array to \a newsize bytes. \a optim is either
  \c MemOptim (the default) or \c SpeedOptim.

  <b>Note:</b> \c SpeedOptim is only available if TQt is built in a
  particular configuration. By default, \c SpeedOptim is not available
  for general use.
*/
bool TQGArray::resize( uint newsize, Optimization optim )
{
#ifndef QT_QGARRAY_SPEED_OPTIM
    Q_UNUSED(optim);
#endif

    if ( newsize == shd->len
#ifdef QT_QGARRAY_SPEED_OPTIM
	 && newsize == shd->maxl
#endif
	) // nothing to do
	return TRUE;
    if ( newsize == 0 ) {			// remove array
	if ( shd->data )
	    DELETE(shd->data);
	shd->data = 0;
	shd->len = 0;
#ifdef QT_QGARRAY_SPEED_OPTIM
	shd->maxl = 0;
#endif
	return TRUE;
    }

    uint newmaxl = newsize;
#ifdef QT_QGARRAY_SPEED_OPTIM
    if ( optim == SpeedOptim ) {
	if ( newsize <= shd->maxl &&
	     ( newsize * 4 > shd->maxl || shd->maxl <= 4 ) ) {
	    shd->len = newsize;
	    return TRUE;
	}
	newmaxl = 4;
	while ( newmaxl < newsize )
	    newmaxl *= 2;
	// try to spare some memory
	if ( newmaxl >= 1024 * 1024 && newsize <= newmaxl - (newmaxl >> 2) )
	    newmaxl -= newmaxl >> 2;
    }
    shd->maxl = newmaxl;
#endif

    if ( shd->data ) {				// existing data
#if defined(DONT_USE_REALLOC)
	char *newdata = NEW(char,newsize);	// manual realloc
	memcpy( newdata, shd->data, TQMIN(shd->len,newmaxl) );
	DELETE(shd->data);
	shd->data = newdata;
#else
	shd->data = (char *)realloc( shd->data, newmaxl );
#endif
    } else {
	shd->data = NEW(char,newmaxl);
    }
    if ( !shd->data )				// no memory
	return FALSE;
    shd->len = newsize;
    return TRUE;
}

/*!\overload
*/
bool TQGArray::resize( uint newsize )
{
    return resize( newsize, MemOptim );
}


/*!
  Fills the array with the repeated occurrences of \a d, which is
  \a sz bytes long.
  If \a len is specified as different from -1, then the array will be
  resized to \a len*sz before it is filled.

  Returns TRUE if successful, or FALSE if the memory cannot be allocated
  (only when \a len != -1).

  \sa resize()
*/

bool TQGArray::fill( const char *d, int len, uint sz )
{
    if ( len < 0 )
	len = shd->len/sz;			// default: use array length
    else if ( !resize( len*sz ) )
	return FALSE;
    if ( sz == 1 )				// 8 bit elements
	memset( data(), *d, len );
    else if ( sz == 4 ) {			// 32 bit elements
	TQ_INT32 *x = (TQ_INT32*)data();
	TQ_INT32 v = *((TQ_INT32*)d);
	while ( len-- )
	    *x++ = v;
    } else if ( sz == 2 ) {			// 16 bit elements
	TQ_INT16 *x = (TQ_INT16*)data();
	TQ_INT16 v = *((TQ_INT16*)d);
	while ( len-- )
	    *x++ = v;
    } else {					// any other size elements
	char *x = data();
	while ( len-- ) {			// more complicated
	    memcpy( x, d, sz );
	    x += sz;
	}
    }
    return TRUE;
}

/*!
    \overload
  Shallow copy. Dereference the current array and references the data
  contained in \a a instead. Returns a reference to this array.
  \sa operator=()
*/

TQGArray &TQGArray::assign( const TQGArray &a )
{
    a.shd->ref();				// avoid 'a = a'
    if ( shd->deref() ) {			// delete when last reference
	if ( shd->data )			// is lost
	    DELETE(shd->data);
	deleteData( shd );
    }
    shd = a.shd;
    return *this;
}

/*!
  Shallow copy. Dereference the current array and references the
  array data \a d, which contains \a len bytes.
  Returns a reference to this array.

  Do not delete \a d later, because TQGArray takes care of that.
*/

TQGArray &TQGArray::assign( const char *d, uint len )
{
    if ( shd->count > 1 ) {			// disconnect this
	shd->count--;
	shd = newData();
	TQ_CHECK_PTR( shd );
    } else {
	if ( shd->data )
	    DELETE(shd->data);
    }
    shd->data = (char *)d;
    shd->len =
#ifdef QT_QGARRAY_SPEED_OPTIM
	shd->maxl =
#endif
	len;
    return *this;
}

/*!
  Deep copy. Dereference the current array and obtains a copy of the data
  contained in \a a instead. Returns a reference to this array.
  \sa assign(), operator=()
*/

TQGArray &TQGArray::duplicate( const TQGArray &a )
{
    if ( a.shd == shd ) {			// a.duplicate(a) !
	if ( shd->count > 1 ) {
	    shd->count--;
	    array_data *n = newData();
	    TQ_CHECK_PTR( n );
	    if ( (n->len=shd->len) ) {
		n->data = NEW(char,n->len);
		TQ_CHECK_PTR( n->data );
		if ( n->data )
		    memcpy( n->data, shd->data, n->len );
	    } else {
		n->data = 0;
	    }
	    shd = n;
	}
	return *this;
    }
    char *oldptr = 0;
    if ( shd->count > 1 ) {			// disconnect this
	shd->count--;
	shd = newData();
	TQ_CHECK_PTR( shd );
    } else {					// delete after copy was made
	oldptr = shd->data;
    }
    if ( a.shd->len ) {				// duplicate data
	shd->data = NEW(char,a.shd->len);
	TQ_CHECK_PTR( shd->data );
	if ( shd->data )
	    memcpy( shd->data, a.shd->data, a.shd->len );
    } else {
	shd->data = 0;
    }
    shd->len =
#ifdef QT_QGARRAY_SPEED_OPTIM
	shd->maxl =
#endif
	a.shd->len;
    if ( oldptr )
	DELETE(oldptr);
    return *this;
}

/*!
    \overload
  Deep copy. Dereferences the current array and obtains a copy of
  \a len characters from array data \a d instead.  Returns a reference
  to this array.
  \sa assign(), operator=()
*/

TQGArray &TQGArray::duplicate( const char *d, uint len )
{
    char *data;
    if ( d == 0 || len == 0 ) {
	data = 0;
	len  = 0;
    } else {
	if ( shd->count == 1 && shd->len == len ) {
	    if ( shd->data != d )		// avoid self-assignment
		memcpy( shd->data, d, len );	// use same buffer
	    return *this;
	}
	data = NEW(char,len);
	TQ_CHECK_PTR( data );
	memcpy( data, d, len );
    }
    if ( shd->count > 1 ) {			// detach
	shd->count--;
	shd = newData();
	TQ_CHECK_PTR( shd );
    } else {					// just a single reference
	if ( shd->data )
	    DELETE(shd->data);
    }
    shd->data = data;
    shd->len =
#ifdef QT_QGARRAY_SPEED_OPTIM
	shd->maxl =
#endif
	len;
    return *this;
}

/*!
  Resizes this array to \a len bytes and copies the \a len bytes at
  address \a d into it.

  \warning This function disregards the reference count mechanism.  If
  other TQGArrays reference the same data as this, all will be updated.
*/

void TQGArray::store( const char *d, uint len )
{						// store, but not deref
    resize( len );
    memcpy( shd->data, d, len );
}


/*!
  \fn array_data *TQGArray::sharedBlock() const

  Returns a pointer to the shared array block.

  \warning

  Do not use this function.  Using it is begging for trouble.  We dare
  not remove it, for fear of breaking code, but we \e strongly
  discourage new use of it.
*/

/*!
  \fn void TQGArray::setSharedBlock( array_data *p )

  Sets the shared array block to \a p.

  \warning

  Do not use this function.  Using it is begging for trouble.  We dare
  not remove it, for fear of breaking code, but we \e strongly
  discourage new use of it.
*/


/*!
  Sets raw data and returns a reference to the array.

  Dereferences the current array and sets the new array data to \a d and
  the new array size to \a len.	 Do not attempt to resize or re-assign the
  array data when raw data has been set.
  Call resetRawData(d,len) to reset the array.

  Setting raw data is useful because it sets TQMemArray data without
  allocating memory or copying data.

  Example of intended use:
  \code
    static uchar bindata[] = { 231, 1, 44, ... };
    TQByteArray	a;
    a.setRawData( bindata, sizeof(bindata) );	// a points to bindata
    TQDataStream s( a, IO_ReadOnly );		// open on a's data
    s >> <something>;				// read raw bindata
    s.close();
    a.resetRawData( bindata, sizeof(bindata) ); // finished
  \endcode

  Example of misuse (do not do this):
  \code
    static uchar bindata[] = { 231, 1, 44, ... };
    TQByteArray	a, b;
    a.setRawData( bindata, sizeof(bindata) );	// a points to bindata
    a.resize( 8 );				// will crash
    b = a;					// will crash
    a[2] = 123;					// might crash
      // forget to resetRawData - will crash
  \endcode

  \warning If you do not call resetRawData(), TQGArray will attempt to
  deallocate or reallocate the raw data, which might not be too good.
  Be careful.
*/

TQGArray &TQGArray::setRawData( const char *d, uint len )
{
    duplicate( 0, 0 );				// set null data
    shd->data = (char *)d;
    shd->len  = len;
    return *this;
}

/*!
  Resets raw data.

  The arguments must be the data, \a d, and length \a len, that were
  passed to setRawData().  This is for consistency checking.
*/

void TQGArray::resetRawData( const char *d, uint len )
{
    if ( d != shd->data || len != shd->len ) {
#if defined(QT_CHECK_STATE)
	tqWarning( "TQGArray::resetRawData: Inconsistent arguments" );
#endif
	return;
    }
    shd->data = 0;
    shd->len  = 0;
}


/*!
  Finds the first occurrence of \a d in the array from position \a index,
  where \a sz is the size of the \a d element.

  Note that \a index is given in units of \a sz, not bytes.

  This function only compares whole cells, not bytes.
*/

int TQGArray::find( const char *d, uint index, uint sz ) const
{
    index *= sz;
    if ( index >= shd->len ) {
#if defined(QT_CHECK_RANGE)
	tqWarning( "TQGArray::find: Index %d out of range", index/sz );
#endif
	return -1;
    }
    uint i;
    uint ii;
    switch ( sz ) {
	case 1: {				// 8 bit elements
	    char *x = data() + index;
	    char v = *d;
	    for ( i=index; i<shd->len; i++ ) {
		if ( *x++ == v )
		    break;
	    }
	    ii = i;
	    }
	    break;
	case 2: {				// 16 bit elements
	    TQ_INT16 *x = (TQ_INT16*)(data() + index);
	    TQ_INT16 v = *((TQ_INT16*)d);
	    for ( i=index; i<shd->len; i+=2 ) {
		if ( *x++ == v )
		    break;
	    }
	    ii = i/2;
	    }
	    break;
	case 4: {				// 32 bit elements
	    TQ_INT32 *x = (TQ_INT32*)(data() + index);
	    TQ_INT32 v = *((TQ_INT32*)d);
	    for ( i=index; i<shd->len; i+=4 ) {
		if ( *x++ == v )
		    break;
	    }
	    ii = i/4;
	    }
	    break;
	default: {				// any size elements
	    for ( i=index; i<shd->len; i+=sz ) {
		if ( memcmp( d, &shd->data[i], sz ) == 0 )
		    break;
	    }
	    ii = i/sz;
	    }
	    break;
    }
    return i<shd->len ? (int)ii : -1;
}

/*!
  Returns the number of occurrences of \a d in the array, where \a sz is
  the size of the \a d element.

  This function only compares whole cells, not bytes.
*/

int TQGArray::contains( const char *d, uint sz ) const
{
    uint i = shd->len;
    int count = 0;
    switch ( sz ) {
	case 1: {				// 8 bit elements
	    char *x = data();
	    char v = *d;
	    while ( i-- ) {
		if ( *x++ == v )
		    count++;
	    }
	    }
	    break;
	case 2: {				// 16 bit elements
	    TQ_INT16 *x = (TQ_INT16*)data();
	    TQ_INT16 v = *((TQ_INT16*)d);
	    i /= 2;
	    while ( i-- ) {
		if ( *x++ == v )
		    count++;
	    }
	    }
	    break;
	case 4: {				// 32 bit elements
	    TQ_INT32 *x = (TQ_INT32*)data();
	    TQ_INT32 v = *((TQ_INT32*)d);
	    i /= 4;
	    while ( i-- ) {
		if ( *x++ == v )
		    count++;
	    }
	    }
	    break;
	default: {				// any size elements
	    for ( i=0; i<shd->len; i+=sz ) {
		if ( memcmp(d, &shd->data[i], sz) == 0 )
		    count++;
	    }
	    }
	    break;
    }
    return count;
}

static int cmp_item_size = 0;

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#ifdef Q_OS_TEMP
static int __cdecl cmp_arr( const void *n1, const void *n2 )
#else
static int cmp_arr( const void *n1, const void *n2 )
#endif
{
    return ( n1 && n2 ) ? memcmp( n1, n2, cmp_item_size )
			: ( n1 ? 1 : ( n2 ? -1 : 0 ) );
    // ### TQt 3.0: Add a virtual compareItems() method and call that instead
}

#if defined(Q_C_CALLBACKS)
}
#endif

/*!
  Sorts the first \a sz items of the array.
*/

void TQGArray::sort( uint sz )
{
    int numItems = size() / sz;
    if ( numItems < 2 )
	return;

#ifdef TQT_THREAD_SUPPORT
    TQMutexLocker locker( tqt_global_mutexpool ?
			 tqt_global_mutexpool->get( &cmp_item_size ) : 0 );
#endif // TQT_THREAD_SUPPORT

    cmp_item_size = sz;
    qsort( shd->data, numItems, sz, cmp_arr );
}

/*!
  Binary search; assumes that \a d is a sorted array of size \a sz.
*/

int TQGArray::bsearch( const char *d, uint sz ) const
{
    int numItems = size() / sz;
    if ( !numItems )
	return -1;

#ifdef TQT_THREAD_SUPPORT
    TQMutexLocker locker( tqt_global_mutexpool ?
			 tqt_global_mutexpool->get( &cmp_item_size ) : 0 );
#endif // TQT_THREAD_SUPPORT

    cmp_item_size = sz;
    char* r = (char*)::bsearch( d, shd->data, numItems, sz, cmp_arr );
    if ( !r )
	return -1;
    while( (r >= shd->data + sz) && (cmp_arr( r - sz, d ) == 0) )
	r -= sz;	// search to first of equal elements; bsearch is undef
    return (int)(( r - shd->data ) / sz);
}


/*!
  \fn char *TQGArray::at( uint index ) const

  Returns a pointer to the byte at offset \a index in the array.
*/

/*!
  Expand the array if necessary, and copies (the first part of) its
  contents from the \a index * \a sz bytes at \a d.

  Returns TRUE if the operation succeeds, FALSE if it runs out of
  memory.

  \warning This function disregards the reference count mechanism.  If
  other TQGArrays reference the same data as this, all will be changed.
*/

bool TQGArray::setExpand( uint index, const char *d, uint sz )
{
    index *= sz;
    if ( index >= shd->len ) {
	if ( !resize( index+sz ) )		// no memory
	    return FALSE;
    }
    memcpy( data() + index, d, sz );
    return TRUE;
}


/*!
  Prints a warning message if at() or [] is given a bad index.
*/

void TQGArray::msg_index( uint index )
{
#if defined(QT_CHECK_RANGE)
    tqWarning( "TQGArray::at: Absolute index %d out of range", index );
#else
    Q_UNUSED( index )
#endif
}


/*!
  Returns a new shared array block.
*/

TQGArray::array_data * TQGArray::newData()
{
    return new array_data;
}


/*!
  Deletes the shared array block \a p.
*/

void TQGArray::deleteData( array_data *p )
{
    delete p;
}