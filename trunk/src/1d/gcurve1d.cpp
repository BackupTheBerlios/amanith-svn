/****************************************************************************
** $file: amanith/src/1d/gcurve1d.cpp   0.1.0.0   edited Jun 30 08:00
**
** 1D Base curve segment implementation.
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

#include "amanith/1d/gcurve1d.h"
#include "amanith/gerror.h"
#include "amanith/numerics/gintegration.h"

/*!
	\file gcurve1d.cpp
	\brief Implementation file for the base GCurve1D class.
*/

namespace Amanith {


// *********************************************************************
//                             GCurve1D
// *********************************************************************

// constructor
GCurve1D::GCurve1D() : GElement(), gDomain(0, 1) {
}
// constructor
GCurve1D::GCurve1D(const GElement* Owner) : GElement(Owner), gDomain(0, 1) {
}

// destructor
GCurve1D::~GCurve1D() {
}

// get curve tangent (specifying global parameter)
GReal GCurve1D::Tangent(const GReal u) const {

	return Derivative(G_FIRST_ORDER_DERIVATIVE, u);
}

// get curve speed (specifying global parameter)
GReal GCurve1D::Speed(const GReal u) const {

	return Derivative(G_FIRST_ORDER_DERIVATIVE, u);
}

// returns the length of the curve between the 2 specified global parameter values
GReal GCurve1D::Length(const GReal u0, const GReal u1, const GReal MaxError) const {

	GReal uu0, uu1, result;
	GBool errorBounded;

	// be sure of good interval ends
	if (u0 > u1) {
		uu0 = u1;
		uu1 = u0;
	}
	else {
		uu0 = u0;
		uu1 = u1;
	}
	// integration over speed
	errorBounded = GIntegration::Romberg(result, uu0, uu1, SpeedEvaluationCallBack, (void *)this, MaxError);
	return result;
}

// giving CurvePos = Length(t), it solve for t = Inverse(Length(s))
GBool GCurve1D::GlobalParameter(GReal& Result, const GReal CurvePos,
								const GReal MaxError, const GUInt32 MaxIterations) const {

	GReal totalLen, localLen, pivot, error;
	GReal precision = GMath::Max(2 * G_EPSILON, MaxError);
	GUInt32 i;

	// check for out of range parameter
	if (CurvePos <= 0) {
		Result = DomainStart();
		return G_TRUE;
	}
	totalLen = TotalLength();
	if (CurvePos >= totalLen) {
        Result = DomainEnd();
		return G_TRUE;
	}

    // Newton's method for root searching; here we have to find root of function f = Length(u) - CurvePos
	// u is the value we are searching for
	localLen = CurvePos / totalLen;
	// here's a good starting point
    pivot = (1 - localLen) * DomainStart() + localLen * DomainEnd();
    for (i = 0; i < MaxIterations; i++) {
        error = Length(DomainStart(), pivot) - CurvePos;
		// test relative error
		if (GMath::Abs(error / totalLen) <= precision) {
            Result = pivot;
			return G_TRUE;
		}
        pivot -= error / Speed(pivot);
    }
    Result = pivot;
	return G_FALSE;
}

// cut the curve, giving the 2 new left and right curves
GError GCurve1D::Cut(const GReal u, GCurve1D *RightCurve, GCurve1D *LeftCurve) const {

	if (u < DomainStart() || u > DomainEnd())
		return G_OUT_OF_RANGE;

	if (!RightCurve && !LeftCurve)
		return G_NO_ERROR;

	// ensure that output curves are of the same type of source (uncut) curve
	if (RightCurve) {
		if (ClassID() != RightCurve->ClassID())
			/*!
				\todo a possible solution would be to temporary instance 2 curves of the same type,
				then do cutting and if possible convert cut curves into wanted types
			*/
			return G_MISSED_FEATURE;
	}
	// ensure that output curves are of the same type of source (uncut) curve
	if (LeftCurve) {
		if (ClassID() != LeftCurve->ClassID())
			/*!
				\todo a possible solution would be to temporary instance 2 curves of the same type,
				then do cutting and if possible convert cut curves into wanted types.
			*/
			return G_MISSED_FEATURE;
	}
	return DoCut(u, RightCurve, LeftCurve);
}

// cut a slice from the curve
GError GCurve1D::Cut(const GReal u0, const GReal u1, GCurve1D *OutCurve) const {

	if (!OutCurve)
		return G_NO_ERROR;

	GInterval<GReal> requestedInterval(u0, u1);

	requestedInterval &= gDomain;
	if (requestedInterval.IsEmpty())
		return G_INVALID_OPERATION;

	// ensure that output curve is of the same type of source (uncut) curve
	if (ClassID() != OutCurve->ClassID())
		/*!
			\todo a possible solution would be to temporary instance a curve of the same type,
			then do cutting and if possible convert cut curve into wanted type.
		*/
		return G_MISSED_FEATURE;

	GCurve1D *left;
	GError err;

	left = (GCurve1D *)CreateNew(ClassID());
	if (!left)
		return G_UNSUPPORTED_CLASSID;

	err = DoCut(requestedInterval.End(), (GCurve1D *)NULL, left);
	if (err == G_NO_ERROR)
		err = left->DoCut(requestedInterval.Start(), OutCurve, (GCurve1D *)NULL);
	// free temporary curves
	delete left;
	return err;
}

// cut curve by length
GError GCurve1D::CutByLength(const GReal CurvePos, GCurve1D *RightCurve, GCurve1D *LeftCurve, const GReal MaxError) const {

	GReal u;

	if (!LeftCurve && !RightCurve)
		return G_NO_ERROR;

	// first do inverse mapping
	GlobalParameter(u, CurvePos, MaxError);
	// and then cut
	return Cut(u, RightCurve, LeftCurve);
}

// cut curve by length
GError GCurve1D::CutByLength(const GReal CurvePos0, const GReal CurvePos1, GCurve1D *Curve, const GReal MaxError) const {

	GReal u0, u1;

	if (!Curve)
		return G_NO_ERROR;

	// first do inverse mapping
	GlobalParameter(u0, CurvePos0, MaxError);
	GlobalParameter(u1, CurvePos1, MaxError);
	// and then cut
	return Cut(u0, u1, Curve);
}

// translate
void GCurve1D::Translate(const GReal Translation) {

	GInt32 i, j = PointsCount();

	for (i = 0; i < j; ++i)
		SetPoint(i, Translation + Point(i));
}

// scale
void GCurve1D::Scale(const GReal Pivot, const GReal ScaleAmount) {

	GInt32 i, j = PointsCount();

	for (i = 0; i < j; ++i)
		SetPoint(i, ((Point(i) - Pivot) * ScaleAmount) + Pivot);
}


// cloning function
GError GCurve1D::BaseClone(const GElement& Source) {

	const GCurve1D& k = (const GCurve1D&)Source;

	// copy parameter range
	gDomain = k.gDomain;
	return GElement::BaseClone(Source);
}

// static speed evaluation callback
GReal GCurve1D::SpeedEvaluationCallBack(const GReal u, void *Data) {

	GCurve1D *c = (GCurve1D *)Data;

	return c->Speed(u);
}

};	// end namespace Amanith
