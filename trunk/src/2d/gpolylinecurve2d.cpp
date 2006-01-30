/****************************************************************************
** $file: amanith/src/2d/gpolylinecurve2d.cpp   0.3.0.0   edited Jan, 30 2006
**
** 2D Polyline curve segment implementation.
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

#include "amanith/2d/gpolylinecurve2d.h"
#include <algorithm>

/*!
	\file gpolylinecurve2d.cpp
	\brief Implementation of 2D poly line multicurve class.
*/

namespace Amanith {


// *********************************************************************
//                           GPolyLineCurve2D
// *********************************************************************

// constructor
GPolyLineCurve2D::GPolyLineCurve2D() : GMultiCurve2D() {
}

// constructor
GPolyLineCurve2D::GPolyLineCurve2D(const GElement* Owner) : GMultiCurve2D(Owner) {
}

// destructor
GPolyLineCurve2D::~GPolyLineCurve2D() {
}

// clear the curve (remove keys and set an empty knots interval)
void GPolyLineCurve2D::Clear() {

	gKeys.clear();
	GCurve2D::Clear();
}

// cloning function
GError GPolyLineCurve2D::BaseClone(const GElement& Source) {

	const GPolyLineCurve2D& k = (const GPolyLineCurve2D&)Source;

	// copy keys array
	gKeys = k.gKeys;
	return GMultiCurve2D::BaseClone(Source);
}

// get Index-th control point
GPoint2 GPolyLineCurve2D::Point(const GUInt32 Index) const {

	if (Index >= PointsCount())
		return GPoint2(G_MIN_REAL, G_MIN_REAL);
	return gKeys[Index].Value;
}

// set Index-th control point
GError GPolyLineCurve2D::SetPoint(const GUInt32 Index, const GPoint2& NewPoint) {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;
	gKeys[Index].Value = NewPoint;
	return G_NO_ERROR;
}

// get a key
GError GPolyLineCurve2D::Key(const GUInt32 Index, GPolyLineKey2D& KeyValue) const {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;
	KeyValue = gKeys[Index];
	return G_NO_ERROR;
}

// set a key
GError GPolyLineCurve2D::SetKey(const GUInt32 Index, const GPoint2& NewKeyValue) {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;

	gKeys[Index].Value = NewKeyValue;
	return G_NO_ERROR;
}

// get parameter corresponding to point index
GError GPolyLineCurve2D::DoGetPointParameter(const GUInt32 Index, GReal& Parameter) const {

	Parameter = gKeys[Index].Parameter;
	return G_NO_ERROR;
}

// set parameter corresponding to point index
GError GPolyLineCurve2D::DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
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

	GPolyLineKey2D tmpKey(gKeys[Index]);
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
				GDynArray<GPolyLineKey2D>::iterator it = gKeys.begin();
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
GError GPolyLineCurve2D::SetPoints(const GDynArray<GPoint2>& NewPoints,
								   const GReal NewMinValue, const GReal NewMaxValue, const GBool Uniform) {

	GUInt32 i, j = (GUInt32)NewPoints.size();
	GReal step, u, len;
	GPolyLineKey2D tmpkey;
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
		return (GCurve2D::SetDomain(requestedInterval.Start(), requestedInterval.Start()));
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
			len += Distance(NewPoints[i], NewPoints[i + 1]);

		if (len <= G_EPSILON)
			return G_INVALID_PARAMETER;

		Clear();
		tmpkey.Parameter = requestedInterval.Start();
		tmpkey.Value = NewPoints[0];
		gKeys.push_back(tmpkey);
		for (i = 1; i < j - 1; i++) {
			tmpkey.Parameter += Distance(NewPoints[i], NewPoints[i - 1]) / len;
			tmpkey.Value = NewPoints[i];
			gKeys.push_back(tmpkey);
		}
		// to avoid numeric instabilities we force last key to be equal at requestedInterval.End
		tmpkey.Parameter = requestedInterval.End();
		tmpkey.Value = NewPoints[j - 1];
		gKeys.push_back(tmpkey);
	}
	return (GCurve2D::SetDomain(NewMinValue, NewMaxValue));
}

inline bool PolyLineKeyLE(const Amanith::GPolyLineKey2D& k1, const Amanith::GPolyLineKey2D& k2) {

	if (k1.Parameter <= k2.Parameter)
		return G_TRUE;
	return G_FALSE;
}

inline bool PolyLineKeyL(const Amanith::GPolyLineKey2D& k1, const Amanith::GPolyLineKey2D& k2) {

	if (k1.Parameter < k2.Parameter)
		return G_TRUE;
	return G_FALSE;
}

// sort keys
void GPolyLineCurve2D::SortKeys() {

	std::sort(gKeys.begin(), gKeys.end(), PolyLineKeyLE);
}

// set keys
GError GPolyLineCurve2D::SetKeys(const GDynArray<GPolyLineKey2D>& NewKeys) {

	GUInt32 i = (GUInt32)NewKeys.size();
	if (i < 1)
		return G_INVALID_PARAMETER;

	gKeys = NewKeys;
	SortKeys();
	return GCurve2D::SetDomain(gKeys[0].Parameter, gKeys[i - 1].Parameter);
}


// given a parameter value, it returns the index such as parameter is between keys [index, index+1)
GBool GPolyLineCurve2D::ParamToKeyIndex(const GReal Param, GUInt32& KeyIndex) const {

	GDynArray<GPolyLineKey2D>::const_iterator result;
	GPolyLineKey2D tmpKey;

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

// get max variation (chordal distance) in the domain range
//GReal GPolyLineCurve2D::Variation(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1) const {
GReal GPolyLineCurve2D::Variation() const {

	GUInt32 i, j = (GUInt32)gKeys.size();

	if (j < 3)
		return 0;

	GReal tmpDist, dist = -1;
	GLineSegment2 ls(gKeys[0].Value, gKeys[j - 1].Value);

	for (i = 1; i < j - 1; ++i) {
		tmpDist = DistanceSquared(gKeys[i].Value, ls);
		if (tmpDist > dist)
			dist = tmpDist;
	}
	return dist;
/*
	GLineSegment2 ls(p0, p1);
	GUInt32 keyIndex, i;
	GReal parameter, dist, tmpDist;
	GBool b;

	b = ParamToKeyIndex(u0, keyIndex);
	if (b) {
		keyIndex++;
		i = PointsCount();
		dist = -1;
		parameter = u0;
		while (keyIndex < i &&  gKeys[keyIndex].Parameter < u1) {
			tmpDist = DistanceSquared(gKeys[keyIndex].Value, ls);
			if (tmpDist > dist)
				dist = tmpDist;
			// next key
			keyIndex++;
		}
		return dist;
	}
	return 0;*/
}

// cut the curve, giving the 2 new set of control points that represents 2 poly-line curves (with the
// same degree of the original one)
GError GPolyLineCurve2D::DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const {

	GUInt32 keyIndex;
	GReal ratio;
	GBool b;

	GPolyLineCurve2D *lCurve = (GPolyLineCurve2D *)LeftCurve;
	GPolyLineCurve2D *rCurve = (GPolyLineCurve2D *)RightCurve;

	b = ParamToKeyIndex(u, keyIndex);
	G_ASSERT(b == G_TRUE);

	if (lCurve) {
		// clear internal structures
		lCurve->Clear();
		// set knots interval
		lCurve->GCurve2D::SetDomain(DomainStart(), u);
		// copy all previous keys
		for (GUInt32 i = 0; i <= keyIndex; i++)
			lCurve->gKeys.push_back(gKeys[i]);
		// include "cut" point if is not shared
		if (GMath::Abs(u - gKeys[keyIndex].Parameter) > G_EPSILON) {
			ratio = (u - gKeys[keyIndex].Parameter) / (gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter);
			GPolyLineKey2D tmpKey(u, GMath::Lerp(ratio, gKeys[keyIndex].Value, gKeys[keyIndex + 1].Value));
			lCurve->gKeys.push_back(tmpKey);
		}
	}

	if (rCurve) {
		// clear internal structures
		rCurve->Clear();
		// set knots interval
		rCurve->GCurve2D::SetDomain(u, DomainEnd());
		// include "cut" point if is not shared
		if (GMath::Abs(u - gKeys[keyIndex].Parameter) > G_EPSILON) {
			ratio = (u - gKeys[keyIndex].Parameter) / (gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter);
			GPolyLineKey2D tmpKey(u, GMath::Lerp(ratio, gKeys[keyIndex].Value, gKeys[keyIndex + 1].Value));
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
GError GPolyLineCurve2D::DoAddPoint(const GReal Parameter, const GPoint2 *NewPoint, GUInt32& Index,
									GBool& AlreadyExists) {

	GInt32 i = (GInt32)PointsCount();

	// empty curve
	if (i == 0) {
		if (!NewPoint)
			return G_INVALID_OPERATION;
		gKeys.push_back(GPolyLineKey2D(Parameter, *NewPoint));
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
			gKeys.insert(gKeys.begin(), GPolyLineKey2D(Parameter, *NewPoint));
			Index = 0;
			AlreadyExists = G_FALSE;
		}
		// insert back
		else {
			gKeys.push_back(GPolyLineKey2D(Parameter, *NewPoint));
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
		gKeys.insert(gKeys.begin(), GPolyLineKey2D(Parameter, *NewPoint));
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
		gKeys.push_back(GPolyLineKey2D(Parameter, *NewPoint));
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
	GDynArray<GPolyLineKey2D>::iterator it = gKeys.begin();
	if (NewPoint) {
		it += (keyIndex + 1);
		gKeys.insert(it, GPolyLineKey2D(Parameter, *NewPoint));
		Index = keyIndex + 1;
	}
	else {
		GReal ratio = (Parameter - gKeys[keyIndex].Parameter) / (gKeys[keyIndex + 1].Parameter- gKeys[keyIndex].Parameter);
		it += (keyIndex + 1);
		gKeys.insert(it, GPolyLineKey2D(Parameter, GMath::Lerp(ratio, gKeys[keyIndex].Value, gKeys[keyIndex + 1].Value)));
		Index = keyIndex + 1;
	}
	AlreadyExists = G_FALSE;
	return G_NO_ERROR;
}

// remove a point(key); index is ensured to be valid and we are sure that after removing we'll have
// (at least) a minimal (2-keys made) multi-curve
GError GPolyLineCurve2D::DoRemovePoint(const GUInt32 Index) {

	GDynArray<GPolyLineKey2D>::iterator it = gKeys.begin();

	// go tho Index-th position
	it += Index;
	// lets erase key
	gKeys.erase(it);
	return G_NO_ERROR;
}


// set global parameters corresponding to the start point and to the end point
GError GPolyLineCurve2D::SetDomain(const GReal NewMinValue, const GReal NewMaxValue) {
	
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
	return (GCurve2D::SetDomain(newInterval.Start(), newInterval.End()));
}

// intersect the curve with a ray, and returns a list of intersections
GBool GPolyLineCurve2D::IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
									 const GReal Precision, const GUInt32 MaxIterations) const {

	GLineSegment2 ls;
	GInt32 i, j = (GInt32)PointsCount();
	GUInt32 flags;
	GPoint2 p;
	GVector2 intersectionInfo;
	GReal locParams[2], lastIntersection, ratio, tolerance;
	GBool b, result = G_FALSE;

	if (j < 2)
		return G_FALSE;

	// just to avoid warnings
	if (MaxIterations) {
	}

	tolerance = GMath::Max(G_EPSILON, Precision);
	lastIntersection = -1;

	for (i = 0; i < j - 1; i++) {
		// build a line segment from key (i) to key (i+1)
		ls.SetStartPoint(gKeys[i].Value);
		ls.SetEndPoint(gKeys[i + 1].Value);
		// check for intersection
		b = Intersect(ls, NormalizedRay, flags, locParams);
		result |= b;
		if (b) {
			// in case of tangent, just pick only first intersection
			if (flags & INFINITE_SOLUTIONS) {
				if (GMath::Abs(lastIntersection - locParams[0]) > tolerance) {
					p = NormalizedRay.Origin() + locParams[0] * NormalizedRay.Direction();
					ratio = Distance(p, gKeys[i].Value) / Distance(gKeys[i].Value, gKeys[i + 1].Value);
					// set and save the new solution found
					intersectionInfo.Set(GMath::Lerp(ratio, gKeys[i].Parameter, gKeys[i + 1].Parameter), locParams[0]);
					Intersections.push_back(intersectionInfo);
					lastIntersection = locParams[0];
				}
				if (GMath::Abs(lastIntersection - locParams[1]) > tolerance) {
					p = NormalizedRay.Origin() + locParams[1] * NormalizedRay.Direction();
					ratio = Distance(p, gKeys[i].Value) / Distance(gKeys[i].Value, gKeys[i + 1].Value);
					// set and save the new solution found
					intersectionInfo.Set(GMath::Lerp(ratio, gKeys[i].Parameter, gKeys[i + 1].Parameter), locParams[1]);
					Intersections.push_back(intersectionInfo);
					lastIntersection = locParams[0];
				}
			}
			// in this case we have just a single solution
			else {
				if (GMath::Abs(lastIntersection - locParams[0]) > tolerance) {
					p = NormalizedRay.Origin() + locParams[0] * NormalizedRay.Direction();
					ratio = Distance(p, gKeys[i].Value) / Distance(gKeys[i].Value, gKeys[i + 1].Value);
					// set and save the new solution found
					intersectionInfo.Set(GMath::Lerp(ratio, gKeys[i].Parameter, gKeys[i + 1].Parameter), locParams[0]);
					Intersections.push_back(intersectionInfo);
					lastIntersection = locParams[0];
				}
			}
		}
	}
	return result;
}


// returns the length of the curve between the 2 specified global parameter values
GReal GPolyLineCurve2D::Length(const GReal u0, const GReal u1, const GReal MaxError) const {

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
				len += Amanith::Length(gKeys[i + 1].Value - gKeys[i].Value) * (1 - ratio);
			}
			else
				len += Amanith::Length(gKeys[i + 1].Value - gKeys[i].Value);
			// jump to the next segment
			i++;
		}
		// this is the case of the last interested segment, so calculate the remaining curve piece and exit
		else {
			if (gKeys[i].Parameter < requestedInterval.Start()) {
				ratio = (requestedInterval.Start() - gKeys[i].Parameter) / (gKeys[i + 1].Parameter - gKeys[i].Parameter);
				ratio2 = (requestedInterval.End() - gKeys[i].Parameter) / (gKeys[i + 1].Parameter - gKeys[i].Parameter);
				len += (ratio2 - ratio) * Amanith::Length(gKeys[i + 1].Value - gKeys[i].Value);
			}
			else {
				ratio = (requestedInterval.End() - gKeys[i].Parameter) / (gKeys[i + 1].Parameter - gKeys[i].Parameter);
				len += ratio * Amanith::Length(gKeys[i + 1].Value - gKeys[i].Value);
			}
			break;
		}
	}
	return len;
}

GError GPolyLineCurve2D::Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
								 const GBool IncludeLastPoint) const {

	GInt32 i, j = (GInt32)PointsCount();

	if (j < 2)
		return G_INVALID_OPERATION;

	// just to avoid warnings
	if (MaxDeviation) {
	}

	if (!IncludeLastPoint)
		j--;

	for (i = 0; i < j; i++)
		Contour.push_back(gKeys[i].Value);
	return G_NO_ERROR;

}

// return the curve value calculated at global parameter u
GPoint2 GPolyLineCurve2D::Evaluate(const GReal u) const {

	GUInt32 keyIndex;
	GBool b;
	GReal ratio;

	if (PointsCount() < 1)
		return GPoint2(G_MIN_REAL, G_MIN_REAL);

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
GVector2 GPolyLineCurve2D::Derivative(const GDerivativeOrder Order, const GReal u) const {

	GUInt32 keyIndex;
	GBool b;

	if (Order >= G_SECOND_ORDER_DERIVATIVE || PointsCount() < 2)
		return GVector2(0, 0);

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

void GPolyLineCurve2D::DerivativeLR(const GDerivativeOrder Order, const GReal u,
									GVector2& LeftDerivative, GVector2& RightDerivative) const {

	if (Order >= G_SECOND_ORDER_DERIVATIVE || PointsCount() < 2) {
		LeftDerivative = RightDerivative = GVector2(0, 0);
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
