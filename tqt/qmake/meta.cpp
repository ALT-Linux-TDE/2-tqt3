/****************************************************************************
**
** Implementation of TQMakeMetaInfo class.
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

#include "meta.h"
#include "project.h"
#include "option.h"
#include <ntqdir.h>

TQMap<TQString, TQMap<TQString, TQStringList> > TQMakeMetaInfo::cache_vars;

TQMakeMetaInfo::TQMakeMetaInfo()
{
    
}


bool
TQMakeMetaInfo::readLib(const TQString &lib)
{
    clear();
    TQString meta_file = findLib(lib);
    
    if(cache_vars.contains(meta_file)) {
	vars = cache_vars[meta_file];
	return TRUE;
    }

    bool ret = FALSE;
    if(!meta_file.isNull()) {
	if(meta_file.endsWith(Option::pkgcfg_ext)) {
	    if((ret=readPkgCfgFile(meta_file))) 
		meta_type = "pkgcfg";
	} else if(meta_file.endsWith(Option::libtool_ext)) {
	    if((ret=readLibtoolFile(meta_file))) 
		meta_type = "libtool";
	} else if(meta_file.endsWith(Option::prl_ext)) {
	    TQMakeProject proj;
	    if(!proj.read(Option::fixPathToLocalOS(meta_file), 
			  TQDir::currentDirPath(), TQMakeProject::ReadProFile))
		return FALSE;
	    meta_type = "tqmake";
	    vars = proj.variables();
	    ret = TRUE;
	} else {
	    warn_msg(WarnLogic, "TQMakeMetaInfo: unknown file format for %s", meta_file.latin1());
	}
    }
    if(ret) 
	cache_vars.insert(meta_file, vars);
    return ret;
}


void
TQMakeMetaInfo::clear()
{
    vars.clear();
}


TQString
TQMakeMetaInfo::findLib(const TQString &lib)
{
    TQString ret = TQString::null;
    TQString extns[] = { Option::prl_ext, /*Option::pkgcfg_ext, Option::libtool_ext,*/ TQString::null };
    for(int extn = 0; !extns[extn].isNull(); extn++) {
	if(lib.endsWith(extns[extn]))
	    ret = TQFile::exists(lib) ? lib : TQString::null;
    }
    if(ret.isNull()) {
	for(int extn = 0; !extns[extn].isNull(); extn++) {
	    if(TQFile::exists(lib + extns[extn])) {
		ret = lib + extns[extn];
		break;
	    }
	}
    }
    if(ret.isNull())
	debug_msg(2, "TQMakeMetaInfo: Cannot find info file for %s", lib.latin1());
    else
	debug_msg(2, "TQMakeMetaInfo: Found info file %s for %s", ret.latin1(), lib.latin1());
    return ret;
}


bool
TQMakeMetaInfo::readLibtoolFile(const TQString &f)
{
    /* I can just run the .la through the .pro parser since they are compatible.. */
    TQMakeProject proj;
    if(!proj.read(Option::fixPathToLocalOS(f), TQDir::currentDirPath(), TQMakeProject::ReadProFile))
	return FALSE;
    TQString dirf = Option::fixPathToTargetOS(f).section(Option::dir_sep, 0, -2);
    if(dirf == f)
	dirf = "";
    else if(!dirf.isEmpty() && !dirf.endsWith(Option::output_dir))
	dirf += Option::dir_sep;
    TQMap<TQString, TQStringList> &v = proj.variables();
    for(TQMap<TQString, TQStringList>::Iterator it = v.begin(); it != v.end(); ++it) {
	TQStringList lst = it.data();
	if(lst.count() == 1 && (lst.first().startsWith("'") || lst.first().startsWith("\"")) &&
	   lst.first().endsWith(TQString(lst.first()[0])))
	    lst = lst.first().mid(1, lst.first().length() - 2);
	if(!vars.contains("QMAKE_PRL_TARGET") &&
	   (it.key() == "dlname" || it.key() == "library_names" || it.key() == "old_library")) {
	    TQString dir = v["libdir"].first();
	    if((dir.startsWith("'") || dir.startsWith("\"")) && dir.endsWith(TQString(dir[0])))
		dir = dir.mid(1, dir.length() - 2);
	    dir = dir.stripWhiteSpace();
	    if(!dir.isEmpty() && !dir.endsWith(Option::dir_sep))
		dir += Option::dir_sep;
	    if(lst.count() == 1)
		lst = TQStringList::split(" ", lst.first());
	    for(TQStringList::Iterator lst_it = lst.begin(); lst_it != lst.end(); ++lst_it) {
		bool found = FALSE;
		TQString dirs[] = { "", dir, dirf, dirf + ".libs" + TQDir::separator(), "(term)" };
		for(int i = 0; !found && dirs[i] != "(term)"; i++) {
		    if(TQFile::exists(dirs[i] + (*lst_it))) {
			TQString targ = dirs[i] + (*lst_it);
			if(TQDir::isRelativePath(targ)) 
			    targ.prepend(TQDir::currentDirPath() + TQDir::separator());
			vars["QMAKE_PRL_TARGET"] << targ;
			found = TRUE;
		    }
		}
		if(found)
 		    break;
	    }
	} else if(it.key() == "dependency_libs") {
	    if(lst.count() == 1) {
		TQString dep = lst.first();
		if((dep.startsWith("'") || dep.startsWith("\"")) && dep.endsWith(TQString(dep[0])))
		    dep = dep.mid(1, dep.length() - 2);
		lst = TQStringList::split(" ", dep.stripWhiteSpace());
	    }
	    TQMakeProject *conf = NULL;
	    for(TQStringList::Iterator lit = lst.begin(); lit != lst.end(); ++lit) {
		if((*lit).startsWith("-R")) {
		    if(!conf) {
			conf = new TQMakeProject;
			conf->read(TQMakeProject::ReadAll ^ TQMakeProject::ReadProFile);
		    }
		    if(!conf->isEmpty("QMAKE_RPATH"))
			(*lit) = conf->first("QMAKE_RPATH") + (*lit).mid(2);
		}
	    }
	    if(conf)
		delete conf;
	    vars["QMAKE_PRL_LIBS"] += lst;
	}
    }
    return TRUE;
}

bool
TQMakeMetaInfo::readPkgCfgFile(const TQString &f)
{
    fprintf(stderr, "Must implement reading in pkg-config files (%s)!!!\n", f.latin1());
    return FALSE;
}
