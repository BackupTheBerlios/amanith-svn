/****************************************************************************
** $file: amanith/src/1d/gpolylinecurve1d.cpp   0.2.0.0   edited Dec, 12 2005
**
** 1D Polyline curve segment implementation.
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

#include "amanith/1d/gpolylinecurve1d.h"
#include <algorithm>

/*!
	\file gpolylinecurve1d.cpp
	\brief Implementation of 1D poly line multicurve class.
*/

namespace Amanith {


// *********************************************************************
//                           GPolyLineCurve1D
// *********************************************************************

// constructor
GPolyLineCurve1D::GPolyLineCurve1D() : GMultiCurve1D() {
}

// constructor
GPolyLineCurve1D::GPolyLineCurve1D(const GElement* Owner) : GMultiCurve1D(Owner) {
}

// destructor
GPolyLineCurve1D::~GPolyLineCurve1D() {
}

// clear the curve (remove keys and set an empty knots interval)
void GPolyLineCurve1D::Clear() {

	gKeys.clear();
	GCurve1D::Clear();
}

// cloning function
GError GPolyLineCurve1D::BaseClone(const GElement& Source) {

	const GPolyLineCurve1D& k = (const GPolyLineCurve1D&)Source;

	// copy keys array
	gKeys = k.gKeys;
	return GMultiCurve1D::BaseClone(Source);
}

// get Index-th control point
GReal GPolyLineCurve1D::Point(const GUInt32 Index) const {

	if (Index >= PointsCount())
		return G_MIN_REAL;
	return gKeys[Index].Value;
}

// set Index-th control point
GError GPolyLineCurve1D::SetPoint(const GUInt32 Index, const GReal NewPoint) {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;
	gKeys[Index].Value = NewPoint;
	return G_NO_ERROR;
}

// get a key
GError GPolyLineCurve1D::Key(const GUInt32 Index, GPolyLineKey1D& KeyValue) const {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;
	KeyValue = gKeys[Index];
	return G_NO_ERROR;
}

// set a key
GError GPolyLineCurve1D::SetKey(const GUInt32 Index, const GReal NewKeyValue) {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;

	gKeys[Index].Value = NewKeyValue;
	return G_NO_ERROR;
}

// get parameter corresponding to point index
GError GPolyLineCurve1D::DoGetPointParameter(const GUInt32 Index, GReal& Parameter) const {

	Parameter = gKeys[Index].Parameter;
	return G_NO_ERROR;
}

// set parameter corresponding to point index
GError GPolyLineCurve1D::DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
											 GUInt32& NewIndex, GBool& AlreadyExists) {

	GUInt32 i, j = PointsCount(), deleteIndex = 0;
	GError err;
	GBool b, sameInterval, deleteKey;

	// if point is not moved, just return
	if (GMath::Abs(NewParamValue - gKeys[Index].Parameter) <= G_EPSILON) {
		NewIndex = Index;
		AlreadyExists = G_TRUE;
		return G_NO_ERROR;
	}

	GPolyLineKey1D tmpKey(gKeys[Index]);
	tmpKey.Parameter = NewParamValue;

	b = ParamToKeyIndex(NewParamValue, i);
	if (b) {
		if (Index == i || i == Index - 1)
			sameInterval = G_TRUE;
		else
			sameInterval = G_FALSE;

		if (GMath::Abs(gKeys[i].Parameter - NewParamValue) <= G_EPSILON) {
			gKeys[i].Value = tmpKey.Value;
			NewIndex = i;
			AlreadyExists = G_TRUE;
			deleteIndex = Index;
			deleteKey = G_TRUE;
		}
		else
		if (GMath::Abs(gKeys[i + 1].Parameter - NewParamValue) <= G_EPSILON) {
			gKeys[i + 1].Value = tmpKey.Value;
			NewIndex = i + 1;
			AlreadyExists = G_TRUE;
			deleteIndex = Index;
			deleteKey = G_TRUE;
		}
		else {
			if (sameInterval) {
				gKeys[Index].Parameter = NewParamValue;
				deleteKey = G_FALSE;
				NewIndex = Index;
			}
			else {
				GDynArray<GPolyLineKey1D>::iterator it = gKeys.begin();
				it += (i + 1);
				gKeys.insert(it, tmpKey);
				NewIndex = i + 1;
				if (Index < i)
					deleteIndex = Index;
				else
					deleteIndex = Index + 1;
				AlreadyExists = G_FALSE;
				deleteKey = G_TRUE;
			}
		}
	}
	else {
		deleteKey = G_TRUE;
		if (GMath::Abs(DomainStart() - NewParamValue) <= G_EPSILON) {
			gKeys[0].Value = tmpKey.Value;
			NewIndex = 0;
			deleteIndex = Index;
			AlreadyExists = G_TRUE;
		}
		else
		if (GMath::Abs(DomainEnd() - NewParamValue) <= G_EPSILON) {
			gKeys[j - 1].Value = tmpKey.Value;
			NewIndex = j - 1;
			deleteIndex = Index;
			AlreadyExists = G_TRUE;
		}
		else {
			// append key at the beginning
			if (NewParamValue < DomainStart()) {
				gKeys.insert(gKeys.begin(), tmpKey);
				deleteIndex = Index + 1;
				NewIndex = 0;
			}
			else {
				// append key at the end
				gKeys.push_back(tmpKey);
				deleteIndex = Index;
				NewIndex = (GUInt32)gKeys.size() - 1;
			}
			AlreadyExists = G_FALSE;
		}
	}

	// check if we have to delete key
	err = G_NO_ERROR;
	if (deleteKey) {
		err = DoRemovePoint(deleteIndex);
		if (NewIndex > deleteIndex)
			NewIndex--;
	}
	return err;
}

// set control points
GError GPolyLineCurve1D::SetPoints(const GDynArray<GReal>& NewPoints,
								   const GReal NewMinValue, const GReal NewMaxValue, const GBool Uniform) {

	GUInt32 i, j = (GUInt32)NewPoints.size();
	GReal step, u, len;
	GPolyLineKey1D tmpkey;
	GInterval<GReal> requestedInterval(NewMinValue, NewMaxValue);

	// a multicurve must be made of almost 2 points/key
	if (j < 1 || requestedInterval.Length() < G_EPSILON)
		return G_INVALID_PARAMETER;

	// single key curve
	if (j == 1) {
		Clear();
		tmpkey.Value = NewPoints[0];
		tmpkey.Parameter = requestedInterval.Start();
		gKeys.push_back(tmpkey);
		return (GCurve1D::SetDomain(requestedInterval.Start(), requestedInterval.Start()));
	}

	if (Uniform) {
		Clear();
		// uniform parameter distribution
		step = requestedInterval.Length() / (GReal)(j - 1);
		u = requestedInterval.Start();

		for (i = 0; i < j - 1; i++) {
			tmpkey.Parameter = u;
			tmpkey.Value = NewPoints[i];
			gKeys.push_back(tmpkey);
			u += step;
		}
		// to avoid numeric instabilities we force last key to be equal at requestedInterval.End
		tmpkey.Parameter = requestedInterval.End();
		tmpkey.Value = NewPoints[j - 1];
		gKeys.push_back(tmpkey);
	}
	else {
		// chord length parameter distribution, first calculate global length
		len = 0;
		for (i = 0; i < j - 1; i++)
			len += GMath::Abs(NewPoints[i] - NewPoints[i + 1]);

		if (len <= G_EPSILON)
			return G_INVALID_PARAMETER;

		Clear();
		tmpkey.Parameter = requestedInterval.Start();
		tmpkey.Value = NewPoints[0];
		gKeys.push_back(tmpkey);
		for (i = 1; i < j - 1; i++) {
			tmpkey.Parameter += GMath::Abs(NewPoints[i] - NewPoints[i - 1]) / len;
			tmpkey.Value = NewPoints[i];
			gKeys.push_back(tmpkey);
		}
		// to avoid numeric instabilities we force last key to be equal at requestedInterval.End
		tmpkey.Parameter = requestedInterval.End();
		tmpkey.Value = NewPoints[j - 1];
		gKeys.push_back(tmpkey);
	}
	return (GCurve1D::SetDomain(NewMinValue, NewMaxValue));
}

inline bool PolyLineKeyLE(const Amanith::GPolyLineKey1D& k1, const Amanith::GPolyLineKey1D& k2) {

	if (k1.Parameter <= k2.Parameter)
		return G_TRUE;
	return G_FALSE;
}

inline bool PolyLineKeyL(const Amanith::GPolyLineKey1D& k1, const Amanith::GPolyLineKey1D& k2) {

	if (k1.Parameter < k2.Parameter)
		return G_TRUE;
	return G_FALSE;
}

// sort keys
void GPolyLineCurve1D::SortKeys() {

	std::sort(gKeys.begin(), gKeys.end(), PolyLineKeyLE);
}

// set keys
GError GPolyLineCurve1D::SetKeys(const GDynArray<GPolyLineKey1D>& NewKeys) {

	GUInt32 i = (GUInt32)NewKeys.size();
	if (i < 1)
		return G_INVALID_PARAMETER;

	gKeys = NewKeys;
	SortKeys();
	return GCurve1D::SetDomain(gKeys[0].Parameter, gKeys[i - 1].Parameter);
}


// given a parameter value, it returns the index such as parameter is between keys [index, index+1)
GBool GPolyLineCurve1D::ParamToKeyIndex(const GReal Param, GUInt32& KeyIndex) const {

	GDynArray<GPolyLineKey1D>::const_iterator result;
	GPolyLineKey1D tmpKey;

	tmpKey.Parameter = Param;
	result = std::lower_bound(gKeys.begin(), gKeys.end(), tmpKey, PolyLineKeyL);

	if (result == gKeys.end())
		return G_FALSE;

	if (result == gKeys.begin() && (*result).Parameter > tmpKey.Parameter)
		return G_FALSE;

	if ((*result).Parameter == tmpKey.Parameter)
		KeyIndex = (GUInt32)(result - gKeys.begin());
	else
		KeyIndex = (GUInt32)((result - gKeys.begin()) - 1);

	return G_TRUE;
}

// cut the curve, giving the 2 new set of control points that represents 2 poly-line curves (with the
// same degree of the original one)
GError GPolyLineCurve1D::DoCut(const GReal u, GCurve1D *RightCurve, GCurve1D *LeftCurve) const {

	GUInt32 keyIndex;
	GReal ratio;
	GBool b;

	GPolyLineCurve1D *lCurve = (GPolyLineCurve1D *)LeftCurve;
	GPolyLineCurve1D *rCurve = (GPolyLineCurve1D *)RightCurve;

	b = ParamToKeyIndex(u, keyIndex);
	G_ASSERT(b == G_TRUE);

	if (lCurve) {
		// clear internal structures
		lCurve->Clear();
		// set knots interval
		lCurve->GCurve1D::SetDomain(DomainStart(), u);
		// copy all previous keys
		for (GUInt32 i = 0; i <= keyIndex; i++)
			lCurve->gKeys.push_back(gKeys[i]);
		// include "cut" point if is not shared
		if (GMath::Abs(u - gKeys[keyIndex].Parameter) > G_EPSILON) {
			ratio = (u - gKeys[keyIndex].Parameter) / (gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter);
			GPolyLineKey1D tmpKey(u, GMath::Lerp(ratio, gKeys[keyIndex].Value, gKeys[keyIndex + 1].Value));
			lCurve->gKeys.push_back(tmpKey);
		}
	}

	if (rCurve) {
		// clear internal structures
		rCurve->Clear();
		// set knots interval
		rCurve->GCurve1D::SetDomain(u, DomainEnd());
		// include "cut" point if is not shared
		if (GMath::Abs(u - gKeys[keyIndex].Parameter) > G_EPSILON) {
			ratio = (u - gKeys[keyIndex].Parameter) / (gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter);
			GPolyLineKey1D tmpKey(u, GMath::Lerp(ratio, gKeys[keyIndex].Value, gKeys[keyIndex + 1].Value));
			rCurve->gKeys.push_back(tmpKey);
		}
		else
			keyIndex--;

		GUInt32 j = PointsCount();
		// copy all foregoing keys
		for (GUInt32 i = keyIndex + 1; i < j; i++)
			rCurve->gKeys.push_back(gKeys[i]);
	}
	return G_NO_ERROR;
}

// add a point; if vertex pointer is NULL point must be inserted ON curve
GError GPolyLineCurve1D::DoAddPoint(const GReal Parameter, const GReal *NewPoint, GUInt32& Index,
									GBool& AlreadyExists) {

	GInt32 i = (GInt32)PointsCount();

	// empty curve
	if (i == 0) {
		if (!NewPoint)
			return G_INVALID_OPERATION;
		gKeys.push_back(GPolyLineKey1D(Parameter, *NewPoint));
		Index = 0;
		AlreadyExists = G_FALSE;
		return G_NO_ERROR;
	}
	// 1 only key
	if (i == 1) {
		if (!NewPoint)
			return G_INVALID_OPERATION;
		// i'm overlapping key 0
		if (GMath::Abs(Parameter - gKeys[0].Parameter) <= G_EPSILON) {
			Index = 0;
			AlreadyExists = G_TRUE;
			gKeys[0].Value = *NewPoint;
		}
		else
		// insert front
		if (Parameter < gKeys[0].Parameter) {
			gKeys.insert(gKeys.begin(), GPolyLineKey1D(Parameter, *NewPoint));
			Index = 0;
			AlreadyExists = G_FALSE;
		}
		// insert back
		else {
			gKeys.push_back(GPolyLineKey1D(Parameter, *NewPoint));
			Index = 1;
			AlreadyExists = G_FALSE;
		}
		return G_NO_ERROR;
	}

	// 2 or more keys: first check outside cases, then check inside cases

	// append new point at the beginning
	if (Parameter < DomainStart() - G_EPSILON) {
		if (!NewPoint)
			return G_INVALID_OPERATION;
		gKeys.insert(gKeys.begin(), GPolyLineKey1D(Parameter, *NewPoint));
		Index = 0;
		AlreadyExists = G_FALSE;
		return G_NO_ERROR;
	}
	else
	if (Parameter <= DomainStart() + G_EPSILON) {
		if (NewPoint)
			gKeys[0].Value = *NewPoint;
		Index = 0;
		AlreadyExists = G_TRUE;
		return G_NO_ERROR;
	}
	// append new point at the end
	if (Parameter > DomainEnd() + G_EPSILON) {
		if (!NewPoint)
			return G_INVALID_OPERATION;
		gKeys.push_back(GPolyLineKey1D(Parameter, *NewPoint));
		Index = PointsCount() - 1;
		AlreadyExists = G_FALSE;
		return G_NO_ERROR;
	}
	else
	if (Parameter >= DomainEnd() - G_EPSILON) {
		if (NewPoint)
			gKeys[PointsCount() - 1].Value = *NewPoint;
		Index = PointsCount() - 1;
		AlreadyExists = G_TRUE;
		return G_NO_ERROR;
	}

	// inside curve domain
	GUInt32 keyIndex;
	GBool b = ParamToKeyIndex(Parameter, keyIndex);
	G_ASSERT(b == G_TRUE);
	if (b != G_TRUE)
		return G_UNKNOWN_ERROR;

	// test left key point
	if (GMath::Abs(gKeys[keyIndex].Parameter - Parameter) <= G_EPSILON) {
		if (NewPoint)
			gKeys[keyIndex].Value = *NewPoint;
		Index = keyIndex;
		AlreadyExists = G_TRUE;
		return G_NO_ERROR;
	}
	else
	// test right key point
	if (GMath::Abs(gKeys[keyIndex + 1].Parameter - Parameter) <= G_EPSILON) {
		if (NewPoint)
			gKeys[keyIndex + 1].Value = *NewPoint;
		Index = keyIndex + 1;
		AlreadyExists = G_TRUE;
		return G_NO_ERROR;
	}
	// in this case point must be added ON the curve, because we are not on a shared point
	GDynArray<GPolyLineKey1D>::iterator it = gKeys.begin();
	if (NewPoint) {
		it += (keyIndex + 1);
		gKeys.insert(it, GPolyLineKey1D(Parameter, *NewPoint));
		Index = keyIndex + 1;
	}
	else {
		GReal ratio = (Parameter - gKeys[keyIndex].Parameter) / (gKeys[keyIndex + 1].Parameter- gKeys[keyIndex].Parameter);
		it += (keyIndex + 1);
		gKeys.insert(it, GPolyLineKey1D(Parameter, GMath::Lerp(ratio, gKeys[keyIndex].Value, gKeys[keyIndex + 1].Value)));
		Index = keyIndex + 1;
	}
	AlreadyExists = G_FALSE;
	return G_NO_ERROR;
}

// remove a point(key); index is ensured to be valid and we are sure that after removing we'll have
// (at least) a minimal (2-keys made) multi-curve
GError GPolyLineCurve1D::DoRemovePoint(const GUInt32 Index) {

	GDynArray<GPolyLineKey1D>::iterator it = gKeys.begin();

	// go tho Index-th position
	it += Index;
	// lets erase key
	gKeys.erase(it);
	return G_NO_ERROR;
}


// set global parameters corresponding to the start point and to the end point
GError GPolyLineCurve1D::SetDomain(const GReal NewMinValue, const GReal NewMaxValue) {
	
	GInterval<GReal> newInterval(NewMinValue, NewMaxValue);
	GUInt32 i, j = PointsCount();
	GReal s, k;

	// check if new range is empty
	if (newInterval.IsEmpty())
		return G_INVALID_PARAMETER;

	// to avoid numeric instabilities, if the new interval is equal (under machine precision) to the
	// current interval, it's better to do nothing
	if ((GMath::Abs(newInterval.Start() - DomainStart()) <= G_EPSILON) &&
		(GMath::Abs(newInterval.End() - DomainEnd()) <= G_EPSILON))
		return G_NO_ERROR;

	// first check if the new interval has the same length of the current one; in this case just translate
	// knots, without doing a rescale (this avoids numerical instabilities)
	if (GMath::Abs(newInterval.Length() - Domain().Length()) <= G_EPSILON) {
		// calculate shifting amount
		k = newInterval.Start() - DomainStart();
		for (i = 0; i < j; i++)
			gKeys[i].Parameter += k;
	}
	// do a full re-parametrization
	else {
		// calculate intervals ratio
		k = newInterval.Length() / Domain().Length();
		// calculate new internal knots
		for (i = 0; i < j; i++) {
			s = (gKeys[i].Parameter - DomainStart()) * k + newInterval.Start();
			gKeys[i].Parameter = s;
		}
	}
	// set internal members
	return (GCurve1D::SetDomain(newInterval.Start(), newInterval.End()));
}

// returns the length of the curve between the 2 specified global parameter values
GReal GPolyLineCurve1D::Length(const GReal u0, const GReal u1, const GReal MaxError) const {

	GUInt32 keyIndex, j, i;
	GReal len, ratio, ratio2;
	GInterval<GReal> requestedInterval(u0, u1);

	if (PointsCount() < 2)
		return 0;

	// just avoid warnings
	if (MaxError) {
	}

	// check if requested interval is not empty
	requestedInterval &= Domain();
	if (requestedInterval.IsEmpty())
		return 0;

	// find the start key index
	ParamToKeyIndex(requestedInterval.Start(), keyIndex);

	// loops over interested segments
	i = keyIndex;
	j = PointsCount();
	len = 0;
	while (i < j) {
		if (requestedInterval.End() > gKeys[i + 1].Parameter) {
			if (gKeys[i].Parameter < requestedInterval.Start()) {
				ratio = (requestedInterval.Start() - gKeys[i].Parameter) / (gKeys[i + 1].Parameter - gKeys[i].Parameter);
				len += GMath::Abs(gKeys[i + 1].Value - gKeys[i].Value) * (1 - ratio);
			}
			else
				len += GMath::Abs(gKeys[i + 1].Value - gKeys[i].Value);
			// jump to the next segment
			i++;
		}
		// this is the case of the last interested segment, so calculate the remaining curve piece and exit
		else {
			if (gKeys[i].Parameter < requestedInterval.Start()) {
				ratio = (requestedInterval.Start() - gKeys[i].Parameter) / (gKeys[i + 1].Parameter - gKeys[i].Parameter);
				ratio2 = (requestedInterval.End() - gKeys[i].Parameter) / (gKeys[i + 1].Parameter - gKeys[i].Parameter);
				len += (ratio2 - ratio) * GMath::Abs(gKeys[i + 1].Value - gKeys[i].Value);
			}
			else {
				ratio = (requestedInterval.End() - gKeys[i].Parameter) / (gKeys[i + 1].Parameter - gKeys[i].Parameter);
				len += ratio * GMath::Abs(gKeys[i + 1].Value - gKeys[i].Value);
			}
			break;
		}
	}
	return len;
}

// return the curve value calculated at global parameter u
GReal GPolyLineCurve1D::Evaluate(const GReal u) const {

	GUInt32 keyIndex;
	GBool b;
	GReal ratio;

	if (PointsCount() < 1)
		return G_MIN_REAL;

	// check for global parameter out of range
	if (u <= DomainStart())
		return StartPoint();
	else
	if (u >= DomainEnd())
		return EndPoint();

	G_ASSERT(PointsCount() >= 2);

	b = ParamToKeyIndex(u, keyIndex);
	G_ASSERT (b == G_TRUE);
	ratio = (u - gKeys[keyIndex].Parameter) / (gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter);
	return GMath::Lerp(ratio, gKeys[keyIndex].Value, gKeys[keyIndex + 1].Value);
}

// return the derivate Order-th calculated at global parameter u
GReal GPolyLineCurve1D::Derivative(const GDerivativeOrder Order, const GReal u) const {

	GUInt32 keyIndex;
	GBool b;

	if (PointsCount() < 2)
		return G_MIN_REAL;

	if (Order >= G_SECOND_ORDER_DERIVATIVE)
		return 0;

	// clamp parameter inside valid interval
	if (u <= DomainStart())
		keyIndex = 1;
	else
	if (u >= DomainEnd())
		keyIndex = PointsCount() - 2;
	else {
		b = ParamToKeyIndex(u, keyIndex);
		G_ASSERT (b == G_TRUE);
	}
	GReal dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
	return ((gKeys[keyIndex + 1].Value - gKeys[keyIndex].Value) * dtdu);
}

void GPolyLineCurve1D::DerivativeLR(const GDerivativeOrder Order, const GReal u,
									GReal& LeftDerivative, GReal& RightDerivative) const {

	if (PointsCount() < 2) {
		LeftDerivative = RightDerivative = G_MIN_REAL;
		return;
	}
	if (Order >= G_SECOND_ORDER_DERIVATIVE) {
		LeftDerivative = RightDerivative = 0;
		return;
	}

	GReal dtdu;
	GUInt32 keyIndex;

	// clamp parameter inside valid interval
	if (u <= DomainStart()) {
		dtdu = gKeys[1].Parameter - gKeys[0].Parameter;
		LeftDerivative = RightDerivative = (gKeys[1].Value - gKeys[0].Value) * dtdu;
	}
	else
	if (u >= DomainEnd()) {
		keyIndex = PointsCount() - 2;
		dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
		LeftDerivative = RightDerivative = (gKeys[keyIndex + 1].Value - gKeys[keyIndex].Value) * dtdu;
	}
	else {
		GUInt32 keyIndex;
#ifdef _DEBUG
		GBool b = ParamToKeyIndex(u, keyIndex);
		G_ASSERT (b == G_TRUE);
#else
		ParamToKeyIndex(u, keyIndex);
#endif
		// test if point is shared
		if (GMath::Abs(u - gKeys[keyIndex].Parameter) <= G_EPSILON) {
			// left derivative
			dtdu = gKeys[keyIndex].Parameter - gKeys[keyIndex - 1].Parameter;
			LeftDerivative = (gKeys[keyIndex].Value - gKeys[keyIndex - 1].Value) * dtdu;
			// right derivative
			dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
			RightDerivative = (gKeys[keyIndex + 1].Value - gKeys[keyIndex].Value) * dtdu;
		}
		else {
			// non-shared point
			dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
			LeftDerivative = RightDerivative = (gKeys[keyIndex + 1].Value - gKeys[keyIndex].Value) * dtdu;
		}
	}
}

};	// end namespace Amanith
