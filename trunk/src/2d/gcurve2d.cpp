/****************************************************************************
** $file: amanith/src/2d/gcurve2d.cpp   0.1.0.0   edited Jun 30 08:00
**
** 2D Base curve segment implementation.
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

#include "amanith/2d/gcurve2d.h"
#include "amanith/geometry/gdistance.h"
#include "amanith/geometry/gxform.h"
#include "amanith/geometry/gxformconv.h"
#include "amanith/gerror.h"
#include "amanith/numerics/gintegration.h"

/*!
	\file gcurve2d.cpp
	\brief Implementation file for the base GCurve2D class.
*/

namespace Amanith {


// *********************************************************************
//                             GCurve2D
// *********************************************************************

// constructor
GCurve2D::GCurve2D() : GElement(), gDomain(0, 1) {
}
// constructor
GCurve2D::GCurve2D(const GElement* Owner) : GElement(Owner), gDomain(0, 1) {
}

// destructor
GCurve2D::~GCurve2D() {
}

// get curve tangent (specifying global parameter)
GVector2 GCurve2D::Tangent(const GReal u) const {

	GVector2 v = Derivative(G_FIRST_ORDER_DERIVATIVE, u);
	v.Normalize();
	return v;
}

// get curve normal (specifying global parameter)
GVector2 GCurve2D::Normal(const GReal u) const {

	GVector2 t = Tangent(u);
	return GVector2(t[G_Y], -t[G_X]);
}

// get curvature (specifying global parameter)
GReal GCurve2D::Curvature(const GReal u) const {

	// we have to calculate curvature K(u), from differential calculus:
	// v = first derivate vector
	// t = second derivate vector
	// --> k(u) = | v(u) cross t(u) | / |v(u)|^3
	GVector2 v = Derivative(G_FIRST_ORDER_DERIVATIVE, u);
	GReal m = v.LengthSquared();
	if (m > G_EPSILON) {
		GVector2 t = Derivative(G_SECOND_ORDER_DERIVATIVE, u);
		return ((v[G_X] * t[G_Y]) - (v[G_X] * t[G_Y])) * (GMath::Pow(m, (GReal)-1.5));
	}
	else
		// degenerated case, we can't calculate curvature
		return 0;
}

// get curve speed (specifying global parameter)
GReal GCurve2D::Speed(const GReal u) const {

	GVector2 t = Derivative(G_FIRST_ORDER_DERIVATIVE, u);
	return t.Length();
}

// get max variation (chordal distance) in the (global parameter) range [u0;u1]
GReal GCurve2D::Variation(const GReal u0, const GReal u1) const {

	GInterval<GReal> requestedInterval(u0, u1);
	requestedInterval &= gDomain;

	if (requestedInterval.IsEmpty())
		return 0;

	GPoint2 p0 = Evaluate(u0);
	GPoint2 p1 = Evaluate(u1);
	return Variation(u0, u1, p0, p1);
}

// flats (tessellates) the curve specifying a max error/variation (chordal distance)
GError GCurve2D::Flatten(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1,
						GDynArray<GPoint2>& Contour, const GReal MaxDeviation) const {

	GReal uPivot, tmpVar;
	GPoint2 vPivot;

	// calculate current variation
	tmpVar = Variation(u0, u1, p0, p1);
	// if it's too big lets split the cure for flattening
	if ((tmpVar > MaxDeviation) && (GMath::Abs(u1 - u0) > 2 * G_EPSILON)) {
		// pivot point
		uPivot = (u0 + u1) * (GReal)0.5;
		vPivot = Evaluate(uPivot);
		// flat left part
		Flatten(u0, uPivot, p0, vPivot, Contour, MaxDeviation);
		// flat right part
		Flatten(uPivot, u1, vPivot, p1, Contour, MaxDeviation);
	}
	// in this case we can push a "good" point
	else
		Contour.push_back(p0);
	return G_NO_ERROR;
}

// flats (tessellates) the curve specifying a max error/variation (chordal distance)
GError GCurve2D::Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
						 const GBool IncludeLastPoint) const {

	GError err;

	if (MaxDeviation <= 0)
		return G_INVALID_PARAMETER;

	GPoint2 p0 = Evaluate(DomainStart());
	GPoint2 p1 = Evaluate(DomainEnd());

	err = Flatten(DomainStart(), DomainEnd(), p0, p1, Contour, MaxDeviation);
	if ((err == G_NO_ERROR) && (IncludeLastPoint))
		Contour.push_back(p1);
	return err;
}

// returns the length of the curve between the 2 specified global parameter values
GReal GCurve2D::Length(const GReal u0, const GReal u1, const GReal MaxError) const {

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
GBool GCurve2D::GlobalParameter(GReal& Result, const GReal CurvePos,
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
GError GCurve2D::Cut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const {

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
GError GCurve2D::Cut(const GReal u0, const GReal u1, GCurve2D *OutCurve) const {

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

	GCurve2D *left;
	GError err;

	left = (GCurve2D *)CreateNew(ClassID());
	if (!left)
		return G_UNSUPPORTED_CLASSID;

	err = DoCut(requestedInterval.End(), (GCurve2D *)NULL, left);
	if (err == G_NO_ERROR)
		err = left->DoCut(requestedInterval.Start(), OutCurve, (GCurve2D *)NULL);
	// free temporary curves
	delete left;
	return err;
}

// cut curve by length
GError GCurve2D::CutByLength(const GReal CurvePos, GCurve2D *RightCurve, GCurve2D *LeftCurve, const GReal MaxError) const {

	GReal u;

	if (!LeftCurve && !RightCurve)
		return G_NO_ERROR;

	// first do inverse mapping
	GlobalParameter(u, CurvePos, MaxError);
	// and then cut
	return Cut(u, RightCurve, LeftCurve);
}

// cut curve by length
GError GCurve2D::CutByLength(const GReal CurvePos0, const GReal CurvePos1, GCurve2D *Curve, const GReal MaxError) const {

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
void GCurve2D::Translate(const GVector2& Translation) {

	GMatrix23 matrix;

	TranslationToMatrix(matrix, Translation);
	XForm(matrix);
}

// rotate
void GCurve2D::Rotate(const GPoint2& Pivot, const GReal RadAmount) {

	GMatrix33 rot, preTrans, postTrans, matrix;

	TranslationToMatrix(preTrans, -Pivot);
	TranslationToMatrix(postTrans, (const GVector2&)Pivot);
	RotationToMatrix(rot, RadAmount);
	matrix = postTrans * (rot * preTrans);
	XForm(matrix, G_FALSE);
}

// scale
void GCurve2D::Scale(const GPoint2& Pivot, const GReal XScaleAmount, const GReal YScaleAmount) {

	GMatrix33 scale, preTrans, postTrans, matrix;

	TranslationToMatrix(preTrans, -Pivot);
	TranslationToMatrix(postTrans, (const GVector2&)Pivot);
	ScaleToMatrix(scale, GVect<GReal, 2>(XScaleAmount, YScaleAmount));
	matrix = postTrans * (scale * preTrans);
	XForm(matrix, G_FALSE);
}

// transform
void GCurve2D::XForm(const GMatrix23& Matrix) {

	GUInt32 i, j = PointsCount();
	GPoint2 p;

	for (i = 0; i < j; i++) {
		p = Matrix * Point(i);
		SetPoint(i, p);
	}
}

// transform with projection option
void GCurve2D::XForm(const GMatrix33& Matrix, const GBool DoProjection) {

	GUInt32 i, j = PointsCount();
	GPoint2 p;

	if (DoProjection == G_FALSE) {
		for (i = 0; i < j; i++) {
			p = Matrix * Point(i);
			SetPoint(i, p);
		}
	}
	else {
		GPoint3 q(0, 0, 1), w;
		for (i = 0; i < j; i++) {
			p = Point(i);
			q.Set(p[G_X], p[G_Y]);
			w = Matrix * q;
			if (GMath::Abs(w[G_Z]) <= 2 * G_EPSILON)
				p.Set(w[G_X], w[G_Y]);
			else
				p.Set(w[G_X] / w[G_Z], w[G_Y] / w[G_Z]);
			SetPoint(i, p);
		}
	}
}

// cloning function
GError GCurve2D::BaseClone(const GElement& Source) {

	const GCurve2D& k = (const GCurve2D&)Source;

	// copy parameter range
	gDomain = k.gDomain;
	return GElement::BaseClone(Source);
}

// static speed evaluation callback
GReal GCurve2D::SpeedEvaluationCallBack(const GReal u, void *Data) {

	GCurve2D *c;

	c = (GCurve2D *)Data;
	return c->Speed(u);
}

};	// end namespace Amanith
