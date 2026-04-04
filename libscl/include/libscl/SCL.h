/*
 * This file is part of the SCL software.
 * The license which this software falls under is as follows:
 *
 * Copyright (C) 2004-2010 Douglas Jerome <douglas@backstep.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* *****************************************************************************

FILE NAME

	$RCSfile: SCL.h,v $
	$Revision: 1.13 $
	$Date: 2010/04/05 02:28:33 $

PROGRAM INFORMATION

	Developed by:	SCL project
	Developer:	Douglas Jerome, drj, <douglas@backstep.org>

FILE DESCRIPTION

	Small Container Library: Common Constants, Data Types, etc.

	This file is intended to be an internal SCL header file included by
	other SCL header files.  There may be not much use for SCL client code
	to directly include this file.

CHANGE LOG

	04apr10	drj	Added context pointer to SCL_cbfn_t.

	31mar10	drj	Changed the silly check for NULL being zero to not
			make a warning on 64-bit systems.

	11jun06	drj	Fixed the definition of NULL.

	07jun06	drj	Fixed "ASSERT" to be "SCL_ASSERT".

	06jun06	drj	Fixed my name.  Added some #undef before #define.  Made
			better MSVC and GCC variations of SCL_CHECK_UINT32_1,
			SCL_CHECK_UINT32_2.  Added DEBUG support.

	25sep05	drj	Added _POSIX_C_SOURCE for "<unistd.h>".

	11jul05	drj	Mindless rename of inclusion-control macro.

	02dec04	drj	Conditionally define uint32_t only on not Solaris.

	02dec04	drj	Conditionally define uint32_t only on Linux.

	16nov04	drj	File generation.

***************************************************************************** */


#ifndef SCL_H
#define SCL_H 1


#ifdef	__cplusplus
extern	"C"	{
#endif


/* ************************************************************************* */
/*                                                                           */
/*      I n c l u d e d   F i l e s                                          */
/*                                                                           */
/* ************************************************************************* */

#ifdef	C99
#   include	<stdint.h>
#endif
#ifdef	_unix
#   undef	_POSIX_C_SOURCE
#   define	_POSIX_C_SOURCE 200112L /* posix.1 and posix.4 and MORE */
#   include	<unistd.h>
#endif


/* ************************************************************************* */
/*                                                                           */
/*      M a n i f e s t   C o n s t a n t s                                  */
/*                                                                           */
/* ************************************************************************* */

#ifdef	_WIN32
#   undef	WIN32
#   define	WIN32
#endif

/*
 * Setup proper export, import keyword(s) for some compiler environments.
 * This fun stuff is nicked from SDL (www.libsdl.org).
 */
#undef	DECLS
#ifdef	WIN32
#   ifdef	SCL_EXPORTS
#      define	DECLS	__declspec(dllexport)
#   else
#      define	DECLS	__declspec(dllimport)
#   endif
#else
#   define	DECLS
#endif

/*
 * Setup proper C calling convention keyword(s) for some compiler environments.
 * This fun stuff is nicked from SDL (www.libsdl.org).
 */
#undef	DECLC
#ifdef	WIN32
#   define	DECLC	_cdecl
#else
#   define	DECLC
#endif

#undef	DECLS
#define DECLS
#undef	DECLC
#define DECLC
/*
 * Set up a proper compiler-specific inline directive.  This fun stuff is
 * nicked from SDL (www.libsdl.org).
 */
#undef SCL_INLINE_OKAY
#ifdef __GNUC__
#   define SCL_INLINE_OKAY
#else
#   if defined(_MSC_VER)    || defined(__BORLANDC__) || \
       defined(__DMC__)     || defined(__SC__)       || \
       defined(__WATCOMC__) || defined(__LCC__)	|| defined(__ARMCC_VERSION)
#      define SCL_INLINE_OKAY
#      define __inline__ __inline
#   else
#      if !defined(__MRC__) && !defined(_SGI_SOURCE)
#         define SCL_INLINE_OKAY
#         define __inline__ inline
#      endif
#   endif
#endif
#ifndef SCL_INLINE_OKAY
#   define __inline__
#endif

/*
 * Set up a proper compiler-specific "unused" tag, for tagging unused variables
 * and function arguments.
 */
#if defined( __GNUC__ ) && !defined( __cplusplus ) && !defined(IOS)
#   define	__unused__	__attribute__ ((unused))
#else
#   define	__unused__
#endif

/*
 * Once, I didn't have a NULL.  It is a sad thing to not have a NULL.
 */
#ifndef	NULL
#   ifndef	__cplusplus
#      define	NULL	((void*)0)
#   else
#      define	NULL	(0)
#   endif
#endif

/*
 * This is a compile-time check for NULL being equal to 0.  If NULL is non-zero
 * then this next line should create a compile error.
 */
//typedef int SCL_CHECK_NULL[1-(((long long)NULL)*2)];


/* ************************************************************************* */
/*                                                                           */
/*      D a t a   T y p e s   a n d   S t r u c t u r e s                    */
/*                                                                           */
/* ************************************************************************* */

//extern void*calloc(size_t,size_t);
//extern void free(void*);
#include <stdlib.h>

#define	SCL_ALLOCATOR(s)	(calloc(1,s))
#define	SCL_DEALLOCATOR(x)	(free(x))

/*
 * This enumeration is the collection of return values that can be returned by
 * various SCL functions.
 */
enum
   {
   SCL_OK       = 0,  /* all ok                */
   SCL_ERROR    = -1, /* general error         */
   SCL_BADARG   = -2, /* bad argument          */
   SCL_CORRUPT  = -3, /* internal corruption   */
   SCL_NOMEM    = -4, /* can't allocate memory */
   SCL_NOTFOUND = -5, /* can't find item       */
   SCL_NOSVC    = -6, /* service not available */
   SCL_DUPKEY   = -7  /* duplicate key         */
   };

/*
 *
 */
//#ifndef	SOLARIS
//#ifndef	__uint32_t_defined
//typedef   unsigned int   uint32_t;
//#define	__uint32_t_defined
//#endif
//#endif
/*
 * This is a compile-time check for uint32_t being four bytes in size.  If
 * uint32_t is not four bytes in size, then these next lines should create a
 * compile error.
 */

#if defined(ADS) || defined(_MSC_VER)
typedef int SCL_CHECK_UINT32_1[4-sizeof(uint32_t)+1];
typedef int SCL_CHECK_UINT32_2[sizeof(uint32_t)-4+1];
#else
typedef int SCL_CHECK_UINT32_1[4-sizeof(uint32_t)];
typedef int SCL_CHECK_UINT32_2[sizeof(uint32_t)-4];
#endif

/*
 * Some SCL subsystems make use of client-provided callback functions.  Client
 * code provides function pointers (callbacks) and some SCL subsystems will
 * invoke the client functions via these function callback pointers.  These are
 * the callback function pointer types:
 */
typedef int      (*SCL_cbfn_t)(const char*,long,void*,void*);
typedef uint32_t (*SCL_mdfn_t)(const char*,size_t);
typedef void*    (*SCL_getfn_t)(size_t);
typedef int      (*SCL_putfn_t)(void*,size_t);

/*
 * Usefull commentary goes here.
 */
typedef   void*   SCL_iterator_t;


/* ************************************************************************* */
/*                                                                           */
/*      P u b l i c   G l o b a l   V a r i a b l e s                        */
/*                                                                           */
/* ************************************************************************* */

/* (none) */


/* ************************************************************************* */
/*                                                                           */
/*      F u n c t i o n   P r o t o t y p e s                                */
/*                                                                           */
/* ************************************************************************* */

/* (none) */


/* ************************************************************************* */
/*                                                                           */
/*      D e b u g   S u p p o r t                                            */
/*                                                                           */
/* ************************************************************************* */

#ifdef	_DEBUG
#   undef	DEBUG
#   define	DEBUG
#endif

#undef	SCL_ASSERT
#define	SCL_ASSERT(exp)	((void)0)
#ifdef	DEBUG
#   include	<stdio.h>
#   undef	SCL_ASSERT
#   define	SCL_ASSERT(exp)						\
        {if (!(exp)) {							\
        (void)fflush (stdout);						\
        (void)fflush (stderr);						\
        (void)fprintf(stderr,						\
        "Assertion failed: FILE %s, LINE %u, EXPRESSION \"%s\".\n",	\
        __FILE__,__LINE__,#exp);					\
        (void)fflush (stderr); }}
#endif



#ifdef	__cplusplus
}
#endif


#endif
