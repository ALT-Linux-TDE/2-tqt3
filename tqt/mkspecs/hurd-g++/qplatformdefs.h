#ifndef TQPLATFORMDEFS_H
#define TQPLATFORMDEFS_H

// Get TQt defines/settings

#include "tqglobal.h"

#ifndef _DEFAULT_SOURCE
#  define _DEFAULT_SOURCE
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
// Header files <netinet/in.h> and <arpa/nameser.h> are not included
// by <resolv.h> on older versions of the GNU C library. Note that
// <arpa/nameser.h> must be included before <resolv.h>.
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>


#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	SIG_IGN

#define QT_SOCKLEN_T		socklen_t

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)
#define QT_SNPRINTF		::snprintf
#define QT_VSNPRINTF		::vsnprintf
#endif


#endif // TQPLATFORMDEFS_H
