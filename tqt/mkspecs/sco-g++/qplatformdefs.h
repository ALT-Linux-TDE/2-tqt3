#ifndef TQPLATFORMDEFS_H
#define TQPLATFORMDEFS_H

// Get TQt defines/settings

#include "tqglobal.h"

// Set any POSIX/XOPEN defines at the top of this file to turn on specific APIs

// This is to have PATH_MAX defined in <limits.h>
#ifndef _IBCS2
#  define _IBCS2
#endif

#include <unistd.h>


// We are hot - unistd.h should have turned on the specific APIs we requested


#ifdef TQT_THREAD_SUPPORT
#include <pthread.h>
#endif

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

// DNS header files are not fully covered by X/Open specifications.
// In particular nothing is said about res_* :/
// On Unixware header files <netinet/in.h> and <arpa/nameser.h> are not
// included by <resolv.h>. Note that <arpa/nameser.h> must be included
// before <resolv.h>.
#include <netinet/in.h>
#define class r_class // SCO OpenServer 5.0.6
#include <arpa/nameser.h>
#undef class
#include <resolv.h>


#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	SIG_IGN

#define QT_SOCKLEN_T		int

#define QT_SNPRINTF		::snprintf
#define QT_VSNPRINTF		::vsnprintf

// SCO OpenServer redefines raise -> kill
#if defined(raise)
# undef raise
#endif

#endif // TQPLATFORMDEFS_H
