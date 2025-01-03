#ifndef TQPLATFORMDEFS_H
#define TQPLATFORMDEFS_H

// Get TQt defines/settings

#include "tqglobal.h"

// Set any POSIX/XOPEN defines at the top of this file to turn on specific APIs

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
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

// DNS header files are not fully covered by X/Open specifications.
// In particular nothing is said about res_* :/
#include <resolv.h>


#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	SIG_IGN

#if defined(_XOPEN_SOURCE) && defined(_OSF_SOURCE)
// Not available in the <unistd.h> header file of Tru64 4.0F.
// Fixed in the <unistd.h> header of Tru64 5.0A so we copy/paste from there...
extern "C" int usleep(useconds_t);
#endif

#if defined(_POSIX_PII_SOCKET)
#  define QT_SOCKLEN_T		socklen_t
#elif defined(_XOPEN_SOURCE_EXTENDED)
#  define QT_SOCKLEN_T		size_t
#else
#  define QT_SOCKLEN_T		int
#endif

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE-0 >= 400)
// trying to detect Tru64 5.0 and better...
#define QT_SNPRINTF		::snprintf
#define QT_VSNPRINTF		::vsnprintf
#endif


#endif // TQPLATFORMDEFS_H
