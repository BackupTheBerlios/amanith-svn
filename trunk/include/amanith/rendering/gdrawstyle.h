/****************************************************************************
** $file: amanith/rendering/gdrawstyle.h   0.2.0.0   edited Dec, 12 2005
**
** Draw style.
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

#ifndef GDRAWSTYLE_H
#define GDRAWSTYLE_H

#include "amanith/2d/gpixelmap.h"
#include "amanith/geometry/gaabox.h"

/*!
	\file gdrawstyle.h
	\brief Types and data structures that define render styles.
*/
namespace Amanith {

	//! Cap style
	enum GCapStyle {
		//! Butt cap style.
		G_BUTT_CAP,
		//! Round cap style.
		G_ROUND_CAP,
		//! Square cap style.
		G_SQUARE_CAP
	};

	//! Join style.
	enum GJoinStyle {
		//! Miter join style.
		G_MITER_JOIN,
		//! Round join style.
		G_ROUND_JOIN,
		//! Bevel join style.
		G_BEVEL_JOIN
	};

	//! Stroke style.
	enum GStrokeStyle {
		//! Solid stroke style.
		G_SOLID_STROKE,
		//! Dashed stroke style.
		G_DASHED_STROKE
	};

	//! Fill rule.
	enum GFillRule {
		//! Odd-even fill rule.
		G_ODD_EVEN_FILLRULE,
		//! Even-odd fill rule.
		G_EVEN_ODD_FILLRULE,
		//! Non-zero fill rule.
		G_NON_ZERO_FILLRULE,
		//! Any fill rule.
		G_ANY_FILLRULE
	};

	//! Paint type.
	enum GPaintType {
		//! Color paint type.
		G_COLOR_PAINT_TYPE,
		//! Gradient paint type.
		G_GRADIENT_PAINT_TYPE,
		//! Pattern paint type.
		G_PATTERN_PAINT_TYPE
	};

	//! Gradient type.
	enum GGradientType {
		//! Linear gradient.
		G_LINEAR_GRADIENT,
		//! Radial gradient.
		G_RADIAL_GRADIENT,
		//! Conical gradient.
		G_CONICAL_GRADIENT
	};

	//! Color ramp spread mode.
	enum GColorRampSpreadMode {
		//! Pad spread mode.
		G_PAD_COLOR_RAMP_SPREAD,
		//! Repeat spread mode.
		G_REPEAT_COLOR_RAMP_SPREAD,
		//! Reflect (known also as ping-pong) spread mode.
		G_REFLECT_COLOR_RAMP_SPREAD
	};

	//! Color interpolation.
	enum GColorRampInterpolation {
		//! Constant color interpolation.
		G_CONSTANT_COLOR_INTERPOLATION,
		//! Linear color interpolation.
		G_LINEAR_COLOR_INTERPOLATION,
		//! Hermite color interpolation.
		G_HERMITE_COLOR_INTERPOLATION
	};

	//! Tiling mode.
	enum GTilingMode {
		//! Pad tiling mode.
		G_PAD_TILE,
		//! Repeat tiling mode.
		G_REPEAT_TILE,
		//! Reflect (known also as ping-pong) tiling mode.
		G_REFLECT_TILE
	};

	//! Image/pattern quality level.
	enum GImageQuality {
		//! Low image/pattern quality.
		G_LOW_IMAGE_QUALITY,
		//! Normal image/pattern quality.
		G_NORMAL_IMAGE_QUALITY,
		//! High image/pattern quality.
		G_HIGH_IMAGE_QUALITY
	};

	// *********************************************************************
	//                            GGradientDesc
	// *********************************************************************

	/*!
		\class GGradientDesc
		\brief This class maintains informations used to describe a gradient.

		Every used gradient has an associated GGradientDesc class instance. This class describes completely a
		gradient. Maintained informations are:\n\n

		- Type: it can be LINEAR, RADIAL and CONICAL.\n\n
		\image html gradients_types.png
		\n
		- Start point: the point associated to the first color key; for conical gradients this is the cone vertex point.
		- Aux point: for linear gradients it represents an 'end' point associated with the last color key. For
		radial gradients it corresponds to the focus point, and for conical gradients it specifies a 'target'
		point that coupled with start point defines a direction associated to the first color key.
		- Radius: it has sense only for radial gradients.\n
		- Color interpolation: the interpolation schema to adopt during pixel color evaluation, it can be CONSTANT,
		LINEAR and HERMITE.\n\n
		\image html gradients_interpolations.png
		\n
		- Spread mode: indicates what happens if the gradient starts or ends inside the bounds of the target
		region. Possible values are: PAD (which says to use the terminal colors of the gradient
		to fill the remainder of the target region), REPEAT (which says to repeat the gradient pattern
		start-to-end, start-to-end, start-toend, etc. continuously until the target region is filled) and REFLECT
		(which says to reflect the gradient pattern start-to-end, end-to-start, start-to-end, etc. continuously
		until the target rectangle is filled).\n\n
		\image html gradients_spreadmodes.png
		\n
		- Color keys: an array of color keys (also known as color stops) that define used colors.\n
		- Matrix: an optional 3x3 matrix, used to transform the gradient into another coordinates system.
	*/
	class G_EXPORT GGradientDesc {

	private:
		//! Gradient type.
		GGradientType gType;
		//! Point related to the first color keys (for conical gradients this is the cone vertex point).
		GPoint2 gStartPoint;
		/*!
			Auxiliary point: for linear gradients it's the endpoint, for radial gradients it's the focus and for
			conical gradients specifies a 'target' point that associated with start point defines a direction
			associated to the first color key.
		*/
		GPoint2 gAuxPoint;
		//! For radial gradients it's the radius.
		GReal gRadius;
		//! Color keys.
		GDynArray<GKeyValue> gColorKeys;
		//! Color interpolation.
		GColorRampInterpolation gColorInterpolation;
		//! Spread mode.
		GColorRampSpreadMode gSpreadMode;
		//! Gradient matrix.
		GMatrix33 gMatrix;
		//! Inverse gradient matrix.
		GMatrix33 gInverseMatrix;

	protected:
		//! Modified bit flags
		GUInt32 gModified;

		//! Set gradient type modified flag.
		void SetTypeModified(const GBool Modified);
		//! Set start point modified flag.
		void SetStartPointModified(const GBool Modified);
		//! Set auxiliary point modified flag.
		void SetAuxPointModified(const GBool Modified);
		//! Set color keys modified flag.
		void SetColorKeysModified(const GBool Modified);
		//! Set matrix modified flag.
		void SetColorInterpolationModified(const GBool Modified);
		//! Set spread mode modified flag.
		void SetSpreadModeModified(const GBool Modified);
		//! Set matrix modified flag.
		void SetMatrixModified(const GBool Modified);

	public:
		//! Default constructor, it build an empty linear gradient.
		GGradientDesc();
		//! Destructor
		virtual ~GGradientDesc();
		//! Get gradient type
		inline GGradientType Type() const {
			return gType;
		}
		//! Set gradient type
		void SetType(const GGradientType Type);
		//! Get start point (the point associated to the first color key, for conical gradients this is the cone vertex point)
		inline const GPoint2& StartPoint() const {
			return gStartPoint;
		}
		//! Set start point (the point associated to the first color key, for conical gradients this is the cone vertex point)
		void SetStartPoint(const GPoint2& Point);
		/*!
			Get auxiliary point.\n
			For linear gradients it represents an 'end' point associated with the last color key. For
			radial gradients it corresponds to the focus point, and for conical gradients it specifies a 'target'
			point that coupled with start point defines a direction associated to the first color key.
		*/
		inline const GPoint2& AuxPoint() const {
			return gAuxPoint;
		}
		/*!
			Set auxiliary point.\n
			For linear gradients it represents an 'end' point associated with the last color key. For
			radial gradients it corresponds to the focus point, and for conical gradients it specifies a 'target'
			point that associated with start point defines a direction associated to the first color key.
		*/
		void SetAuxPoint(const GPoint2& Point);
		//! Get radius, it has sense only for radial gradients.
		inline GReal Radius() const {
			return gRadius;
		}
		//! Set radius, it has sense only for radial gradients.
		void SetRadius(const GReal Radius);
		//! Get color keys.
		inline const GDynArray<GKeyValue>& ColorKeys() const {
			return gColorKeys;
		}
		//! Set color keys.
		virtual void SetColorKeys(const GDynArray<GKeyValue>& ColorKeys);
		//! Get color interpolation method.
		inline GColorRampInterpolation ColorInterpolation() const {
			return gColorInterpolation;
		}
		//! Set color interpolation method.
		void SetColorInterpolation(const GColorRampInterpolation Interpolation);
		//! Get spread mode.
		inline GColorRampSpreadMode SpreadMode() const {
			return gSpreadMode;
		}
		//! Set spread mode.
		void SetSpreadMode(const GColorRampSpreadMode SpreadMode);
		//! Get gradient matrix, used to transform the gradient into another coordinates system.
		inline const GMatrix33& Matrix() const {
			return gMatrix;
		}
		//! Get inverse gradient matrix.
		inline const GMatrix33& InverseMatrix() const {
			return gInverseMatrix;
		}
		/*!
			Set gradient matrix, used to transform the gradient into another coordinates system.

			\note the specified new Matrix <b>MUST NOT</b> be singular; in this case the new Matrix wont be set.
		*/
		void SetMatrix(const GMatrix33& Matrix);
		//! G_TRUE if gradient type has been modified from last time it was used, else G_FALSE.
		GBool TypeModified() const;
		//! G_TRUE if gradient start point has been modified from last time it was used, else G_FALSE.
		GBool StartPointModified() const;
		//! G_TRUE if auxiliary point has been modified from last time it was used, else G_FALSE.
		GBool AuxPointModified() const;
		//! G_TRUE if color keys array has been modified from last time it was used, else G_FALSE.
		GBool ColorKeysModified() const;
		//! G_TRUE if color interpolation method has been modified from last time it was used, else G_FALSE.
		GBool ColorInterpolationModified() const;
		//! G_TRUE if gradient spread mode has been modified from last time it was used, else G_FALSE.
		GBool SpreadModeModified() const;
		//! G_TRUE if gradient matrix has been modified from last time it was used, else G_FALSE.
		GBool MatrixModified() const;
		//! G_TRUE if at least one gradient attribute has been modified from last time it was used, else G_FALSE.
		inline GBool Modified() const {
			return (gModified != 0);
		}
	};

	// *********************************************************************
	//                            GPatternDesc
	// *********************************************************************

	/*!
		\class GPatternDesc
		\brief This class maintains informations used to describe a graphic pattern.

		Every used graphic pattern has an associated GPatternDesc class instance.
		A pattern is used to fill or stroke an object using a pre-defined graphic object (a bitmap) which can be
		replicated (tiled) at fixed intervals in x and y to cover the areas to be painted.
		This class describes completely a graphic pattern. Maintained informations are:\n\n

		- Tiling mode: defines possible methods for defining colors for source pixels that lie outside the bounds
		of the source image. Possible values are: PAD (specifies that pixels outside the bounds of the source
		image should be taken as having the same color as the closest edge pixel of the source image),
		REPEAT (specifies that source image should be repeated indefinitely in all directions) and
		REFLECT (specifies that source image should be reflected indefinitely in all directions).\n\n
		\image html pattern_tilemodes.png
		\n
		- Logical window: it defines a reference rectangle somewhere on the	infinite canvas. The tiling theoretically
		extends a series of such rectangles to infinity in X and Y (positive and negative).
		- Matrix: an optional 3x3 matrix, used to transform the pattern into another coordinates system.
	*/
	class G_EXPORT GPatternDesc {

	private:
		//! Tiling mode.
		GTilingMode gTilingMode;
		//! Pattern matrix.
		GMatrix33 gMatrix;
		//! Inverse pattern matrix.
		GMatrix33 gInverseMatrix;
		//! Pattern logical window.
		GAABox2 gLogicalWindow;

	protected:
		//! Modified bit flags.
		GUInt32 gModified;

		//! Set tiling mode modified flag.
		void SetTilingModeModified(const GBool Modified);
		//! Set matrix modified flag.
		void SetMatrixModified(const GBool Modified);
		//! Set logical window modified flag.
		void SetLogicalWindowModified(const GBool Modified);

	public:
		//! Constructor, it builds an empty pattern.
		GPatternDesc();
		//! Destructor
		virtual ~GPatternDesc();
		//! Get pattern logical window.
		inline const GAABox2& LogicalWindow() const {
			return gLogicalWindow;
		}
		//! Set pattern logical window.
		void SetLogicalWindow(const GPoint2& LowLeft, const GPoint2& UpperRight);
		//! Get tiling mode.
		inline const GTilingMode TilingMode() const {
			return gTilingMode;
		}
		//! Set tiling mode.
		void SetTilingMode(const GTilingMode TilingMode);
		//! Get pattern matrix, used to transform the pattern into another coordinates system.
		inline const GMatrix33& Matrix() const {
			return gMatrix;
		}
		//! Get inverse pattern matrix.
		inline const GMatrix33& InverseMatrix() const {
			return gInverseMatrix;
		}
		/*!
			Set pattern matrix, used to transform the pattern into another coordinates system.

			\note the specified new Matrix <b>MUST NOT</b> be singular; in this case the new Matrix wont be set.
		*/
		void SetMatrix(const GMatrix33& Matrix);
		/*!
			Set pattern image/bitmap.\n

			\param Image the source bitmap, it can be in any color depth.
			\param Quality the image quality that must be used when drawing is done using this pattern.
			\note every derived classes <b>MUST</b> implement this method.
		*/
		virtual void SetImage(const GPixelMap *Image, const GImageQuality Quality) = 0;
		//! G_TRUE if pattern tiling mode has been modified from last time it was used, else G_FALSE.
		GBool TilingModeModified() const;
		//! G_TRUE if pattern matrix has been modified from last time it was used, else G_FALSE.
		GBool MatrixModified() const;
		//! G_TRUE if pattern logical window has been modified from last time it was used, else G_FALSE.
		GBool LogicalWindowModified() const;
		//! G_TRUE if at least one pattern attribute has been modified from last time it was used, else G_FALSE.
		inline GBool Modified() const {
			return (gModified != 0);
		}
	};

	
	// *********************************************************************
	//                             GDrawStyle
	// *********************************************************************

	//! Compositing operations.
	enum GCompositingOperation {
		//! Clear Porter-Duff operation.
		G_CLEAR_OP,
		//! Src Porter-Duff operation.
		G_SRC_OP,
		//! Dst Porter-Duff operation.
		G_DST_OP,
		//! Src over Porter-Duff operation.
		G_SRC_OVER_OP,
		//! Dst over Porter-Duff operation.
		G_DST_OVER_OP,
		//! Src in Porter-Duff operation.
		G_SRC_IN_OP,
		//! Dst in Porter-Duff operation.
		G_DST_IN_OP,
		//! Src out Porter-Duff operation.
		G_SRC_OUT_OP,
		//! Dst out Porter-Duff operation.
		G_DST_OUT_OP,
		//! Src atop Porter-Duff operation.
		G_SRC_ATOP_OP,
		//! Dst atop Porter-Duff operation.
		G_DST_ATOP_OP,
		//! Xor Porter-Duff operation.
		G_XOR_OP,
		//! Plus extended Porter-Duff operation.
		G_PLUS_OP,
		//! Multiply extended Porter-Duff operation.
		G_MULTIPLY_OP,
		//! Screen extended Porter-Duff operation.
		G_SCREEN_OP,
		//! Overlay extended Porter-Duff operation.
		G_OVERLAY_OP,
		//! Darken extended Porter-Duff operation.
		G_DARKEN_OP,
		//! Lighten extended Porter-Duff operation.
		G_LIGHTEN_OP,
		//! Color dodge extended Porter-Duff operation.
		G_COLOR_DODGE_OP,
		//! Color burn extended Porter-Duff operation.
		G_COLOR_BURN_OP,
		//! Hard light extended Porter-Duff operation.
		G_HARD_LIGHT_OP,
		//! Soft light extended Porter-Duff operation.
		G_SOFT_LIGHT_OP,
		//! Difference extended Porter-Duff operation.
		G_DIFFERENCE_OP,
		//! Exclusion extended Porter-Duff operation.
		G_EXCLUSION_OP
	};

	/*!
		\class GDrawStyle
		\brief This class maintains informations used to describe a render style.

		A render style is a set of attributes and properties that defines how shapes will be drawn on
		screen. Maintained informations are:\n\n

		- Stroke pen width: the width of the stroke, in logic units.\n
		- Stroke start cap style: specifies the shape to be used at the begin of open subpaths
		or open shapes when they are stroked. It can be BUTT, ROUND and SQUARE.\n
		- Stroke end cap style: specifies the shape to be used at the end of open subpaths
		or open shapes when they are stroked. It can be BUTT, ROUND and SQUARE.\n\n
		\image html stroke_endcaps.png
		\n
		- Stroke join style: specifies the shape to be used at the corners of paths or shapes when they are stroked.
		It can be BEVEL, MITER and ROUND.\n\n
		\image html stroke_joins.png
		\n
		- Stroke miter limit: when two line segments meet at a sharp angle and miter joins have been specified, it
		is possible	for the miter to extend far beyond the thickness of the line stroking the path. The miter limit
		imposes a limit on the ratio of the miter length to the stroke width. When the limit is exceeded, the join
		is converted from a miter to a bevel.\n
		- Stroke style: it can be SOLID (non dashed) or DASHED.\n
		- Stroke dash pattern: it controls the pattern of dashes and gaps used to stroke paths. It contains a list
		of numbers that specify the lengths of alternating dashes and gaps. If an odd number of values is
		provided, then the list of values is repeated to yield an even number of values.\n
		- Stroke dash pattern phase (also known dash offset): specifies the distance into the dash pattern to start the dash.\n\n
		\image html dash_pattern_phase.png
		\n
		- Stroke paint type: it can be COLOR (stroke is drawn using a single constant color), GRADIENT (the stroke
		is drawn using the associated gradient) and PATTERN (the stroke is drawn using the associated pattern).\n
		- Stroke color: the color to use for stroke. Red, green and blue components are used when stroke is associated
		with a COLOR paint type. Alpha component is used for every possible stroke paint type.\n
		- Stroke gradient: an associated GGradientDesc instance, that describes the gradient to use (it will be
		used just when the stroke paint type is GRADIENT).\n
		- Stroke pattern: an associated GPatternDesc instance, that describes the pattern to use (it will be
		used just when the stroke paint type is PATTERN).\n
		- Stroke enable/disable flag: it is used to enable or disable stroke painting.\n
		- Fill rule: the fill rule to use for shapes, it can be ODDEVEN or EVENODD.\n
		- Fill paint type: it can be COLOR (fill is drawn using a single constant color), GRADIENT (the fill
		is drawn using the associated gradient) and PATTERN (the fill is drawn using the associated pattern).\n
		- Fill color: the color to use for fill. Red, green and blue components are used when fill is associated
		with a COLOR paint type. Alpha component is used for every possible fill paint type.\n
		- Fill gradient: an associated GGradientDesc instance, that describes the gradient to use (it will be
		used just when the fill paint type is GRADIENT).\n
		- Fill pattern: an associated GPatternDesc instance, that describes the pattern to use (it will be
		used just when the fill paint type is PATTERN).\n
		- Fill enable/disable flag: it is used to enable or disable fill painting.\n
		- Model-view matrix: an optional 3x3 matrix, used to transform geometric entities before drawing.
	*/
	class G_EXPORT GDrawStyle {

	private:
		//! Stroke pen width.
		GReal gStrokeWidth;
		//! Stroke pen thickness, defined as stroke_width / 2.
		GReal gThickness;
		//! Stroke miter limit.
		GReal gStrokeMiterLimit;
		//! Stroke start cap style.
		GCapStyle gStrokeStartCapStyle;
		//! Stroke end cap style.
		GCapStyle gStrokeEndCapStyle;
		//! Stroke join style.
		GJoinStyle gStrokeJoinStyle;
		//! Stroke paint type (it can be color, gradient or pattern).
		GPaintType gStrokePaintType;
		//! Stroke color.
		GVector4 gStrokeColor;
		//! Stroke gradient (pointer).
		GGradientDesc *gStrokeGradientDesc;
		//! Stroke pattern (pointer).
		GPatternDesc *gStrokePatternDesc;
		//! Stroke style (it can be solid or dashed).
		GStrokeStyle gStrokeStyle;
		//! Stroke dash pattern.
		GDynArray<GReal> gStrokeDashPattern;
		//! Stroke dash (initial) phase.
		GReal gStrokeDashPhase;
		//! Stroke dash pattern with incorporated phase.
		GDynArray<GReal> gStrokeOffsettedDashPattern;
		//! G_TRUE if first entry of 'offsetted dash pattern' is an 'empty' value.
		GBool gStrokeOffsettedDashPatternEmpty;
		//! Sum of all dash entries.
		GReal gStrokeDashPatternSum;
		//! Stroke compositing operation.
		GCompositingOperation gStrokeCompOp;
		//! Stroke enable/disable flag.
		GBool gStrokeEnabled;
		//! Fill rule.
		GFillRule gFillRule;
		//! Fill paint type (it can be color, gradient or pattern).
		GPaintType gFillPaintType;
		//! Fill color.
		GVector4 gFillColor;
		//! Fill gradient (pointer).
		GGradientDesc *gFillGradientDesc;
		//! Fill pattern (pointer).
		GPatternDesc *gFillPatternDesc;
		//! Fill compositing operation.
		GCompositingOperation gFillCompOp;
		//! Fill enable/disable flag.
		GBool gFillEnabled;
		//! Model-view matrix.
		GMatrix33 gModelView;
		//! Inverse model-view matrix.
		GMatrix33 gInverseModelView;
		//! Modified bit flags.
		GUInt32 gModified;

	protected:
		void UpdateOffsettedDashPattern();
		//! Set pen width modified flag.
		void SetStrokeWidthModified(const GBool Modified);
		//! Set miter limit modified flag.
		void SetStrokeMiterLimitModified(const GBool Modified);
		//! Set start cap style modified flag.
		void SetStrokeStartCapStyleModified(const GBool Modified);
		//! Set end cap style modified flag.
		void SetStrokeEndCapStyleModified(const GBool Modified);
		//! Set join cap style modified flag.
		void SetStrokeJoinStyleModified(const GBool Modified);
		//! Set stroke paint type modified flag.
		void SetStrokePaintTypeModified(const GBool Modified);
		//! Set stroke style modified flag.
		void SetStrokeStyleModified(const GBool Modified);
		//! Set stroke color modified flag.
		void SetStrokeColorModified(const GBool Modified);
		//! Set dash pattern modified flag.
		void SetStrokeDashPatternModified(const GBool Modified);
		//! Set dash phase modified flag.
		void SetStrokeDashPhaseModified(const GBool Modified);
		//! Set stroke compositing operation modified flag.
		void SetStrokeCompOpModified(const GBool Modified);
		//! Set stroke enable/disable modified flag.
		void SetStrokeEnabledModified(const GBool Modified);
		//! Set fill rule modified flag.
		void SetFillRuleModified(const GBool Modified);
		//! Set fill paint type modified flag.
		void SetFillPaintTypeModified(const GBool Modified);
		//! Set fill color modified flag.
		void SetFillColorModified(const GBool Modified);
		//! Set fill compositing operation modified flag.
		void SetFillCompOpModified(const GBool Modified);
		//! Set fill enable/disable modified flag.
		void SetFillEnabledModified(const GBool Modified);
		// set model-view modified bit flag
		void SetModelViewModified(const GBool Modified);

	public:
		//! Default constructor, it builds a default render style.
		GDrawStyle();
		//! Destructor.
		virtual ~GDrawStyle();
		//! Assignment operator.
		GDrawStyle& operator =(const GDrawStyle& Source);
		//! Get stroke pen width.
		inline GReal StrokeWidth() const {
			return gStrokeWidth;
		}
		//! Set stroke pen width. Default implementation takes the absolute value of Width.
		virtual void SetStrokeWidth(const GReal Width);
		//! Get stroke pen thickness (half stroke pen width).
		inline GReal StrokeThickness() const {
			return gThickness;
		}
		//! Get stroke miter limit.
		inline GReal StrokeMiterLimit() const {
			return gStrokeMiterLimit;
		}
		//! Set stroke miter limit. Default implementation takes the absolute value of MiterLimit.
		virtual void SetStrokeMiterLimit(const GReal MiterLimit);
		//! Get stroke start cap style.
		inline GCapStyle StrokeStartCapStyle() const {
			return gStrokeStartCapStyle;
		}
		//! Set stroke start cap style.
		virtual void SetStrokeStartCapStyle(const GCapStyle CapStyle);
		//! Get stroke end cap style.
		inline GCapStyle StrokeEndCapStyle() const {
			return gStrokeEndCapStyle;
		}
		//! Set stroke end cap style.
		virtual void SetStrokeEndCapStyle(const GCapStyle CapStyle);
		//! Get stroke join style.
		inline GJoinStyle StrokeJoinStyle() const {
			return gStrokeJoinStyle;
		}
		//! Set stroke join style.
		virtual void SetStrokeJoinStyle(const GJoinStyle JoinStyle);
		//! Get stroke paint type.
		inline GPaintType StrokePaintType() const {
			return gStrokePaintType;
		}
		//! Set stroke paint type.
		virtual void SetStrokePaintType(const GPaintType Type);
		//! Get stroke color.
		inline const GVectBase<GReal, 4>& StrokeColor() const {
			return gStrokeColor;
		}
		//! Set stroke color. Default implementation takes care of 'modified' bit flag.
		virtual void SetStrokeColor(const GVectBase<GReal, 4>& Color);
		//! Get stroke gradient descriptor (pointer), NULL if a stroke gradient has not been set.
		inline GGradientDesc* StrokeGradient() const {
			return gStrokeGradientDesc;
		}
		//! Set stroke gradient descriptor (pointer).
		inline void SetStrokeGradient(GGradientDesc *Gradient) {
			gStrokeGradientDesc = Gradient;
		}
		//! Get stroke pattern descriptor (pointer), NULL if a stroke pattern has not been set.
		inline GPatternDesc* StrokePattern() const {
			return gStrokePatternDesc;
		}
		//! Set stroke pattern descriptor (pointer).
		inline void SetStrokePattern(GPatternDesc *Pattern) {
			gStrokePatternDesc = Pattern;
		}
		//! Get stroke style.
		inline const GStrokeStyle StrokeStyle() const {
			return gStrokeStyle;
		}
		//! Set stroke style.
		virtual void SetStrokeStyle(const GStrokeStyle Style);
		//! Get stroke dash pattern.
		inline const GDynArray<GReal>& StrokeDashPattern() const {
			return gStrokeDashPattern;
		}
		//! Set stroke dash pattern.
		virtual void SetStrokeDashPattern(const GDynArray<GReal> DashPattern);
		//! Get stroke dash phase.
		inline GReal StrokeDashPhase() const {
			return gStrokeDashPhase;
		}
		//! Set stroke dash phase, and update offsetted dash pattern.
		virtual void SetStrokeDashPhase(const GReal DashPhase);
		//! Get stroke enabled/disabled flag.
		inline GBool StrokeEnabled() const {
			return gStrokeEnabled;
		}
		//! Get current stroke compositing operation.
		inline GCompositingOperation StrokeCompOp() const {
			return gStrokeCompOp;
		}
		//! Set stroke compositing operation.
		virtual void SetStrokeCompOp(const GCompositingOperation CompOp);
		//! Set stroke enabled/disabled flag.
		virtual void SetStrokeEnabled(const GBool Enabled);
		//! Get fill rule.
		inline GFillRule FillRule() const {
			return gFillRule;
		}
		//! Set fill rule.
		virtual void SetFillRule(const GFillRule Rule);
		//! Get fill paint type.
		inline GPaintType FillPaintType() const {
			return gFillPaintType;
		}
		//! Set fill paint type.
		virtual void SetFillPaintType(const GPaintType Type);
		//! Get fill color.
		inline const GVectBase<GReal, 4>& FillColor() const {
			return gFillColor;
		}
		//! Set fill color. Default implementation takes care of 'modified' bit flag.
		virtual void SetFillColor(const GVectBase<GReal, 4>& Color);
		//! Get fill gradient descriptor (pointer), NULL if a fill gradient has not been set.
		inline GGradientDesc* FillGradient() const {
			return gFillGradientDesc;
		}
		//! Set fill gradient descriptor (pointer).
		inline void SetFillGradient(GGradientDesc *Gradient) {
			gFillGradientDesc = Gradient;
		}
		//! Get fill pattern descriptor (pointer), NULL if a fill pattern has not been set.
		inline GPatternDesc* FillPattern() const {
			return gFillPatternDesc;
		}
		//! Set fill pattern descriptor (pointer).
		inline void SetFillPattern(GPatternDesc *Pattern) {
			gFillPatternDesc = Pattern;
		}
		//! Get current fill compositing operation.
		inline GCompositingOperation FillCompOp() const {
			return gFillCompOp;
		}
		//! Set fill compositing operation.
		virtual void SetFillCompOp(const GCompositingOperation CompOp);
		//! Get fill enabled/disabled flag.
		inline GBool FillEnabled() const {
			return gFillEnabled;
		}
		//! Set fill enabled/disabled flag.
		virtual void SetFillEnabled(const GBool Enabled);
		//! Get model-view matrix.
		inline const GMatrix33& ModelView() const {
			return gModelView;
		}
		//! Get inverse model-view matrix.
		inline const GMatrix33& InverseModelView() const {
			return gInverseModelView;
		}
		//! Set model-view matrix.
		virtual void SetModelView(const GMatrix33& Matrix);
		//! Get the corresponding offsetted dash pattern as it would not have initial phase.
		inline const GDynArray<GReal>& StrokeOffsettedDashPattern() const {
			return gStrokeOffsettedDashPattern;
		}
		//! G_TRUE if first entry of offsetted dash pattern is an "off" trait.
		inline GBool StrokeOffsettedDashPatternEmpty() const {
			return gStrokeOffsettedDashPatternEmpty;
		}
		//! Get total length of dash pattern (equivalent to the sum of all entries).
		inline GReal StrokeDashPatternSum() const {
			return gStrokeDashPatternSum;
		}
		//! G_TRUE if stroke pen width has been modified from last time it was used, else G_FALSE.
		GBool StrokeWidthModified() const;
		//! G_TRUE if stroke miter limit has been modified from last time it was used, else G_FALSE.
		GBool StrokeMiterLimitModified() const;
		//! G_TRUE if stroke start cap style has been modified from last time it was used, else G_FALSE.
		GBool StrokeStartCapStyleModified() const;
		//! G_TRUE if stroke end cap style has been modified from last time it was used, else G_FALSE.
		GBool StrokeEndCapStyleModified() const;
		//! G_TRUE if stroke join style has been modified from last time it was used, else G_FALSE.
		GBool StrokeJoinStyleModified() const;
		//! G_TRUE if stroke paint type has been modified from last time it was used, else G_FALSE.
		GBool StrokePaintTypeModified() const;
		//! G_TRUE if stroke style has been modified from last time it was used, else G_FALSE.
		GBool StrokeStyleModified() const;
		//! G_TRUE if stroke color has been modified from last time it was used, else G_FALSE.
		GBool StrokeColorModified() const;
		//! G_TRUE if stroke gradient has been modified from last time it was used, else G_FALSE.
		inline GBool StrokeGradientModified() const {
			if (gStrokeGradientDesc)
				return gStrokeGradientDesc->Modified();
			else
				return G_FALSE;
		}
		//! G_TRUE if stroke pattern has been modified from last time it was used, else G_FALSE.
		inline GBool StrokePatternModified() const {
			if (gStrokePatternDesc)
				return gStrokePatternDesc->Modified();
			else
				return G_FALSE;
		}
		//! G_TRUE if stroke dash pattern has been modified from last time it was used, else G_FALSE.
		GBool StrokeDashPatternModified() const;
		//! G_TRUE if stroke dash phase has been modified from last time it was used, else G_FALSE.
		GBool StrokeDashPhaseModified() const;
		//! G_TRUE if stroke compositing operation flag has been modified from last time it was used, else G_FALSE.
		GBool StrokeCompOpModified() const;
		//! G_TRUE if stroke enabled/disabled flag has been modified from last time it was used, else G_FALSE.
		GBool StrokeEnabledModified() const;
		//! G_TRUE if fill rule has been modified from last time it was used, else G_FALSE.
		GBool FillRuleModified() const;
		//! G_TRUE if fill paint type has been modified from last time it was used, else G_FALSE.
		GBool FillPaintTypeModified() const;
		//! G_TRUE if fill color has been modified from last time it was used, else G_FALSE.
		GBool FillColorModified() const;
		//! G_TRUE if fill gradient has been modified from last time it was used, else G_FALSE.
		inline GBool FillGradientModified() const {
			if (gFillGradientDesc)
				return gFillGradientDesc->Modified();
			else
				return G_FALSE;
		}
		//! G_TRUE if fill pattern has been modified from last time it was used, else G_FALSE.
		inline GBool FillPatternModified() const {
			if (gFillPatternDesc)
				return gFillPatternDesc->Modified();
			else
				return G_FALSE;
		}
		//! G_TRUE if fill compositing operation flag has been modified from last time it was used, else G_FALSE.
		GBool FillCompOpModified() const;
		//! G_TRUE if fill enabled/disabled flag has been modified from last time it was used, else G_FALSE.
		GBool FillEnabledModified() const;
		//! G_TRUE if model-view matrix has been modified from last time it was used, else G_FALSE.
		GBool ModelViewModified() const;
		//! G_TRUE if at least one render style attribute has been modified from last time it was used, else G_FALSE.
		inline GBool Modified() const {
			return ((gModified != 0) || StrokeGradientModified() || StrokePatternModified() ||
					FillGradientModified() || FillPatternModified());
		}
	};

};	// end namespace Amanith

#endif
