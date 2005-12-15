/****************************************************************************
** $file: amanith/src/ganimtrsnode2d.cpp   0.2.0.0   edited Dec, 12 2005
**
** 2D TRS (Translation-Rotation-Scale) animated node implementation.
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
#include "amanith/2d/ganimtrsnode2d.h"
#include "amanith/geometry/gxformconv.h"

/*!
	\file ganimtrsnode2d.cpp
	\brief Implementation of GAnimTRSNode2D class.
*/

namespace Amanith {


// *********************************************************************
//                            GAnimTRSNode2D
// *********************************************************************

// default constructor
GAnimTRSNode2D::GAnimTRSNode2D() : GAnimElement() {

	// gPivotPosition is automatically set to (0, 0)
	gPivotRotation = 0;
	gPivotScale.Set(1, 1);
	gFather = NULL;
	gCustomData = NULL;
	
	// add TRS properties
	GBool alreadyExists;
	GUInt32 index;

	GProperty *tmpProp = AddProperty("transform", G_PROPERTY_CLASSID, GKeyValue(), alreadyExists, index);
	if (tmpProp) {
		G_ASSERT(alreadyExists == G_FALSE);
		// default value for "x" and "y" sub-properties will be set automatically to 0
		tmpProp->AddProperty("position", G_TWOHERMITEPROPERTY1D_CLASSID, GKeyValue(), alreadyExists, index);
		G_ASSERT(alreadyExists == G_FALSE);
		tmpProp->AddProperty("rotation", G_HERMITEPROPERTY1D_CLASSID, GKeyValue((GReal)0), alreadyExists, index);
		G_ASSERT(alreadyExists == G_FALSE);
		// we must impose a default value of 1 to "x" and "y" sub-properties
		GProperty *p = tmpProp->AddProperty("scale", G_TWOHERMITEPROPERTY1D_CLASSID, GKeyValue(), alreadyExists, index);
		G_ASSERT(alreadyExists == G_FALSE);
		p->Property("x")->SetDefaultValue(GKeyValue((GReal)1));
		p->Property("y")->SetDefaultValue(GKeyValue((GReal)1));
	}
}

// default constructor with owner
GAnimTRSNode2D::GAnimTRSNode2D(const GElement* Owner) : GAnimElement(Owner) {

	// gPivotPosition is automatically set to (0, 0)
	gPivotRotation = 0;
	gPivotScale.Set(1, 1);
	gFather = NULL;
	gCustomData = NULL;

	// add TRS properties
	GBool alreadyExists;
	GUInt32 index;

	GProperty *tmpProp = AddProperty("transform", G_PROPERTY_CLASSID, GKeyValue(), alreadyExists, index);
	if (tmpProp) {
		G_ASSERT(alreadyExists == G_FALSE);
		// default value for "x" and "y" sub-properties will be set automatically to 0
		tmpProp->AddProperty("position", G_TWOHERMITEPROPERTY1D_CLASSID, GKeyValue(), alreadyExists, index);
		G_ASSERT(alreadyExists == G_FALSE);
		tmpProp->AddProperty("rotation", G_HERMITEPROPERTY1D_CLASSID, GKeyValue((GReal)0), alreadyExists, index);
		G_ASSERT(alreadyExists == G_FALSE);
		// we must impose a default value of 1 to "x" and "y" sub-properties
		GProperty *p = tmpProp->AddProperty("scale", G_TWOHERMITEPROPERTY1D_CLASSID, GKeyValue(), alreadyExists, index);
		G_ASSERT(alreadyExists == G_FALSE);
		p->Property("x")->SetDefaultValue(GKeyValue((GReal)1));
		p->Property("y")->SetDefaultValue(GKeyValue((GReal)1));
	}
}

// destructor, delete all keys and internal ease curve (if it exists)
GAnimTRSNode2D::~GAnimTRSNode2D() {

	// detach firs-level children form me
	GUInt32 i, j = ChildrenCounts();

	for (i = 0; i < j; i++) {
		GAnimTRSNode2D *tmpNode = gChildren[i];
		// do the detach
		tmpNode->SetFather(NULL);
	}
	// if i have a father, i must detach me from him
	if (gFather)
		gFather->DetachChildNode(this);
}

void GAnimTRSNode2D::DetachChildNode(const GUInt32 ChildIndex) {

	if (ChildIndex >= ChildrenCounts())
		return;

	GDynArray<GAnimTRSNode2D *>::iterator it = gChildren.begin();
	it += ChildIndex;

	GAnimTRSNode2D *n = (*it);
	n->gFather = NULL;
	gChildren.erase(it);
}

GBool GAnimTRSNode2D::DetachChildNode(const GAnimTRSNode2D *ChildNode) {

	if (!ChildNode)
		return G_FALSE;

	G_ASSERT(ChildNode->Father() == this);

	GUInt32 i, j = ChildrenCounts();
	for (i = 0; i < j; i++) {
		if (gChildren[i] == ChildNode) {
			DetachChildNode(i);
			return G_TRUE;
		}
	}
	return G_FALSE;
}

GBool GAnimTRSNode2D::AttachChildNode(GAnimTRSNode2D *Node) {

	if (!Node)
		return G_FALSE;

	if (Node->Father() == this) {

		// for debugging purpose verify that specified node is included into internal children list
#ifdef _DEBUG
		GUInt32 i, j = ChildrenCounts();
		GBool found = G_FALSE;

		for (i = 0; i < j; i++) {
			if (gChildren[i] == Node) {
				found = G_TRUE;
				break;
			}
		}
		G_ASSERT(found == G_TRUE);
#endif
		return G_TRUE;
	}

	// if the node had already a father, first we must detach it; after detaching, the node will have a NULL
	// father
	if (Node->Father())
		Node->Father()->DetachChildNode(Node);
	// now assign new father and insert the node into internal children list
	Node->gFather = this;

	// for debugging purpose verify that specified node is NOT included into internal children list
#ifdef _DEBUG
	GUInt32 k, w = ChildrenCounts();
	for (k = 0; k < w; k++)
		G_ASSERT(gChildren[k] != Node);
#endif
	gChildren.push_back(Node);
	return G_FALSE;
}

GError GAnimTRSNode2D::SetFather(GAnimTRSNode2D *NewFather, const GBool AffectTracks) {

	GTimeInterval tmpValid;

	// if specified father is NULL, detach this node (it will become a root node)
	if (!NewFather) {
		if (AffectTracks) {
			// offset position track
			GPoint2 wPos = Position(0, G_WORLD_SPACE, tmpValid);
			GPoint2 lPos = Position(0, G_LOCAL_SPACE, tmpValid);
			OffsetPositionTrack(wPos - lPos);
			// offset rotation track
			GReal wRot = Rotation(0, G_WORLD_SPACE, tmpValid);
			GReal lRot = Rotation(0, G_LOCAL_SPACE, tmpValid);
			OffsetRotationTrack(wRot - lRot);
			// offset scale track
			GVector2 wScl = Scale(0, G_WORLD_SPACE, tmpValid);
			GVector2 lScl = Scale(0, G_LOCAL_SPACE, tmpValid);
			GVector2 tmpScale(wScl);
			if (GMath::Abs(lScl[G_X]) > G_EPSILON)
				tmpScale[G_X] /= lScl[G_X];
			if (GMath::Abs(lScl[G_Y]) > G_EPSILON)
				tmpScale[G_Y] /= lScl[G_Y];
			OffsetScaleTrack(tmpScale);
		}
		// detach me from my current father (if it exists)
		if (gFather)
			gFather->DetachChildNode(this);
	}
	else {
		if (AffectTracks) {
			// offset position track
			GPoint2 wPos = Position(0, G_WORLD_SPACE, tmpValid);
			GPoint2 lPos = Position(0, G_LOCAL_SPACE, tmpValid);
			GPoint2 fwPos = NewFather->Position(0, G_WORLD_SPACE, tmpValid);
			OffsetPositionTrack(fwPos - wPos - lPos);
			// offset rotation track
			GReal wRot = Rotation(0, G_WORLD_SPACE, tmpValid);
			GReal lRot = Rotation(0, G_LOCAL_SPACE, tmpValid);
			GReal fwRot = NewFather->Rotation(0, G_WORLD_SPACE, tmpValid);
			OffsetRotationTrack(fwRot - wRot - lRot);
			// offset scale track
			GVector2 wScl = Scale(0, G_WORLD_SPACE, tmpValid);
			GVector2 lScl = Scale(0, G_LOCAL_SPACE, tmpValid);
			GPoint2 fwScl = NewFather->Scale(0, G_WORLD_SPACE, tmpValid);
			GVector2 tmpScale(fwScl);
			GVector2 tmpDen(wScl[G_X] * lScl[G_X], wScl[G_Y] * lScl[G_Y]);
			if (GMath::Abs(tmpDen[G_X]) > G_EPSILON)
				tmpScale[G_X] /= tmpDen[G_X];
			else
				tmpScale[G_X] = 1;

			if (GMath::Abs(tmpDen[G_Y]) > G_EPSILON)
				tmpScale[G_Y] /= tmpDen[G_Y];
			else
				tmpScale[G_Y] = 1;
			OffsetScaleTrack(tmpScale);
		}
		// detach me from my current father (if it exists)
		if (gFather)
			gFather->DetachChildNode(this);
		// attach me to the new father
		NewFather->AttachChildNode(this);
	}
	return G_NO_ERROR;
}

GPoint2 GAnimTRSNode2D::Position(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const {

	GProperty *tmpProp = Property("transform");

	if (!tmpProp)
		return GPoint2(0, 0);

	tmpProp = tmpProp->Property("position");
	G_ASSERT(tmpProp);
	GTimeInterval tmpValid = G_FOREVER_TIMEINTERVAL;
	GKeyValue xValue, yValue;

	// extract translation
	GProperty *xProp = tmpProp->Property("x");
	GProperty *yProp = tmpProp->Property("y");
	G_ASSERT(xProp != NULL);
	G_ASSERT(yProp != NULL);
	// build translation factor
	GError xErr = xProp->Value(xValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	GError yErr = yProp->Value(yValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (xErr != G_NO_ERROR || yErr != G_NO_ERROR)
		return GPoint2(0, 0);

	ValidInterval = tmpValid;

	// take care of father
	if (gFather && Space == G_WORLD_SPACE) {
		GPoint2 retValue(xValue.RealValue(), yValue.RealValue());
		retValue += gFather->Position(TimePos, G_WORLD_SPACE, tmpValid);
		ValidInterval &= tmpValid;
		return retValue;
	}
	else
		return GPoint2(xValue.RealValue(), yValue.RealValue());
}

GReal GAnimTRSNode2D::Rotation(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const {

	GProperty *tmpProp = Property("transform");

	if (!tmpProp)
		return 0;

	tmpProp = tmpProp->Property("rotation");
	G_ASSERT(tmpProp);
	GTimeInterval tmpValid = G_FOREVER_TIMEINTERVAL;
	GKeyValue tmpValue;

	GError err = tmpProp->Value(tmpValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (err != G_NO_ERROR)
		return 0;

	ValidInterval = tmpValid;
	// take care of father
	if (gFather && Space == G_WORLD_SPACE) {
		GReal retValue = tmpValue.RealValue();
		retValue += gFather->Rotation(TimePos, G_WORLD_SPACE, tmpValid);
		ValidInterval &= tmpValid;
		return retValue;
	}
	else
		return tmpValue.RealValue();
}

GVectBase<GReal, 2> GAnimTRSNode2D::Scale(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const {

	GProperty *tmpProp = Property("transform");

	if (!tmpProp)
		return GVector2(1, 1);

	tmpProp = tmpProp->Property("scale");
	G_ASSERT(tmpProp);
	GTimeInterval tmpValid = G_FOREVER_TIMEINTERVAL;
	GKeyValue xValue, yValue;

	// extract scale
	GProperty *xProp = tmpProp->Property("x");
	GProperty *yProp = tmpProp->Property("y");
	G_ASSERT(xProp != NULL);
	G_ASSERT(yProp != NULL);
	GError xErr = xProp->Value(xValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	GError yErr = yProp->Value(yValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (xErr != G_NO_ERROR || yErr != G_NO_ERROR)
		return GVector2(1, 1);

	ValidInterval = tmpValid;
	// take care of father
	if (gFather && Space == G_WORLD_SPACE) {
		GVectBase<GReal, 2> retValue(xValue.RealValue(), yValue.RealValue());
		retValue += gFather->Scale(TimePos, G_WORLD_SPACE, tmpValid);
		ValidInterval &= tmpValid;
		return retValue;
	}
	else
		return GVectBase<GReal, 2>(xValue.RealValue(), yValue.RealValue());
}

// build pivot TRS transformation matrix
GMatrix33 GAnimTRSNode2D::PivotMatrix() const {

	// build translation factor
	GMatrix33 translation;
	TranslationToMatrix(translation, -gPivotPosition);
	// build rotation factor
	GMatrix33 rotation;
	RotationToMatrix(rotation, gPivotRotation);
	// build scale factor
	GMatrix33 scale;
	ScaleToMatrix(scale, gPivotScale);
	// return the resulting matrix (first scale, then rotation and finally translation)
	return (translation * (rotation * scale));
}

// get pivot inverse matrix only
GMatrix33 GAnimTRSNode2D::InversePivotMatrix() const {

	// build (inverse) translation factor
	GMatrix33 invTranslation;
	TranslationToMatrix(invTranslation, gPivotPosition);

	// build (inverse) rotation factor
	GMatrix33 invRotation;
	RotationToMatrix(invRotation, -gPivotRotation);
	// build (inverse) scale factor
	GMatrix33 invScale;
	GVector2 tmpScale(gPivotScale);
	if (GMath::Abs(tmpScale[G_X]) > G_EPSILON)
		tmpScale[G_X] = 1 / tmpScale[G_X];
	if (GMath::Abs(tmpScale[G_Y]) > G_EPSILON)
		tmpScale[G_Y] = 1 / tmpScale[G_Y];
	ScaleToMatrix(invScale, tmpScale);
	// return the resulting (inverse) matrix
	return (invScale * (invRotation * invTranslation));
}

GMatrix33 GAnimTRSNode2D::Matrix(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const {

	GProperty *tmpProp = Property("transform");

	// this can be the case of a curve not created through a kernel
	if (!tmpProp) {
		ValidInterval = G_FOREVER_TIMEINTERVAL;
		return GMatrix33();
	}

	GMatrix33 translation;
	GMatrix33 rotation;
	GMatrix33 scale;
	GTimeInterval tmpValid = G_FOREVER_TIMEINTERVAL;
	GKeyValue xValue, yValue;
	GError xErr, yErr;
	GProperty *transProp = tmpProp->Property("position");
	GProperty *rotProp = tmpProp->Property("rotation");
	GProperty *scaleProp = tmpProp->Property("scale");

	G_ASSERT(transProp != NULL);
	G_ASSERT(rotProp != NULL);
	G_ASSERT(scaleProp != NULL);

	// extract translation
	GProperty *xProp = transProp->Property("x");
	GProperty *yProp = transProp->Property("y");
	G_ASSERT(xProp != NULL);
	G_ASSERT(yProp != NULL);
	// build translation factor
	xErr = xProp->Value(xValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	yErr = yProp->Value(yValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (xErr != G_NO_ERROR || yErr != G_NO_ERROR)
		return GMatrix33();
	TranslationToMatrix(translation, GVector2(xValue.RealValue(), yValue.RealValue()));
	// build rotation factor
	xErr = rotProp->Value(xValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (xErr != G_NO_ERROR)
		return GMatrix33();
	RotationToMatrix(rotation, xValue.RealValue());
	// extract scale
	xProp = scaleProp->Property("x");
	yProp = scaleProp->Property("y");
	G_ASSERT(xProp != NULL);
	G_ASSERT(yProp != NULL);
	// build scale factor
	xErr = xProp->Value(xValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	yErr = yProp->Value(yValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (xErr != G_NO_ERROR || yErr != G_NO_ERROR)
		return GMatrix33();
	ScaleToMatrix(scale, GVector2(xValue.RealValue(), yValue.RealValue()));

	ValidInterval = tmpValid;
	// take care of father
	GMatrix33 localMatrix = (translation * (rotation * scale));
	if (gFather && Space == G_WORLD_SPACE) {
		GMatrix33 fatherMatrix = gFather->Matrix(TimePos, G_WORLD_SPACE, tmpValid);
		ValidInterval &= tmpValid;
		return (fatherMatrix * localMatrix);
	}
	else
		return localMatrix;
}

GMatrix33 GAnimTRSNode2D::InverseMatrix(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const {

	GProperty *tmpProp = Property("transform");

	// this can be the case of a curve not created through a kernel
	if (!tmpProp) {
		ValidInterval = G_FOREVER_TIMEINTERVAL;
		return GMatrix33();
	}

	GMatrix33 invTranslation;
	GMatrix33 invRotation;
	GMatrix33 invScale;
	GTimeInterval tmpValid = G_FOREVER_TIMEINTERVAL;
	GKeyValue xValue, yValue;
	GError xErr, yErr;
	GProperty *transProp = tmpProp->Property("position");
	GProperty *rotProp = tmpProp->Property("rotation");
	GProperty *scaleProp = tmpProp->Property("scale");

	G_ASSERT(transProp != NULL);
	G_ASSERT(rotProp != NULL);
	G_ASSERT(scaleProp != NULL);

	// extract translation
	GProperty *xProp = transProp->Property("x");
	GProperty *yProp = transProp->Property("y");
	G_ASSERT(xProp != NULL);
	G_ASSERT(yProp != NULL);
	// build translation factor
	xErr = xProp->Value(xValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	yErr = yProp->Value(yValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (xErr != G_NO_ERROR || yErr != G_NO_ERROR)
		return GMatrix33();
	TranslationToMatrix(invTranslation, GVector2(-xValue.RealValue(), -yValue.RealValue()));

	// build rotation factor
	xErr = rotProp->Value(xValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (xErr != G_NO_ERROR)
		return GMatrix33();
	RotationToMatrix(invRotation, -xValue.RealValue());

	// extract scale
	xProp = scaleProp->Property("x");
	yProp = scaleProp->Property("y");
	G_ASSERT(xProp != NULL);
	G_ASSERT(yProp != NULL);
	// build scale factor
	xErr = xProp->Value(xValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	yErr = yProp->Value(yValue, tmpValid, TimePos, G_ABSOLUTE_VALUE);
	if (xErr != G_NO_ERROR || yErr != G_NO_ERROR)
		return GMatrix33();
	GPoint2 tmpScale(1, 1);
	if (GMath::Abs(xValue.RealValue()) > G_EPSILON)
		tmpScale[G_X] = 1 / xValue.RealValue();
	if (GMath::Abs(yValue.RealValue()) > G_EPSILON)
		tmpScale[G_Y] = 1 / yValue.RealValue();
	ScaleToMatrix(invScale, tmpScale);

	ValidInterval = tmpValid;
	// take care of father
	GMatrix33 invLocalMatrix = (invScale * (invRotation * invTranslation));
	if (gFather && Space == G_WORLD_SPACE) {
		GMatrix33 invFatherMatrix = gFather->InverseMatrix(TimePos, G_WORLD_SPACE, tmpValid);
		ValidInterval &= tmpValid;
		return (invLocalMatrix * invFatherMatrix);
	}
	else
		return invLocalMatrix;
}

GError GAnimTRSNode2D::SetPosition(const GTimeValue TimePos, const GVectBase<GReal, 2>& RelPosition) {

	GError err;
	GProperty *tmpProp = Property("transform");
	
	// this can be the case of a curve not created through a kernel
	if (!tmpProp)
		err = G_MISSING_KERNEL;
	else {
		GKeyValue tmpValue;
		tmpValue.SetTimePosition(TimePos);

		// extract position track
		GProperty *posProp = tmpProp->Property("position");
		G_ASSERT(posProp);

		// set "x" property
		tmpProp = posProp->Property("x");
		G_ASSERT(tmpProp != NULL);
		tmpValue.SetValue(RelPosition[G_X]);
		err = tmpProp->SetValue(tmpValue, TimePos, G_ABSOLUTE_VALUE);
		if (err != G_NO_ERROR)
			return err;
		// set "y" property
		tmpProp = posProp->Property("y");
		G_ASSERT(tmpProp != NULL);
		tmpValue.SetValue(RelPosition[G_Y]);
		err = tmpProp->SetValue(tmpValue, TimePos, G_ABSOLUTE_VALUE);
	}
	return err;
}

GError GAnimTRSNode2D::SetRotation(const GTimeValue TimePos, const GReal& RelRotation) {

	GError err;
	GProperty *tmpProp = Property("transform");

	// this can be the case of a curve not created through a kernel
	if (!tmpProp)
		err = G_MISSING_KERNEL;
	else {
		// extract position track
		GProperty *rotProp = tmpProp->Property("rotation");
		G_ASSERT(rotProp);
		GKeyValue tmpValue(TimePos, RelRotation);
		err = rotProp->SetValue(tmpValue, TimePos, G_ABSOLUTE_VALUE);
	}
	return err;
}

GError GAnimTRSNode2D::SetScale(const GTimeValue TimePos, const GVectBase<GReal, 2>& RelScale) {

	GError err;
	GProperty *tmpProp = Property("transform");

	// this can be the case of a curve not created through a kernel
	if (!tmpProp) {
		err = G_MISSING_KERNEL;
	}
	else {
		// extract position track
		GProperty *scaleProp = tmpProp->Property("scale");
		G_ASSERT(scaleProp);

		GKeyValue tmpValue;
		tmpValue.SetTimePosition(TimePos);

		// set "x" property
		tmpProp = scaleProp->Property("x");
		G_ASSERT(tmpProp != NULL);
		tmpValue.SetValue(RelScale[G_X]);
		err = tmpProp->SetValue(tmpValue, TimePos, G_ABSOLUTE_VALUE);
		if (err != G_NO_ERROR)
			return err;
		// set "y" property
		tmpProp = scaleProp->Property("y");
		G_ASSERT(tmpProp != NULL);
		tmpValue.SetValue(RelScale[G_Y]);
		err = tmpProp->SetValue(tmpValue, TimePos, G_ABSOLUTE_VALUE);
	}
	return err;
}

// move all position keys by an additive offset vector
void GAnimTRSNode2D::OffsetPositionTrack(const GVector2& OffsetVector) {

	GProperty *tmpProp = Property("transform"), *posProp;
	GUInt32 i, j;
	GKeyValue tmpKey;

	if (!tmpProp)
		return;

	posProp = tmpProp->Property("position");
	G_ASSERT(posProp != NULL);
	// x-property
	tmpProp = posProp->Property("x");
	G_ASSERT(tmpProp != NULL);

	j = tmpProp->KeysCount();
	if (j == 0) {
		tmpKey = tmpProp->DefaultValue();
		tmpKey.SetValue(OffsetVector[G_X] + tmpKey.RealValue());
		tmpProp->SetDefaultValue(tmpKey);
	}
	else {
		for (i = 0; i < j; ++i) {
			tmpProp->Key(i, tmpKey);
			tmpKey.SetValue(tmpKey.RealValue() + OffsetVector[G_X]);
			tmpProp->SetKey(i, tmpKey);
		}
	}

	// y-property
	tmpProp = posProp->Property("y");
	G_ASSERT(tmpProp != NULL);

	j =  tmpProp->KeysCount();
	if (j == 0) {
		tmpKey = tmpProp->DefaultValue();
		tmpKey.SetValue(OffsetVector[G_Y] + tmpKey.RealValue());
		tmpProp->SetDefaultValue(tmpKey);
	}
	else {
		for (i = 0; i < j; ++i) {
			tmpProp->Key(i, tmpKey);
			tmpKey.SetValue(tmpKey.RealValue() + OffsetVector[G_Y]);
			tmpProp->SetKey(i, tmpKey);
		}
	}
}

// move all rotation keys by an additive offset angle (expressed in radiant)
void GAnimTRSNode2D::OffsetRotationTrack(const GReal OffsetAngle) {

	GProperty *tmpProp = Property("transform"), *rotProp;
	GUInt32 i, j;
	GKeyValue tmpKey;

	if (!tmpProp)
		return;

	rotProp = tmpProp->Property("rotation");
	G_ASSERT(rotProp != NULL);

	j = rotProp->KeysCount();
	if (j == 0) {
		tmpKey = rotProp->DefaultValue();
		tmpKey.SetValue(OffsetAngle + tmpKey.RealValue());
		rotProp->SetDefaultValue(tmpKey);
	}
	else {
		for (i = 0; i < j; ++i) {
			rotProp->Key(i, tmpKey);
			rotProp->SetValue(tmpKey.RealValue() + OffsetAngle);
			rotProp->SetKey(i, tmpKey);
		}
	}
}

// scale all scaling keys by an offset factor
void GAnimTRSNode2D::OffsetScaleTrack(const GVectBase<GReal, 2>& OffsetFactors) {

	GProperty *tmpProp = Property("transform"), *sclProp;
	GUInt32 i, j;
	GKeyValue tmpKey;

	if (!tmpProp)
		return;

	sclProp = tmpProp->Property("scale");
	G_ASSERT(sclProp != NULL);

	// x-property
	tmpProp = sclProp->Property("x");
	G_ASSERT(tmpProp != NULL);
	j = tmpProp->KeysCount();
	if (j == 0) {
		tmpKey = tmpProp->DefaultValue();
		tmpKey.SetValue(OffsetFactors[G_X] * tmpKey.RealValue());
		tmpProp->SetDefaultValue(tmpKey);
	}
	else {
		for (i = 0; i < j; ++i) {
			tmpProp->Key(i, tmpKey);
			tmpKey.SetValue(tmpKey.RealValue() * OffsetFactors[G_X]);
			tmpProp->SetKey(i, tmpKey);
		}
	}

	// y-property
	tmpProp = sclProp->Property("y");
	G_ASSERT(tmpProp != NULL);
	j =  tmpProp->KeysCount();
	if (j == 0) {
		tmpKey = tmpProp->DefaultValue();
		tmpKey.SetValue(OffsetFactors[G_Y] * tmpKey.RealValue());
		tmpProp->SetDefaultValue(tmpKey);
	}
	else {
		for (i = 0; i < j; ++i) {
			tmpProp->Key(i, tmpKey);
			tmpKey.SetValue(tmpKey.RealValue() * OffsetFactors[G_Y]);
			tmpProp->SetKey(i, tmpKey);
		}
	}
}

GError GAnimTRSNode2D::SetPivotPosition(const GVectBase<GReal, 2>& NewPosition, const GBool AffectChildren) {

	// calculate move delta
	GVector2 delta = NewPosition - gPivotPosition;

	// if AffectChildren is true, counter transform them so they wont move.
	if (AffectChildren) {
		// this node must be modified adding to the position keys the quantity 'delta'.
		// First level children, instead, must translate with a quantity '-delta'
		// children at level 2 or more deep, wont be touched
		GUInt32 i, j = ChildrenCounts();

		for (i = 0; i < j; i++) {
			GAnimTRSNode2D *child = gChildren[i];
			G_ASSERT(child != NULL);
			child->OffsetPositionTrack(-delta);
		}
	}
	OffsetPositionTrack(delta);
	// set the new pivot position
	gPivotPosition = NewPosition;
	return G_NO_ERROR;
}

GError GAnimTRSNode2D::SetPivotRotation(const GReal& NewAngle, const GBool AffectChildren) {

	// calculate rotation delta
	GReal delta = NewAngle - gPivotRotation;

	// if AffectChildren is true, counter transform them so they won't rotate.
	if (AffectChildren) {
		// this node must be modified adding to the rotation keys the quantity 'delta'.
		// First level children, instead, must translate with a quantity '-delta'
		// children at level 2 or more deep, wont be touched
		GUInt32 i, j = ChildrenCounts();

		for (i = 0; i < j; i++) {
			GAnimTRSNode2D *child = gChildren[i];
			G_ASSERT(child != NULL);
			child->OffsetRotationTrack(-delta);
		}
	}
	OffsetRotationTrack(delta);
	// set the new pivot rotation
	gPivotRotation = NewAngle;
	return G_NO_ERROR;
}

GError GAnimTRSNode2D::SetPivotScale(const GVectBase<GReal, 2>& NewScaleFactors, const GBool AffectChildren) {

	// calculate scale delta
	GVector2 delta(1, 1);
	
	if (GMath::Abs(gPivotScale[G_X]) > G_EPSILON)
		delta[G_X] = NewScaleFactors[G_X] / gPivotScale[G_X];

	if (GMath::Abs(gPivotScale[G_Y]) > G_EPSILON)
		delta[G_Y] = NewScaleFactors[G_Y] / gPivotScale[G_Y];

	// if AffectChildren is true, counter transform them so they won't scale.
	if (AffectChildren) {
		
		GVector2 invDelta(1, 1);

		if (GMath::Abs(delta[G_X]) > G_EPSILON)
			invDelta[G_X] = 1 / delta[G_X];

		if (GMath::Abs(delta[G_Y]) > G_EPSILON)
			invDelta[G_Y] = 1 / delta[G_Y];

		// this node must be modified scaling to the scale keys the quantity 'delta'.
		// First level children, instead, must translate with a quantity '-delta'
		// children at level 2 or more deep, wont be touched
		GUInt32 i, j = ChildrenCounts();

		for (i = 0; i < j; i++) {
			GAnimTRSNode2D *child = gChildren[i];
			G_ASSERT(child != NULL);
			child->OffsetScaleTrack(invDelta);
		}
	}
	OffsetScaleTrack(delta);
	// set the new pivot rotation
	gPivotScale = NewScaleFactors;
	return G_NO_ERROR;
}

GError GAnimTRSNode2D::BaseClone(const GElement& Source) {

	const GAnimTRSNode2D& s = (const GAnimTRSNode2D&)Source;

	// first isolate children, setting as their new father a NULL father
	GUInt32 i, j = ChildrenCounts();
	for (i = 0; i < j; ++i) {
		GAnimTRSNode2D *n = gChildren[i];
		G_ASSERT(n != NULL);
		n->SetFather(NULL);
	}
	gChildren.clear();
	// detach me from my father (if exists)
	if (gFather)
		gFather->DetachChildNode(this);
	gFather = NULL;
	// copy pivot informations
	gPivotPosition = s.gPivotPosition;
	gPivotRotation = s.gPivotRotation;
	gPivotScale = s.gPivotScale;
	// give the control to base class
	return GAnimElement::BaseClone(Source);
}

}
