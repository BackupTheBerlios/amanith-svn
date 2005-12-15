/****************************************************************************
** $file: amanith/src/rendering/gopenglstroke.cpp   0.2.0.0   edited Dec, 12 2005
**
** OpenGL based draw board stroke functions implementation.
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

#include "amanith/rendering/gopenglboard.h"

/*!
	\file gopenglstroke.cpp
	\brief OpenGL based draw board stroke functions implementation file.
*/

namespace Amanith {


// *********************************************************************
//                             GOpenGLBoard
// *********************************************************************

void GOpenGLBoard::DrawGLCapsLine(const GBool DoStartCap, const GCapStyle StartCapStyle,
								  const GBool DoEndCap, const GCapStyle EndCapStyle,
								  const GPoint2& P0, const GPoint2& P1, const GReal Thickness, const GReal RoundAuxCoef) {

	GVector2 dirSeg = P1 - P0;
	// perpendicular direction in CCW respect to dirSeg
	GVector2 perpSeg(-dirSeg[G_Y], dirSeg[G_X]);
	perpSeg *= (Thickness / perpSeg.Length());


	glBegin(GL_POLYGON);

	// calculate fundamentals points
	GPoint2 j0 = P0 + perpSeg;
	GPoint2 j1 = P0 - perpSeg;
	GPoint2 l0 = P1 + perpSeg;
	GPoint2 l1 = P1 - perpSeg;
	GReal l;

	if (StartCapStyle == G_SQUARE_CAP || EndCapStyle == G_SQUARE_CAP)
		l = dirSeg.Normalize() + Thickness;

	// end cap
	if (!DoEndCap) {
		// behavior is like we would have a butt cap
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(l0.Data());
			glVertex2dv(l1.Data());
		#else
			glVertex2fv(l0.Data());
			glVertex2fv(l1.Data());
		#endif
	}
	else {
		GPoint2 a, b;
		switch (EndCapStyle) {

			case G_BUTT_CAP:
				#ifdef DOUBLE_REAL_TYPE
					glVertex2dv(l0.Data());
					glVertex2dv(l1.Data());
				#else
					glVertex2fv(l0.Data());
					glVertex2fv(l1.Data());
				#endif
				break;

			case G_ROUND_CAP:
				DrawGLCircleSlice(P1, RoundAuxCoef, l0, l1, (GReal)G_PI, G_FALSE);
				break;

			case G_SQUARE_CAP:
				a = j0 + l * dirSeg;
				b = j1 + l * dirSeg;
				#ifdef DOUBLE_REAL_TYPE
					glVertex2dv(a.Data());
					glVertex2dv(b.Data());
				#else
					glVertex2fv(a.Data());
					glVertex2fv(b.Data());
				#endif
				break;
		}
	}


	// start cap
	if (!DoStartCap) {
		// behavior is like we would have a butt cap
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j1.Data());
			glVertex2dv(j0.Data());
		#else
			glVertex2fv(j1.Data());
			glVertex2fv(j0.Data());
		#endif
	}
	else {
		GPoint2 a, b;

		switch (StartCapStyle) {

			case G_BUTT_CAP:
				#ifdef DOUBLE_REAL_TYPE
					glVertex2dv(j1.Data());
					glVertex2dv(j0.Data());
				#else
					glVertex2fv(j1.Data());
					glVertex2fv(j0.Data());
				#endif
				break;

			case G_ROUND_CAP:
				DrawGLCircleSlice(P0, RoundAuxCoef, j1, j0, (GReal)G_PI, G_FALSE);
				break;

			case G_SQUARE_CAP:
				a = l0 - l * dirSeg;
				b = l1 - l * dirSeg;
				#ifdef DOUBLE_REAL_TYPE
					glVertex2dv(b.Data());
					glVertex2dv(a.Data());
				#else
					glVertex2fv(b.Data());
					glVertex2fv(a.Data());
				#endif
				break;
		}
	}
	glEnd();
}

void GOpenGLBoard::DrawGLJoinLine(const GJoinStyle JoinStyle, const GReal MiterLimitMulThickness,
								  const GPoint2& Previous, const GPoint2& P0, const GPoint2& P1,
								  const GReal Thickness, const GReal RoundAuxCoef) {

	GVector2 dirPrev = P0 - Previous;
	GVector2 dirSeg = P1 - P0;

	// calculate perpendicular vectors (taking care of Thickness of the line)
	GVector2 normPerpPrev(dirPrev[G_Y], -dirPrev[G_X]);
	normPerpPrev.Normalize();
	GVector2 perpPrev(Thickness * normPerpPrev[G_X], Thickness * normPerpPrev[G_Y]);

	GVector2 normPerpSeg(dirSeg[G_Y], -dirSeg[G_X]);
	normPerpSeg.Normalize();
	GVector2 perpSeg(Thickness * normPerpSeg[G_X], Thickness * normPerpSeg[G_Y]);

	// make sure the normal vectors are pointing outwards
	if (Cross(perpPrev, perpSeg) < 0) {
		perpPrev = -perpPrev;
		perpSeg = -perpSeg;
	}


	glBegin(GL_POLYGON);

	// calculate fundamentals points
	GPoint2 j0 = P0 + perpPrev;
	GPoint2 j1 = P0 + perpSeg;
	GPoint2 l0 = j1 + dirSeg;
	GPoint2 l2 = P0 - perpSeg;
	GPoint2 l1 = l2 + dirSeg;

	// round join
	if (JoinStyle == G_ROUND_JOIN) {
		GReal roundJoinAngle = GMath::Acos(Dot(normPerpPrev, normPerpSeg));
		DrawGLCircleSlice(P0, RoundAuxCoef, j0, j1, roundJoinAngle, CounterClockWise(P0, j0, j1));
	}
	else
	// miter join
	if (JoinStyle == G_MITER_JOIN) {

		GRay2 prevRay, segRay;
		GUInt32 intFlags;
		GReal intParam[2];
		GBool intFound;

		GPoint2 o = Previous + perpPrev;
		prevRay.SetOrigin(o);
		prevRay.SetDirection(dirPrev);

		segRay.SetOrigin(l0);
		segRay.SetDirection(-dirSeg);

		intFound = Intersect(prevRay, segRay, intFlags, intParam);
		if (intFound) {

			GPoint2 intPoint = prevRay.Origin() + (prevRay.Direction() * intParam[0]);
			GVector2 intDir = intPoint - P0;
			GReal intDirLen = intDir.Normalize();

			if (intDirLen > MiterLimitMulThickness)
				intPoint = P0 + MiterLimitMulThickness * intDir;

			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(j0.Data());
				glVertex2dv(intPoint.Data());
				glVertex2dv(j1.Data());
			#else
				glVertex2fv(j0.Data());
				glVertex2fv(intPoint.Data());
				glVertex2fv(j1.Data());
			#endif
		}
	}
	// bevel join
	else {
		G_ASSERT(JoinStyle == G_BEVEL_JOIN);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j0.Data());
			glVertex2dv(j1.Data());
		#else
			glVertex2fv(j0.Data());
			glVertex2fv(j1.Data());
		#endif
	}

	#ifdef DOUBLE_REAL_TYPE
		glVertex2dv(l0.Data());
		glVertex2dv(l1.Data());
		glVertex2dv(l2.Data());
	#else
		glVertex2fv(l0.Data());
		glVertex2fv(l1.Data());
		glVertex2fv(l2.Data());
	#endif

	glEnd();
}

void GOpenGLBoard::DrawGLJoinLineCap(const GJoinStyle JoinStyle, const GReal MiterLimitMulThickness,
									 const GPoint2& Previous, const GPoint2& P0, const GPoint2& P1,
									 const GReal Thickness, const GCapStyle EndCapStyle, const GReal RoundAuxCoef) {


	GVector2 dirPrev = P0 - Previous;
	GVector2 dirSeg = P1 - P0;

	// calculate perpendicular vectors (taking care of Thickness of the line)
	GVector2 normPerpPrev(dirPrev[G_Y], -dirPrev[G_X]);
	normPerpPrev.Normalize();
	GVector2 perpPrev(Thickness * normPerpPrev[G_X], Thickness * normPerpPrev[G_Y]);

	GVector2 normPerpSeg(dirSeg[G_Y], -dirSeg[G_X]);
	normPerpSeg.Normalize();
	GVector2 perpSeg(Thickness * normPerpSeg[G_X], Thickness * normPerpSeg[G_Y]);

	// make sure the normal vectors are pointing outwards
	if (Cross(perpPrev, perpSeg) < 0) {
		perpPrev = -perpPrev;
		perpSeg = -perpSeg;
	}


	glBegin(GL_POLYGON);

	// calculate fundamentals points
	GPoint2 j0 = P0 + perpPrev;
	GPoint2 j1 = P0 + perpSeg;
	GPoint2 l0 = j1 + dirSeg;
	GPoint2 l2 = P0 - perpSeg;
	GPoint2 l1 = l2 + dirSeg;

	// round join
	if (JoinStyle == G_ROUND_JOIN) {
		GReal roundJoinAngle = GMath::Acos(Dot(normPerpPrev, normPerpSeg));
		DrawGLCircleSlice(P0, RoundAuxCoef, j0, j1, roundJoinAngle, CounterClockWise(P0, j0, j1));
	}
	else
	// miter join
	if (JoinStyle == G_MITER_JOIN) {

		GRay2 prevRay, segRay;
		GUInt32 intFlags;
		GReal intParam[2];
		GBool intFound;

		GPoint2 o = Previous + perpPrev;
		prevRay.SetOrigin(o);
		prevRay.SetDirection(dirPrev);

		segRay.SetOrigin(l0);
		segRay.SetDirection(-dirSeg);

		intFound = Intersect(prevRay, segRay, intFlags, intParam);
		if (intFound) {

			GPoint2 intPoint = prevRay.Origin() + (prevRay.Direction() * intParam[0]);
			GVector2 intDir = intPoint - P0;
			GReal intDirLen = intDir.Normalize();

			if (intDirLen > MiterLimitMulThickness)
				intPoint = P0 + MiterLimitMulThickness * intDir;

			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(j0.Data());
				glVertex2dv(intPoint.Data());
				glVertex2dv(j1.Data());
			#else
				glVertex2fv(j0.Data());
				glVertex2fv(intPoint.Data());
				glVertex2fv(j1.Data());
			#endif
		}
	}
	// bevel join
	else {
		G_ASSERT(JoinStyle == G_BEVEL_JOIN);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j0.Data());
			glVertex2dv(j1.Data());
		#else
			glVertex2fv(j0.Data());
			glVertex2fv(j1.Data());
		#endif
	}

	GReal l;
	switch (EndCapStyle) {

		case G_BUTT_CAP:
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(l0.Data());
				glVertex2dv(l1.Data());
				glVertex2dv(l2.Data());
			#else
				glVertex2fv(l0.Data());
				glVertex2fv(l1.Data());
				glVertex2fv(l2.Data());
			#endif
			break;

		case G_ROUND_CAP:
			DrawGLCircleSlice(P1, RoundAuxCoef, l0, l1, (GReal)G_PI, CounterClockWise(P0, l0, l1));
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(l2.Data());
			#else
				glVertex2fv(l2.Data());
			#endif
			break;

		case G_SQUARE_CAP:
			l = dirSeg.Normalize() + Thickness;
			l0 = j1 + l * dirSeg;
			l1 = l2 + l * dirSeg;
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(l0.Data());
				glVertex2dv(l1.Data());
				glVertex2dv(l2.Data());
			#else
				glVertex2fv(l0.Data());
				glVertex2fv(l1.Data());
				glVertex2fv(l2.Data());
			#endif
			break;
	}
	glEnd();
}

void GOpenGLBoard::DrawGLCircleSlice(const GPoint2& Center, const GReal RoundAuxCoef, const GPoint2& Start,
									 const GPoint2& End, const GReal SpanAngle, const GBool CCW) {

	GReal deltaAngle;
	GUInt32 n = 3;
	GReal n1 = SpanAngle * RoundAuxCoef;
	if (n1 > 3)
		n = (GUInt32)n1;
	G_ASSERT(n >= 3);

	// generate points
	if (CCW)
		deltaAngle = (SpanAngle / n);
	else
		deltaAngle = (-SpanAngle / n);
	GReal cosDelta = GMath::Cos(deltaAngle);
	GReal sinDelta = GMath::Sin(deltaAngle);

	GPoint2 p(Start - Center), q, r;
	// draw cap

#ifdef DOUBLE_REAL_TYPE
	glVertex2dv(Start.Data());
#else
	glVertex2fv(Start.Data());
#endif
	for (GUInt32 i = 0; i < n - 1; ++i) {
		q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
		r = q + Center;
#ifdef DOUBLE_REAL_TYPE
		glVertex2dv(r.Data());
#else
		glVertex2fv(r.Data());
#endif
		p = q;
	}
#ifdef DOUBLE_REAL_TYPE
	glVertex2dv(End.Data());
#else
	glVertex2fv(End.Data());
#endif
}

void GOpenGLBoard::DrawGLJoin(const GPoint2& JoinCenter, const GVector2& InDirection, const GReal InDistance,
							  const GVector2& OutDirection, const GReal OutDistance, const GJoinStyle JoinStyle,
							  const GReal MiterLimitMulThickness, const GCapStyle StartCapStyle, const GCapStyle EndCapStyle,
							  const GReal Thickness, const GReal RoundAuxCoef) {

	if (InDistance < G_EPSILON && OutDistance < G_EPSILON)
		return;

	GVector2 normPerpInExt(InDirection[G_Y], -InDirection[G_X]);
	GVector2 normPerpOutExt(OutDirection[G_Y], -OutDirection[G_X]);

	// make sure the normal vectors are pointing outwards
	if (Cross(normPerpInExt, normPerpOutExt) < 0) {
		  normPerpInExt = -normPerpInExt;
		  normPerpOutExt = -normPerpOutExt;
	}

	GVector2 perpInExt(normPerpInExt[G_X] * Thickness, normPerpInExt[G_Y] * Thickness);
	GVector2 perpOutExt(normPerpOutExt[G_X] * Thickness, normPerpOutExt[G_Y] * Thickness);

	GVector2 vTmp = (normPerpInExt + normPerpOutExt) * (GReal)0.5;
	vTmp.Normalize();

	GPoint2 j0 = JoinCenter + perpInExt;
	GPoint2 j1 = JoinCenter + perpOutExt;
	GPoint2 t0 = JoinCenter - perpInExt;
	GPoint2 t1 = JoinCenter - perpOutExt;
	GPoint2 v0 = t0 - InDistance * InDirection;
	GPoint2 v1 = t1 + OutDistance * OutDirection;
	GPoint2 k0 = j0 - InDistance * InDirection;
	GPoint2 k1 = j1 + OutDistance * OutDirection;

	// we must find vc using t0 and t1 that are length independent
	GUInt32 intFlags;
	GReal intParam[2];
	GRay2 r0(t0, -InDirection);
	GRay2 r1(t1, OutDirection);
	GBool intFound = Intersect(r0, r1, intFlags, intParam);
	// this could be the case of 2 collinear and opposite directions
	if (!intFound)
		return;

	// calculate fundamentals points
	GPoint2 a, b, jc;

	GPoint2 vc = t0 - InDirection * intParam[0];
	GPoint2 w0 = vc + ((GReal)2 * perpInExt);
	GPoint2 w1 = vc + ((GReal)2 * perpOutExt);

	GReal halfRoundJoinAngle = GMath::Acos(Dot(normPerpInExt, normPerpOutExt)) * (GReal)0.5;

	// handle degenerative distance cases
	GVector2 n = v0 - vc;
	if (InDistance < G_EPSILON || Dot(n, InDirection) > 0) {
		k0 = w0;
		v0 = vc;
	}
	GPoint2 s0 = (k0 + v0) * (GReal)0.5;
	// calculate ccw/cw direction
	GBool ccw = CounterClockWise(j0, t0, s0);

	glBegin(GL_POLYGON);
	// start cap
	switch (StartCapStyle) {

		case G_BUTT_CAP:
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(v0.Data());
				glVertex2dv(k0.Data());
			#else
				glVertex2fv(v0.Data());
				glVertex2fv(k0.Data());
			#endif
			break;

		case G_ROUND_CAP:
			DrawGLCircleSlice(s0, RoundAuxCoef, v0, k0, (GReal)G_PI, ccw);
			break;

		case G_SQUARE_CAP:
			a = k0 - Thickness * InDirection;
			b = v0 - Thickness * InDirection;
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(b.Data());
				glVertex2dv(a.Data());
			#else
				glVertex2fv(b.Data());
				glVertex2fv(a.Data());
			#endif
			break;
	}

	// join
	if (JoinStyle == G_BEVEL_JOIN) {
		jc = (j0 + j1) * (GReal)0.5;
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j0.Data());
			glVertex2dv(jc.Data());
		#else
			glVertex2fv(j0.Data());
			glVertex2fv(jc.Data());
		#endif
	}
	else
	if (JoinStyle == G_ROUND_JOIN) {
		jc = JoinCenter + Thickness * vTmp;
		DrawGLCircleSlice(JoinCenter, RoundAuxCoef, j0, jc, halfRoundJoinAngle, ccw);
	}
	else {
		G_ASSERT(JoinStyle == G_MITER_JOIN);

		GVector2 intDir = JoinCenter - vc;
		jc = JoinCenter + intDir;
		GReal intDirLen = intDir.Normalize();

		if (intDirLen > MiterLimitMulThickness)
			jc = JoinCenter + MiterLimitMulThickness * intDir;

		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j0.Data());
			glVertex2dv(jc.Data());
		#else
			glVertex2fv(j0.Data());
			glVertex2fv(jc.Data());
		#endif
	}

	#ifdef DOUBLE_REAL_TYPE
		glVertex2dv(w1.Data());
		glVertex2dv(vc.Data());
	#else
		glVertex2fv(w1.Data());
		glVertex2fv(vc.Data());
	#endif
	glEnd();


	// handle degenerative distance cases
	n = vc - v1;
	if (OutDistance < G_EPSILON || Dot(n, OutDirection) > 0) {
		k1 = w1;
		v1 = vc;
	}
	GPoint2 s1 = (k1 + v1) * (GReal)0.5;


	glBegin(GL_POLYGON);
	// end cap
	switch (EndCapStyle) {

		case G_BUTT_CAP:
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(k1.Data());
				glVertex2dv(v1.Data());
			#else
				glVertex2fv(k1.Data());
				glVertex2fv(v1.Data());
			#endif
			break;

		case G_ROUND_CAP:
			DrawGLCircleSlice(s1, RoundAuxCoef, k1, v1, (GReal)G_PI, ccw);
			break;

		case G_SQUARE_CAP:
			a = k1 + Thickness * OutDirection;
			b = v1 + Thickness * OutDirection;
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(a.Data());
				glVertex2dv(b.Data());
			#else
				glVertex2fv(a.Data());
				glVertex2fv(b.Data());
			#endif
			break;
	}

	#ifdef DOUBLE_REAL_TYPE
		glVertex2dv(vc.Data());
		glVertex2dv(w0.Data());
	#else
		glVertex2fv(vc.Data());
		glVertex2fv(w0.Data());
	#endif

	switch (JoinStyle) {

		case G_BEVEL_JOIN:
		case G_MITER_JOIN:
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(jc.Data());
				glVertex2dv(j1.Data());
			#else
				glVertex2fv(jc.Data());
				glVertex2fv(j1.Data());
			#endif
			break;

		case G_ROUND_JOIN:
			DrawGLCircleSlice(JoinCenter, RoundAuxCoef, jc, j1, halfRoundJoinAngle, ccw);
			break;
	}
	glEnd();
}

void GOpenGLBoard::DrawSolidStroke(const GCapStyle StartCapStyle, const GCapStyle EndCapStyle,
								   const GJoinStyle JoinStyle, const GReal MiterLimitMulThickness,
								   Point2ConstIt PointsBegin, Point2ConstIt PointsEnd,
								   const GBool Closed, const GReal Thickness, const GReal RoundAuxCoeff) {


	GDynArray<GPoint2>::const_iterator it0 = PointsBegin, it1, it2;
	GPoint2 lastPoint = *(PointsEnd - 1);

	it1 = it0;
	it1++;
	it2 = it1;
	it2++;

	// a single line contour (2 points)
	if ((PointsEnd - PointsBegin) == 2)
		DrawGLCapsLine(G_TRUE, StartCapStyle, G_TRUE, EndCapStyle, *it0, *it1, Thickness, RoundAuxCoeff);
	else {
		GReal dist = Distance(*PointsBegin, lastPoint);

		if ((PointsEnd - PointsBegin) == 3 && (dist < G_EPSILON))
			DrawGLCapsLine(G_TRUE, StartCapStyle, G_TRUE, EndCapStyle, *it0, *it1, Thickness, RoundAuxCoeff);
		// contour made at least by 3 (different) points
		else {
			if (!Closed) {
				// draw start cap and line segment
				DrawGLCapsLine(G_TRUE, StartCapStyle, G_FALSE, EndCapStyle, *it0, *it1, Thickness, RoundAuxCoeff);

				GDynArray<GPoint2>::const_iterator itEnd = PointsEnd;
				itEnd--;
				// draw intermediate join-line couples
				while (it2 != itEnd)	{
					DrawGLJoinLine(JoinStyle, MiterLimitMulThickness, *it0, *it1, *it2, Thickness, RoundAuxCoeff);
					it0 = it1;
					it1 = it2;
					it2++;
				}
				// draw last join-line-endcap
				DrawGLJoinLineCap(JoinStyle, MiterLimitMulThickness, *it0, *it1, *it2, Thickness, EndCapStyle, RoundAuxCoeff);
			}
			else {
				if (dist > G_EPSILON)
					DrawGLJoinLine(JoinStyle, MiterLimitMulThickness, lastPoint, *it0, *it1, Thickness, RoundAuxCoeff);
				else {
					GDynArray<GPoint2>::const_iterator it4 = PointsEnd;
					it4 -= 2;
					DrawGLJoinLine(JoinStyle, MiterLimitMulThickness, *it4, *it0, *it1, Thickness, RoundAuxCoeff);
				}

				// draw other segments
				while (it2 != PointsEnd)	{
					DrawGLJoinLine(JoinStyle, MiterLimitMulThickness, *it0, *it1, *it2, Thickness, RoundAuxCoeff);
					it0 = it1;
					it1 = it2;
					it2++;
				}
				// check that first and last point must be geometrically different
				if (dist > G_EPSILON) {
					// line segment and join back to the start
					it2 = PointsBegin;
					DrawGLJoinLine(JoinStyle, MiterLimitMulThickness, *it0, *it1, *it2, Thickness, RoundAuxCoeff);
				}
			}
		}
	}
}

void GOpenGLBoard::DrawDashedStroke(const GOpenGLDrawStyle& Style,
									Point2ConstIt PointsBegin, Point2ConstIt PointsEnd,
									const GBool Closed,	const GReal Thickness, const GReal RoundAuxCoeff) {

	GDynArray<GPoint2>::const_iterator it0 = PointsBegin, it1, it2;

	it1 = it0;
	it1++;
	it2 = it1;
	it2++;

	const GDynArray<GReal>& OfsDashPat = Style.StrokeOffsettedDashPattern();
	GBool empty = Style.StrokeOffsettedDashPatternEmpty();
	GBool startDrawed = !empty;
	GReal PatSum = Style.StrokeDashPatternSum();
	GUInt32 patSize = (GUInt32)OfsDashPat.size();
	GVector2 v, w;
	GReal dashPatVal;
	GPoint2 p0, p1;

	if (PatSum <= G_EPSILON)
		return;

	GUInt32 dashIdx = 0;
	GReal lv, lvOld, lw;

	p0 = *it0;
	v = (*it1) - (*it0);
	lv = v.Normalize();

	if (!Closed) {

		while (lv >= 0 && it1 != PointsEnd) {

			// eat a dash entry
			dashPatVal = OfsDashPat[dashIdx++];
			lvOld = lv;
			lv -= dashPatVal;

			if (!empty) {
				// draw just a line
				if (lv >= 0) {
					p1 = p0 + dashPatVal * v;
					DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), p0, p1, Thickness, RoundAuxCoeff);
					p0 = p1;
				}
				else {
					if (it2 != PointsEnd) {

						GCapStyle tmpStyle = Style.StrokeStartCapStyle();
						GReal tmpLen = lvOld;
						GVector2 oldDir = v;

						// draw remaining piece of current segment, then draw also whole "eaten" segments
						do {
							w = (*it2) - (*it1);
							lw = w.Normalize();
							DrawGLJoin(*it1, v, tmpLen, w, 0, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
										tmpStyle, G_BUTT_CAP, Thickness, RoundAuxCoeff);
							if (tmpLen != 0)
								tmpStyle = G_BUTT_CAP;
							tmpLen = lw;
							lvOld = lv;
							lv += lw;
							it0 = it1;
							it1 = it2;
							it2++;
							oldDir = v;
							v = w;
						} while(lv < 0 && it2 != PointsEnd);

						// draw remained piece of last segment (so we can take care of end cap style)
						if (it2 != PointsEnd) {
							DrawGLJoin(*it0, oldDir, 0, v, -lvOld, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
										tmpStyle, Style.StrokeEndCapStyle(), Thickness, RoundAuxCoeff);
							p0 = (*it0) - lvOld * v;
						}
						else {
							if (lw < -lvOld) {
								DrawGLJoin(*it0, oldDir, 0, v, lw, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
											G_BUTT_CAP, Style.StrokeEndCapStyle(), Thickness, RoundAuxCoeff);
								// now exit
								it1++;
							}
							else {
								DrawGLJoin(*it0, oldDir, 0, v, -lvOld, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
											tmpStyle, Style.StrokeEndCapStyle(), Thickness, RoundAuxCoeff);
								p0 = (*it0) - lvOld * v;
							}
						}
					}
					// last segment
					else {
						DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(),
										p0, *it1, Thickness, RoundAuxCoeff);
						// now exit
						it1++;
					}
				}
			}
			else {
				if (lv >= 0)
					p0 = p0 + dashPatVal * v;
				else {
					if (it2 != PointsEnd) {
						// skip remaining piece of current segment, then skip also whole "eaten" segments
						do {
							w = (*it2) - (*it1);
							lw = w.Normalize();
							lvOld = lv;
							lv += lw;
							it0 = it1;
							it1 = it2;
							it2++;
						} while(lv <= 0 && it2 != PointsEnd);

						if (lw < -lvOld)
							// now exit
							it1++;
						else {
							p0 = (*it0) - lvOld * w;
							v = w;
						}
					}
					// last segment, now exit
					else
						it1++;
				}
			}
			// next dash pattern entry
			if (dashIdx >= patSize) {
				dashIdx = 0;
				empty = Style.StrokeOffsettedDashPatternEmpty();
			}
			else
				empty = !empty;
		}
	}
	// closed contour
	else {
		GDynArray<GPoint2>::const_iterator itEnd = PointsEnd;
		GBool recycle = G_TRUE;
		GBool endDrawed;
		GCapStyle tmpStartCapStyle = G_BUTT_CAP;

recycleLabel:
		while (lv >= 0 && it1 != itEnd) {
			// eat a dash entry
			dashPatVal = OfsDashPat[dashIdx++];
			lvOld = lv;
			lv -= dashPatVal;

			if (!empty) {
				// draw just a line
				if (lv >= 0) {
					p1 = p0 + dashPatVal * v;
					DrawGLCapsLine(G_TRUE, tmpStartCapStyle, G_TRUE, Style.StrokeEndCapStyle(),	p0, p1, Thickness, RoundAuxCoeff);
					p0 = p1;
				}
				else {
					if (it2 != itEnd) {

						GCapStyle tmpStyle = tmpStartCapStyle;
						GReal tmpLen = lvOld;
						GVector2 oldDir = v;
						// draw remaining piece of current segment, then draw also whole "eaten" segments
						do {
							w = (*it2) - (*it1);
							lw = w.Normalize();

							DrawGLJoin(*it1, v, tmpLen, w, 0, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
										tmpStyle, G_BUTT_CAP, Thickness, RoundAuxCoeff);

							if (tmpLen != 0)
								tmpStyle = G_BUTT_CAP;

							tmpLen = lw;
							lvOld = lv;
							lv += lw;
							it0 = it1;
							it1 = it2;
							it2++;
							oldDir = v;
							v = w;
						} while(lv < 0 && it2 != itEnd);
						// draw remained piece of last segment (so we can take care of end cap style)
						if (it2 != itEnd) {
							DrawGLJoin(*it0, oldDir, 0, v, -lvOld, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
										tmpStyle, Style.StrokeEndCapStyle(), Thickness, RoundAuxCoeff);
							p0 = (*it0) - lvOld * v;
						}
						else {
							if (lw < -lvOld)
								// now exit
								it1++;
							else {
								DrawGLJoin(*it0, oldDir, 0, v, -lvOld, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
											tmpStyle, Style.StrokeEndCapStyle(), Thickness, RoundAuxCoeff);
								p0 = (*it0) - lvOld * v;
							}
						}
					}
					// last segment, now exit
					else
						it1++;
				}
			}
			else {
				if (lv >= 0)
					p0 = p0 + dashPatVal * v;
				else {
					if (it2 != itEnd) {
						// skip remaining piece of current segment, then skip also whole "eaten" segments
						do {
							w = (*it2) - (*it1);
							lw = w.Normalize();
							lvOld = lv;
							lv += lw;
							it0 = it1;
							it1 = it2;
							it2++;
						} while(lv <= 0 && it2 != itEnd);
						if (lw < -lvOld)
							// now exit
							it1++;
						else {
							p0 = (*it0) - lvOld * w;
							v = w;
						}
					}
					// last segment, now exit
					else
						it1++;
				}
			}
			tmpStartCapStyle = Style.StrokeStartCapStyle();
			// next dash pattern entry
			if (dashIdx >= patSize) {
				dashIdx = 0;
				empty = Style.StrokeOffsettedDashPatternEmpty();
			}
			else
				empty = !empty;
		}

		it0 = PointsEnd;
		it0--;
		it1 = it2 = PointsBegin;
		it2++;
		endDrawed = empty;

		if (recycle) {

			itEnd = it2;

			w = (*it1) - (*it0);
			lw = w.Normalize();
			p0 = (*it0) - lv * w;

			// take care of last connection
			if (endDrawed) {
				if (lw > -lv)
					DrawGLJoin(*it0, v, dashPatVal + lv, w, -lv, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
								Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), Thickness, RoundAuxCoeff);
				else {
					if (startDrawed)
						DrawGLJoin(*it0, v, dashPatVal + lv, w, lw, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
									Style.StrokeStartCapStyle(), G_BUTT_CAP, Thickness, RoundAuxCoeff);
					else
						DrawGLJoin(*it0, v, dashPatVal + lv, w, lw, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
									Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), Thickness, RoundAuxCoeff);
				}
			}
			lv += lw;
			v = w;
			// now draw the segment that connect last point to the first one (it's the same result that we can
			// obtain if we could push a point into specified Points array)
			recycle = G_FALSE;
			goto recycleLabel;
		}
		else {
			v = (*it1) - (*it0);
			v.Normalize();
			w = (*it2) - (*it1);
			w.Normalize();

			// take care of first connection
			if (endDrawed) {
				if (startDrawed) {
					dashPatVal = OfsDashPat[0];
					DrawGLJoin(*it1, v, lvOld, w, dashPatVal, Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(),
								Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), Thickness, RoundAuxCoeff);
				}
				else
					DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), p0, *it1, Thickness, RoundAuxCoeff);
			}
			else {
				if (startDrawed) {
					dashPatVal = OfsDashPat[0];
					p1 = (*it1) + dashPatVal * w;
					DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_FALSE, G_BUTT_CAP, *it1, p1, Thickness, RoundAuxCoeff);				
				}
			}
		}
	}
}

};	// end namespace Amanith
