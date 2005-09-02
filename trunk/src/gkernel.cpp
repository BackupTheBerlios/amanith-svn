/****************************************************************************
** $file: amanith/src/gkernel.cpp   0.1.0.0   edited Jun 30 08:00
**
** Kernel implementation.
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

#include "amanith/gkernel.h"

// lets include standard plugin for registration
#include "amanith/gelement.h"
#include "amanith/gproperty.h"
#include "amanith/1d/gcurve1d.h"
#include "amanith/1d/gbeziercurve1d.h"
#include "amanith/1d/gbsplinecurve1d.h"
#include "amanith/1d/gmulticurve1d.h"
#include "amanith/1d/gpolylinecurve1d.h"
#include "amanith/1d/ghermitecurve1d.h"
#include "amanith/2d/gpixelmap.h"
#include "amanith/2d/gfont2d.h"
#include "amanith/2d/gmesh2d.h"
#include "amanith/2d/gpath2d.h"
#include "amanith/2d/gcurve2d.h"
#include "amanith/2d/gbeziercurve2d.h"
#include "amanith/2d/gbsplinecurve2d.h"
#include "amanith/2d/gmulticurve2d.h"
#include "amanith/2d/gpolylinecurve2d.h"
#include "amanith/2d/ghermitecurve2d.h"
#include "amanith/gimpexp.h"
#include "amanith/support/gutilities.h"

#if defined(_JPEG_PLUGIN) && defined(_JPEG_PLUGIN_INTERNAL)
	#include "../plugins/jpeg/gjpegimpexp.h"
#endif

#if defined(_PNG_PLUGIN) && defined(_PNG_PLUGIN_INTERNAL)
	#include "../plugins/png/gpngimpexp.h"
#endif

#if defined(_FONTS_PLUGIN) && defined(_FONTS_PLUGIN_INTERNAL)
	#include "../plugins/fonts/gfontsimpexp.h"
#endif

/*!
	\file gkernel.cpp
	\brief Implementation of GKernel class.
*/

namespace Amanith {

typedef GUInt32 (*ProxiesCountFunc)();
typedef GElementProxy* (*ProxyInstanceFunc)(const GUInt32 Index);

// *********************************************************************
//                                GKernel
// *********************************************************************

/*!
	\class GKernel
	\brief It's the core class that make introspection and plugins system possible and full working.

	Kernel must be used for this kind of tasks:\n\n
	- Create a class instance, specifying a class descriptor or a class name. Example:\n\n
\code
GBezierCurve2D *newBezier1 = kernel->CreateNew(G_BEZIERCURVE2D_CLASSID);
GBezierCurve2D *newBezier2 = kernel->CreateNew("GBezierCurve2D");
\endcode
	
	- Load external plugins, using functions like LoadPlugins() or LoadPlugin().\n
	- Automatic memory retrieving: when a kernel is deleted all created class instances are delete. Example:\n\n
\code
void FunFunction() {
	GKernel *newKernel = new GKernel();
	GBezierCurve2D *tmpBezier1 = kernel->CreateNew(G_BEZIERCURVE2D_CLASSID);
	GBezierCurve2D *tmpBezier2 = kernel->CreateNew("GBezierCurve2D");
	// do something with curves...
	...
	delete newKernel;
}
\endcode
	When "delete newKernel" instruction is executed, tmpBezier1 and tmpBezier2 will be automatically freed.\n\n
	- Types introspection: every classes instantiated with a kernel can answer to some useful queries. Example:\n\n
\code

GBezierCurve2D *tmpBezier1 = kernel->CreateNew(G_BEZIERCURVE2D_CLASSID);

if (tmpBezier1->IsOfType(G_CURVE2D_CLASSID)) {
	// now cast is type-safe
	GCurve2D *curve = tmpBezier1;
}

// here's a type-safe cast; after this instruction 'nullFont' variable will be NULL because
// introspection system knows that 'tmpBezier1' variable is not a GFont2D class or
// a GFont2D derived class
GFont2D *nullFont = tmpBezier1->As(G_FONT2D_CLASSID);

// using kernel, every instance can be used as "introspection provider"; this means that a class instanced through
// kernels have the same inspection power of the same kernel
if (tmpBezier1->IsTypeOfType(G_BSPLINECURVE2D_CLASSID, G_ELEMENT_CLASSID))
	printf("I can say without doubts that GBSplineCurve2D class derives from GElement class");

\endcode
*/

/*!
	Register all standard proxies, and scan Amanith's plugin directory to search for external plugins. Each found
	plugin will be automatically loaded.
*/
GKernel::GKernel() : GElement() {

	RegisterProxies();

	GString pluginsDir = SysUtils::AmanithPluginsPath();
	LoadPlugins(StrUtils::ToAscii(pluginsDir), G_TRUE);
}

/*!
	Register all standard proxies, and scan Amanith's plugin directory to search for external plugins. Each found
	plugin will be automatically loaded.
*/
GKernel::GKernel(const GElement* Owner) : GElement(Owner) {

	RegisterProxies();

	GString pluginsDir = SysUtils::AmanithPluginsPath();
	LoadPlugins(StrUtils::ToAscii(pluginsDir), G_TRUE);
}

GError GKernel::BaseClone(const GElement& Source) {
	const GKernel& k = (const GKernel&)Source;
	// just to avoid warnings
	if (k.Owner()) {
	}
	return G_NO_ERROR;
}

// register standard proxies
void GKernel::RegisterProxies() {

	RegisterElementProxy(G_ELEMENT_PROXY);
	RegisterElementProxy(G_ANIMELEMENT_PROXY);
	RegisterElementProxy(G_PROPERTY_PROXY);
	RegisterElementProxy(G_IMPEXP_PROXY);
	RegisterElementProxy(G_KERNEL_PROXY);

	// pixel map plugin
	RegisterElementProxy(G_PIXELMAP_PROXY);

	// jpeg import/export plugin
	#if defined(_JPEG_PLUGIN) && defined(_JPEG_PLUGIN_INTERNAL)
		RegisterElementProxy(G_JPEGIMPEXP_PROXY);
	#endif
	// png import/export plugin
	#if defined(_PNG_PLUGIN) && defined(_PNG_PLUGIN_INTERNAL)
		RegisterElementProxy(G_PNGIMPEXP_PROXY);
	#endif
	// fonts import/export plugin
	#if defined(_FONTS_PLUGIN) && defined(_FONTS_PLUGIN_INTERNAL)
		RegisterElementProxy(G_FONTSIMPEXP_PROXY);
	#endif

	// 1D stuff
	RegisterElementProxy(G_CURVE1D_PROXY);
	RegisterElementProxy(G_BEZIERCURVE1D_PROXY);
	RegisterElementProxy(G_BSPLINECURVE1D_PROXY);
	RegisterElementProxy(G_MULTICURVE1D_PROXY);
	RegisterElementProxy(G_POLYLINECURVE1D_PROXY);
	RegisterElementProxy(G_HERMITECURVE1D_PROXY);
	// 2D stuff
	RegisterElementProxy(G_FONT2D_PROXY);
	RegisterElementProxy(G_PATH2D_PROXY);
	RegisterElementProxy(G_CURVE2D_PROXY);
	RegisterElementProxy(G_BEZIERCURVE2D_PROXY);
	RegisterElementProxy(G_BSPLINECURVE2D_PROXY);
	RegisterElementProxy(G_MULTICURVE2D_PROXY);
	RegisterElementProxy(G_POLYLINECURVE2D_PROXY);
	RegisterElementProxy(G_HERMITECURVE2D_PROXY);

	// animation stuff
	RegisterElementProxy(G_CONSTANTPROPERTY1D_PROXY);
	RegisterElementProxy(G_LINEARPROPERTY1D_PROXY);
	RegisterElementProxy(G_HERMITEPROPERTY1D_PROXY);
}

/*!
	Unload from memory every registered plugins, and free all instanced classes.
*/
GKernel::~GKernel() {

	GInt32 i, j;
	GError err;
	GString s1, s2, msg;
	GElement *elem;

	G_DEBUG("GKernel destructor: destroying undelete instances");
	// check if some instances are already alive. NB: destroying must proceed in sequential order!
	while (gElementsInstances.size() > 0) {
		elem = gElementsInstances.front();
		if (elem)
			delete elem;
	}

	// unload all external plugins
	j = (GInt32)gRegisteredProxies.size();
	G_DEBUG("Gkernel destructor, begin to unload " + StrUtils::ToString(j) + " proxies");

	for (i = 0; i < j; i++) {
		// if proxy has been loaded from an external file (typically a plugin) i have to unload it from memory
		if (gRegisteredProxies[i].gExternal) {
			// unload plugin
			err = gRegisteredProxies[i].gLoader.UnloadPlug();
			if (err != G_NO_ERROR) {
				msg = "Kernel destroying error: unable to unload plugin " + GString(gRegisteredProxies[i].gProxy->ClassID().IDName());
				G_DEBUG(msg);
			}
		}
	}
}

// unload a single plugin from memory
GError GKernel::UnLoadPlugin(const GClassID& PluginClassID) {

	GProxyState p;
	GError err;
	GInt32 i;

	// find a proxy that wraps specified plugin classid
	err = FindProxy(PluginClassID, i);
	// if proxy is not found or it's not external, lets exit; else try to unload it
	if ((err == G_NO_ERROR) && (gRegisteredProxies[i].gExternal)) {
		/*if ((gRegisteredProxies[i].UserInstancesCount() <= 0) &&
			(gRegisteredProxies[i].KernelInstancesCount() <= 0)) {
			err = gRegisteredProxies[i].gLoader.UnloadPlug();
		}
		else
			err = G_PLUGIN_ISTANCED;*/
		err = gRegisteredProxies[i].gLoader.UnloadPlug();
	}
	return err;
}

/*!
	\param Path a string containing the path to scan. It must be valid (non null and existing)
	\param FoundPlugins a string list containing full filename of found plugins during scanning
	\param ScanSubFolders if true specifies that scanning must be done recursively on all included subdirectories
	\return G_NO_ERROR in case of success, else an error code
*/
GError GKernel::ScanDirectory(const GString& Path, GStringList& FoundPlugins, const GBool ScanSubFolders) {

	GStringList tmpList;
	GSystemInfo sInfo;
	GError err;

	SysUtils::SystemInfo(sInfo);
	err = FileUtils::ScanPath(tmpList, StrUtils::ToAscii(Path), ScanSubFolders,
							  G_TRUE, // add full path to file names
							  StrUtils::ToAscii(sInfo.DynamicLibExtension));
	if (err == G_NO_ERROR)
		FoundPlugins = tmpList;
	return err;
}

/*!
	\param FullFileName a string containing a full plugin filename. It must be non NULL.
	\param Filter an optional class descriptor filter. If specified (non NULL), then plugin will be loaded only if
	it derives from the specified class identifier.
	\return G_NO_ERROR in case of success, else an error code
*/
GError GKernel::LoadPlugin(const GChar8 *FullFileName, const GClassID* Filter) {

	GUInt32 i, j, loadedOk;
	GError err, errSym1, errSym2;
	ProxiesCountFunc sym1 = NULL;
	ProxyInstanceFunc sym2 = NULL;
	GElementProxy *proxy;
	const GElementProxy *constProxy;
	GString fName = FullFileName;

	if (fName.length() == 0)
		return G_INVALID_PARAMETER;

	// set library loader
	GPlugLoader loader(FullFileName);
	// we don't wanna automatically unload library from memory
	loader.SetUnloadOnDestroy(G_FALSE);
	// load dynamic library
	err = loader.LoadPlug();

	if (err == G_NO_ERROR) {
		// resolve ProxiesCount() and ProxyInstance() functions
		errSym1 = loader.ResolveSymbol("ProxiesCount", (GPlugSymbolAddress *)&sym1);
		errSym2 = loader.ResolveSymbol("ProxyInstance", (GPlugSymbolAddress *)&sym2);
		if ((errSym1 == G_NO_ERROR) && (errSym2 == G_NO_ERROR)) {
			G_ASSERT((sym1 != NULL) && (sym2 != NULL));
			// lets extract every exported proxies
			j = sym1();
			// trace how many proxy we can register successfully
			loadedOk = 0;
			for (i = 0; i < j; i++) {
				// get i-th proxy
				proxy = sym2(i);
				if (proxy) {
					constProxy = proxy;
					if ((Filter) && (IsTypeOfType(constProxy->ClassID(), *Filter))) {
						err = RegisterElementProxy(*constProxy, G_TRUE, &loader);
						if (err == G_NO_ERROR)
							loadedOk++;
					}
					else {
						err = RegisterElementProxy(*constProxy, G_TRUE, &loader);
						if (err == G_NO_ERROR)
							loadedOk++;
					}
				}
			}
			// if we haven't register one proxy at least, we can unload plugin to retrieve memory
			if (loadedOk == 0)
				// unload dynamic library from memory
				loader.UnloadPlug();
		}
		else
			// unload dynamic library from memory
			loader.UnloadPlug();
	}
	else
		// unload dynamic library from memory
		loader.UnloadPlug();

	return G_NO_ERROR;
}

/*!
	\param PathName a string containing a path to scan for plugins. It must be non NULL.
	\param ScanSubFolders if true specifies that scanning must be done recursively on all included subdirectories
	\param Filter an optional class descriptor filter. If specified (non NULL), then found plugins will be loaded only if
	they derive from the specified class identifier.
	\return G_NO_ERROR in case of success, else an error code
*/
GError GKernel::LoadPlugins(const GChar8 *PathName, const GBool ScanSubFolders, const GClassID* Filter) {

	GError err;
	GStringList list;

	// scan directory for plugins: we wanna a full (path included) filename
	err = ScanDirectory(PathName, list, ScanSubFolders);
	if (err != G_NO_ERROR)
		return err;

	GStringList::const_iterator it;
	for (it = list.begin(); it != list.end(); ++it)
		LoadPlugin(StrUtils::ToAscii(*it), Filter);
	return G_NO_ERROR;
}

/*!
	\param Features a list of import/export features supported by this plugin.
	\return In case of success operation G_NO_ERROR will be returned and features list will be filled, else an error code
*/
GError GKernel::ImpExpFeatures(GDynArray<GImpExpFeature>& Features) const {

	GDynArray<GProxyState> impexpProxies;
	GImpExp *plug;
	GUInt32 i, j;

	Features.clear();
	// get all importer and exporter plugins
	ChildClassProxies(G_IMPEXP_CLASSID, impexpProxies);
	j = (GUInt32)impexpProxies.size();
	for (i = 0; i < j; i++) {
		plug = (GImpExp *)impexpProxies[i].Proxy()->CreateNew(this);
		// abort error: i have a proxy but i can't create it!
		if (!plug)
			return G_UNKNOWN_ERROR;
		// append features
		const GDynArray<GImpExpFeature>& f = plug->Features();
		Features.insert(Features.end(), f.begin(), f.end());
		// delete temporary plugin
		delete plug;
	}
	return G_NO_ERROR;
}

/*!
	\param Features a list of import/export features supported by this plugin.
	\param ID a class descriptor filter. Only features that support this class identifier will be returned
	\return In case of success operation G_NO_ERROR will be returned and features list will be filled, else an error code
*/
GError GKernel::ImpExpFeatures(const GClassID& ID, GDynArray<GImpExpFeature>& Features) const {

	GDynArray<GProxyState> impexpProxies;
	GImpExp *plug;
	GUInt32 i, j, k, w;

	Features.clear();
	// get all importers and exporters plugin
	ChildClassProxies(G_IMPEXP_CLASSID, impexpProxies);
	j = (GUInt32)impexpProxies.size();
	for (i = 0; i < j; i++) {
		plug = (GImpExp *)impexpProxies[i].Proxy()->CreateNew(this);
		// abort error: i have a proxy but i can't create it!
		if (!plug)
			return G_UNKNOWN_ERROR;
		// append features
		const GDynArray<GImpExpFeature>& f = plug->Features();
		w = (GUInt32)f.size();
		for (k = 0; k < w; k++) {
			if (f[k].ClassID() == ID)
				Features.push_back(f[k]);
		}
		// delete temporary plugin
		delete plug;
	}
	return G_NO_ERROR;
}

/*!
	\param FileName is a string containing the full file name (path + name + extension) of the file we wanna read from.
	It must be non-empty.
	\param Element a reference to the element that will be filled with information read from file.
	\param Options a string containing a semicolon-separated options. Each option must be in the format name=value.\n
	(ex: "allowblanks=false;version=3")
	\param FormatName an optional filter; it's case insensitive.
	\return if operation will complete successfully, G_NO_ERROR will be returned, else an error code.
 */
GError GKernel::Load(const GChar8 *FileName, GElement& Element, const GChar8 *Options, const GChar8 *FormatName) {

	GDynArray<GProxyState> impexpProxies;
	GImpExp *plug;
	GUInt32 i, j;
	GError err;
	GBool b1, b2;
	GString fName, fExt;

	// extract file name and extension
	fName = StrUtils::OSFixPath(FileName, G_FALSE);
	fExt = StrUtils::ExtractFileExt(fName);
	// check for a non-empty filename
	if (fName.length() <= 0)
		return G_INVALID_PARAMETER;
	// get all importers and exporters plugin
	ChildClassProxies(G_IMPEXP_CLASSID, impexpProxies);
	// first step: try with format
	j = (GUInt32)impexpProxies.size();
	for (i = 0; i < j; i++) {
		// create a plugin
		plug = (GImpExp *)impexpProxies[i].Proxy()->CreateNew(this);
		// abort error: i have a proxy but i can't create it!
		if (!plug)
			return G_UNKNOWN_ERROR;
		// check for file format support
		b1 = G_FALSE;
		if (FormatName)
			b1 = plug->FormatSupported(FormatName, Element.ClassID(), G_TRUE, G_FALSE);
		// check for file extension support
		b2 = plug->FileExtensionSupported(StrUtils::ToAscii(fExt), Element.ClassID(), G_TRUE, G_FALSE);
		// if some support is guaranteed try to load element
		if (b1 || b2) {
			err = plug->Read(FileName, Element, Options);
			delete plug;
			return err;
		}
		delete plug;
	}
	return G_UNSUPPORTED_CLASSID;
}

/*!
	\param FileName is a string containing the full file name (path + name + extension) of the file we wanna write
	to. It must be non-empty.
	\param Element a reference to the element that will be "saved" to file.
	\param Options a string containing a semicolon-separated options. Each option must be in the format name=value.
	\param FormatName an optional filter; it's case insensitive.
	\return G_NO_ERROR if operation succeeds, an error code otherwise.
*/
GError GKernel::Save(const GChar8 *FileName, const GElement& Element, const GChar8 *Options, const GChar8 *FormatName) {

	GDynArray<GProxyState> impexpProxies;
	GImpExp *plug;
	GUInt32 i, j;
	GError err;
	GBool b1, b2;
	GString fName, fExt;

	// extract file name and extension
	fName = StrUtils::OSFixPath(FileName, G_FALSE);
	fExt = StrUtils::ExtractFileExt(fName);
	// check for a non-empty filename
	if (fName.length() <= 0)
		return G_INVALID_PARAMETER;
	// get all importers and exporters plugin
	ChildClassProxies(G_IMPEXP_CLASSID, impexpProxies);
	// first step: try with format
	j = (GUInt32)impexpProxies.size();
	for (i = 0; i < j; i++) {
		// create a plugin
		plug = (GImpExp *)impexpProxies[i].Proxy()->CreateNew(this);
		// abort error: i have a proxy but i can't create it!
		if (!plug)
			return G_UNKNOWN_ERROR;
		// check for file format support
		b1 = G_FALSE;
		if (FormatName)
			b1 = plug->FormatSupported(FormatName, Element.ClassID(), G_FALSE, G_TRUE);
		// check for file extension support
		b2 = plug->FileExtensionSupported(StrUtils::ToAscii(fExt), Element.ClassID(), G_FALSE, G_TRUE);
		// if some support is guaranteed try to load element
		if (b1 || b2) {
			err = plug->Write(FileName, Element, Options);
			//Delete(plug);
			delete plug;
			return err;
		}
		//Delete(plug);
		delete plug;
	}
	return G_UNSUPPORTED_CLASSID;
}

// return the Index-th proxy
GError GKernel::Proxy(const GInt32 Index, GProxyState& OutProxy) const {

	if ((Index < 0) || (Index >= ProxiesCount()))
		return G_OUT_OF_RANGE;
	OutProxy = gRegisteredProxies[Index];
	return G_NO_ERROR;
}

// register an element proxy
GError GKernel::RegisterElementProxy(const GElementProxy& ElementProxy, const GBool External,
									 const GPlugLoader *Loader) {

	GError err;
	GProxyState p;
	GInt32 i;
	
	// first try if an existing compatible proxy has been already registered
	err = FindProxy(ElementProxy.ClassID(), i);

	if (err == G_PLUGIN_NOTPRESENT) {
		p.gProxy = &ElementProxy;
		p.gExternal = External;
		if (External) {
			G_ASSERT(Loader != NULL);
			p.gLoader = *Loader;
		}
		// add a new proxy to the internal list
		gRegisteredProxies.push_back(p);
		return G_NO_ERROR;
	}
	else {
		G_DEBUG(GString(ElementProxy.ClassID().IDName()) + " already registered!");
		return G_PLUGIN_ALREADYLOADED;
	}
}

// find a registered proxy that match passed classID; return NULL if not found
GError GKernel::FindProxy(const GClassID& Class_ID, GInt32& ProxyIndex) const {

	GInt32 i, j;
	GProxyState p;

	j = (GInt32)gRegisteredProxies.size();
	for (i = 0; i < j; i++) {
		p = gRegisteredProxies[i];
		if (p.Proxy()->ClassID() == Class_ID) {
			ProxyIndex = i;
			return G_NO_ERROR;
		}
	}
	return G_PLUGIN_NOTPRESENT;
}

// find a registered proxy that match passed className
GError GKernel::FindProxy(const GString& Class_Name, GInt32& ProxyIndex) const {

	GInt32 i, j;
	GProxyState p;

	j = (GInt32)gRegisteredProxies.size();
	for (i = 0; i < j; i++) {
		p = gRegisteredProxies[i];
		if (StrUtils::SameText(p.Proxy()->ClassID().IDName(), Class_Name)) {
			ProxyIndex = i;
			return G_NO_ERROR;
		}
	}
	return G_PLUGIN_NOTPRESENT;
}

// get class id from a class name
GError GKernel::ClassIDFromClassName(const GString& Class_Name, GClassID& OutID) const {

	GInt32 i;
	GError err;

	err = FindProxy(Class_Name, i);
	if (err == G_NO_ERROR)
		OutID = gRegisteredProxies[i].Proxy()->ClassID();
	return err;
}

inline bool ElementsPtrCmp(const GElement *Element1, const GElement *Element2) {

	const void *e1 = (const void *)Element1;
	const void *e2 = (const void *)Element2;

	if (e1 < e2)
		return true;
	return false;
}

// create new element specifying a class descriptor
GElement* GKernel::CreateNew(const GClassID& Class_ID) {

	GError err;
	GInt32 i;
	GElement* newElem;

	err = FindProxy(Class_ID, i);
	// try with kernel's owner (kernel clustering chain)
	if (err != G_NO_ERROR) {
		newElem = GElement::CreateNew(Class_ID);
		return newElem;
	}
	else {
		newElem = gRegisteredProxies[i].Proxy()->CreateNew(this);
		return newElem;
	}
}

//! Creation of a new element specifying a class name
GElement* GKernel::CreateNew(const GString& ClassName) {

	if (ClassName.size() == 0)
		return NULL;

	GClassID cID;
	GError err = ClassIDFromClassName(ClassName, cID);

	if (err == G_NO_ERROR)
		return CreateNew(cID);
	else
		return NULL;
}

GError GKernel::AddElementReference(const GElement *Element) {

	if (!Element)
		return G_INVALID_PARAMETER;

	GList<GElement *>::iterator it;
/*
	it = std::lower_bound(gElementsInstances.begin(), gElementsInstances.end(), DeleteElement, ElementsPtrCmp);
	if (it == gElementsInstances.end() || (*it) != DeleteElement)
		gElementsInstances.insert(it, (GElement *)DeleteElement);
*/
	for (it = gElementsInstances.begin(); it != gElementsInstances.end(); ++it)
		if ((*it) == Element)
			return G_NO_ERROR;

	gElementsInstances.push_back((GElement *)Element);
	return G_NO_ERROR;
}

// remove an element reference from the proxy that created it
GError GKernel::RemoveElementReference(const GElement *Element) {

	if (!Element)
		return G_INVALID_PARAMETER;

	GList<GElement *>::iterator it;
/*
	it = std::lower_bound(gElementsInstances.begin(), gElementsInstances.end(), DeleteElement, ElementsPtrCmp);
	if (it != gElementsInstances.end() && (*it) == DeleteElement)
		gElementsInstances.erase(it);
*/
	for (it = gElementsInstances.begin(); it != gElementsInstances.end(); ++it) {
		if ((*it) == Element) {
			gElementsInstances.erase(it);
			return G_NO_ERROR;
		}
	}
	return G_NO_ERROR;
}

// is 'InspectedType' of type (equal or derived) 'Class_ID' ?
GBool GKernel::IsTypeOfType(const GClassID& InspectedType, const GClassID& Class_ID) const {

	GError err;
	GInt32 i;

	// find a proxy that can create the element type
	err = FindProxy(InspectedType, i);
	while (err == G_NO_ERROR) {
		if (gRegisteredProxies[i].Proxy()->ClassID() == Class_ID)
			return G_TRUE;
		// we can't go up further more
		if (gRegisteredProxies[i].Proxy()->IsRootClass())
			return G_FALSE;
		err = FindProxy(gRegisteredProxies[i].Proxy()->DerivedClassID(), i);
	}
	return G_FALSE;
}

/*!
	\param Source the source element
	\param Destination the destination element
	\return If operation will be succeeding G_NO_ERROR will be returned, else an error code.
	\note This function can fail in two cases:\n
	- Source and Destination are of the same type (same ClassID). In this case failure can result from the physical cloning operation
	- Source and Destination are of different type. In this case failure can result because 2 reasons:
		- The Source element cannot be converted into a temporary element of the same class of Destination. Conversion
		is possible thanks to converter plugins. If kernel has not registered a such plugin a G_PLUGIN_NOTPRESENT error
		code will be returned.
		- The Source element can be converted into a temporary element of the same class of Destination, but cloning of
		temporary_object -> Destination has failed.
*/
GError GKernel::Copy(const GElement& Source, GElement& Destination) const {

	//! \todo check if we can choose a converter plugin that support this copy.
	if (Source.ClassID() == Destination.ClassID())
		return Destination.CopyFrom(Source);
	else
		return G_PLUGIN_NOTPRESENT;
}

/*!
	\param ProxyList a list of proxies (well, proxies states) that manage root classes. A root class is a class that not
	derive from other classes (ex: GElement).\n
	\note The list is first cleared, and then filled up with requested proxies.
	\sa Plugin Wizard Tool, you can find it into $AMANITHDIR/tools/plugwizard directory.
*/
void GKernel::RootClassProxies(GDynArray<GProxyState>& ProxyList) const {

	GInt32 i, j;
	GProxyState p;

	ProxyList.clear();
	j = ProxiesCount();
	for (i = 0; i < j; i++) {
		p = gRegisteredProxies[i];
		if (p.Proxy()->IsRootClass())
			ProxyList.push_back(p);
	}
}

/*!
	\param FatherClassID a class descriptor specifying what kind of proxies we wanna retrieve
	\param ChildList a list of proxies that manage classes that derive from FatherClassID.
	\note The list is first cleared, and then filled up with requested proxies.
	\sa Plugin Wizard Tool, you can find it into $AMANITHDIR/tools/plugwizard directory.
*/
void GKernel::ChildClassProxies(const GClassID& FatherClassID, GDynArray<GProxyState>& ChildList) const {

	GInt32 i, j;
	GProxyState p;

	ChildList.clear();
	j = ProxiesCount();
	for (i = 0; i < j; i++) {
		p = gRegisteredProxies[i];
		// we check for parent type and prevent a self classid reference
		if ((p.Proxy()->DerivedClassID() == FatherClassID) && (!p.Proxy()->IsRootClass()))
			ChildList.push_back(p);
	}
}

}
