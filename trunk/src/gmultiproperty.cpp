/****************************************************************************
** $file: amanith/src/gmultiproperty.cpp   0.2.0.0   edited Dec, 12 2005
**
** Basic animated multi-properties implementation.
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

#include "amanith/gmultiproperty.h"
#include "amanith/gkernel.h"
#include "amanith/support/gutilities.h"

/*!
	\file gmultiproperty.cpp
	\brief Implementation of basic animated multi-properties.
*/

namespace Amanith {


// *********************************************************************
//                          GMultiProperty1D
// *********************************************************************

GMultiProperty1D::GMultiProperty1D() : GProperty() {
}

GMultiProperty1D::GMultiProperty1D(const GElement* Owner) : GProperty(Owner) {
}

GMultiProperty1D::~GMultiProperty1D() {
}

GError GMultiProperty1D::BaseClone(const GElement& Source) {

	const GMultiProperty1D& k = (const GMultiProperty1D&)Source;

	gSubPropertiesType = k.gSubPropertiesType;
	return GProperty::BaseClone(Source);
}

GError GMultiProperty1D::Init(const GUInt32 SubPropertiesCount, const GClassID& SubPropertiesType,
							  const GKeyValue& DefaultValue, const GString& Labels) {

	if (SubPropertiesCount == 0)
		return G_INVALID_PARAMETER;

	// check if sub-properties type is supported
	if (Owner()) {
		GKernel *k = (GKernel *)Owner();
		if (!k->IsSupported(SubPropertiesType))
			return G_UNSUPPORTED_CLASSID;
	}

	// empty this property
	DeleteProperties();
	// extract labels
	GStringList extractedLabels = StrUtils::Split(Labels, ";", G_FALSE);

	GBool alreadyExists;
	GUInt32 index;
	GProperty *tmpProperty;

	// labels and SubPropertiesCount match, use specified names
	if (extractedLabels.size() == SubPropertiesCount) {
		GStringList::const_iterator it = extractedLabels.begin();
		for (GUInt32 i = 0; i < SubPropertiesCount; ++i) {
			tmpProperty = AddProperty(*it, SubPropertiesType, DefaultValue, alreadyExists, index);
			if (!tmpProperty)
				return G_MEMORY_ERROR;
			it++;
		}
	}
	// default name schema, use natural numbers "0", "1", "2", ...
	else {
		for (GUInt32 i = 0; i < SubPropertiesCount; ++i) {
			tmpProperty = AddProperty(StrUtils::ToString(i), SubPropertiesType, DefaultValue, alreadyExists, index);
			if (!tmpProperty)
				return G_MEMORY_ERROR;
		}
	}
	gSubPropertiesType = SubPropertiesType;
	return G_NO_ERROR;
}


// *********************************************************************
//                       GTwoHermiteProperty1D
// *********************************************************************
GTwoHermiteProperty1D::GTwoHermiteProperty1D() : GMultiProperty1D() {

	this->Init(2, G_HERMITEPROPERTY1D_CLASSID, GKeyValue(GVector2(0, 0)), "x;y");
}

GTwoHermiteProperty1D::GTwoHermiteProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {
	
	this->Init(2, G_HERMITEPROPERTY1D_CLASSID, GKeyValue(GVector2(0, 0)), "x;y");
}

GTwoHermiteProperty1D::~GTwoHermiteProperty1D() {
}

// *********************************************************************
//                       GThreeHermiteProperty1D
// *********************************************************************
GThreeHermiteProperty1D::GThreeHermiteProperty1D() : GMultiProperty1D() {

	this->Init(3, G_HERMITEPROPERTY1D_CLASSID, GKeyValue(GVector3(0, 0, 0)), "x;y;z");
}

GThreeHermiteProperty1D::GThreeHermiteProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {

	this->Init(3, G_HERMITEPROPERTY1D_CLASSID, GKeyValue(GVector3(0, 0, 0)), "x;y;z");
}

GThreeHermiteProperty1D::~GThreeHermiteProperty1D() {
}

// *********************************************************************
//                       GFourHermiteProperty1D
// *********************************************************************
GFourHermiteProperty1D::GFourHermiteProperty1D() : GMultiProperty1D() {

	this->Init(4, G_HERMITEPROPERTY1D_CLASSID, GKeyValue(GVector4(0, 0, 0, 0)), "x;y;z;w");
}

GFourHermiteProperty1D::GFourHermiteProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {

	this->Init(4, G_HERMITEPROPERTY1D_CLASSID, GKeyValue(GVector4(0, 0, 0, 0)), "x;y;z;w");
}

GFourHermiteProperty1D::~GFourHermiteProperty1D() {
}


// *********************************************************************
//                       GTwoLinearProperty1D
// *********************************************************************
GTwoLinearProperty1D::GTwoLinearProperty1D() : GMultiProperty1D() {

	this->Init(2, G_LINEARPROPERTY1D_CLASSID, GKeyValue(GVector2(0, 0)), "x;y");
}

GTwoLinearProperty1D::GTwoLinearProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {

	this->Init(2, G_LINEARPROPERTY1D_CLASSID, GKeyValue(GVector2(0, 0)), "x;y");
}

GTwoLinearProperty1D::~GTwoLinearProperty1D() {
}

// *********************************************************************
//                       GThreeLinearProperty1D
// *********************************************************************
GThreeLinearProperty1D::GThreeLinearProperty1D() : GMultiProperty1D() {

	this->Init(3, G_LINEARPROPERTY1D_CLASSID, GKeyValue(GVector3(0, 0, 0)), "x;y;z");
}

GThreeLinearProperty1D::GThreeLinearProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {

	this->Init(3, G_LINEARPROPERTY1D_CLASSID, GKeyValue(GVector3(0, 0, 0)), "x;y;z");
}

GThreeLinearProperty1D::~GThreeLinearProperty1D() {
}

// *********************************************************************
//                       GFourLinearProperty1D
// *********************************************************************
GFourLinearProperty1D::GFourLinearProperty1D() : GMultiProperty1D() {

	this->Init(4, G_LINEARPROPERTY1D_CLASSID, GKeyValue(GVector4(0, 0, 0, 0)), "x;y;z;w");
}

GFourLinearProperty1D::GFourLinearProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {

	this->Init(4, G_LINEARPROPERTY1D_CLASSID, GKeyValue(GVector4(0, 0, 0, 0)), "x;y;z;w");
}

GFourLinearProperty1D::~GFourLinearProperty1D() {
}

// *********************************************************************
//                       GTwoConstantProperty1D
// *********************************************************************
GTwoConstantProperty1D::GTwoConstantProperty1D() : GMultiProperty1D() {

	this->Init(2, G_CONSTANTPROPERTY1D_CLASSID, GKeyValue(GVector2(0, 0)), "x;y");
}

GTwoConstantProperty1D::GTwoConstantProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {

	this->Init(2, G_CONSTANTPROPERTY1D_CLASSID, GKeyValue(GVector2(0, 0)), "x;y");
}

GTwoConstantProperty1D::~GTwoConstantProperty1D() {
}

// *********************************************************************
//                       GThreeConstantProperty1D
// *********************************************************************
GThreeConstantProperty1D::GThreeConstantProperty1D() : GMultiProperty1D() {

	this->Init(3, G_CONSTANTPROPERTY1D_CLASSID, GKeyValue(GVector3(0, 0, 0)), "x;y;z");
}

GThreeConstantProperty1D::GThreeConstantProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {

	this->Init(3, G_CONSTANTPROPERTY1D_CLASSID, GKeyValue(GVector3(0, 0, 0)), "x;y;z");
}

GThreeConstantProperty1D::~GThreeConstantProperty1D() {
}

// *********************************************************************
//                       GFourConstantProperty1D
// *********************************************************************
GFourConstantProperty1D::GFourConstantProperty1D() : GMultiProperty1D() {

	this->Init(4, G_CONSTANTPROPERTY1D_CLASSID, GKeyValue(GVector4(0, 0, 0, 0)), "x;y;z;w");
}

GFourConstantProperty1D::GFourConstantProperty1D(const GElement* Owner) : GMultiProperty1D(Owner) {

	this->Init(4, G_CONSTANTPROPERTY1D_CLASSID, GKeyValue(GVector4(0, 0, 0, 0)), "x;y;z;w");
}

GFourConstantProperty1D::~GFourConstantProperty1D() {
}

}
