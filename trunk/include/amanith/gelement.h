/****************************************************************************
** $file: amanith/gelement.h   0.1.0.0   edited Jun 30 08:00
**
** Base elements definition.
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

#ifndef GELEMENT_H
#define GELEMENT_H

#include "amanith/gclassid.h"
#include "amanith/geometry/ginterval.h"
#include "amanith/geometry/gvect.h"
#include "amanith/gmath.h"
#include "amanith/support/gutilities.h"

/*!
	\file gelement.h
	\brief GElement header file.
*/

namespace Amanith {

	// *********************************************************************
	//                            GTimeInterval
	// *********************************************************************

	//! Time interval, it's an interval where values are of type GTimeValue
	typedef GInterval<GTimeValue> GTimeInterval;
	//! Time interval with the maximum possible length
	static const GTimeInterval G_FOREVER_TIMEINTERVAL = GTimeInterval(-G_MAX_INT32, G_MAX_INT32);
	//! An empty interval
	static const GTimeInterval G_NEVER_TIMEINTERVAL = GTimeInterval(-G_MAX_INT32, -G_MAX_INT32);


	// *********************************************************************
	//                               GElement
	// *********************************************************************

	//! GElement static class descriptor.
	static const GClassID G_ELEMENT_CLASSID = GClassID("GElement", 0x99699BEC, 0xD171411E, 0xAB48F5DA, 0xD9083C2A);

	class G_EXPORT GElement {

	private:
		//! Kernel, owner of this element
		const GElement* gOwner;

	protected:
		//! Creates a new element given a class id.\n If element can't be created a NULL pointer is returned.
		GElement* CreateNew(const GClassID& Class_ID) const;
		//!	Get class identifier from a class name.
		GError ClassIDFromClassName(const GString& Class_Name, GClassID& OutID) const;
		// Creates a new element given a class name.
		GElement* CreateNew(const GString& Class_Name);
		/*!
			Base clone function. Every derived class must implement this function.\n Source parameter is ensured to be
			of the same class, so casting made inside this function is type-safe. The typical implementation uses a chain
			propagation (towards father classes), relegating this function to the simple copy of specific internal state.\n
			Here's and example taken from GHermiteCurve2D class (this class derives from GMultiCurve2D):
\code
	GError GHermiteCurve2D::BaseClone(const GElement& Source) {

		const GHermiteCurve2D& src = (const GHermiteCurve2D&)Source;

		// copy keys array
		gKeys = src.gKeys;
		// continue cloning passing the control to base (father) class
		return GMultiCurve2D::BaseClone(Source);
}
\endcode
		*/
		virtual GError BaseClone(const GElement& Source) {
			// just to avoid warnings
			if (Source.Owner())
				return G_NO_ERROR;
			return G_NO_ERROR;
		}

	public:
		//! Default constructor, set owner to NULL
		GElement();
		//! Constructor, specifying owner
		GElement(const GElement* Owner);
		//! Destructor
		virtual ~GElement();
		//! Is an element of a specified type?\n It does include inheritance inspection (using owner / kernel)
		GBool IsOfType(const GClassID& Class_ID) const;
		/*!
			Type-safe cast.\n This function cast this element to the type specified by Class_ID parameter.
			If cast can be done (it means that this element is of Class_ID type or is derived by Class_ID type), a
			non-null element pointer (equal to 'this') is returned. If cast cannot be done a NULL pointer is returned.
		*/
		GElement* As(const GClassID& Class_ID) {
			if (IsOfType(Class_ID))
				return this;
			return NULL;
		}
		/*!
			Type-safe cast (const version).\n This function cast this element to the type specified by Class_ID parameter.
			If cast can be done (it means that this element is of Class_ID type or is derived by Class_ID type), a
			non-null element pointer (equal to 'this') is returned. If cast cannot be done a NULL pointer is returned.
		*/
		const GElement* As(const GClassID& Class_ID) const {
			if (IsOfType(Class_ID))
				return this;
			return NULL;
		}
		//! Is 'InspectedType' derived-from/of 'Class_ID' type?
		virtual GBool IsTypeOfType(const GClassID& InspectedType, const GClassID& Class_ID) const;
		//! Physical copy, using a source element.
		GError CopyFrom(const GElement& Source);
		/*!
			Physical copy into 'Destination' element.\n
			If copy can't be done an error code is returned, else G_NO_ERROR.

			\note This function is provided for convenience, it's a simple Destination.CopyFrom(*this).
		*/
		inline GError CopyTo(GElement& Destination) const {
			return Destination.CopyFrom(*this);
		}
		/*!
			Assignment operator.
			\note It uses CopyFrom() function.
		*/
		inline GElement& operator =(const GElement& Source) {

			GError err = CopyFrom(Source);
			if (err != G_NO_ERROR) {
				G_DEBUG("GElement assignment operator, CopyFrom has returned this error code: " + StrUtils::ToString(err));
			}
			return *this;
		}
		//! Get element owner (kernel).
		inline const GElement* Owner() const {
			return gOwner;
		}
		//! Get class descriptor
		virtual const GClassID& ClassID() const {
			return G_ELEMENT_CLASSID;
		}
		//! Get base class (father class) descriptor
		virtual const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
		//! Is a root class?\n Root class is a class where derived class descriptor is the same of the inspected class.
		GBool IsRootClass() const {
			if (ClassID() == DerivedClassID())
				return G_TRUE;
			return G_FALSE;
		}
	};


	// *********************************************************************
	//                            GElementProxy
	// *********************************************************************

	/*! \class GElementProxy
		\brief This class implements a GElement proxy (provider).

		Through a GElementProxy it's possible to create a GElement class (or an its derived class), specifying eventually
		an owner.\n GElementProxy is at the base of the plugin system, based on a central manager that is implemented by
		GKernel class.\n
		Every derived class (proxies), must implement CreateNew() method to make class creation possible. This is generally
		true, except in cases where class has got some virtual (and non implemented methods). Even if class has got
		all methods implemented, proxies could introduce an intermediate "virtual / non instantiable" layer that
		could be used in some useful situations.
	*/
	class G_EXPORT GElementProxy {
	public:
		//! Create a new element. Every derived class must implements this method.
		virtual GElement* CreateNew(const GElement* Owner = NULL) const {
			if (!Owner)
				return NULL;
			return NULL;
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		virtual const GClassID& ClassID() const {
			return G_ELEMENT_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		virtual const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
		/*!
			Does this proxy manages a root class?\n Root class is a class where
			derived class descriptor is the same of the inspected class.
		*/
		GBool IsRootClass() const {
			if (ClassID() == DerivedClassID())
				return G_TRUE;
			return G_FALSE;
		}
		//! Version of this proxy.
		virtual GInt32 Version() const {
			return G_AMANITH_VERSION;
		}
		//! Destructor, this empty virtual declaration make gcc 4.0 happy with warnings.
		virtual ~GElementProxy() {
		}
	};

	//! Static proxy for GElement class.
	static const GElementProxy G_ELEMENT_PROXY;

};	// end namespace Amanith

#endif
