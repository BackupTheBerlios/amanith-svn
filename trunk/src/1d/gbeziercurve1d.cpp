/****************************************************************************
** $file: amanith/src/1d/gbezcurve1d.cpp   0.1.0.0   edited Jun 30 08:00
**
** 1D Bezier curve segment implementation.
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

#include "amanith/1d/gbeziercurve1d.h"
#include "amanith/1d/ghermitecurve1d.h"
#include "amanith/gerror.h"

/*!
	\file gbeziercurve1d.cpp
	\brief Implementation of 1D Bezier curve class.
*/

namespace Amanith {


// *********************************************************************
//                             GBezierCurve1D
// *********************************************************************

// constructor
GBezierCurve1D::GBezierCurve1D() : GCurve1D() {

	gModified = G_FALSE;
}

// constructor
GBezierCurve1D::GBezierCurve1D(const GElement* Owner) : GCurve1D(Owner) {

	gModified = G_FALSE;
}

// destructor
GBezierCurve1D::~GBezierCurve1D() {
}

// clear the curve (remove control points and set an empty knots interval)
void GBezierCurve1D::Clear() {

	gPoints.clear();
	gForwDiff1.clear();
	gForwDiff2.clear();
	gModified = G_FALSE;
	GCurve1D::Clear();
}

// get number of control points
GUInt32 GBezierCurve1D::PointsCount() const {

	return (GUInt32)gPoints.size();
}

// get curve degree
GInt32 GBezierCurve1D::Degree() const {

	return ((GInt32)PointsCount() - 1);
}

// get Index-th point
GReal GBezierCurve1D::Point(const GUInt32 Index) const {

	if (Index >= PointsCount())
		return G_MIN_REAL;
	return gPoints[Index];
}

// set Index-th point
GError GBezierCurve1D::SetPoint(const GUInt32 Index, const GReal NewPoint) {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;
	// copy new point
	gPoints[Index] = NewPoint;
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set control points
GError GBezierCurve1D::SetPoints(const GDynArray<GReal>& NewPoints) {

	if (NewPoints.size() == 0)
		return G_INVALID_PARAMETER;
	// copy new points
	gPoints = NewPoints;
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set control points for quadratic Bezier curves, provided for convenience.
GError GBezierCurve1D::SetPoints(const GReal P0, const GReal P1, const GReal P2) {
	
	// copy new points
	gPoints.resize(3);
	gPoints[0] = P0;
	gPoints[1] = P1;
	gPoints[2] = P2;
	gModified = G_TRUE;
	return G_NO_ERROR;
}

// set control points for cubic Bezier curves, provided for convenience.
GError GBezierCurve1D::SetPoints(const GReal P0, const GReal P1, const GReal P2, const GReal P3) {

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
GError GBezierCurve1D::SetDomain(const GReal NewMinValue, const GReal NewMaxValue) {

	GError err = GCurve1D::SetDomain(NewMinValue, NewMaxValue);

	if (err == G_NO_ERROR)
		// this is because forward differences depend on knots interval
		gModified = G_TRUE;
	return err;
}

// Bezier to Hermite conversion
GError GBezierCurve1D::ConvertToHermite(GHermiteCurve1D& Curve) const {

	if (Degree() != 3)
		return G_INVALID_OPERATION;

	GDynArray<GHermiteKey1D> keys(2);
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

// increases by one the degree of the curve
GError GBezierCurve1D::HigherDegree() {
	
	GBezierCurve1D tmpCurve;
	GError err = HigherDegree(tmpCurve);
	if (err == G_NO_ERROR)
		gPoints = tmpCurve.gPoints;
	return err;
}

// increases by one the degree of the curve and gives the result out
GError GBezierCurve1D::HigherDegree(GBezierCurve1D& OutputCurve) const {

	GInt32 i, j;
	GReal k0, k1;
	GInt32 p = Degree();
	GDynArray<GReal> newPoints(p + 2);

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

GError GBezierCurve1D::HigherDegree(const GInt32 HowManyTimes) {

	GBezierCurve1D tmpCurve;
	GError err = HigherDegree(HowManyTimes, tmpCurve);
	if (err == G_NO_ERROR)
		(*this) = tmpCurve;
	return err;
}

GError GBezierCurve1D::HigherDegree(const GInt32 HowManyTimes, GBezierCurve1D& OutputCurve) const {

	GInt32 ph, ph2, i, j, mpi, k, w, p = Degree();
	GInt32 s = (p + HowManyTimes + 1) * (p + 1);
	GReal invBin;
	GDynArray<GReal> elevatingCoeff(s, (GReal)1);
	GDynArray<GReal> newPoints(p + HowManyTimes + 1);

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
		newPoints[i] = 0;
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
GError GBezierCurve1D::LowerDegree() {

	GError err;
	GBezierCurve1D tmpCurve;

	if (Degree() < 2)
		return G_NO_ERROR;

	err = LowerDegree(tmpCurve);
	if (err == G_NO_ERROR)
		(*this) = tmpCurve;
	return err;
}

// decreases by one the degree of the curve and gives the result out
GError GBezierCurve1D::LowerDegree(GBezierCurve1D& OutputCurve) const {

	GInt32 i, j, k, r, odd;
	GReal v1, v2;
	GReal fDegree, fi, k0, k1, step;

	j = Degree();
	if (j < 2)
		return G_NO_ERROR;
	fDegree = (GReal)j;

	GDynArray<GReal> newPoints(j);
	newPoints[0] = gPoints[0];
	newPoints[j - 1] = gPoints[j];
	// special case where output curve is the chord
	if (j == 2) {
		OutputCurve.SetPoints(newPoints);
		return G_NO_ERROR;
	}

	GDynArray<GReal> newPoints2(j);
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

GReal GBezierCurve1D::Evaluate(const GReal u) const {

	GReal t, t1, fact, nOveri;
	GReal tmpPoint;
	GInt32 i, n;

	// degree must be at least one
	n = Degree();
	if (n <= 0)
		return G_MIN_REAL;

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
void GBezierCurve1D::BuildForwDiff() const {

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
GReal GBezierCurve1D::Derivative(const GDerivativeOrder Order, const GReal u) const {

	GDynArray<GReal>::const_iterator it;
	GReal t, t1, fact, nOveri, uu;
	GReal tmpPoint;
	GInt32 i, n;

	// calculate degree of 
	n = Degree() - Order;
	// in this case derivate is null (ex: third derivate of a quadratic Bezier curve)
	if (n < 0)
		return G_MIN_REAL;
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
GError GBezierCurve1D::DoCut(const GReal u, GCurve1D *RightCurve, GCurve1D *LeftCurve) const {

	GReal t, t1;
	GInt32 n, i, r, j;
	GBezierCurve1D *rCurve = (GBezierCurve1D *)RightCurve;
	GBezierCurve1D *lCurve = (GBezierCurve1D *)LeftCurve;

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

// cloning function
GError GBezierCurve1D::BaseClone(const GElement& Source) {

	const GBezierCurve1D& k = (const GBezierCurve1D&)Source;

	// copy control points
	gPoints = k.gPoints;
	// copy array for forward differences
	gForwDiff1 = k.gForwDiff1;
	gForwDiff2 = k.gForwDiff2;
	// copy 'modified' flag
	gModified = k.gModified;
	return GCurve1D::BaseClone(Source);
}

};	// end namespace Amanith
