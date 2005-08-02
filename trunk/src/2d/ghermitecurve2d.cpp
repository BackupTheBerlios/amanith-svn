/****************************************************************************
** $file: amanith/src/2d/g2dhermitecurve.cpp   0.1.0.0   edited Jun 30 08:00
**
** 2D Hermite curve segment implementation.
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

#include "amanith/2d/ghermitecurve2d.h"
#include "amanith/numerics/gintegration.h"
#include <algorithm>

/*!
	\file ghermitecurve2d.cpp
	\brief Implementation of 2D Hermite multicurve class.
*/

namespace Amanith {


// *********************************************************************
//                           GHermiteCurve2D
// *********************************************************************

// constructor
GHermiteCurve2D::GHermiteCurve2D() : GMultiCurve2D() {
}

// constructor
GHermiteCurve2D::GHermiteCurve2D(const GElement* Owner) : GMultiCurve2D(Owner) {
}

// destructor
GHermiteCurve2D::~GHermiteCurve2D() {
}

// clear the curve (remove keys and set an empty knots interval)
void GHermiteCurve2D::Clear() {

	gKeys.clear();
	GCurve2D::SetDomain(G_MIN_REAL, G_MIN_REAL);
}

// cloning function
GError GHermiteCurve2D::BaseClone(const GElement& Source) {

	const GHermiteCurve2D& k = (const GHermiteCurve2D&)Source;

	// copy keys array
	gKeys = k.gKeys;
	return GMultiCurve2D::BaseClone(Source);
}

// get Index-th control point
GPoint2 GHermiteCurve2D::Point(const GInt32 Index) const {

	if ((Index < 0) || (Index >= PointsCount()))
		return GPoint2(G_MIN_REAL, G_MIN_REAL);
	return gKeys[Index].Value;
}

// set Index-th control point
GError GHermiteCurve2D::SetPoint(const GInt32 Index, const GPoint2& NewPoint) {

	if ((Index < 0) || (Index >= PointsCount()))
		return G_OUT_OF_RANGE;
	gKeys[Index].Value = NewPoint;
	return G_NO_ERROR;
}

// get a key
GError GHermiteCurve2D::Key(const GUInt32 Index, GHermiteKey& KeyValue) const {

	if (Index >= (GUInt32)PointsCount())
		return G_OUT_OF_RANGE;
	KeyValue = gKeys[Index];
	return G_NO_ERROR;
}

// set a key
GError GHermiteCurve2D::SetKey(const GUInt32 Index, const GPoint2& NewKeyValue,
							   const GVector2& InTangent, const GVector2& OutTangent) {

	GInt32 i = PointsCount();

	if ((GInt32)Index >= i)
		return G_OUT_OF_RANGE;

	gKeys[Index].Value = NewKeyValue;
	gKeys[Index].InTangent = InTangent;
	gKeys[Index].OutTangent = OutTangent;

	return G_NO_ERROR;
}

// get parameter corresponding to point index
GError GHermiteCurve2D::DoGetPointParameter(const GUInt32 Index, GReal& Parameter) const {

	Parameter = gKeys[Index].Parameter;
	return G_NO_ERROR;
}

// set parameter corresponding to point index
GError GHermiteCurve2D::DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
											GUInt32& NewIndex, GBool& AlreadyExists) {

	GUInt32 i, j = (GUInt32)PointsCount(), deleteIndex = 0;
	GError err;
	GReal dtdu = 0, dtdu1 = 0, dtdu2 = 0;
	GBool b, sameInterval, deleteKey;

	// if point is not moved, just return
	if (GMath::Abs(NewParamValue - gKeys[Index].Parameter) <= G_EPSILON)
		return G_NO_ERROR;

	GHermiteKey tmpKey(gKeys[Index]);
	tmpKey.Parameter = NewParamValue;

	b = ParamToKeyIndex(NewParamValue, i);
	if (b) {
		if (Index == i || i == Index - 1)
			sameInterval = G_TRUE;
		else
			sameInterval = G_FALSE;

		if (GMath::Abs(gKeys[i].Parameter - NewParamValue) <= G_EPSILON) {
			gKeys[i].Value = tmpKey.Value;
			gKeys[i].InTangent = tmpKey.InTangent;
			gKeys[i].OutTangent = tmpKey.OutTangent;
			NewIndex = i;
			AlreadyExists = G_TRUE;
			deleteIndex = Index;
			deleteKey = G_TRUE;
		}
		else
		if (GMath::Abs(gKeys[i + 1].Parameter - NewParamValue) <= G_EPSILON) {
			gKeys[i + 1].Value = tmpKey.Value;
			gKeys[i + 1].InTangent = tmpKey.InTangent;
			gKeys[i + 1].OutTangent = tmpKey.OutTangent;
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
				GDynArray<GHermiteKey>::iterator it = gKeys.begin();
				it += (i + 1);
				// keep same global first derivative
				dtdu = gKeys[i + 1].Parameter - gKeys[i].Parameter;
				dtdu1 = NewParamValue - gKeys[i].Parameter;
				dtdu2 = gKeys[i + 1].Parameter - NewParamValue;
				gKeys[i].OutTangent *= (dtdu1 / dtdu);
				gKeys[i + 1].InTangent *= (dtdu2 / dtdu);
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
			gKeys[0].InTangent = tmpKey.InTangent;
			gKeys[0].OutTangent = tmpKey.OutTangent;
			NewIndex = 0;
			deleteIndex = Index;
			AlreadyExists = G_TRUE;
		}
		else
		if (GMath::Abs(DomainEnd() - NewParamValue) <= G_EPSILON) {
			gKeys[j - 1].Value = tmpKey.Value;
			gKeys[j - 1].InTangent = tmpKey.InTangent;
			gKeys[j - 1].OutTangent = tmpKey.OutTangent;
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

// calculate tangents using Catmull-Rom schema
void GHermiteCurve2D::CalcCatmullRomTangents(const GUInt32 Index0, const GUInt32 Index1) {

	G_ASSERT(gKeys.size() >= 2);
	GInterval<GUInt32> requestedInterval(Index0, Index1);

	if (requestedInterval.Start() >= gKeys.size())
		return;

	// 2 keys case
	if (gKeys.size() == 2) {
		if (Index0 == 0)
			gKeys[0].OutTangent = (GReal)0.5 * (gKeys[1].Value - gKeys[0].Value);
		else
		if (Index0 == 1)
			gKeys[1].InTangent = (GReal)0.5 * (gKeys[1].Value - gKeys[0].Value);
		if (Index0 == Index1)
			return;
		if (Index1 == 0)
			gKeys[0].OutTangent = (GReal)0.5 * (gKeys[1].Value - gKeys[0].Value);
		else
		if (Index1 == 1)
			gKeys[1].InTangent = (GReal)0.5 * (gKeys[1].Value - gKeys[0].Value);
		return;
	}

	// full Catmull-Rom schema (3 or more keys)
	GUInt32 i, j = (GUInt32)PointsCount(), i0, i1;
	GReal cso, csi;
	GVector2 v1, v2;

	if (requestedInterval.End() >= j)
		requestedInterval.SetEnd(j - 1);

	if (requestedInterval.Start() == 0) {
		csi = gKeys[2].Parameter - gKeys[0].Parameter;
		cso = gKeys[1].Parameter - gKeys[0].Parameter;
		v1 = (-cso / (2 * csi)) * (gKeys[2].Value - gKeys[0].Value);
		v2 = ((GReal)3 / (GReal)2) * (gKeys[1].Value - gKeys[0].Value);
		gKeys[0].OutTangent = (v1 + v2);

		i0 = requestedInterval.Start() + 1;
	}
	else
		i0 = requestedInterval.Start();

	if (requestedInterval.End() == j - 1) {
		csi = gKeys[j - 1].Parameter - gKeys[j - 3].Parameter;
		cso = gKeys[j - 1].Parameter - gKeys[j - 2].Parameter;
		v1 = (-cso / (2 * csi)) * (gKeys[j - 1].Value - gKeys[j - 3].Value);
		v2 = ((GReal)3 / (GReal)2) * (gKeys[j - 1].Value - gKeys[j - 2].Value);
		gKeys[j - 1].InTangent = (v1 + v2);

		i1 = requestedInterval.End() - 1;
	}
	else
		i1 = requestedInterval.End();

	for (i = i0; i <= i1; i++) {
		cso = (gKeys[i + 1].Parameter - gKeys[i].Parameter) / (gKeys[i + 1].Parameter - gKeys[i - 1].Parameter);
		csi = (gKeys[i].Parameter - gKeys[i - 1].Parameter) / (gKeys[i + 1].Parameter - gKeys[i - 1].Parameter);
		gKeys[i].OutTangent = cso * (gKeys[i + 1].Value - gKeys[i - 1].Value);
		gKeys[i].InTangent = csi * (gKeys[i + 1].Value - gKeys[i - 1].Value);
	}
}

// static speed evaluation callback (useful for length evaluation)
GReal GHermiteCurve2D::SegmentSpeedEvaluationCallBack(const GReal u, void *Data) {

	G_ASSERT(Data != NULL);
	GHermiteCallBackData *data = (GHermiteCallBackData *)Data;

	return (data->Curve->SegmentDerivative(data->KeyIndex, G_FIRST_ORDER_DERIVATIVE, u)).Length();
}

// calculate length of i-th segment (key (i) to key (i+1)), between 2 parameters; it suppose that
// Index is valid and also parameter range is inside specified segment range
GReal GHermiteCurve2D::SegmentLength(const GUInt32 Index, const GReal MinParam, const GReal MaxParam,
									 const GReal MaxError) const {

	G_ASSERT((GInt32)Index < PointsCount() - 1);
	G_ASSERT(MinParam >= gKeys[Index].Parameter && MaxParam <= gKeys[Index + 1].Parameter);

	GHermiteCallBackData callBackData(this, Index);
	GReal result;

	// integration over speed
	GIntegration::Romberg(result, MinParam, MaxParam, SegmentSpeedEvaluationCallBack, &callBackData, MaxError);
	return result;
}

// calculate point on i-th segment (key (i) to key (i+1)); it suppose that
// Index is valid and also parameter value is inside segment range
GPoint2 GHermiteCurve2D::SegmentEvaluate(const GUInt32 Index, const GReal Parameter) const {

	G_ASSERT((GInt32)Index < PointsCount() - 1);
	G_ASSERT(Parameter >= gKeys[Index].Parameter && Parameter <= gKeys[Index + 1].Parameter);

	GReal t = (Parameter - gKeys[Index].Parameter) / (gKeys[Index + 1].Parameter - gKeys[Index].Parameter);
	GReal t2 = t * t;
	GReal t3 = t2 * t;
	// Hermite basis functions
	GReal h1 =  2 * t3 - 3 * t2 + 1;
	GReal h2 = -2 * t3 + 3 * t2;
	GReal h3 = t3 - 2 * t2 + t;
	GReal h4 = t3 -  t2;
	return ((h1 * gKeys[Index].Value) + (h2 * gKeys[Index + 1].Value) +
			(h3 * gKeys[Index].OutTangent) + (h4 * gKeys[Index + 1].InTangent));
}

GVector2 GHermiteCurve2D::SegmentTangent(const GUInt32 Index, const GDerivativeOrder Order,
										 const GReal Parameter) const {

	G_ASSERT((GInt32)Index < PointsCount() - 1);
	G_ASSERT(Parameter >= gKeys[Index].Parameter && Parameter <= gKeys[Index + 1].Parameter);

	GReal dtdu = 1 / (gKeys[Index + 1].Parameter - gKeys[Index].Parameter);
	GReal t = (Parameter - gKeys[Index].Parameter) * dtdu;

	// first order derivative
	if (Order == G_FIRST_ORDER_DERIVATIVE) {
		GReal t2 = t * t;
		// Hermite basis functions derivatives (first order)
		GReal h1 = 6 * t2 - 6 * t;
		GReal h2 = -h1;
		GReal h3 = 3 * t2 - 4 * t + 1;
		GReal h4 = 3 * t2 - 2 * t;
		return (((h1 * gKeys[Index].Value) + (h2 * gKeys[Index + 1].Value) +
				(h3 * gKeys[Index].OutTangent) + (h4 * gKeys[Index + 1].InTangent)));
	}
	// second order derivative
	else {
		// Hermite basis functions derivatives (second order)
		GReal h1 = 12 * t - 6;
		GReal h2 = -h1;
		GReal h3 = 6 * t - 4;
		GReal h4 = 6 * t - 2;
		return ((h1 * gKeys[Index].Value) + (h2 * gKeys[Index + 1].Value) +
			(h3 * gKeys[Index].OutTangent) + (h4 * gKeys[Index + 1].InTangent));
	}
}

// calculate derivative on i-th segment (key (i) to key (i+1)); it suppose that
// Index is valid and also parameter value is inside segment range
GVector2 GHermiteCurve2D::SegmentDerivative(const GUInt32 Index, const GDerivativeOrder Order,
											const GReal Parameter) const{

	G_ASSERT((GInt32)Index < PointsCount() - 1);
	G_ASSERT(Parameter >= gKeys[Index].Parameter && Parameter <= gKeys[Index + 1].Parameter);

	GReal dtdu = 1 / (gKeys[Index + 1].Parameter - gKeys[Index].Parameter);

	// first order derivative
	if (Order == G_FIRST_ORDER_DERIVATIVE)
		return (SegmentTangent(Index, Order, Parameter) * dtdu);
	// second order derivative
	else
		return (SegmentTangent(Index, Order, Parameter) * (dtdu * dtdu));
}

// calculate variation on i-th segment (key (i) to key (i+1)); it suppose that
// Index is valid and also parameter range is inside specified segment range
GReal GHermiteCurve2D::SegmentVariation(const GUInt32 Index, const GReal MinParam, const GReal MaxParam) const {

	G_ASSERT((GInt32)Index < PointsCount() - 1);
	G_ASSERT(MinParam >= gKeys[Index].Parameter && MaxParam <= gKeys[Index + 1].Parameter);

	GUInt32 i, numSeg;
	GReal step, u, curVariation, tmpVariation;
	GPoint2 p0(SegmentEvaluate(Index, MinParam));
	GPoint2 p1(SegmentEvaluate(Index, MaxParam));
	GPoint2 v;
	GRay2 ray(p0, p1 - p0);

	// normalize ray direction
	ray.Normalize();
	// taken from Graphics Gems 3: "Curve tessellation criteria through sampling"
	numSeg = 2 * (3 + 1);
	step = (MaxParam - MinParam) / (GReal)numSeg;
	u = MinParam;
	curVariation = -1;
	for (i = 0; i < numSeg - 1; i++) {
		u += step;
		v = Evaluate(u);
		tmpVariation = DistanceSquared(v, ray); 
		if (tmpVariation > curVariation)
			curVariation = tmpVariation;
	}
	return curVariation;
}

// set control points
GError GHermiteCurve2D::SetPoints(const GDynArray<GPoint2>& NewPoints,
								  const GReal NewMinValue, const GReal NewMaxValue, const GBool Uniform) {

	GUInt32 i, j = (GUInt32)NewPoints.size();
	GReal step, u, len;
	GHermiteKey tmpkey;
	GInterval<GReal> requestedInterval(NewMinValue, NewMaxValue);

	// a multicurve must be made of almost 2 points/key
	if (j < 2 || requestedInterval.Length() < G_EPSILON)
		return G_INVALID_PARAMETER;

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
		// calculate tangents using Catmull-Rom schema
		CalcCatmullRomTangents(0, j - 1);
	}
	else {
		// first build curve in an uniform manner
		SetPoints(NewPoints, NewMinValue, NewMaxValue, G_TRUE);

		// now rescale keys parameter to be proportional to arcs length; first calculate global length
		len = TotalLength();
		if (len <= G_EPSILON)
			return G_INVALID_PARAMETER;
		// build new keys parameters buffer
		GDynArray<GReal> newParameters(j - 1);
		u = 0;
		for (i = 1; i < j - 1; i++) {
			u += SegmentLength(i - 1, gKeys[i - 1].Parameter, gKeys[i].Parameter, G_EPSILON);
			newParameters[i] = u / len;
		}
		// now we can assign calculated parameters
		for (i = 1; i < j - 1; i++)
			gKeys[i].Parameter = newParameters[i];
		CalcCatmullRomTangents(0, j - 1);
	}
	return (GCurve2D::SetDomain(NewMinValue, NewMaxValue));
}

inline bool HermiteKeyLE(const Amanith::GHermiteKey& k1, const Amanith::GHermiteKey& k2) {

	if (k1.Parameter <= k2.Parameter)
		return G_TRUE;
	return G_FALSE;
}

// sort keys
void GHermiteCurve2D::SortKeys() {

	std::sort(gKeys.begin(), gKeys.end(), HermiteKeyLE);
}

// set keys
GError GHermiteCurve2D::SetKeys(const GDynArray<GHermiteKey>& NewKeys) {

	GUInt32 j = (GUInt32)NewKeys.size();
	if (j < 2)
		return G_INVALID_PARAMETER;

	gKeys = NewKeys;
	SortKeys();

	return GCurve2D::SetDomain(gKeys[0].Parameter, gKeys[j - 1].Parameter);
}


// given a parameter value, it returns the index such as parameter is between keys [index, index+1)
GBool GHermiteCurve2D::ParamToKeyIndex(const GReal Param, GUInt32& KeyIndex) const {

	GDynArray<GHermiteKey>::const_iterator result;
	GHermiteKey tmpKey;

	tmpKey.Parameter = Param;
	result = std::lower_bound(gKeys.begin(), gKeys.end(), tmpKey, HermiteKeyLE);

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

// get max variation (chordal distance) in the range [u0;u1]; here are necessary also
// curve evaluations at the interval ends
GReal GHermiteCurve2D::Variation(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1) const {

	GUInt32 keyIndex, j = (GUInt32)PointsCount(), i;

	if (j < 2)
		return 0;

	GReal result, tmpVariation;
	GInterval<GReal> requestedInterval(u0, u1);

	// just to avoid warnings
	if (p0[0] && p1[0]) {
	}

	// check if requested interval is not empty
	requestedInterval &= Domain();
	if (requestedInterval.IsEmpty())
		return 0;

	// find the start key index
	ParamToKeyIndex(u0, keyIndex);

	// loops over interested segments
	i = keyIndex;
	result = -1;
	while (i < j) {
		if (requestedInterval.End() > gKeys[i + 1].Parameter) {
			if (gKeys[i].Parameter < requestedInterval.Start()) {
				tmpVariation = SegmentVariation(i, requestedInterval.Start(), gKeys[i + 1].Parameter);
				if (tmpVariation > result)
					result = tmpVariation;
			}
			else {
				tmpVariation = SegmentVariation(i, gKeys[i].Parameter, gKeys[i + 1].Parameter);
				if (tmpVariation > result)
					result = tmpVariation;
			}
			// jump to the next segment
			i++;
		}
		// this is the case of the last interested segment, so calculate the remaining curve piece and exit
		else {
			if (gKeys[i].Parameter < requestedInterval.Start()) {
				tmpVariation = SegmentVariation(i, requestedInterval.Start(), requestedInterval.End());
				if (tmpVariation > result)
					result = tmpVariation;
			}
			else {
				tmpVariation = SegmentVariation(i, gKeys[i].Parameter, requestedInterval.End());
				if (tmpVariation > result)
					result = tmpVariation;
			}
			break;
		}
	}
	return result;
}

// cut the curve, giving the 2 new set of control points that represents 2 poly-line curves (with the
// same degree of the original one)
GError GHermiteCurve2D::DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const {

	GUInt32 keyIndex, i;
	GReal dtdu = 0, dtdu1, dtdu2 = 0;
	GBool b;

	GHermiteCurve2D *lCurve = (GHermiteCurve2D *)LeftCurve;
	GHermiteCurve2D *rCurve = (GHermiteCurve2D *)RightCurve;

	b = ParamToKeyIndex(u, keyIndex);
	G_ASSERT(b == G_TRUE);

	if (lCurve) {
		// clear internal structures
		lCurve->Clear();
		// set knots interval
		lCurve->GCurve2D::SetDomain(DomainStart(), u);
		// copy all previous keys
		for (i = 0; i <= keyIndex; i++)
			lCurve->gKeys.push_back(gKeys[i]);
		// include "cut" point if is not shared
		if (GMath::Abs(u - gKeys[keyIndex].Parameter) > G_EPSILON) {
			// find point on curve and its tangent
			GPoint2 p(SegmentEvaluate(keyIndex, u));
			GVector2 tangent(SegmentTangent(keyIndex, G_FIRST_ORDER_DERIVATIVE, u));

			dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
			dtdu1 = u -  gKeys[keyIndex].Parameter;
			lCurve->gKeys[keyIndex].OutTangent *= (dtdu1 / dtdu);

			tangent *= (dtdu1 / dtdu);
			lCurve->gKeys.push_back(GHermiteKey(u, p, tangent, tangent));
		}
	}

	if (rCurve) {
		// clear internal structures
		rCurve->Clear();
		// set knots interval
		rCurve->GCurve2D::SetDomain(u, DomainEnd());
		// include "cut" point if is not shared
		b = G_FALSE;
		if (GMath::Abs(u - gKeys[keyIndex].Parameter) > G_EPSILON) {
			// find point on curve and its tangent
			GPoint2 p(SegmentEvaluate(keyIndex, u));
			GVector2 tangent(SegmentTangent(keyIndex, G_FIRST_ORDER_DERIVATIVE, u));

			dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
			dtdu2 =  gKeys[keyIndex + 1].Parameter - u;
			tangent *= (dtdu2 / dtdu);
			// create a key and push it
			rCurve->gKeys.push_back(GHermiteKey(u, p, tangent, tangent));
			b = G_TRUE;
		}
		else
			keyIndex--;

		GUInt32 j = PointsCount();
		// copy all foregoing keys
		for (i = keyIndex + 1; i < j; i++)
			rCurve->gKeys.push_back(gKeys[i]);

		if (b)
			rCurve->gKeys[1].InTangent *= (dtdu2 / dtdu);

	}
	return G_NO_ERROR;
}

// add a key
GError GHermiteCurve2D::AddKey(const GHermiteKey& NewKey, GUInt32& InsertedIndex, GBool& AlreadyExists) {

	GBool b;
	GUInt32 keyIndex;

	if (PointsCount() < 2)
		return G_INVALID_OPERATION;

	b = ParamToKeyIndex(NewKey.Parameter, keyIndex);
	if (b) {
		// test for first end
		if (GMath::Abs(gKeys[keyIndex].Parameter - NewKey.Parameter) <= G_EPSILON) {
			InsertedIndex = keyIndex;
			AlreadyExists = G_TRUE;
			return SetKey(keyIndex, NewKey.Value, NewKey.InTangent, NewKey.OutTangent);
		}
		else
		// test for last end
		if (GMath::Abs(gKeys[keyIndex + 1].Parameter - NewKey.Parameter) <= G_EPSILON) {
			InsertedIndex = keyIndex + 1;
			AlreadyExists = G_TRUE;
			return SetKey(keyIndex + 1, NewKey.Value, NewKey.InTangent, NewKey.OutTangent);
		}
		// key is inside segment
		else {
			GReal dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
			GReal dtdu1 = NewKey.Parameter -  gKeys[keyIndex].Parameter;
			gKeys[keyIndex].OutTangent *= (dtdu1 / dtdu);

			GReal dtdu2 = gKeys[keyIndex + 1].Parameter - NewKey.Parameter;
			gKeys[keyIndex + 1].InTangent *= (dtdu2 / dtdu);
			// insert key
			GDynArray<GHermiteKey>::iterator it = gKeys.begin();
			it += (keyIndex + 1);
			gKeys.insert(it, NewKey);
			InsertedIndex = keyIndex + 1;
			AlreadyExists = G_FALSE;
			return G_NO_ERROR;
		}
	}
	else {
		// test for first key
		if (GMath::Abs(DomainStart() - NewKey.Parameter) <= G_EPSILON) {
			InsertedIndex = 0;
			AlreadyExists = G_TRUE;
			return SetKey(0, NewKey.Value, NewKey.InTangent, NewKey.OutTangent);
		}
		else
		// test for last key
		if (GMath::Abs(DomainEnd() - NewKey.Parameter) <= G_EPSILON) {
			InsertedIndex = PointsCount() - 1;
			AlreadyExists = G_TRUE;
			return SetKey(InsertedIndex, NewKey.Value, NewKey.InTangent, NewKey.OutTangent);
		}
		// key is outside
		else {
			AlreadyExists = G_FALSE;
			// append key to front
			if (NewKey.Parameter < DomainStart()) {
				InsertedIndex = 0;
				gKeys.insert(gKeys.begin(), NewKey);
				return GCurve2D::SetDomain(NewKey.Parameter, DomainEnd());
			}
			// append key to back
			else {
				gKeys.push_back(NewKey);
				InsertedIndex = PointsCount() - 1;
				return GCurve2D::SetDomain(DomainStart(), NewKey.Parameter);
			}
		}
	}
}

// add a point; if vertex pointer is NULL point must be inserted ON curve
GError GHermiteCurve2D::DoAddPoint(const GReal Parameter, const GPoint2 *NewPoint, GUInt32& Index,
								   GBool& AlreadyExists) {

	// append new point at the beginning
	if (Parameter < DomainStart() - G_EPSILON) {
		if (!NewPoint)
			return G_INVALID_PARAMETER;
		GHermiteKey tmpKey(Parameter, *NewPoint);
		gKeys.insert(gKeys.begin(), tmpKey);
		// build tangent
		CalcCatmullRomTangents(0, 0);
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
			return G_INVALID_PARAMETER;
		GHermiteKey tmpKey(Parameter, *NewPoint);
		gKeys.push_back(tmpKey);
		// build tangent
		CalcCatmullRomTangents(PointsCount() - 1, PointsCount() - 1);
		Index = (GUInt32)PointsCount() - 1;
		AlreadyExists = G_FALSE;
		return G_NO_ERROR;
	}
	else
	if (Parameter >= DomainEnd() - G_EPSILON) {
		if (NewPoint)
			gKeys[PointsCount() - 1].Value = *NewPoint;
		Index = (GUInt32)PointsCount() - 1;
		AlreadyExists = G_TRUE;
		return G_NO_ERROR;
	}
	else {
		GUInt32 keyIndex;
		GBool b;

		b = ParamToKeyIndex(Parameter, keyIndex);
		G_ASSERT(b == G_TRUE);

		if (GMath::Abs(gKeys[keyIndex].Parameter - Parameter) <= G_EPSILON) {
			if (NewPoint)
				gKeys[keyIndex].Value = *NewPoint;
			Index = keyIndex;
			AlreadyExists = G_TRUE;
			return G_NO_ERROR;
		}
		else
		if (GMath::Abs(gKeys[keyIndex + 1].Parameter - Parameter) <= G_EPSILON) {
			if (NewPoint)
				gKeys[keyIndex + 1].Value = *NewPoint;
			Index = keyIndex + 1;
			AlreadyExists = G_TRUE;
			return G_NO_ERROR;
		}
		// in this case point must be added ON the curve
		else {
			GDynArray<GHermiteKey>::iterator it = gKeys.begin();
			if (NewPoint) {
				// patch previous and following keys tangents
				GReal dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
				GReal dtdu1 = Parameter -  gKeys[keyIndex].Parameter;
				gKeys[Index].OutTangent *= (dtdu1 / dtdu);
				GReal dtdu2 = gKeys[keyIndex + 1].Parameter - Parameter;
				gKeys[Index + 1].InTangent *= (dtdu2 / dtdu);
				// build a key with specified value and tangents
				GHermiteKey tmpKey(Parameter, *NewPoint);
				// insert new key
				it += (keyIndex + 1);
				gKeys.insert(it, tmpKey);
				// build tangent
				CalcCatmullRomTangents(keyIndex + 1, keyIndex + 1);
				Index = keyIndex + 1;
			}
			else {
				GReal dtdu1, dtdu2, dtdu;
				// we must know point on curve
				GPoint2 p(SegmentEvaluate(keyIndex, Parameter));
				// we must know tangent vector
				GVector2 tangent(SegmentTangent(keyIndex, G_FIRST_ORDER_DERIVATIVE, Parameter));

				dtdu = gKeys[keyIndex + 1].Parameter - gKeys[keyIndex].Parameter;
				dtdu1 = Parameter -  gKeys[keyIndex].Parameter;
				gKeys[keyIndex].OutTangent *= (dtdu1 / dtdu);

				dtdu2 = gKeys[keyIndex + 1].Parameter - Parameter;
				gKeys[keyIndex + 1].InTangent *= (dtdu2 / dtdu);

				GHermiteKey tmpKey(Parameter, p, tangent * (dtdu1 / dtdu), tangent * (dtdu2 / dtdu));
				it += (keyIndex + 1);
				gKeys.insert(it, tmpKey);
				Index = keyIndex + 1;
			}
			AlreadyExists = G_FALSE;
			return G_NO_ERROR;
		}
	}
	return G_NO_ERROR;
}

// remove a point(key); index is ensured to be valid and we are sure that after removing we'll have
// (at least) a minimal (2-keys made) multi-curve
GError GHermiteCurve2D::DoRemovePoint(const GUInt32 Index) {

	GUInt32 i = (GUInt32)PointsCount();
	GDynArray<GHermiteKey>::iterator it = gKeys.begin();

	// we are sure that before removing we have at least 3 keys
	if (Index == 0) {
	}
	else
	if (Index == i - 1) {
	}
	else {
		GReal dtdu = gKeys[Index + 1].Parameter - gKeys[Index - 1].Parameter;
		GReal dtdu1 = gKeys[Index].Parameter - gKeys[Index - 1].Parameter;
		GReal dtdu2 = gKeys[Index + 1].Parameter - gKeys[Index].Parameter;
		gKeys[Index - 1].OutTangent *= (dtdu / dtdu1);
		gKeys[Index + 1].InTangent *= (dtdu / dtdu2);
	}
	// go tho Index-th position
	it += Index;
	// lets erase key
	gKeys.erase(it);
	return G_NO_ERROR;
}


// set global parameters corresponding to the start point and to the end point
GError GHermiteCurve2D::SetDomain(const GReal NewMinValue, const GReal NewMaxValue) {
	
	GInterval<GReal> newInterval(NewMinValue, NewMaxValue);
	GUInt32 i, j = (GUInt32)PointsCount();
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

// convert the Index-th segment (from key (i) to key (i-1)) into a cubic Bezier form; it
// suppose that Index is valid
void GHermiteCurve2D::SegmentToBezierConversion(const GUInt32 Index, GBezierCurve2D& Result) const {

	G_ASSERT((GInt32)Index < PointsCount() - 1);
	Result.SetPoints(gKeys[Index].Value, (gKeys[Index].OutTangent / 3) + gKeys[Index].Value,
					 gKeys[Index + 1].Value - (gKeys[Index + 1].InTangent / 3), gKeys[Index + 1].Value);
	Result.SetDomain(gKeys[Index].Parameter, gKeys[Index + 1].Parameter);
}

// convert the Index-th segment (from key (i) to key (i-1)) into a cubic Bezier form
GError GHermiteCurve2D::SegmentToBezier(const GUInt32 Index, GBezierCurve2D& Result) {

	if ((GInt32)Index >= PointsCount() - 1)
		return G_OUT_OF_RANGE;

	SegmentToBezierConversion(Index, Result);
	return G_NO_ERROR;
}

// calculate ray intersection of i-th segment (key (i) to key (i+1)); it suppose that
// Index is valid
GBool GHermiteCurve2D::SegmentIntersectRay(const GUInt32 Index, const GRay2& NormalizedRay,
										   GDynArray<GVector2>& Intersections,
										   const GReal Precision, const GInt32 MaxIterations) const {

	G_ASSERT((GInt32)Index < PointsCount() - 1);

	// convert Hermite curve into a Bezier form
	GBezierCurve2D tmpBez;
	tmpBez.SetPoints(gKeys[Index].Value, (gKeys[Index].OutTangent / 3) + gKeys[Index].Value,
					gKeys[Index + 1].Value - (gKeys[Index + 1].InTangent / 3), gKeys[Index + 1].Value);
	tmpBez.SetDomain(gKeys[Index].Parameter, gKeys[Index + 1].Parameter);
	// find intersections
	return tmpBez.IntersectRay(NormalizedRay, Intersections, Precision, MaxIterations);
}

// intersect the curve with a ray, and returns a list of intersections
GBool GHermiteCurve2D::IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
									 const GReal Precision, const GInt32 MaxIterations) const {


	GUInt32 i, j = (GUInt32)PointsCount(), k, w;

	if (j < 2)
		return G_FALSE;

	GReal lastIntersection, tolerance;
	GBool intFound = G_FALSE;
	GDynArray<GVector2> tmpSolutions;
	GVector2 solution;

	tolerance = GMath::Max(G_EPSILON, Precision);
	lastIntersection = -1;
	k = 0;
	for (i = 0; i < j - 1; i++) {
		intFound |= SegmentIntersectRay(i, NormalizedRay, tmpSolutions, tolerance, MaxIterations);
		// check to not push identical solutions
		w = (GUInt32)tmpSolutions.size();
		for (; k < w; k++) {
			if (GMath::Abs(lastIntersection - tmpSolutions[k][0]) > tolerance) {
				Intersections.push_back(tmpSolutions[k]);
				lastIntersection = tmpSolutions[k][0];
			}
		}
	}
	return intFound;
}

// returns the length of the curve between the 2 specified global parameter values
GReal GHermiteCurve2D::Length(const GReal u0, const GReal u1, const GReal MaxError) const {

	GUInt32 keyIndex, j, i;
	GReal result;
	GInterval<GReal> requestedInterval(u0, u1);

	// check if requested interval is not empty
	requestedInterval &= Domain();
	if (requestedInterval.IsEmpty())
		return 0;

	// find the start key index
	ParamToKeyIndex(requestedInterval.Start(), keyIndex);

	// loops over interested segments
	i = keyIndex;
	j = (GUInt32)PointsCount();
	result = 0;
	while (i < j) {
		if (requestedInterval.End() > gKeys[i + 1].Parameter) {
			if (gKeys[i].Parameter < requestedInterval.Start())
				result += SegmentLength(i, requestedInterval.Start(), gKeys[i + 1].Parameter, MaxError);
			else
				result += SegmentLength(i, gKeys[i].Parameter, gKeys[i + 1].Parameter, MaxError);
			// jump to the next segment
			i++;
		}
		// this is the case of the last interested segment, so calculate the remaining curve piece and exit
		else {
			if (gKeys[i].Parameter < requestedInterval.Start())
				result += SegmentLength(i, requestedInterval.Start(), requestedInterval.End(), MaxError);
			else
				result += SegmentLength(i, gKeys[i].Parameter, requestedInterval.End(), MaxError);
			break;
		}
	}
	return result;
}

// flats (tessellates) i-th segment (key (i) to key (i+1)) specifying a max
// error/variation (chordal distance); it suppose that Index is valid
GError GHermiteCurve2D::SegmentFlatten(const GUInt32 Index, GDynArray<GPoint2>& Contour,
									   const GReal MaxDeviation, const GBool IncludeLastPoint) const {

   G_ASSERT((GInt32)Index < PointsCount() - 1);
   GBezierCurve2D tmpBez;
   GError err;

   SegmentToBezierConversion(Index, tmpBez);
   err = tmpBez.Flatten(Contour, MaxDeviation, IncludeLastPoint);
   return err;
}

// flats (tessellates) the curve specifying a max error/variation (chordal distance)
GError GHermiteCurve2D::Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
								const GBool IncludeLastPoint) const {
	
	GInt32 i, j = PointsCount();
	GError err;

	if (j < 2)
		return G_INVALID_OPERATION;

	if (MaxDeviation <= 0)
		return G_INVALID_PARAMETER;

	for (i = 0; i < j - 2; i++) {
		err = SegmentFlatten(i, Contour, MaxDeviation, G_TRUE);
		if (err != G_NO_ERROR)
			return err;
	}
	err = SegmentFlatten(j - 2, Contour, MaxDeviation, IncludeLastPoint);
	return err;
}

// return the curve value calculated at global parameter u
GPoint2 GHermiteCurve2D::Evaluate(const GReal u) const {

	GUInt32 keyIndex;
	GBool b;

	if (PointsCount() < 2)
		return GPoint2(G_MIN_REAL, G_MIN_REAL);

	// check for global parameter out of range
	if (u <= DomainStart())
		return StartPoint();
	else
	if (u >= DomainEnd())
		return EndPoint();

	b = ParamToKeyIndex(u, keyIndex);
	G_ASSERT (b == G_TRUE);
	return SegmentEvaluate(keyIndex, u);
}

// return the derivate Order-th calculated at global parameter u
GVector2 GHermiteCurve2D::Derivative(const GDerivativeOrder Order, const GReal u) const {

	GUInt32 keyIndex;
	GBool b;
	GReal uu;

	if (PointsCount() < 2)
		return GVector2(0, 0);

	// clamp parameter inside valid interval
	if (u <= DomainStart()) {
		uu = DomainStart();
		keyIndex = 0;
	}
	else
	if (u >= DomainEnd()) {
		uu = DomainEnd();
		keyIndex = PointsCount() - 2;
	}
	else {
		uu = u;
		b = ParamToKeyIndex(uu, keyIndex);
		G_ASSERT (b == G_TRUE);
	}
	return SegmentDerivative(keyIndex, Order, uu);
}


};	// end namespace Amanith
