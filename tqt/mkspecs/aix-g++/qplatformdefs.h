#ifndef TQPLATFORMDEFS_H
#define TQPLATFORMDEFS_H

// Get TQt defines/settings

#include "tqglobal.h"

// Set any POSIX/XOPEN defines at the top of this file to turn on specific APIs

#include <unistd.h>


// We are hot - unistd.h should have turned on the specific APIs we requested


#ifdef TQT_THREAD_SUPPORT
// uncomment if you have problems with <sys/proc.h> because your gcc
// hasn't been built on exactly the same OS version your are using now.
// typedef int crid_t;
// typedef unsigned int class_id_t;
#include <pthread.h>
#endif

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <dlfcn.h>
#include <strings.h> // AIX X11 headers define FD_ZERO using bzero()

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

// DNS header files are not fully covered by X/Open specifications.
// In particular nothing is said about res_* :/
// On AIX header files <netinet/in.h> and <arpa/nameser.h> are not
// included by <resolv.h>. Note that <arpa/nameser.h> must be included
// before <resolv.h>.
#include <netinet/in.h>
#define class nsrr_class // AIX 4.3.1.0
#include <arpa/nameser.h>
#undef class
#include <resolv.h>


#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	SIG_IGN

#ifdef _AIX43
// AIX 4.3
// The AIX 4.3 online documentation says 'size_t' but a user asked IBM
// and they told him the documentation is wrong. And anyway 'socklen_t'
// reportedly works for all AIX 4.3 users.
#define QT_SOCKLEN_T		socklen_t
#elif _AIX42
// AIX 4.2
#define QT_SOCKLEN_T		size_t
#else
// AIX 4.1
#define QT_SOCKLEN_T		size_t
// override
#define QT_SOCKOPTLEN_T		int
#endif

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)
// AIX 4.3 and better only
#define QT_SNPRINTF		::snprintf
#define QT_VSNPRINTF		::vsnprintf
#endif


#endif // TQPLATFORMDEFS_H
