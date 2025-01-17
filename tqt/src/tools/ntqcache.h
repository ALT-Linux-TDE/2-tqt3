/****************************************************************************
**
** Definition of TQCache template class
**
** Created : 950209
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

#ifndef TQCACHE_H
#define TQCACHE_H

#ifndef QT_H
#include "ntqgcache.h"
#endif // QT_H

template<class type>
class TQCache
#ifdef Q_QDOC
	: public TQPtrCollection
#else
	: public TQGCache
#endif
{
public:
    TQCache( const TQCache<type> &c ) : TQGCache(c) {}
    TQCache( int maxCost=100, int size=17, bool caseSensitive=TRUE )
	: TQGCache( maxCost, size, StringKey, caseSensitive, FALSE ) {}
   ~TQCache()				{ clear(); }
    TQCache<type> &operator=( const TQCache<type> &c )
			{ return (TQCache<type>&)TQGCache::operator=(c); }
    int	  maxCost()   const		{ return TQGCache::maxCost(); }
    int	  totalCost() const		{ return TQGCache::totalCost(); }
    void  setMaxCost( int m )		{ TQGCache::setMaxCost(m); }
    uint  count()     const		{ return TQGCache::count(); }
    uint  size()      const		{ return TQGCache::size(); }
    bool  isEmpty()   const		{ return TQGCache::count() == 0; }
    void  clear()			{ TQGCache::clear(); }
    bool  insert( const TQString &k, const type *d, int c=1, int p=0 )
			{ return TQGCache::insert_string(k,(Item)d,c,p);}
    bool  remove( const TQString &k )
			{ return TQGCache::remove_string(k); }
    type *take( const TQString &k )
			{ return (type *)TQGCache::take_string(k); }
    type *find( const TQString &k, bool ref=TRUE ) const
			{ return (type *)TQGCache::find_string(k,ref);}
    type *operator[]( const TQString &k ) const
			{ return (type *)TQGCache::find_string(k);}
    void  statistics() const	      { TQGCache::statistics(); }
private:
    void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void TQCache<void>::deleteItem( TQPtrCollection::Item )
{
}
#endif

template<class type> inline void TQCache<type>::deleteItem( TQPtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

template<class type>
class TQCacheIterator : public TQGCacheIterator
{
public:
    TQCacheIterator( const TQCache<type> &c ):TQGCacheIterator((TQGCache &)c) {}
    TQCacheIterator( const TQCacheIterator<type> &ci)
				: TQGCacheIterator( (TQGCacheIterator &)ci ) {}
    TQCacheIterator<type> &operator=(const TQCacheIterator<type>&ci)
	{ return ( TQCacheIterator<type>&)TQGCacheIterator::operator=( ci ); }
    uint  count()   const     { return TQGCacheIterator::count(); }
    bool  isEmpty() const     { return TQGCacheIterator::count() == 0; }
    bool  atFirst() const     { return TQGCacheIterator::atFirst(); }
    bool  atLast()  const     { return TQGCacheIterator::atLast(); }
    type *toFirst()	      { return (type *)TQGCacheIterator::toFirst(); }
    type *toLast()	      { return (type *)TQGCacheIterator::toLast(); }
    operator type *() const   { return (type *)TQGCacheIterator::get(); }
    type *current()   const   { return (type *)TQGCacheIterator::get(); }
    TQString currentKey() const{ return TQGCacheIterator::getKeyString(); }
    type *operator()()	      { return (type *)TQGCacheIterator::operator()();}
    type *operator++()	      { return (type *)TQGCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)TQGCacheIterator::operator+=(j);}
    type *operator--()	      { return (type *)TQGCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)TQGCacheIterator::operator-=(j);}
};

#endif // TQCACHE_H
