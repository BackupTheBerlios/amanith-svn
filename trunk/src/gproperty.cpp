/****************************************************************************
** $file: amanith/src/gproperty.cpp   0.1.0.0   edited Jun 30 08:00
**
** Basic animated properties implementation.
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

#include "amanith/gproperty.h"

/*!
	\file gproperty.cpp
	\brief Implementation of basic animated properties.
*/

namespace Amanith {


// *********************************************************************
//                         GHermiteProperty1D
// *********************************************************************

GHermiteProperty1D::GHermiteProperty1D() : GProperty() {
}

GHermiteProperty1D::GHermiteProperty1D(const GElement* Owner) : GProperty(Owner) {
}

GHermiteProperty1D::~GHermiteProperty1D() {
}

GError GHermiteProperty1D::DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const {

	GHermiteKey1D tmpKey;

	GError err = gInterpolationCurve.Key(Index, tmpKey);
	if (err == G_NO_ERROR) {
		OutputKey.SetKeyType(G_REAL_KEY);
		OutputKey.SetTimePosition(tmpKey.Parameter);
		OutputKey.SetValue(tmpKey.Value);
	}
	return err;
}

GError GHermiteProperty1D::DoSetKey(const GUInt32 Index, const GKeyValue& NewKeyValue) {

	return gInterpolationCurve.SetPoint(Index, NewKeyValue.RealValue());
}

GError GHermiteProperty1D::DoAddKey(const GTimeValue TimePos, GUInt32& Index, GBool& AlreadyExists) {

	return gInterpolationCurve.AddPoint(TimePos, Index, AlreadyExists);
}

GError GHermiteProperty1D::DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex,
								     GBool& AlreadyExists) {

	return gInterpolationCurve.SetPointParameter(Index, NewTimePos, NewIndex, AlreadyExists);
}

GError GHermiteProperty1D::DoRemoveKey(const GUInt32 Index) {

	return gInterpolationCurve.RemovePoint(Index);
}

GError GHermiteProperty1D::DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue TimePos,
									  const GValueMethod GetMethod) const {

	//! \todo Implement GHermiteProperty1D::DoGetValue with G_RELATIVE_VALUE calculation method.
	if (GetMethod == G_RELATIVE_VALUE)
		return G_MISSED_FEATURE;

	GReal localValue;
	GInt32 i = gInterpolationCurve.PointsCount();

	if (i == 0) {
		G_ASSERT(0 == 1);
		return G_NO_ERROR;
	}
	else
	if (i == 1) {
		OutputValue.SetValue(gInterpolationCurve.Point(0));
		OutputValue.SetTimePosition(TimePos);
	}
	else {
		// extract value
		localValue = gInterpolationCurve.Evaluate(TimePos);
		OutputValue.SetValue(localValue);
		OutputValue.SetTimePosition(TimePos);
		// set interval of validity
		ValidInterval &= GTimeInterval(TimePos, TimePos);
	}
	return G_NO_ERROR;
}

GError GHermiteProperty1D::DoSetValue(const GKeyValue& InputValue, const GTimeValue TimePos, 
									  const GValueMethod SetMethod) {

	//! \todo Implement GHermiteProperty1D::DoSetValue with G_RELATIVE_VALUE calculation method.
	if (SetMethod == G_RELATIVE_VALUE)
		return G_MISSED_FEATURE;

	GUInt32 i;
	GBool alreadyExist;

	return gInterpolationCurve.AddPoint(TimePos, InputValue.RealValue(), i, alreadyExist);
}

GInt32 GHermiteProperty1D::DoGetKeysCount() const {

	return gInterpolationCurve.PointsCount();
}

// build a new keys track; the specified array is ensure to contain at least 1 key
GError GHermiteProperty1D::DoSetKeys(const GDynArray<GKeyValue>& Keys) {

	GUInt32 i, j = (GUInt32)Keys.size();
	GDynArray<GHermiteKey1D> tmpKeys(j);

	for (i = 0; i < j; i++)
		tmpKeys[i] = GHermiteKey1D(Keys[i].TimePosition(), Keys[i].RealValue());

	// here tangents are (0, 0)
	GError err = gInterpolationCurve.SetKeys(tmpKeys);
	if (err == G_NO_ERROR) {
		// now we must re-calculate tangents using Catmull-Rom schema
		gInterpolationCurve.RecalcSmoothTangents();
	}
	return err;
}

// get a full Hermite key, specifying index
GError GHermiteProperty1D::HermiteKey(const GUInt32 Index, GHermiteKey1D& OutputKey) const {

	return gInterpolationCurve.Key(Index, OutputKey);
}

// set a full Hermite key, specifying index and all values
GError GHermiteProperty1D::SetHermiteKey(const GUInt32 Index, const GReal NewKeyValue,
										 const GReal InTangent, const GReal OutTangent) {

	return gInterpolationCurve.SetKey(Index, NewKeyValue, InTangent, OutTangent);
}

// set all keys (they don't need to be sorted by time), specifying full Hermite values
GError GHermiteProperty1D::SetHermiteKeys(const GDynArray<GHermiteKey1D>& Keys) {

	return gInterpolationCurve.SetKeys(Keys);
}

GError GHermiteProperty1D::BaseClone(const GElement& Source) {

	const GHermiteProperty1D& p = (const GHermiteProperty1D&)Source;
	GError err;

	// copy interpolation curve
	err = gInterpolationCurve.CopyFrom(p.gInterpolationCurve);
	if (err != G_NO_ERROR)
		return err;
	return GProperty::BaseClone(Source);
}


// *********************************************************************
//                         GLinearProperty1D
// *********************************************************************

GLinearProperty1D::GLinearProperty1D() : GProperty() {
}

GLinearProperty1D::GLinearProperty1D(const GElement* Owner) : GProperty(Owner) {
}

GLinearProperty1D::~GLinearProperty1D() {
}

GError GLinearProperty1D::DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const {

	GPolyLineKey1D tmpKey;

	GError err = gInterpolationCurve.Key(Index, tmpKey);
	if (err == G_NO_ERROR) {
		OutputKey.SetKeyType(G_REAL_KEY);
		OutputKey.SetTimePosition(tmpKey.Parameter);
		OutputKey.SetValue(tmpKey.Value);
	}
	return err;
}

GError GLinearProperty1D::DoSetKey(const GUInt32 Index, const GKeyValue& NewKeyValue) {

	return gInterpolationCurve.SetPoint(Index, NewKeyValue.RealValue());
}

GError GLinearProperty1D::DoAddKey(const GTimeValue TimePos, GUInt32& Index, GBool& AlreadyExists) {

	return gInterpolationCurve.AddPoint(TimePos, Index, AlreadyExists);
}

GError GLinearProperty1D::DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex,
								     GBool& AlreadyExists) {

	return gInterpolationCurve.SetPointParameter(Index, NewTimePos, NewIndex, AlreadyExists);
}

GError GLinearProperty1D::DoRemoveKey(const GUInt32 Index) {

	return gInterpolationCurve.RemovePoint(Index);
}

GError GLinearProperty1D::DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue TimePos,
									  const GValueMethod GetMethod) const {

	//! \todo Implement GLinearProperty1D::DoGetValue with G_RELATIVE_VALUE calculation method.
	if (GetMethod == G_RELATIVE_VALUE)
		return G_MISSED_FEATURE;

	GReal localValue;
	GInt32 i = gInterpolationCurve.PointsCount();

	if (i == 0) {
		G_ASSERT(0 == 1);
		return G_NO_ERROR;
	}
	else
	if (i == 1) {
		OutputValue.SetValue(gInterpolationCurve.Point(0));
		OutputValue.SetTimePosition(TimePos);
	}
	else {
		// extract value
		localValue = gInterpolationCurve.Evaluate(TimePos);
		OutputValue.SetValue(localValue);
		OutputValue.SetTimePosition(TimePos);
		// set interval of validity
		ValidInterval &= GTimeInterval(TimePos, TimePos);
	}
	return G_NO_ERROR;
}

GError GLinearProperty1D::DoSetValue(const GKeyValue& InputValue, const GTimeValue TimePos, const GValueMethod SetMethod) {

	//! \todo Implement GLinearProperty1D::DoSetValue with G_RELATIVE_VALUE calculation method.
	if (SetMethod == G_RELATIVE_VALUE)
		return G_MISSED_FEATURE;

	GUInt32 i;
	GBool alreadyExist;

	return gInterpolationCurve.AddPoint(TimePos, InputValue.RealValue(), i, alreadyExist);
}

GInt32 GLinearProperty1D::DoGetKeysCount() const {

	return gInterpolationCurve.PointsCount();
}

// build a new keys track; the specified array is ensure to contain at least 1 key
GError GLinearProperty1D::DoSetKeys(const GDynArray<GKeyValue>& Keys) {

	GUInt32 i, j = (GUInt32)Keys.size();
	GDynArray<GPolyLineKey1D> tmpKeys(j);

	for (i = 0; i < j; i++)
		tmpKeys[i] = GPolyLineKey1D(Keys[i].TimePosition(), Keys[i].RealValue());

	return gInterpolationCurve.SetKeys(tmpKeys);
}

GError GLinearProperty1D::BaseClone(const GElement& Source) {

	const GLinearProperty1D& p = (const GLinearProperty1D&)Source;
	GError err;

	// copy interpolation curve
	err = gInterpolationCurve.CopyFrom(p.gInterpolationCurve);
	if (err != G_NO_ERROR)
		return err;
	return GProperty::BaseClone(Source);
}


// *********************************************************************
//                        GConstantProperty1D
// *********************************************************************

GConstantProperty1D::GConstantProperty1D() : GProperty() {
}

GConstantProperty1D::GConstantProperty1D(const GElement* Owner) : GProperty(Owner) {
}

GConstantProperty1D::~GConstantProperty1D() {
}

GError GConstantProperty1D::DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const {

	GPolyLineKey1D tmpKey;

	GError err = gInterpolationCurve.Key(Index, tmpKey);
	if (err == G_NO_ERROR) {
		OutputKey.SetKeyType(G_REAL_KEY);
		OutputKey.SetTimePosition(tmpKey.Parameter);
		OutputKey.SetValue(tmpKey.Value);
	}
	return err;
}

GError GConstantProperty1D::DoSetKey(const GUInt32 Index, const GKeyValue& NewKeyValue) {

	return gInterpolationCurve.SetPoint(Index, NewKeyValue.RealValue());
}

GError GConstantProperty1D::DoAddKey(const GTimeValue TimePos, GUInt32& Index, GBool& AlreadyExists) {

	return gInterpolationCurve.AddPoint(TimePos, Index, AlreadyExists);
}

GError GConstantProperty1D::DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex,
								     GBool& AlreadyExists) {

	return gInterpolationCurve.SetPointParameter(Index, NewTimePos, NewIndex, AlreadyExists);
}

GError GConstantProperty1D::DoRemoveKey(const GUInt32 Index) {

	return gInterpolationCurve.RemovePoint(Index);
}

GError GConstantProperty1D::DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue TimePos,
									   const GValueMethod GetMethod) const {

	//! \todo Implement GConstantProperty1D::DoGetValue with G_RELATIVE_VALUE calculation method.
	if (GetMethod == G_RELATIVE_VALUE)
		return G_MISSED_FEATURE;

	GInt32 i = gInterpolationCurve.PointsCount();

	if (i == 0) {
		G_ASSERT(0 == 1);
		return G_NO_ERROR;
	}
	else
	if (i == 1) {
		OutputValue.SetValue(gInterpolationCurve.Point(0));
		OutputValue.SetTimePosition(TimePos);
	}
	else {
		GUInt32 keyIndex;

		// this check is needed because ParamToKeyIndex return the lower key index of the interval where time
		// is included. If Time is equal to DomainEnd(), then the index keyIndex+1 is not valid!
		if (TimePos == gInterpolationCurve.DomainEnd()) {
			OutputValue.SetValue(gInterpolationCurve.EndPoint());
			OutputValue.SetTimePosition(TimePos);
			// set interval of validity
			ValidInterval &= GTimeInterval(TimePos, TimePos);
		}
		else {
			GBool b = gInterpolationCurve.ParamToKeyIndex(TimePos, keyIndex);
			G_ASSERT(b == G_TRUE);
			if (b != G_TRUE)
				return G_UNKNOWN_ERROR;
			// extract value
			OutputValue.SetValue(gInterpolationCurve.Point(keyIndex));
			OutputValue.SetTimePosition(TimePos);
			// set interval of validity
			GReal t;
			gInterpolationCurve.PointParameter(keyIndex + 1, t);
			ValidInterval &= GTimeInterval(TimePos, t);
		}
	}
	return G_NO_ERROR;
}

GError GConstantProperty1D::DoSetValue(const GKeyValue& InputValue, const GTimeValue TimePos,
									   const GValueMethod SetMethod) {

	//! \todo Implement GConstantProperty1D::DoSetValue with G_RELATIVE_VALUE calculation method.
	if (SetMethod == G_RELATIVE_VALUE)
		return G_MISSED_FEATURE;

	GUInt32 i;
	GBool alreadyExist;

	return gInterpolationCurve.AddPoint(TimePos, InputValue.RealValue(), i, alreadyExist);
}

GInt32 GConstantProperty1D::DoGetKeysCount() const {

	return gInterpolationCurve.PointsCount();
}

// build a new keys track; the specified array is ensure to contain at least 1 key
GError GConstantProperty1D::DoSetKeys(const GDynArray<GKeyValue>& Keys) {

	GUInt32 i, j = (GUInt32)Keys.size();
	GDynArray<GPolyLineKey1D> tmpKeys(j);

	for (i = 0; i < j; i++)
		tmpKeys[i] = GPolyLineKey1D(Keys[i].TimePosition(), Keys[i].RealValue());

	return gInterpolationCurve.SetKeys(tmpKeys);
}

GError GConstantProperty1D::BaseClone(const GElement& Source) {

	const GConstantProperty1D& p = (const GConstantProperty1D&)Source;
	GError err;

	// copy interpolation curve
	err = gInterpolationCurve.CopyFrom(p.gInterpolationCurve);
	if (err != G_NO_ERROR)
		return err;
	return GProperty::BaseClone(Source);
}

}
