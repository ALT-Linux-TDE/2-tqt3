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
#include <dl.h>
#define QT_HPUX_LD
#define TQT_NO_LIBRARY_UNLOAD

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
// #ifdef __cplusplus
// extern "C" {
// #endif
// getres() is mangled because of missing extern "C" on
// HP-UX 11.x systems missing PHCO_23963
#include <resolv.h>
// #ifdef __cplusplus
// }
// #endif
// Undeclared - at least on HP-UX 10.20.
extern "C" int res_init();

#define TQT_NO_LIBRARY_UNLOAD


#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	SIG_IGN

#define QT_SOCKLEN_T		int

// presence of _XOPEN_UNIX can be used to detect HP-UX 10 or higher
#if !defined(_XOPEN_UNIX)
// HP-UX 9's select() didn't accept fd_set, yet.
#define select(a,b,c,d,e) select((a), (int *)(b), (int *)(c), (int *)(d), (e))
#endif


#endif // TQPLATFORMDEFS_H
