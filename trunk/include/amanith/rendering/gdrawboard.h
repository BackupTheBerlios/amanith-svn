/****************************************************************************
** $file: amanith/rendering/gdrawboard.h   0.1.1.0   edited Sep 24 08:00
**
** Abstract draw board.
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

#ifndef GDRAWBOARD_H
#define GDRAWBOARD_H

#include "amanith/gglobal.h"
#include "amanith/2d/gpath2d.h"
#include "amanith/rendering/gdrawstyle.h"

/*!
	\file gdrawboard.h
	\brief Abstract draw board header file.
*/
namespace Amanith {

	// rendering quality
	enum GRenderingQuality {
		G_LOW_RENDERING_QUALITY,
		G_NORMAL_RENDERING_QUALITY,
		G_HIGH_RENDERING_QUALITY
	};

	// target mode
	enum GTargetMode {
		G_COLOR_MODE,
		G_CLIP_MODE
	};

	// clipping operation/mode
	enum GClipOperation {
		G_REPLACE_CLIP,
		G_INTERSECTION_CLIP
	};


	// *********************************************************************
	//                           GRenderingContext
	// *********************************************************************
	struct GRenderingContext {
	public:
		GDrawStyle *gDrawStyle;
		GRenderingQuality gRenderingQuality;
		GReal gOpacity;
		GTargetMode gTargetMode;
		GBool gClipEnabled;
		GClipOperation gClipOperation;

	public:
		// constructor
		GRenderingContext();
	};

	// *********************************************************************
	//                             GDrawBoard
	// *********************************************************************
	class G_EXPORT GDrawBoard {
	private:
		GBool gInsideGroup;
		GRenderingContext gCurrentContext;

	protected:
		GPoint<GUInt32, 4> gViewport;  // (x, y) = low-left corner; z = width; w = height
		GPoint4 gProjection; // x = left; y = right; z = bottom; w = top

		virtual void DoSetRenderingQuality(const GRenderingQuality Quality) = 0;
		virtual void DoSetTargetMode(const GTargetMode Mode) = 0;
		virtual void DoSetClipOperation(const GClipOperation Operation) = 0;
		virtual void DoSetClipEnabled(const GBool Enabled) = 0;
		virtual void DoPopClipMask() = 0;
		virtual void DoSetGroupOpacity(const GReal Opacity) = 0;
		virtual void DoGroupBegin(const GAABox2& LogicBox) = 0;
		virtual void DoGroupEnd() = 0;
		virtual void DoFlush() = 0;
		virtual void DoFinish() = 0;
		virtual void DoClear(const GReal Red, const GReal Green, const GReal Blue, const GBool ClearClipMasks) = 0;
		virtual void DoSetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
								   const GUInt32 Width, const GUInt32 Height) = 0;
		virtual void DoSetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top) = 0;
		// draw primitives

		// here we are sure that corners are opposite and ordered
		virtual void DoDrawRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner) = 0;
		// here we are sure that corners are opposite and ordered and arc dimensions are > 0
		virtual void DoDrawRoundRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner,
										const GReal ArcWidth, const GReal ArcHeight) = 0;

		virtual void DoDrawLine(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1) = 0;
		virtual void DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) = 0;
		virtual void DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) = 0;
		virtual void DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength,
									const GReal YSemiAxisLength, const GReal OffsetRotation,
									const GReal StartAngle, const GReal EndAngle, const GBool CCW) = 0;
		virtual void DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
									const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) = 0;

		// here we are sure that semi-axes lengths are greater than 0
		virtual void DoDrawEllipse(GDrawStyle& Style, const GPoint2& Center,
									const GReal XSemiAxisLength, const GReal YSemiAxisLength) = 0;
		// here we are sure that Radius is greater than 0
		virtual void DoDrawCircle(GDrawStyle& Style, const GPoint2& Center, const GReal Radius) = 0;
		// here we are sure that Points has at least 2 entries
		virtual void DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed) = 0;
		// here we are sure that Curve has a number of points greater than 1
		virtual void DoDrawPath(GDrawStyle& Style, const GCurve2D& Curve) = 0;
		virtual void DoDrawPaths(GDrawStyle& Style, const GDynArray<GCurve2D *>& Curves) = 0;

		// create a draw style (must be implemented by all derived classes)
		void InitDrawStyle();
		virtual GDrawStyle *CreateDrawStyle() const = 0;
		inline GDrawStyle *CurrentStyle() {
			return gCurrentContext.gDrawStyle;
		}

	public:
		GDrawBoard();
		virtual ~GDrawBoard();

		// read only parameters
		virtual GUInt32 MaxDashCount() const = 0;
		virtual GUInt32 MaxKernelSize() const = 0;
		virtual GUInt32 MaxSeparableKernelSize() const = 0;
		virtual GUInt32 MaxColorKeys() const = 0;
		virtual GUInt32 MaxImageWidth() const = 0;
		virtual GUInt32 MaxImageHeight() const = 0;
		virtual GUInt32 MaxImageBytes() const = 0;
		// paint resources
		virtual GGradientDesc *CreateLinearGradient(const GPoint2& StartPoint, const GPoint2& EndPoint,
													const GDynArray<GKeyValue>& ColorKeys,
													const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
													const GColorRampSpreadMode SpreadMode = G_PAD_COLOR_RAMP_SPREAD,
													const GMatrix33& Matrix = G_MATRIX_IDENTITY33) = 0;
		virtual GGradientDesc *CreateRadialGradient(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
													const GDynArray<GKeyValue>& ColorKeys,
													const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
													const GColorRampSpreadMode SpreadMode = G_PAD_COLOR_RAMP_SPREAD,
													const GMatrix33& Matrix = G_MATRIX_IDENTITY33) = 0;
		virtual GGradientDesc *CreateConicalGradient(const GPoint2& Center, const GPoint2& Target,
													 const GDynArray<GKeyValue>& ColorKeys,
													 const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
													 const GMatrix33& Matrix = G_MATRIX_IDENTITY33) = 0;
		virtual GPatternDesc *CreatePattern(const GPixelMap *Image, const GImageQuality Quality,
											const GTilingMode TilingMode = G_REPEAT_TILE,
											const GAABox2 *LogicalWindow = NULL,
											const GMatrix33& Matrix = G_MATRIX_IDENTITY33) = 0;

		//---------------------------------------------------------------------------
		//                             RENDERING CONTEXT
		//---------------------------------------------------------------------------
		// group opacity
		GReal GroupOpacity() const;
		void SetGroupOpacity(const GReal Opacity);
		// rendering quality
		GRenderingQuality RenderingQuality() const;
		void SetRenderingQuality(const GRenderingQuality Quality);
		// target mode
		GTargetMode TargetMode() const;
		void SetTargetMode(const GTargetMode Mode);
		// clip operation
		GClipOperation ClipOperation() const;
		void SetClipOperation(const GClipOperation Operation);
		// clip enable/disable
		GBool ClipEnabled() const;
		void SetClipEnabled(const GBool Enabled);
		// pop last clip mask
		void PopClipMask();
		// model-view matrix
		const GMatrix33& ModelViewMatrix() const;
		void SetModelViewMatrix(const GMatrix33& Matrix);
		//---------------------------------------------------------------------------
		//                                  STROKE
		//---------------------------------------------------------------------------
		// stroke start cap
		GCapStyle StrokeStartCapStyle() const;
		void SetStrokeStartCapStyle(const GCapStyle Style);
		// stroke end cap
		GCapStyle StrokeEndCapStyle() const;
		void SetStrokeEndCapStyle(const GCapStyle Style);
		// stroke join style
		GJoinStyle StrokeJoinStyle() const;
		void SetStrokeJoinStyle(const GJoinStyle Style);
		// stroke width		
		GReal StrokeWidth() const;
		void SetStrokeWidth(const GReal Width);
		// stroke style
		GStrokeStyle StrokeStyle() const;
		void SetStrokeStyle(const GStrokeStyle Style);
		// stroke dash pattern
		const GDynArray<GReal>& StrokeDashPattern() const;
		GError SetStrokeDashPattern(const GDynArray<GReal>& Pattern);
		// stroke dash phase
		GReal StrokeDashPhase() const;
		void SetStrokeDashPhase(const GReal DashPhase);
		// stroke paint type
		GPaintType StrokePaintType() const;
		void SetStrokePaintType(const GPaintType PaintType);
		// stroke color
		const GVectBase<GReal, 4>& StrokeColor() const;
		void SetStrokeColor(const GVectBase<GReal, 4>& Color);
		// stroke gradient
		GGradientDesc *StrokeGradient() const;
		void SetStrokeGradient(GGradientDesc *Gradient);
		// stroke pattern
		GPatternDesc *StrokePattern() const;
		void SetStrokePattern(GPatternDesc *Pattern);
		// stroke enable/disable
		GBool StrokeEnabled() const;
		void SetStrokeEnabled(const GBool Enabled);

		//---------------------------------------------------------------------------
		//                                  FILL
		//---------------------------------------------------------------------------
		// fill rule
		GFillRule FillRule() const;
		void SetFillRule(const GFillRule Rule);
		// fill paint type
		GPaintType FillPaintType() const;
		void SetFillPaintType(const GPaintType PaintType);
		// fill color
		const GVectBase<GReal, 4>& FillColor() const;
		void SetFillColor(const GVectBase<GReal, 4>& Color);
		// fill gradient
		GGradientDesc *FillGradient() const;
		void SetFillGradient(GGradientDesc *Gradient);
		// fill pattern
		GPatternDesc *FillPattern() const;
		void SetFillPattern(GPatternDesc *Pattern);
		// fill enable/disable
		GBool FillEnabled() const;
		void SetFillEnabled(const GBool Enabled);

		//---------------------------------------------------------------------------
		//                           PRIMITIVES & COMMANDS
		//---------------------------------------------------------------------------
		// Low level drawing functions
		void DrawLine(const GPoint2& P0, const GPoint2& P1);
		void DrawBezier(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2);
		void DrawBezier(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3);
		void DrawEllipseArc(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
							const GReal OffsetRotation = 0, const GReal StartAngle = 0, const GReal EndAngle = G_2PI,
							const GBool CCW = G_TRUE);
		void DrawEllipseArc(const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
							const GReal OffsetRotation = 0, const GBool LargeArc = G_TRUE, const GBool CCW = G_FALSE);
		void DrawPolygon(const GDynArray<GPoint2>& Points, const GBool Closed);

		// High level drawing functions
		void DrawRectangle(const GPoint2& Center, const GReal Width, const GReal Height);
		void DrawRectangle(const GPoint2& P0, const GPoint2& P1);
		void DrawRoundRectangle(const GPoint2& Center, const GReal Width, const GReal Height,
								const GReal ArcWidth, const GReal ArcHeight);
		void DrawRoundRectangle(const GPoint2& P0, const GPoint2& P1,
								const GReal ArcWidth, const GReal ArcHeight);
		void DrawEllipse(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength);
		void DrawCircle(const GPoint2& Center, const GReal Radius);
		void DrawPath(const GCurve2D& Curve);
		void DrawPaths(const GDynArray<GCurve2D *>& Curves);
		void DrawPaths(const GString& SVGPathDescription);
		inline void DrawPaths(const GChar8 *SVGPathDescription) {
			DrawPaths(GString(SVGPathDescription));
		}
		// SVG-like path commands
		virtual void BeginPaths() = 0;
		// begin a sub-path
		virtual void MoveTo(const GPoint2& P, const GBool Relative) = 0;
		inline void MoveTo(const GReal X, const GReal Y, const GBool Relative) {
			MoveTo(GPoint2(X, Y), Relative);
		}
		// draw a line
		virtual void LineTo(const GPoint2& P, const GBool Relative) = 0;
		inline void LineTo(const GReal X, const GReal Y, const GBool Relative) {
			LineTo(GPoint2(X, Y), Relative);
		}
		// draw an horizontal line
		virtual void HorizontalLineTo(const GReal X, const GBool Relative) = 0;
		// draw a vertical line
		virtual void VerticalLineTo(const GReal Y, const GBool Relative) = 0;
		// draw a cubic Bezier curve
		virtual void CurveTo(const GPoint2& P1, const GPoint2& P2, const GPoint2& P, const GBool Relative) = 0;
		inline void CurveTo(const GReal X1, const GReal Y1, const GReal X2, const GReal Y2,
							const GReal X, const GReal Y, const GBool Relative) {
			CurveTo(GPoint2(X1, Y1), GPoint2(X2, Y2), GPoint2(X, Y), Relative);
		}
		// draw a quadratic Bezier curve
		virtual void CurveTo(const GPoint2& P1, const GPoint2& P, const GBool Relative) = 0;
		inline void CurveTo(const GReal X1, const GReal Y1, const GReal X, const GReal Y, const GBool Relative) {
			CurveTo(GPoint2(X1, Y1), GPoint2(X, Y), Relative);
		}
		// draw a cubic Bezier curve using smooth tangent
		virtual void SmoothCurveTo(const GPoint2& P2, const GPoint2& P, const GBool Relative) = 0;
		void SmoothCurveTo(const GReal X2, const GReal Y2, const GReal X, const GReal Y, const GBool Relative) {
			SmoothCurveTo(GPoint2(X2, Y2), GPoint2(X, Y), Relative);
		}
		// draw a quadratic Bezier curve using smooth tangent
		virtual void SmoothCurveTo(const GPoint2& P, const GBool Relative) = 0;
		void SmoothCurveTo(const GReal X, const GReal Y, const GBool Relative) {
			SmoothCurveTo(GPoint2(X, Y), Relative);
		}
		// draw an elliptical arc
		virtual void EllipticalArcTo(const GReal Rx, const GReal Ry, const GReal XRot, const GBool LargeArc,
									const GBool Sweep, const GPoint2& P, const GBool Relative) = 0;
		inline void EllipticalArcTo(const GReal Rx, const GReal Ry, const GReal XRot, const GBool LargeArc,
									const GBool Sweep, const GReal X, const GReal Y, const GBool Relative) {
			EllipticalArcTo(Rx, Ry, XRot, LargeArc, Sweep, GPoint2(X, Y), Relative);
		}
		// close current sub-path
		virtual void ClosePath() = 0;
		virtual void EndPaths() = 0;

		// clear board
		void Clear(const GReal Red, const GReal Green, const GReal Blue, const GBool ClearClipMasks = G_TRUE);
		/*!
			This function ensures that all outstanding requests on the current context will complete in finite time.
			Flush may return prior to the actual completion of all requests.
		*/
		void Flush();
		void Finish();
		// group begin/end
		void GroupBegin();
		void GroupBegin(const GAABox2& LogicBox);
		void GroupEnd();
		// physical viewport
		void Viewport(GUInt32& LowLeftCornerX, GUInt32& LowLeftCornerY, GUInt32& Width, GUInt32& Height);
		void SetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY, const GUInt32 Width, const GUInt32 Height);
		// logical viewport (it defines projection matrix)
		void Projection(GReal& Left, GReal& Right, GReal& Bottom, GReal& Top);
		void SetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top);
		// coordinates conversion from logical to physical
		GPoint<GInt32, 2> LogicalToPhysicalInt(const GPoint2& LogicalPoint);
		GPoint2 LogicalToPhysicalReal(const GPoint2& LogicalPoint);
		// coordinates conversion from physical to logical
		GPoint2 PhysicalToLogical(const GPoint<GInt32, 2>& PhysicalPoint);
		// return G_TRUE if current rendering operations are inside a GroupBegin() / GroupEnd() constructor
		inline GBool InsideGroup() const {
			return gInsideGroup;
		}
	};

};	// end namespace Amanith

#endif
