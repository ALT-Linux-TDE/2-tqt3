/****************************************************************************
**
** Implementation of MetrowerksMakefileGenerator class.
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

#include "metrowerks_xml.h"
#include "option.h"
#include <ntqdir.h>
#include <ntqdict.h>
#include <ntqregexp.h>
#include <stdlib.h>
#include <time.h>
#if !defined(TQWS) && defined(Q_OS_MAC)
#include <Carbon/Carbon.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

MetrowerksMakefileGenerator::MetrowerksMakefileGenerator(TQMakeProject *p) : MakefileGenerator(p), init_flag(FALSE)
{

}

bool
MetrowerksMakefileGenerator::writeMakefile(TQTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	/* for now just dump, I need to generated an empty xml or something.. */
	fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
		var("QMAKE_FAILED_REQUIREMENTS").latin1());
	return TRUE;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
	return writeMakeParts(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
	writeHeader(t);
	tqDebug("Not supported!");
	return TRUE;
    }
    return FALSE;
}

bool
MetrowerksMakefileGenerator::writeMakeParts(TQTextStream &t)
{
    //..grrr.. libs!
    TQStringList extra_objs;
    bool do_libs = TRUE;
    if(project->first("TEMPLATE") == "app") 
	extra_objs += project->variables()["QMAKE_CRT_OBJECTS"];
    else if(project->first("TEMPLATE") == "lib" && project->isActiveConfig("staticlib"))
	do_libs = FALSE;
    if(do_libs)
	extra_objs += project->variables()["QMAKE_LIBS"];
    for(TQStringList::Iterator val_it = extra_objs.begin(); 
	val_it != extra_objs.end(); ++val_it) {
	if((*val_it).startsWith("-L")) {
	    TQString dir((*val_it).right((*val_it).length() - 2));
	    fixEnvVariables(dir);
	    if(project->variables()["DEPENDPATH"].findIndex(dir) == -1 &&
	       project->variables()["INCLUDEPATH"].findIndex(dir) == -1)
		project->variables()["INCLUDEPATH"].append(dir);
	} else if((*val_it).startsWith("-l")) {
	    TQString lib("lib" + (*val_it).right((*val_it).length() - 2)  + "." + 
			project->first("QMAKE_EXTENSION_SHLIB"));
	    if(project->variables()["LIBRARIES"].findIndex(lib) == -1)
		project->variables()["LIBRARIES"].append(lib);
	} else 
	    if((*val_it) == "-framework") {
	    ++val_it;
	    if(val_it == extra_objs.end())
		break;
	    TQString frmwrk = (*val_it) + ".framework";
	    if(project->variables()["FRAMEWORKS"].findIndex(frmwrk) == -1)
		project->variables()["FRAMEWORKS"].append(frmwrk);
	} else if((*val_it).left(1) != "-") {
	    TQString lib=(*val_it);
	    int s = lib.findRev('/');
	    if(s != -1) {
		TQString dir = lib.left(s);
		lib = lib.right(lib.length() - s - 1);
		fixEnvVariables(dir);
		if(project->variables()["DEPENDPATH"].findIndex(dir) == -1 &&
		   project->variables()["INCLUDEPATH"].findIndex(dir) == -1)
		    project->variables()["INCLUDEPATH"].append(dir);
	    }
	    project->variables()["LIBRARIES"].append(lib);
	}
    }
    //let metrowerks find the files & set the files to the type I expect
    TQDict<void> seen(293);
    TQString paths[] = { TQString("SRCMOC"), TQString("FORMS"), TQString("UICDECLS"),
			TQString("UICIMPLS"), TQString("SOURCES"),TQString("HEADERS"),
			TQString::null };
    for(int y = 0; paths[y] != TQString::null; y++) {
	TQStringList &l = project->variables()[paths[y]];
	for(TQStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
	    //establish file types
	    seen.insert((*val_it), (void *)1);
	    createFork((*val_it)); //the file itself
	    TQStringList &d = findDependencies((*val_it)); //depends
	    for(TQStringList::Iterator dep_it = d.begin(); dep_it != d.end(); ++dep_it) {
		if(!seen.find((*dep_it))) {
		    seen.insert((*dep_it), (void *)1);
		    createFork((*dep_it));
		}
	    }
	    //now chop it
	    int s = (*val_it).findRev('/');
	    if(s != -1) {
		TQString dir = (*val_it).left(s);
		(*val_it) = (*val_it).right((*val_it).length() - s - 1);
		TQString tmpd=dir, tmpv;
		if(fixifyToMacPath(tmpd, tmpv)) {
		    bool add_in = TRUE;
		    TQString deps[] = { TQString("DEPENDPATH"), 
				     TQString("INCLUDEPATH"), TQString::null }, 
				     dd, dv;
		    for(int yy = 0; deps[yy] != TQString::null; yy++) {
			TQStringList &l2 = project->variables()[deps[yy]];
			for(TQStringList::Iterator val_it2 = l2.begin(); 
			    val_it2 != l2.end(); ++val_it2) {
			    TQString dd= (*val_it2), dv;
			    if(!fixifyToMacPath(dd, dv)) 
				continue;
			    if(dd == tmpd && tmpv == dv) {
				add_in = FALSE;
				break;
			    }
			}
		    }
		    if(add_in) 
			project->variables()["INCLUDEPATH"].append(dir);
		}
	    }
	}
    }
    //need a defines file
    if(!project->isEmpty("DEFINES")) {
	TQString pre_pref = project->first("TARGET_STEM");
	if(project->first("TEMPLATE") == "lib")
	    pre_pref += project->isActiveConfig("staticlib") ? "_static" : "_shared";
	project->variables()["CODEWARRIOR_PREFIX_HEADER"].append(pre_pref + "_prefix.h");
    }

    TQString xmlfile = findTemplate(project->first("QMAKE_XML_TEMPLATE"));
    TQFile file(xmlfile);
    if(!file.open(IO_ReadOnly )) {
	fprintf(stderr, "Cannot open XML file: %s\n", 
		project->first("QMAKE_XML_TEMPLATE").latin1());
	return FALSE;
    }
    TQTextStream xml(&file);
    createFork(Option::output.name());

    int rep;
    TQString line;
    while ( !xml.eof() ) {
	line = xml.readLine();
	while((rep = line.find(TQRegExp("\\$\\$[!a-zA-Z0-9_-]*"))) != -1) {
	    TQString torep = line.mid(rep, line.find(TQRegExp("[^\\$!a-zA-Z0-9_-]"), rep) - rep);
	    TQString variable = torep.right(torep.length()-2);

	    t << line.left(rep); //output the left side
	    line = line.right(line.length() - (rep + torep.length())); //now past the variable
	    if(variable == "CODEWARRIOR_HEADERS" || variable == "CODEWARRIOR_SOURCES" || 
	       variable == "CODEWARRIOR_LIBRARIES" || variable == "CODEWARRIOR_QPREPROCESS" ||
		variable == "CODEWARRIOR_QPREPROCESSOUT") {
		TQString outcmd=variable.right(variable.length() - variable.findRev('_') - 1);
		TQStringList args;
		if(outcmd == "TQPREPROCESS")
		    args << "UICS" << "MOCS";
		else if(outcmd == "TQPREPROCESSOUT")
		    args << "SRCMOC" << "UICIMPLS" << "UICDELCS";
		else
		    args << outcmd;
		for(TQStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
		    TQString arg = (*arit);
		    TQString kind = "Text";
		    if(arg == "LIBRARIES")
			kind = "Library";
		    if(!project->variables()[arg].isEmpty()) {
			TQStringList &list = project->variables()[arg];
			for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
			    TQString flag;
			    if(project->isActiveConfig("debug")) {
				bool debug = TRUE;
				if(outcmd == "TQPREPROCESS") {
				    debug = FALSE;
				} else {
				    for(TQStringList::Iterator hit = Option::h_ext.begin(); hit != Option::h_ext.end(); ++hit) {
					if((*it).endsWith((*hit))) { 
					    debug = FALSE;
					    break;
					}
				    }
				}
				if(debug)
				    flag = "Debug";
			    }
			    t << "\t\t\t\t<FILE>" << endl
			      << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
			      << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
			      << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
			      << "\t\t\t\t\t<FILEKIND>" << kind << "</FILEKIND>" << endl
			      << "\t\t\t\t\t<FILEFLAGS>" << flag << "</FILEFLAGS>" << endl
			      << "\t\t\t\t</FILE>" << endl;
			}
		    }
		}
	    } else if(variable == "CODEWARRIOR_SOURCES_LINKORDER" || 
		      variable == "CODEWARRIOR_HEADERS_LINKORDER" ||
		      variable == "CODEWARRIOR_LIBRARIES_LINKORDER" || 
		      variable == "CODEWARRIOR_QPREPROCESS_LINKORDER" ||
		      variable == "CODEWARRIOR_QPREPROCESSOUT_LINKORDER") {
		TQString outcmd=variable.mid(variable.find('_')+1, 
					    variable.findRev('_')-(variable.find('_')+1));
		TQStringList args;
		if(outcmd == "TQPREPROCESS")
		    args << "UICS" << "MOCS";
		else if(outcmd == "TQPREPROCESSOUT")
		    args << "SRCMOC" << "UICIMPLS" << "UICDELCS";
		else
		    args << outcmd;
		for(TQStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
		    TQString arg = (*arit);
		    if(!project->variables()[arg].isEmpty()) {
			TQStringList &list = project->variables()[arg];
			for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
			    t << "\t\t\t\t<FILEREF>" << endl
			      << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
			      << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
			      << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
			      << "\t\t\t\t</FILEREF>" << endl;
			}
		    }
		}
	    } else if(variable == "CODEWARRIOR_HEADERS_GROUP" || 
		      variable == "CODEWARRIOR_SOURCES_GROUP" ||
		      variable == "CODEWARRIOR_LIBRARIES_GROUP" || 
		      variable == "CODEWARRIOR_QPREPROCESS_GROUP" ||
		      variable == "CODEWARRIOR_QPREPROCESSOUT_GROUP") {
		TQString outcmd = variable.mid(variable.find('_')+1, 
					      variable.findRev('_')-(variable.find('_')+1));
		TQStringList args;
		if(outcmd == "TQPREPROCESS")
		    args << "UICS" << "MOCS";
		else if(outcmd == "TQPREPROCESSOUT")
		    args << "SRCMOC" << "UICIMPLS" << "UICDELCS";
		else
		    args << outcmd;
		for(TQStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
		    TQString arg = (*arit);
		    if(!project->variables()[arg].isEmpty()) {
			TQStringList &list = project->variables()[arg];
			for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
			    t << "\t\t\t\t<FILEREF>" << endl
			      << "\t\t\t\t\t<TARGETNAME>" << var("TARGET_STEM") << "</TARGETNAME>" 
			      << endl
			      << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
			      << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
			      << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
			      << "\t\t\t\t</FILEREF>" << endl;
			}
		    }
		}
	    } else if(variable == "CODEWARRIOR_FRAMEWORKS") {
		if(!project->isEmpty("FRAMEWORKS")) {
		    TQStringList &list = project->variables()["FRAMEWORKS"];
		    for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
			t << "\t\t\t\t<FRAMEWORK>" << endl
			  << "\t\t\t\t\t<FILEREF>" << endl
			  << "\t\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
			  << "\t\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
			  << "\t\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
			  << "\t\t\t\t\t</FILEREF>" << endl
			  << "\t\t\t\t</FRAMEWORK>" << endl;
		    }
		}
	    } else if(variable == "CODEWARRIOR_DEPENDPATH" || variable == "CODEWARRIOR_INCLUDEPATH" ||
		variable == "CODEWARRIOR_FRAMEWORKPATH") {	
		TQString arg=variable.right(variable.length()-variable.find('_')-1);
		TQStringList list;
		if(arg == "INCLUDEPATH") {
		    list = project->variables()[arg];
		    list << Option::mkfile::qmakespec;
		    list << TQDir::current().currentDirPath();

		    TQStringList &l = project->variables()["QMAKE_LIBS_PATH"];
		    for(TQStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
			TQString p = (*val_it), v;
			if(!fixifyToMacPath(p, v))
			    continue;

			t << "\t\t\t\t\t<SETTING>" << endl
			  << "\t\t\t\t\t\t<SETTING><NAME>SearchPath</NAME>" << endl
			  << "\t\t\t\t\t\t\t<SETTING><NAME>Path</NAME>"
			  << "<VALUE>" << p << "</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>CodeWarrior</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t\t</SETTING>" << endl
			  << "\t\t\t\t\t\t<SETTING><NAME>Recursive</NAME><VALUE>true</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t\t<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t</SETTING>" << endl;
		    }
		} else if(variable == "DEPENDPATH") {
		    TQStringList &l = project->variables()[arg];
		    for(TQStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it)
		    {
			//apparently tmake used colon separation...
			TQStringList damn = TQStringList::split(':', (*val_it));
			if(!damn.isEmpty())
			    list += damn;
			else
			    list.append((*val_it));
		    }
		} else {
		    list = project->variables()[arg];
		}
		for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		    TQString p = (*it), v, recursive = "false", framework = "false";
		    if(p.startsWith("recursive--")) {
			p = p.right(p.length() - 11);
			recursive = "true";
		    } 
		    if(!fixifyToMacPath(p, v))
			continue;
		    if(arg == "FRAMEWORKPATH")
			framework = "true";

		    t << "\t\t\t\t\t<SETTING>" << endl
		      << "\t\t\t\t\t\t<SETTING><NAME>SearchPath</NAME>" << endl
		      << "\t\t\t\t\t\t\t<SETTING><NAME>Path</NAME>"
		      << "<VALUE>" << p << "</VALUE></SETTING>" << endl
		      << "\t\t\t\t\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>" << endl
		      << "\t\t\t\t\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>" << v << "</VALUE></SETTING>" << endl
		      << "\t\t\t\t\t\t</SETTING>" << endl
		      << "\t\t\t\t\t\t<SETTING><NAME>Recursive</NAME><VALUE>" << recursive << "</VALUE></SETTING>" << endl
		    << "\t\t\t\t\t\t<SETTING><NAME>FrameworkPath</NAME><VALUE>" << framework << "</VALUE></SETTING>" << endl
		    << "\t\t\t\t\t\t<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>" << endl
		      << "\t\t\t\t\t</SETTING>" << endl;
		}
	    } else if(variable == "CODEWARRIOR_WARNING" || variable == "!CODEWARRIOR_WARNING") {
		bool b = ((!project->isActiveConfig("warn_off")) && 
			  project->isActiveConfig("warn_on"));
		if(variable.startsWith("!"))
		    b = !b;
		t << (int)b;
	    } else if(variable == "CODEWARRIOR_TEMPLATE") {
		if(project->first("TEMPLATE") == "app" ) {
		    t << "Executable";
		} else if(project->first("TEMPLATE") == "lib") {
		    if(project->isActiveConfig("staticlib"))
		       t << "Library";
		    else
			t << "SharedLibrary";
		}
	    } else if(variable == "CODEWARRIOR_OUTPUT_DIR") {
		TQString outdir = "{Project}/", volume;
		if(!project->isEmpty("DESTDIR"))
		    outdir = project->first("DESTDIR");
		if(project->first("TEMPLATE") == "app" && !project->isActiveConfig("console"))
		    outdir += var("TARGET") + ".app/Contents/MacOS/";
		if(fixifyToMacPath(outdir, volume, FALSE)) {
                    t << "\t\t\t<SETTING><NAME>Path</NAME><VALUE>" << outdir << "</VALUE></SETTING>" 
		      << endl
		      << "\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>" << endl
		      << "\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>" << volume << "</VALUE></SETTING>" 
		      << endl;
		}
	    } else if(variable == "CODEWARRIOR_PACKAGER_PANEL") {
		if(project->first("TEMPLATE") == "app" && !project->isActiveConfig("console")) {
		    TQString outdir = "{Project}/", volume;
		    if(!project->isEmpty("DESTDIR"))
			outdir = project->first("DESTDIR");
		    outdir += var("TARGET") + ".app";
		    if(fixifyToMacPath(outdir, volume, FALSE)) {
			t << "\t\t<SETTING><NAME>MWMacOSPackager_UsePackager</NAME>"
			  << "<VALUE>1</VALUE></SETTING>" << "\n"
			  << "\t\t<SETTING><NAME>MWMacOSPackager_FolderToPackage</NAME>" << "\n"
			  << "\t\t\t<SETTING><NAME>Path</NAME><VALUE>" << outdir 
			  << "</VALUE></SETTING>" << "\n"
			  << "\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>" 
			  << "\n"
			  << "\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>" << volume 
			  << "</VALUE></SETTING>" << "\n"
			  << "\t\t</SETTING>" << "\n"
			  << "\t\t<SETTING><NAME>MWMacOSPackager_CreateClassicAlias</NAME>"
			  << "<VALUE>0</VALUE></SETTING>" << "\n"
			  << "\t\t<SETTING><NAME>MWMacOSPackager_ClassicAliasMethod</NAME>"
			  << "<VALUE>UseTargetOutput</VALUE></SETTING>" << "\n"
			  << "\t\t<SETTING><NAME>MWMacOSPackager_ClassicAliasPath</NAME>"
			  << "<VALUE></VALUE></SETTING>" << "\n"
			  << "\t\t<SETTING><NAME>MWMacOSPackager_CreatePkgInfo</NAME>"
			  << "<VALUE>1</VALUE></SETTING>" << "\n"
			  << "\t\t<SETTING><NAME>MWMacOSPackager_PkgCreatorType</NAME>" 
			  << "<VALUE>CUTE</VALUE></SETTING>" << "\n"
			  << "\t\t<SETTING><NAME>MWMacOSPackager_PkgFileType</NAME>" 
			  << "<VALUE>APPL</VALUE></SETTING>" << endl;
		    }
		}
	    } else if(variable == "CODEWARRIOR_FILETYPE") {
		if(project->first("TEMPLATE") == "lib")
			t << "MYDL";
		else
			t << "MEXE";
	    } else if(variable == "CODEWARRIOR_TQTDIR") {
		t << getenv("TQTDIR");
	    } else if(variable == "CODEWARRIOR_CACHEMODDATES") {
		t << "true";
	    } else {
		t << var(variable);
	    }
	}
	t << line << endl;
    }
    t << endl;
    file.close();

    if(mocAware()) { 
	TQString mocs = project->first("MOCS");
	TQFile mocfile(mocs);
	if(!mocfile.open(IO_WriteOnly)) {
	    fprintf(stderr, "Cannot open MOCS file: %s\n", mocs.latin1());
	} else {
	    createFork(mocs);
	    TQTextStream mocs(&mocfile);
	    TQStringList &list = project->variables()["SRCMOC"];
	    for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		TQString src = findMocSource((*it));
		if(src.findRev('/') != -1)
		    src = src.right(src.length() - src.findRev('/') - 1);
		mocs << src << endl;
	    }
	    mocfile.close();
	}
    }

    if(!project->isEmpty("FORMS")) { 
	TQString uics = project->first("UICS");
	TQFile uicfile(uics);
	if(!uicfile.open(IO_WriteOnly)) {
	    fprintf(stderr, "Cannot open UICS file: %s\n", uics.latin1());
	} else {
	    createFork(uics);
	    TQTextStream uics(&uicfile);
	    TQStringList &list = project->variables()["FORMS"];
	    for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		TQString ui = (*it);
		if(ui.findRev('/') != -1)
		    ui = ui.right(ui.length() - ui.findRev('/') - 1);
		uics << ui << endl;
	    }
	    uicfile.close();
	}
    }

    if(!project->isEmpty("CODEWARRIOR_PREFIX_HEADER")) {
	TQFile prefixfile(project->first("CODEWARRIOR_PREFIX_HEADER"));
	if(!prefixfile.open(IO_WriteOnly)) {
	    fprintf(stderr, "Cannot open PREFIX file: %s\n", prefixfile.name().latin1());
	} else {
	    createFork(project->first("CODEWARRIOR_PREFIX_HEADER"));
	    TQTextStream prefix(&prefixfile);
	    TQStringList &list = project->variables()["DEFINES"];
	    for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		if((*it).find('=') != -1) {
		    int x = (*it).find('=');
		    prefix << "#define " << (*it).left(x) << " " << (*it).right((*it).length() - x - 1) << endl;
		} else {
		    prefix << "#define " << (*it) << endl;
		}
	    }
	    prefixfile.close();
	}
    }
    return TRUE;
}



void
MetrowerksMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    if ( project->isEmpty("QMAKE_XML_TEMPLATE") ) 
	project->variables()["QMAKE_XML_TEMPLATE"].append("mwerkstmpl.xml");

    TQStringList &configs = project->variables()["CONFIG"];
    if(project->isActiveConfig("qt")) {
	if(configs.findIndex("moc")) configs.append("moc");
	if ( !( (project->first("TARGET") == "qt") || (project->first("TARGET") == "qte") ||
		(project->first("TARGET") == "tqt-mt") ) ) 
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT"];
	if(configs.findIndex("moc")) 
	    configs.append("moc");
	if ( !project->isActiveConfig("debug") ) 
	    project->variables()["DEFINES"].append("TQT_NO_DEBUG");
    }

    //version handling
    if(project->variables()["VERSION"].isEmpty())
	project->variables()["VERSION"].append("1.0." + 
					       (project->isEmpty("VER_PAT") ? TQString("0") : 
						project->first("VER_PAT")) );
    TQStringList ver = TQStringList::split('.', project->first("VERSION"));
    ver << "0" << "0"; //make sure there are three
    project->variables()["VER_MAJ"].append(ver[0]);
    project->variables()["VER_MIN"].append(ver[1]);
    project->variables()["VER_PAT"].append(ver[2]);

    if( !project->isEmpty("LIBS") )
	project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    if( project->variables()["QMAKE_EXTENSION_SHLIB"].isEmpty() )
	project->variables()["QMAKE_EXTENSION_SHLIB"].append( "dylib" );

    if ( project->isActiveConfig("moc") ) {
	TQString mocfile = project->first("TARGET");
	if(project->first("TEMPLATE") == "lib")
	    mocfile += project->isActiveConfig("staticlib") ? "_static" : "_shared";
	project->variables()["MOCS"].append(mocfile + ".mocs");
	setMocAware(TRUE);
    }
    if(!project->isEmpty("FORMS")) {
	TQString uicfile = project->first("TARGET");
	if(project->first("TEMPLATE") == "lib")
	    uicfile += project->isActiveConfig("staticlib") ? "_static" : "_shared";
	project->variables()["UICS"].append(uicfile + ".uics");
    }
    if(project->isEmpty("DESTDIR"))
	project->variables()["DESTDIR"].append(TQDir::currentDirPath());
    MakefileGenerator::init();

    if ( project->isActiveConfig("opengl") ) {
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_OPENGL"];
	if ( (project->first("TARGET") == "qt") || (project->first("TARGET") == "qte") ||
	     (project->first("TARGET") == "tqt-mt") )
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL_QT"];
	else 
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL"];
    }

    if(project->isActiveConfig("qt"))
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_TQT"];
    if(project->isEmpty("FRAMEWORKPATH"))
	project->variables()["FRAMEWORKPATH"].append("/System/Library/Frameworks/");

    //set the target up
    project->variables()["TARGET_STEM"] = project->variables()["TARGET"];
    if(project->first("TEMPLATE") == "lib") {
	if(project->isActiveConfig("staticlib"))
	    project->variables()["TARGET"].first() =  "lib" + project->first("TARGET") + ".lib";
	else
	    project->variables()["TARGET"].first() =  "lib" + project->first("TARGET") + "." +
						      project->first("QMAKE_EXTENSION_SHLIB");

	project->variables()["CODEWARRIOR_VERSION"].append(project->first("VER_MAJ") +
							  project->first("VER_MIN") +
							  project->first("VER_PAT"));
    } else {
	project->variables()["CODEWARRIOR_VERSION"].append("0");
	if(project->isEmpty("QMAKE_ENTRYPOINT"))
	   project->variables()["QMAKE_ENTRYPOINT"].append("start");
	project->variables()["CODEWARRIOR_ENTRYPOINT"].append(
	    project->first("QMAKE_ENTRYPOINT"));
    }
}


TQString
MetrowerksMakefileGenerator::findTemplate(const TQString &file)
{
    TQString ret;
    if(!TQFile::exists(ret = file) && 
       !TQFile::exists((ret = Option::mkfile::qmakespec + TQDir::separator() + file)) && 
       !TQFile::exists((ret = TQString(getenv("TQTDIR")) + "/mkspecs/mac-mwerks/" + file)) &&
       !TQFile::exists((ret = (TQString(getenv("HOME")) + "/.tmake/" + file))))
	return "";
    return ret;
}

bool
MetrowerksMakefileGenerator::createFork(const TQString &f)
{
#if !defined(TQWS) && defined(Q_OS_MACX)
    FSRef fref;
    FSSpec fileSpec;
    if(TQFile::exists(f)) {
	mode_t perms = 0;
	{
	    struct stat s;
	    stat(f.latin1(), &s);
	    if(!(s.st_mode & S_IWUSR)) {
		perms = s.st_mode;
		chmod(f.latin1(), perms | S_IWUSR);
	    }
	}
	FILE *o = fopen(f.latin1(), "a");
	if(!o)
	    return FALSE;
	if(FSPathMakeRef((const UInt8 *)f.latin1(), &fref, NULL) == noErr) {
	    if(FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, &fileSpec, NULL) == noErr) 
		FSpCreateResFile(&fileSpec, 'CUTE', 'TEXT', smSystemScript);
	    else 
		tqDebug("bogus %d", __LINE__);
	} else 
	    tqDebug("bogus %d", __LINE__);
	fclose(o);
	if(perms)
	    chmod(f.latin1(), perms);
    }
#else
    Q_UNUSED(f)
#endif
    return TRUE;
}

bool
MetrowerksMakefileGenerator::fixifyToMacPath(TQString &p, TQString &v, bool )
{
    v = "Absolute";
    if(p.find(':') != -1) //guess its macish already
	return TRUE;

    static TQString st_volume;
    if(st_volume.isEmpty()) {
	st_volume = var("QMAKE_VOLUMENAME");
#if !defined(TQWS) && defined(Q_OS_MACX)
	if(st_volume.isEmpty()) {
	    uchar foo[512];
	    HVolumeParam pb;
	    memset(&pb, '\0', sizeof(pb));
	    pb.ioVRefNum = 0;
	    pb.ioNamePtr = foo;
	    if(PBHGetVInfoSync((HParmBlkPtr)&pb) == noErr) {
		int len = foo[0];
		memcpy(foo,foo+1, len);
		foo[len] = '\0';
		st_volume = (char *)foo;
	    }
	}
#endif
    }
    TQString volume = st_volume;

    fixEnvVariables(p);
    if(p.startsWith("\"") && p.endsWith("\""))
	p = p.mid(1, p.length() - 2);
    if(p.isEmpty()) 
	return FALSE;
    if(!p.endsWith("/"))
	p += "/";
    if(TQDir::isRelativePath(p)) {
	if(p.startsWith("{")) {
	    int eoc = p.find('}');
	    if(eoc == -1)
		return FALSE;
	    volume = p.mid(1, eoc - 1);
	    p = p.right(p.length() - eoc - 1);
	} else {
	    TQFileInfo fi(p);
	    if(fi.convertToAbs()) //strange
		return FALSE;
	    p = fi.filePath();
	} 
    } 
    p = TQDir::cleanDirPath(p);
    if(!volume.isEmpty()) 
	v = volume;
    p.replace("/", ":");
    if(p.right(1) != ":")
	p += ':';
    return TRUE;
}

void
MetrowerksMakefileGenerator::processPrlFiles()
{
    TQPtrList<MakefileDependDir> libdirs;
    libdirs.setAutoDelete(TRUE);
    const TQString lflags[] = { "QMAKE_LIBS", TQString::null };
    for(int i = 0; !lflags[i].isNull(); i++) {
	for(bool ret = FALSE; TRUE; ret = FALSE) {
	    TQStringList l_out;
	    TQStringList &l = project->variables()[lflags[i]];
	    for(TQStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		TQString opt = (*it);
		if(opt.startsWith("-")) {
		    if(opt.startsWith("-L")) {
			TQString r = opt.right(opt.length() - 2), l = r;
			fixEnvVariables(l);
			libdirs.append(new MakefileDependDir(r.replace( "\"", ""),
							     l.replace( "\"", "")));
		    } else if(opt.left(2) == "-l") {
			TQString lib = opt.right(opt.length() - 2), prl;
			for(MakefileDependDir *mdd = libdirs.first(); mdd; mdd = libdirs.next() ) {
			    prl = mdd->local_dir + Option::dir_sep + "lib" + lib;
			    if(processPrlFile(prl)) {
				if(prl.startsWith(mdd->local_dir))
				    prl.replace(0, mdd->local_dir.length(), mdd->real_dir);
				TQRegExp reg("^.*lib(" + lib + "[^.]*)\\." + 
					    project->first("QMAKE_EXTENSION_SHLIB") + "$");
				if(reg.exactMatch(prl))
				    prl = "-l" + reg.cap(1);
				opt = prl;
				ret = TRUE;
				break;
			    }
			}
		    } else if(opt == "-framework") {
			l_out.append(opt);
			++it;
			opt = (*it);
			TQString prl = "/System/Library/Frameworks/" + opt +
				      ".framework/" + opt;
			if(processPrlFile(prl))
			    ret = TRUE;
		    }
		    if(!opt.isEmpty())
			l_out.append(opt);
		} else {
		    if(processPrlFile(opt))
			ret = TRUE;
		    if(!opt.isEmpty())
			l_out.append(opt);
		}
	    }
	    if(ret)
		l = l_out;
	    else
		break;
	}
    }
}

void
MetrowerksMakefileGenerator::processPrlVariable(const TQString &var, const TQStringList &l)
{
    if(var == "QMAKE_PRL_LIBS") {
	TQStringList &out = project->variables()["QMAKE_LIBS"];
	for(TQStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	    bool append = TRUE;
	    if((*it).startsWith("-")) {
		if((*it).startsWith("-l") || (*it).startsWith("-L")) {
		    append = out.findIndex((*it)) == -1;
		} else if((*it).startsWith("-framework")) {
		    ++it;
		    for(TQStringList::ConstIterator outit = out.begin(); 
			outit != out.end(); ++it) {
			if((*outit) == "-framework") {
			    ++outit;
			    if((*outit) == (*it)) {
				append = FALSE;
				break;
			    }
			}
		    }
		}
	    } else if(TQFile::exists((*it))) {
		append = out.findIndex((*it));
	    }
	    if(append)
		out.append((*it));
	}
    } else {
	MakefileGenerator::processPrlVariable(var, l);
    }
}


bool
MetrowerksMakefileGenerator::openOutput(TQFile &file) const
{
    TQString outdir;
    if(!file.name().isEmpty()) {
	TQFileInfo fi(file);
	if(fi.isDir())
	    outdir = file.name() + TQDir::separator();
    }
    if(!outdir.isEmpty() || file.name().isEmpty()) 
	file.setName(outdir + project->first("TARGET") + ".xml");
    return MakefileGenerator::openOutput(file);
}
