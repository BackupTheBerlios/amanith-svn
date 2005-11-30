/****************************************************************************
** $file: amanith/rendering/gdrawstyle.h   0.1.1.0   edited Sep 24 08:00
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
	\brief Generic draw style.
*/
namespace Amanith {

	// cap style
	enum GCapStyle {
		G_BUTT_CAP,
		G_ROUND_CAP,
		G_SQUARE_CAP
	};

	// join style
	enum GJoinStyle {
		G_MITER_JOIN,
		G_ROUND_JOIN,
		G_BEVEL_JOIN
	};

	// stroke style
	enum GStrokeStyle {
		G_SOLID_STROKE,
		G_DASHED_STROKE
	};

	// fill rule
	enum GFillRule {
		G_ODD_EVEN_FILLRULE,
		G_EVEN_ODD_FILLRULE,
		G_NON_ZERO_FILLRULE,
		G_ANY_FILLRULE
	};

	// paint type
	enum GPaintType {
		G_COLOR_PAINT_TYPE,
		G_GRADIENT_PAINT_TYPE,
		G_PATTERN_PAINT_TYPE
	};

	// gradient type
	enum GGradientType {
		G_LINEAR_GRADIENT,
		G_RADIAL_GRADIENT,
		G_CONICAL_GRADIENT
	};

	// ramp spread mode
	enum GColorRampSpreadMode {
		G_PAD_COLOR_RAMP_SPREAD,
		G_REPEAT_COLOR_RAMP_SPREAD,
		G_REFLECT_COLOR_RAMP_SPREAD
	};

	// color interpolation
	enum GColorRampInterpolation {
		G_CONSTANT_COLOR_INTERPOLATION,
		G_LINEAR_COLOR_INTERPOLATION,
		G_HERMITE_COLOR_INTERPOLATION
	};

	// tiling mode
	enum GTilingMode {
		G_PAD_TILE,
		G_REPEAT_TILE,
		G_REFLECT_TILE
	};

	// image/pattern quality
	enum GImageQuality {
		G_LOW_IMAGE_QUALITY,
		G_NORMAL_IMAGE_QUALITY,
		G_HIGH_IMAGE_QUALITY
	};

	// *********************************************************************
	//                            GGradientDesc
	// *********************************************************************
	// description of a gradient
	class G_EXPORT GGradientDesc {

	private:
		// gradient type
		GGradientType gType;
		// point related to the first color keys
		GPoint2 gStartPoint;
		// auxiliary point: for linear gradients it's the endpoint, for radial gradients it's the the center
		GPoint2 gAuxPoint;
		// for radial gradients it's the radius
		GReal gRadius;
		// color keys
		GDynArray<GKeyValue> gColorKeys;
		// color interpolation
		GColorRampInterpolation gColorInterpolation;
		// spread mode
		GColorRampSpreadMode gSpreadMode;
		// gradient matrix
		GMatrix33 gMatrix;

	protected:
		// modified flag
		GUInt32 gModified;

		void SetTypeModified(const GBool Modified);
		void SetStartPointModified(const GBool Modified);
		void SetAuxPointModified(const GBool Modified);
		void SetColorKeysModified(const GBool Modified);
		void SetColorInterpolationModified(const GBool Modified);
		void SetSpreadModeModified(const GBool Modified);
		void SetMatrixModified(const GBool Modified);

	public:
		// default constructor
		GGradientDesc();
		// destructor
		virtual ~GGradientDesc();
		// get gradient type
		inline GGradientType Type() const {
			return gType;
		}
		// set gradient type
		void SetType(const GGradientType Type);
		// get start point
		inline const GPoint2& StartPoint() const {
			return gStartPoint;
		}
		// set start point
		void SetStartPoint(const GPoint2& Point);
		// get auxiliary point
		inline const GPoint2& AuxPoint() const {
			return gAuxPoint;
		}
		// set auxiliary point
		void SetAuxPoint(const GPoint2& Point);
		// get radius (for radial gradients)
		inline GReal Radius() const {
			return gRadius;
		}
		// set radius (for radial gradients)
		void SetRadius(const GReal Radius);
		// get color keys
		inline const GDynArray<GKeyValue>& ColorKeys() const {
			return gColorKeys;
		}
		// set color keys
		virtual void SetColorKeys(const GDynArray<GKeyValue>& ColorKeys);
		// get color interpolation
		inline GColorRampInterpolation ColorInterpolation() const {
			return gColorInterpolation;
		}
		// set color interpolation
		void SetColorInterpolation(const GColorRampInterpolation Interpolation);
		// get spread mode
		inline GColorRampSpreadMode SpreadMode() const {
			return gSpreadMode;
		}
		// set spread mode
		void SetSpreadMode(const GColorRampSpreadMode SpreadMode);
		// get matrix
		inline const GMatrix33& Matrix() const {
			return gMatrix;
		}
		// set matrix
		void SetMatrix(const GMatrix33& Matrix);
		// modified bit flags
		GBool TypeModified() const;
		GBool StartPointModified() const;
		GBool AuxPointModified() const;
		GBool ColorKeysModified() const;
		GBool ColorInterpolationModified() const;
		GBool SpreadModeModified() const;
		GBool MatrixModified() const;
		inline GBool Modified() const {
			return (gModified != 0);
		}
	};

	// *********************************************************************
	//                            GPatternDesc
	// *********************************************************************
	// description of a pattern
	class G_EXPORT GPatternDesc {

	public:
		// tiling mode
		GTilingMode gTilingMode;
		// pattern matrix
		GMatrix33 gMatrix;
		// pattern logical window
		GAABox2 gLogicalWindow;

	protected:
		// modified flag
		GUInt32 gModified;

		void SetTilingModeModified(const GBool Modified);
		void SetMatrixModified(const GBool Modified);
		void SetLogicalWindowModified(const GBool Modified);

	public:
		// constructor
		GPatternDesc();
		// destructor
		virtual ~GPatternDesc();
		// get logical window
		inline const GAABox2& LogicalWindow() const {
			return gLogicalWindow;
		}
		// set logical window
		void SetLogicalWindow(const GPoint2& LowLeft, const GPoint2& UpperRight);
		// get tiling mode
		inline const GTilingMode TilingMode() const {
			return gTilingMode;
		}
		void SetTilingMode(const GTilingMode TilingMode);
		// get matrix
		inline const GMatrix33& Matrix() const {
			return gMatrix;
		}
		// set matrix
		void SetMatrix(const GMatrix33& Matrix);
		// set image
		virtual void SetImage(const GPixelMap *Image, const GImageQuality Quality) = 0;
		// modified bit flags
		GBool TilingModeModified() const;
		GBool MatrixModified() const;
		GBool LogicalWindowModified() const;
		//GBool ImageModified() const;
		inline GBool Modified() const {
			return (gModified != 0);
		}
	};

	
	// *********************************************************************
	//                             GDrawStyle
	// *********************************************************************

	// a general rendering context
	class G_EXPORT GDrawStyle {

	private:
		// stroke parameters
		GReal gStrokeWidth;
		GReal gThickness;  // defined as stroke_width / 2
		GReal gMiterMulThickness; // defined as Miter * Thickness
		GReal gStrokeMiterLimit;
		GCapStyle gStrokeStartCapStyle;
		GCapStyle gStrokeEndCapStyle;
		GJoinStyle gStrokeJoinStyle;
		GPaintType gStrokePaintType;
		GVector4 gStrokeColor;
		GGradientDesc *gStrokeGradientDesc;
		GPatternDesc *gStrokePatternDesc;
		GStrokeStyle gStrokeStyle;
		GDynArray<GReal> gStrokeDashPattern;
		GReal gStrokeDashPhase;
		GDynArray<GReal> gStrokeOffsettedDashPattern;
		GBool gStrokeOffsettedDashPatternEmpty;
		GReal gStrokeDashPatternSum;
		GBool gStrokeEnabled;
		// fill parameters
		GFillRule gFillRule;
		GPaintType gFillPaintType;
		GVector4 gFillColor;
		GGradientDesc *gFillGradientDesc;
		GPatternDesc *gFillPatternDesc;
		GBool gFillEnabled;
		// model-view matrix
		GMatrix33 gModelView;
		// modified flag
		GUInt32 gModified;
		// custom/user data
		void *gCustomData;

	protected:
		void UpdateOffsettedDashPattern();

	public:
		// default constructor
		GDrawStyle();
		// assignment operator
		GDrawStyle& operator =(const GDrawStyle& Source);
		// get stroke width
		inline GReal StrokeWidth() const {
			return gStrokeWidth;
		}
		// set stroke width
		void SetStrokeWidth(const GReal Width);
		// get stroke thickness (half stroke width)
		inline GReal StrokeThickness() const {
			return gThickness;
		}
		// get stroke miter limit
		inline GReal StrokeMiterLimit() const {
			return gStrokeMiterLimit;
		}
		// get Miterlimit * Thickness
		inline GReal StrokeMiterLimitMulThickness() const {
			return gMiterMulThickness;
		}
		// set stroke miter limit
		void SetStrokeMiterLimit(const GReal MiterLimit);
		// get stroke start cap style
		inline GCapStyle StrokeStartCapStyle() const {
			return gStrokeStartCapStyle;
		}
		// set stroke start cap style
		void SetStrokeStartCapStyle(const GCapStyle CapStyle);
		// get stroke end cap style
		inline GCapStyle StrokeEndCapStyle() const {
			return gStrokeEndCapStyle;
		}
		// set stroke end cap style
		void SetStrokeEndCapStyle(const GCapStyle CapStyle);
		// get stroke join style
		inline GJoinStyle StrokeJoinStyle() const {
			return gStrokeJoinStyle;
		}
		// set stroke join style
		void SetStrokeJoinStyle(const GJoinStyle JoinStyle);
		// get stroke paint type
		inline GPaintType StrokePaintType() const {
			return gStrokePaintType;
		}
		// set stroke paint type
		void SetStrokePaintType(const GPaintType Type);
		// get stroke color
		inline const GVectBase<GReal, 4>& StrokeColor() const {
			return gStrokeColor;
		}
		// set stroke color
		void SetStrokeColor(const GVectBase<GReal, 4>& Color);
		// get stroke gradient settings
		inline GGradientDesc* StrokeGradient() const {
			return gStrokeGradientDesc;
		}
		// set stroke gradient settings
		inline void SetStrokeGradient(GGradientDesc *Gradient) {
			gStrokeGradientDesc = Gradient;
		}
		// get stroke pattern settings
		inline GPatternDesc* StrokePattern() const {
			return gStrokePatternDesc;
		}
		// set stroke pattern settings
		inline void SetStrokePattern(GPatternDesc *Pattern) {
			gStrokePatternDesc = Pattern;
		}
		// get stroke style
		inline const GStrokeStyle StrokeStyle() const {
			return gStrokeStyle;
		}
		// set stroke style
		void SetStrokeStyle(const GStrokeStyle Style);
		// get stroke dash pattern
		inline const GDynArray<GReal>& StrokeDashPattern() const {
			return gStrokeDashPattern;
		}
		// set stroke dash pattern
		void SetStrokeDashPattern(const GDynArray<GReal> DashPattern);
		// get stroke dash phase
		inline GReal StrokeDashPhase() const {
			return gStrokeDashPhase;
		}
		// set stroke dash phase
		void SetStrokeDashPhase(const GReal DashPhase);
		// get stroke enabled
		inline GBool StrokeEnabled() const {
			return gStrokeEnabled;
		}
		// set stroke enabled
		void SetStrokeEnabled(const GBool Enabled);
		// get fill rule
		inline GFillRule FillRule() const {
			return gFillRule;
		}
		// set fill rule
		void SetFillRule(const GFillRule Rule);
		// get fill paint type
		inline GPaintType FillPaintType() const {
			return gFillPaintType;
		}
		// set fill paint type
		void SetFillPaintType(const GPaintType Type);
		// get fill color
		inline const GVectBase<GReal, 4>& FillColor() const {
			return gFillColor;
		}
		// set fill color
		void SetFillColor(const GVectBase<GReal, 4>& Color);
		// get fill gradient settings
		inline GGradientDesc* FillGradient() const {
			return gFillGradientDesc;
		}
		// set fill gradient settings
		inline void SetFillGradient(GGradientDesc *Gradient) {
			gFillGradientDesc = Gradient;
		}
		// get fill pattern settings
		inline GPatternDesc* FillPattern() const {
			return gFillPatternDesc;
		}
		// get fill pattern settings
		inline void SetFillPattern(GPatternDesc *Pattern) {
			gFillPatternDesc = Pattern;
		}
		// get fill enabled
		inline GBool FillEnabled() const {
			return gFillEnabled;
		}
		// set fill enabled
		void SetFillEnabled(const GBool Enabled);
		// get model-view matrix
		inline const GMatrix33& ModelView() const {
			return gModelView;
		}
		// set model-view matrix
		void SetModelView(const GMatrix33 Matrix);
		// get custom data
		inline void *CustomData() const {
			return gCustomData;
		}
		// set custom data
		inline void SetCustomData(void *Data) {
			gCustomData = Data;
		}
		// get the corresponding offsetted dash pattern as it would not have initial phase
		inline const GDynArray<GReal>& StrokeOffsettedDashPattern() const {
			return gStrokeOffsettedDashPattern;
		}
		// G_TRUE if first entry of offsetted dash pattern is an "off" trait
		inline GBool StrokeOffsettedDashPatternEmpty() const {
			return gStrokeOffsettedDashPatternEmpty;
		}
		// get total length of dash pattern (equivalent to the sum of all entries)
		inline GReal StrokeDashPatternSum() const {
			return gStrokeDashPatternSum;
		}

		// modified bit flags
		GBool StrokeWidthModified() const;
		GBool StrokeMiterLimitModified() const;
		GBool StrokeStartCapStyleModified() const;
		GBool StrokeEndCapStyleModified() const;
		GBool StrokeJoinStyleModified() const;
		GBool StrokePaintTypeModified() const;
		GBool StrokeStyleModified() const;
		GBool StrokeColorModified() const;
		inline GBool StrokeGradientModified() const {
			if (gStrokeGradientDesc)
				return gStrokeGradientDesc->Modified();
			else
				return G_FALSE;
		}
		inline GBool StrokePatternModified() const {
			if (gStrokePatternDesc)
				return gStrokePatternDesc->Modified();
			else
				return G_FALSE;
		}
		GBool StrokeDashPatternModified() const;
		GBool StrokeDashPhaseModified() const;
		GBool StrokeEnabledModified() const;
		GBool FillRuleModified() const;
		GBool FillPaintTypeModified() const;
		GBool FillColorModified() const;
		inline GBool FillGradientModified() const {
			if (gFillGradientDesc)
				return gFillGradientDesc->Modified();
			else
				return G_FALSE;
		}
		inline GBool FillPatternModified() const {
			if (gFillPatternDesc)
				return gFillPatternDesc->Modified();
			else
				return G_FALSE;
		}
		GBool FillEnabledModified() const;
		GBool ModelViewModified() const;
		inline GBool Modified() const {
			return ((gModified != 0) || StrokeGradientModified() || StrokePatternModified() ||
					FillGradientModified() || FillPatternModified());
		}

		// set stroke modified bit flags
		void SetStrokeWidthModified(const GBool Modified);
		void SetStrokeMiterLimitModified(const GBool Modified);
		void SetStrokeStartCapStyleModified(const GBool Modified);
		void SetStrokeEndCapStyleModified(const GBool Modified);
		void SetStrokeJoinStyleModified(const GBool Modified);
		void SetStrokePaintTypeModified(const GBool Modified);
		void SetStrokeStyleModified(const GBool Modified);
		void SetStrokeColorModified(const GBool Modified);
		void SetStrokeDashPatternModified(const GBool Modified);
		void SetStrokeDashPhaseModified(const GBool Modified);
		void SetStrokeEnabledModified(const GBool Modified);
		// set fill modified bit flags
		void SetFillRuleModified(const GBool Modified);
		void SetFillPaintTypeModified(const GBool Modified);
		void SetFillColorModified(const GBool Modified);
		void SetFillEnabledModified(const GBool Modified);
		// set model-view modified bit flag
		void SetModelViewModified(const GBool Modified);
	};

};	// end namespace Amanith

#endif
