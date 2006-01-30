/****************************************************************************
** $file: amanith/src/2d/gellipsecurve2d.cpp   0.3.0.0   edited Jan, 30 2006
**
** 2D ellipse curve segment implementation.
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

#include "amanith/2d/gellipsecurve2d.h"
#include "amanith/geometry/gxformconv.h"
#include "amanith/geometry/gxform.h"
#include "amanith/numerics/geigen.h"
/*!
	\file gellipsecurve2d.cpp
	\brief Implementation of 2D ellipse arc/curve class.
*/

namespace Amanith {


// *********************************************************************
//                             GEllipseCurve2D
// *********************************************************************

// constructor
GEllipseCurve2D::GEllipseCurve2D() : GCurve2D() {

	// gCenter constructor will set center at (0, 0)
	gXSemiAxisLength = 0;
	gYSemiAxisLength = 0;
	gOffsetRotation = 0;
	gCosOfsRot = 1;
	gSinOfsRot = 0;
	gStartAngle = 0;
	gEndAngle = 0;
	gCCW = G_TRUE;
}

// constructor
GEllipseCurve2D::GEllipseCurve2D(const GElement* Owner) : GCurve2D(Owner) {

	// gCenter constructor will set center at (0, 0)
	gXSemiAxisLength = -1;
	gYSemiAxisLength = -1;
	gOffsetRotation = 0;
	gCosOfsRot = 1;
	gSinOfsRot = 0;
	gStartAngle = 0;
	gEndAngle = (GReal)G_2PI;
	gCCW = G_TRUE;
}

// destructor
GEllipseCurve2D::~GEllipseCurve2D() {
}

void GEllipseCurve2D::Clear() {

	gCenter.Set(0, 0);
	gXSemiAxisLength = 0;
	gYSemiAxisLength = 0;
	gOffsetRotation = 0;
	gCosOfsRot = 1;
	gSinOfsRot = 0;
	gStartAngle = 0;
	gEndAngle = 0;
	GCurve2D::Clear();
}

// get number of control points
GUInt32 GEllipseCurve2D::PointsCount() const {

	return 2;
}

// get Index-th point
GPoint2 GEllipseCurve2D::Point(const GUInt32 Index) const {

	if (Index >= PointsCount())
		return GPoint2(G_MIN_REAL, G_MIN_REAL);

	// return start point
	if (Index == 0)
		return EvaluateByAngle(gStartAngle);
	// return end point
	else
		return EvaluateByAngle(gEndAngle);
}

GBool GEllipseCurve2D::IsLargeArc(const GReal StartAngle, const GReal EndAngle, const GBool CCW) {

	GReal l;

	if (CCW) {
		if (StartAngle < EndAngle)
			l = EndAngle - StartAngle;
		else
			l = (GReal)G_2PI - StartAngle + EndAngle;
	}
	// cw
	else {
		if (StartAngle < EndAngle)
			l = (GReal)G_2PI - EndAngle + StartAngle;
		else
			l = StartAngle - EndAngle;
	}
	if (l <= G_PI)
		return G_FALSE;
	else
		return G_TRUE;
}

GReal GEllipseCurve2D::FixAngle(const GReal Angle) {

	GReal n;

	if (Angle < 0) {
		n = GMath::Ceil(-Angle / (GReal)G_2PI);
		return (Angle + n * (GReal)G_2PI);
	}
	if (Angle > (GReal)G_2PI) {
		n = GMath::Floor(Angle / (GReal)G_2PI);
		return (Angle - n * (GReal)G_2PI);
	}
	else
		return Angle;
}

// set Index-th point
GError GEllipseCurve2D::SetPoint(const GUInt32 Index, const GPoint2& NewPoint) {

	if (Index >= PointsCount())
		return G_OUT_OF_RANGE;

	// set start point
	if (Index == 0)
		SetEllipse(NewPoint, EndPoint(), gXSemiAxisLength, gYSemiAxisLength, gOffsetRotation, LargeArc(), gCCW);
	// set end point
	else
		SetEllipse(StartPoint(), NewPoint, gXSemiAxisLength, gYSemiAxisLength, gOffsetRotation, LargeArc(), gCCW);
	return G_NO_ERROR;
}

void GEllipseCurve2D::SetEllipse(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
								 const GReal OffsetRotation, const GReal StartAngle, const GReal EndAngle,
								 const GBool CCW) {

	gCenter = Center;
	gXSemiAxisLength = GMath::Abs(XSemiAxisLength);
	gYSemiAxisLength = GMath::Abs(YSemiAxisLength);
	gOffsetRotation = OffsetRotation;
	gCosOfsRot = GMath::Cos(gOffsetRotation);
	gSinOfsRot = GMath::Sin(gOffsetRotation);
	gCCW = CCW;
	SetAngleDomain(StartAngle, EndAngle);
}

GBool GEllipseCurve2D::FindUnitCircles(const GPoint2& P0, const GPoint2& P1, GPoint2& C0, GPoint2& C1,
									   GReal& GrowingFactor) {

	GVector2 d = P1 - P0;
	GPoint2 pm = (P1 + P0) * (GReal)0.5;
	GReal dsq, disc, s, sdx, sdy;

	// solve for intersecting unit circles
	dsq = d.LengthSquared();
	if (dsq <= G_EPSILON) {
		// points are coincident
		GrowingFactor = 0;
		return G_FALSE;
	}
	disc = ((GReal)1 / dsq) - (GReal)0.25;
	if (disc <= G_EPSILON) {
		// points are too far apart, we must calculate the smallest factor that permits a solution
		GrowingFactor = d.Length() * (GReal)0.50001;
		return G_FALSE;
	}
	// two distinct centers
	s = GMath::Sqrt(disc);
	sdx = s * d[G_X];
	sdy = s * d[G_Y];
	C0.Set(pm[G_X] + sdy, pm[G_Y] - sdx);
	C1.Set(pm[G_X] - sdy, pm[G_Y] + sdx);
	return G_TRUE;
}

GError GEllipseCurve2D::SetEllipse(const GPoint2& P0, const GPoint2& P1,
								   const GReal XSemiAxisLength, const GReal YSemiAxisLength,
								   const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) {

	if (XSemiAxisLength <= G_EPSILON || YSemiAxisLength <= G_EPSILON || Amanith::Length(P1 - P0) <= G_EPSILON)
		return G_INVALID_PARAMETER;

	// pre-compute rotation matrix entries
	GReal c = GMath::Cos(OffsetRotation);
	GReal s = GMath::Sin(OffsetRotation);

	GPoint2 p0, p1, c0, c1, cc0, cc1;
	GReal theta0, theta1, theta2, theta3, gf = 1;

	// transform (x0, y0) and (x1, y1) into unit space using (inverse) rotate, followed by (inverse) scale
	p0[G_X] = (P0[G_X] * c + P0[G_Y] * s) / XSemiAxisLength;
	p0[G_Y] = (-P0[G_X] * s + P0[G_Y] * c) / YSemiAxisLength;
	p1[G_X] = (P1[G_X] * c + P1[G_Y] * s) / XSemiAxisLength;
	p1[G_Y] = (-P1[G_X] * s + P1[G_Y] * c) / YSemiAxisLength;

	GBool b = GEllipseCurve2D::FindUnitCircles(p0, p1, c0, c1, gf);

	if (!b) {
		if (gf == 0)
			return G_INVALID_PARAMETER;

		GReal oneOverGf = (GReal)1 / gf;
		b = GEllipseCurve2D::FindUnitCircles(oneOverGf * p0, oneOverGf * p1, c0, c1, gf);
		G_ASSERT(b == G_TRUE);

		GVector2 vv0 = (oneOverGf * p0) - c0;
		GVector2 vv1 = (oneOverGf * p1) - c0;
		GVector2 vv2 = (oneOverGf * p0) - c1;
		GVector2 vv3 = (oneOverGf * p1) - c1;
		theta0 = GMath::Atan2(vv0[G_Y], vv0[G_X]);
		theta1 = GMath::Atan2(vv1[G_Y], vv1[G_X]);
		theta2 = GMath::Atan2(vv2[G_Y], vv2[G_X]);
		theta3 = GMath::Atan2(vv3[G_Y], vv3[G_X]);
	}
	else {
		GVector2 vv0 = p0 - c0;
		GVector2 vv1 = p1 - c0;
		GVector2 vv2 = p0 - c1;
		GVector2 vv3 = p1 - c1;
		theta0 = GMath::Atan2(vv0[G_Y], vv0[G_X]);
		theta1 = GMath::Atan2(vv1[G_Y], vv1[G_X]);
		theta2 = GMath::Atan2(vv2[G_Y], vv2[G_X]);
		theta3 = GMath::Atan2(vv3[G_Y], vv3[G_X]);
	}

	// transform back to original coordinate space using (forward) scale followed by (forward) rotate
	c0[G_X] *= XSemiAxisLength * gf;
	c0[G_Y] *= YSemiAxisLength * gf;
	c1[G_X] *= XSemiAxisLength * gf;
	c1[G_Y] *= YSemiAxisLength * gf;

	// cc0 and cc1 now are the final centers
	cc0[G_X] = c0[G_X] * c - c0[G_Y] * s;
	cc0[G_Y] = c0[G_X] * s + c0[G_Y] * c;
	cc1[G_X] = c1[G_X] * c - c1[G_Y] * s;
	cc1[G_Y] = c1[G_X] * s + c1[G_Y] * c;

	GReal cross0, cross1;
	GVector2 v0, v1, v2, v3;

	v0 = P0 - cc0;
	v1 = P1 - cc0;
	v2 = P0 - cc1;
	v3 = P1 - cc1;

	// cross0 < 0 means that v0 span the greater angle in ccw to overlap v1
	cross0 = Cross(v0, v1);
	// cross1 < 0 means that v2 span the greater angle in ccw to overlap v3
	cross1 = Cross(v2, v3);

	if (LargeArc) {
		if (cross0 < 0) {
			if (CCW) {
				// ellipse cc0
				SetEllipse(cc0, XSemiAxisLength * gf, YSemiAxisLength * gf, OffsetRotation, theta0, theta1, G_TRUE);
			}
			else {
				// ellipse cc1 (to do: invert domain)
				SetEllipse(cc1, XSemiAxisLength * gf, YSemiAxisLength * gf, OffsetRotation, theta2, theta3, G_FALSE);
			}
		}
		else {
			if (CCW) {
				// ellipse cc1
				SetEllipse(cc1, XSemiAxisLength * gf, YSemiAxisLength * gf, OffsetRotation, theta2, theta3, G_TRUE);
			}
			else {
				// ellipse cc0
				SetEllipse(cc0, XSemiAxisLength * gf, YSemiAxisLength * gf, OffsetRotation, theta0, theta1, G_FALSE);
			}
		}
	}
	else {
		if (cross0 > 0) {
			if (CCW) {
				// ellipse cc0
				SetEllipse(cc0, XSemiAxisLength * gf, YSemiAxisLength * gf, OffsetRotation, theta0, theta1, G_TRUE);
			}
			else {
				// ellipse cc1
				SetEllipse(cc1, XSemiAxisLength * gf, YSemiAxisLength * gf, OffsetRotation, theta2, theta3, G_FALSE);
			}
		}
		else {
			if (CCW) {
				// ellipse cc1
				SetEllipse(cc1, XSemiAxisLength * gf, YSemiAxisLength * gf, OffsetRotation, theta2, theta3, G_TRUE);
			}
			else {
				// ellipse cc0 (to do: invert domain)
				SetEllipse(cc0, XSemiAxisLength * gf, YSemiAxisLength * gf, OffsetRotation, theta0, theta1, G_FALSE);
			}
		}
	}
	return G_NO_ERROR;
}

void GEllipseCurve2D::SetAngleDomain(const GReal NewStartAngle, const GReal NewEndAngle) {

	gStartAngle = GEllipseCurve2D::FixAngle(NewStartAngle);
	gEndAngle = GEllipseCurve2D::FixAngle(NewEndAngle);
}

GBool GEllipseCurve2D::InsideAngleDomain(const GReal Angle, GReal& Ratio) const {

	GReal ang = GEllipseCurve2D::FixAngle(Angle);

	if (gCCW) {
		if (gStartAngle < gEndAngle) {
			if (ang >= gStartAngle && ang <= gEndAngle) {
				Ratio = (ang - gStartAngle) / (gEndAngle - gStartAngle);
				return G_TRUE;
			}
			else
				return G_FALSE;
		}
		else {
			if (ang >= gEndAngle && ang <= gStartAngle)
				return G_FALSE;
			else {
				ang = ang - gStartAngle;
				if (ang < 0)
					ang += (GReal)G_2PI;
				Ratio = ang / ((GReal)G_2PI - gStartAngle + gEndAngle);
				return G_TRUE;
			}
		}
	}
	// cw
	else {
		if (gStartAngle < gEndAngle) {
			if (ang >= gStartAngle && ang <= gEndAngle)
				return G_FALSE;
			else {
				ang = gStartAngle - ang;
				if (ang < 0)
					ang += (GReal)G_2PI;
				Ratio = ang / ((GReal)G_2PI - gEndAngle + gStartAngle);
				return G_TRUE;
			}
		}
		else {
			if (ang >= gEndAngle && ang <= gStartAngle) {
				Ratio = (gStartAngle - ang) / (gStartAngle - gEndAngle);
				return G_TRUE;
			}
			else
				return G_FALSE;
		}
	}
}

// intersect the curve with a ray, and returns a list of intersections
GBool GEllipseCurve2D::IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
									const GReal Precision, const GUInt32 MaxIterations) const {

	// just to avoid warnings
	if (Precision && MaxIterations) {
	}

	// transform ray into ellipse coordinate system, so that ellipse will be centered at (0, 0) and will
	// be axis-aligned
	GVector2 v = NormalizedRay.Direction();
	GPoint2 p = NormalizedRay.Origin() - gCenter;
	GPoint2 o(p[G_X] * gCosOfsRot + p[G_Y] * gSinOfsRot, -p[G_X] * gSinOfsRot + p[G_Y] * gCosOfsRot);
	GVector2 d(v[G_X] * gCosOfsRot + v[G_Y] * gSinOfsRot, -v[G_X] * gSinOfsRot + v[G_Y] * gCosOfsRot);

	// calculate ellipse-ray intersection, solving for ray parameter
	GReal t0, t1, ratio, theta;
	GReal lxSqr = gXSemiAxisLength * gXSemiAxisLength;
	GReal lySqr = gYSemiAxisLength * gYSemiAxisLength;
	GReal A = lxSqr * d[G_Y] * d[G_Y] + lySqr * d[G_X] * d[G_X];
	GReal B = 2 * (lxSqr * d[G_Y] * o[G_Y] + lySqr * d[G_X] * o[G_X]);
	GReal C = lxSqr * (o[G_Y] * o[G_Y] - lySqr) + lySqr * o[G_X] * o[G_X];

	GInt32 numSol = GMath::QuadraticFormula(t0, t1, A, B, C);
	if (numSol == 0)
		return G_FALSE;
	else
	if (numSol == 1) {
		if (t0 < 0)
			return G_FALSE;
		
		GPoint2 p = o + t0 * d;
		theta = GMath::Atan2(p[G_Y] * gXSemiAxisLength, p[G_X] * gYSemiAxisLength);

		if (InsideAngleDomain(theta, ratio)) {
			Intersections.push_back(GVector2(GMath::Lerp(ratio, DomainStart(), DomainEnd()), t0));
			return G_TRUE;
		}
		else
			return G_FALSE;
	}
	else {
		G_ASSERT(numSol == 2);
		GBool intFound = G_FALSE;

		if (t0 > 0) {
			GPoint2 p = o + t0 * d;
			theta = GMath::Atan2(p[G_Y] * gXSemiAxisLength, p[G_X] * gYSemiAxisLength);
			if (InsideAngleDomain(theta, ratio)) {
				Intersections.push_back(GVector2(GMath::Lerp(ratio, DomainStart(), DomainEnd()), t0));
				intFound = G_TRUE;
			}
		}
		if (t1 > 0) {
			GPoint2 p = o + t1 * d;
			theta = GMath::Atan2(p[G_Y] * gXSemiAxisLength, p[G_X] * gYSemiAxisLength);
			if (InsideAngleDomain(theta, ratio)) {
				Intersections.push_back(GVector2(GMath::Lerp(ratio, DomainStart(), DomainEnd()), t0));
				intFound = G_TRUE;
			}
		}
		return intFound;
	}
}

GReal GEllipseCurve2D::MapAngle(const GReal u) const {

	G_ASSERT(Domain().IsInInterval(u));

	GReal r = (u - DomainStart()) / (Domain().Length());

	if (gCCW) {
		if (gStartAngle < gEndAngle)
			return GMath::Lerp(r, gStartAngle, gEndAngle);
		else {
			GReal res = gStartAngle + r * ((GReal)G_2PI - gStartAngle + gEndAngle);
			if (res > (GReal)G_2PI)
				res -= (GReal)G_2PI;
			return res;
		}
	}
	// cw
	else {
		if (gStartAngle < gEndAngle) {
			GReal res = gStartAngle - r * ((GReal)G_2PI - gEndAngle + gStartAngle);
			if (res < 0)
				res += (GReal)G_2PI;
			return res;
		}
		else
			return (gStartAngle - r * (gStartAngle - gEndAngle));
	}
}

GPoint2 GEllipseCurve2D::Evaluate(const GReal u) const {

	GReal l = Domain().Length();

	if (l <= G_EPSILON)
		return StartPoint();

	GReal clampedU = GMath::Clamp(u, DomainStart(), DomainEnd());
	GReal angle = MapAngle(clampedU);

	return EvaluateByAngle(angle);
}

// return the derivate Order-th calculated at global parameter u
GVector2 GEllipseCurve2D::Derivative(const GDerivativeOrder Order, const GReal u) const {

	GReal l = Domain().Length();

	if (l <= G_EPSILON)
		return G_NULL_POINT2;

	GReal clampedU = GMath::Clamp(u, DomainStart(), DomainEnd());
	GReal angle = MapAngle(clampedU);

	if (Order == G_FIRST_ORDER_DERIVATIVE) {

		if (gCCW) {
			if (gStartAngle < gEndAngle)
				l = (gEndAngle - gStartAngle) / l;
			else
				l = ((GReal)G_2PI - gStartAngle + gEndAngle) / l;
		}
		// cw
		else {
			if (gStartAngle < gEndAngle)
				l = -((GReal)G_2PI - gEndAngle + gStartAngle) / l;
			else
				l = -(gStartAngle - gEndAngle) / l;
		}

		GVector2 v((-gXSemiAxisLength * GMath::Sin(angle)) * l, (gYSemiAxisLength * GMath::Cos(angle)) * l);
		return GVector2(v[G_X] * gCosOfsRot - gSinOfsRot * v[G_Y], v[G_X] * gSinOfsRot + v[G_Y] * gCosOfsRot);
	}
	else {
		if (gCCW) {
			if (gStartAngle < gEndAngle) {
				l = (gEndAngle - gStartAngle) / l;
				l *= l;
			}
			else {
				l = ((GReal)G_2PI - gStartAngle + gEndAngle) / l;
				l *= l;
			}
		}
		// cw
		else {
			if (gStartAngle < gEndAngle) {
				l = -((GReal)G_2PI - gEndAngle + gStartAngle) / l;
				l *= l;
			}
			else {
				l = -(gStartAngle - gEndAngle) / l;
				l *= l;
			}
		}

		G_ASSERT(Order == G_SECOND_ORDER_DERIVATIVE);
		GVector2 v((-gXSemiAxisLength * GMath::Cos(angle)) * l, (-gYSemiAxisLength * GMath::Sin(angle)) * l);
		return GVector2(v[G_X] * gCosOfsRot - gSinOfsRot * v[G_Y], v[G_X] * gSinOfsRot + v[G_Y] * gCosOfsRot);
	}
}

GError GEllipseCurve2D::DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const {

	GEllipseCurve2D *rCurve = (GEllipseCurve2D *)RightCurve;
	GEllipseCurve2D *lCurve = (GEllipseCurve2D *)LeftCurve;

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

	GReal angle = MapAngle(u);

	if (rCurve) {
		rCurve->gCenter = gCenter;
		rCurve->gXSemiAxisLength = gXSemiAxisLength;
		rCurve->gYSemiAxisLength = gYSemiAxisLength;
		rCurve->gOffsetRotation = gOffsetRotation;
		rCurve->gCosOfsRot = gCosOfsRot;
		rCurve->gSinOfsRot = gSinOfsRot;
		rCurve->gCCW = gCCW;
		rCurve->SetAngleDomain(angle, gEndAngle);
		rCurve->SetDomain(DomainStart(), u);
	}

	if (lCurve) {
		lCurve->gCenter = gCenter;
		lCurve->gXSemiAxisLength = gXSemiAxisLength;
		lCurve->gYSemiAxisLength = gYSemiAxisLength;
		lCurve->gOffsetRotation = gOffsetRotation;
		lCurve->gCosOfsRot = gCosOfsRot;
		lCurve->gSinOfsRot = gSinOfsRot;
		lCurve->gCCW = gCCW;
		lCurve->SetAngleDomain(gStartAngle, angle);
		lCurve->SetDomain(u, DomainEnd());
	}
	return G_NO_ERROR;
}

// flats (tessellates) the curve specifying a max error/variation (chordal distance)
GError GEllipseCurve2D::Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
								const GBool IncludeLastPoint) const {

	if (MaxDeviation <= 0)
		return G_INVALID_PARAMETER;

	// first find major semi-axis length
	GReal r = GMath::Max(gXSemiAxisLength, gYSemiAxisLength);

	// MaxDeviation is a squared chordal distance, we must report to this value to a linear chordal distance
	GReal flatness = GMath::Sqrt(MaxDeviation);

	// now calculate the number of segments to produce (number of times we have to subdivide angle) that
	// permit to have a squared chordal distance less than MaxDeviation
	GReal beta, dev = GMath::Clamp(flatness, G_EPSILON, r - (G_EPSILON * r));

	if (gCCW) {
		if (gStartAngle < gEndAngle)
			beta = gEndAngle - gStartAngle;
		else
			beta = (GReal)G_2PI - gStartAngle + gEndAngle;
	}
	// cw
	else {
		if (gStartAngle < gEndAngle)
			beta = (GReal)G_2PI - gEndAngle + gStartAngle;
		else
			beta = gStartAngle - gEndAngle;
	}

	GUInt32 n = 1;
	GReal n1 = beta / (2 * GMath::Acos(1 - dev / r));
	if (n1 > 1 && n1 >= (beta * (GReal)G_ONE_OVER_PI)) {
		if (n1 > n)
			n = (GUInt32)GMath::Ceil(n1);
	}
	G_ASSERT(n >= 1);

	GReal aOverb = gXSemiAxisLength / gYSemiAxisLength;
	GReal bOvera = gYSemiAxisLength / gXSemiAxisLength;
	GReal deltaAngle = beta / (GReal)n;
	GReal cosDelta, sinDelta;
	GPoint2 w;

	if (gCCW) {
		cosDelta = GMath::Cos(deltaAngle);
		sinDelta = GMath::Sin(deltaAngle);
	}
	else {
		cosDelta = GMath::Cos(-deltaAngle);
		sinDelta = GMath::Sin(-deltaAngle);
	}

	// push first point
	GPoint2 p(gXSemiAxisLength * GMath::Cos(gStartAngle), gYSemiAxisLength * GMath::Sin(gStartAngle));
	GPoint2 q(gCosOfsRot * p[G_X] - gSinOfsRot * p[G_Y] + gCenter[G_X], gSinOfsRot * p[G_X] + gCosOfsRot * p[G_Y] + gCenter[G_Y]);
	Contour.push_back(q);

	// generate the other points
	for (GUInt32 i = 0; i < n - 1; i++) {
		w.Set(p[G_X] * cosDelta - aOverb * p[G_Y] * sinDelta, bOvera * p[G_X] * sinDelta + p[G_Y] * cosDelta);
		q.Set(gCosOfsRot * w[G_X] - gSinOfsRot * w[G_Y] + gCenter[G_X], gSinOfsRot * w[G_X] + gCosOfsRot * w[G_Y] + gCenter[G_Y]);
		Contour.push_back(q);
		p = w;
	}
	// check for last point
	if (IncludeLastPoint) {
		w.Set(p[G_X] * cosDelta - aOverb * p[G_Y] * sinDelta, bOvera * p[G_X] * sinDelta + p[G_Y] * cosDelta);
		q.Set(gCosOfsRot * w[G_X] - gSinOfsRot * w[G_Y] + gCenter[G_X], gSinOfsRot * w[G_X] + gCosOfsRot * w[G_Y] + gCenter[G_Y]);
		Contour.push_back(q);
	}
	return G_NO_ERROR;
}

// get max variation (squared chordal distance) in the domain range
GReal GEllipseCurve2D::Variation() const {

	GPoint2 midP = (StartPoint() + EndPoint()) / (GReal)2;
	GReal midAngle = MapAngle(DomainStart() + Domain().Length() / (GReal)2);
	GPoint2 midEllipse = EvaluateByAngle(midAngle);
	return DistanceSquared(midP, midEllipse);
}

// cloning function
GError GEllipseCurve2D::BaseClone(const GElement& Source) {

	const GEllipseCurve2D& k = (const GEllipseCurve2D&)Source;

	gCenter = k.gCenter;
	gXSemiAxisLength = k.gXSemiAxisLength;
	gYSemiAxisLength = k.gYSemiAxisLength;
	gOffsetRotation = k.gOffsetRotation;
	gCosOfsRot = k.gCosOfsRot;
	gSinOfsRot = k.gSinOfsRot;
	gCCW = k.gCCW;
	gStartAngle = k.gStartAngle;
	gEndAngle = k.gEndAngle;
	return GCurve2D::BaseClone(Source);
}

void GEllipseCurve2D::Translate(const GVector2& Translation) {

	gCenter += Translation;
}

void GEllipseCurve2D::Rotate(const GPoint2& Pivot, const GReal RadAmount) {

	GMatrix33 rot, preTrans, postTrans, matrix;

	TranslationToMatrix(preTrans, -Pivot);
	TranslationToMatrix(postTrans, (const GVector2&)Pivot);
	RotationToMatrix(rot, RadAmount);
	matrix = postTrans * (rot * preTrans);

	// rotate center
	GPoint2 q = matrix * gCenter;
	gCenter = q;
	// add angle to offset rotation
	gOffsetRotation += RadAmount;
	gCosOfsRot = GMath::Cos(gOffsetRotation);
	gSinOfsRot = GMath::Sin(gOffsetRotation);
}

void GEllipseCurve2D::Scale(const GPoint2& Pivot, const GReal XScaleAmount, const GReal YScaleAmount) {

	GMatrix33 scale, preTrans, postTrans, matrix;

	TranslationToMatrix(preTrans, -Pivot);
	TranslationToMatrix(postTrans, (const GVector2&)Pivot);
	ScaleToMatrix(scale, GVect<GReal, 2>(XScaleAmount, YScaleAmount));
	matrix = postTrans * (scale * preTrans);

	XForm(matrix, G_FALSE);
}

GPoint2 GEllipseCurve2D::EvaluateByAngle(const GReal Angle) const {

	GPoint2 p(gXSemiAxisLength * GMath::Cos(Angle), gYSemiAxisLength * GMath::Sin(Angle));
	return GPoint2((p[G_X] * gCosOfsRot - gSinOfsRot * p[G_Y]) + gCenter[G_X], (p[G_X] * gSinOfsRot + p[G_Y] * gCosOfsRot) + gCenter[G_Y]);
}

void GEllipseCurve2D::XForm(const GMatrix23& Matrix) {

	GPoint2 p0, p1, p2, p3, p4;
	GMatrix<GReal, 5, 5> M, invM;
	GVect<GReal, 5> rhs, solution;
	GBool fullRank, newCCW;
	GReal detM, ofsRot;
	GReal angMin = gStartAngle, angMax = gEndAngle, step1, step2, ang1, ang2;

	/*
		We must ensure that under specified Matrix ellipse must remain and ellipse, so we must compute
		all new parameters.
		First of all we use conic equation in the form ax^2 + 2bxy + dy^2 + 2cx + 2ey + 1 = 0, and we
		impose the passage through 5 points that are granted to be on transformed ellipse (chosen in a way
		that ensures system solving stability).  So we solve the system and get a, b, d, c, e parameters.
		Now we have to port conic form into used one (semi axes lengths, center, offset rotation, start angle,
		end angle and cw/ccw direction.
		To do this, we calculate Grahm matrix associated to conic (ellipse is a quadratic form), then we
		diagonalize it using spectral decomposition.
		Eigenvalues give us new semi axes lengths, and eigenvectors give us offset rotation. Start angle and end
		angle are calculated from transformed endpoints and new semi axes lengths.
		Cw/ccw direction is calculated imposing that spanned angle (from start angle to end angle) will remain the
		same in the sense of large/small arc.
	*/

	GPoint2 oldTransfCenter = Matrix * gCenter;
	gCenter.Set(0, 0);

	// calculate end points
	p0 = Matrix * StartPoint();
	p1 = Matrix * EndPoint();
	// calculate other 3 points
	if (gStartAngle > gEndAngle) {
		GReal swap = angMin;
		angMin = angMax;
		angMax = swap;
	}
	if (LargeArc()) {
		step1 = (angMax - angMin) / (GReal)3;
		step2 = ((GReal)G_2PI - angMax + angMin) / (GReal)2;
		p2 = Matrix * EvaluateByAngle(angMin + step1);
		p3 = Matrix * EvaluateByAngle(angMin + 2 * step1);
		p4 = Matrix * EvaluateByAngle(angMax + step2);
	}
	else {
		step1 = (angMax - angMin) / (GReal)2;
		step2 = ((GReal)G_2PI - angMax + angMin) / (GReal)3;
		p2 = Matrix * EvaluateByAngle(angMin + step1);
		p3 = Matrix * EvaluateByAngle(angMin + step2);
		p4 = Matrix * EvaluateByAngle(angMax + 2 * step2);
	}
	// build system matrix
	M[0][0] = p0[G_X] * p0[G_X];
	M[0][1] = (GReal)2 * (p0[G_X] * p0[G_Y]);
	M[0][2] = p0[G_Y] * p0[G_Y];
	M[0][3] = (GReal)2 * p0[G_X];
	M[0][4] = (GReal)2 * p0[G_Y];
	M[1][0] = p1[G_X] * p1[G_X];
	M[1][1] = (GReal)2 * (p1[G_X] * p1[G_Y]);
	M[1][2] = p1[G_Y] * p1[G_Y];
	M[1][3] = (GReal)2 * p1[G_X];
	M[1][4] = (GReal)2 * p1[G_Y];
	M[2][0] = p2[G_X] * p2[G_X];
	M[2][1] = (GReal)2 * (p2[G_X] * p2[G_Y]);
	M[2][2] = p2[G_Y] * p2[G_Y];
	M[2][3] = (GReal)2 * p2[G_X];
	M[2][4] = (GReal)2 * p2[G_Y];
	M[3][0] = p3[G_X] * p3[G_X];
	M[3][1] = (GReal)2 * (p3[G_X] * p3[G_Y]);
	M[3][2] = p3[G_Y] * p3[G_Y];
	M[3][3] = (GReal)2 * p3[G_X];
	M[3][4] = (GReal)2 * p3[G_Y];
	M[4][0] = p4[G_X] * p4[G_X];
	M[4][1] = (GReal)2 * (p4[G_X] * p4[G_Y]);
	M[4][2] = p4[G_Y] * p4[G_Y];
	M[4][3] = (GReal)2 * p4[G_X];
	M[4][4] = (GReal)2 * p4[G_Y];
	// build rhs vector
	rhs.Fill(-1);
	// invert system matrix
	fullRank = InvertFull_GJ(invM, M, detM);
	if (fullRank) {
		
		GMatrix33 grahm;
		GVector2 eValue1, eValue2, eValue3;
		GVector3 eVector1, eVector2, eVector3;
		
		// solve the system and get conic coefficients
		solution = invM * rhs;
		// build associated Grahm matrix
		grahm[0][0] = solution[0]; grahm[0][1] = solution[1]; grahm[0][2] = solution[3];
		grahm[1][0] = solution[1]; grahm[1][1] = solution[2]; grahm[1][2] = solution[4];
		grahm[2][0] = solution[3]; grahm[2][1] = solution[4]; grahm[2][2] = 1;
		// diagonalize Grahm matrix using spectral decomposition
		GEigen eigenSolver(grahm, G_TRUE, eValue1, eValue2, eValue3, eVector1, eVector2, eVector3, G_FALSE);
		// calculate new semi axes lengths
		GReal newXSemiLen = GMath::Sqrt((GReal)1 / GMath::Abs(eValue1[G_X]));
		GReal newYSemiLen = GMath::Sqrt((GReal)1 / GMath::Abs(eValue2[G_X]));
		// calculate offset rotation (relative to x-axis)
		ofsRot = GMath::Atan2(eVector1[G_Y], eVector1[G_X]);
		// port transformed end points to new ellipse coordinate system (so the new ellipse is centered
		// at (0, 0) and axes aligned
		GReal cosOfsRot = GMath::Cos(ofsRot);
		GReal sinOfsRot = GMath::Sin(ofsRot);
		GPoint2 q0(cosOfsRot * p0[G_X] + sinOfsRot * p0[G_Y], -sinOfsRot * p0[G_X] + cosOfsRot * p0[G_Y]);
		GPoint2 q1(cosOfsRot * p1[G_X] + sinOfsRot * p1[G_Y], -sinOfsRot * p1[G_X] + cosOfsRot * p1[G_Y]);
		// calculate new start/end angles
		ang1 = GMath::Atan2(q0[G_Y] / newYSemiLen, q0[G_X] / newXSemiLen);
		ang2 = GMath::Atan2(q1[G_Y] / newYSemiLen, q1[G_X] / newXSemiLen);
		// calculate direction
		GBool lArc = GEllipseCurve2D::IsLargeArc(GEllipseCurve2D::FixAngle(ang1), GEllipseCurve2D::FixAngle(ang2), gCCW);
		if (lArc != LargeArc())
			newCCW = !gCCW;
		else
			newCCW = gCCW;
		// set the brand new ellipse!
		//SetEllipse(c, newXSemiLen, newYSemiLen, ofsRot, ang1, ang2, newCCW);
		SetEllipse(oldTransfCenter, newXSemiLen, newYSemiLen, ofsRot, ang1, ang2, newCCW);
	}
	else
		// in this case we can only ensure first and last point positions
		SetEllipse(p0, p1, gXSemiAxisLength, gYSemiAxisLength, gOffsetRotation, LargeArc(), gCCW);
}

void GEllipseCurve2D::XForm(const GMatrix33& Matrix, const GBool DoProjection) {

	GPoint2 p0, p1, p2, p3, p4;
	GMatrix<GReal, 5, 5> M, invM;
	GVect<GReal, 5> rhs, solution;
	GBool fullRank, newCCW;
	GReal detM, ofsRot;
	GReal angMin = gStartAngle, angMax = gEndAngle, step1, step2, ang1, ang2;

	/*
		We must ensure that under specified Matrix ellipse must remain and ellipse, so we must compute
		all new parameters.
		First of all we use conic equation in the form ax^2 + 2bxy + dy^2 + 2cx + 2ey + 1 = 0, and we
		impose the passage through 5 points that are granted to be on transformed ellipse (chosen in a way
		that ensures system solving stability).  So we solve the system and get a, b, d, c, e parameters.
		Now we have to port conic form into used one (semi axes lengths, center, offset rotation, start angle,
		end angle and cw/ccw direction.
		To do this, we calculate Grahm matrix associated to conic (ellipse is a quadratic form), then we
		diagonalize it using spectral decomposition.
		Eigenvalues give us new semi axes lengths, and eigenvectors give us offset rotation. Start angle and end
		angle are calculated from transformed endpoints and new semi axes lengths.
		Cw/ccw direction is calculated imposing that spanned angle (from start angle to end angle) will remain the
		same in the sense of large/small arc.
	*/

	GPoint2 oldTransfCenter = Matrix * gCenter;
	gCenter.Set(0, 0);

	if (!DoProjection) {
		// calculate end points
		p0 = Matrix * StartPoint();
		p1 = Matrix * EndPoint();
		// calculate other 3 points
		if (gStartAngle > gEndAngle) {
			GReal swap = angMin;
			angMin = angMax;
			angMax = swap;
		}
		if (LargeArc()) {
			step1 = (angMax - angMin) / (GReal)3;
			step2 = ((GReal)G_2PI - angMax + angMin) / (GReal)2;
			p2 = Matrix * EvaluateByAngle(angMin + step1);
			p3 = Matrix * EvaluateByAngle(angMin + 2 * step1);
			p4 = Matrix * EvaluateByAngle(angMax + step2);
		}
		else {
			step1 = (angMax - angMin) / (GReal)2;
			step2 = ((GReal)G_2PI - angMax + angMin) / (GReal)3;
			p2 = Matrix * EvaluateByAngle(angMin + step1);
			p3 = Matrix * EvaluateByAngle(angMin + step2);
			p4 = Matrix * EvaluateByAngle(angMax + 2 * step2);
		}
	}
	else {
		GPoint3 prjP(0, 0, 1), prjW;
		#define PRJ_TRANSFORM(p2d, res2D) \
			prjP.Set(p2d[G_X], p2d[G_Y]); \
			prjW = Matrix * prjP; \
			if (GMath::Abs(prjW[G_Z]) <= 2 * G_EPSILON) \
				res2D.Set(prjW[G_X], prjW[G_Y]); \
			else \
				res2D.Set(prjW[G_X] / prjW[G_Z], prjW[G_Y] / prjW[G_Z]);

		// calculate end points
		p0 = StartPoint();
		PRJ_TRANSFORM(p0, p0);
		p1 = StartPoint();
		PRJ_TRANSFORM(p1, p1);

		// calculate other 3 points
		if (gStartAngle > gEndAngle) {
			GReal swap = angMin;
			angMin = angMax;
			angMax = swap;
		}
		if (LargeArc()) {
			step1 = (angMax - angMin) / (GReal)3;
			step2 = ((GReal)G_2PI - angMax + angMin) / (GReal)2;
			p2 = EvaluateByAngle(angMin + step1);
			PRJ_TRANSFORM(p2, p2);
			p3 = EvaluateByAngle(angMin + 2 * step1);
			PRJ_TRANSFORM(p3, p3);
			p4 = EvaluateByAngle(angMax + step2);
			PRJ_TRANSFORM(p4, p4);
		}
		else {
			step1 = (angMax - angMin) / (GReal)2;
			step2 = ((GReal)G_2PI - angMax + angMin) / (GReal)3;
			p2 = EvaluateByAngle(angMin + step1);
			PRJ_TRANSFORM(p2, p2);
			p3 = EvaluateByAngle(angMin + step2);
			PRJ_TRANSFORM(p3, p3);
			p4 = EvaluateByAngle(angMax + 2 * step2);
			PRJ_TRANSFORM(p4, p4);
		}
	}

	// build system matrix
	M[0][0] = p0[G_X] * p0[G_X];
	M[0][1] = (GReal)2 * (p0[G_X] * p0[G_Y]);
	M[0][2] = p0[G_Y] * p0[G_Y];
	M[0][3] = (GReal)2 * p0[G_X];
	M[0][4] = (GReal)2 * p0[G_Y];
	M[1][0] = p1[G_X] * p1[G_X];
	M[1][1] = (GReal)2 * (p1[G_X] * p1[G_Y]);
	M[1][2] = p1[G_Y] * p1[G_Y];
	M[1][3] = (GReal)2 * p1[G_X];
	M[1][4] = (GReal)2 * p1[G_Y];
	M[2][0] = p2[G_X] * p2[G_X];
	M[2][1] = (GReal)2 * (p2[G_X] * p2[G_Y]);
	M[2][2] = p2[G_Y] * p2[G_Y];
	M[2][3] = (GReal)2 * p2[G_X];
	M[2][4] = (GReal)2 * p2[G_Y];
	M[3][0] = p3[G_X] * p3[G_X];
	M[3][1] = (GReal)2 * (p3[G_X] * p3[G_Y]);
	M[3][2] = p3[G_Y] * p3[G_Y];
	M[3][3] = (GReal)2 * p3[G_X];
	M[3][4] = (GReal)2 * p3[G_Y];
	M[4][0] = p4[G_X] * p4[G_X];
	M[4][1] = (GReal)2 * (p4[G_X] * p4[G_Y]);
	M[4][2] = p4[G_Y] * p4[G_Y];
	M[4][3] = (GReal)2 * p4[G_X];
	M[4][4] = (GReal)2 * p4[G_Y];
	// build rhs vector
	rhs.Fill(-1);
	// invert system matrix
	fullRank = InvertFull_GJ(invM, M, detM);
	if (fullRank) {

		GMatrix33 grahm;
		GVector2 eValue1, eValue2, eValue3;
		GVector3 eVector1, eVector2, eVector3;

		// solve the system and get conic coefficients
		solution = invM * rhs;
		// build associated Grahm matrix
		grahm[0][0] = solution[0]; grahm[0][1] = solution[1]; grahm[0][2] = solution[3];
		grahm[1][0] = solution[1]; grahm[1][1] = solution[2]; grahm[1][2] = solution[4];
		grahm[2][0] = solution[3]; grahm[2][1] = solution[4]; grahm[2][2] = 1;
		// diagonalize Grahm matrix using spectral decomposition
		GEigen eigenSolver(grahm, G_TRUE, eValue1, eValue2, eValue3, eVector1, eVector2, eVector3, G_FALSE);
		// calculate new semi axes lengths
		GReal newXSemiLen = GMath::Sqrt((GReal)1 / GMath::Abs(eValue1[G_X]));
		GReal newYSemiLen = GMath::Sqrt((GReal)1 / GMath::Abs(eValue2[G_X]));
		// calculate offset rotation (relative to x-axis)
		ofsRot = GMath::Atan2(eVector1[G_Y], eVector1[G_X]);
		// port transformed end points to new ellipse coordinate system (so the new ellipse is centered
		// at (0, 0) and axes aligned
		GReal cosOfsRot = GMath::Cos(ofsRot);
		GReal sinOfsRot = GMath::Sin(ofsRot);
		GPoint2 q0(cosOfsRot * p0[G_X] + sinOfsRot * p0[G_Y], -sinOfsRot * p0[G_X] + cosOfsRot * p0[G_Y]);
		GPoint2 q1(cosOfsRot * p1[G_X] + sinOfsRot * p1[G_Y], -sinOfsRot * p1[G_X] + cosOfsRot * p1[G_Y]);
		// calculate new start/end angles
		ang1 = GMath::Atan2(q0[G_Y] / newYSemiLen, q0[G_X] / newXSemiLen);
		ang2 = GMath::Atan2(q1[G_Y] / newYSemiLen, q1[G_X] / newXSemiLen);
		// calculate direction
		GBool lArc = GEllipseCurve2D::IsLargeArc(GEllipseCurve2D::FixAngle(ang1), GEllipseCurve2D::FixAngle(ang2), gCCW);
		if (lArc != LargeArc())
			newCCW = !gCCW;
		else
			newCCW = gCCW;
		// set the brand new ellipse!
		//SetEllipse(c, newXSemiLen, newYSemiLen,	ofsRot, ang1, ang2, newCCW);
		SetEllipse(oldTransfCenter, newXSemiLen, newYSemiLen, ofsRot, ang1, ang2, newCCW);
	}
	else
		// in this case we can only ensure first and last point positions
		SetEllipse(p0, p1, gXSemiAxisLength, gYSemiAxisLength, gOffsetRotation, LargeArc(), gCCW);
}

};	// end namespace Amanith
