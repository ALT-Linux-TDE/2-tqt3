/****************************************************************************
**
** Definition of ProjectBuilderMakefileGenerator class.
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of qmake.
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

#ifndef __PBUILDER_PBX_H__
#define __PBUILDER_PBX_H__

#include "unixmake.h"

class ProjectBuilderMakefileGenerator : public UnixMakefileGenerator
{
    TQString pbx_dir;
    int pbuilderVersion() const;
    bool writeSubdirs(TQTextStream &, bool);
    bool writeMakeParts(TQTextStream &);
    bool writeMakefile(TQTextStream &);

    TQString pbxbuild();
    TQMap<TQString, TQString> keys;
    TQString keyFor(const TQString &file);
    TQString fixQuotes(const TQString &val);
    TQString fixEnvs(const TQString &file);
    TQString fixEnvsList(const TQString &where);
    int     reftypeForFile(const TQString &where);
    TQString projectSuffix() const;

public:
    ProjectBuilderMakefileGenerator(TQMakeProject *p);
    ~ProjectBuilderMakefileGenerator();

    virtual bool openOutput(TQFile &) const;
protected:
    bool doPrecompiledHeaders() const { return FALSE; }
    virtual bool doDepends() const { return FALSE; } //never necesary
};

inline ProjectBuilderMakefileGenerator::~ProjectBuilderMakefileGenerator()
{ }


#endif /* __PBUILDER_PBX_H__ */
