/* config.h.in.  Generated automatically from configure.in by autoheader.  */
/*
** This file has been automatically generated by 'acconfig' from aclocal.m4
** Copyright (C) 1988 Eleftherios Gkioulekas <lf@amath.washington.edu>
**  
** This file is free software; as a special exception the author gives
** unlimited permission to copy and/or distribute it, with or without 
** modifications, as long as this notice is preserved.
** 
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
** implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* This is the top section */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* And now the rest of the boys */
#define CXX_HAS_BUGGY_FOR_LOOPS 1
/* #undef CXX_HAS_NO_BOOL */
/* #undef ID3_ENABLE_DEBUG */
/* #undef ID3_DISABLE_ASSERT */
/* #undef ID3_DISABLE_CHECKS */
/* #undef ID3_ICONV_FORMAT_UTF16BE */
/* #undef ID3_ICONV_FORMAT_UTF16 */
/* #undef ID3_ICONV_FORMAT_UTF8 */
/* #undef ID3_ICONV_FORMAT_ASCII */
/* #undef ID3LIB_ICONV_OLDSTYLE */
/* #undef ID3LIB_ICONV_CAST_OK */

/* config.h defines these preprocesser symbols to be used by id3lib for
 * determining internal versioning information.  The intent is that these
 * macros will be made available in the library via constants, functions,
 * or static methods.
 */
/* #undef HAVE_ZLIB */
/* #undef HAVE_GETOPT_LONG */
#define _ID3LIB_NAME "id3lib"
#define _ID3LIB_VERSION "3.8.3"
#define _ID3LIB_VERSION0 "3.8.3\0"
#define _ID3LIB_FULLNAME "id3lib-3.8.3"
#define _ID3LIB_MAJOR_VERSION 3
#define _ID3LIB_MINOR_VERSION 8
#define _ID3LIB_PATCH_VERSION 3
#define _ID3LIB_INTERFACE_AGE 0
#define _ID3LIB_BINARY_AGE 0
/* #undef ID3_COMPILED_WITH_DEBUGGING */
/* */

/* Define if you have the getopt_long function.  */
#define HAVE_GETOPT_LONG 1

/* Define if you have the mkstemp function.  */
/* #undef HAVE_MKSTEMP */

/* Define if you have the ftruncate function.  */
/* #undef HAVE_TRUNCATE */

/* Define if you have the <cctype> header file.  */
#define HAVE_CCTYPE 1

/* Define if you have the <climits> header file.  */
#define HAVE_CLIMITS 1

/* Define if you have the <cstdio> header file.  */
#define HAVE_CSTDIO 1

/* Define if you have the <cstdlib> header file.  */
#define HAVE_CSTDLIB 1

/* Define if you have the <cstring> header file.  */
#define HAVE_CSTRING 1

/* Define if you have the <fstream> header file.  */
#define HAVE_FSTREAM 1

/* Define if you have the <fstream.h> header file.  */
#define HAVE_FSTREAM_H 1

/* Define if you have the <iconv.h> header file.  */
/* #undef HAVE_ICONV_H */

/* Define if you have the <iomanip> header file.  */
#define HAVE_IOMANIP 1

/* Define if you have the <iomanip.h> header file.  */
#define HAVE_IOMANIP_H 1

/* Define if you have the <iostream> header file.  */
#define HAVE_IOSTREAM 1

/* Define if you have the <iostream.h> header file.  */
#define HAVE_IOSTREAM_H 1

/* Define if you have the <libcw/sys.h> header file.  */
/* #undef HAVE_LIBCW_SYS_H */

/* Define if you have the <bitset> header file. */
#define HAVE_BITSET 1

/* Define if you have the <string> header file.  */
#define HAVE_STRING 1

/* Define if you have the <sys/param.h> header file.  */
/* #undef HAVE_SYS_PARAM_H */

/* Define if you have the <unistd.h> header file.  */
/* #undef HAVE_UNISTD_H */

/* Define if you have the <wchar.h> header file.  */
#define HAVE_WCHAR_H 1

/* Define if you have the <zlib.h> header file.  */
/* #undef HAVE_ZLIB_H */

/* Name of package */
#define PACKAGE "id3lib"

/* Version number of package */
#define VERSION _ID3LIB_VERSION

/* This is the bottom section */

// This file defines portability work-arounds for various proprietory
// C++ compilers

// Workaround for compilers with buggy for-loop scoping
// That's quite a few compilers actually including recent versions of
// Dec Alpha cxx, HP-UX CC and SGI CC.
// The trivial "if" statement provides the correct scoping to the 
// for loop

#ifdef CXX_HAS_BUGGY_FOR_LOOPS
/* #undef for */
#define for if(1) for
#endif

//
// If the C++ compiler we use doesn't have bool, then
// the following is a near-perfect work-around. 
// You must make sure your code does not depend on "int" and "bool"
// being two different types, in overloading for instance.
//

#ifdef CXX_HAS_NO_BOOL
#define bool int
#define true 1
#define false 0
#endif
   
#if defined (ID3_ENABLE_DEBUG) && defined (HAVE_LIBCW_SYS_H) && defined (__cplusplus)

#define DEBUG

#include <libcw/sys.h>
#include <libcw/debug.h>

#define ID3D_INIT_DOUT()    Debug( libcw_do.on() )
#define ID3D_INIT_WARNING() Debug( dc::warning.on() )
#define ID3D_INIT_NOTICE()  Debug( dc::notice.on() )
#define ID3D_NOTICE(x)      Dout( dc::notice, x )
#define ID3D_WARNING(x)     Dout( dc::warning, x )

#else

#  define ID3D_INIT_DOUT()
#  define ID3D_INIT_WARNING()
#  define ID3D_INIT_NOTICE()
#  define ID3D_NOTICE(x)
#  define ID3D_WARNING(x)

#endif /* defined (ID3_ENABLE_DEBUG) && defined (HAVE_LIBCW_SYS_H) */
   
