/****************************************************************************
** $file: amanith/src/2d/gbsplinecurve2d.cpp   0.3.0.0   edited Jan, 30 2006
**
** 2D B-Spline curve segment implementation.
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

#include "amanith/2d/gbsplinecurve2d.h"
#include "amanith/2d/gbeziercurve2d.h"
#include "amanith/geometry/gintersect.h"
#include "amanith/geometry/gxformconv.h"
#include "amanith/geometry/gxform.h"
#include "amanith/gerror.h"

/*!
	\file gbsplinecurve2d.cpp
	\brief Implementation of 2D BSpline curve class.
*/
namespace Amanith {

// *********************************************************************
//                          GBSplineCurve2D
// *********************************************************************

// constructor
GBSplineCurve2D::GBSplineCurve2D() : GCurve2D() {

	gDegree = 0;
	gOpened = G_TRUE;
	gUniform = G_TRUE;
	gModified = G_FALSE;
}

// constructor
GBSplineCurve2D::GBSplineCurve2D(const GElement* Owner) : GCurve2D(Owner) {

	gDegree = 0;
	gOpened = G_TRUE;
	gUniform = G_TRUE;
	gModified = G_FALSE;
}

// destructor
GBSplineCurve2D::~GBSplineCurve2D() {
}

// clear the curve (remove control points and set an empty knots interval)
void GBSplineCurve2D::Clear() {

	gPoints.clear();
	gDegree = 0;
	gKnots.clear();
	gOpened = G_TRUE;
	gUniform = G_TRUE;
	gModified = G_FALSE;
	gForwDiff1.clear();
	gForwDiff2.clear();
	gBasisFuncEval.clear();
	GCurve2D::Clear();
}

// get number of control points
GUInt32 GBSplineCurve2D::PointsCount() const {

	return (GUInt32)gPoints.size();
}

// get curve degree
GInt32 GBSplineCurve2D::Degree() const {

	return gDegree;
}

// get Index-th point
GPoint2 GBSplineCurve2D::Point(const GUInt32 Index) const {

	if (Index >= PointsCount())
		return GPoint2(G_MIN_REAL, G_MIN_REAL);
	return gPoints[Index];
}

// set Index-th point
GError GBSplineCurve2D::SetPoint(const GUInt32 Index, const GPoint2& NewPoint) {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;
	// copy new point
	gPoints[Index] = NewPoint;
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set control points
GError GBSplineCurve2D::SetPoints(const GDynArray<GPoint2>& NewPoints, const GInt32 Degree,
								  const GBool Opened, const GBool Uniform) {


	GInt32 n = (GInt32)NewPoints.size();

	if ((n < 2) || (Degree <= 0) || (Degree > n - 1))
		return G_INVALID_PARAMETER;
	// copy new points
	gPoints = NewPoints;
	// set degree (must be less or equal to number of points)
	gDegree = Degree;
	// set opened flag
	gOpened = Opened;
	// set uniform flag
	gUniform = Uniform;
	// resize support arrays for basis function evaluation:
	// - first row = left (and ders)
	// - second row = right
	// first and second row are used also for a (for derivatives calculation)
	// - fifth to (3+degree+1)-th for ndu
	gBasisFuncEval.resize((gDegree + 1 + 3) * (gDegree + 1));
	// build knots array
	BuildKnots(gKnots, gPoints, DomainStart(), DomainEnd(), gDegree, gOpened, gUniform);
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set control points specifying also knots range
GError GBSplineCurve2D::SetPoints(const GDynArray<GPoint2>& NewPoints, const GInt32 Degree,
								  const GReal NewMinValue, const GReal NewMaxValue,
								  const GBool Opened, const GBool Uniform) {

	GError err;

	err = GCurve2D::SetDomain(NewMinValue, NewMaxValue);
	if (err == G_NO_ERROR)
		err = SetPoints(NewPoints, Degree, Opened, Uniform);
	return err;
}

// set control points
GError GBSplineCurve2D::SetPoints(const GDynArray<GPoint2>& NewPoints, const GDynArray<GReal>& Knots,
								  const GInt32 Degree, const GBool Uniform) {

	GInt32 n = (GInt32)NewPoints.size();

	if ((n < 2) || (Degree <= 0) || (Degree > n - 1))
		return G_INVALID_PARAMETER;
	// copy new points
	gPoints = NewPoints;
	// copy knots
	gKnots = Knots;
	// set degree (must be less or equal to number of points)
	gDegree = Degree;
	// set uniform flag
	gUniform = Uniform;
	// resize support arrays for basis function evaluation:
	// - first row = left (and ders)
	// - second row = right
	// first and second row are used also for a (for derivatives calculation)
	// - fifth to (5+degree+1)-th for ndu
	gBasisFuncEval.resize((gDegree + 1 + 3) * (gDegree + 1));
	gModified = G_TRUE;
	// set opened flag
	GCurve2D::SetDomain(Knots[0], Knots[Knots.size() -1]);
	gOpened = IsClamped();
	return G_NO_ERROR;
}

// return true if B-spline is clamped (first and last knots have a multiplicity greater or equal to
// the degree)
GBool GBSplineCurve2D::IsClamped() const {

	GInt32 mult;

	mult = Multiplicity(DomainStart());
	if (mult < Degree() + 1)
		return G_FALSE;

	mult = Multiplicity(DomainEnd());
	if (mult < Degree() + 1)
		return G_FALSE;

	return G_TRUE;
}

// returns numer of intersection between control polygon and a ray
GInt32 GBSplineCurve2D::CrossingCount(const GRay2& Ray) const {

	GBool intersected;
	GInt32 i, k;
	GUInt32 intersFlags;
	GReal localIntersParams[2];
	GLineSegment2 seg;

	k = 0;
	for (i = 0; i < gDegree; i++) {
		seg.SetStartPoint(gPoints[i]);
		seg.SetEndPoint(gPoints[i + 1]);
		intersected = Intersect(Ray, seg, intersFlags, localIntersParams);
		if (intersected)
			k++;
	}
	return k;
}

// returns numer of intersection between control polygon and y axis
GInt32 GBSplineCurve2D::CrossingCountX() const {

	GInt32 i, j, numCross = 0;
	GInt32 sign, old_sign;

	sign = old_sign = GMath::Sign(gPoints[0][G_Y]);
	j = (GInt32)PointsCount() - 1;
	for (i = 1; i <= j; i++) {
		sign = GMath::Sign(gPoints[i][G_Y]);
		if (sign != old_sign)
			numCross++;
		old_sign = sign;
	}
	return numCross;
}

// ray intersection based on Newton schema
GBool GBSplineCurve2D::IntersectXRay(GDynArray<GVector2>& Intersections, const GReal Precision,
									 const GUInt32 MaxIterations) const {

	GInt32 crossCount;
	GReal u;
	GBool intLeft, intRight;
	GVector2 intInfo;

	// exploits variation-diminishing property
	crossCount = CrossingCountX();
	if (crossCount <= 0)
		return G_FALSE;
	else
	if (crossCount == 1) {
		// in this case, we are sure that sign(StartPoint.y) != sign(EndPoint.y), so we can use
		// Brent method for 0-searching
		GReal relerr;
		GReal a, b, c, d = 0, e = 0, xm, p, q, r, s, tol1;
		GPoint2 fa, fb, fc;
		GUInt32 i;

		relerr = GMath::Max(G_EPSILON, GMath::Abs(Precision));
		a = DomainStart();
		c = b = DomainEnd();
		fa = StartPoint();
		fc = fb = EndPoint();

		for (i = 0; i < MaxIterations; i++) {
			// no inclusion of a root
			if (fb[G_Y] * (fc[G_Y] / GMath::Abs(fc[G_Y])) > 0) {
				c  = a;				// between b and c ?        
				fc = fa;			// alter c so that b and c  
				e  = d = b - a;		// include the root of f    
			}
			// if fc has the smaller modulus interchange interval end points
			if (GMath::Abs(fc[G_Y]) < GMath::Abs(fb[G_Y])) {        
				a = b;
				b = c;
				c = a;
				fa = fb;
				fb = fc;
				fc = fa;
			}
  			tol1 = (GReal)0.5 * relerr * GMath::Abs(b);
			xm = (GReal)0.5 * (c - b);
			// reached desired accuracy
			if ((fb[G_Y] == 0) || (GMath::Abs(xm) <= tol1)) {
				// test if solution is opposite to ray
				if (fb[G_X] < 0)
					return G_FALSE;
				// curve parameter
				intInfo[G_X] = b;
				// ray distance from its origin
				intInfo[G_Y] = fb[G_X];
				// save found solution
				Intersections.push_back(intInfo);
				return G_TRUE;
			}
			r = 0;
			if (GMath::Abs(e) < tol1 || GMath::Abs(fa[G_Y]) <= GMath::Abs(fb[G_Y]))
				e = d = xm;
			else {
				if (a != c) {						// if a is not equal to c
					q =  fa[G_Y] / fc[G_Y];			// with a, b and c we have 3 points for  
					r = fb[G_Y] / fc[G_Y];			// an inverse quadratic interpolation    
					s = fb[G_Y] / fa[G_Y];
					p = s * (2 * xm * q * (q - r) - (b - a) * (r - 1));
					q = (q - 1) * (r - 1) * (s - 1);
				}
				else {
					// use the secant method or linear
					s = fb[G_Y] / fa[G_Y];
					// interpolation
					p = 2 * xm * s;
					q = 1 - s;
				}
				// alter the sign of p/q for the subsequent division
				if (p > 0)
					q = -q;
				else
					p = -p;
				if ((2 * p  >= 3 * xm * q - GMath::Abs(tol1 * q)) || (p >=  GMath::Abs((GReal)0.5 * e * q)))
					e = d = xm;
				else {
					// compute the quotient p/q for both iterations which will be used to modify b 
					e = d;
					d = p / q;
				}
			}
			// store the best approximation b and its function value fb in a and fa
			a  = b;
			fa = fb;
			if (GMath::Abs(d) > tol1)
				b += d;
			else {
				if (xm >= 0)
					b += tol1;
				else
					b -= tol1;
			}
			fb = Evaluate(b);
		}
	}

	GBSplineCurve2D leftCurve, rightCurve;
	// pivot point
	u = (DomainStart() + DomainEnd()) * (GReal)0.5;
	Cut(u, &rightCurve, &leftCurve);
	intLeft = leftCurve.IntersectXRay(Intersections, Precision, MaxIterations);
	intRight = rightCurve.IntersectXRay(Intersections, Precision, MaxIterations);
	return (intLeft | intRight);
}

// returns control polygon length
GReal GBSplineCurve2D::ControlPolygonLength(const GUInt32 FromIndex, const GUInt32 ToIndex) const {

	GInterval<GInt32> requestedInterval((GInt32)FromIndex, (GInt32)ToIndex);
	GInterval<GInt32> permittedInterval(0, (GInt32)PointsCount() - 1);

	// just to be sure if some value is out of range
	requestedInterval &= permittedInterval;
	GReal l = 0;
	for (GInt32 i = requestedInterval.Start(); i < requestedInterval.End(); i++)
		l += Distance(gPoints[i + 1],  gPoints[i]);
	return l;
}

// intersect the curve with a ray, and returns a list of intersections
GBool GBSplineCurve2D::IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
									const GReal Precision, const GUInt32 MaxIterations) const {

	if (PointsCount() <= 1)
		return G_FALSE;

	GMatrix33 rayTrans, rayRot, rayMatrix;
	GBSplineCurve2D tmpCurve = (*this);
	GUInt32 i, j;
	GBool b;
	
	// build a matrix that makes ray to become X axis, at origin = (0, 0)
	rayRot[G_X][G_X] = NormalizedRay.Direction()[G_X];
	rayRot[G_X][G_Y] = NormalizedRay.Direction()[G_Y];
	rayRot[G_Y][G_X] = -NormalizedRay.Direction()[G_Y];
	rayRot[G_Y][G_Y] = NormalizedRay.Direction()[G_X];
	TranslationToMatrix(rayTrans, -NormalizedRay.Origin());
	rayMatrix = rayRot * rayTrans;

	// transform each control point
	j = PointsCount();
	for (i = 0; i < j; i++)
		tmpCurve.gPoints[i] = rayMatrix * gPoints[i];

	if (MaxIterations == 0)
		b = tmpCurve.IntersectXRay(Intersections, Precision, 1);
	else
		b = tmpCurve.IntersectXRay(Intersections, Precision, MaxIterations);
	return b;
}

// knot insertion
GError GBSplineCurve2D::InsertKnot(const GReal u) {

	if ((u < DomainStart()) || (u > DomainEnd()))
		return G_INVALID_PARAMETER;

	GInt32 i, span, s, pc = (GInt32)PointsCount(), kc = KnotsCount();
	GDynArray<GPoint2> newPoints(pc + 1);
	GDynArray<GReal> newKnots(kc + 1);
	GReal c, den;
	
	// now we have to insert knot u; first we have to find knot span and its multiplicity
	span = FindSpanMult(u, s);
	// we cannot insert knot, because it has got the maximum multiplicity already
	if (s >= gDegree)
		return G_NO_ERROR;
	// build new knots: new knots are the old ones plus the one inserted
	for (i = 0; i <= span; i++)
		newKnots[i] = gKnots[i];
	newKnots[i] = u;
	for (i = span + 1; i < kc; i++)
		newKnots[i + 1] = gKnots[i];
	// copy unaltered control points
	for (i = 0; i <= span - gDegree; i++)
		newPoints[i] = gPoints[i];
	for (i = span - s; i <= pc - 1; i++)
		newPoints[i + 1] = gPoints[i];
	// calculated new control points
	for (i = span - gDegree + 1; i <= span - s; i++) {
		den = gKnots[i + gDegree] - gKnots[i];
		c = (u - gKnots[i]) / den;
		newPoints[i] = (1 - c) * gPoints[i - 1] + c * gPoints[i];
	}
	gKnots = newKnots;
	gPoints = newPoints;
	gModified = G_TRUE;
	// the spline, if was uniform, remains uniform only if we have inserted an existing knot value
	if (s < 1)
		gUniform = G_FALSE;
	// check if we have to change gOpened flag
	gOpened = IsClamped();
	return G_NO_ERROR;
}

// knot averaging
GError GBSplineCurve2D::KnotsAveraging(GDynArray<GReal>& OutKnots, const GDynArray<GReal>& ChordalKnots,
									   const GInt32 Degree, const GInt32 Offset0, const GInt32 HowManyAveraged) {

	GInt32 i, j, n, m;
	GReal u, oneOverDegree;
	
	n = (GInt32)ChordalKnots.size() - 1;
	m = 2 * (Degree + 1) + HowManyAveraged - 1;
	OutKnots.resize(m + 1);
	for (i = 0; i <= Degree; i++)
		OutKnots[i] = ChordalKnots[0];
	oneOverDegree = (GReal)1 / (GReal)Degree;
	for (i = 0; i < HowManyAveraged; i++) {
		u = 0;
		for (j = 0; j < Degree; j++)
			u += ChordalKnots[Offset0 + j + i];
		u *= oneOverDegree;
		OutKnots[i + Degree + 1] = u;
	}
	for (i = m - Degree; i <= m; i++)
		OutKnots[i] = ChordalKnots[n];
	return G_NO_ERROR;
}

// build a chord-length parametrized knots array
void GBSplineCurve2D::ChordLengthKnots(GDynArray<GReal>& OutKnots, const GDynArray<GPoint2>& Points,
									   const GReal MinKnotValue, const GReal MaxKnotValue) {

	GReal c, d, u, delta;
	GInt32 i, n;

	n = (GInt32)Points.size() - 1;
	d = 0;
	// calculate total length
	for (i = 1; i <= n; i++)
		d += GMath::Sqrt(Distance(Points[i], Points[i - 1]));

	OutKnots.resize(n + 1);
	OutKnots[0] = MinKnotValue;
	delta = (MaxKnotValue - MinKnotValue) / d;
	for (i = 1; i <= n - 1; i++) {
		c = GMath::Sqrt(Distance(Points[i], Points[i - 1]));
		u = OutKnots[i - 1] + (c * delta);
		OutKnots[i] = u;
	}
	OutKnots[n] = MaxKnotValue;
}

// build a knot array (clamped and non-uniform) with a centripetal schema and knot averaging
void GBSplineCurve2D::BuildCentripetalKnots(GDynArray<GReal>& OutKnots,
											const GDynArray<GPoint2>& ControlPoints,
											const GReal MinValue, const GReal MaxValue,
											const GInt32 Degree) {

	GDynArray<GReal> uk;
	GInt32 n;

	ChordLengthKnots(uk, ControlPoints, MinValue, MaxValue);
	n = (GInt32)ControlPoints.size() - 1;
	KnotsAveraging(OutKnots, uk, Degree, 1, n - Degree);
}

// build a uniform knots array, open(clamped) or not
void GBSplineCurve2D::BuildUniformKnots(GDynArray<GReal>& OutKnots,
										const GReal MinValue, const GReal MaxValue,
										const GInt32 Degree, const GInt32 NumControlPoints,
										const GBool Clamped) {

	GInt32 i, n, m;
	GReal u, step;

	// this is the fundamental equality m = n + p + 1, where m + 1 = number of knots
	n = NumControlPoints - 1;
	m = n + Degree + 1;
	OutKnots.resize(m + 1);
	if (Clamped) {
		// for clamped spline, first knot must have a Degree + 1 multiplicity
		for (i = 0; i < Degree + 1; i++)
			OutKnots[i] = MinValue;
		// calculate internal knots
		step = (MaxValue - MinValue) / (GReal)(NumControlPoints - Degree);
		u = MinValue;
		for (i = 1; i <= n - Degree; i++) {
			u += step;
			OutKnots[i + Degree] = u;
		}
		// for clamped spline, last knot must have a Degree + 1 multiplicity
		for (i = m - Degree; i <= m; i++)
			OutKnots[i] = MaxValue;
	}
	// in this case we want to preserve b-spline domain to be [MinValue; MaxValue] so external
	// nodes are created outside this interval
	else {
		step = (MaxValue - MinValue) / (GReal)(NumControlPoints - Degree);
		u = MinValue;
		for (i = Degree; i >= 0; i--) {
			OutKnots[i] = u;
			u -= step;
		}
		u = MinValue;
		for (i = Degree + 1; i <= m - Degree - 1; i++) {
			u += step;
			OutKnots[i] = u;
		}
		u = MaxValue;
		for (i = m - Degree; i <= m; i++) {
			OutKnots[i] = u;
			u += step;
		}
	}
}

// build the knot array
void GBSplineCurve2D::BuildKnots(GDynArray<GReal>& OutKnots,
								 const GDynArray<GPoint2>& ControlPoints,
								 const GReal MinValue, const GReal MaxValue,
								 const GInt32 Degree, const GBool OpenedKnots, const GBool UniformKnots) {

	GInt32 numControlPoints = (GInt32)ControlPoints.size();

	if (UniformKnots)
		BuildUniformKnots(OutKnots, MinValue, MaxValue, Degree, numControlPoints, OpenedKnots);
	else {
		if (OpenedKnots)
			BuildCentripetalKnots(OutKnots, ControlPoints, MinValue, MaxValue, Degree);
		else
			// in this case there isn't a good non-uniform parametric representation, so lets be uniform
			BuildUniformKnots(OutKnots, MinValue, MaxValue, Degree, numControlPoints, G_FALSE);
	}
}

// set global parameters corresponding to the start point and to the end point
GError GBSplineCurve2D::SetDomain(const GReal NewMinValue, const GReal NewMaxValue) {

	#define PRECISION 2 * G_EPSILON
	// new re-parametrization, lets call:
	// a = gMinKnotParam, b = gMaxKnotParam  (current range)
	// u belongs to [a;b]  (current global parameter)
	//
	// c = NewMinValue, d = NewMaxValue  (new range)
	// s belongs to [c;d]  (new global parameter)
	//
	// lets call k = (d - c) / (b - a)
	//
	// f(s) = (1 / k) * s - (c / k) + a
	// u = f(s) is well posed, because:
	// 
	// f(c) = a
	// f(d) = b
	// f'(s) > 0 for all s included in [c;d] (it means that f(s) is strictly increasing)
	//
	// It results that C(f(s)) is a re-parametrization of C(u). C(s) is geometrically the same curve as C(u), but
	// parametrically they are different:
	//
	// - internal point mapping: internal knots becomes s = Inverse(f)(u)
	//                                                   i              i
	// - modifications of end derivatives:
	//   C'(s) = C'(u) * f'(s) = C'(u) * (1 / k)
	//   C''(s) = C'(u) * f''(s) + C''(u) * (f'(s))^2 = C''(u) * (1 / k)^2
	//   Note that only magnitudes change
	GInt32 i0, i1, i, m;
	GReal k, s;
	GInterval<GReal> newInterval(NewMinValue, NewMaxValue);

	// check if new range is empty
	if (newInterval.IsEmpty())
		return G_INVALID_PARAMETER;

	// to avoid numeric instabilities, if the new interval is equal (under machine precision) to the
	// current interval, it's better to do nothing
	if ((GMath::Abs(newInterval.Start() - DomainStart()) <= PRECISION) &&
		(GMath::Abs(newInterval.End() - DomainEnd()) <= PRECISION))
		return G_NO_ERROR;

	m = KnotsCount() - 1;
	// first check if the new interval has the same length of the current one; in this case just translate
	// knots, without doing a rescale
	if (GMath::Abs(newInterval.Length() - Domain().Length()) <= PRECISION) {
		// calculate shifting amount
		k = newInterval.Start() - DomainStart();
		for (i = 0; i <= m; i++)
			gKnots[i] += k;
	}
	// do a full re-parametrization
	else {
		// calculate intervals ratio
		k = newInterval.Length() / Domain().Length();
		// find internal knots indexes
		if (gOpened) {
			i0 = gDegree + 1;
			i1 = m - (gDegree + 1);
		}
		else {
			i0 = Multiplicity(DomainStart());
			i1 = m - Multiplicity(DomainEnd());
		}
		// calculate new internal knots
		for (i = i0; i <= i1; i++) {
			s = (gKnots[i] - DomainStart()) * k + newInterval.Start();
			gKnots[i] = s;
		}
		// set new min value
		for (i = 0; i < i0; i++)
			gKnots[i] = newInterval.Start();
		// set new max value
		for (i = i1 + 1; i <= m; i++)
			gKnots[i] = newInterval.End();
	}
	gModified = G_TRUE;
	// set internal members
	return (GCurve2D::SetDomain(newInterval.Start(), newInterval.End()));
}

// find knot span
GInt32 GBSplineCurve2D::FindSpan(const GReal u) const {

	GInt32 i, j;

	if (gOpened) {
		if (u >= DomainEnd())
			return (GInt32)PointsCount() - 1;
	}
	j = (GInt32)gKnots.size() - 1;
	for (i = 0; i < j; i++)
		if ((u >= gKnots[i]) && (u < gKnots[i + 1]))
			return i;
	return -1;
}

// find at the same time knot span and its multiplicity
GInt32 GBSplineCurve2D::FindSpanMult(const GReal u, GInt32& Multiplicity) const {

	GInt32 i, j, k;

	if (gOpened) {
		if (u >= DomainEnd()) {
			Multiplicity = gDegree + 1;
			return (GInt32)PointsCount() - 1;
		}
	}
	j = (GInt32)gKnots.size() - 1;
	for (i = 0; i < j; i++)

		if ((u >= gKnots[i]) && (u < gKnots[i + 1])) {
			// we have found span, lets find multiplicity
			if (u != gKnots[i]) {
				Multiplicity = 0;
				return i;
			}
			Multiplicity = 1;
			k = i;
			while ((k > 0) && (gKnots[k - 1] == u)) {
				Multiplicity++;
				k--;
			}
			return i;
		}
	return -1;
}

// calculate knot multiplicity
GInt32 GBSplineCurve2D::Multiplicity(const GReal u) const {

	GInt32 i = 0, j = (GInt32)gKnots.size(), res = 0;

	while ((i < j) && (gKnots[i] <= u)) {
		if (gKnots[i] == u)
			res++;
		i++;
	}
	return res;
}

// get knots multiplicities
void GBSplineCurve2D::Multiplicities(GDynArray<GKnotMultiplicity>& Values) const {

	GKnotMultiplicity tmpValue;
	GReal u;
	GInt32 i, j;

	// clear output array
	Values.clear();

	// initialize first element
	j = KnotsCount();
	tmpValue.Value = gKnots[0];
	tmpValue.Multiplicity = 1;
	// loop over knots
	for (i = 1; i < j; i++) {
		u = gKnots[i];
		if (u == tmpValue.Value)
			tmpValue.Multiplicity++;
		else {
			Values.push_back(tmpValue);
			tmpValue.Value = u;
			tmpValue.Multiplicity = 1;
		}
	}
	Values.push_back(tmpValue);
}

GPoint2 GBSplineCurve2D::Evaluate(const GReal u) const {

	GPoint2 tmpPoint;
	GInt32 j, span;
	GReal uu;
	GReal *c;

	if (PointsCount() == 0)
		return G_NULL_POINT2;
	
	// clamp parameter inside valid interval
	if (u < DomainStart())
		uu = DomainStart();
	else
	if (u > DomainEnd())
		uu = DomainEnd();
	else
		uu = u;

	// NB: if u belongs to the interval [u;  u) then the only non null basis functions of degree p are:
	//                                    i   i+1
	// N, ........., N
	//  i-p, p        i, p
	span = FindSpan(uu);
	c = BasisFunctions(span, gDegree, uu);
	for (j = 0; j <= gDegree; j++)
		tmpPoint += c[j] * gPoints[span - gDegree + j];
	return tmpPoint;
}

// evaluate Index-th basic function of specified degree at global parameter value u
GReal GBSplineCurve2D::EvaluateBasisFunc(const GReal u, const GInt32 Index, const GInt32 Degree) const {

	GInt32 i, j, k, s;
	GReal temp, uLeft, uRight, saved;

	// test for multiple(clamped) ends knots
	if ((Index == 0) && (u == DomainStart())) {
		s = Multiplicity(u);
		if (s >= gDegree + 1)
			return 1;
	}
	else
	if ((Index == KnotsCount() - gDegree - 2) && (u == gKnots[KnotsCount() - 1])) {
		s = Multiplicity(u);
		if (s >= gDegree + 1)
			return 1;
	}
	// Basis function N(u) is non-zero on [u  ; u     )
	//                 i,p                  i    i+p+1
	if ((u < gKnots[Index]) || (u >= gKnots[Index + Degree + 1]))
		return 0;

	// first we evaluate all N (u) witch is 1 for u in [u;   u )
	//                        i,0                        i    i+1
	for (i = 0; i < Degree + 1; i++) {
		if ((u >= gKnots[i + Index]) && (u < gKnots[i + 1 + Index]))
			gBasisFuncEval[i] = 1;
		else
			gBasisFuncEval[i] = 0;
	}
	// Ni,0      -- Ni,1 .............Ni,p 
	// Ni+1,0     /................../
	// ............................/
	// Ni+p-1,0  -- Ni+p-1,1  .../
	// Ni+p,0     /
	for (k = 1; k <= gDegree; k++) {
		if (gBasisFuncEval[0] == 0)
			saved = 0;
		else
			saved = ((u - gKnots[Index]) * gBasisFuncEval[0]) / (gKnots[Index + k] - gKnots[Index]);
		for (j = 0; j < gDegree - k + 1; j++) {
			uLeft = gKnots[Index + j + 1];
			uRight = gKnots[Index + j + k + 1];
			if (gBasisFuncEval[j + 1] == 0) {
				gBasisFuncEval[j] = saved;
				saved = 0;
			}
			else {
				temp = gBasisFuncEval[j + 1] / (uRight - uLeft);
				gBasisFuncEval[j] = saved + (uRight - u) * temp;
				saved = (u - uLeft) * temp;
			}
		}
	}
	return gBasisFuncEval[0];
}

// calculate first and second order forward differences
void GBSplineCurve2D::BuildForwDiff() const {

	GInt32 i, j;
	GReal k;

	// calculate first order forward differences
	j = (GInt32)PointsCount() - 1;
	gForwDiff1.resize(j);
	for (i = 0; i < j; i++) {
		k = (gKnots[i + gDegree + 1] - gKnots[i + 1]);
		if (k != 0) {
			k = (GReal)gDegree / k;
			gForwDiff1[i] = k * (gPoints[i + 1] - gPoints[i]);
		}
		else
			gForwDiff1[i] = G_NULL_POINT2;
	}
	// calculate second order forward differences
	j--;
	gForwDiff2.resize(j);
	for (i = 0; i < j; i++) {
		k = (gKnots[i + gDegree + 1] - gKnots[i + 2]);
		if (k != 0) {
			k = (GReal)(gDegree - 1) / k;
			gForwDiff2[i] = k * (gForwDiff1[i + 1] - gForwDiff1[i]);
		}
		else
			gForwDiff2[i] = G_NULL_POINT2;
	}
	gModified = G_FALSE;
}

// evaluate non-vanishing basis functions for a given span index, and global parameter
GReal* GBSplineCurve2D::BasisFunctions(const GInt32 SpanIndex, const GInt32 Degree, const GReal u) const {

	// Degree is always less or equal to gDegree
	GReal *left = &gBasisFuncEval[Degree + 1];
	GReal *right = &gBasisFuncEval[(Degree + 1) * 2];
	GReal saved, temp;

	gBasisFuncEval[0] = 1;
	for (GInt32 j = 1; j <= Degree; j++) {
		left[j] = u - gKnots[SpanIndex + 1 - j];
		right[j] = gKnots[SpanIndex + j] - u;
		saved = 0;
		for (GInt32 r = 0; r < j; r++) {
			temp = gBasisFuncEval[r] / (right[r + 1] + left[j - r]);
			gBasisFuncEval[r] = saved + right[r + 1] * temp;
			saved = left[j - r] * temp;
		}
		gBasisFuncEval[j] = saved;
	}
	return &gBasisFuncEval[0];
}

// evaluate non-vanishing basis functions derivatives (up to Degree) for a given span index
// and global parameter
GReal* GBSplineCurve2D::BasisFuncDerivatives(const GInt32 Order, const GInt32 SpanIndex,
											 const GInt32 Degree, const GReal u) const {

	GReal *left, *right, *ders, *swap, *a1, *a2, *ndu;
	GReal saved, temp, d;
	GInt32 j, r, k, n, rk, pk, j1, j2, p;
	#define NDU(i, j) ndu[(i) * (gDegree + 1) + (j)]

	// just to be sure with degree
	p = GMath::Min(Degree, gDegree);

	n = (GInt32)PointsCount() - 1;
	left = &gBasisFuncEval[0];
	right = &gBasisFuncEval[p + 1];

	ndu = &gBasisFuncEval[3 * (p + 1)];
	NDU(0, 0) = 1;
	for (j = 1; j <= p; j++) {
		left[j] = u - gKnots[SpanIndex + 1 - j];
		right[j] = gKnots[SpanIndex + j] - u;
		saved = 0;
		for (r = 0; r < j; r++) {
			// lower triangle
			NDU(j, r) = right[r + 1] + left[j - r];
			temp = NDU(r, j - 1) / NDU(j, r);
			// upper triangle
			NDU(r, j) = saved + right[r + 1] * temp;
			saved = left[j - r] * temp;
		}
		NDU(j, j) = saved;
	}

	ders = &gBasisFuncEval[0];
	// load basis functions
	for (j = 0; j <= p; j++)
		ders[j] = NDU(j, p);
	// if we wanna only basis functions evaluation, lets exit
	if (Order <= 0)
		return ders;
	// now computer Order-th derivative
	for (r = 0; r <= p; r++) {
		a1 = &gBasisFuncEval[p + 1];
		a2 = &gBasisFuncEval[2 * (p + 1)];
		a1[0] = 1;
		for (k = 1; k <= Order; k++) {
			d = 0;
			rk = r - k;
			pk = gDegree - k;
			if (r >= k) {
				a2[0] = a1[0] / NDU(pk + 1, rk);
				d = a2[0] * NDU(rk, pk);
			}
			if (rk >= -1)
				j1 = 1;
			else
				j1 = -rk;
			if (r - 1 <= pk)
				j2 = k - 1;
			else
				j2 = p - r;
			for (j = j1; j <= j2; j++) {
				a2[j] = (a1[j] - a1[j - 1]) / NDU(pk + 1, rk + j);
				d += a2[j] * NDU(rk + j, pk);
			}
			if (r <= pk) {
				a2[k] = -a1[k - 1] / NDU(pk + 1, r);
				d += a2[k] * NDU(r, pk);
			}
			ders[r] = d;
			// switch rows
			swap = a1;
			a1 = a2;
			a2 = swap;
		}
	}
	// multiply by the correct factors, lets calculate degree! / (degree - order)!
	temp = (GReal)(p - Order + 1);
	for (k = 0; k < (Order - 1); k++)
		temp = temp * (temp + 1);
	for (j = 0; j <= p; j++)
		ders[j] *= temp;
	return ders;
}


// return the derivate Order-th calculated at global parameter u
GVector2 GBSplineCurve2D::Derivative(const GDerivativeOrder Order, const GReal u) const {

	GPoint2 tmpPoint;
	GInt32 j, span;
	GReal uu, *c;

	if (PointsCount() == 0)
		return G_NULL_POINT2;
	// clamp parameter inside valid interval
	if (u < DomainStart())
		uu = DomainStart();
	else
	if (u > DomainEnd())
		uu = DomainEnd();
	else
		uu = u;

	span = FindSpan(uu);
	// for clamped splines we can use an optimized calculus
	if (gOpened) {
		if (gModified)
			BuildForwDiff();
		c = BasisFunctions(span, gDegree - Order, uu);
		if (Order == G_FIRST_ORDER_DERIVATIVE)
			for (j = 0; j <= gDegree - Order; j++)
				tmpPoint += c[j] * gForwDiff1[span - gDegree + j];
		else
		if (Order == G_SECOND_ORDER_DERIVATIVE)
			for (j = 0; j <= gDegree - Order; j++)
				tmpPoint += c[j] * gForwDiff2[span - gDegree + j];
	}
	else {
		c = BasisFuncDerivatives(Order, span, gDegree, uu);
		for (j = 0; j <= gDegree; j++)
			tmpPoint += c[j] * gPoints[span - gDegree + j];
	}
	return tmpPoint;
}


// decreases by one the degree of the curve
GError GBSplineCurve2D::LowerDegree() {

	// degree elevating is not yet implemented for unclamped b-splines
	if (gOpened == G_FALSE)
		return G_MISSED_FEATURE;

	if (Degree() < 2)
		return G_NO_ERROR;

	GBSplineCurve2D tmpCurve;
	GError err = LowerDegree(tmpCurve);
	if (err == G_NO_ERROR)
		(*this) = tmpCurve;
	return err;
}

// decreases by one the degree of the curve and gives the result out
GError GBSplineCurve2D::LowerDegree(GBSplineCurve2D& OutputCurve) const {

	GInt32 p = gDegree;
	GInt32 i, j, k, b, ph, kind, cind, mult, a, m, r, oldr,
		   s, mh, nh, lbz, save, first, last, kj;
	GReal numer, alfa, beta;
	GInt32 nSigned, mSigned;

	// degree elevating is not yet implemented for unclamped b-splines
	if (gOpened == G_FALSE)
		return G_MISSED_FEATURE;

	if (Degree() < 2)
		return G_NO_ERROR;

	// get knots multiplicities
	GDynArray<GKnotMultiplicity> tmpMult;
	Multiplicities(tmpMult);
	j = (GInt32)tmpMult.size();
	s = j - 2;
	// nSigned = (numer of new control points) - 1
	nSigned = ((GInt32)PointsCount() - 1) - s - 1;
	// mSigned must satisfy mSigned = nSigned + (Degree() - 1) + 1
	mSigned = 0;
	// note that a knot multiplicity can be 1, witch implies that the knot is not present
	// in the new knot vector
	for (i = 0; i < j; i++) {
		k = tmpMult[i].Multiplicity - 1;
		mSigned = mSigned + k;
	}
	// b-spline cannot be degree reduced if equation is not satisfied
	if (nSigned + (Degree() - 1) + 1 != mSigned - 1)
		return G_OUT_OF_RANGE;

	// pth-degree Bezier control points of the current segment
	GDynArray<GPoint2> bpts(p + 1);
	// degree reduced Bezier control points
	GDynArray<GPoint2> rbpts(p);
	// leftmost control points of the next Bezier segment
	GDynArray<GPoint2> Nextbpts(p - 1);
	// new control points
	GDynArray<GPoint2> Pw(nSigned + 1);
	// new knots
	GDynArray<GReal> Uh(mSigned);
	// knot insertion alphas
	GDynArray<GReal> alphas(p - 1);
	// temporary Bezier curve, used to do degree reduction of a Bezier segment
	GBezierCurve2D tmpBezCurve;
	tmpBezCurve.SetDomain(DomainStart(), DomainEnd());

	ph = p - 1;
	mh = ph;
	kind = ph + 1;
	r = -1;
	a = p;
	b = p + 1;
	cind = 1;
	mult = p;
	m = ((GInt32)PointsCount() - 1) + p + 1;
	Pw[0] = gPoints[0];
	// compute left end of knot vector
	for (i = 0; i <= ph; i++)
		Uh[i] = gKnots[0];
	// initialize first Bezier segment
	for (i = 0; i <= p; i++)
		bpts[i] = gPoints[i];

	// loop through the knot vector
	while (b < m) {

		// compute knot multiplicity
		i = b;
		while (b < m && gKnots[b] == gKnots[b + 1])
			b++;
		mult = b - i + 1;
		mh = mh + mult - 1;
		oldr = r;
		r = p - mult;
		if (oldr > 0)
			lbz = (oldr + 2) / 2;
		else
			lbz = 1;
		// insert knot gKnots[b] r times
		if (r > 0) {
			numer = gKnots[b] - gKnots[a];
			for (k = p; k > mult; k--)
				alphas[k - mult - 1] = numer / (gKnots[a + k] - gKnots[a]);

			for (j = 1; j <= r; j++) {
				save = r - j;
				s = mult + j;
				for (k = p; k >= s; k--)
					bpts[k] = alphas[k - s] * bpts[k] + (1 - alphas[k - s]) * bpts[k - 1];
				Nextbpts[save] = bpts[p];
			}
		}
		// degree reduce Bezier segment
		tmpBezCurve.SetPoints(bpts);
		tmpBezCurve.LowerDegree();
		rbpts = tmpBezCurve.Points();

		// remove knot gKnots[a] oldr times
		if (oldr > 0) {
			first = kind;
			last = kind;
			for (k = 0; k < oldr; k++) {
				i = first;
				j = last;
				kj = j - kind;
				while (j - i > k) {
					alfa = (gKnots[a] - Uh[i - 1]) / (gKnots[b] - Uh[i - 1]);
					beta = (gKnots[a] - Uh[j - k - 1]) / (gKnots[b] - Uh[j - k - 1]);
					Pw[i - 1] = (Pw[i - 1] - (1 - alfa) * Pw[i - 2]) / alfa;
					rbpts[kj] = (rbpts[kj] - beta * rbpts[kj + 1]) / (1 - beta);
					i++;
					j--;
					kj--;
				}
				first--;
				last++;
			}
			cind = i - 1;
		}
		// load knot vector and control points
		if (a != p)
			for (i = 0; i < ph - oldr; i++) {
				Uh[kind] = gKnots[a];
				kind++;
			}
		for (i = lbz; i <= ph; i++) {
			Pw[cind] = rbpts[i];
			cind++;
		}
		// set up for next pass through
		if (b < m) {
			for (i = 0; i < r; i++)
				bpts[i] = Nextbpts[i];
			for (i = r; i <= p; i++)
				bpts[i] = gPoints[b - p + i];
			a = b;
			b++;
		}
		else
			for (i = 0; i <= ph; i++)
				Uh[kind + i] = gKnots[b];
	}  // end while (b < m)

	nh = mh - ph - 1;
	OutputCurve.SetPoints(Pw, Uh, gDegree - 1, gUniform);
	return G_NO_ERROR;
}

// increases by one the degree of the curve
GError GBSplineCurve2D::HigherDegree(const GInt32 HowManyTimes) {

	// degree elevating is not yet implemented for unclamped b-splines
	if (gOpened == G_FALSE)
		return G_MISSED_FEATURE;

	GBSplineCurve2D tmpCurve;
	GError err = HigherDegree(HowManyTimes, tmpCurve);
	if (err == G_NO_ERROR)
		(*this) = tmpCurve;
	return err;
}

// increases by one the degree of the curve and gives the result out
GError GBSplineCurve2D::HigherDegree(const GInt32 HowManyTimes, GBSplineCurve2D& OutputCurve) const {

	GInt32 p = gDegree;
	GInt32 mpi, i, j, k, b, ph, ph2, kind, cind, mul, a, m, r, oldr,
		   s, mh, lbz, rbz, save, first, last, tr, kj, t = HowManyTimes;
	GReal inv, ua, ub, numer, den, bet, alf, gam;

	// degree elevating is not yet implemented for unclamped b-splines
	if (gOpened == G_FALSE)
		return G_MISSED_FEATURE;

	// get knots multiplicities
	GDynArray<GKnotMultiplicity> tmpMult;
	Multiplicities(tmpMult);
	s = (GInt32)tmpMult.size() - 2;

	// coefficients for degree elevating the Bezier segments
	GDynArray<GReal> bezalfs((p + t + 1) * (p + 1));
	// pth-degree Bezier control points of the current segment
	GDynArray<GPoint2> bpts(p + 1);
	// (p + HowManyTimes)th-degree Bezier control points of the current segment
	GDynArray<GPoint2> ebpts(p + t + 1);
	// leftmost control points of the next Bezier segment
	GDynArray<GPoint2> Nextbpts(p - 1);
	// new control points
	GDynArray<GPoint2> Qw((GInt32)PointsCount() + s + 1);
	// new knots
	GDynArray<GReal> Uh(KnotsCount() + s + 2);
	// knot insertion alphas
	GDynArray<GReal> alfs(p - 1);
	// useful macro
	#define BEZALFS(i, j) bezalfs[(i) * (p + 1) + (j)]

	m = (GInt32)PointsCount() + p;
	ph = p + t;
	ph2 = ph / 2;
	// compute Bezier degree elevation coefficients
	BEZALFS(0, 0) = BEZALFS(ph, p) = 1;
	for (i = 1; i<= ph2; i++) {
		inv = (GReal)1 / (GReal)GMath::Binomial((GReal)ph, (GReal)i);
		mpi = GMath::Min(p, i);
		for (j = GMath::Max(0, i - t); j <= mpi; j++)
			BEZALFS(i, j) = inv * (GReal)GMath::Binomial((GReal)p, (GReal)j) * (GReal)GMath::Binomial((GReal)t, (GReal)(i - j));
	}

	for (i = ph2 + 1; i <= ph - 1; i++) {
		mpi = GMath::Min(p, i);
		// this exploits the fact that "n over i" is equal to "n over (n-i)"
		for (j = GMath::Max(0, i - t); j <= mpi; j++)
			BEZALFS(i, j) = BEZALFS(ph - i, p - j);
	}

	mh = ph;
	kind = ph + 1;
	r = -1;
	a = p;
	b = p + 1;
	cind = 1;
	ua = gKnots[0];
	Qw[0] = gPoints[0];
	for (i = 0; i <= ph; i++)
		Uh[i] = ua;
	// initialize first Bezier segment
	for (i = 0; i <= p; i++)
		bpts[i] = gPoints[i];
	// loop through knot vector
	while (b < m) {
		i = b;
		// count multiplicity
		while (b < m && gKnots[b] == gKnots[b + 1])
			b++;
		mul = b - i + 1;
		mh = mh + mul + t;
		ub = gKnots[b];
		oldr = r;
		r = p - mul;
		// insert knot u(b) r times
		if (oldr > 0)
			lbz = (oldr + 2) / 2;
		else
			lbz = 1;
		if (r > 0)
			rbz = ph - (r + 1) / 2;
		else
			rbz = ph;
		// insert knot to get Bezier segment
		if (r > 0) {
			numer = ub - ua;
			for (k = p; k > mul; k--)
				alfs[k - mul - 1] = numer / (gKnots[a + k] - ua);

			for (j = 1; j <= r; j++) {
				save = r - j;
				s = mul + j;
				for (k = p; k >= s; k--) {
					bpts[k] = alfs[k - s] * bpts[k] + (1 - alfs[k - s]) * bpts[k - 1];
				}
				Nextbpts[save] = bpts[p];
			}
		} // end insert knot
		
		// degree elevate Bezier
		for (i = lbz; i <= ph; i++) {
			// only points lbz, ..., ph are used below
			ebpts[i].Set(0, 0);
			mpi = GMath::Min(p, i);
			for (j = GMath::Max(0, i - t); j <= mpi; j++)
				ebpts[i] = ebpts[i] + BEZALFS(i, j) * bpts[j];
		} // end degree elevating

		if (oldr > 1) {
			// must remove knot u = gKnots[a] oldr times
			first = kind - 2;
			last = kind;
			den = ub - ua;
			bet = (ub - Uh[kind - 1]) / den;
			// knot removal loop
			for (tr = 1; tr < oldr; tr++) {
				i = first;
				j = last;
				kj = j - kind + 1;
				// loop and compute the new control points for one removal step
				while ((j - i) > tr) {
					if (i < cind) {
						alf = (ub - Uh[i]) / (ua - Uh[i]);
						Qw[i] = alf * Qw[i] + (1 - alf) * Qw[i - 1];
					}
					if (j >= lbz) {
						if ((j - tr) <= (kind - ph + oldr)) {
							gam = (ub - Uh[j - tr]) / den;
							ebpts[kj] = gam * ebpts[kj] + (1 - gam) * ebpts[kj + 1];
						}
						else {
							ebpts[kj] = bet * ebpts[kj] + (1 - bet) * ebpts[kj + 1];
						}
					}
					i++;
					j--;
					kj--;
				}
				first--;
				last++;
			}
		} // end of removing knot u = gKnots[a]

		// load the knot ua
		if (a != p)
			for (i = 0; i < ph - oldr; i++) {
				Uh[kind] = ua;
				kind++;
			}
		// load control points into Qw
		for (j = lbz; j <= rbz; j++) {
			Qw[cind] = ebpts[j];
			cind++;
		}
		// set up for the next pass through loop
		if (b < m) {
			for (j = 0; j < r; j++)
				bpts[j] = Nextbpts[j];
			for (j = r; j <= p; j++)
				bpts[j] = gPoints[b - p + j];
			a = b;
			b++;
			ua = ub;
		}
		else
			// end knot
			for (i = 0; i <= ph; i++)
				Uh[kind + i] = ub;
	} // end while (b < m)

	OutputCurve.SetPoints(Qw, Uh, gDegree + HowManyTimes, gUniform);
	return G_NO_ERROR;
}

// cut the curve, giving the 2 new set of control points that represents 2 Bezier curve (with the
// same degree of the original one)
GError GBSplineCurve2D::DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const {

	GBSplineCurve2D *rCurve = (GBSplineCurve2D *)RightCurve;
	GBSplineCurve2D *lCurve = (GBSplineCurve2D *)LeftCurve;

	if (u == DomainStart()) {
		if (rCurve)
			rCurve->CopyFrom(*this);
		if (lCurve)
			lCurve->Clear();
		return G_NO_ERROR;
	}
	else
	if (u == DomainEnd()) {
		if (lCurve)
			lCurve->CopyFrom(*this);
		if (rCurve)
			rCurve->Clear();
		return G_NO_ERROR;
	}

	GInt32 k, i, j, s, h, r;
	GDynArray<GPoint2> leftPoints, rightPoints;
	GDynArray<GPoint2> deBoor;
	GDynArray<GReal> leftKnots, rightKnots;
	GPoint2 v;
	GReal den, a;

	k = FindSpanMult(u, s);
	h = gDegree - s;

	// left untouched control points
	for (i = 0; i <= k - gDegree; i++)
		leftPoints.push_back(gPoints[i]);
	// right untouched control points	
	j = (GInt32)PointsCount() - 1;
	for (i = j; i >= k - s; i--)
		rightPoints.push_back(gPoints[i]);
	// create first step Deboor polygon
	for (i = k - gDegree; i <= k - s; i++)
		deBoor.push_back(gPoints[i]);
	j = (GInt32)deBoor.size();
	// lets subdivide the curve 
	for (r = 1; r <= h ; r++) {
		for (i = k - s; i >= k - gDegree + r; i--) {
			j = i - (k - gDegree);
			den = gKnots[i + gDegree - r + 1] - gKnots[i];
			// ai, r = (u - ui) / (ui+p-r+1 - ui) 
			a = (u - gKnots[i]) / den;
			// Pi,r = (1 - ai,r) Pi-1,r-1 + ai,r Pi,r-1 
			deBoor[j] = (1 - a) * deBoor[j - 1] + a * deBoor[j];
		}
		// push first vertex into left curve
		j = (k - gDegree + r) - (k - gDegree);
		v = deBoor[j];
		leftPoints.push_back(v);
		// push last vertex into left curve
		j = (k - s) - (k - gDegree);
		v = deBoor[j];
		rightPoints.push_back(v);
	}

	// the knot vector for the left curve includes all knots in [MinKnotParam, u) followed by
	// Degree+1 copies of u
	j = (GInt32)gKnots.size() - 1;
	for (i = 0; i <= j; i++)
		if ((gKnots[i] >= DomainStart()) && (gKnots[i] < u))
			leftKnots.push_back(gKnots[i]);
	for (i = 0; i < gDegree + 1; i++)
		leftKnots.push_back(u);

	// lets mirror "right" points
	ReverseArray(rightPoints, 0, (GUInt32)(rightPoints.size() - 1));

	// the knot vector for the right curve includes Degree+1 copies of u followed by all
	// knots in (u, MinKnotParam] 
	for (i = 0; i < gDegree + 1; i++)
		rightKnots.push_back(u);
	j = (GInt32)gKnots.size() - 1;
	for (i = 0; i <= j; i++)
		if ((gKnots[i] > u) && (gKnots[i] <= DomainEnd()))
			rightKnots.push_back(gKnots[i]);
	// in the general case knots are not uniform after cutting. To be more precise: the new 2 b-splines are
	// uniform if uncut b-spline was uniform and cut has been done at a non-simple knot
	if ((gUniform) && (s > 0)) {
		if (rCurve)
			rCurve->SetPoints(rightPoints, rightKnots, gDegree, G_TRUE);
		if (lCurve)
			lCurve->SetPoints(leftPoints, leftKnots, gDegree, G_TRUE);
	}
	else {
		if (rCurve)
			rCurve->SetPoints(rightPoints, rightKnots, gDegree, G_FALSE);
		if (lCurve)
			lCurve->SetPoints(leftPoints, leftKnots, gDegree, G_FALSE);
	}
	return G_NO_ERROR;
}

GError GBSplineCurve2D::Flatten(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1,
								GDynArray<GPoint2>& Contour, const GReal MaxDeviation) const {

	// calculate current variation
	GReal tmpVar = Variation(u0, u1, p0, p1);
	// if it's too big lets split the curve for flattening
	if ((tmpVar > MaxDeviation) && (GMath::Abs(u1 - u0) > 2 * G_EPSILON)) {
		// pivot point
		GReal uPivot = (u0 + u1) * (GReal)0.5;
		GPoint2 vPivot = Evaluate(uPivot);
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
GError GBSplineCurve2D::Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
								const GBool IncludeLastPoint) const {

	if (MaxDeviation <= 0)
		return G_INVALID_PARAMETER;

	GUInt32 i = PointsCount();
	if (i == 0)
		return G_NO_ERROR;

	GPoint2 p0, p1;
	if (gOpened) {
		p0 = gPoints[0];
		p1 = gPoints[i - 1];
	}
	else {
		p0 = Evaluate(DomainStart());
		p1 = Evaluate(DomainEnd());
	}

	GError err = Flatten(DomainStart(), DomainEnd(), p0, p1, Contour, MaxDeviation);
	if ((err == G_NO_ERROR) && (IncludeLastPoint))
		Contour.push_back(p1);
	return err;
}

// get max variation (chordal distance) in the domain range
GReal GBSplineCurve2D::Variation() const {

	GUInt32 i = (GUInt32)gPoints.size();

	if (i < 2)
		return 0;

	if (gOpened)
		return Variation(DomainStart(), DomainEnd(), gPoints[0], gPoints[i - 1]);
	else {
		GPoint2 p0 = Evaluate(DomainStart());
		GPoint2 p1 = Evaluate(DomainEnd());
		return Variation(DomainStart(), DomainEnd(), p0, p1);
	}
}

// get max variation (chordal distance) in the range [u0;u1]; here are necessary also
// curve evaluations at the interval ends
GReal GBSplineCurve2D::Variation(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1) const {

	GInt32 i, numSeg;
	GReal step, u, curVariation, tmpVariation;
	GPoint2 v;
	GRay2 ray(p0, p1 - p0);

	// normalize ray direction
	ray.Normalize();
	// taken from Graphics Gems 3: "Curve tessellation criteria through sampling"
	numSeg = 2 * (gDegree + 1);
	step = (u1 - u0) / (GReal)numSeg;
	u = u0;
	curVariation = -1.0;
	for (i = 0; i < numSeg - 1; i++) {
		u += step;
		v = Evaluate(u);
		tmpVariation = DistanceSquared(v, ray); 
		if (tmpVariation > curVariation)
			curVariation = tmpVariation;
	}
	return curVariation;
}

// cloning function
GError GBSplineCurve2D::BaseClone(const GElement& Source) {

	const GBSplineCurve2D& k = (const GBSplineCurve2D&)Source;

	// copy control points and knots
	gPoints = k.gPoints;
	gKnots = k.gKnots;
	// copy array for forward differences
	gForwDiff1 = k.gForwDiff1;
	gForwDiff2 = k.gForwDiff2;
	// copy 'modified' flag
	gModified = k.gModified;
	gDegree = k.gDegree;
	gOpened = k.gOpened;
	gUniform = k.gUniform;
	gBasisFuncEval = k.gBasisFuncEval;
	return GCurve2D::BaseClone(Source);
}

// taken from Numerical Recipes in C and adapted to the spline case (optimized)
static void BandEncodec(GDynArray<GReal>& a, const GInt32 n, const GInt32 m1,
						const GInt32 m2, GDynArray<GReal>& al, GDynArray<GInt32>& indx) {

	GInt32 i, j, k, l;
	GInt32 mm;
	GReal dum;
	#define A(i, j) a[(i - 1) * (m1 + m2 + 1) + (j - 1)]
	#define AL(i, j) al[(i - 1) * m1 + (j - 1)]
	#define INDX(i) indx[(i) - 1]
	#define SWAP(a, b) { dum = (a); (a) = (b); (b) = dum; }
	
	mm = m1 + m2 + 1;
	l = m1;
	// rearrange the storage a bit
	for (i = 1; i <= m1; i++) {
		for (j = m1 + 2 - i; j <= mm; j++)
			A(i, j - l) = A(i, j);
		l--;
		for (j = mm - l; j <= mm; j++)
			A(i, j) = 0;
	}
	l = m1;
	for (k = 1; k <= n; k++) {
		// for each row...
		dum = A(k, 1);
		i = k;
		if (l < n)
			l++;
		for (j = k + 1; j <= l; j++) {
			// find the pivot element
			if (GMath::Abs(A(j, 1)) > GMath::Abs(dum)) {
				dum = A(j, 1);
				i = j;
			}
		}
		INDX(k) = i;
		// matrix is algorithmically singular, but proceed anyway with TINY pivot (desirable in
		// some applications)
		if (dum == 0)
			A(k, 1) = G_EPSILON;
		// interchange rows
		if (i != k) {
			for (j = 1; j <= mm; j++)
				SWAP(A(k, j), A(i, j))
		}
		// do the elimination
		for (i = k + 1; i <= l; i++) {
			dum = A(i, 1) / A(k, 1);
			AL(k, i - k) = dum;
			for (j = 2; j <= mm; j++)
				A(i, j - 1) = A(i, j) - dum * A(k, j);
			A(i, mm) = 0;
		}
	}
	#undef A
	#undef AL
	#undef INDX
	#undef SWAP
}

// taken from Numerical Recipes in C and adapted to the spline case (optimized)
static void BandedBackSubstitution(GDynArray<GReal>& a, const GInt32 n, const GInt32 m1, const GInt32 m2,
								   const GDynArray<GReal>& al, const GDynArray<GInt32>& indx,
								   GDynArray<GPoint2>& b) {

	// Given the arrays a, al, and indx as returned from bandec, and given a right-hand side vector
	// b[0..n-1], solves the band diagonal linear equations A · x = b. The solution vector x overwrites
	// b[0..n-1]. The other input arrays are not modified, and can be left in place for successive calls
	// with different right-hand sides
	GInt32 i, k, l;
	GInt32 mm;
	GPoint2 dum;
	#define A(i, j) a[(i) * (m1 + m2 + 1) + (j)]
	#define AL(i, j) al[(i) * m1 + (j)]
	#define SWAP(a, b) { dum = (a); (a) = (b); (b) = dum; }

	mm = m1 + m2 + 1;
	l = m1;
	// forward substitution, unscrambling the permuted rows as we go
	for (k = 0; k <= n - 1; k++) {
		i = indx[k] - 1;
		if (i != k)
			SWAP(b[k], b[i])
		if (l < n)
			l++;
		for (i = k + 1; i <= l - 1; i++)
			b[i] -= (AL(k, i - k - 1) * b[k]);
	}
	l = 1;
	for (i = n - 1; i >= 0; i--) {
		// back substitution
		dum = b[i];
		for (k = 2 - 1; k <= l - 1; k++)
			dum -= (A(i, k) * b[k + i]);
		b[i] = dum / A(i, 0);
		if (l < mm)
			l++;
	}
	#undef A
	#undef AL
	#undef SWAP
}


// resolve banded system for curve fitting
GError GBSplineCurve2D::SolveBandedSystem(const GDynArray<GReal>& BasisMatrix, const GInt32 MatrixSize,
										  const GInt32 LeftSemiBandWidth, const GInt32 RightSemiBandWidth,
										  GDynArray<GPoint2>& Rhs) {

	GInt32 i, j;

	// construct compact representation of basis functions matrix
	GDynArray<GReal> compactMatrix(MatrixSize * (LeftSemiBandWidth + RightSemiBandWidth + 1), 0);
	#define CMATRIX(i, j) compactMatrix[(i)*(LeftSemiBandWidth + RightSemiBandWidth + 1) + (j)]
	#define MATRIX(i, j) BasisMatrix[(i)*MatrixSize + (j)]

	// copy diagonal and superdiagonal elements
	for (i = 0; i <= RightSemiBandWidth; i++)
		for (j = 0; j <= MatrixSize - 1 - i; j++)
			CMATRIX(j, LeftSemiBandWidth + i) = MATRIX(j, i + j);
	// copy subdiagonal
	for (i = 1; i <= LeftSemiBandWidth; i++)
		for (j = 0; j <= MatrixSize - 1 - i; j++)
			CMATRIX(j + i, LeftSemiBandWidth - i) = MATRIX(i + j, j);
	// resolve banded linear system
	GDynArray<GReal> al(MatrixSize * LeftSemiBandWidth);
	GDynArray<GInt32> indx(MatrixSize);

	BandEncodec(compactMatrix, MatrixSize, LeftSemiBandWidth, RightSemiBandWidth, al, indx);
	BandedBackSubstitution(compactMatrix, MatrixSize, LeftSemiBandWidth, RightSemiBandWidth, al, indx, Rhs);

	#undef CMATRIX
	#undef MATRIX
	return G_NO_ERROR;
}

// curve (global) fitting
GError GBSplineCurve2D::GlobalFit(const GInt32 Degree, const GDynArray<GPoint2>& FitPoints,
								  const GReal MinKnotValue, const GReal MaxKnotValue) {

	if ((Degree <= 0) || (Degree >= (GInt32)FitPoints.size()))
		return G_INVALID_PARAMETER;

	GInt32 n = (GInt32)FitPoints.size() - 1, m = n + Degree + 1, i, j, span;
	GDynArray<GReal> matrix((n + 1) * (n + 1), 0);
	GDynArray<GReal> knots(m + 1);
	GDynArray<GReal> uk;
	GReal *basisFuncs;
	#define MATRIX(i, j) matrix[(i)*(n + 1) + (j)]

	// generate knot vector
	ChordLengthKnots(uk, FitPoints, MinKnotValue, MaxKnotValue);
	KnotsAveraging(knots, uk, Degree, 1, n - Degree);
	// set basic structures to the output curve and set knots range
	SetPoints(FitPoints, knots, Degree, G_FALSE);

	// build basis functions matrix
	for (i = 1; i <= n - 1; i++) {
		span = FindSpan(uk[i]);
		basisFuncs = BasisFunctions(span, Degree, uk[i]);
		// build i-th row
		for (j = 0; j <= Degree; j++)
			MATRIX(i, span - Degree + j) = basisFuncs[j];
	}
	// build first and last row
	MATRIX(0, 0) = MATRIX(n, n) = (GReal)1;
	// resolve the system
	return SolveBandedSystem(matrix, n + 1, Degree - 1, Degree - 1, gPoints);
	#undef MATRIX
}

// curve (global) fitting with first derivative specified at end points
GError GBSplineCurve2D::GlobalFit(const GInt32 Degree, const GDynArray<GPoint2>& FitPoints,
								  const GVector2& Derivative0, const GVector2& Derivative1,
								  const GReal MinKnotValue, const GReal MaxKnotValue) {

	GInt32 i, j, span, n = (GInt32)FitPoints.size() - 1, m;

	if ((Degree <= 0) || (Degree > n + 2))
		return G_INVALID_PARAMETER;

	if (Degree == 1)
		m = n + Degree + 1;
	else
		m = n + Degree + 3;

	GDynArray<GReal> matrix((n + 3) * (n + 3), 0);
	GDynArray<GReal> knots(m + 1);
	GDynArray<GReal> uk(n + 1);
	GDynArray<GPoint2> rhs(n + 3);
	GReal *basisFuncs, oneOverDegree;
	#define MATRIX(i, j) matrix[(i)*(n + 3) + (j)]

	// generate knot vector
	ChordLengthKnots(uk, FitPoints, MinKnotValue, MaxKnotValue);
	oneOverDegree = (GReal)1 / (GReal)Degree;
	if (Degree == 1)
		KnotsAveraging(knots, uk, Degree, 0, n - Degree);
	else
		KnotsAveraging(knots, uk, Degree, 0, n - Degree + 2);

	// in this case the only possible solution is the control polygon specified by fit points,
	// derivatives constraints cannot be satisfied
	if (Degree == 1) {
		SetPoints(FitPoints, knots, Degree, G_FALSE);
		return G_NO_ERROR;
	}
	// build rhs
	rhs[0] = FitPoints[0];
	rhs[1] = ((knots[Degree + 1] - MinKnotValue) * oneOverDegree) * Derivative0;
	for (i = 1; i <= n - 1; i++)
		rhs[i + 1] = FitPoints[i];
	rhs[n + 1] = ((MaxKnotValue - knots[m - Degree - 1]) * oneOverDegree) * Derivative1;
	rhs[n + 2] = FitPoints[n];

	// set basic structures to the output curve
	SetPoints(rhs, knots, Degree, G_FALSE);

	// build basis functions matrix
	for (i = 1; i <= n - 1; i++) {
		span = FindSpan(uk[i]);
		basisFuncs = BasisFunctions(span, Degree, uk[i]);
		// build i-th row
		for (j = 0; j <= Degree; j++)
			MATRIX(i + 1, span - Degree + j) = basisFuncs[j];
	}
	// build first row
	MATRIX(0, 0) = (GReal)1;
	// build second row (first derivative condition)
	MATRIX(1, 0) = -1;
	MATRIX(1, 1) = 1;
	// build "last but one" row (first derivative condition)
	MATRIX(n + 1, n + 1) = -1;
	MATRIX(n + 1, n + 2) = 1;
	// build last row
	MATRIX(n + 2, n + 2) = (GReal)1;
	// resolve the system
	return SolveBandedSystem(matrix, n + 3, Degree - 1, Degree - 1, gPoints);
	#undef MATRIX
}

// curve (global) natural (second derivaives = 0) fitting
GError GBSplineCurve2D::GlobalNaturalFit(const GInt32 Degree, const GDynArray<GPoint2>& FitPoints,
										 const GReal MinKnotValue, const GReal MaxKnotValue) {

	GInt32 i, j, span, n = (GInt32)FitPoints.size() - 1, m;

	if ((Degree <= 0) || (Degree > n + 2))
		return G_INVALID_PARAMETER;

	if (Degree == 1)
		m = n + Degree + 1;
	else
		m = n + Degree + 3;

	GDynArray<GReal> matrix((n + 3) * (n + 3), 0);
	GDynArray<GReal> knots(m + 1);
	GDynArray<GReal> uk(n + 1);
	GDynArray<GPoint2> rhs(n + 3);
	GReal *basisFuncs, c;
	#define MATRIX(i, j) matrix[(i)*(n + 3) + (j)]

	// generate knot vector
	ChordLengthKnots(uk, FitPoints, MinKnotValue, MaxKnotValue);
	if (Degree == 1)
		KnotsAveraging(knots, uk, Degree, 0, n - Degree);
	else
		KnotsAveraging(knots, uk, Degree, 0, n - Degree + 2);

	// in this case the only possible solution is the control polygon specified by fit points,
	// second order derivatives constraints are satisfied by definition
	if (Degree == 1) {
		SetPoints(FitPoints, knots, Degree, G_FALSE);
		return G_NO_ERROR;
	}

	// build rhs
	rhs[0] = FitPoints[0];
	rhs[1] = G_NULL_POINT2;
	for (i = 1; i <= n - 1; i++)
		rhs[i + 1] = FitPoints[i];
	rhs[n + 1] = G_NULL_POINT2;
	rhs[n + 2] = FitPoints[n];

	// set basic structures to the output curve
	SetPoints(rhs, knots, Degree, G_FALSE);

	// build basis functions matrix
	for (i = 1; i <= n - 1; i++) {
		span = FindSpan(uk[i]);
		basisFuncs = BasisFunctions(span, Degree, uk[i]);
		// build i-th row
		for (j = 0; j <= Degree; j++)
			MATRIX(i + 1, span - Degree + j) = basisFuncs[j];
	}
	// build first row
	MATRIX(0, 0) = (GReal)1;
	// build second row (second derivative condition)
	c = (GReal)Degree * (GReal)(Degree - 1);
	c = c / (knots[Degree + 1] - MinKnotValue);
	MATRIX(1, 0) = c / (knots[Degree + 1] - MinKnotValue);
	MATRIX(1, 1) = -c * ((knots[Degree + 1] - MinKnotValue) + (knots[Degree + 2] - MinKnotValue)) / ((knots[Degree + 1] - MinKnotValue) * (knots[Degree + 2] - MinKnotValue));
	MATRIX(1, 2) = c / (knots[Degree + 2] - MinKnotValue);
	// build "last but one" row (second derivative condition)
	c = (GReal)Degree * (GReal)(Degree - 1);
	c = c / (MaxKnotValue - knots[m - Degree - 1]);
	MATRIX(n + 1, n) = c / (MaxKnotValue - knots[m - Degree - 2]);
	MATRIX(n + 1, n + 1) = -c * ((2 * MaxKnotValue - knots[m - Degree - 1] - knots[m - Degree - 2]) / ((MaxKnotValue - knots[m - Degree - 1]) * (MaxKnotValue - knots[m - Degree - 2])));
	MATRIX(n + 1, n + 2) = c / (MaxKnotValue - knots[m - Degree - 1]);
	// build last row
	MATRIX(n + 2, n + 2) = (GReal)1;
	// resolve the system
	return SolveBandedSystem(matrix, n + 3, Degree - 1, Degree - 1, gPoints);
	#undef MATRIX
}

// curve (global) fitting with first derivative specified at each point
GError GBSplineCurve2D::GlobalFit(const GInt32 Degree, const GDynArray<GPoint2>& FitPoints,
								  const GDynArray<GVector2>& Derivatives,
								  const GReal MinKnotValue, const GReal MaxKnotValue) {

	GInt32 n = (GInt32)FitPoints.size() - 1, m, i, j, k, span;

	if ((Degree <= 0) || (Degree > 2 * n + 1))
		return G_INVALID_PARAMETER;

	if (FitPoints.size() != Derivatives.size())
		return G_INVALID_PARAMETER;

	if (Degree == 1)
		m = n + Degree + 1;
	else
		m = (n + Degree + 1) + (n + 1);

	GDynArray<GReal> matrix(2 * (n + 1) * 2 * (n + 1), 0);
	GDynArray<GPoint2> rhs(2 * (n + 1));
	GDynArray<GReal> knots(m + 1);
	GDynArray<GReal> uk(n + 1);
	GReal *basisFuncs, *derFuncs, oneOverDegree, u;
	#define MATRIX(i, j) matrix[(i) * (2 * (n + 1)) + (j)]

	// generate knot vector (and intermediate ones)
	ChordLengthKnots(uk, FitPoints, MinKnotValue, MaxKnotValue);
	// for clamped spline, first knot must have a Degree + 1 multiplicity
	for (i = 0; i < Degree + 1; i++)
		knots[i] = MinKnotValue;
	// for clamped spline, last knot must have a Degree + 1 multiplicity
	for (i = m - Degree; i <= m; i++)
		knots[i] = MaxKnotValue;

	// calculate internal knots
	oneOverDegree = (GReal)1 / (GReal)Degree;
	switch (Degree) {
		case 1:
			KnotsAveraging(knots, uk, 1, 1, n - 1);
			SetPoints(FitPoints, knots, Degree, G_FALSE);
			return G_NO_ERROR;
			break;

		case 2:
			k = Degree + 1;
			for (i = 1; i <= n; i++) {
				knots[k++] = (uk[i - 1] + uk[i]) * oneOverDegree;
				knots[k++] = uk[i];
			}
			break;

		case 3:
			// at each knot, we have first to translate uk to be positive
			// this fix negative parameter for knot range
			k = Degree + 1;
			u = (((uk[1] + MinKnotValue) + (MinKnotValue + MinKnotValue)) * (GReal)0.5) - MinKnotValue;
			knots[k++] = u;
			for (i = 1; i <= n - 2; i++) {
				u = (2 * (uk[i] + MinKnotValue) + (uk[i + 1] + MinKnotValue)) * oneOverDegree;
				knots[k++] = u - MinKnotValue;
				u = (uk[i] + MinKnotValue + 2 * (uk[i + 1] + MinKnotValue)) * oneOverDegree;
				knots[k++] = u - MinKnotValue;
			}
			u = (uk[n - 1] + MinKnotValue + uk[n] + MinKnotValue) * (GReal)0.5;
			knots[m - Degree - 1] = u - MinKnotValue;
			break;

		default: {
			GDynArray<GReal> uk2(2 * n + 2);
			for (i = 0; i < n; i++) {
				uk2[2 * i] = uk[i];
				uk2[2 * i + 1] = (uk[i] + uk[i + 1]) * (GReal)0.5;
			}
			uk2[2 * n + 1] = uk[n];
      		uk2[2 * n] = (uk2[2 * n + 1] + uk2[2 * n - 1]) * (GReal)0.5;
			// knot averaging
			for (j = 1; j < 2 * n + 2 - Degree; j++) {
				knots[j + Degree] = 0;
				for (i = j; i < j + Degree; i++)
					knots[j + Degree] += uk2[i];
				knots[j + Degree] *= oneOverDegree;
			}
		}
	}

	// build rhs
	rhs[0] = FitPoints[0];
	rhs[1] = ((knots[Degree + 1] - MinKnotValue) * oneOverDegree) * Derivatives[0];
	k = 2;
	for (i = 1; i <= n - 1; i++) {
		rhs[k++] = FitPoints[i];
		rhs[k++] = Derivatives[i];
	}
	rhs[2 * (n + 1) - 2] = ((MaxKnotValue - knots[m - Degree - 1]) * oneOverDegree) * Derivatives[n];
	rhs[2 * (n + 1) - 1] = FitPoints[n];

	// set basic structures to the output curve
	SetPoints(rhs, knots, Degree, G_FALSE);

	// build basis functions matrix
	k = 2;
	for (i = 1; i <= n - 1; i++) {
		span = FindSpan(uk[i]);
		basisFuncs = BasisFunctions(span, Degree, uk[i]);
		// build row corresponding to the point interpolation condition
		for (j = 0; j <= Degree; j++)
			MATRIX(k, span - Degree + j) = basisFuncs[j];
		// build row corresponding to the derivative interpolation condition
		k++;
		derFuncs = BasisFuncDerivatives(G_FIRST_ORDER_DERIVATIVE, span, Degree, uk[i]);
		for (j = 0; j <= Degree; j++)
			MATRIX(k, span - Degree + j) = derFuncs[j];
		k++;
	}
	// build first row
	MATRIX(0, 0) = (GReal)1;
	// build second row (first derivative condition)
	MATRIX(1, 0) = -1;
	MATRIX(1, 1) = 1;
	// build "last but one" row (first derivative condition)
	MATRIX(2 * (n + 1) - 2 , 2 * (n + 1) - 2) = -1;
	MATRIX(2 * (n + 1) - 2, 2 * (n + 1) - 1) = 1;
	// build last row
	MATRIX(2 * (n + 1) - 1, 2 * (n + 1) - 1) = (GReal)1;
	// resolve the system
	return SolveBandedSystem(matrix, 2 * (n + 1), Degree - 1, Degree - 1, gPoints);
	#undef MATRIX
}

};	// end namespace Amanith
