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
//#include "amanith/geometry/gxform.h"
//#include "amanith/geometry/gxformconv.h"


/*!
	\file gopenglgeometries.cpp
	\brief OpenGL based draw board geometries functions implementation file.
*/

namespace Amanith {

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
	GBool doublePass = SetGLClipEnabled(TargetMode(), ClipOperation());

	if (TargetMode() == G_CLIP_MODE) {

		// take care of replace operation (overflow)
		ClipReplaceOverflowFix();

		// draw fill
		if (Style.FillEnabled()) {
			glBegin(GL_POLYGON);
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(pts[0].Data());
				glVertex2dv(pts[1].Data());
				glVertex2dv(pts[2].Data());
				glVertex2dv(pts[3].Data());
			#else
				glVertex2fv(pts[0].Data());
				glVertex2fv(pts[1].Data());
				glVertex2fv(pts[2].Data());
				glVertex2fv(pts[3].Data());
			#endif
			glEnd();
		}

		// draw stroke
		if (Style.StrokeEnabled()) {
			if (Style.StrokeStyle() == G_SOLID_STROKE)
				DrawSolidStroke(Style, pts, G_TRUE, Style.StrokeThickness());
			else
				DrawDashedStroke(Style, pts, G_TRUE, Style.StrokeThickness());
		}

		// take care of replace operation
		UpdateClipMasksState();
		// calculate bound box of the drawn clip mask
		GAABox2 tmpBox(MinCorner, MaxCorner);
		GPoint2 pMin(tmpBox.Min());
		GPoint2 pMax(tmpBox.Max());
		if (Style.StrokeEnabled()) {
			pMin[G_X] -= Style.StrokeThickness();
			pMin[G_Y] -= Style.StrokeThickness();
			pMax[G_X] += Style.StrokeThickness();
			pMax[G_Y] += Style.StrokeThickness();
		}
		tmpBox.SetMinMax(pMin, pMax);
		gClipMasksBoxes.push_back(tmpBox);
		return;
	}

	// in color mode, if we are inside a GroupBegin() / GroupEnd() constructor and group opacity is 0
	// do not draw anything
	if (InsideGroup() && gGroupOpacitySupport && GroupOpacity() <= 0)
		return;


	if (Style.FillEnabled()) {

		// set fill style using OpenGL
		GBool useDepth = UseFillStyle(Style);

		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			// draw fill
			glBegin(GL_POLYGON);
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(pts[0].Data());
				glVertex2dv(pts[1].Data());
				glVertex2dv(pts[2].Data());
				glVertex2dv(pts[3].Data());
			#else
				glVertex2fv(pts[0].Data());
				glVertex2fv(pts[1].Data());
				glVertex2fv(pts[2].Data());
				glVertex2fv(pts[3].Data());
			#endif
			glEnd();
			StencilEnableTop();
		}

		if (useDepth)
			PushDepthMask();

		// draw fill
		glBegin(GL_POLYGON);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(pts[0].Data());
			glVertex2dv(pts[1].Data());
			glVertex2dv(pts[2].Data());
			glVertex2dv(pts[3].Data());
		#else
			glVertex2fv(pts[0].Data());
			glVertex2fv(pts[1].Data());
			glVertex2fv(pts[2].Data());
			glVertex2fv(pts[3].Data());
		#endif
		glEnd();

		// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
		if (useDepth) {
			GAABox2 tmpBox(MinCorner, MaxCorner);
			GPoint2 pMin(tmpBox.Min());
			GPoint2 pMax(tmpBox.Max());
			tmpBox.SetMinMax(pMin, pMax);
			DrawAndPopDepthMask(tmpBox, Style, G_TRUE);
		}
	}


	// take care to enable stencil test if necessary
	SetGLClipEnabled(TargetMode(), ClipOperation());

	if (Style.StrokeEnabled()) {

		// set stroke style using OpenGL
		GBool useDepth = UseStrokeStyle(Style);

		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			if (Style.StrokeStyle() == G_SOLID_STROKE)
				DrawSolidStroke(Style, pts, G_TRUE, Style.StrokeThickness());
			else
				DrawDashedStroke(Style, pts, G_TRUE, Style.StrokeThickness());
			StencilEnableTop();
		}

		if (useDepth)
			PushDepthMask();

		// draw stroke
		if (Style.StrokeStyle() == G_SOLID_STROKE)
			DrawSolidStroke(Style, pts, G_TRUE, Style.StrokeThickness());
		else
			DrawDashedStroke(Style, pts, G_TRUE, Style.StrokeThickness());

		// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
		if (useDepth) {
			GAABox2 tmpBox(MinCorner, MaxCorner);
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
}

void GOpenGLBoard::DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) {

	if (Style.StrokeWidth() && P0[0] && P1[0] && P2[0]) {
	}
}

void GOpenGLBoard::DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) {

	if (Style.StrokeWidth() && P0[0] && P1[0] && P2[0] && P3[0]) {
	}
}

void GOpenGLBoard::DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength,
									const GReal YSemiAxisLength, const GReal OffsetRotation,
									const GReal StartAngle, const GReal EndAngle, const GBool CCW) {

	if (Style.StrokeWidth() && Center[0] && XSemiAxisLength && YSemiAxisLength && OffsetRotation && StartAngle && EndAngle && CCW) {
	}
}

void GOpenGLBoard::DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
									const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) {
	if (Style.StrokeWidth() && P0[0] && P1[0] && XSemiAxisLength && YSemiAxisLength && OffsetRotation && LargeArc && CCW) {
	}
}

void GOpenGLBoard::DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed) {

	if (Closed && Style.StrokeWidth()) {
	}

	// empty contours, or 1 point contour, lets exit immediately
	if (Points.size() < 2)
		return;
/*
	SetGLClipEnabled(TargetMode(), ClipOperation());

	// update and use style
	UpdateStyle(Style);
	UseStrokeStyle(Style);

	// draw fill
	if (Style.FillEnabled()) {
	}

	// draw stroke
	if (Style.StrokeEnabled()) {
		// solid stroke
		if (Style.StrokeStyle() == G_SOLID_STROKE)
			DrawSolidStroke(Style, Points, Closed, Style.StrokeThickness());
		else {
			if (Points.size() <= 2)
				DrawDashedStroke(Style, Points, G_FALSE, Style.StrokeThickness());
			else
				DrawDashedStroke(Style, Points, Closed, Style.StrokeThickness());
		}
	}

	if (TargetMode() == G_CLIP_MODE) {

		if (ClipOperation() == G_REPLACE_CLIP) {
			gClipMasksBoxes.clear();
			gFirstClipMaskReplace = G_TRUE;
		}
		else {
			if (gClipMasksBoxes.empty())
				gFirstClipMaskReplace = G_FALSE;
		}

		GReal expandLength = 0;
		if (Style.StrokeEnabled())
			expandLength = GMath::Max(Style.StrokeThickness(), Style.StrokeThickness() * Style.StrokeMiterLimit());

		GAABox2 tmpBox(Points);
		GPoint2 newMin = tmpBox.Min();
		GPoint2 newMax = tmpBox.Max();
		newMin[G_X] -= expandLength;
		newMin[G_Y] -= expandLength;
		newMax[G_X] += expandLength;
		newMax[G_Y] += expandLength;
		tmpBox.SetMinMax(newMin, newMax);
		gClipMasksBoxes.push_back(tmpBox);
	}*/
}

};	// end namespace Amanith
