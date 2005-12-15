/****************************************************************************
** $file: amanith/src/gpluglib.h   0.2.0.0   edited Dec, 12 2005
**
** Low level plugin loader implementation.
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

#include "amanith/gpluglib.h"
#include "amanith/support/gutilities.h"

#if defined(G_OS_WIN) && !defined(__CYGWIN__)
	#include <io.h>
	#include <windows.h>
	#include <winbase.h>
#elif defined(G_OS_MAC)
	#include <dlfcn.h>
#else // all other unix
	#include <unistd.h>
	#ifdef G_OS_HPUX
		// Although HP-UX has dlopen() it is broken! We therefore need to stick
		// to shl_load()/shl_unload()/shl_findsym()
		#include <dl.h>
		#include <errno.h>
	#else
		#include <dlfcn.h>
	#endif
#endif

/*!
	\file gpluglib.cpp
	\brief Implementation of GPlugLoader class (dynamic library loader).
*/

namespace Amanith {

/*!
	\class GPlugLoader
	\brief Low level dynamic library loader class.

	The GPlugLoader class provides a wrapper for handling shared libraries.\n
	An instance of a GPlugLoader object can handle a single shared library and provide access to the functionality in the
	library in a platform independent way.
	GPlugLoader can also unload the library on destruction and release unused resources.\n
	A typical use of GPlugLoader is to resolve an exported symbol in a shared object, and to call the function that
	this symbol represents.	The following code loads a library, resolves the symbol "ProxiesCount", and calls the function.

\code
	typedef GUInt32 (*ProxiesCountFunc)();
	GError err;
	ProxiesCountFunc dllFunc;
	GPlugLoader myDLL("some.dll");
	
	// load dynamic library
	err = myDLL.LoadPlug();
	if (err == G_NO_ERROR) {
		err = myDLL.ResolveSymbol("ProxiesCount", (GPlugSymbolAddress *)&dllFunc);
		if (err == G_NO_ERROR)
			GUInt32 proxCount = dllFunc();
	}
}
\endcode
	\n When the GPlugLoader object is destroyed the library will be unloaded, making all references to memory
	allocated in the library invalid.
*/


/*!
	Initialize internal fields: set library handle to NULL, copy specified library filename and set the
	'UnloadOnDestroy' flag to G_TRUE.
*/
GPlugLoader::GPlugLoader(const GChar8 *FullLibraryName) {

	gPlugHandle = NULL;
	gUnloadOnDestroy = G_TRUE;
	if (FullLibraryName)
		// taken from MSDN: "When specifying a path, be sure to use backslashes (\), not forward slashes (/)."
		// so we must insure a "good" path depending on OS
		gFullPlugName = StrUtils::OSFixPath(FullLibraryName);
	else
		gFullPlugName = "";
}

/*!
	If 'UnloadOnDestroy' flag is G_TRUE, then library (if loaded) will be unloaded from memory.
*/
GPlugLoader::~GPlugLoader() {

	if (gUnloadOnDestroy)
		UnloadPlug();
}

GBool GPlugLoader::IsPlugLoaded() const {

	if (gPlugHandle)
		return G_TRUE;
	return G_FALSE;
}

GString GPlugLoader::PlugName() const {

	return gFullPlugName;
}

/*!
	This function is the one that allows to load the dynamic library whose filename has been set with constructor method.
	\return If the operation will succeed G_NO_ERROR constant will be returned, else an error code.
	\note This function can fail for different reasons, for example:
	- The specified filename in the constructor is an empty (or invalid) string; in this case a G_INVALID_PARAMETER error
	code will be returned.
	- The specified filename in the constructor does not refer to an existing filename; in this case a
	G_FILE_NOT_FOUND error code will be returned.
	- The specified filename in the constructor does refer to an existing filename, but it's not a dynamic library; in
	this case a G_INVALID_FORMAT error code will be returned.
*/
GError GPlugLoader::LoadPlug() {

	// we don't wanna an empty filename
	if (gFullPlugName.length() <= 0)
		return G_INVALID_PARAMETER;
	// test if file do exist
	if (!FileUtils::FileExists(StrUtils::ToAscii(gFullPlugName)))
		return G_FILE_NOT_FOUND;
	// now try to load shared library
	return LoadFilePlug(StrUtils::ToAscii(gFullPlugName));
}

GError GPlugLoader::LoadFilePlug(const GChar8 *FullLibraryName) {

    GPlugHandle handle = NULL;

	// we don't wanna a null filename
	if (!FullLibraryName)
		return G_INVALID_PARAMETER;
	// unplug current library
	UnloadPlug();

	#if defined(G_OS_WIN) && !defined(__CYGWIN__)
		handle = LoadLibraryA(FullLibraryName);
	#elif defined(G_OS_HPUX)
		// BIND_FIRST is necessary for some reason
		handle = shl_load(FullLibraryName, BIND_DEFERRED | BIND_FIRST | BIND_VERBOSE, 0);
	// other Unix (it works also on MacOSX and Tiger)
	#else 
		handle = dlopen(FullLibraryName, RTLD_LAZY | RTLD_GLOBAL);
		// dlopen will not work with files in the current directory unless
		// they are prefaced with './'  (DB - Nov 5, 2003).
		// So we must check if file was expressed as a local file (without path)
		GString fPath = StrUtils::ExtractFilePath(FullLibraryName);
		if ((!handle) && (fPath.length() <= 0)) {
			GString localLibraryName = GString("./") + GString(FullLibraryName);
			handle = dlopen(StrUtils::ToAscii(localLibraryName), RTLD_LAZY | RTLD_GLOBAL);
		}
	#endif

	if (!handle)
		return G_INVALID_FORMAT;

	gPlugHandle = handle;
	return G_NO_ERROR;
}

GError GPlugLoader::UnloadPlug() {

    if (gPlugHandle) {
		#if defined(G_OS_WIN) && !defined(__CYGWIN__)
			FreeLibrary((HMODULE)gPlugHandle);
		#elif defined(G_OS_HPUX)
			// fortunately, shl_t is a pointer
			shl_unload(static_cast<shl_t>(gPlugHandle));
		#else // other unix (it works also on MacOSX and Tiger)
			dlclose(gPlugHandle);
		#endif    
		gPlugHandle = NULL;
		return G_NO_ERROR;
    }
	return G_PLUGIN_NOTLOADED;
}

GError GPlugLoader::ResolveSymbol(const GChar8 *SymbolName, GPlugSymbolAddress *ResolvedSymbol) const {

	void *result = NULL;

	if ((!SymbolName) || (!ResolvedSymbol))
		return G_INVALID_PARAMETER;
    if (!gPlugHandle)
		return G_PLUGIN_NOTLOADED;
	#if defined(G_OS_WIN) && !defined(__CYGWIN__)
		result = (GPlugSymbolAddress)GetProcAddress((HMODULE)gPlugHandle, SymbolName);
		if (!result)
			return G_PLUGIN_SYMBOL_UNRESOLVED;
		*ResolvedSymbol = result;
		return G_NO_ERROR;
	#elif defined(G_OS_HPUX)
		if (shl_findsym (reinterpret_cast<shl_t*>(&gPlugHandle), SymbolName, TYPE_PROCEDURE, result) == 0) {
			*ResolvedSymbol = result;
			return G_NO_ERROR;
		}
		return G_PLUGIN_SYMBOL_UNRESOLVED;
	#else // other unix (it works also on MacOSX and Tiger)
		result = dlsym(gPlugHandle,  SymbolName);
		if (!result)
			return G_PLUGIN_SYMBOL_UNRESOLVED;
		*ResolvedSymbol = result;
		return G_NO_ERROR;
	#endif
}

}
