/****************************************************************************
** $file: amanith/src/gelement.cpp   0.1.0.0   edited Jun 30 08:00
**
** Base elements implementation.
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

#include "amanith/gelement.h"
#include "amanith/gkernel.h"
#include "amanith/support/gutilities.h"

/*!
	\file gelement.cpp
	\brief Implementation of GElement class.
*/

namespace Amanith {

const GClassID GClassID::Null = GClassID();


// *********************************************************************
//                               GElement
// *********************************************************************

/*!
	\class GElement
	\brief Minimum element class equipped with introspection and cloning system.

	GElement represents the base class that permits introspection and other facilities, like
	chain-cloning system, centralized creation and destruction.\n
	Every derived class must implement this functions:\n
	\n
	ClassID(): to get class descriptor.\n
	DerivedClassID(): to get base class (father class) descriptor.\n
	BaseClone(): to make physical copy possible. A typical implementation of this function includes just the
	copy of private and protected members, and then calls base class (father class) BaseClone() method.
*/

// constructor
GElement::GElement(): gOwner(NULL) {
}

/*!
	Check if owner is valid (it must be described by a G_KERNEL_CLASSID class identifier). If it's valid, we must notify
	him to add this element instance to internal list.
*/
GElement::GElement(const GElement* Owner) {

	if (Owner->IsOfType(G_KERNEL_CLASSID)) {
		gOwner = Owner;
		GKernel *kernel = (GKernel *)gOwner;
		kernel->AddElementReference(this);
	}
	else
		gOwner = NULL;
}

/*!
	If owner is not NULL, then notify him to remove this element instance from internal list.
*/
GElement::~GElement() {

	// is now very important to acknowledge my owner (kernel) that i'm going to be freed.
	if (gOwner) {
		G_ASSERT(gOwner->IsOfType(G_KERNEL_CLASSID) == G_TRUE);
		GKernel *kernel = (GKernel *)gOwner;
		kernel->RemoveElementReference(this);
	}
}

/*!
	Creation is made possible thanks kernel only, using proxies mechanism. If this element has not be created by a
	kernel, then it can't create new elements through this method.
*/
GElement* GElement::CreateNew(const GClassID& Class_ID) const {

	if (gOwner) {
		G_ASSERT(gOwner->IsOfType(G_KERNEL_CLASSID) == G_TRUE);
		GKernel *kernel = (GKernel *)gOwner;
		return (kernel->CreateNew(Class_ID));
	}
	return NULL;
}

/*!
	\param Class_Name a string containing the class name that we wanna know the descriptor of
	\param OutID a class descriptor that will be returned in case of successful operation.
	\return If operation will succeed G_NO_ERROR constant, else an error code.
	\note Class name comparison is case-insensitive.\n
	A return value of G_UNSUPPORTED_CLASSID means that specified Class_Name does not correspond to any
	registered proxies, so the class descriptor cannot be "resolved".
*/
GError GElement::ClassIDFromClassName(const GString& Class_Name, GClassID& OutID) const {

	// first lets see if class name match my class name
	if (StrUtils::SameText(Class_Name, ClassID().IDName())) {
		OutID = ClassID();
		return G_NO_ERROR;
	}
	// now try with owner
	if (gOwner) {
		G_ASSERT(gOwner->IsOfType(G_KERNEL_CLASSID) == G_TRUE);
		GKernel *kernel = (GKernel *)gOwner;
		return kernel->ClassIDFromClassName(Class_Name, OutID);
	}
	return G_UNSUPPORTED_CLASSID;
}

/*!
	If element can't be created a NULL pointer is returned.
	\note Class name comparison is case-insensitive.
*/
GElement* GElement::CreateNew(const GString& Class_Name) {

	GClassID cid;
	GError err;
	
	err = ClassIDFromClassName(Class_Name, cid);
	if (err == G_NO_ERROR)
		return CreateNew(cid);
	return NULL;
}

GBool GElement::IsTypeOfType(const GClassID& InspectedType, const GClassID& Class_ID) const {

	// trivial case
	if (Class_ID == InspectedType)
		return G_TRUE;
	// as an element, i can just say that my ClassID is of DerivedClassID type
	if ((InspectedType == ClassID()) && (Class_ID == DerivedClassID()))
		return G_TRUE;
	// try with owner
	if (gOwner)
		return gOwner->IsTypeOfType(InspectedType, Class_ID);
	// no more chances left
	return G_FALSE;
}

// is an element of a specified type? it does include inheritance inspection
GBool GElement::IsOfType(const GClassID& Class_ID) const {

	// trivial case
	if ((Class_ID == ClassID()) || (Class_ID == DerivedClassID()))
		return G_TRUE;
	return IsTypeOfType(ClassID(), Class_ID);
}

/*!
	If copy can't be done an error code is returned, else G_NO_ERROR.
	The copy schema is subdivided into 3 steps:

	-# If Source is the same object pointed by 'this' pointer, just exit with a G_NO_ERROR code.
	-# If Source is not of the same type of this element, use owner (kernel) to do the copy (using Copy() function).\n
	The kernel will try to do a (physical) type conversion (using registered converters plugins), and then do the
	conversion. For further details see GKernel::Copy().
	-# If Source is of the same type of this element, then it is safe to call BaseClone().

	\note If as Source parameter will be passed a 'this' reference the function just returns a G_NO_ERROR without doing
	nothing. So a call like element->CopyFrom(*element) is not dangerous.

*/
GError GElement::CopyFrom(const GElement& Source) {

	const GElement *ptr	= &Source;

	// check for a self to self copy
	if (ptr == this)
		return G_NO_ERROR;
	// check for a valid (compatible) source
	if (!Source.IsOfType(ClassID())) {
		if (gOwner) {
			G_ASSERT(gOwner->IsOfType(G_KERNEL_CLASSID) == G_TRUE);
			GKernel *kernel = (GKernel *)gOwner;
			return kernel->Copy(Source, *this);
		}
		else
			return G_UNSUPPORTED_CLASSID;
	}
	// now is safe to do a clone
	return BaseClone(Source);
}

}
