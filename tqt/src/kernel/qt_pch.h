/*
 * This is a precompiled header file for use in Xcode / Mac GCC / 
 * GCC >= 3.4 / VC to greatly speed the building of TQt. It may also be
 * of use to people developing their own project, but it is probably
 * better to define your own header.  Use of this header is currently
 * UNSUPPORTED.
 */

#if (defined(_WIN32) || defined(__NT__))
#  define QT_UNDEF_MACROS_IN_PCH
#  define _WINSCARD_H_
#  define _POSIX_ 	/* Make sure PATH_MAX et al. are defined    */
#  include <limits.h>
#  undef _POSIX_  	/* Don't polute                             */
#endif

#if defined __cplusplus
#  if defined(__GNUC__)
#    ifndef TQT_NO_STL
#      include <ios>
#      undef _GLIBCPP_FULLY_COMPLIANT_HEADERS  // Makes tqlocale.cpp compile
#    endif
#  endif
#include <tqmap.h>		    // I must be first!
#include <private/tqucomextra_p.h>  // All moc genereated code has this include
#include <tqapplication.h>
#include <tqbitmap.h>
#include <tqcursor.h>
#include <tqdatetime.h>
#include <tqglobal.h>
#include <tqimage.h>
#include <tqmetaobject.h>  // All moc genereated code has this include
#include <tqobject.h>
#include <tqpainter.h>
#include <tqpixmap.h>
#include <qplatformdefs.h>
#include <tqptrlist.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqstyle.h>
#include <tqtimer.h>
#include <tqwidget.h>

#include <limits.h>
#include <stdlib.h>
#if defined(__GNUC__)
#  ifndef TQT_NO_STL
#    define _GLIBCPP_FULLY_COMPLIANT_HEADERS
#  endif
#endif
#endif

#if defined(QT_UNDEF_MACROS_IN_PCH)
#  undef max /*  These are defined in windef.h, but                   */
#  undef min /*  we don't want them when building TQt                  */
#  undef _WINSCARD_H_
#endif
