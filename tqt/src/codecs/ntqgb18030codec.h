/****************************************************************************
**
** Definition of TQGb18030Codec class
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

// Contributed by James Su <suzhe@gnuchina.org>

#ifndef TQGB18030CODEC_H
#define TQGB18030CODEC_H

#ifndef QT_H
#include "ntqtextcodec.h"
#endif // QT_H


#ifndef TQT_NO_BIG_CODECS

#if defined(QT_PLUGIN)
#define Q_EXPORT_CODECS_CN
#else
#define Q_EXPORT_CODECS_CN TQ_EXPORT
#endif

class Q_EXPORT_CODECS_CN TQGb18030Codec : public TQTextCodec {
public:
    TQGb18030Codec();

    int mibEnum() const;
    const char* name() const;

    TQTextDecoder* makeDecoder() const;

#if !defined(Q_NO_USING_KEYWORD)
    using TQTextCodec::fromUnicode;
#endif
    TQCString fromUnicode(const TQString& uc, int& lenInOut) const;
    TQString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

class Q_EXPORT_CODECS_CN TQGbkCodec : public TQGb18030Codec {
public:
    TQGbkCodec();

    int mibEnum() const;
    const char* name() const;

    TQTextDecoder* makeDecoder() const;

#if !defined(Q_NO_USING_KEYWORD)
    using TQGb18030Codec::fromUnicode;
#endif
    TQCString fromUnicode(const TQString& uc, int& lenInOut) const;
    TQString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

class Q_EXPORT_CODECS_CN TQGb2312Codec : public TQGb18030Codec {
public:
    TQGb2312Codec();

    int mibEnum() const;
    const char* name() const;

    TQTextDecoder* makeDecoder() const;

#if !defined(Q_NO_USING_KEYWORD)
    using TQGb18030Codec::fromUnicode;
#endif
    TQCString fromUnicode(const TQString& uc, int& lenInOut) const;
    TQString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

#endif
#endif
