/****************************************************************************
** $file: amanith/gkernel.h   0.3.0.0   edited Jan, 30 2006
**
** Kernel definition.
**
**
** Copyright (C) 2004-2006 Mazatech Inc. All rights reserved.
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

#ifndef GKERNEL_H
#define GKERNEL_H

#include "amanith/gelement.h"
#include "amanith/gpluglib.h"
#include "amanith/gimpexp.h"

/*!
	\file gkernel.h
	\brief GKernel class header file.
*/
namespace Amanith {

	// *********************************************************************
	//                                GProxyState
	// *********************************************************************

	/*!
		\class GProxyState
		\brief Proxy state descriptor.

		GProxyState class maintains the state of a proxy. State description is made of several fields:

		Proxy: a reference to the proxy that this class maintains the state of.
		External: flag, if true it means that proxy has been loaded from an external file (typically a plugin).
		Loader: if proxy has been loaded from an external file, this field is an instance of a GPlugLoader that has been
		used to load file.
	*/
	class G_EXPORT GProxyState {

	friend class GKernel;

	private:
		//! Proxy pointer
		const GElementProxy *gProxy;
		//! If true, it means that plugin was loaded from an external file
		GBool gExternal;
		//! Dynamic plugin loader (valid for external plugins only)
		GPlugLoader gLoader;

	public:
		//! Constructor
		GProxyState() {
			gProxy = NULL;
			gExternal = G_FALSE;
		}
		//! Get proxy pointer
		inline const GElementProxy *Proxy() const {
			return gProxy;
		}
		// If true it means that proxy has been loaded from an external file (typically a plugin)
		inline GBool External() const {
			return gExternal;
		}
	};


	// *********************************************************************
	//                                GKernel
	// *********************************************************************

	//! GKernel static class descriptor.
	static const GClassID G_KERNEL_CLASSID = GClassID("GKernel", 0x7B1633B2, 0xCE4C4F0E, 0xBB6227F2, 0x77FB8EF5);

	class G_EXPORT GKernel : public GElement {

	friend class GElement;

	private:
		//! List of all registered proxy
		GDynArray<GProxyState> gRegisteredProxies;
		//! List of created instances
		GList<GElement *> gElementsInstances;

		//! Register all standard proxies
		void RegisterProxies();

	protected:
		//! Scan a directory searching for plugins
		GError ScanDirectory(const GString& Path, GStringList& FoundPlugins, const GBool ScanSubFolders);
		//! Register a single element proxy
		GError RegisterElementProxy(const GElementProxy& ElementProxy, const GBool External = G_FALSE,
									const GPlugLoader *Loader = NULL);
		//! Find a registered proxy that manage elements of specified class descriptor
		GError FindProxy(const GClassID& Class_ID, GInt32& ProxyIndex) const;
		//! Find a registered proxy that manage elements of specified class name
		GError FindProxy(const GString& Class_Name, GInt32& ProxyIndex) const;
		//! Clone function
		GError BaseClone(const GElement& Source);
		//! Remove an element reference from the the internal instances list
		GError RemoveElementReference(const GElement *Element);
		//! Add an element reference inside the internal instances list
		GError AddElementReference(const GElement *Element);

	public:
		//! Default constructor
		GKernel();
		//! Constructor with owner parameter
		GKernel(const GElement* Owner);
		//! Destructor
		~GKernel();
		//! Get class descriptor from a class name
		GError ClassIDFromClassName(const GString& Class_Name, GClassID& OutID) const;
		//! Copy (physically) a source element into a destination one
		GError Copy(const GElement& Source, GElement& Destination) const;
		//! Creation of a new element: here's the effective implementation that differs from every other GElement
		GElement* CreateNew(const GClassID& Class_ID);
		//! Creation of a new element specifying a class name
		GElement* CreateNew(const GString& ClassName);
		//! Is 'InspectedType' of type (equal or derived) 'Class_ID' ?
		GBool IsTypeOfType(const GClassID& InspectedType, const GClassID& Class_ID) const;
		//! Ask for a supported element type.
		GBool IsSupported(const GClassID& Class_ID) const;
		//! Load an external plugin, specifying a filter class descriptor
		GError LoadPlugin(const GChar8 *FullFileName, const GClassID* Filter = NULL);
		//! Scan a directory for plugins, and load them
		GError LoadPlugins(const GChar8 *PathName, const GBool ScanSubFolders, const GClassID* Filter = NULL);
		//! Unload a single plugin from memory
		GError UnLoadPlugin(const GClassID& PluginClassID);
		//! Load an element from an external file
		GError Load(const GChar8 *FileName, GElement& Element,
					const GChar8 *Options, const GChar8 *FormatName = NULL);
		//! Save an element to an external file
		GError Save(const GChar8 *FileName, const GElement& Element,
					const GChar8 *Options, const GChar8 *FormatName = NULL);
		//! Retrieve import and export features supported by this kernel
		GError ImpExpFeatures(GDynArray<GImpExpFeature>& Features) const;
		//! Retrieve import and export features for a class ID supported by this kernel
		GError ImpExpFeatures(const GClassID& ID, GDynArray<GImpExpFeature>& Features) const;
		//! Return number of registered proxies
		inline GInt32 ProxiesCount() const {
			return (GInt32)gRegisteredProxies.size();
		}
		//! Get the Index-th proxy
		GError Proxy(const GInt32 Index, GProxyState& OutProxy) const;
		//! Return registered proxies list
		inline const GDynArray<GProxyState>& Proxies() const {
			return gRegisteredProxies;
		}
		//! Get all root-class proxies
		void RootClassProxies(GDynArray<GProxyState>& ProxyList) const;
		//! Get all proxies that manage classes derived from the one specified as first parameter
		void ChildClassProxies(const GClassID& FatherClassID, GDynArray<GProxyState>& ChildList) const;
		//! Get all proxies that manage classes derived from the one managed by proxy specified as first parameter
		inline void ChildClassProxies(const GElementProxy& FatherProxy, GDynArray<GProxyState>& ChildList) const {
			ChildClassProxies(FatherProxy.ClassID(), ChildList);
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_KERNEL_CLASSID;
		}
	};


	// *********************************************************************
	//                                GKernelProxy
	// *********************************************************************
	/*!
		\class GKernelProxy
		\brief This class implements a GKernel proxy (provider).
	*/
	class G_EXPORT GKernelProxy : public GElementProxy {
	public:
		//! Create a new GKernel class
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GKernel(Owner);
		}
		// get derived class ID
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy
		const GClassID& ClassID() const {
			return G_KERNEL_CLASSID;
		}
	};

	//! Static proxy for GKernel class.
	static const GKernelProxy G_KERNEL_PROXY;

};	// end namespace Amanith

#endif
