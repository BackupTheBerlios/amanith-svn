/****************************************************************************
** $file: amanith/src/rendering/gdrawboard.cpp   0.1.1.0   edited Sep 24 08:00
**
** Abstract draw board implementation.
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

#include "amanith/rendering/gdrawboard.h"
#include <new>

/*!
	\file gdrawboard.cpp
	\brief Abstract draw board implementation file.
*/

namespace Amanith {

// *********************************************************************
//                          GRenderingContext
// *********************************************************************
GRenderingContext::GRenderingContext() {

	gOpacity = 1;
	gRenderingQuality = G_NORMAL_RENDERING_QUALITY;
	gTargetMode = G_COLOR_MODE;
	// clip parameters
	gClipEnabled = G_TRUE;
	gClipOperation = G_INTERSECTION_CLIP;

}

// *********************************************************************
//                             GDrawBoard
// *********************************************************************
GDrawBoard::GDrawBoard() {

	gInsideGroup = G_FALSE;
	gViewport.Set(0, 0, 1, 1);
	gProjection.Set(0, 1, 0, 1);
}

GDrawBoard::~GDrawBoard() {
}

// flush (asynchronous)
void GDrawBoard::Flush() {

	if (!gInsideGroup)
		DoFlush();
}

// finish (synchronous)
void GDrawBoard::Finish() {

	if (!gInsideGroup)
		DoFinish();
}

void GDrawBoard::Clear(const GReal Red, const GReal Green, const GReal Blue, const GBool ClearClipMasks) {

	if (!gInsideGroup)
		DoClear(GMath::Clamp(Red, (GReal)0, (GReal)1), GMath::Clamp(Green, (GReal)0, (GReal)1),
				GMath::Clamp(Blue, (GReal)0, (GReal)1), ClearClipMasks);
}

// group begin
void GDrawBoard::GroupBegin() {

	if (!gInsideGroup) {
		gInsideGroup = G_TRUE;
		//gProjection; // x = left; y = right; z = bottom; w = top
		GPoint2 p0(gProjection[G_X], gProjection[G_Z]);
		GPoint2 p1(gProjection[G_Y], gProjection[G_W]);
		DoGroupBegin(GAABox2(p0, p1));
	}
}

void GDrawBoard::GroupBegin(const GAABox2& LogicBox) {

	if (!gInsideGroup) {
		gInsideGroup = G_TRUE;

		GInterval<GReal> xInterval0(LogicBox.Min()[G_X], LogicBox.Max()[G_X]);
		GInterval<GReal> yInterval0(LogicBox.Min()[G_Y], LogicBox.Max()[G_Y]);

		// gProjection; // x = left; y = right; z = bottom; w = top
		GInterval<GReal> xInterval1(gProjection[G_X], gProjection[G_Y]);
		GInterval<GReal> yInterval1(gProjection[G_Z], gProjection[G_W]);

		xInterval0 &= xInterval1;
		yInterval0 &= yInterval1;

		GPoint2 p0(xInterval0.Start(), yInterval0.Start());
		GPoint2 p1(xInterval0.End(), yInterval0.End());
		DoGroupBegin(GAABox2(p0, p1));
	}
}

// group end
void GDrawBoard::GroupEnd() {

	if (gInsideGroup) {
		DoGroupEnd();
		gInsideGroup = G_FALSE;
	}
}

// get active draw style
GDrawStyle *GDrawBoard::ActiveDrawStyle() {

	return &gCurrentContext.gDrawStyle;
}

const GDrawStyle *GDrawBoard::ActiveDrawStyle() const {

	return &gCurrentContext.gDrawStyle;
}

// physical viewport
void GDrawBoard::Viewport(GUInt32& LowLeftCornerX, GUInt32& LowLeftCornerY, GUInt32& Width, GUInt32& Height) {

	LowLeftCornerX = gViewport[G_X];
	LowLeftCornerY = gViewport[G_Y];
	Width = gViewport[G_Z];
	Height = gViewport[G_W];
}

void GDrawBoard::SetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY, const GUInt32 Width, const GUInt32 Height) {

	GUInt32 w = Width;
	GUInt32 h = Height;

	if (w == 0)
		w = 1;
	if (h == 0)
		h = 1;

	if (!gInsideGroup) {
		gViewport.Set(LowLeftCornerX, LowLeftCornerY, w, h);
		DoSetViewport(LowLeftCornerX, LowLeftCornerY, w, h);
	}
}

// projection (matrix)
void GDrawBoard::Projection(GReal& Left, GReal& Right, GReal& Bottom, GReal& Top) {

	Left = gProjection[G_X];
	Right = gProjection[G_Y];
	Bottom = gProjection[G_Z];
	Top = gProjection[G_W];
}

void GDrawBoard::SetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top) {

	if (Left == Right || Bottom == Top || gInsideGroup)
		return;

	if (!gInsideGroup) {
		gProjection.Set(Left, Right, Bottom, Top);
		DoSetProjection(Left, Right, Bottom, Top);
	}
}

// coordinates conversion from logical to physical
GPoint<GInt32, 2> GDrawBoard::LogicalToPhysicalInt(const GPoint2& LogicalPoint) {

	// GPoint<GUInt32, 4> gViewport;  // (x, y) = low-left corner; z = width; w = height
	// GPoint4 gProjection; // x = left; y = right; z = bottom; w = top

	GReal rx = (LogicalPoint[G_X] - gProjection[G_X]) / (gProjection[G_Y] - gProjection[G_X]);
	GInt32 x = (GInt32)((GReal)gViewport[G_X] + (GReal)gViewport[G_Z] * rx);
	GReal ry = (LogicalPoint[G_Y] - gProjection[G_Z]) / (gProjection[G_W] - gProjection[G_Z]);
	GInt32 y = (GInt32)((GReal)gViewport[G_Y] + (GReal)gViewport[G_W] * ry);
	return (GPoint<GInt32, 2>(x, y));
}

// coordinates conversion from logical to physical
GPoint2 GDrawBoard::LogicalToPhysicalReal(const GPoint2& LogicalPoint) {

	// GPoint<GUInt32, 4> gViewport;  // (x, y) = low-left corner; z = width; w = height
	// GPoint4 gProjection; // x = left; y = right; z = bottom; w = top

	GReal rx = (LogicalPoint[G_X] - gProjection[G_X]) / (gProjection[G_Y] - gProjection[G_X]);
	GReal x = ((GReal)gViewport[G_X] + (GReal)gViewport[G_Z] * rx);
	GReal ry = (LogicalPoint[G_Y] - gProjection[G_Z]) / (gProjection[G_W] - gProjection[G_Z]);
	GReal y = ((GReal)gViewport[G_Y] + (GReal)gViewport[G_W] * ry);
	return (GPoint2(x, y));
}

// coordinates conversion from physical to logical
GPoint2 GDrawBoard::PhysicalToLogical(const GPoint<GInt32, 2>& PhysicalPoint) {

	// GPoint<GUInt32, 4> gViewport;  // (x, y) = low-left corner; z = width; w = height
	// GPoint4 gProjection; // x = left; y = right; z = bottom; w = top

	GReal rx = (GReal)((PhysicalPoint[G_X] - (GInt32)gViewport[G_X]) / (GReal)gViewport[G_Z]);
	GReal x = gProjection[G_X] + rx * (gProjection[G_Y] - gProjection[G_X]);
	GReal ry = (GReal)((PhysicalPoint[G_Y] - (GInt32)gViewport[G_Y]) / (GReal)gViewport[G_W]);
	GReal y = gProjection[G_Z] + ry * (gProjection[G_W] - gProjection[G_Z]);
	return GPoint2(x, y);
}


//---------------------------------------------------------------------------
//                             RENDERING CONTEXT
//---------------------------------------------------------------------------

// rendering quality
GRenderingQuality GDrawBoard::RenderingQuality() const {

	return gCurrentContext.gRenderingQuality;
}

void GDrawBoard::SetRenderingQuality(const GRenderingQuality Quality) {

	if (!gInsideGroup) {
		gCurrentContext.gRenderingQuality = Quality;
		DoSetRenderingQuality(Quality);
	}
}

// target mode
GTargetMode GDrawBoard::TargetMode() const {

	return gCurrentContext.gTargetMode;
}

void GDrawBoard::SetTargetMode(const GTargetMode Mode) {

	if (!gInsideGroup) {
		gCurrentContext.gTargetMode = Mode;
		DoSetTargetMode(Mode);
	}
}

// clip operation
GClipOperation GDrawBoard::ClipOperation() const {

	return gCurrentContext.gClipOperation;
}

void GDrawBoard::SetClipOperation(const GClipOperation Operation) {

	if (!gInsideGroup) {
		gCurrentContext.gClipOperation = Operation;
		DoSetClipOperation(Operation);
	}
}

// clip enabled
GBool GDrawBoard::ClipEnabled() const {

	return gCurrentContext.gClipEnabled;
}

void GDrawBoard::SetClipEnabled(const GBool Enabled) {

	if (!gInsideGroup) {
		gCurrentContext.gClipEnabled = Enabled;
		DoSetClipEnabled(Enabled);
	}
}

// pop last clip mask
void GDrawBoard::PopClipMask() {

	if (!gInsideGroup)
		DoPopClipMask();
}

// opacity
GReal GDrawBoard::GroupOpacity() const {

	return gCurrentContext.gOpacity;
}

void GDrawBoard::SetGroupOpacity(const GReal Opacity) {

	if (!gInsideGroup) {
		gCurrentContext.gOpacity = GMath::Clamp(Opacity, (GReal)0, (GReal)1);
		DoSetGroupOpacity(gCurrentContext.gOpacity);
	}
}

// model-view matrix
const GMatrix33& GDrawBoard::ModelViewMatrix() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->ModelView();
}

void GDrawBoard::SetModelViewMatrix(const GMatrix33& Matrix) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetModelView(Matrix);
}
//-----------------------------------------------------------

// stroke start cap
GCapStyle GDrawBoard::StrokeStartCapStyle() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeStartCapStyle();
}

void GDrawBoard::SetStrokeStartCapStyle(const GCapStyle Style) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeStartCapStyle(Style);
}

// stroke end cap
GCapStyle GDrawBoard::StrokeEndCapStyle() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeEndCapStyle();
}

void GDrawBoard::SetStrokeEndCapStyle(const GCapStyle Style) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeEndCapStyle(Style);
}

// stroke join style
GJoinStyle GDrawBoard::StrokeJoinStyle() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeJoinStyle();
}

void GDrawBoard::SetStrokeJoinStyle(const GJoinStyle Style) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeJoinStyle(Style);
}

// stroke width		
GReal GDrawBoard::StrokeWidth() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeWidth();
}

void GDrawBoard::SetStrokeWidth(const GReal Width) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeWidth(Width);
}

// stroke style
GStrokeStyle GDrawBoard::StrokeStyle() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeStyle();
}

void GDrawBoard::SetStrokeStyle(const GStrokeStyle Style) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeStyle(Style);
}

// stroke dash pattern
const GDynArray<GReal>& GDrawBoard::StrokeDashPattern() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeDashPattern();
}

GError GDrawBoard::SetStrokeDashPattern(const GDynArray<GReal>& Pattern) {

	if ((GUInt32)Pattern.size() > MaxDashCount())
		return G_MEMORY_ERROR;

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeDashPattern(Pattern);
	return G_NO_ERROR;
}

// stroke dash phase
GReal GDrawBoard::StrokeDashPhase() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeDashPhase();
}

void GDrawBoard::SetStrokeDashPhase(const GReal DashPhase) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeDashPhase(DashPhase);
}

// stroke paint type
GPaintType GDrawBoard::StrokePaintType() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokePaintType();
}

void GDrawBoard::SetStrokePaintType(const GPaintType PaintType) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokePaintType(PaintType);
}

// stroke color
const GVectBase<GReal, 4>& GDrawBoard::StrokeColor() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeColor();
}

void GDrawBoard::SetStrokeColor(const GVectBase<GReal, 4>& Color) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeColor(Color);
}

// stroke gradient
GGradientDesc *GDrawBoard::StrokeGradient() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeGradient();
}

void GDrawBoard::SetStrokeGradient(GGradientDesc *Gradient) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeGradient(Gradient);
}

// stroke pattern
GPatternDesc *GDrawBoard::StrokePattern() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokePattern();
}

void GDrawBoard::SetStrokePattern(GPatternDesc *Pattern) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokePattern(Pattern);
}

// stroke enable/disable
GBool GDrawBoard::StrokeEnabled() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->StrokeEnabled();
}

void GDrawBoard::SetStrokeEnabled(const GBool Enabled) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetStrokeEnabled(Enabled);
}

// fill rule
GFillRule GDrawBoard::FillRule() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->FillRule();
}

void GDrawBoard::SetFillRule(const GFillRule Rule) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetFillRule(Rule);
}

// fill paint type
GPaintType GDrawBoard::FillPaintType() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->FillPaintType();
}

void GDrawBoard::SetFillPaintType(const GPaintType PaintType) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetFillPaintType(PaintType);
}

// fill color
const GVectBase<GReal, 4>& GDrawBoard::FillColor() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->FillColor();
}

void GDrawBoard::SetFillColor(const GVectBase<GReal, 4>& Color) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetFillColor(Color);
}

// fill gradient
GGradientDesc *GDrawBoard::FillGradient() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->FillGradient();
}

void GDrawBoard::SetFillGradient(GGradientDesc *Gradient) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetFillGradient(Gradient);
}

// fill pattern
GPatternDesc *GDrawBoard::FillPattern() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->FillPattern();
}

void GDrawBoard::SetFillPattern(GPatternDesc *Pattern) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetFillPattern(Pattern);
}

// fill enable/disable
GBool GDrawBoard::FillEnabled() const {

	const GDrawStyle *s = ActiveDrawStyle();
	return s->FillEnabled();
}

void GDrawBoard::SetFillEnabled(const GBool Enabled) {

	GDrawStyle *s = ActiveDrawStyle();
	s->SetFillEnabled(Enabled);
}

//---------------------------------------------------------------------------
//                           PRIMITIVES & COMMANDS
//---------------------------------------------------------------------------
// Low level drawing functions
void GDrawBoard::DrawLine(const GPoint2& P0, const GPoint2& P1) {

	GDrawStyle *s = ActiveDrawStyle();

	if (s->StrokeEnabled())
		DoDrawLine(*s, P0, P1);
}

void GDrawBoard::DrawBezier(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) {

	GDrawStyle *s = ActiveDrawStyle();

	if (s->StrokeEnabled())
		DoDrawBezier(*s, P0, P1, P2);
}

void GDrawBoard::DrawBezier(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) {

	GDrawStyle *s = ActiveDrawStyle();

	if (s->StrokeEnabled())
		DoDrawBezier(*s, P0, P1, P2, P3);
}

void GDrawBoard::DrawEllipseArc(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
								const GReal OffsetRotation, const GReal StartAngle, const GReal EndAngle, const GBool CCW) {

	GDrawStyle *s = ActiveDrawStyle();

	if (s->StrokeEnabled())
		DoDrawEllipseArc(*s, Center, XSemiAxisLength, YSemiAxisLength, OffsetRotation, StartAngle, EndAngle, CCW);
}

void GDrawBoard::DrawEllipseArc(const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
								const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) {

	GDrawStyle *s = ActiveDrawStyle();

	if (s->StrokeEnabled())
		DoDrawEllipseArc(*s, P0, P1, XSemiAxisLength, YSemiAxisLength, OffsetRotation, LargeArc, CCW);
}

void GDrawBoard::DrawPolygon(const GDynArray<GPoint2>& Points, const GBool Closed) {

	GDrawStyle *s = ActiveDrawStyle();

	if (s->StrokeEnabled() || s->FillEnabled())
		DoDrawPolygon(*s, Points, Closed);
}

// High level drawing functions
void GDrawBoard::DrawRectangle(const GPoint2& Center, const GReal Width, const GReal Height) {

	GReal halfW = Width * (GReal)0.5;
	GReal halfH = Height * (GReal)0.5;

	GPoint2 p0(Center[G_X] - halfW, Center[G_Y] - halfH);
	GPoint2 p1(Center[G_X] + halfW, Center[G_Y] + halfH);

	GAABox2 box(p0, p1);
	GDrawStyle *s = ActiveDrawStyle();

	if (s->StrokeEnabled() || s->FillEnabled())
		DoDrawRectangle(*s, box.Min(), box.Max());
}

void GDrawBoard::DrawRectangle(const GPoint2& P0, const GPoint2& P1) {

	GAABox2 box(P0, P1);
	GDrawStyle *s = ActiveDrawStyle();

	if (s->StrokeEnabled() || s->FillEnabled())
		DoDrawRectangle(*s, box.Min(), box.Max());
}

void GDrawBoard::DrawRoundRectangle(const GPoint2& Center, const GReal Width, const GReal Height,
									const GReal ArcWidth, const GReal ArcHeight) {

	if (Center[0] && Width && Height && ArcWidth && ArcHeight) {
	}
}

void GDrawBoard::DrawRoundRectangle(const GPoint2& P0, const GPoint2& P1, const GReal ArcWidth, const GReal ArcHeight) {

	if (P0[0] && P1[0] && ArcWidth && ArcHeight) {
	}
}

void GDrawBoard::DrawEllipse(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength) {

	if (Center[0] && XSemiAxisLength && YSemiAxisLength) {
	}
}

void GDrawBoard::DrawCircle(const GPoint2& Center, const GReal Radius) {

	if (Center[0] && Radius) {
	}
}

void GDrawBoard::DrawPath(const GCurve2D& Curve) {

	if (Curve.StartPoint()[0]) {
	}
}

void GDrawBoard::DrawText(const GPoint2& StartPoint, const GString& TextLine) {

	if (StartPoint[0] && TextLine.length()) {
	}
}

};  // end namespace Amanith
