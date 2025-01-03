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
// Cygwin does not provide <sys/ipc.h> and <sys/shm.h> because it
// doesn't support SysV IPC or shared memory. See for example:
// 	http://afni.nimh.nih.gov/afni/afniboard/messages/1725.html
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <windows.h>

// Resolver functions are not implemented in Cygwin, headers
// <arpa/nameser.h> and <resolv.h> are missing:
// 	http://cygwin.com//cgi-bin/cygwin-todo.cgi?20000426.105252
// Maybe bindlib_w32
// 	http://www.php.net/extra/bindlib_w32.zip
// or Don Berstein's djbdns:
// 	http://cr.yp.to/djbdns.html
// provide this functionality, but not necessarily under the Cygwin license...
#include <netinet/in.h>


#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	SIG_IGN

#define QT_SOCKLEN_T		socklen_t

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)
#define QT_SNPRINTF		::snprintf
#define QT_VSNPRINTF		::vsnprintf
#endif


#endif // TQPLATFORMDEFS_H
