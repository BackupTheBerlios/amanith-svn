/****************************************************************************
** $file: amanith/src/2d/gbeziercurve2d.cpp   0.1.1.0   edited Sep 24 08:00
**
** 2D Bezier curve segment implementation.
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

#include "amanith/2d/gbeziercurve2d.h"
#include "amanith/2d/ghermitecurve2d.h"
#include "amanith/geometry/gintersect.h"
#include "amanith/geometry/gxformconv.h"
#include "amanith/geometry/gxform.h"
#include "amanith/gerror.h"

/*!
	\file gbeziercurve2d.cpp
	\brief Implementation of 2D Bezier curve class.
*/

namespace Amanith {


// *********************************************************************
//                             GBezierCurve2D
// *********************************************************************

// constructor
GBezierCurve2D::GBezierCurve2D() : GCurve2D() {

	gModified = G_FALSE;
}

// constructor
GBezierCurve2D::GBezierCurve2D(const GElement* Owner) : GCurve2D(Owner) {

	gModified = G_FALSE;
}

// destructor
GBezierCurve2D::~GBezierCurve2D() {
}

// clear the curve (remove control points and set an empty knots interval)
void GBezierCurve2D::Clear() {

	gPoints.clear();
	gForwDiff1.clear();
	gForwDiff2.clear();
	gModified = G_FALSE;
	GCurve2D::Clear();
}

// get number of control points
GUInt32 GBezierCurve2D::PointsCount() const {

	return (GUInt32)gPoints.size();
}

// get curve degree
GInt32 GBezierCurve2D::Degree() const {

	return ((GInt32)PointsCount() - 1);
}

// get Index-th point
GPoint2 GBezierCurve2D::Point(const GUInt32 Index) const {

	if (Index >= PointsCount())
		return GPoint2(G_MIN_REAL, G_MIN_REAL);
	return gPoints[Index];
}

// set Index-th point
GError GBezierCurve2D::SetPoint(const GUInt32 Index, const GPoint2& NewPoint) {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;
	// copy new point
	gPoints[Index] = NewPoint;
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set control points
GError GBezierCurve2D::SetPoints(const GDynArray<GPoint2>& NewPoints) {

	if (NewPoints.size() == 0)
		return G_INVALID_PARAMETER;
	// copy new points
	gPoints = NewPoints;
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set control points for quadratic Bezier curves, provided for convenience.
GError GBezierCurve2D::SetPoints(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) {
	
	// copy new points
	gPoints.resize(3);
	gPoints[0] = P0;
	gPoints[1] = P1;
	gPoints[2] = P2;
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set control points for cubic Bezier curves, provided for convenience.
GError GBezierCurve2D::SetPoints(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) {

	// copy new points
	gPoints.resize(4);
	gPoints[0] = P0;
	gPoints[1] = P1;
	gPoints[2] = P2;
	gPoints[3] = P3;
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set global parameters corresponding to the start point and to the end point
GError GBezierCurve2D::SetDomain(const GReal NewMinValue, const GReal NewMaxValue) {

	GError err = GCurve2D::SetDomain(NewMinValue, NewMaxValue);

	if (err == G_NO_ERROR)
		// this is because forward differences depend on knots interval
		gModified = G_TRUE;
	return err;
}

// Bezier to Hermite conversion
GError GBezierCurve2D::ConvertToHermite(GHermiteCurve2D& Curve) const {

	if (Degree() != 3)
		return G_INVALID_OPERATION;

	GDynArray<GHermiteKey2D> keys(2);
	// first key
	keys[0].Parameter = DomainStart();
	keys[0].Value = gPoints[0];
	keys[0].InTangent = keys[0].OutTangent = 3 * (gPoints[1] - gPoints[0]);
	// last key
	keys[1].Parameter = DomainEnd();
	keys[1].Value = gPoints[3];
	keys[1].InTangent = keys[1].OutTangent = 3 * (gPoints[3] - gPoints[2]);
	return Curve.SetKeys(keys);
}

// returns number of intersection between control polygon and a ray
GInt32 GBezierCurve2D::CrossingCount(const GRay2& Ray) const {

	GBool intersected;
	GInt32 i, j, k;
	GUInt32 intersFlags;
	GReal localIntersParams[2];
	GLineSegment2 seg;

	j = Degree();
	k = 0;
	for (i = 0; i < j; i++) {
		seg.SetStartPoint(gPoints[i]);
		seg.SetEndPoint(gPoints[i + 1]);
		intersected = Intersect(Ray, seg, intersFlags, localIntersParams);
		if (intersected)
			k++;
	}
	return k;
}

// returns number of intersection between control polygon and X axis
GInt32 GBezierCurve2D::CrossingCountX() const {

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
GBool GBezierCurve2D::IntersectXRay(GDynArray<GVector2>& Intersections, const GReal Precision,
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

	GBezierCurve2D leftCurve, rightCurve;
	// pivot point
	u = (DomainStart() + DomainEnd()) * (GReal)0.5;
	Cut(u, &rightCurve, &leftCurve);
	intLeft = leftCurve.IntersectXRay(Intersections, Precision, MaxIterations);
	intRight = rightCurve.IntersectXRay(Intersections, Precision, MaxIterations);
	return (intLeft | intRight);
}

// returns control polygon length
GReal GBezierCurve2D::ControlPolygonLength(const GUInt32 FromIndex, const GUInt32 ToIndex) const {

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
GBool GBezierCurve2D::IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
								const GReal Precision, const GUInt32 MaxIterations) const {

	if (PointsCount() <= 1)
		return G_FALSE;

	GMatrix33 rayTrans, rayRot, rayMatrix;
	GBezierCurve2D tmpCurve = (*this);
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

// increases by one the degree of the curve
GError GBezierCurve2D::HigherDegree() {
	
	GBezierCurve2D tmpCurve;
	GError err = HigherDegree(tmpCurve);
	if (err == G_NO_ERROR)
		gPoints = tmpCurve.gPoints;
	return err;
}

// increases by one the degree of the curve and gives the result out
GError GBezierCurve2D::HigherDegree(GBezierCurve2D& OutputCurve) const {

	GInt32 i, j;
	GReal k0, k1;
	GInt32 p = Degree();
	GDynArray<GPoint2> newPoints(p + 2);

	j = p + 2;
	newPoints[0] = StartPoint();
	for (i = 1; i < j - 1; i++) {
		k0 = 1 - (GReal)i / (j - 1);
		k1 = (GReal)i / (j - 1);
		newPoints[i] = (k0 * gPoints[i]) + (k1 * gPoints[i - 1]);
	}
	newPoints[p + 1] = EndPoint();
	OutputCurve.SetPoints(newPoints);
	return G_NO_ERROR;
}

GError GBezierCurve2D::HigherDegree(const GInt32 HowManyTimes) {

	GBezierCurve2D tmpCurve;
	GError err = HigherDegree(HowManyTimes, tmpCurve);
	if (err == G_NO_ERROR)
		(*this) = tmpCurve;
	return err;
}

GError GBezierCurve2D::HigherDegree(const GInt32 HowManyTimes, GBezierCurve2D& OutputCurve) const {

	GInt32 ph, ph2, i, j, mpi, k, w, p = Degree();
	GInt32 s = (p + HowManyTimes + 1) * (p + 1);
	GReal invBin;
	GDynArray<GReal> elevatingCoeff(s, (GReal)1);
	GDynArray<GPoint2> newPoints(p + HowManyTimes + 1);

	ph = p + HowManyTimes;
	ph2 = ph / 2;
	//
	// Control points of a t times degree elevated Bezier curve
	//
	//   t    min(degree, i)    (p over j) * (t over (i - j)
	//  P  = SUM              -------------------------------  * P       with i = 0, 1, ..., p+t
	//   i    j=max(0, i-t)         ((p + t) over i)              j
	//
	// compute Bezier degree elevation coefficients
	for (i = 1; i <= ph2; i++) {
		invBin = (GReal)1 / (GReal)GMath::Binomial((GReal)ph, (GReal)i);
		mpi = GMath::Min(p, i);
		for (j = GMath::Max((GInt32)0, i - HowManyTimes); j <= mpi; j++) {
			k = (i * (p + 1)) + j;
			elevatingCoeff[k] = invBin * (GReal)GMath::Binomial((GReal)p, (GReal)j) * (GReal)GMath::Binomial((GReal)HowManyTimes, (GReal)(i - j));
		}
	}
	for (i = ph2 + 1; i <= ph - 1; i++) {
		mpi = GMath::Min(p, i);
		for (j = GMath::Max((GInt32)0, i - HowManyTimes); j <= mpi; j++) {
			k = i * (p + 1) + j;
			w = (ph - i) * (p + 1) + (p - j);
			// this exploits that, in general, "n over i" = "n over (n-i)"
			elevatingCoeff[k] = elevatingCoeff[w];
		}
	}
	// lets elevate degree
	newPoints[0] = StartPoint();
	for (i = 1; i <= p + HowManyTimes - 1; i++) {
		mpi = GMath::Min(p, i);
		newPoints[i].Set(0, 0);
		for (j = GMath::Max((GInt32)0, i - HowManyTimes); j <= mpi; j++) {
			k = i * (p + 1) + j;
			newPoints[i] += (elevatingCoeff[k] * gPoints[j]);
		}
	}
	newPoints[p + HowManyTimes] = EndPoint();
	OutputCurve.SetPoints(newPoints);
	return G_NO_ERROR;
}

// decreases by one the degree of the curve
GError GBezierCurve2D::LowerDegree() {

	GError err;
	GBezierCurve2D tmpCurve;

	if (Degree() < 2)
		return G_NO_ERROR;

	err = LowerDegree(tmpCurve);
	if (err == G_NO_ERROR)
		(*this) = tmpCurve;
	return err;
}

// decreases by one the degree of the curve and gives the result out
GError GBezierCurve2D::LowerDegree(GBezierCurve2D& OutputCurve) const {

	GInt32 i, j, k, r, odd;
	GPoint2 v1, v2;
	GReal fDegree, fi, k0, k1, step;

	j = Degree();
	if (j < 2)
		return G_NO_ERROR;
	fDegree = (GReal)j;

	GDynArray<GPoint2> newPoints(j);
	newPoints[0] = gPoints[0];
	newPoints[j - 1] = gPoints[j];
	// special case where output curve is the chord
	if (j == 2) {
		OutputCurve.SetPoints(newPoints);
		return G_NO_ERROR;
	}

	GDynArray<GPoint2> newPoints2(j);
	newPoints2[0] = gPoints[0];
	newPoints2[j - 1] = gPoints[j];

	// classic odd/even schema proposed in The Nurbs Book
	r = (j - 1) / 2;
	odd = j & 1;
	// test if degree is odd
	if (odd)
		k = r - 1;
	else
		k = r;
	// "left" part
	for (i = 1; i <= k; i++) {
		fi = (GReal)i / fDegree;
		newPoints[i] = (gPoints[i] - fi * newPoints[i - 1]) / (1 - fi);
	}
	// "right" part
	for (i = j - 2; i >= r + 1; i--) {
		fi = (GReal)(i + 1) / fDegree;
		newPoints[i] = (gPoints[i + 1] - (1 - fi) * newPoints[i + 1]) / fi;
	}
	if (odd) {
		fi = (GReal)r / fDegree;
		v1 = (gPoints[r] - fi * newPoints[r - 1]) / (1 - fi);
		fi = (GReal)(r + 1) / fDegree;
		v2 = (gPoints[r + 1] - (1 - fi) * newPoints[r + 1]) / fi;
		newPoints[r] = (v1 + v2) * (GReal)0.5;
	}

	// Our custom schema: use the "left" part formula over whole curve, then use "right" part formula
	// over whole curve, and then average with linear weights. This method is a little slower, but shows better
	// graphical results.

	// first extrapolation
	for (i = 1; i < j - 1; i++) {
		fi = (GReal)i / fDegree;
		newPoints2[i] = (gPoints[i] - fi * newPoints2[i - 1]) / (1 - fi);
	}
	step = 1 / fDegree;
	k0 = 0;
	for (i = j - 2; i > 0; i--) {
		// second extrapolation
		fi = (GReal)(i + 1) / fDegree;
		v1 = (gPoints[i + 1] - (1 - fi) * newPoints2[i + 1]) / fi;
		// weighted extrapolation
		k0 += step;
		k1 = 1 - k0;
		newPoints2[i] = (k0 * newPoints2[i]) + (k1 * v1);
	}
	// lets do average schemas
	for (i = 0 ; i < Degree(); i++)
		newPoints[i] = (newPoints[i] + newPoints2[i]) * (GReal)0.5;

	// set points to the output curve
	OutputCurve.SetPoints(newPoints);
	return G_NO_ERROR;
}

GPoint2 GBezierCurve2D::Evaluate(const GReal u) const {

	GReal t, t1, fact, nOveri;
	GPoint2 tmpPoint;
	GInt32 i, n;

	// degree must be at least one
	n = Degree();
	if (n <= 0)
		return GPoint2(G_MIN_REAL, G_MIN_REAL);

	// check for global parameter out of range
	if (u <= DomainStart())
		return StartPoint();
	if (u >= DomainEnd())
		return EndPoint();

	// t is local parameter, it belongs to [0;1] interval
	t =  (u - DomainStart()) / Domain().Length();
	t1 = 1 - t;
	// Horner-Bezier algorithm
	fact = 1;
	tmpPoint = gPoints[0] * t1;
	i = 1;
	nOveri = 1;
	while (i < n) {
		fact = fact * t;
		nOveri = (nOveri * (n - i + 1)) / i;
		tmpPoint = (tmpPoint + fact * nOveri * gPoints[i]) * t1;
		i++;
	};

	tmpPoint = tmpPoint + fact * t * gPoints[i];
	return tmpPoint;
}

// calculate first and second order forward differences
void GBezierCurve2D::BuildForwDiff() const {

	GInt32 i, j;

	// calculate first order forward differences
	j = Degree();
	gForwDiff1.resize(j);
	for (i = 0; i < j; i++)
		gForwDiff1[i] = (j / Domain().Length()) * (gPoints[i + 1] - gPoints[i]);
	// calculate second order forward differences
	j--;
	gForwDiff2.resize(j);
	for (i = 0; i < j; i++)
		gForwDiff2[i] = (j / Domain().Length()) * (gForwDiff1[i + 1] - gForwDiff1[i]);

	// calculate external coefficients for derivate calculus
	//  r
	// d     n       n!     n-r           r           n-r
	// ---- b(t) = ------  SUM   (forwdiff) * Bernstein(t)
	//   r         (n-r)!   i=0           bi           i
	// dt
	gModified = G_FALSE;
}

// return the derivate Order-th calculated at global parameter u
GVector2 GBezierCurve2D::Derivative(const GDerivativeOrder Order, const GReal u) const {

	GDynArray<GPoint2>::const_iterator it;
	GReal t, t1, fact, nOveri, uu;
	GPoint2 tmpPoint;
	GInt32 i, n;

	// calculate degree of 
	n = Degree() - Order;
	// in this case derivate is null (ex: third derivate of a quadratic Bezier curve)
	if (n < 0)
		return G_NULL_POINT2;
	// if any point has been changed we have to recalculate forward differences
	if (gModified)
		BuildForwDiff();

	// clamp parameter inside valid interval
	if (u < DomainStart())
		uu = DomainStart();
	else
	if (u > DomainEnd())
		uu = DomainEnd();
	else
		uu = u;

	switch(Order) {
		case G_FIRST_ORDER_DERIVATIVE:
			it = gForwDiff1.begin();
			break;
		case G_SECOND_ORDER_DERIVATIVE:
			it = gForwDiff2.begin();
			break;
	}
	if (n == 0)
		return (*it);
	// t is local parameter, it belongs to [0;1] interval
	t = (uu - DomainStart()) / Domain().Length();
	t1 = (GReal)1 - t;
	// Horner-Bezier algorithm
	fact = (GReal)1;
	tmpPoint = (*it) * t1;
	it++;
	i = 1;
	nOveri = 1;
	while (i < n) {
		fact = fact * t;
		nOveri = (nOveri * (n - i + 1)) / i;
		tmpPoint = (tmpPoint + fact * nOveri * (*it)) * t1;
		it++;
		i++;
	}
	tmpPoint = tmpPoint + fact * t * (*it);
	return tmpPoint;
}

// cut the curve, giving the 2 new set of control points that represents 2 Bezier curve (with the
// same degree of the original one)
// We use De Casteljau's Algorithm
GError GBezierCurve2D::DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const {

	GReal t, t1;
	GInt32 n, i, r, j;
	GPoint2 tmpPoint;
	GBezierCurve2D *rCurve = (GBezierCurve2D *)RightCurve;
	GBezierCurve2D *lCurve = (GBezierCurve2D *)LeftCurve;

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

	n = Degree();
	t = (u - DomainStart()) / Domain().Length();
	t1 = 1 - t;

	// right part
	if (rCurve) {
		rCurve->SetPoints(gPoints);
		r = 1;
		i = 0;
		do {
			do {
				rCurve->gPoints[i] = (t1 * rCurve->gPoints[i]) + (t * rCurve->gPoints[i + 1]);
				i++;
			} while (i <= n - r);
			i = 0;
			r++;
		} while (r <= n);
		// set right curve global parameter range
		rCurve->SetDomain(u, DomainEnd());
	}

	// left part
	if (lCurve) {
		t1 = 1 - t1;
		t = 1 - t;
		j = Degree() + 1;
		lCurve->gPoints.resize(j);
		lCurve->gModified = G_TRUE;
		for (i = 0; i < j; i++)
			lCurve->gPoints[i] = gPoints[n - i];

		r = 1;
		i = 0;
		do {
			do {
				lCurve->gPoints[i] = t1 * lCurve->gPoints[i] + t * lCurve->gPoints[i + 1];
				i++;
			} while (i <= n - r);
			i = 0;
			r++;
		} while (r <= n);

		// lets mirror points
		ReverseArray(lCurve->gPoints, 0, j - 1);
		// set left curve global parameter range
		lCurve->SetDomain(DomainStart(), u);
	}
	return G_NO_ERROR;
}

// optimized flattening for quadratic Bezier
GError GBezierCurve2D::Flatten2(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
							 const GBool IncludeLastPoint) const {

	if (Degree() != 2)
		return G_INVALID_OPERATION;

	GBezierCurve2D tmpBez;
	GReal u, eps;
	GVector2 k;

	tmpBez.SetPoints(gPoints);
	tmpBez.SetDomain(0, 1);

	u = 0;
	eps = (GReal)2 * GMath::Pow(MaxDeviation, (GReal)0.25);
	do {
		Contour.push_back(tmpBez.gPoints[0]);
		k = 2 * tmpBez.gPoints[1] - tmpBez.gPoints[0] - tmpBez.gPoints[2];
		u = eps / GMath::Sqrt(k.Length());
		tmpBez.gPoints[0] = (1 - u) * tmpBez.gPoints[0] + u * tmpBez.gPoints[1];
		tmpBez.gPoints[1] = (1 - u) * tmpBez.gPoints[1] + u * tmpBez.gPoints[2];
		tmpBez.gPoints[0] = (1 - u) * tmpBez.gPoints[0] + u * tmpBez.gPoints[1];
		
	} while(u < 1);

	if (IncludeLastPoint)
		Contour.push_back(gPoints[2]);
	return G_NO_ERROR;
}

//	Calculate parametric values of cusp and inflection points (t1 <= cusp <= t2)
//	Return false if no inflection points
GBool GBezierCurve2D::FindInflectionPoints(const GReal Ax, const GReal Bx, const GReal Cx,
										const GReal Ay, const GReal By, const GReal Cy,
										GReal& Flex1, GReal& Flex2, GReal& Cuspid) const {

	Flex1 = Flex2 = Cuspid = 0;

	GReal a = (GReal)6 * (Ay * Bx - Ax * By);
	GReal b = (GReal)3 * (Ay * Cx - Ax * Cy);
	GReal c = By * Cx - Bx * Cy;
	GReal r2 = (b * b - (GReal)2 * a * c);

	if (r2 < 0 || GMath::Abs(a) < (GReal)0.0001)
		return G_FALSE;
	else {
		Cuspid = -b / a;
		GReal r = GMath::Sqrt(r2) / a;
		if (a > 0) {
			Flex1 = Cuspid - r;
			Flex2 = Cuspid + r;
		}
		else {
			Flex1 = Cuspid + r;
			Flex2 = Cuspid - r;
		}
		return G_TRUE;
	}
}

//	Find "flat" range around inflection point at t
void GBezierCurve2D::ExciseInflectionPoint(const GReal Flex, const GReal Flatness,
										GReal& ParamMinus, GReal& ParamPlus) const {
		
	G_ASSERT (Degree() == 3);

	if (Flex <= 0) {
		ParamMinus = ParamPlus = -1;
		return;
	}
	else
	if (Flex >= 1) {
		ParamMinus = ParamPlus = 2;
		return;
	}

	GBezierCurve2D bez2;

	Cut(Flex, &bez2, NULL);

	GPoint2 p1(bez2.gPoints[0]), p2(bez2.gPoints[1]), p3(bez2.gPoints[2]), p4(bez2.gPoints[3]);

	GReal cx = 3 * (p2[G_X] - p1[G_X]);
	GReal cy = 3 * (p2[G_Y] - p1[G_Y]);			
	GReal ex = 3 * (p2[G_X] - p3[G_X]);
	GReal ey = 3 * (p2[G_Y] - p3[G_Y]);
	GReal bx = -cx - ex;
	GReal by = -cy - ey;
	GReal ax = (ex - bx) / 3 + p4[G_X] - p3[G_X];
	GReal ay = (ey - by) / 3 + p4[G_Y] - p3[G_Y];
	GReal derOfAccel = GMath::Abs(6 * (ey * ax - ex * ay) / GMath::Sqrt(ex * ex + ey * ey)) + 0.00001f; 
	GReal tbez2 = GMath::Pow((GReal)9 * Flatness / derOfAccel, (GReal)1 / (GReal)3);
	ParamPlus  = Flex + (1 - Flex) * tbez2;
	ParamMinus = Flex - (1 - Flex) * tbez2;
}

// Polyline points (excluding last end point) along curve using parabolic approximation
// Precondition: there are no inflection points
void GBezierCurve2D::ParabolicApproxBezierPointsNoInflPts(const GReal Flatness, GDynArray<GPoint2>& Contour) const {

	G_ASSERT (Degree() == 3);

	GBezierCurve2D bez;
	GBezierCurve2D bez2;

	bez.gPoints = gPoints;
	bez.SetDomain(0, 1);
	Contour.push_back(gPoints[0]);
	while (1) {
		GReal dx = bez.gPoints[1][G_X] - bez.gPoints[0][G_X];
		GReal dy = bez.gPoints[1][G_Y] - bez.gPoints[0][G_Y];			

		GReal dNorm = GMath::Sqrt(dx * dx + dy * dy);
		if (dNorm <= G_EPSILON)
			return;

		GReal d = GMath::Abs(dx * (bez.gPoints[2][G_Y] - bez.gPoints[1][G_Y]) - dy * (bez.gPoints[2][G_X] - bez.gPoints[1][G_X]));

		GReal t = GMath::Sqrt((GReal)4 / (GReal)3 * dNorm * Flatness / d);
		if (t >= (GReal)1 - (GReal)G_EPSILON)
			break;
		//bez.Cut(t, &bez2, (GBezierCurve2D *)NULL);
		bez.DoCut(t, &bez2, (GCurve2D *)NULL);
		Contour.push_back(bez2.gPoints[0]);
		bez.gPoints = bez2.gPoints;
		bez.SetDomain(0, 1);
	}
}

// optimized flattening for cubic Bezier
GError GBezierCurve2D::Flatten3(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
								const GBool IncludeLastPoint) const {

	if (Degree() != 3)
		 return G_INVALID_OPERATION;

	GBezierCurve2D tmpBez;
	tmpBez.gPoints = gPoints;
	tmpBez.SetDomain(0, 1);

	GPoint2 p1(tmpBez.gPoints[0]), p2(tmpBez.gPoints[1]), p3(tmpBez.gPoints[2]), p4(tmpBez.gPoints[3]);

	if (p2 == p3) {
		p2[G_X] += (GReal)0.0001 * GMath::Max(GMath::Abs(p1[G_X] - p4[G_X]), GMath::Abs(p1[G_Y] - p4[G_Y]));
	}

	// MaxDeviation is a squared chordal distance, we must report to this value to a linear chordal distance
	GReal flatness = GMath::Sqrt(GMath::Abs(MaxDeviation));

	GReal cx = (p2[G_X] - p1[G_X]);
	GReal cy = (p2[G_Y] - p1[G_Y]);			
	GReal ex = (p2[G_X] - p3[G_X]);
	GReal ey = (p2[G_Y] - p3[G_Y]);
	GReal bx = (-cx) + (-ex);
	GReal by = (-cy) + (-ey);
	GReal ax = (ex - bx) + p4[G_X] - p3[G_X];
	GReal ay = (ey - by) + p4[G_Y] - p3[G_Y];

	GReal ip1Minus, ip1, ip1Plus, ip2Minus, ip2, ip2Plus, cusp;
	if (!FindInflectionPoints(ax, 3 * bx, 3 * cx, ay, 3 * by, 3 * cy, ip1, ip2, cusp)) {
		tmpBez.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
		if (IncludeLastPoint)
			Contour.push_back(p4);
		return G_NO_ERROR;
	}

	GBezierCurve2D bez1, bez2, bez3;

	tmpBez.ExciseInflectionPoint(ip1, flatness, ip1Minus, ip1Plus);
	tmpBez.ExciseInflectionPoint(ip2, flatness, ip2Minus, ip2Plus);

	if (0 < ip1Minus) {
		if (1 <= ip1Minus) {
			tmpBez.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
		}
		else { // (ip1Minus < 1)
			tmpBez.Cut(ip1Minus, (GBezierCurve2D *)NULL, &bez1);
			bez1.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);

			Contour.push_back(tmpBez.Evaluate(ip1Minus));
			if (ip2Minus < ip1Plus)	{	// cusp
				if (cusp < 1) {
					Contour.push_back(tmpBez.Evaluate(cusp));
				}
				if (ip2Plus < 1) {
					tmpBez.Cut(ip2Plus, &bez2, (GBezierCurve2D *)NULL);
					bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
				}
			} 
			else
			if (ip1Plus < 1) {
				if (ip2Minus < 1) {
					//tmpBez.Cut(ip1Plus, ip2Minus, &bez2);
					DoCut(ip2Minus, (GCurve2D *)NULL, &bez3);
					DoCut(ip1Plus, &bez2, (GCurve2D *)NULL);

					bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);

					Contour.push_back(tmpBez.Evaluate(ip2Minus));
					if (ip2Plus < 1) {
						tmpBez.Cut(ip2Plus, &bez2, (GBezierCurve2D *)NULL);
						bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
					}
				}
				else { //(ip2Minus >= 1)
					tmpBez.Cut(ip1Plus, &bez2, (GBezierCurve2D *)NULL);
					bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
				}
			}
		}
	}
	else
	if (0 < ip1Plus) {
		Contour.push_back(p1);
		if (ip2Minus < ip1Plus)	{	// cusp
			if (cusp < 1) {
				Contour.push_back(tmpBez.Evaluate(cusp));
			}
			if (ip2Plus < 1) {
				tmpBez.Cut(ip2Plus, &bez2, (GBezierCurve2D *)NULL);
				bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
			}
		} 
		else
		if (ip1Plus < 1) {
			if (ip2Minus < 1) {
				//tmpBez.Cut(ip1Plus, ip2Minus, &bez2);
				DoCut(ip1Plus, (GCurve2D *)NULL, &bez3);
				DoCut(ip2Minus, &bez2, (GCurve2D *)NULL);

				bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);

				Contour.push_back(tmpBez.Evaluate(ip2Minus));
				if (ip2Plus < 1) {
					tmpBez.Cut(ip2Plus, &bez2, (GBezierCurve2D *)NULL);
					bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
				}
			}
			else { //(ip2Minus >= 1)
				tmpBez.Cut(ip1Plus, &bez2, (GBezierCurve2D *)NULL);
				bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
			}
		}
	}
	else
	if (0 < ip2Minus) {
		if (ip2Minus < 1) {
			tmpBez.Cut(ip2Minus, (GBezierCurve2D *)NULL, &bez1);
			bez1.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);

			Contour.push_back(tmpBez.Evaluate(ip2Minus));
			if (ip2Plus < 1) {
				tmpBez.Cut(ip2Plus, &bez2, (GBezierCurve2D *)NULL);
				bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
			}
		}
		else { //(ip2Minus >= 1)
			tmpBez.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
		}
	}
	else
	if (0 < ip2Plus) {
		Contour.push_back(p1);
		if (ip2Plus < 1) {
			tmpBez.Cut(ip2Plus, &bez2, (GBezierCurve2D *)NULL);
			bez2.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);
		}
	}
	else
		tmpBez.ParabolicApproxBezierPointsNoInflPts(flatness, Contour);

	if (IncludeLastPoint)
		Contour.push_back(p4);
	return G_NO_ERROR;
 }


 GError GBezierCurve2D::Flatten(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1,
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
GError GBezierCurve2D::Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
							const GBool IncludeLastPoint) const {

	if (MaxDeviation <= 0)
		return G_INVALID_PARAMETER;

	GInt32 deg = Degree();

	if (deg <= 0)
		return G_NO_ERROR;

	// optimized version for quadratic curves
	if (deg == 2)
		return Flatten2(Contour, MaxDeviation, IncludeLastPoint);
	else
	if (deg == 3)
		return Flatten3(Contour, MaxDeviation, IncludeLastPoint);
	else {
		GPoint2 p0(gPoints[0]);
		GPoint2 p1(gPoints[deg]);

		GError err = Flatten(DomainStart(), DomainEnd(), p0, p1, Contour, MaxDeviation);
		if ((err == G_NO_ERROR) && (IncludeLastPoint))
			Contour.push_back(p1);
		return err;
	}
}

// get max variation (squared chordal distance) in the domain range
GReal GBezierCurve2D::Variation() const {

	GInt32 deg = Degree();

	if (deg <= 1)
		return 0;

	//! \todo Optimized variation for a cubic curve
	if (deg == 3)
		return Variation(DomainStart(), DomainEnd(), gPoints[0], gPoints[deg]);
	else
		return Variation(DomainStart(), DomainEnd(), gPoints[0], gPoints[deg]);
}

// get max variation (squared chordal distance) in the range [u0;u1]; here are necessary also
// curve evaluations at the interval ends
GReal GBezierCurve2D::Variation(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1) const {

	GInt32 i, numSeg;
	GReal step, u, curVariation, tmpVariation;
	GPoint2 v;
	GRay2 ray(p0, p1 - p0);

	// normalize ray direction
	ray.Normalize();
	// taken from Graphics Gems 3: "Curve tessellation criteria through sampling"
	numSeg = 2 * (Degree() + 1);
	step = (u1 - u0) / (GReal)numSeg;
	u = u0;
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


// cloning function
GError GBezierCurve2D::BaseClone(const GElement& Source) {

	const GBezierCurve2D& k = (const GBezierCurve2D&)Source;

	// copy control points
	gPoints = k.gPoints;
	// copy array for forward differences
	gForwDiff1 = k.gForwDiff1;
	gForwDiff2 = k.gForwDiff2;
	// copy 'modified' flag
	gModified = k.gModified;
	return GCurve2D::BaseClone(Source);
}

};	// end namespace Amanith
