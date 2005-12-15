/****************************************************************************
** $file: amanith/src/rendering/gdrawboard.cpp   0.2.0.0   edited Dec, 12 2005
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
#include "amanith/support/gsvgpathtokenizer.h"
#include "amanith/geometry/gxformconv.h"
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
	gDrawStyle = NULL;
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

	if (gCurrentContext.gDrawStyle)
		delete gCurrentContext.gDrawStyle;
}

void GDrawBoard::InitDrawStyle() {
	GDrawStyle *s = CreateDrawStyle();
	gCurrentContext.gDrawStyle = s;
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
		// gProjection x = left; y = right; z = bottom; w = top
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

		// gProjection x = left; y = right; z = bottom; w = top
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

// physical to logical matrix
GMatrix33 GDrawBoard::PhysicalToLogicalMatrix() const {

	GMatrix33 preTrans, scale, postTrans;

	// build physical to logical matrix
	TranslationToMatrix(preTrans, GVector2(-(GReal)gViewport[G_X], -(GReal)gViewport[G_Y]));
	ScaleToMatrix(scale, GVector2((gProjection[G_Y] - gProjection[G_X]) / (GReal)gViewport[G_Z],
								  (gProjection[G_W] - gProjection[G_Z]) / (GReal)gViewport[G_W]));
	TranslationToMatrix(postTrans, GVector2(gProjection[G_X], gProjection[G_Z]));
	// build final matrix
	return (postTrans * (scale * preTrans));
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

	return gCurrentContext.gDrawStyle->ModelView();
}

void GDrawBoard::SetModelViewMatrix(const GMatrix33& Matrix) {

	gCurrentContext.gDrawStyle->SetModelView(Matrix);
}
//-----------------------------------------------------------

// stroke start cap
GCapStyle GDrawBoard::StrokeStartCapStyle() const {

	return gCurrentContext.gDrawStyle->StrokeStartCapStyle();
}

void GDrawBoard::SetStrokeStartCapStyle(const GCapStyle Style) {

	gCurrentContext.gDrawStyle->SetStrokeStartCapStyle(Style);
}

// stroke end cap
GCapStyle GDrawBoard::StrokeEndCapStyle() const {

	return gCurrentContext.gDrawStyle->StrokeEndCapStyle();
}

void GDrawBoard::SetStrokeEndCapStyle(const GCapStyle Style) {

	gCurrentContext.gDrawStyle->SetStrokeEndCapStyle(Style);
}

// stroke join style
GJoinStyle GDrawBoard::StrokeJoinStyle() const {

	return gCurrentContext.gDrawStyle->StrokeJoinStyle();
}

void GDrawBoard::SetStrokeJoinStyle(const GJoinStyle Style) {

	gCurrentContext.gDrawStyle->SetStrokeJoinStyle(Style);
}

// stroke width		
GReal GDrawBoard::StrokeWidth() const {

	return gCurrentContext.gDrawStyle->StrokeWidth();
}

void GDrawBoard::SetStrokeWidth(const GReal Width) {

	gCurrentContext.gDrawStyle->SetStrokeWidth(Width);
}

// stroke miter limit
GReal GDrawBoard::StrokeMiterLimit() const {

	return gCurrentContext.gDrawStyle->StrokeMiterLimit();
}

void GDrawBoard::SetStrokeMiterLimit(const GReal Limit) {

	gCurrentContext.gDrawStyle->SetStrokeMiterLimit(Limit);
}

// stroke style
GStrokeStyle GDrawBoard::StrokeStyle() const {

	return gCurrentContext.gDrawStyle->StrokeStyle();
}

void GDrawBoard::SetStrokeStyle(const GStrokeStyle Style) {

	gCurrentContext.gDrawStyle->SetStrokeStyle(Style);
}

// stroke dash pattern
const GDynArray<GReal>& GDrawBoard::StrokeDashPattern() const {

	return gCurrentContext.gDrawStyle->StrokeDashPattern();
}

GError GDrawBoard::SetStrokeDashPattern(const GDynArray<GReal>& Pattern) {

	if ((GUInt32)Pattern.size() > MaxDashCount())
		return G_MEMORY_ERROR;

	gCurrentContext.gDrawStyle->SetStrokeDashPattern(Pattern);
	return G_NO_ERROR;
}

// stroke dash phase
GReal GDrawBoard::StrokeDashPhase() const {

	return gCurrentContext.gDrawStyle->StrokeDashPhase();
}

void GDrawBoard::SetStrokeDashPhase(const GReal DashPhase) {

	gCurrentContext.gDrawStyle->SetStrokeDashPhase(DashPhase);
}

// stroke paint type
GPaintType GDrawBoard::StrokePaintType() const {

	return gCurrentContext.gDrawStyle->StrokePaintType();
}

void GDrawBoard::SetStrokePaintType(const GPaintType PaintType) {

	gCurrentContext.gDrawStyle->SetStrokePaintType(PaintType);
}

// stroke color
const GVectBase<GReal, 4>& GDrawBoard::StrokeColor() const {

	return gCurrentContext.gDrawStyle->StrokeColor();
}

void GDrawBoard::SetStrokeColor(const GVectBase<GReal, 4>& Color) {

	gCurrentContext.gDrawStyle->SetStrokeColor(Color);
}

// stroke gradient
GGradientDesc *GDrawBoard::StrokeGradient() const {

	return gCurrentContext.gDrawStyle->StrokeGradient();
}

void GDrawBoard::SetStrokeGradient(GGradientDesc *Gradient) {

	gCurrentContext.gDrawStyle->SetStrokeGradient(Gradient);
}

// stroke pattern
GPatternDesc *GDrawBoard::StrokePattern() const {

	return gCurrentContext.gDrawStyle->StrokePattern();
}

void GDrawBoard::SetStrokePattern(GPatternDesc *Pattern) {

	gCurrentContext.gDrawStyle->SetStrokePattern(Pattern);
}

// stroke enable/disable
GBool GDrawBoard::StrokeEnabled() const {

	return gCurrentContext.gDrawStyle->StrokeEnabled();
}

void GDrawBoard::SetStrokeEnabled(const GBool Enabled) {

	gCurrentContext.gDrawStyle->SetStrokeEnabled(Enabled);
}

// fill rule
GFillRule GDrawBoard::FillRule() const {

	return gCurrentContext.gDrawStyle->FillRule();
}

void GDrawBoard::SetFillRule(const GFillRule Rule) {

	gCurrentContext.gDrawStyle->SetFillRule(Rule);
}

// fill paint type
GPaintType GDrawBoard::FillPaintType() const {

	return gCurrentContext.gDrawStyle->FillPaintType();
}

void GDrawBoard::SetFillPaintType(const GPaintType PaintType) {

	gCurrentContext.gDrawStyle->SetFillPaintType(PaintType);
}

// fill color
const GVectBase<GReal, 4>& GDrawBoard::FillColor() const {

	return gCurrentContext.gDrawStyle->FillColor();
}

void GDrawBoard::SetFillColor(const GVectBase<GReal, 4>& Color) {

	gCurrentContext.gDrawStyle->SetFillColor(Color);
}

// fill gradient
GGradientDesc *GDrawBoard::FillGradient() const {

	return gCurrentContext.gDrawStyle->FillGradient();
}

void GDrawBoard::SetFillGradient(GGradientDesc *Gradient) {

	gCurrentContext.gDrawStyle->SetFillGradient(Gradient);
}

// fill pattern
GPatternDesc *GDrawBoard::FillPattern() const {

	return gCurrentContext.gDrawStyle->FillPattern();
}

void GDrawBoard::SetFillPattern(GPatternDesc *Pattern) {

	gCurrentContext.gDrawStyle->SetFillPattern(Pattern);
}

// fill enable/disable
GBool GDrawBoard::FillEnabled() const {

	return gCurrentContext.gDrawStyle->FillEnabled();
}

void GDrawBoard::SetFillEnabled(const GBool Enabled) {

	gCurrentContext.gDrawStyle->SetFillEnabled(Enabled);
}

//---------------------------------------------------------------------------
//                           PRIMITIVES & COMMANDS
//---------------------------------------------------------------------------
// Low level drawing functions
void GDrawBoard::DrawLine(const GPoint2& P0, const GPoint2& P1) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if (s->StrokeEnabled())
		DoDrawLine(*s, P0, P1);
}

void GDrawBoard::DrawBezier(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if (s->StrokeEnabled() || s->FillEnabled())
		DoDrawBezier(*s, P0, P1, P2);
}

void GDrawBoard::DrawBezier(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if (s->StrokeEnabled() || s->FillEnabled())
		DoDrawBezier(*s, P0, P1, P2, P3);
}

void GDrawBoard::DrawEllipseArc(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
								const GReal OffsetRotation, const GReal StartAngle, const GReal EndAngle, const GBool CCW) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if ((s->StrokeEnabled() || s->FillEnabled()) && XSemiAxisLength > 0 && YSemiAxisLength > 0)
		DoDrawEllipseArc(*s, Center, XSemiAxisLength, YSemiAxisLength, OffsetRotation, StartAngle, EndAngle, CCW);
}

void GDrawBoard::DrawEllipseArc(const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
								const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if ((s->StrokeEnabled() || s->FillEnabled()) && XSemiAxisLength > 0 && YSemiAxisLength > 0)
		DoDrawEllipseArc(*s, P0, P1, XSemiAxisLength, YSemiAxisLength, OffsetRotation, LargeArc, CCW);
}

void GDrawBoard::DrawPolygon(const GDynArray<GPoint2>& Points, const GBool Closed) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

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
	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if (s->StrokeEnabled() || s->FillEnabled())
		DoDrawRectangle(*s, box.Min(), box.Max());
}

void GDrawBoard::DrawRectangle(const GPoint2& P0, const GPoint2& P1) {

	GAABox2 box(P0, P1);
	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if (s->StrokeEnabled() || s->FillEnabled())
		DoDrawRectangle(*s, box.Min(), box.Max());
}

void GDrawBoard::DrawRoundRectangle(const GPoint2& Center, const GReal Width, const GReal Height,
									const GReal ArcWidth, const GReal ArcHeight) {

	GReal halfW = Width * (GReal)0.5;
	GReal halfH = Height * (GReal)0.5;

	GPoint2 p0(Center[G_X] - halfW, Center[G_Y] - halfH);
	GPoint2 p1(Center[G_X] + halfW, Center[G_Y] + halfH);

	GAABox2 box(p0, p1);
	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if ((s->StrokeEnabled() || s->FillEnabled()) && ArcWidth > 0 && ArcHeight > 0) {
		// arc dimensions cannot be larger than box half-dimensions
		if (ArcWidth >= halfW)
			halfW *= (GReal)0.999;
		if (ArcHeight >= halfH)
			halfH *= (GReal)0.999;
		DoDrawRoundRectangle(*s, box.Min(), box.Max(), halfW, halfH);
	}
}

void GDrawBoard::DrawRoundRectangle(const GPoint2& P0, const GPoint2& P1, const GReal ArcWidth, const GReal ArcHeight) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;
	GAABox2 box(P0, P1);

	if ((s->StrokeEnabled() || s->FillEnabled()) && ArcWidth > 0 && ArcHeight > 0) {
		// arc dimensions cannot be larger than box half-dimensions
		GReal aw = ArcWidth;
		GReal ah = ArcHeight;
		GReal halfW = box.HalfDimension(G_X);
		GReal halfH = box.HalfDimension(G_Y);

		if (aw >= halfW)
			aw = halfW * (GReal)0.999;
		if (ah >= halfH)
			ah = halfH * (GReal)0.999;

		DoDrawRoundRectangle(*s, box.Min(), box.Max(), aw, ah);
	}
}

void GDrawBoard::DrawEllipse(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if ((s->StrokeEnabled() || s->FillEnabled()) && XSemiAxisLength > 0 && YSemiAxisLength > 0)
		DoDrawEllipse(*s, Center, XSemiAxisLength, YSemiAxisLength);
}

void GDrawBoard::DrawCircle(const GPoint2& Center, const GReal Radius) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if ((s->StrokeEnabled() || s->FillEnabled()) && Radius > 0)
		DoDrawCircle(*s, Center, Radius);
}

void GDrawBoard::DrawPath(const GCurve2D& Curve) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if ((s->StrokeEnabled() || s->FillEnabled()) && Curve.PointsCount() > 1)
		DoDrawPath(*s, Curve);
}

void GDrawBoard::DrawPaths(const GDynArray<GCurve2D *>& Curves) {

	GDrawStyle *s = gCurrentContext.gDrawStyle;

	if ((s->StrokeEnabled() || s->FillEnabled()) && Curves.size() > 0)
		DoDrawPaths(*s, Curves);
}

void GDrawBoard::DrawPaths(const GString& SVGPathDescription) {

	if (SVGPathDescription.length() < 2)
		return;

	GReal arg[6];
	GBool largeArc, sweep;
	GChar8 cmd;
	// instantiate a new tokenizer
	GSVGPathTokenizer tokenizer(SVGPathDescription);

	BeginPaths();

	while (tokenizer.NextTkn()) {

		// extract command
		cmd = tokenizer.LastCmd();

		switch(cmd)	{

			// moveto command
			case 'M':
			case 'm':
				arg[0] = tokenizer.LastNum();
				arg[1] = tokenizer.NextTknAsReal(cmd);
				MoveTo(arg[0], arg[1], cmd == 'm');
				break;

			// lineto command
			case 'L':
			case 'l':
				arg[0] = tokenizer.LastNum();
				arg[1] = tokenizer.NextTknAsReal(cmd);
				LineTo(arg[0], arg[1], cmd == 'l');
				break;

			// vertical lineto command
			case 'V':
			case 'v':
				arg[0] = tokenizer.LastNum();
				VerticalLineTo(arg[0], cmd == 'v');
				break;

			// horizontal lineto command
			case 'H':
			case 'h':
				arg[0] = tokenizer.LastNum();
				HorizontalLineTo(arg[0], cmd == 'h');
				break;

			// quadratic Bezier curve
			case 'Q':
			case 'q':
				arg[0] = tokenizer.LastNum();
				arg[1] = tokenizer.NextTknAsReal(cmd);
				arg[2] = tokenizer.NextTknAsReal(cmd);
				arg[3] = tokenizer.NextTknAsReal(cmd);
				CurveTo(arg[0], arg[1], arg[2], arg[3], cmd == 'q');
				break;

			// quadratic Bezier smooth curve
			case 'T':
			case 't':
				arg[0] = tokenizer.LastNum();
				arg[1] = tokenizer.NextTknAsReal(cmd);
				SmoothCurveTo(arg[0], arg[1], cmd == 't');
				break;

			// cubic Bezier curve
			case 'C':
			case 'c':
				arg[0] = tokenizer.LastNum();
				arg[1] = tokenizer.NextTknAsReal(cmd);
				arg[2] = tokenizer.NextTknAsReal(cmd);
				arg[3] = tokenizer.NextTknAsReal(cmd);
				arg[4] = tokenizer.NextTknAsReal(cmd);
				arg[5] = tokenizer.NextTknAsReal(cmd);
				CurveTo(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], cmd == 'c');
				break;

			// cubic Bezier smooth curve
			case 'S':
			case 's':
				arg[0] = tokenizer.LastNum();
				arg[1] = tokenizer.NextTknAsReal(cmd);
				arg[2] = tokenizer.NextTknAsReal(cmd);
				arg[3] = tokenizer.NextTknAsReal(cmd);
				SmoothCurveTo(arg[0], arg[1], arg[2], arg[3], cmd == 's');
				break;

			case 'A':
			case 'a':
				arg[0] = tokenizer.LastNum();
				arg[1] = tokenizer.NextTknAsReal(cmd);
				arg[2] = tokenizer.NextTknAsReal(cmd);
				largeArc = tokenizer.NextTknAsBool(cmd);
				sweep = tokenizer.NextTknAsBool(cmd);
				arg[3] = tokenizer.NextTknAsReal(cmd);
				arg[4] = tokenizer.NextTknAsReal(cmd);
				EllipticalArcTo(arg[0], arg[1], GMath::Deg2Rad(arg[2]), largeArc, sweep, arg[3], arg[4], cmd == 'a');
				break;

			case 'Z':
			case 'z':
				ClosePath();
				break;

		default:
			G_DEBUG("SVG path invalid command");
		}
	}

	EndPaths();
}

};  // end namespace Amanith
