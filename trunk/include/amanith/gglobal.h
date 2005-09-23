/****************************************************************************
** $file: amanith/gglobal.h   0.1.1.0   edited Sep 24 08:00
**
** Global types and constants.
**
**
** Copyright (C) 2004-2005 Mazatech Inc. All rights reserved.
**
** This file is part of Amanith Framework.
**
** This file may be distributed and/or modified under the terms of the Q Public License
** as defined by Mazatech Inc. of Italy and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Amanith Professional Edition license may use this file in
** accordance with the Amanith Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.mazatech.com or email sales@mazatech.com for
** information about Amanith Commercial License Agreements.
** See http://www.amanith.org/ for opensource version, public forums and news.
**
** Contact info@mazatech.com if any conditions of this licensing are
** not clear to you.
**********************************************************************/

#ifndef GGLOBAL_H
#define GGLOBAL_H

/*!
	\file gglobal.h
	\brief Global Amanith types and constants defintion.
*/

// try to make Amanith framework Qt-independent...maybe in the future another framework
// could/must be used
#include <cstddef>
#include <string>
#include <vector>
#include <list>
#include <iostream>


//! Very simple assertion stuff
#ifdef _DEBUG
	#include <assert.h>
	#define G_ASSERT(val) assert((val))
#else
	#define G_ASSERT(val) ((void)0)
#endif

//! Very simple debugging stuff
#ifdef _DEBUG
	#define G_DEBUG(val) \
		std::cerr << (std::string("G_DEBUG: ") + std::string(val).c_str() + "\n")
#else
	#define G_DEBUG(val) ((void)0)
#endif


/*!
	\namespace Amanith
	\brief Amanith framework
*/
namespace Amanith {

	// G_VERSION is (major << 24) + (minor << 16) + (patch << 8) + build
	#define G_AMANITH_VERSION 0x00010000
	#define G_VERSION_STR "0.1.0.0"

	//! English language option
	#define G_LANG_EN     0
	//! Italian language option
	#define G_LANG_IT     1
	//! German language option
	#define G_LANG_DE     2
	//! French language option
	#define G_LANG_FR     3
	//! Spanish language option
	#define G_LANG_ES     4
	//! Chinese language option
	#define G_LANG_CN     5
	//! Japanese language option
	#define G_LANG_JP     6
	//! Russian language option
	#define G_LANG_RU     7
	//! Suomi language option
	#define G_LANG_FI     8
	//! Used language for error codes and strings
	#define G_USED_LANG   G_LANG_EN


    //! Indexes enumeration for single component access
	enum GVectorIndex {
		//! X component
		G_X = 0,
		//! Y component
		G_Y = 1,
		//! Z component
		G_Z = 2,
		//! W component
		G_W = 3
	};

	//! Type used to specify in witch order Euler transforms will be applied (ex: rotations)
	enum GEulerOrder {
		G_XYX = 0,
		G_XYZ = 1,
		G_XZX = 2,
		G_XZY = 3,
		G_YXY = 4,
		G_YXZ = 5,
		G_YZX = 6,
		G_YZY = 7,
		G_ZXY = 8,
		G_ZXZ = 9,
		G_ZYX = 10,
		G_ZYZ = 11
	};
	//! Sort options
	enum GSortType {
		//! No sort
		G_NO_SORT,
		//! Ascending sort
		G_ASCENDING_SORT,
		//! Descending sort
		G_DESCENDING_SORT
	};
	//! Relative position of an entity respect to a surface
	enum GSurfaceSide {
		//! On surface
		G_ONSURFACE,
		//! Inside surface
		G_INSIDE,
		//! Outside surface
		G_OUTSIDE
	};
	//! Box side enumeration constants
	enum GBoxSide {
		//! Left box side
		G_LEFTSIDE,
		//! Right box side
		G_RIGHTSIDE,
		//! Top box side
		G_TOPSIDE,
		//! Bottom box side
		G_BOTTOMSIDE,
		//! Front box side
		G_FRONTSIDE,
		//! Back box side
		G_BACKSIDE
	};

	// BeOS (Beos is not detected by Qt, so it's included outer)
	#if defined(__BEOS__)
		#define G_OS_BEOS
	#endif

	// Codewarrior doesn't define any Windows symbols until some headers are included
	#if __MWERKS__
		#include <stddef.h>
	#endif
	// Darwin OS (Without Mac OS X)
	#if defined(__DARWIN_X11__)
		#define G_OS_DARWIN
	// MacOS X
	#elif defined(__APPLE__) && defined(__GNUC__)
		#define G_OS_MACX
	#elif defined(__MACOSX__)
		#define G_OS_MACX
	// MacOS 9
	#elif defined(macintosh)
		#define G_OS_MAC9
	// Free BSD
	#elif defined(__FreeBSD__)
		#define G_OS_FREEBSD
	// Net BSD
	#elif defined(__NetBSD__)
		#define G_OS_NETBSD
	// Open BSD
	#elif defined(__OpenBSD__)
		#define G_OS_OPENBSD
	// BSD / OS
	#elif defined(__bsdi__)
		#define G_OS_BSDI
	// SGI Irix
	#elif defined(__sgi)
		#define G_OS_IRIX
	// IBM AIX
	#elif defined(_AIX)
		#define G_OS_AIX
	// Win64 core
	#elif !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
		#define G_OS_WIN32
		#define G_OS_WIN64
	// Win64 core (Windows 95/98/ME/NT/2000/XP)
	#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
		#define G_OS_WIN32
		// some old libraries/application test for _WIN32 instead of the standard WIN32
		#ifndef _WIN32
			#define _WIN32
		#endif
	#elif defined(__MWERKS__) && defined(__INTEL__)
		#define G_OS_WIN32
	// Linux
	#elif defined(__LINUX__) || defined(__linux__) || defined(__linux)
		#define G_OS_LINUX
	// Sun SOLARIS
	#elif defined(__sun) || defined(sun) || defined(__SUN__)
		#define G_OS_SOLARIS
	// HP-UX (the standard macro is __hpux)
	#elif defined(__hpux) || defined(__hpux__)
		#define G_OS_HPUX
	#endif

	// merge macros for Windows and Mac
	#if defined(G_OS_WIN32) || defined(G_OS_WIN64)
		#define G_OS_WIN
		// impose that minimum requirements are Windows 2000 for NT kernels, and Windows 98
		// for 9x kernels
		#define _WIN32_WINNT 0x0500
		#define _WIN32_WINDOWS 0x0410
	#endif
	#if defined(G_OS_MAC9) || defined(G_OS_MACX)
		#define G_OS_MAC
	#endif

	// UNIX like system
	#if defined(__UNIX__) || defined(__unix) || defined(__unix__) || \
		defined(G_OS_LINUX) || defined(G_OS_IRIX) || \
		defined(G_OS_HPUX) || defined(G_OS_SOLARIS) || defined(G_OS_AIX) ||  defined(G_OS_BEOS)
			#define G_OS_UNIXLIKE
	#endif
	// BSD systems
	#if defined(G_OS_FREEBSD) || defined(G_OS_NETBSD) || defined(G_OS_OPENBSD) || \
		defined(G_OS_BSDI)
			#define G_OS_BSDLIKE
	#endif

	// basic platform-indipendent types

	//! String type
	#define GString std::string
	//! Integer type, 1 byte size
	#define GInt8 signed char
	//! Integer type, 2 bytes size
	#define GInt16 short
	//! Integer type, 4 bytes size
	#define GInt32 int
	//! Unsigned integer type, 1 byte size
	#define GUInt8 unsigned char
	//! Unsigned integer type, 2 bytes size
	#define GUInt16 unsigned short
	//! Unsigned integer type, 4 bytes size
	#define GUInt32 unsigned int
	#if defined(G_OS_WIN64)
		//! Unsigned long type
		#define GULong unsigned __int64
		//! Signed long type
		#define GLong __int64
	#else
		//! Unsigned long type
		#define GULong unsigned long
		//! Signed long type
		#define GLong long
	#endif
	#if defined(G_OS_WIN) && !defined(__GNUC__)
		//! Signed integer type, 8 bytes size
		#define GInt64 __int64
		//! Unsigned integer type, 8 bytes size
		#define GUInt64 unsigned __int64
	#else
		//! Signed integer type, 8 bytes size
		#define GInt64 long long
		//! Unsigned integer type, 8 bytes size
		#define GUInt64 unsigned long long
	#endif
	//! Signed long long type
	typedef GInt64 GLLong;
	//! Unsigned long long type
	typedef GUInt64 GULLong;
	//! Dynamic vector container
	#define GDynArray std::vector
	//! List container
	#define GList std::list
	//! String list (a list of strings)
	#define GStringList std::list<GString>
	//! Boolean type
	#define GBool bool
	//! Float type
	#define GFloat float
	//! Double precision type
	#define GDouble double
	//! 8-bit signed char type
	#define GChar8 char
	//! 8-bit unsigned char type (sometime known as byte)
	typedef GUInt8 GUChar8;
	//! Error type. As a general rule, all error values are negative numbers
	typedef GInt32 GError;

	#ifdef DOUBLE_REAL_TYPE
		//! Scalar type used for geometry types (point, matrix, quaternion, and so on)
		typedef GDouble GReal;
		//! Tolerance constants for equal comparisons (smallest number such that 1 + G_EPSILON != 1)
		const GReal G_EPSILON = 2.2204460492503131e-16;                   
		//! Maximum GReal value
		const GReal G_MAX_REAL = 1.7976931348623157e+308;
		//! Minimum GReal value
		const GReal G_MIN_REAL = -1.7976931348623157e+308;
	#else
		//! Scalar type used for geometry types (point, matrix, quaternion, and so on)
		typedef GFloat GReal;
		//! Tolerance constants for equal comparisons (smallest number such that 1 + G_EPSILON != 1)
		const GReal G_EPSILON = 1.1920928955078125e-07;
		//! Maximum GReal value
		const GReal G_MAX_REAL = 3.4028234663852886e+38;
		//! Minimum GReal value
		const GReal G_MIN_REAL = -3.4028234663852886e+38;
	#endif

	//! time value type
	typedef GReal GTimeValue;

	//! Maximum signed GInt8 value
	#define G_MAX_INT8 127
	//! Minimum signed GInt8 value
	#define G_MIN_INT8 -128
	//! Maximum signed GInt16 value
	#define G_MAX_INT16 32767
	//! Minimum signed GInt16 value
	#define G_MIN_INT16 -32768
	//! Maximum signed GInt32 value
	#define G_MAX_INT32 2147483647
	//! Minimum signed GInt32 value
	#define G_MIN_INT32 -2147483648
	//! Maximum signed GLong value
	#define G_MAX_LONG 2147483647
	//! Minimum signed GLong value
	#define G_MIN_LONG -2147483648
	//! Maximum signed GInt64 value
	#define G_MAX_INT64 9223372036854775807
	//! Minimum signed GInt64 value
	#define G_MIN_INT64 -9223372036854775808
	//! Maximum unsigned GUInt8 value
	#define G_MAX_UINT8 255
	//! Maximum unsigned GUInt16 value
	#define G_MAX_UINT16 65535
	//! Maximum unsigned GUInt32 value
	#define G_MAX_UINT32 4294967295
	//! Maximum unsigned GULong value
	#define G_MAX_ULONG 4294967295
	//! Maximum unsigned GUInt64 value
	#define G_MAX_UINT64 0xffffffffffffffff
	//! Maximum signed GFloat value
	#define G_MAX_FLOAT 3.4028234663852886e+38
	//! Minimum signed GFloat value
	#define G_MIN_FLOAT -3.4028234663852886e+38
	//! Maximum signed GDouble value
	#define G_MAX_DOUBLE 1.7976931348623157e+308
	//! Minimum signed GDouble value
	#define G_MIN_DOUBLE -1.7976931348623157e+308

	//! True boolean value used by GBool type
	#define G_TRUE  true
	//! False boolean value used by GBool type
	#define G_FALSE false

	// dynamic library build: platform-specific settings
	#if defined(G_OS_WIN) && !defined(__CYGWIN__)
	#  if defined(G_NO_DLL)
	#    undef G_MAKE_DLL
	#    undef G_USE_DLL
	#  elif defined(G_MAKE_DLL)	// create a Amanith DLL library
	#    if defined(G_USE_DLL)
	#      undef G_USE_DLL
	#    endif
	#    define G_EXPORT  __declspec(dllexport)
	#  elif defined(G_USE_DLL)		// use a Amanith DLL library
	#    define G_EXPORT  __declspec(dllimport)
	#  endif
	#else
	#  undef G_MAKE_DLL		// ignore these for other platforms
	#  undef G_USE_DLL
	#endif
	#ifndef G_EXPORT
		#define G_EXPORT
	#endif

	#ifndef G_EXTERN_C
		#ifdef __cplusplus
			#define G_EXTERN_C extern "C"
		#else
			#define G_EXTERN_C extern
		#endif
	#endif

	#ifndef G_EXTERN
		#define G_EXTERN extern
	#endif


	// dynamic plugin build: platform-specific settings
	#if defined(G_OS_WIN) && !defined(__CYGWIN__)
	#  if defined(G_MAKE_PLUGIN)
	#    if defined(G_USE_PLUGIN)
	#      undef G_USE_PLUGIN
	#    endif
	#    define G_PLUGIN_EXPORT  __declspec(dllexport)
	#  elif defined(G_USE_PLUGIN)
	#    define G_PLUGIN_EXPORT  __declspec(dllimport)
	#  endif
	#else
	#  undef G_MAKE_PLUGIN
	#  undef G_USE_PLUGIN
	#endif
	#ifndef G_PLUGIN_EXPORT
		#define G_PLUGIN_EXPORT
	#endif

	//! Endian types
	enum GEndianType {
		//! Big endian order (Motorola, PowerPC)
		G_BIG_ENDIAN,
		//! Little endian order (Intel)
		G_LITTLE_ENDIAN
	};

	//! Structure used to store system informations
	struct GSystemInfo {
		//! Word size, expressed in bits
		GUInt32 WordSize;
		//! Endian type
		GEndianType EndianType;
		//! Delimiter character used to delimit paths
		GChar8 TrailerPathDelimiter;
		//! Extension used by dynamic library files
		GString DynamicLibExtension;
		//! Operating system minor version
		GInt32 OSMinorVersion;
		//! Operating system major version
		GInt32 OSMajorVersion;
		//! Operating system build number
		GInt32 OSBuildNumber;
	};

	// just for safe float operations
	#if defined(G_OS_DARWIN) || defined(G_OS_MAC) || defined(G_OS_HPUX) || defined(G_OS_SOLARIS) || \
		defined(_XOPEN_SOURCE) || defined(G_OS_LINUX) || defined(G_OS_BSDLIKE) || defined(G_OS_IRIX) || \
		defined(__CYGWIN__) || defined(__GNUC__)
		#define G_NO_ACOSF 1
		#define G_NO_ASINF 1
		#define G_NO_TANF 1
		#define G_NO_ATAN2F 1
		#define G_NO_COSF 1
		#define G_NO_SINF 1
		#define G_NO_TANF 1
		#define G_NO_SQRTF 1
		#define G_NO_LOGF 1
		#define G_NO_EXPF 1
		#define G_NO_POWF 1
		#define G_NO_CEILF 1
		#define G_NO_FLOORF 1
	#endif

	// some VisualC compilers annoying warning
	#if defined(_MSC_VER)
		#pragma warning( disable : 4251 )
	#endif
};	// end namespace Amanith

#endif
