/****************************************************************************
** $file: amanith/gpluglib.h   0.1.0.0   edited Jun 30 08:00
**
** Low level plugin loader definition.
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

#ifndef GPLUGLIB_H
#define GPLUGLIB_H

#include "amanith/gglobal.h"
#include "amanith/gerror.h"

/*!
	\file gpluglib.h
	\brief Dynamic library loader header file.
*/
namespace Amanith {

	// *********************************************************************
	//                               GPlugLoader
	// *********************************************************************
	typedef void* GPlugHandle;
    typedef void* GPlugSymbolAddress;

	class G_EXPORT GPlugLoader {

    private:
		//! SO handle of the loaded dynamic library
        GPlugHandle gPlugHandle;
		//! Full filename of the loaded dynamic library
		GString gFullPlugName;
		/*!
			Internal flag; if G_TRUE, library will be automatically unloaded when GPlugLoader will be destroyed.\n
			If G_FALSE library will not be unloaded from memory when GPlugLoader will be destroyed.
		*/
		GBool gUnloadOnDestroy;

	protected:
		//! Do the effective loading of FullLibraryName shared library.
		GError LoadFilePlug(const GChar8 *FullLibraryName);

    public:
		//! Constructor
		GPlugLoader(const GChar8 *FullLibraryName = NULL);
		//! Destructor
		~GPlugLoader();
		//! Load external dynamic library
        GError LoadPlug();
		//! Unload dynamic library from memory
		GError UnloadPlug();
		//! Returns G_TRUE if a dynamic library has been loaded by this class, G_FALSE otherwise
		GBool IsPlugLoaded() const;
		//! Returns the full filename of loaded dynamic library.
        GString PlugName() const;
		//! Get the handle of loaded dynamic library
		inline const GPlugHandle PlugHandle() const {
			return gPlugHandle;
		}
		//! Resolve (get a pointer to) a symbol exported by loaded dynamic library
        GError ResolveSymbol(const GChar8 *SymbolName, GPlugSymbolAddress *ResolvedSymbol) const;
		//! Get 'unload on destroy' flag
		inline GBool UnloadOnDestroy() const {
			return gUnloadOnDestroy;
		}
		//! Set 'unload on destroy' flag
		inline void SetUnloadOnDestroy(const GBool MustUnload) {
			gUnloadOnDestroy = MustUnload;
		}
	};

};	// end namespace Amanith

#endif
