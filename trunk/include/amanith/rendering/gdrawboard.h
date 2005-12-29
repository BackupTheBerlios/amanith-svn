/****************************************************************************
** $file: amanith/rendering/gdrawboard.h   0.2.0.0   edited Dec, 12 2005
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

	//! Rendering quality.
	enum GRenderingQuality {
		//! Low rendering quality.
		G_LOW_RENDERING_QUALITY,
		//! Normal rendering quality.
		G_NORMAL_RENDERING_QUALITY,
		//! High rendering quality.
		G_HIGH_RENDERING_QUALITY
	};

	//! Target mode.
	enum GTargetMode {
		//! Color mode.
		G_COLOR_MODE,
		//! Clip mode.
		G_CLIP_MODE,
		//! Color and cache mode.
		G_COLOR_AND_CACHE_MODE,
		//! Clip and cache mode.
		G_CLIP_AND_CACHE_MODE,
		//! Cache mode
		G_CACHE_MODE
	};

	//! Clipping operation (when target mode is clip).
	enum GClipOperation {
		//! Replace operation.
		G_REPLACE_CLIP,
		//! Intersection operation.
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


	// forward declaration
	class GDrawBoard;

	/*!
		\class GCacheBank
		\brief This virtual class represents a cache bank, intended as a set of contiguous cache slots (each slot
		is in unique correspondence to a single drawn primitive on the current target buffer).
		
		A cache bank, after being filled, can be reused to draw cached shapes. The bank can be invalidated using
		the function Invalidate().
	*/
	class G_EXPORT GCacheBank {

		friend class GDrawBoard;

	public:
		//! Constructor, it build an empty cache bank.
		GCacheBank() {
		}
		//! Destructor, it invalidates cached shapes and frees memory.
		virtual ~GCacheBank() {
		}
		/*!
			Get the number of slots (cached shapes).

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GUInt32 SlotsCount() const = 0;
		/*!
			Invalidate the whole cache bank, freeing associated memory.

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void Invalidate() = 0;
	};

	// *********************************************************************
	//                             GDrawBoard
	// *********************************************************************

	/*!
		\class GDrawBoard
		\brief An abstract drawboard paint server.

		This class represents an abstract drawboard, intended as an immediate paint server.
		Using the drawboard, users can draw everything, from simple lines to complex shapes. All geometric
		entities can be stroked and/or filled, using several combinations of draw styles.
		The core functionality of GDrawBoard is drawing, and there are functions to draw most primitives:
		DrawLine(), DrawRectangle(), DrawRoundRectangle(), DrawEllipse(), DrawBezier(), DrawEllipseArc(),
		DrawPolygon(), DrawPaths().\n

		Drawboard exposes also an SVG primitives layer, so paths can be built using SVG compatible commands, like
		MoveTo(), LineTo(), HorizontalLineTo(), VerticalLineTo(), CurveTo(), SmoothCurveTo(), EllipticalArcTo().
		For convenience (and to make the user life simpler), a method that draw a complete SVG path given his
		string description (the SVG 'd' attribute) has been exposed also.\n
		Through the function DrawPath(), all Amanith curves (the ones derived from GCurve2D class) can be
		rendered, so the user is not limited to SVG/OpenVG-like shapes, but he can draw (for example) high
		order Bezier and B-Spline curves.\n\n

		Shape's coordinates are always expressed into the logical window system. This logical window (that represents
		a portion of the infinitive worldspace canvas) is then automatically mapped into the physical viewport.
		Users can change the logical window using the function SetProjection(), and they can change physical
		viewport using the function SetViewport(). Some utility functions are exposed too, they can be useful to
		convert a point from logical to physical and viceversa; take a look at PhysicalToLogical(),
		LogicalToPhysicalInt(), LogicalToPhysicalReal().\n
		Drawboard permits also to set an optional 'world' matrix; this matrix will be applied to all drawing
		operation, take a look at SetModelViewMatrix() function.\n\n

		Drawboard is adaptive, in the sense that user can choose a set of options that influence the rendering
		quality and time performance, through the SetRenderingQuality() function.\n
		All painting operation can be clipped by optional paths, that the user can define using exposed methods.
		These masks can be enabled and disabled at any time.\n
		Clip paths can be defined using the same set of drawing functions, because the drawboard has 2
		target buffers: color buffer and clip buffer. The user can switch between them using the function SetTargetMode().\n\n

		Shapes can be stroked and/or filled, independently.\n
		There are methods to change stroke attributes, like
		SetStrokeStartCapStyle(), SetStrokeEndCapStyle(), SetStrokeJoinStyle(), SetStrokeWidth(), SetStrokeMiterLimit(),
		SetStrokeStyle(), SetStrokeDashPattern(), SetStrokeDashPhase(), SetStrokePaintType(), SetStrokeColor(),
		SetStrokeGradient(), SetStrokePattern().\n
		Of course there are similar methods for fill: SetFillRule(), SetFillPaintType(), SetFillColor(),
		SetFillGradient(), SetFillPattern().\n
		Another important style feature is group opacity: using SetGroupOpacity() function the user can set
		group opacity, and then he can draw a group of shapes between the calls GroupBegin() and GroupEnd().\n
	*/
	class G_EXPORT GDrawBoard {
	private:
		GBool gInsideGroup;
		GRenderingContext gCurrentContext;

	protected:
		//! Current viewport.
		GPoint<GUInt32, 4> gViewport;  // (x, y) = low-left corner; z = width; w = height
		//! Current logical window.
		GPoint4 gProjection; // x = left; y = right; z = bottom; w = top

		/*!
			Do the effective set of rendering quality settings.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoSetRenderingQuality(const GRenderingQuality Quality) = 0;
		/*!
			Do the effective set of target mode (target buffer).

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoSetTargetMode(const GTargetMode Mode) = 0;
		/*!
			Do the effective set clip operation to use for clip paths.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoSetClipOperation(const GClipOperation Operation) = 0;
		/*!
			Enable (G_TRUE value) or disable (G_FALSE value) clip paths.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoSetClipEnabled(const GBool Enabled) = 0;
		/*!
			Do the effective pop of the last clip path from stack.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoPopClipMask() = 0;
		/*!
			Do the effective set of group opacity.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoSetGroupOpacity(const GReal Opacity) = 0;
		/*!
			Do the effective opening of a group block.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoGroupBegin(const GAABox2& LogicBox) = 0;
		/*!
			Do the effective closure of a group block.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoGroupEnd() = 0;
		/*!
			Do the effective non-blocking flush of all outstanding requests on the current context.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoFlush() = 0;
		/*!
			Do the effective blocking flush of all outstanding requests on the current context.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoFinish() = 0;
		/*!
			Do the effective drawboard cleaning.

			This methods permits to clear the entire drawboard, using s specified color. In the same 'atomic'
			operation the clipbuffer can be cleared too.
			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.

			\param Red the red component of the clear color.
			\param Green the green component of the clear color.
			\param Blue the blue component of the clear color.
			\param ClearClipMasks if G_TRUE also clipbuffer will be cleared. In this case all existing masks will
			be dropped.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoClear(const GReal Red, const GReal Green, const GReal Blue, const GBool ClearClipMasks) = 0;
		/*!
			Do the effective set of physical viewport.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\param LowLeftCornerX lowleft corner abscissa of the viewport.
			\param LowLeftCornerY lowleft corner ordinate of the viewport.
			\param Width width of the viewport expressed in pixels, it's ensured to be grater than 0.
			\param Height height of the viewport expressed in pixels, it's ensured to be grater than 0.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoSetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
								   const GUInt32 Width, const GUInt32 Height) = 0;
		/*!
			Do the effective set of the logical viewport.

			Logical viewport (that defines implicitly a projection matrix) is used to select a rectangular portion of
			infinitive canvas. This can be used to do all kind of panning and zooming operations.
			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block, in
			addition Left is ensure to be different than Right and Bottom is ensured to be different from Top.
			\param Left the abscissa of the bottom-left corner of current logical window.
			\param Right the abscissa of the top-right corner of current logical window.
			\param Bottom the ordinate of the bottom-left corner of current logical window.
			\param Top the ordinate of the top-right corner of current logical window.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoSetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top) = 0;
		/*!
			Do the effective drawing of a rectangle.

			When this method is called, it's ensured that corners are opposite and ordered.

			\param Style the drawstyle to use.
			\param MinCorner a corner of the rectangle.
			\param MaxCorner the opposite (to MinCorner) corner of the rectangle.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner) = 0;
		/*!
			Do the effective drawing of a round rectangle.

			When this method is called, it's ensured that corners are opposite and ordered.

			\param Style the drawstyle to use.
			\param MinCorner a corner of the rectangle.
			\param MaxCorner the opposite (to MinCorner) corner of the rectangle.
			\param ArcWidth the x-axis radius of the ellipse used to round off the corners of the rectangle.
			It's ensured to be positive.
			\param ArcHeight the y-axis radius of the ellipse used to round off the corners of the rectangle.
			It's ensured to be positive.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawRoundRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner,
											const GReal ArcWidth, const GReal ArcHeight) = 0;
		/*!
			Do the effective drawing of a line.

			\param Style the drawstyle to use.
			\param P0 the line start point.
			\param P1 the line end point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawLine(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1) = 0;
		/*!
			Do the effective drawing of a quadratic Bezier curve, going form P0 to
			P2 and having P1 as central control point.

			\param Style the drawstyle to use.
			\param P0 the curve start point.
			\param P1 the curve control point.
			\param P2 the curve end point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) = 0;
		/*!
			Do the effective drawing of a cubic Bezier curve, going form P0 to P4 and
			having P1 and P2 as control points.

			\param Style the drawstyle to use.
			\param P0 the curve start point.
			\param P1 the curve first control point (associated with P0).
			\param P2 the curve second control point (associated with P3).
			\param P3 the curve end point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) = 0;
		/*!
			Do the effective drawing of an ellipse arc.

			\param Style the drawstyle to use.
			\param Center the center of the ellipse.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It's ensured to be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It's ensured to be greater than 0.
			\param OffsetRotation an optional offset rotation (expressed in radians) relative to the x-axis.
			\param StartAngle an angle (in radians) that defines the start point of the arc.
			\param EndAngle an angle (in radians) that defines the end point of the arc.
			\param CCW if G_TRUE the ellipse arc has to go from start point to end point in counter clockwise
			direction, else G_FALSE (clockwise direction).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength,
										const GReal YSemiAxisLength, const GReal OffsetRotation,
										const GReal StartAngle, const GReal EndAngle, const GBool CCW) = 0;
		/*!
			Do the effective drawing of an ellipse arc.

			\param Style the drawstyle to use.
			\param P0 the start point of the arc.
			\param P1 the end point of the arc.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It's ensured to be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It's ensured to be greater than 0.
			\param OffsetRotation an optional offset rotation (expressed in radians) relative to the x-axis.
			\param LargeArc if G_TRUE the ellipse arc has to go from start point to end point spanning the
			longest chord, else G_FALSE.
			\param CCW if G_TRUE the ellipse arc has to go from start point to end point in counter clockwise
			direction, else G_FALSE (clockwise direction).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
										const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) = 0;
		/*!
			Do the effective drawing of a whole ellipse.

			\param Style the drawstyle to use.
			\param Center the center of the ellipse.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It's ensured to be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It's ensured to be greater than 0.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawEllipse(GDrawStyle& Style, const GPoint2& Center,
									const GReal XSemiAxisLength, const GReal YSemiAxisLength) = 0;
		/*!
			Do the effective drawing of a circle.

			\param Style the drawstyle to use.
			\param Center the center of the circle.
			\param Radius the radius of the circle, it's ensured to be positive.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawCircle(GDrawStyle& Style, const GPoint2& Center, const GReal Radius) = 0;
		/*!
			Draw the effective drawing of a polygon.

			\param Style the drawstyle to use.
			\param Points the set of consecutive points that define the polygon, it's ensured that at least 2 points
			entries exist.
			\param Closed if G_TRUE the polygon is considered closed (an imaginary edge goes from last point to
			the first one).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed) = 0;
		/*!
			Do the effective drawing of curve/path.

			\param Style the drawstyle to use.
			\param Curve the curve to draw, it consists of at least 2 points.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawPath(GDrawStyle& Style, const GCurve2D& Curve) = 0;
		/*!
			Do the effective drawing of a set of curves and paths.

			This method permits to draw complex shapes, made of more than 1 curve/path, using the currently set
			fill rule.

			\param Style the drawstyle to use.
			\param Curves an array of curves pointers. Each pointer must be valid.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it must returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 DoDrawPaths(GDrawStyle& Style, const GDynArray<GCurve2D *>& Curves) = 0;
		/*!
			Draw the current cache bank slots. Here it's ensured that current cache bank is non-NULL and
			that FirstSlotIndex <= LastSlotIndex.

			\param Style the drawstyle to use.
			\param FirstSlotIndex first cache slot (index) to draw. It's ensured to be inside the valid range.
			\param LastSlotIndex last cache slot (index) to draw. It's ensured to be inside the valid range.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void DoDrawCacheSlots(GDrawStyle& Style, const GUInt32 FirstSlotIndex, const GUInt32 LastSlotIndex) = 0;
		/*!
			Do the effective screenshot.

			This function permits to grab a rectangular portion of the framebuffer, and put it into
			a bitmap. Here it's ensured that P0 <= P1.

			\param Output the output image, where grabbed portion will be copied into.
			\param P0 a corner of the rectangle portion to grab.
			\param P1 the opposite (to P0) corner of the rectangle.
			\return G_NO_ERROR if the operation succeeds, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GError DoScreenShot(GPixelMap& Output, const GVectBase<GUInt32, 2>& P0, const GVectBase<GUInt32, 2>& P1) const = 0;


		inline GDrawStyle *CurrentStyle() {
			return gCurrentContext.gDrawStyle;
		}
		void InitDrawStyle();
		/*!
			Create a new draw style.

			A drawboard implementation could create an extended drawstyle structure (derived from GDrawStyle)
			and associate with it implementation-dependent data.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GDrawStyle *CreateDrawStyle() const = 0;

	public:
		//! Default constructor.
		GDrawBoard();
		//! Destructor.
		virtual ~GDrawBoard();
		/*!
			Get the maximum supported number of dash entries.

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GUInt32 MaxDashCount() const = 0;
		/*!
			Get the maximum supported number of color keys.

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GUInt32 MaxColorKeys() const = 0;
		/*!
			Get the maximum supported pattern width (in pixels).

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GUInt32 MaxImageWidth() const = 0;
		/*!
			Get the maximum supported pattern height (in pixels).

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GUInt32 MaxImageHeight() const = 0;
		/*!
			Get the maximum supported pattern memory size (in bytes).

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GUInt32 MaxImageBytes() const = 0;
		/*!
			Create a linear gradient.

			\param StartPoint the point corresponding to the first color key.
			\param EndPoint the point corresponding to the last color key.
			\param ColorKeys color keys, they must be sorted by time (in ascending order).
			\param Interpolation the interpolation schema to adopt during pixel color evaluation.
			\param SpreadMode indicates what happens if the gradient starts or ends inside the bounds of the target
			region.
			\param Matrix an optional gradient matrix, used to transform the gradient into another coordinates system.
			\return a structure containing the specified gradient, NULL if the gradient can't be created.
			\note this method <b>MUST</b> be implemented by all derived classes. The user <b>MUST NOT</b> delete the
			returned pointer (the drawboard will care of it).
		*/
		virtual GGradientDesc *CreateLinearGradient(const GPoint2& StartPoint, const GPoint2& EndPoint,
													const GDynArray<GKeyValue>& ColorKeys,
													const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
													const GColorRampSpreadMode SpreadMode = G_PAD_COLOR_RAMP_SPREAD,
													const GMatrix33& Matrix = G_MATRIX_IDENTITY33) = 0;
		/*!
			Create a radial gradient.

			\param Center the center of gradient, it will be in correspondence of the first color key.
			\param Focus the focus point.
			\param Radius the radius of the gradient. Points at radius distances from center will be colored using
			last color key.
			\param ColorKeys color keys, they must be sorted by time (in ascending order).
			\param Interpolation the interpolation schema to adopt during pixel color evaluation.
			\param SpreadMode indicates what happens if the gradient starts or ends inside the bounds of the target
			region.
			\param Matrix an optional gradient matrix, used to transform the gradient into another coordinates system.
			\return a structure containing the specified gradient, NULL if the gradient can't be created.
			\note this method <b>MUST</b> be implemented by all derived classes. The user <b>MUST NOT</b> delete the
			returned pointer (the drawboard will care of it).
		*/
		virtual GGradientDesc *CreateRadialGradient(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
													const GDynArray<GKeyValue>& ColorKeys,
													const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
													const GColorRampSpreadMode SpreadMode = G_PAD_COLOR_RAMP_SPREAD,
													const GMatrix33& Matrix = G_MATRIX_IDENTITY33) = 0;
		/*!
			Create a conical gradient.

			\param Center the center of gradient, this is the cone vertex point.
			\param Target it specifies a 'target' point, that coupled with start point defines a direction
			associated to the first color key.
			\param ColorKeys color keys, they must be sorted by time (in ascending order).
			\param Interpolation the interpolation schema to adopt during pixel color evaluation.
			\param Matrix an optional gradient matrix, used to transform the gradient into another coordinates system.
			\return a structure containing the specified gradient, NULL if the gradient can't be created.
			\note this method <b>MUST</b> be implemented by all derived classes. The user <b>MUST NOT</b> delete the
			returned pointer (the drawboard will care of it).
		*/
		virtual GGradientDesc *CreateConicalGradient(const GPoint2& Center, const GPoint2& Target,
													 const GDynArray<GKeyValue>& ColorKeys,
													 const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
													 const GMatrix33& Matrix = G_MATRIX_IDENTITY33) = 0;
		/*!
			Create a bitmap pattern.

			\param Image a source bitmap image, must be valid.
			\param Quality a quality parameter that must be used when the pattern will be drawn on screen.
			\param TilingMode defines possible methods for defining colors for source pixels that lie outside the bounds
			of the source image.
			\param LogicalWindow defines a reference rectangle somewhere on the	infinite canvas. The tiling
			extends a series of such rectangles to infinity in X and Y (positive and negative). If NULL is specified
			a default rectangle (going from the lowleft logical corner with a correct ratio) will be set.
			\param Matrix an optional pattern matrix, used to transform the pattern into another coordinates system.
			\return a structure containing the specified pattern, NULL if the pattern can't be created.
			\note this method <b>MUST</b> be implemented by all derived classes. The user <b>MUST NOT</b> delete the
			returned pointer (the drawboard will care of it).
		*/
		virtual GPatternDesc *CreatePattern(const GPixelMap *Image, const GImageQuality Quality,
											const GTilingMode TilingMode = G_REPEAT_TILE,
											const GAABox2 *LogicalWindow = NULL,
											const GMatrix33& Matrix = G_MATRIX_IDENTITY33) = 0;
		/*!
			Create a new cache bank.

			\return the created cache bank if operation succeeds, else a NULL pointer.
			\note this method <b>MUST</b> be implemented by all derived classes. The user <b>MUST NOT</b> delete the
			returned pointer (the drawboard will care of it).
		*/
		virtual GCacheBank *CreateCacheBank() = 0;
		/*!
			Get current cache bank. NULL if a cache bank hasn't already been set.

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GCacheBank *CacheBank() const = 0;
		/*!
			Set current cache bank, a NULL value is valid.

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void SetCacheBank(GCacheBank *Bank) = 0;


		//---------------------------------------------------------------------------
		//                             RENDERING CONTEXT
		//---------------------------------------------------------------------------
		//! Get group opacity.
		GReal GroupOpacity() const;
		//! Set group opacity.
		void SetGroupOpacity(const GReal Opacity);
		//! Get rendering quality settings.
		GRenderingQuality RenderingQuality() const;
		//! Set rendering quality settings.
		void SetRenderingQuality(const GRenderingQuality Quality);
		//! Get current target mode (target buffer).
		GTargetMode TargetMode() const;
		//! Set current target mode (target buffer).
		void SetTargetMode(const GTargetMode Mode);
		//! Get clip operation currently used for clip paths.
		GClipOperation ClipOperation() const;
		//! Set clip operation to use for clip paths.
		void SetClipOperation(const GClipOperation Operation);
		//! Get clip enable/disable flag.
		GBool ClipEnabled() const;
		//! Enable (G_TRUE value) or disable (G_FALSE value) clip paths.
		void SetClipEnabled(const GBool Enabled);
		//! Pop last clip path from stack.
		void PopClipMask();
		//! Get model-view matrix.
		const GMatrix33& ModelViewMatrix() const;
		//! Set model-view matrix.
		void SetModelViewMatrix(const GMatrix33& Matrix);
		//---------------------------------------------------------------------------
		//                                  STROKE
		//---------------------------------------------------------------------------
		//! Get stroke startcap style.
		GCapStyle StrokeStartCapStyle() const;
		//! Set stroke startcap style.
		void SetStrokeStartCapStyle(const GCapStyle Style);
		//! Get stroke endcap style.
		GCapStyle StrokeEndCapStyle() const;
		//! Set stroke endcap style.
		void SetStrokeEndCapStyle(const GCapStyle Style);
		//! Get stroke join style.
		GJoinStyle StrokeJoinStyle() const;
		//! Set stroke join style.
		void SetStrokeJoinStyle(const GJoinStyle Style);
		//! Get stroke width (in logical units).
		GReal StrokeWidth() const;
		//! Set stroke width (in logical units).
		void SetStrokeWidth(const GReal Width);
		//! Get stroke miter limit.
		GReal StrokeMiterLimit() const;
		//! Set stroke miter limit.
		void SetStrokeMiterLimit(const GReal Limit);
		//! Get stroke style.
		GStrokeStyle StrokeStyle() const;
		//! Set stroke style.
		void SetStrokeStyle(const GStrokeStyle Style);
		//! Get stroke dash pattern.
		const GDynArray<GReal>& StrokeDashPattern() const;
		//! Set stroke dash pattern.
		GError SetStrokeDashPattern(const GDynArray<GReal>& Pattern);
		//! Get stroke dash phase (known also as dash offset).
		GReal StrokeDashPhase() const;
		//! Set stroke dash phase (known also as dash offset).
		void SetStrokeDashPhase(const GReal DashPhase);
		//! Get stroke paint type.
		GPaintType StrokePaintType() const;
		//! Set stroke paint type.
		void SetStrokePaintType(const GPaintType PaintType);
		//! Get stroke color.
		const GVectBase<GReal, 4>& StrokeColor() const;
		//! Set stroke color, specifying color components.
		void SetStrokeColor(const GVectBase<GReal, 4>& Color);
		//! Set stroke color, specifying color components.
		inline void SetStrokeColor(const GReal Red, const GReal Green, const GReal Blue, const GReal Alpha) {
			SetStrokeColor(GVectBase<GReal, 4>(Red, Green, Blue, Alpha));
		}
		//! Get stroke gradient (that will be used when stroke paint type is GRADIENT).
		GGradientDesc *StrokeGradient() const;
		//! Set stroke gradient (that will be used when stroke paint type is GRADIENT).
		void SetStrokeGradient(GGradientDesc *Gradient);
		//! Get stroke pattern (that will be used when stroke paint type is PATTERN).
		GPatternDesc *StrokePattern() const;
		//! Set stroke pattern (that will be used when stroke paint type is PATTERN).
		void SetStrokePattern(GPatternDesc *Pattern);
		//! It returns G_TRUE if stroke painting is enabled, else G_FALSE.
		GBool StrokeEnabled() const;
		//! Enable (G_TRUE value) or disable (G_FALSE value) stroke painting.
		void SetStrokeEnabled(const GBool Enabled);

		//---------------------------------------------------------------------------
		//                                  FILL
		//---------------------------------------------------------------------------
		//! Get fill rule currently used.
		GFillRule FillRule() const;
		//! Set fill rule.
		void SetFillRule(const GFillRule Rule);
		//! Get fill paint type.
		GPaintType FillPaintType() const;
		//! Set fill paint type.
		void SetFillPaintType(const GPaintType PaintType);
		//! Get fill color.
		const GVectBase<GReal, 4>& FillColor() const;
		//! Set fill color, specifying color components.
		void SetFillColor(const GVectBase<GReal, 4>& Color);
		//! Set fill color, specifying color components.
		inline void SetFillColor(const GReal Red, const GReal Green, const GReal Blue, const GReal Alpha) {
			SetFillColor(GVectBase<GReal, 4>(Red, Green, Blue, Alpha));
		}
		//! Get fill gradient (that will be used when stroke paint type is GRADIENT).
		GGradientDesc *FillGradient() const;
		//! Set fill gradient (that will be used when stroke paint type is GRADIENT).
		void SetFillGradient(GGradientDesc *Gradient);
		//! Get fill pattern (that will be used when stroke paint type is PATTERN).
		GPatternDesc *FillPattern() const;
		//! Set fill pattern (that will be used when stroke paint type is PATTERN).
		void SetFillPattern(GPatternDesc *Pattern);
		//! It returns G_TRUE if fill painting is enabled, else G_FALSE.
		GBool FillEnabled() const;
		//! Enable (G_TRUE value) or disable (G_FALSE value) fill painting.
		void SetFillEnabled(const GBool Enabled);

		//---------------------------------------------------------------------------
		//                           PRIMITIVES & COMMANDS
		//---------------------------------------------------------------------------
		/*!
			Draw a line, going form P0 to P1.

			\param P0 first end point.
			\param P1 last end point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawLine(const GPoint2& P0, const GPoint2& P1);
		/*!
			Draw a line, going form (X0, Y0) to (X1, Y1).

			\param X0 first end point abscissa.
			\param Y0 first end point ordinate.
			\param X1 last end point abscissa.
			\param Y1 first end point ordinate.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawLine(const GReal X0, const GReal Y0, const GReal X1, const GReal Y1) {
			return DrawLine(GPoint2(X0, Y0), GPoint2(X1, Y1));
		}
		/*!
			Draw a quadratic Bezier curve, going form P0 to P2 and having P1 as central control point.

			\param P0 first curve point.
			\param P1 control point.
			\param P2 last curve point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawBezier(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2);
		/*!
			Draw a quadratic Bezier curve, going form (X0, Y0) to (X2, Y2) and having (X1, Y1) as central control point.

			\param X0 first curve point abscissa.
			\param Y0 first curve point ordinate.
			\param X1 control point abscissa.
			\param Y1 control point ordinate.
			\param X2 last curve point abscissa.
			\param Y2 last curve point ordinate.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawBezier(const GReal X0, const GReal Y0, const GReal X1, const GReal Y1,
							   const GReal X2, const GReal Y2) {
			return DrawBezier(GPoint2(X0, Y0), GPoint2(X1, Y1), GPoint2(X2, Y2));
		}
		/*!
			Draw a cubic Bezier curve, going form P0 to P3 and having P1 and P2 as control points.

			\param P0 first curve point.
			\param P1 first control point.
			\param P2 second control point.
			\param P3 last curve point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawBezier(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3);
		/*!
			Draw a cubic Bezier curve, going form (X0, Y0) to (X3, Y3) and having (X1, Y1) and (X2, Y2) as control points.

			\param X0 first curve point abscissa.
			\param Y0 first curve point ordinate.
			\param X1 first control point abscissa.
			\param Y1 first control point ordinate.
			\param X2 second control point abscissa.
			\param Y2 second control point ordinate.
			\param X3 last curve point abscissa.
			\param Y3 last curve point ordinate.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawBezier(const GReal X0, const GReal Y0, const GReal X1, const GReal Y1,
							   const GReal X2, const GReal Y2, const GReal X3, const GReal Y3) {
			return DrawBezier(GPoint2(X0, Y0), GPoint2(X1, Y1), GPoint2(X2, Y2), GPoint2(X3, Y3));
		}
		/*!
			Draw an ellipse arc.

			\param Center the center of the ellipse.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It must be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It must be greater than 0.
			\param OffsetRotation an optional offset rotation (expressed in radians) relative to the x-axis.
			\param StartAngle an angle (in radians) that defines the start point of the arc.
			\param EndAngle an angle (in radians) that defines the end point of the arc.
			\param CCW if G_TRUE the ellipse arc has to go from start point to end point in counter clockwise
			direction, else G_FALSE (clockwise direction).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawEllipseArc(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
							  const GReal OffsetRotation = 0, const GReal StartAngle = 0, const GReal EndAngle = G_2PI,
							  const GBool CCW = G_TRUE);
		/*!
			Draw an ellipse arc.

			\param X the center abscissa of the ellipse.
			\param Y the center ordinate of the ellipse.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It must be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It must be greater than 0.
			\param OffsetRotation an optional offset rotation (expressed in radians) relative to the x-axis.
			\param StartAngle an angle (in radians) that defines the start point of the arc.
			\param EndAngle an angle (in radians) that defines the end point of the arc.
			\param CCW if G_TRUE the ellipse arc has to go from start point to end point in counter clockwise
			direction, else G_FALSE (clockwise direction).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawEllipseArc(const GReal X, const GReal Y, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
									 const GReal OffsetRotation = 0, const GReal StartAngle = 0, const GReal EndAngle = G_2PI,
									 const GBool CCW = G_TRUE) {
			return DrawEllipseArc(GPoint2(X, Y), XSemiAxisLength, YSemiAxisLength, OffsetRotation, StartAngle, EndAngle, CCW);
		}
		/*!
			Draw an ellipse arc, using a two-points fit schema.

			\param P0 the start point of the arc.
			\param P1 the end point of the arc.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It must be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It must be greater than 0.
			\param OffsetRotation an optional offset rotation (expressed in radians) relative to the x-axis.
			\param LargeArc if G_TRUE the ellipse arc has to go from start point to end point spanning the
			longest chord, else G_FALSE.
			\param CCW if G_TRUE the ellipse arc has to go from start point to end point in counter clockwise
			direction, else G_FALSE (clockwise direction).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawEllipseArc(const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
							  const GReal OffsetRotation = 0, const GBool LargeArc = G_TRUE, const GBool CCW = G_FALSE);

		/*!
			Draw an ellipse arc, using a two-points fit schema.

			\param X0 the start point abscissa of the arc.
			\param Y0 the start point ordinate of the arc.
			\param X1 the end point abscissa of the arc.
			\param Y1 the end point abscissa of the arc.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It must be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It must be greater than 0.
			\param OffsetRotation an optional offset rotation (expressed in radians) relative to the x-axis.
			\param LargeArc if G_TRUE the ellipse arc has to go from start point to end point spanning the
			longest chord, else G_FALSE.
			\param CCW if G_TRUE the ellipse arc has to go from start point to end point in counter clockwise
			direction, else G_FALSE (clockwise direction).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawEllipseArc(const GReal X0, const GReal Y0, const GReal X1, const GReal Y1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
									 const GReal OffsetRotation = 0, const GBool LargeArc = G_TRUE, const GBool CCW = G_FALSE) {
			return DrawEllipseArc(GPoint2(X0, Y0), GPoint2(X1, Y1), XSemiAxisLength, YSemiAxisLength, OffsetRotation, LargeArc, CCW);
		}
		/*!
			Draw a polygon.

			\param Points the set of consecutive points that define the polygon.
			\param Closed if G_TRUE the polygon is considered closed (an imaginary edge goes from last point to
			the first one).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawPolygon(const GDynArray<GPoint2>& Points, const GBool Closed);
		/*!
			Draw a rectangle.

			\param P0 a corner of the rectangle.
			\param P1 the opposite (to P0) corner of the rectangle.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawRectangle(const GPoint2& P0, const GPoint2& P1);
		/*!
			Draw a rectangle.

			\param X0 a corner abscissa of the rectangle.
			\param Y0 a corner ordinate of the rectangle.
			\param X1 the opposite (to X0) corner abscissa of the rectangle.
			\param Y1 the opposite (to Y0) corner ordinate of the rectangle.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawRectangle(const GReal X0, const GReal Y0, const GReal X1, const GReal Y1) {
			return DrawRectangle(GPoint2(X0, Y0), GPoint2(X1, Y1));
		}
		/*!
			Draw a rounded rectangle.

			\param P0 a corner of the rectangle.
			\param P1 the opposite (to P0) corner of the rectangle.
			\param ArcWidth the x-axis radius of the ellipse used to round off the corners of the rectangle. It must be positive.
			\param ArcHeight the y-axis radius of the ellipse used to round off the corners of the rectangle. It must be positive.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawRoundRectangle(const GPoint2& P0, const GPoint2& P1, const GReal ArcWidth, const GReal ArcHeight);
		/*!
			Draw a rounded rectangle.

			\param X0 a corner abscissa of the rectangle.
			\param Y0 a corner ordinate of the rectangle.
			\param X1 the opposite (to X0) corner abscissa of the rectangle.
			\param Y1 the opposite (to Y0) corner ordinate of the rectangle.
			\param ArcWidth the x-axis radius of the ellipse used to round off the corners of the rectangle. It must be positive.
			\param ArcHeight the y-axis radius of the ellipse used to round off the corners of the rectangle. It must be positive.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawRoundRectangle(const GReal X0, const GReal Y0, const GReal X1, const GReal Y1,
										 const GReal ArcWidth, const GReal ArcHeight) {
			return DrawRoundRectangle(GPoint2(X0, Y0), GPoint2(X1, Y1), ArcWidth, ArcHeight);
		}
		/*!
			Draw an ellipse.

			\param Center the center of the ellipse.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It must be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It must be greater than 0.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawEllipse(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength);
		/*!
			Draw an ellipse.

			\param X the center abscissa of the ellipse.
			\param Y the center ordinate of the ellipse.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It must be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It must be greater than 0.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawEllipse(const GReal X, const GReal Y, const GReal XSemiAxisLength, const GReal YSemiAxisLength) {
			return DrawEllipse(GPoint2(X, Y), XSemiAxisLength, YSemiAxisLength);
		}
		/*!
			Draw a circle.

			\param Center the center of the circle.
			\param Radius the radius of the circle, it must be positive.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawCircle(const GPoint2& Center, const GReal Radius);
		/*!
			Draw a circle.

			\param X the center abscissa of the circle.
			\param Y the center ordinate of the circle.
			\param Radius the radius of the circle, it must be positive.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawCircle(const GReal X, const GReal Y, const GReal Radius) {
			return DrawCircle(GPoint2(X, Y), Radius);
		}
		/*!
			Draw a single curve/path.

			This method permits to draw a single curve, intended as a class derived from GCurve2D.\n
			\param Curve the curve to draw, it must consist of at least 2 points.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawPath(const GCurve2D& Curve);
		/*!
			Draw a set of curves and paths.

			This method permits to draw complex shapes, made of more than 1 curve/path, using the currently set
			fill rule.
			\param Curves an array of curves pointers. Each pointer must be valid.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawPaths(const GDynArray<GCurve2D *>& Curves);
		/*!
			Draw a (multi)path, specifying an SVG description.

			\param SVGPathDescription an non-empty string, describing the path in the classic SVG form (the well
			known 'd' attribute. For example "M 406,74 C 257,67 305,163 359,313 Q 406,361 454,314 C 496,164 544,67 406,74 z".
			\param AnglesMeasureUnits the system units into which angles are expressed inside the string.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DrawPaths(const GString& SVGPathDescription, const GAnglesMeasureUnit AnglesMeasureUnits = G_DEGREE_UNIT);
		/*!
			Draw a (multi)path, specifying an SVG description.

			\param SVGPathDescription an non-empty string, describing the path in the classic SVG form (the well
			known 'd' attribute. For example "M 406,74 C 257,67 305,163 359,313 Q 406,361 454,314 C 496,164 544,67 406,74 z".
			\param AnglesMeasureUnits the system units into which angles are expressed inside the string.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		inline GInt32 DrawPaths(const GChar8 *SVGPathDescription, const GAnglesMeasureUnit AnglesMeasureUnits = G_DEGREE_UNIT) {
			return DrawPaths(GString(SVGPathDescription), AnglesMeasureUnits);
		}
		/*!
			Start an SVG-like path block.
			Each opened block must be closed calling EndPaths() function.

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void BeginPaths() = 0;
		/*!
			Begin an SVG-like sub-path.

			This function is in unique correspondence with SVG path command 'M'/'m'.

			\param P the start point.
			\param Relative if G_TRUE start point coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void MoveTo(const GPoint2& P, const GBool Relative) = 0;
		/*!
			Begin an SVG-like sub-path.

			This function is in unique correspondence with SVG path command 'M'/'m'.

			\param X the x component of the start point.
			\param Y the y component of the start point.
			\param Relative if G_TRUE start point coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		inline void MoveTo(const GReal X, const GReal Y, const GBool Relative) {
			MoveTo(GPoint2(X, Y), Relative);
		}
		/*!
			Draw a line from the current cursor position to the specified point.

			\param P the endpoint of the line.
			\param Relative if G_TRUE end point coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void LineTo(const GPoint2& P, const GBool Relative) = 0;
		/*!
			Draw a line from the current cursor position to the specified point.

			\param X the x component of the end point.
			\param Y the y component of the end point.
			\param Relative if G_TRUE end point coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		inline void LineTo(const GReal X, const GReal Y, const GBool Relative) {
			LineTo(GPoint2(X, Y), Relative);
		}
		/*!
			Draw an horizontal line from the current cursor position to the specified location.

			\param X the endpoint abscissa.
			\param Relative if G_TRUE end point coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void HorizontalLineTo(const GReal X, const GBool Relative) = 0;
		/*!
			Draw a vertical line from the current cursor position to the specified location.

			\param Y the endpoint ordinate.
			\param Relative if G_TRUE end point coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void VerticalLineTo(const GReal Y, const GBool Relative) = 0;
		/*!
			Draw a cubic Bezier curve from current cursor position to P, using P1 as the control point at the
			beginning of the curve and P2 as the control point at the end of the curve.

			\param P1 control point at the beginning of the curve.
			\param P2 control point at the end of the curve.
			\param P curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void CurveTo(const GPoint2& P1, const GPoint2& P2, const GPoint2& P, const GBool Relative) = 0;
		/*!
			Draw a cubic Bezier curve from current cursor position to (X, Y), using (X1, Y1) as the control point at the
			beginning of the curve and (X2, Y2) as the control point at the end of the curve.

			\param X1 abscissa of the control point at the beginning of the curve.
			\param Y1 ordinate of the control point at the beginning of the curve.
			\param X2 abscissa of the control point at the end of the curve.
			\param Y2 ordinate of the control point at the end of the curve.
			\param X abscissa of the curve end point.
			\param Y ordinate of the curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		inline void CurveTo(const GReal X1, const GReal Y1, const GReal X2, const GReal Y2,
							const GReal X, const GReal Y, const GBool Relative) {
			CurveTo(GPoint2(X1, Y1), GPoint2(X2, Y2), GPoint2(X, Y), Relative);
		}
		/*!
			Draw a quadratic Bezier curve from current cursor position to P, using P1 as control point.

			\param P1 control point.
			\param P curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void CurveTo(const GPoint2& P1, const GPoint2& P, const GBool Relative) = 0;
		/*!
			Draw a quadratic Bezier curve from current cursor position to P, using P1 as control point.

			\param X1 abscissa of the control point.
			\param Y1 ordinate of the control point.
			\param X abscissa of the curve end point.
			\param Y ordinate of the curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		inline void CurveTo(const GReal X1, const GReal Y1, const GReal X, const GReal Y, const GBool Relative) {
			CurveTo(GPoint2(X1, Y1), GPoint2(X, Y), Relative);
		}
		/*!
			Draw a cubic Bezier curve using smooth tangent.

			Draws a cubic Bezier curve from	the current cursor position to P. The first control point is assumed to
			be the reflection of the second control point on the previous command relative to the current point.
			At the end of the command, the new cursor position becomes P.

			\param P2 the second control point.
			\param P the final curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void SmoothCurveTo(const GPoint2& P2, const GPoint2& P, const GBool Relative) = 0;
		/*!
			Draw a cubic Bezier curve using smooth tangent.

			Draws a cubic Bezier curve from	the current cursor position to (X, Y). The first control point is assumed to
			be the reflection of the second control point on the previous command relative to the current point.
			At the end of the command, the new cursor position becomes P.

			\param X2 the second control point abscissa.
			\param Y2 the second control point ordinate.
			\param X the final curve end point abscissa.
			\param Y the final curve end point ordinate.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void SmoothCurveTo(const GReal X2, const GReal Y2, const GReal X, const GReal Y, const GBool Relative) {
			SmoothCurveTo(GPoint2(X2, Y2), GPoint2(X, Y), Relative);
		}
		/*!
			Draw a quadratic Bezier curve using smooth tangent.

			Draws a quadratic Bezier curve from	the current cursor position to P. The control point is assumed to
			be the reflection of the control point on the previous command relative to the current point.
			At the end of the command, the new cursor position becomes P.

			\param P the final curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void SmoothCurveTo(const GPoint2& P, const GBool Relative) = 0;
		/*!
			Draw a quadratic Bezier curve using smooth tangent.

			Draws a quadratic Bezier curve from	the current cursor position to (X, Y). The control point is assumed to
			be the reflection of the control point on the previous command relative to the current point.
			At the end of the command, the new cursor position becomes (X, Y).

			\param X the final curve end point abscissa.
			\param Y the final curve end point ordinate.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void SmoothCurveTo(const GReal X, const GReal Y, const GBool Relative) {
			SmoothCurveTo(GPoint2(X, Y), Relative);
		}
		/*!
			Draw an elliptical arc from the current cursor position to P. The size and orientation of
			the ellipse are defined by two radii (Rx, Ry) and an x-axis-rotation (XRot, expressed in radians), which
			indicates how the ellipse as a whole is rotated relative to the current coordinate system.
			The center of the ellipse is calculated automatically to satisfy the constraints imposed by the other
			parameters. LargeArc flag and Sweep flag contribute to the automatic calculations and help determine
			how the	arc is drawn.

			\param Rx x-axis radius.
			\param Ry y-axis radius.
			\param XRot an offset rotation (in radians) which indicates	how the ellipse as a whole is rotated
			relative to the current coordinate system.
			\param LargeArc if G_TRUE then one of the two larger arc sweeps will be chosen; otherwise, if G_FALSE
			one of the smaller arc sweeps will be chosen.
			\param Sweep if G_TRUE then the arc will be drawn in a "positive-angle" direction. A value of G_FALSE
			causes the arc to be drawn in a	"negative-angle" direction.
			\param P the final endpoint to reach.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void EllipticalArcTo(const GReal Rx, const GReal Ry, const GReal XRot, const GBool LargeArc,
									const GBool Sweep, const GPoint2& P, const GBool Relative) = 0;
		/*!
			Draw an elliptical arc from the current cursor position to (X, Y). The size and orientation of
			the ellipse are defined by two radii (Rx, Ry) and an x-axis-rotation (XRot, expressed in radians), which
			indicates how the ellipse as a whole is rotated relative to the current coordinate system.
			The center of the ellipse is calculated automatically to satisfy the constraints imposed by the other
			parameters. LargeArc flag and Sweep flag contribute to the automatic calculations and help determine
			how the	arc is drawn.

			\param Rx x-axis radius.
			\param Ry y-axis radius.
			\param XRot an offset rotation (in radians) which indicates	how the ellipse as a whole is rotated
			relative to the current coordinate system.
			\param LargeArc if G_TRUE then one of the two larger arc sweeps will be chosen; otherwise, if G_FALSE
			one of the smaller arc sweeps will be chosen.
			\param Sweep if G_TRUE then the arc will be drawn in a "positive-angle" direction. A value of G_FALSE
			causes the arc to be drawn in a	"negative-angle" direction.
			\param X the final endpoint abscissa to reach.
			\param Y the final endpoint ordinate to reach.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		inline void EllipticalArcTo(const GReal Rx, const GReal Ry, const GReal XRot, const GBool LargeArc,
									const GBool Sweep, const GReal X, const GReal Y, const GBool Relative) {
			EllipticalArcTo(Rx, Ry, XRot, LargeArc, Sweep, GPoint2(X, Y), Relative);
		}
		/*!
			Close the current SVG-like sub-path.

			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual void ClosePath() = 0;
		/*!
			Close an SVG-like path block.

			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
			\note this method <b>MUST</b> be implemented by all derived classes.
		*/
		virtual GInt32 EndPaths() = 0;
		/*!
			Draw a range of cache slots, taken from the current cache bank.

			\param FirstSlotIndex first cache slot (index) to draw. It must be valid.
			\param LastSlotIndex last cache slot (index) to draw. It must be valid.
		*/
		void DrawCacheSlots(const GUInt32 FirstSlotIndex, const GUInt32 LastSlotIndex);
		/*!
			Draw the entire current cache bank.
		*/
		inline void DrawCacheBank() {
			if (CacheBank() && CacheBank()->SlotsCount() > 0)
				DrawCacheSlots(0, CacheBank()->SlotsCount() - 1);
		}
		/*!
			Draw a single cache slot of the current cache bank.

			\param SlotIndex cache slot (index) to draw. It must be valid.
		*/
		inline void DrawCacheSlot(const GUInt32 SlotIndex) {
			if (CacheBank() && SlotIndex < CacheBank()->SlotsCount())
				DrawCacheSlots(SlotIndex, SlotIndex);
		}
		/*!
			Clear the drawboard.

			This methods permits to clear the entire drawboard, using s specified color. In the same 'atomic'
			operation the clipbuffer can be cleared too.
			\param Red the red component of the clear color.
			\param Green the green component of the clear color.
			\param Blue the blue component of the clear color.
			\param ClearClipMasks if G_TRUE also clipbuffer will be cleared. In this case all existing masks will
			be dropped.
		*/
		void Clear(const GReal Red, const GReal Green, const GReal Blue, const GBool ClearClipMasks = G_TRUE);
		/*!
			This function ensures that all outstanding requests on the current context will complete in finite time.
			Flush may return prior to the actual completion of all requests.
		*/
		void Flush();
		/*!
			This function blocks until all drawing operations are completed.
			This function does not return until the effects of all previously called drawing functions are complete.
			Such effects include all changes to the states and all changes to the framebuffer contents.
		*/
		void Finish();
		/*!
			Open a group block.

			If the drawboard is already inside a group constructor (you can check this with InsideGroup() function),
			this operation has no effects.\n
			All drawing operations done inside a group block will be subjected to the group opacity currently
			set.
		*/
		void GroupBegin();
		/*!
			Open a group block, specifying a zone (a box) that will be affected by successive drawing
			operations. This version permits heavy optimizations by the rendering system.
		*/
		void GroupBegin(const GAABox2& LogicBox);
		/*!
			Close a group block.

			If the drawboard is not inside a group block (you can check this with InsideGroup() function) this
			operation has no effects.
		*/
		void GroupEnd();
		/*!
			Get current physical viewport.

			\param LowLeftCornerX lowleft corner abscissa of the viewport.
			\param LowLeftCornerY lowleft corner ordinate of the viewport.
			\param Width width of the viewport expressed in pixels.
			\param Height height of the viewport expressed in pixels.
		*/
		void Viewport(GUInt32& LowLeftCornerX, GUInt32& LowLeftCornerY, GUInt32& Width, GUInt32& Height);
		/*!
			Set current physical viewport.

			\param LowLeftCornerX lowleft corner abscissa of the viewport.
			\param LowLeftCornerY lowleft corner ordinate of the viewport.
			\param Width width of the viewport expressed in pixels.
			\param Height height of the viewport expressed in pixels.
		*/
		void SetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY, const GUInt32 Width, const GUInt32 Height);
		/*!
			Get current logical viewport.
			
			Logical viewport (that defines implicitly a projection matrix) is used to select a rectangular portion of
			infinitive canvas. This can be used to do all kind of panning and zooming operations.

			\param Left the abscissa of the bottom-left corner of current logical window.
			\param Right the abscissa of the top-right corner of current logical window.
			\param Bottom the ordinate of the bottom-left corner of current logical window.
			\param Top the ordinate of the top-right corner of current logical window.
		*/
		void Projection(GReal& Left, GReal& Right, GReal& Bottom, GReal& Top);
		/*!
			Set current logical viewport.
			
			Logical viewport (that defines implicitly a projection matrix) is used to select a rectangular portion of
			infinitive canvas. This can be used to do all kind of panning and zooming operations.

			\param Left the abscissa of the bottom-left corner of current logical window.
			\param Right the abscissa of the top-right corner of current logical window.
			\param Bottom the ordinate of the bottom-left corner of current logical window.
			\param Top the ordinate of the top-right corner of current logical window.
		*/
		void SetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top);
		/*!
			Coordinates conversion from logical to physical.

			\param LogicalPoint a point whose coordinates are expressed inside the logical window.
			\return the same point expressed inside the physical viewport.
		*/
		GPoint<GInt32, 2> LogicalToPhysicalInt(const GPoint2& LogicalPoint);
		/*!
			Coordinates conversion from logical to physical.

			\param LogicalPoint a point whose coordinates are expressed inside the logical window.
			\return the same point expressed inside the physical viewport.
		*/
		GPoint2 LogicalToPhysicalReal(const GPoint2& LogicalPoint);
		/*!
			Coordinates conversion from physical to logical.

			\param PhysicalPoint a point whose coordinates are expressed inside the physical window/viewport.
			\return the same point expressed inside the logical window.
		*/
		GPoint2 PhysicalToLogical(const GPoint<GInt32, 2>& PhysicalPoint);
		/*!
			Take a screenshot.

			This function permits to grab a rectangular portion of the framebuffer, and put it into
			a bitmap.

			\param Output the output image, where grabbed portion will be copied into.
			\param P0 a corner of the rectangle portion to grab.
			\param P1 the opposite (to P0) corner of the rectangle.
			\return G_NO_ERROR if the operation succeeds, else an error code.
		*/
		GError ScreenShot(GPixelMap& Output, const GVectBase<GUInt32, 2>& P0, const GVectBase<GUInt32, 2>& P1) const;
		/*!
			Take a screenshot.

			This function permits to grab a rectangular portion of the framebuffer, and put it into
			a bitmap.

			\param Output the output image, where grabbed portion will be copied into.
			\param X0 a corner abscissa of the rectangle portion to grab.
			\param Y0 a corner ordinate of the rectangle portion to grab.
			\param X1 the opposite (to X0) corner abscissa of the rectangle.
			\param Y1 the opposite (to Y0) corner ordinate of the rectangle.
			\return G_NO_ERROR if the operation succeeds, else an error code.
		*/
		inline GError ScreenShot(GPixelMap& Output, const GUInt32 X0, const GUInt32 Y0,
								 const GUInt32 X1, const GUInt32 Y1) const {
			return ScreenShot(Output, GVectBase<GUInt32, 2>(X0, Y0), GVectBase<GUInt32, 2>(X1, Y1));
		}
		/*!
			Take a screenshot.

			This function permits to grab the whole screen framebuffer, and put it into a bitmap.

			\param Output the output image, where grabbed portion will be copied into.
			\return G_NO_ERROR if the operation succeeds, else an error code.
		*/
		inline GError ScreenShot(GPixelMap& Output) const {
			return ScreenShot(Output, GPoint<GUInt32, 2>(gViewport[G_X], gViewport[G_Y]),
							  GVectBase<GUInt32, 2>(gViewport[G_X] + gViewport[G_Z] - 1, gViewport[G_Y] + gViewport[G_W] - 1));
		}
		/*!
			Get a matrix that transforms a physical point to its logical representation.
		*/
		GMatrix33 PhysicalToLogicalMatrix() const;
		//! Returns G_TRUE if current rendering operations are inside a GroupBegin() / GroupEnd() constructor.
		inline GBool InsideGroup() const {
			return gInsideGroup;
		}
	};

};	// end namespace Amanith

#endif
