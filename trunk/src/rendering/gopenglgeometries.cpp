/****************************************************************************
** $file: amanith/src/rendering/gopenglgeometries.cpp   0.1.1.0   edited Sep 24 08:00
**
** OpenGL based draw board geometries functions implementation.
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
#include "amanith/2d/gbeziercurve2d.h"
#include "amanith/2d/gellipsecurve2d.h"


/*!
	\file gopenglgeometries.cpp
	\brief OpenGL based draw board geometries functions implementation file.
*/

namespace Amanith {

inline GFillBehavior FillRuleToBehavior(const GFillRule Rule) {

	switch(Rule) {

		case G_ODD_EVEN_FILLRULE:
			return G_ODD_EVEN_RULE;
		case G_EVEN_ODD_FILLRULE:
			return G_EVEN_ODD_RULE;
		case G_NON_ZERO_FILLRULE:
			return G_NON_ZERO_RULE;
		case G_ANY_FILLRULE:
			return G_ANY_RULE;
	default:
		return G_ODD_EVEN_RULE;
	}
}

void GOpenGLBoard::DrawGLPolygon(const GDrawStyle& Style, const GBool ClosedFill, const GBool ClosedStroke,
								 const GJoinStyle FlattenJoinStyle, const GDynArray<GPoint2>& Points,
								 const GBool Convex) {

	// empty contours, or 1 point contour, lets exit immediately
	if (Points.size() < 2)
		return;

	GBool doublePass = SetGLClipEnabled(TargetMode(), ClipOperation());
	GDynArray< GPoint<GDouble, 2> > triangles;
	GDynArray<GInt32> ptsPerContour;
	GDynArray< GPoint<GDouble, 2> >::const_iterator it;
	GDynArray<GPoint2>::const_iterator it2;
	GUInt32 j;
	GAABox2 box;

	#define DRAW_FILL_DOUBLE \
	if (!Convex) { \
		glBegin(GL_TRIANGLES); \
		for (it = triangles.begin(); it != triangles.end(); ++it) { \
			glVertex2dv(it->Data()); \
			it++; \
			glVertex2dv(it->Data()); \
			it++; \
			glVertex2dv(it->Data()); \
		} \
		glEnd(); \
	} \
	else { \
		glBegin(GL_POLYGON); \
		for (it2 = Points.begin(); it2 != Points.end(); ++it2) { \
			glVertex2dv(it2->Data()); \
		} \
		glEnd(); \
	}

	#define DRAW_FILL_FLOAT \
	if (!Convex) { \
		glBegin(GL_TRIANGLES); \
		for (it = triangles.begin(); it != triangles.end(); ++it) { \
			glVertex2dv(it->Data()); \
			it++; \
			glVertex2dv(it->Data()); \
			it++; \
			glVertex2dv(it->Data()); \
		} \
		glEnd(); \
	} \
	else { \
		glBegin(GL_POLYGON); \
		for (it2 = Points.begin(); it2 != Points.end(); ++it2) { \
			glVertex2fv(it2->Data()); \
		} \
		glEnd(); \
	}

	#define DRAW_STROKE \
	if (Style.StrokeStyle() == G_SOLID_STROKE) { \
			DrawSolidStroke(Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), \
							FlattenJoinStyle, Style.StrokeMiterLimitMulThickness(), \
							Points, ClosedStroke, Style.StrokeThickness()); \
	} \
	else \
		DrawDashedStroke(Style, Points, ClosedStroke, Style.StrokeThickness());


	if (ClosedFill) {
		// if not convex or have some degenerations (intersection, overlapping edges, etc) use tesselator
		if (!Convex) {
			ptsPerContour.push_back((GInt32)Points.size());
			gTesselator.Tesselate(Points, ptsPerContour, triangles, box, FillRuleToBehavior(Style.FillRule()));
			j = (GUInt32)triangles.size() / 3;
			G_ASSERT((triangles.size() % 3) == 0);
		}
		// else we must compute box directly
		else
			box.SetMinMax(Points);
	}
	else
		box.SetMinMax(Points);

	if (TargetMode() == G_CLIP_MODE) {

		// take care of replace operation (overflow)
		ClipReplaceOverflowFix();

		// draw fill
		if (ClosedFill) {
			#ifdef DOUBLE_REAL_TYPE
				DRAW_FILL_DOUBLE
			#else
				DRAW_FILL_FLOAT
			#endif
		}

		// draw stroke
		if (Style.StrokeEnabled()) {
			DRAW_STROKE
		}

		// take care of replace operation
		UpdateClipMasksState();
		// calculate bound box of the drawn clip mask
		GPoint2 pMin(box.Min());
		GPoint2 pMax(box.Max());
		if (Style.StrokeEnabled()) {
			pMin[G_X] -= Style.StrokeThickness();
			pMin[G_Y] -= Style.StrokeThickness();
			pMax[G_X] += Style.StrokeThickness();
			pMax[G_Y] += Style.StrokeThickness();
			box.SetMinMax(pMin, pMax);
		}
		gClipMasksBoxes.push_back(box);
		return;
	}

	// in color mode, if we are inside a GroupBegin() / GroupEnd() constructor and group opacity is 0
	// do not draw anything
	if (InsideGroup() && gGroupOpacitySupport && GroupOpacity() <= 0)
		return;

	if (ClosedFill) {
		// set fill style using OpenGL
		GBool useDepth = UseFillStyle(Style);
		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			// draw fill
			#ifdef DOUBLE_REAL_TYPE
				DRAW_FILL_DOUBLE
			#else
				DRAW_FILL_FLOAT
			#endif
			StencilEnableTop();
		}
		if (useDepth)
			PushDepthMask();
		// draw fill
		#ifdef DOUBLE_REAL_TYPE
			DRAW_FILL_DOUBLE
		#else
			DRAW_FILL_FLOAT
		#endif
		// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
		if (useDepth)
			DrawAndPopDepthMask(box, Style, G_TRUE);
	}

	// take care to enable stencil test if necessary
	SetGLClipEnabled(TargetMode(), ClipOperation());

	if (Style.StrokeEnabled()) {
		// set stroke style using OpenGL
		GBool useDepth = UseStrokeStyle(Style);
		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			// draw stroke
			DRAW_STROKE
			StencilEnableTop();
		}
		if (useDepth)
			PushDepthMask();
		// draw stroke
		DRAW_STROKE
		// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
		if (useDepth) {
			GPoint2 pMin(box.Min());
			GPoint2 pMax(box.Max());
			pMin[G_X] -= Style.StrokeThickness();
			pMin[G_Y] -= Style.StrokeThickness();
			pMax[G_X] += Style.StrokeThickness();
			pMax[G_Y] += Style.StrokeThickness();
			box.SetMinMax(pMin, pMax);
			DrawAndPopDepthMask(box, Style, G_FALSE);
		}
	}
	#undef DRAW_FILL_FLOAT
	#undef DRAW_FILL_DOUBLE
	#undef DRAW_STROKE
}

void GOpenGLBoard::DoDrawLine(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1) {

	if (Distance(P0, P1) <= G_EPSILON)
		return;

	// update style
	UpdateStyle(Style);
	GBool doublePass = SetGLClipEnabled(TargetMode(), ClipOperation());

	if (TargetMode() == G_CLIP_MODE) {

		// take care of replace operation (overflow)
		ClipReplaceOverflowFix();

		// draw line segment
		if (Style.StrokeStyle() == G_SOLID_STROKE)
			DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), P0, P1, Style.StrokeThickness());
		else {
			GDynArray<GPoint2> pts(2);
			pts[0] = P0;
			pts[1] = P1;
			DrawDashedStroke(Style, pts, G_FALSE, Style.StrokeThickness());
		}
		// take care of replace operation
		UpdateClipMasksState();

		// calculate bound box of the drawn clip mask
		GAABox2 tmpBox(P0, P1);
		GPoint2 pMin(tmpBox.Min());
		GPoint2 pMax(tmpBox.Max());

		pMin[G_X] -= Style.StrokeThickness();
		pMin[G_Y] -= Style.StrokeThickness();
		pMax[G_X] += Style.StrokeThickness();
		pMax[G_Y] += Style.StrokeThickness();
		tmpBox.SetMinMax(pMin, pMax);
		gClipMasksBoxes.push_back(tmpBox);
		return;
	}

	// in color mode, if we are inside a GroupBegin() / GroupEnd() constructor and group opacity is 0
	// do not draw anything
	if (InsideGroup() && GroupOpacity() <= 0)
		return;

	// set stroke style using OpenGL
	GBool useDepth = UseStrokeStyle(Style);

	// take care of group opacity, first we have to write into stencil buffer
	if (doublePass) {

		GroupFirstPass();

		if (Style.StrokeStyle() == G_SOLID_STROKE)
			DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), P0, P1, Style.StrokeThickness());
		else {
			GDynArray<GPoint2> pts(2);
			pts[0] = P0;
			pts[1] = P1;
			DrawDashedStroke(Style, pts, G_FALSE, Style.StrokeThickness());
		}
		StencilEnableTop();
	}

	if (useDepth)
		PushDepthMask();

	// draw line segment
	if (Style.StrokeStyle() == G_SOLID_STROKE)
		DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), P0, P1, Style.StrokeThickness());
	else {
		GDynArray<GPoint2> pts(2);
		pts[0] = P0;
		pts[1] = P1;
		DrawDashedStroke(Style, pts, G_FALSE, Style.StrokeThickness());
	}

	// geometric radial/conical gradient and transparent entities uses depth clip, so we must pop off clip mask
	if (useDepth) {

		GAABox2 tmpBox(P0, P1);
		GPoint2 pMin(tmpBox.Min());
		GPoint2 pMax(tmpBox.Max());

		pMin[G_X] -= Style.StrokeThickness();
		pMin[G_Y] -= Style.StrokeThickness();
		pMax[G_X] += Style.StrokeThickness();
		pMax[G_Y] += Style.StrokeThickness();
		tmpBox.SetMinMax(pMin, pMax);
		DrawAndPopDepthMask(tmpBox, Style, G_FALSE);
	}
}

void GOpenGLBoard::DoDrawRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner) {

	if (Distance(MinCorner, MaxCorner) <= G_EPSILON)
		return;

	GDynArray<GPoint2> pts(4);
	/*
		p1------p2
		|        |
		|        |
		p0------p3
	*/
	pts[0] = MinCorner;
	pts[1].Set(MinCorner[G_X], MaxCorner[G_Y]);
	pts[2] = MaxCorner;
	pts[3].Set(MaxCorner[G_X], MinCorner[G_Y]);

	// update style
	UpdateStyle(Style);
	// draw polyline
	DrawGLPolygon(Style, Style.FillEnabled(), G_TRUE, Style.StrokeJoinStyle(), pts, G_TRUE);
}

void GOpenGLBoard::DoDrawRoundRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner,
										const GReal ArcWidth, const GReal ArcHeight) {

	if (Distance(MinCorner, MaxCorner) <= G_EPSILON)
		return;

	G_ASSERT(ArcWidth > 0 && ArcHeight > 0);

	GReal radius = GMath::Max(ArcWidth, ArcHeight);
	GReal dev = GMath::Clamp(gFlateness, G_EPSILON, radius - (G_EPSILON * radius));
	GUInt32 n = 3;

	GReal n1 = (GReal)G_PI_OVER2 / ((GReal)2 * GMath::Acos((GReal)1 - dev / radius));
	if (n1 > 3)
		n = (GUInt32)GMath::Ceil(n1);

	// generate points
	GReal deltaAngle = ((GReal)G_PI_OVER2 / n);
	GReal cosDelta = GMath::Cos(deltaAngle);
	GReal sinDelta = GMath::Sin(deltaAngle);
	GReal aOverb = ArcWidth / ArcHeight;
	GReal bOvera = ArcHeight / ArcWidth;
	GPoint2 p, q, m, c;
	GDynArray<GPoint2> pts((n + 1) * 4);
	GDynArray<GPoint2>::iterator it = pts.begin();

	// first sector
	c.Set(MaxCorner[G_X] - ArcWidth, MaxCorner[G_Y] - ArcHeight);
	p.Set(ArcWidth, 0);
	*it = (p + c);
	it++;
	for (GUInt32 i = 0; i < n; ++i) {
		q.Set(p[G_X] * cosDelta - aOverb * p[G_Y] * sinDelta, bOvera * p[G_X] * sinDelta + p[G_Y] * cosDelta);
		*it = (q + c);
		p = q;
		it++;
	}

	// second sector
	c.Set(MinCorner[G_X] + ArcWidth, MaxCorner[G_Y] - ArcHeight);
	p.Set(0, ArcHeight);
	*it = (p + c);
	it++;
	for (GUInt32 i = 0; i < n; ++i) {
		q.Set(p[G_X] * cosDelta - aOverb * p[G_Y] * sinDelta, bOvera * p[G_X] * sinDelta + p[G_Y] * cosDelta);
		*it = (q + c);
		p = q;
		it++;
	}

	// third sector
	c.Set(MinCorner[G_X] + ArcWidth, MinCorner[G_Y] + ArcHeight);
	p.Set(-ArcWidth, 0);
	*it = (p + c);
	it++;
	for (GUInt32 i = 0; i < n; ++i) {
		q.Set(p[G_X] * cosDelta - aOverb * p[G_Y] * sinDelta, bOvera * p[G_X] * sinDelta + p[G_Y] * cosDelta);
		*it = (q + c);
		p = q;
		it++;
	}

	// fourth sector
	c.Set(MaxCorner[G_X] - ArcWidth, MinCorner[G_Y] + ArcHeight);
	p.Set(0, -ArcHeight);
	*it = (p + c);
	it++;
	for (GUInt32 i = 0; i < n; ++i) {
		q.Set(p[G_X] * cosDelta - aOverb * p[G_Y] * sinDelta, bOvera * p[G_X] * sinDelta + p[G_Y] * cosDelta);
		*it = (q + c);
		p = q;
		it++;
	}

	// update style
	UpdateStyle(Style);
	// draw polyline
	DrawGLPolygon(Style, Style.FillEnabled(), G_TRUE, G_BEVEL_JOIN, pts, G_TRUE);
}

void GOpenGLBoard::DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) {

	GBezierCurve2D bez;
	GDynArray<GPoint2> pts;

	// flatten the curve
	bez.SetPoints(P0, P1, P2);
	bez.Flatten(pts, gDeviation, G_TRUE);

	// update style
	UpdateStyle(Style);
	// draw polyline
	DrawGLPolygon(Style, Style.FillEnabled(), G_FALSE, G_BEVEL_JOIN, pts, G_TRUE);
}

void GOpenGLBoard::DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) {

	GBezierCurve2D bez;
	GDynArray<GPoint2> pts;

	// flatten the curve
	bez.SetPoints(P0, P1, P2, P3);
	bez.Flatten(pts, gDeviation, G_TRUE);

	// update style
	UpdateStyle(Style);
	// draw polyline
	DrawGLPolygon(Style, Style.FillEnabled(), G_FALSE, Style.StrokeJoinStyle(), pts, G_FALSE);
}

void GOpenGLBoard::DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength,
									const GReal YSemiAxisLength, const GReal OffsetRotation,
									const GReal StartAngle, const GReal EndAngle, const GBool CCW) {

	GEllipseCurve2D ellipse;
	GDynArray<GPoint2> pts;

	// flatten the curve
	ellipse.SetEllipse(Center, XSemiAxisLength, YSemiAxisLength, OffsetRotation, StartAngle, EndAngle, CCW);
	ellipse.Flatten(pts, gDeviation, G_TRUE);

	// update style
	UpdateStyle(Style);
	// draw polyline
	DrawGLPolygon(Style, Style.FillEnabled(), G_FALSE, G_BEVEL_JOIN, pts, G_TRUE);
}

void GOpenGLBoard::DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
									const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) {

	GEllipseCurve2D ellipse;
	GDynArray<GPoint2> pts;

	// flatten the curve
	ellipse.SetEllipse(P0, P1, XSemiAxisLength, YSemiAxisLength, OffsetRotation, LargeArc, CCW);
	ellipse.Flatten(pts, gDeviation, G_TRUE);

	// update style
	UpdateStyle(Style);
	// draw polyline
	DrawGLPolygon(Style, Style.FillEnabled(), G_FALSE, G_BEVEL_JOIN, pts, G_TRUE);
}

// here we are sure that semi-axes lengths are greater than 0
void GOpenGLBoard::DoDrawEllipse(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength) {

	G_ASSERT(XSemiAxisLength > 0 && YSemiAxisLength > 0);
	GReal radius = GMath::Max(XSemiAxisLength, YSemiAxisLength);

	GReal dev = GMath::Clamp(gFlateness, G_EPSILON, radius - (G_EPSILON * radius));
	GUInt32 n = 4;

	GReal n1 = (GReal)G_2PI / ((GReal)2 * GMath::Acos((GReal)1 - dev / radius));
	if (n1 > 4)
		n = (GUInt32)GMath::Ceil(n1);

	// generate points
	GReal deltaAngle = ((GReal)G_2PI / n);
	GReal cosDelta = GMath::Cos(deltaAngle);
	GReal sinDelta = GMath::Sin(deltaAngle);
	GDynArray<GPoint2> pts(n);
	GDynArray<GPoint2>::iterator it = pts.begin();
	GReal aOverb = XSemiAxisLength / YSemiAxisLength;
	GReal bOvera = YSemiAxisLength / XSemiAxisLength;
	GPoint2 p(XSemiAxisLength, 0), q, m;

	*it = (p + Center);
	it++;
	for (; it != pts.end(); ++it) {
		q.Set(p[G_X] * cosDelta - aOverb * p[G_Y] * sinDelta, bOvera * p[G_X] * sinDelta + p[G_Y] * cosDelta);
		*it = (q + Center);
		p = q;
	}

	// update style
	UpdateStyle(Style);
	// draw polygon
	DrawGLPolygon(Style, Style.FillEnabled(), G_TRUE, G_BEVEL_JOIN, pts, G_TRUE);
}

// here we are sure that Radius is greater than 0
void GOpenGLBoard::DoDrawCircle(GDrawStyle& Style, const GPoint2& Center, const GReal Radius) {

	GReal dev = GMath::Clamp(gFlateness, G_EPSILON, Radius - (G_EPSILON * Radius));
	GUInt32 n = 4;

	GReal n1 = (GReal)G_2PI / ((GReal)2 * GMath::Acos((GReal)1 - dev / Radius));
	if (n1 > 4)
		n = (GUInt32)GMath::Ceil(n1);

	// generate points
	GReal deltaAngle = ((GReal)G_2PI / n);
	GReal cosDelta = GMath::Cos(deltaAngle);
	GReal sinDelta = GMath::Sin(deltaAngle);
	GDynArray<GPoint2> pts(n);
	GDynArray<GPoint2>::iterator it = pts.begin();
	GPoint2 p(Radius, 0), q, m;

	*it = (p + Center);
	it++;
	for (; it != pts.end(); ++it) {
		q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
		*it = (q + Center);
		p = q;
	}

	// update style
	UpdateStyle(Style);
	// draw polygon
	DrawGLPolygon(Style, Style.FillEnabled(), G_TRUE, G_BEVEL_JOIN, pts, G_TRUE);
}

void GOpenGLBoard::DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed) {

	// update style
	UpdateStyle(Style);
	// draw polygon
	DrawGLPolygon(Style, Style.FillEnabled(), Closed, Style.StrokeJoinStyle(), Points, G_FALSE);
}

void GOpenGLBoard::DoDrawPath(GDrawStyle& Style, const GCurve2D& Curve) {

	GDynArray<GPoint2> pts;

	// update style
	UpdateStyle(Style);

	if (Curve.ClassID() != G_PATH2D_CLASSID && !Curve.IsOfType(G_PATH2D_CLASSID)) {
		// flatten the curve
		Curve.Flatten(pts, gDeviation, G_TRUE);
		// draw polyline
		DrawGLPolygon(Style, Style.FillEnabled(), G_FALSE, Style.StrokeJoinStyle(), pts, G_FALSE);
	}
	else {
		const GPath2D& p = (const GPath2D&)Curve;
		// flatten the curve
		p.Flatten(pts, gDeviation, G_TRUE);
		// draw polyline
		DrawGLPolygon(Style, Style.FillEnabled(), p.IsClosed(), Style.StrokeJoinStyle(), pts, G_FALSE);
	}
}

};	// end namespace Amanith
