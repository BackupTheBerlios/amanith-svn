/****************************************************************************
** $file: amanith/src/rendering/gopenglgeometries.cpp   0.2.0.0   edited Dec, 12 2005
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

void GOpenGLBoard::DrawGLPolygon(const GOpenGLDrawStyle& Style, const GBool ClosedFill, const GBool ClosedStroke,
								 const GJoinStyle FlattenJoinStyle, const GDynArray<GPoint2>& Points,
								 const GBool Convex) {

	// empty contours, or 1 point contour, lets exit immediately
	if (Points.size() < 2)
		return;

	GDynArray< GPoint<GDouble, 2> > triangles;
	GDynArray<GInt32> ptsPerContour;
	GDynArray< GPoint<GDouble, 2> >::const_iterator it;
	GDynArray<GPoint2>::const_iterator it2;
	GUInt32 j = 0;
	GAABox2 tmpBox;

	#define DRAW_FILL_DOUBLE \
		if (!Convex) { \
			if (j > 0) { \
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
			if (j > 0) { \
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
		} \
		else { \
			glBegin(GL_POLYGON); \
			for (it2 = Points.begin(); it2 != Points.end(); ++it2) \
				glVertex2fv(it2->Data()); \
			glEnd(); \
		}

	#define DRAW_STROKE \
		if (Style.StrokeStyle() == G_SOLID_STROKE) { \
				DrawSolidStroke(Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), \
								FlattenJoinStyle, Style.StrokeMiterLimitMulThickness(), \
								Points.begin(), Points.end(), ClosedStroke, Style.StrokeThickness(), Style.gRoundJoinAuxCoef); \
		} \
		else \
			DrawDashedStroke(Style, Points.begin(), Points.end(), ClosedStroke, Style.StrokeThickness(), Style.gRoundJoinAuxCoef);


	if (ClosedFill || CachingEnabled()) {
		// if not convex or have some degenerations (intersection, overlapping edges, etc) use tesselator
		if (!Convex) {
			ptsPerContour.push_back((GInt32)Points.size());
			gTesselator.Tesselate(Points, ptsPerContour, triangles, tmpBox, FillRuleToBehavior(Style.FillRule()));
			j = (GUInt32)triangles.size() / 3;
			G_ASSERT((triangles.size() % 3) == 0);
			if (j == 0)
				tmpBox.SetMinMax(Points);
		}
		// else we must compute box directly
		else
			tmpBox.SetMinMax(Points);
	}
	else
		tmpBox.SetMinMax(Points);

	GOpenGLCacheEntry cacheEntry;
	GOpenGLCachedDrawing *cacheSlot = (GOpenGLCachedDrawing *)CacheSlot();

	// cache the primitive, if needed
	if (CachingEnabled()) {
		// expand box to have always the stroke included
		cacheEntry.Box = tmpBox;
		GPoint2 pMin(cacheEntry.Box.Min());
		GPoint2 pMax(cacheEntry.Box.Max());
		pMin[G_X] -= Style.StrokeThickness();
		pMin[G_Y] -= Style.StrokeThickness();
		pMax[G_X] += Style.StrokeThickness();
		pMax[G_Y] += Style.StrokeThickness();
		cacheEntry.Box.SetMinMax(pMin, pMax);
		// draw fill
		cacheEntry.FillDisplayList = glGenLists(1);
		glNewList(cacheEntry.FillDisplayList, GL_COMPILE);
		#ifdef DOUBLE_REAL_TYPE
			DRAW_FILL_DOUBLE
		#else
			DRAW_FILL_FLOAT
		#endif
		glEndList();
		// draw stroke
		cacheEntry.StrokeDisplayList = glGenLists(1);
		glNewList(cacheEntry.StrokeDisplayList, GL_COMPILE);
		DRAW_STROKE
		glEndList();
		cacheSlot->gEntries.push_back(cacheEntry);
	}

	// if we had to draw nothing, just exit
	if (!Style.StrokeEnabled() && !Style.FillEnabled())
		return;
	if (CachingEnabled() && !CachingWriteOnTarget())
		return;

	GBool doublePass = SetGLClipEnabled(TargetMode(), ClipOperation());

	if (TargetMode() == G_CLIP_MODE) {

		if (!gClipMasksSupport)
			return;

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
		if (!InsideGroup())
			UpdateClipMasksState();

		// calculate bound box of the drawn clip mask
		GPoint2 pMin(tmpBox.Min());
		GPoint2 pMax(tmpBox.Max());
		if (Style.StrokeEnabled()) {
			pMin[G_X] -= Style.StrokeThickness();
			pMin[G_Y] -= Style.StrokeThickness();
			pMax[G_X] += Style.StrokeThickness();
			pMax[G_Y] += Style.StrokeThickness();
			tmpBox.SetMinMax(pMin, pMax);
		}
		if (!InsideGroup())
			gClipMasksBoxes.push_back(tmpBox);
		else {
			// build initial group box
			if (gIsFirstGroupDrawing)
				gGroupBox = tmpBox;
			else {
				// expand group box
				gGroupBox.ExtendToInclude(tmpBox.Min());
				gGroupBox.ExtendToInclude(tmpBox.Max());
			}
		}
		gIsFirstGroupDrawing = G_FALSE;
		return;
	}

	// in color mode, if we are inside a GroupBegin() / GroupEnd() constructor and group opacity is 0
	// do not draw anything
	if (InsideGroup() && GroupOpacity() <= 0 && gGroupOpacitySupport)
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
			DrawAndPopDepthMask(tmpBox, Style, G_TRUE);
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
	#undef DRAW_FILL_FLOAT
	#undef DRAW_FILL_DOUBLE
	#undef DRAW_STROKE
}

void GOpenGLBoard::DrawGLPolygons(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
								  const GDynArray<GBool>& ClosedStrokes, const GOpenGLDrawStyle& Style) {

	G_ASSERT(PointsPerContour.size() == ClosedStrokes.size());
	G_ASSERT(PointsPerContour.size() > 0);
	G_ASSERT(Points.size() > 0);

	GDynArray< GPoint<GDouble, 2> > triangles;
	GDynArray< GPoint<GDouble, 2> >::const_iterator itTriangles;
	GUInt32 i, j;
	Point2ConstIt itPts0, itPts1;
	GAABox2 tmpBox;

	#define DRAW_FILL \
		j = (GUInt32)triangles.size() / 3; \
		if (j > 0) { \
			glBegin(GL_TRIANGLES); \
			for (itTriangles = triangles.begin(); itTriangles != triangles.end(); ++itTriangles) { \
				glVertex2dv(itTriangles->Data()); \
				itTriangles++; \
				glVertex2dv(itTriangles->Data()); \
				itTriangles++; \
				glVertex2dv(itTriangles->Data()); \
			} \
			glEnd(); \
		}

	#define DRAW_STROKE \
		j = (GUInt32)PointsPerContour.size(); \
		itPts0 = itPts1 = Points.begin(); \
		if (Style.StrokeStyle() == G_SOLID_STROKE) { \
			for (i = 0; i < j; ++i) { \
				itPts1 += PointsPerContour[i]; \
				if (PointsPerContour[i] == 2) { \
					tmpBox.ExtendToInclude(*itPts0); \
					tmpBox.ExtendToInclude(*(itPts0 + 1)); \
				} \
				DrawSolidStroke(Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), \
								Style.StrokeJoinStyle(), Style.StrokeMiterLimitMulThickness(), \
								itPts0, itPts1, ClosedStrokes[i], Style.StrokeThickness(), Style.gRoundJoinAuxCoef); \
				itPts0 = itPts1; \
			} \
		} \
		else { \
			for (i = 0; i < j; ++i) { \
				itPts1 += PointsPerContour[i]; \
				if (PointsPerContour[i] == 2) { \
					tmpBox.ExtendToInclude(*itPts0); \
					tmpBox.ExtendToInclude(*(itPts0 + 1)); \
				} \
				DrawDashedStroke(Style, itPts0, itPts1, ClosedStrokes[i], Style.StrokeThickness(), Style.gRoundJoinAuxCoef); \
				itPts0 = itPts1; \
			} \
		} \

	if (Style.FillEnabled() || CachingEnabled())
		// if not convex or have some degenerations (intersection, overlapping edges, etc) use tesselator
		gTesselator.Tesselate(Points, PointsPerContour, triangles, tmpBox, FillRuleToBehavior(Style.FillRule()));
	else
		tmpBox.SetMinMax(Points);

	// caching management
	GOpenGLCacheEntry cacheEntry;
	GOpenGLCachedDrawing *cacheSlot = (GOpenGLCachedDrawing *)CacheSlot();

	// cache the primitive, if needed
	if (CachingEnabled()) {
		// draw fill
		cacheEntry.FillDisplayList = glGenLists(1);
		glNewList(cacheEntry.FillDisplayList, GL_COMPILE);
		DRAW_FILL
		glEndList();
		// draw stroke
		cacheEntry.StrokeDisplayList = glGenLists(1);
		glNewList(cacheEntry.StrokeDisplayList, GL_COMPILE);
		DRAW_STROKE
		glEndList();
		// expand box to have always the stroke included
		cacheEntry.Box = tmpBox;
		GPoint2 pMin(cacheEntry.Box.Min());
		GPoint2 pMax(cacheEntry.Box.Max());
		pMin[G_X] -= Style.StrokeThickness();
		pMin[G_Y] -= Style.StrokeThickness();
		pMax[G_X] += Style.StrokeThickness();
		pMax[G_Y] += Style.StrokeThickness();
		cacheEntry.Box.SetMinMax(pMin, pMax);
		cacheSlot->gEntries.push_back(cacheEntry);
	}

	// if we had to draw nothing, just exit
	if (!Style.StrokeEnabled() && !Style.FillEnabled())
		return;
	if (CachingEnabled() && !CachingWriteOnTarget())
		return;

	GBool doublePass = SetGLClipEnabled(TargetMode(), ClipOperation());

	if (TargetMode() == G_CLIP_MODE) {

		if (!gClipMasksSupport)
			return;

		// draw fill
		if (Style.FillEnabled()) {
			DRAW_FILL
		}
		// draw stroke
		if (Style.StrokeEnabled()) {
			DRAW_STROKE
		}
		// take care of replace operation
		if (!InsideGroup())
			UpdateClipMasksState();
		// calculate bound box of the drawn clip mask
		GPoint2 pMin(tmpBox.Min());
		GPoint2 pMax(tmpBox.Max());
		if (Style.StrokeEnabled()) {
			pMin[G_X] -= Style.StrokeThickness();
			pMin[G_Y] -= Style.StrokeThickness();
			pMax[G_X] += Style.StrokeThickness();
			pMax[G_Y] += Style.StrokeThickness();
			tmpBox.SetMinMax(pMin, pMax);
		}

		if (!InsideGroup())
			gClipMasksBoxes.push_back(tmpBox);
		else {
			// build initial group box
			if (gIsFirstGroupDrawing)
				gGroupBox = tmpBox;
			else {
				// expand group box
				gGroupBox.ExtendToInclude(tmpBox.Min());
				gGroupBox.ExtendToInclude(tmpBox.Max());
			}
		}
		gIsFirstGroupDrawing = G_FALSE;
		return;
	}

	// in color mode, if we are inside a GroupBegin() / GroupEnd() constructor and group opacity is 0
	// do not draw anything
	if (InsideGroup() && GroupOpacity() <= 0 && gGroupOpacitySupport)
		return;

	if (Style.FillEnabled()) {
		// set fill style using OpenGL
		GBool useDepth = UseFillStyle(Style);
		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			// draw fill
			DRAW_FILL
			StencilEnableTop();
		}
		if (useDepth)
			PushDepthMask();
		// draw fill
		DRAW_FILL
		// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
		if (useDepth)
			DrawAndPopDepthMask(tmpBox, Style, G_TRUE);
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
	#undef DRAW_FILL
	#undef DRAW_STROKE
}

void GOpenGLBoard::DoDrawLine(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1) {

	if (Distance(P0, P1) <= G_EPSILON)
		return;

	#define DRAW_STROKE \
		if (Style.StrokeStyle() == G_SOLID_STROKE) \
			DrawGLCapsLine(G_TRUE, s.StrokeStartCapStyle(), G_TRUE, s.StrokeEndCapStyle(), P0, P1, s.StrokeThickness(), s.gRoundJoinAuxCoef); \
		else { \
			GDynArray<GPoint2> pts(2); \
			pts[0] = P0; \
			pts[1] = P1; \
			DrawDashedStroke(s, pts.begin(), pts.end(), G_FALSE, s.StrokeThickness(), s.gRoundJoinAuxCoef); \
		}

	GOpenGLCacheEntry cacheEntry;
	GOpenGLCachedDrawing *cacheSlot = (GOpenGLCachedDrawing *)CacheSlot();
	GOpenGLDrawStyle &s = (GOpenGLDrawStyle &)Style;

	// update style
	UpdateStyle(s);

	// calculate bound box
	GAABox2 tmpBox(P0, P1);
	GPoint2 pMin(tmpBox.Min());
	GPoint2 pMax(tmpBox.Max());
	pMin[G_X] -= s.StrokeThickness();
	pMin[G_Y] -= s.StrokeThickness();
	pMax[G_X] += s.StrokeThickness();
	pMax[G_Y] += s.StrokeThickness();
	tmpBox.SetMinMax(pMin, pMax);

	// cache the primitive, if needed
	if (CachingEnabled()) {
		cacheEntry.Box = tmpBox;
		cacheEntry.FillDisplayList = 0;
		cacheEntry.StrokeDisplayList = glGenLists(1);
		glNewList(cacheEntry.StrokeDisplayList, GL_COMPILE);
		// draw line segment inside cache slot
		DRAW_STROKE
		glEndList();
		cacheSlot->gEntries.push_back(cacheEntry);
	}

	// if we had to draw nothing, just exit
	if (!Style.StrokeEnabled())
		return;
	if (CachingEnabled() && !CachingWriteOnTarget())
		return;

	GBool doublePass = SetGLClipEnabled(TargetMode(), ClipOperation());

	if (TargetMode() == G_CLIP_MODE) {

		if (!gClipMasksSupport)
			return;

		// draw line segment
		DRAW_STROKE

		// take care of replace operation
		if (!InsideGroup())
			UpdateClipMasksState();

		if (!InsideGroup())
			gClipMasksBoxes.push_back(tmpBox);
		else {
			// build initial group box
			if (gIsFirstGroupDrawing)
				gGroupBox = tmpBox;
			else {
				// expand group box
				gGroupBox.ExtendToInclude(tmpBox.Min());
				gGroupBox.ExtendToInclude(tmpBox.Max());
			}
		}
		gIsFirstGroupDrawing = G_FALSE;
		return;
	}

	// in color mode, if we are inside a GroupBegin() / GroupEnd() constructor and group opacity is 0
	// do not draw anything
	if (InsideGroup() && GroupOpacity() <= 0 && gGroupOpacitySupport)
		return;

	// set stroke style using OpenGL
	GBool useDepth = UseStrokeStyle(s);

	// take care of group opacity, first we have to write into stencil buffer
	if (doublePass) {

		GroupFirstPass();
		DRAW_STROKE
		StencilEnableTop();
	}

	if (useDepth)
		PushDepthMask();

	// draw line segment
	DRAW_STROKE

	// geometric radial/conical gradient and transparent entities uses depth clip, so we must pop off clip mask
	if (useDepth)
		DrawAndPopDepthMask(tmpBox, s, G_FALSE);

	#undef DRAW_STROKE
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
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polyline
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_TRUE, Style.StrokeJoinStyle(), pts, G_TRUE);
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
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polyline
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_TRUE, G_BEVEL_JOIN, pts, G_TRUE);
}

void GOpenGLBoard::DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) {

	GBezierCurve2D bez;
	GDynArray<GPoint2> pts;

	// flatten the curve
	bez.SetPoints(P0, P1, P2);
	bez.Flatten(pts, gDeviation, G_TRUE);

	// update style
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polyline
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_FALSE, G_BEVEL_JOIN, pts, G_TRUE);
}

void GOpenGLBoard::DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) {

	GBezierCurve2D bez;
	GDynArray<GPoint2> pts;

	// flatten the curve
	bez.SetPoints(P0, P1, P2, P3);
	bez.Flatten(pts, gDeviation, G_TRUE);

	// update style
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polyline
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_FALSE, Style.StrokeJoinStyle(), pts, G_FALSE);
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
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polyline
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_FALSE, G_BEVEL_JOIN, pts, G_TRUE);
}

void GOpenGLBoard::DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
									const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) {

	GEllipseCurve2D ellipse;
	GDynArray<GPoint2> pts;

	// flatten the curve
	ellipse.SetEllipse(P0, P1, XSemiAxisLength, YSemiAxisLength, OffsetRotation, LargeArc, CCW);
	ellipse.Flatten(pts, gDeviation, G_TRUE);

	// update style
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polyline
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_FALSE, G_BEVEL_JOIN, pts, G_TRUE);
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
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polygon
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_TRUE, G_BEVEL_JOIN, pts, G_TRUE);
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
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polygon
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_TRUE, G_BEVEL_JOIN, pts, G_TRUE);
}

void GOpenGLBoard::DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed) {

	// update style
	UpdateStyle((GOpenGLDrawStyle&)Style);
	// draw polygon
	DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), Closed, Style.StrokeJoinStyle(), Points, G_FALSE);
}

void GOpenGLBoard::DoDrawPath(GDrawStyle& Style, const GCurve2D& Curve) {

	GDynArray<GPoint2> pts;

	// update style
	UpdateStyle((GOpenGLDrawStyle&)Style);

	if (Curve.ClassID() != G_PATH2D_CLASSID && !Curve.IsOfType(G_PATH2D_CLASSID)) {
		// flatten the curve
		Curve.Flatten(pts, gDeviation, G_TRUE);
		// draw polyline
		DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), G_FALSE, Style.StrokeJoinStyle(), pts, G_FALSE);
	}
	else {
		const GPath2D& p = (const GPath2D&)Curve;
		// flatten the curve
		p.Flatten(pts, gDeviation, G_TRUE);
		// draw polyline
		DrawGLPolygon((const GOpenGLDrawStyle&)Style, Style.FillEnabled(), p.IsClosed(), Style.StrokeJoinStyle(), pts, G_FALSE);
	}
}

// here we are sure that we have at least one curve
void GOpenGLBoard::DoDrawPaths(GDrawStyle& Style, const GDynArray<GCurve2D *>& Curves) {

	GOpenGLDrawStyle& s = (GOpenGLDrawStyle&)Style;
	GDynArray<GPoint2> pts;
	GDynArray<GInt32> ptsPerContour;
	GInt32 oldSize, newSize;
	GDynArray<GCurve2D *>::const_iterator itCurves = Curves.begin();
	GDynArray<GBool> closedStroke;
	GCurve2D *c;

	oldSize = newSize = 0;
	ptsPerContour.reserve(Curves.size());
	closedStroke.reserve(Curves.size());

	for (; itCurves != Curves.end(); ++itCurves) {
		c = *itCurves;
		if (c && c->PointsCount() > 1) {
			c->Flatten(pts, gDeviation, G_TRUE);
			newSize = (GInt32)pts.size();
			ptsPerContour.push_back(newSize - oldSize);
			if (c->ClassID() != G_PATH2D_CLASSID && !c->IsOfType(G_PATH2D_CLASSID))
				closedStroke.push_back(G_FALSE);
			else {
				GPath2D* p = (GPath2D *)(c);
				closedStroke.push_back(p->IsClosed());
			}
			oldSize = newSize;
		}
	}

	// empty contours, or 1 point contour, lets exit immediately
	if (pts.size() < 2)
		return;

	// update style
	UpdateStyle(s);
	// draw polygons
	DrawGLPolygons(pts, ptsPerContour, closedStroke, s);
}


// SVG-like path commands
void GOpenGLBoard::BeginPaths() {

	if (!gInsideSVGPaths) {
		gSVGPathPoints.clear();
		gSVGPathPointsPerContour.clear();
		gSVGPathClosedStrokes.clear();
		gOldPointsSize = 0;
		gInsideSVGPaths = G_TRUE;
		gInsideSVGPath = G_FALSE;
	}
}

// begin a sub-path
void GOpenGLBoard::MoveTo(const GPoint2& P, const GBool Relative) {

	GInt32 newSize = (GInt32)gSVGPathPoints.size();
	
	// close a potentially pending path
	if (newSize != gOldPointsSize && gInsideSVGPaths) {

		gSVGPathPoints.push_back(gSVGPathCursor);

		newSize = (GInt32)gSVGPathPoints.size();
		gSVGPathPointsPerContour.push_back(newSize - gOldPointsSize);
		gOldPointsSize = newSize;
		gSVGPathClosedStrokes.push_back(G_FALSE);
	}

	if (gInsideSVGPaths)
		gInsideSVGPath = G_TRUE;

	if (Relative)
		gSVGPathCursor += P;
	else
		gSVGPathCursor = P;

	gFirstPathPoint = gLastCurveCP = gSVGPathCursor;
}

// draw a line
void GOpenGLBoard::LineTo(const GPoint2& P, const GBool Relative) {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	gLastCurveCP = gSVGPathCursor;
	gSVGPathPoints.push_back(gSVGPathCursor);
	if (Relative)
		gSVGPathCursor += P;
	else
		gSVGPathCursor = P;
}

// draw an horizontal line
void GOpenGLBoard::HorizontalLineTo(const GReal X, const GBool Relative) {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	gLastCurveCP = gSVGPathCursor;
	gSVGPathPoints.push_back(gSVGPathCursor);
	if (Relative)
		gSVGPathCursor[G_X] += X;
	else
		gSVGPathCursor[G_X] = X;
}

// draw a vertical line
void GOpenGLBoard::VerticalLineTo(const GReal Y, const GBool Relative) {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	gLastCurveCP = gSVGPathCursor;
	gSVGPathPoints.push_back(gSVGPathCursor);
	if (Relative)
		gSVGPathCursor[G_Y] += Y;
	else
		gSVGPathCursor[G_Y] = Y;
}

// draw a cubic Bezier curve
void GOpenGLBoard::CurveTo(const GPoint2& P1, const GPoint2& P2, const GPoint2& P, const GBool Relative) {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	GBezierCurve2D bez;

	if (Relative) {
		bez.SetPoints(gSVGPathCursor, gSVGPathCursor + P1, gSVGPathCursor + P2, gSVGPathCursor + P);
		gSVGPathCursor += P;
		gLastCurveCP = gSVGPathCursor + P2;
	}
	else {
		bez.SetPoints(gSVGPathCursor, P1, P2, P);
		gSVGPathCursor = P;
		gLastCurveCP = P2;
	}
	bez.Flatten(gSVGPathPoints, gDeviation, G_FALSE);
}

// draw a quadratic Bezier curve
void GOpenGLBoard::CurveTo(const GPoint2& P1, const GPoint2& P, const GBool Relative) {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	GBezierCurve2D bez;

	if (Relative) {
		bez.SetPoints(gSVGPathCursor, gSVGPathCursor + P1, gSVGPathCursor + P);
		gSVGPathCursor += P;
		gLastCurveCP = gSVGPathCursor + P1;
	}
	else {
		bez.SetPoints(gSVGPathCursor, P1, P);
		gSVGPathCursor = P;
		gLastCurveCP = P1;
	}
	bez.Flatten(gSVGPathPoints, gDeviation, G_FALSE);
}

// draw a cubic Bezier curve using smooth tangent
void GOpenGLBoard::SmoothCurveTo(const GPoint2& P2, const GPoint2& P, const GBool Relative) {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	GBezierCurve2D bez;

	// calculate reflected point
	GVector2 dir = gSVGPathCursor - gLastCurveCP;
	GPoint2 p1 = gSVGPathCursor + dir;

	if (Relative) {
		bez.SetPoints(gSVGPathCursor, p1, gSVGPathCursor + P2, gSVGPathCursor + P);
		gSVGPathCursor += P;
	}
	else {
		bez.SetPoints(gSVGPathCursor, p1, P2, P);
		gSVGPathCursor = P;
	}
	gLastCurveCP = P2;
	bez.Flatten(gSVGPathPoints, gDeviation, G_FALSE);
}

// draw a quadratic Bezier curve using smooth tangent
void GOpenGLBoard::SmoothCurveTo(const GPoint2& P, const GBool Relative) {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	GBezierCurve2D bez;

	// calculate reflected point
	GVector2 dir = gSVGPathCursor - gLastCurveCP;
	GPoint2 p1 = gSVGPathCursor + dir;

	if (Relative) {
		bez.SetPoints(gSVGPathCursor, p1, gSVGPathCursor + P);
		gSVGPathCursor += P;
	}
	else {
		bez.SetPoints(gSVGPathCursor, p1, P);
		gSVGPathCursor = P;
	}
	gLastCurveCP = p1;
	bez.Flatten(gSVGPathPoints, gDeviation, G_FALSE);
}

// draw an elliptical arc
void GOpenGLBoard::EllipticalArcTo(const GReal Rx, const GReal Ry, const GReal XRot, const GBool LargeArc,
								   const GBool Sweep, const GPoint2& P, const GBool Relative) {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	GEllipseCurve2D ellipse;

	if (Relative)
		ellipse.SetEllipse(gSVGPathCursor, gSVGPathCursor + P, Rx, Ry, XRot, LargeArc, Sweep);
	else
		ellipse.SetEllipse(gSVGPathCursor, P, Rx, Ry, XRot, LargeArc, Sweep);

	ellipse.Flatten(gSVGPathPoints, gDeviation, G_FALSE);
	if (Relative)
		gSVGPathCursor += P;
	else
		gSVGPathCursor = P;

	gLastCurveCP = gSVGPathPoints.back();
}

// close current sub-path
void GOpenGLBoard::ClosePath() {

	if (!gInsideSVGPaths || !gInsideSVGPath)
		return;

	if (Distance(gSVGPathCursor, gFirstPathPoint) > G_EPSILON)
		gSVGPathPoints.push_back(gSVGPathCursor);

	GInt32 newSize = (GInt32)gSVGPathPoints.size();
	
	gSVGPathPointsPerContour.push_back(newSize - gOldPointsSize);
	gOldPointsSize = newSize;
	gSVGPathClosedStrokes.push_back(G_TRUE);
	gInsideSVGPath = G_FALSE;
}

void GOpenGLBoard::EndPaths() {

	if (!gInsideSVGPaths)
		return;

	gInsideSVGPaths = G_FALSE;
	GInt32 newSize = (GInt32)gSVGPathPoints.size();

	// close a potentially pending path
	if (newSize != gOldPointsSize) {

		gSVGPathPoints.push_back(gSVGPathCursor);
		newSize = (GInt32)gSVGPathPoints.size();

		gSVGPathPointsPerContour.push_back(newSize - gOldPointsSize);
		gOldPointsSize = newSize;
		gSVGPathClosedStrokes.push_back(G_FALSE);
	}

	// empty contours, or 1 point contour, lets exit immediately
	if (gSVGPathPoints.size() < 2)
		return;

	GOpenGLDrawStyle *s = (GOpenGLDrawStyle *)CurrentStyle();
	// update style
	UpdateStyle(*s);
	// draw polygons
	DrawGLPolygons(gSVGPathPoints, gSVGPathPointsPerContour, gSVGPathClosedStrokes, *s);
}


};	// end namespace Amanith
