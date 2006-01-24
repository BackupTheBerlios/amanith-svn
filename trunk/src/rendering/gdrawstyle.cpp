/****************************************************************************
** $file: amanith/src/rendering/gdrawstyle.cpp   0.2.0.0   edited Dec, 12 2005
**
** Draw style implementation.
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

#include "amanith/rendering/gdrawstyle.h"
#include "amanith/geometry/gxformconv.h"
#include <cstring>  // for memcmp function

/*!
	\file gdrawstyle.cpp
	\brief Draw style implementation file.
*/

namespace Amanith {

// *********************************************************************
//                            GGradientDesc
// *********************************************************************

#define G_GRADIENT_TYPE_MODIFIED				1
#define G_GRADIENT_STARTPOINT_MODIFIED			2
#define G_GRADIENT_AUXPOINT_MODIFIED			4
#define G_GRADIENT_RADIUS_MODIFIED				8
#define G_GRADIENT_COLORKEYS_MODIFIED			16
#define G_GRADIENT_COLORINTERPOLATION_MODIFIED	32
#define G_GRADIENT_SPREADMODE_MODIFIED			64
#define G_GRADIENT_MATRIX_MODIFIED				128

GGradientDesc::GGradientDesc() {

	gModified = G_GRADIENT_TYPE_MODIFIED | G_GRADIENT_STARTPOINT_MODIFIED | G_GRADIENT_AUXPOINT_MODIFIED;
	gModified |= G_GRADIENT_COLORKEYS_MODIFIED | G_GRADIENT_SPREADMODE_MODIFIED | G_GRADIENT_MATRIX_MODIFIED;

	gType = G_LINEAR_GRADIENT;
	// gStartPoint and gAuxPoint constructors will set (0, 0)
	gRadius = 1;
	gSpreadMode = G_REPEAT_COLOR_RAMP_SPREAD;
	gColorInterpolation = G_LINEAR_COLOR_INTERPOLATION;
	// gMatrix constructor will set identity
}

GGradientDesc::~GGradientDesc() {
}

// set gradient type
void GGradientDesc::SetType(const GGradientType Type) {

	if (Type != gType) {
		gModified |= G_GRADIENT_TYPE_MODIFIED;
		gType = Type;
	}
}

// set start point
void GGradientDesc::SetStartPoint(const GPoint2& Point) {

	if (Point != gStartPoint) {
		gModified |= G_GRADIENT_STARTPOINT_MODIFIED;
		gStartPoint = Point;
	}
}

// set auxiliary point
void GGradientDesc::SetAuxPoint(const GPoint2& Point) {

	if (Point != gAuxPoint) {
		gModified |= G_GRADIENT_AUXPOINT_MODIFIED;
		gAuxPoint = Point;
	}
}

// set radius (for radial gradients)
void GGradientDesc::SetRadius(const GReal Radius) {

	GReal r = GMath::Abs(Radius);
	if (r != gRadius) {
		gModified |= G_GRADIENT_RADIUS_MODIFIED;
		gRadius = r;
	}
}

// set color keys
void GGradientDesc::SetColorKeys(const GDynArray<GKeyValue>& ColorKeys) {

	GUInt32 i, j = (GUInt32)gColorKeys.size();

	if ((GUInt32)ColorKeys.size() != j) {
		gColorKeys = ColorKeys;
		gModified |= G_GRADIENT_COLORKEYS_MODIFIED;
		return;
	}
	// check each color key
	for (i = 0; i < j; ++i) {
		if ((ColorKeys[i].TimePosition() != gColorKeys[i].TimePosition()) ||
			(ColorKeys[i].Vect4Value() != gColorKeys[i].Vect4Value())) {
			gColorKeys = ColorKeys;
			gModified |= G_GRADIENT_COLORKEYS_MODIFIED;
			return;
		}
	}
}

// set color interpolation
void GGradientDesc::SetColorInterpolation(const GColorRampInterpolation Interpolation) {

	if (Interpolation != gColorInterpolation) {
		gModified |= G_GRADIENT_COLORINTERPOLATION_MODIFIED;
		gColorInterpolation = Interpolation;
	}
}

// set spread mode
void GGradientDesc::SetSpreadMode(const GColorRampSpreadMode SpreadMode) {

	if (SpreadMode != gSpreadMode) {
		gModified |= G_GRADIENT_SPREADMODE_MODIFIED;
		gSpreadMode = SpreadMode;
	}
}

// set matrix
void GGradientDesc::SetMatrix(const GMatrix33& Matrix) {

	if (gMatrix != Matrix) {

		// calculate inverse gradient matrix
		GMatrix33 preTrans, tmpInv;
		GMatrix22 n, b;
		GBool ok;
		GReal det;

		TranslationToMatrix(preTrans, GPoint2(-Matrix[0][2], -Matrix[1][2]));
		n[0][0] = Matrix[0][0];
		n[0][1] = Matrix[0][1];
		n[1][0] = Matrix[1][0];
		n[1][1] = Matrix[1][1];
		ok = InvertFull_GJ(b, n, det);
		if (!ok) {
			G_DEBUG("GGradientDesc::SetMatrix, matrix is singular!");
			return;
		}
		tmpInv[0][0] = b[0][0];
		tmpInv[0][1] = b[0][1];
		tmpInv[1][0] = b[1][0];
		tmpInv[1][1] = b[1][1];
		tmpInv[0][2] = 0;
		tmpInv[1][2] = 0;
		gInverseMatrix = tmpInv * preTrans;

		gModified |= G_GRADIENT_MATRIX_MODIFIED;
		gMatrix = Matrix;
	}
}

// modified bit flags
GBool GGradientDesc::TypeModified() const {

	if (gModified & G_GRADIENT_TYPE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GGradientDesc::StartPointModified() const {

	if (gModified & G_GRADIENT_STARTPOINT_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GGradientDesc::AuxPointModified() const {

	if (gModified & G_GRADIENT_AUXPOINT_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GGradientDesc::ColorKeysModified() const {

	if (gModified & G_GRADIENT_COLORKEYS_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GGradientDesc::ColorInterpolationModified() const {

	if (gModified & G_GRADIENT_COLORINTERPOLATION_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GGradientDesc::SpreadModeModified() const {

	if (gModified & G_GRADIENT_SPREADMODE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GGradientDesc::MatrixModified() const {
	
	if (gModified & G_GRADIENT_MATRIX_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

void GGradientDesc::SetTypeModified(const GBool Modified) {

	if (Modified)
		gModified |= G_GRADIENT_TYPE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_GRADIENT_TYPE_MODIFIED);
}

void GGradientDesc::SetStartPointModified(const GBool Modified) {

	if (Modified)
		gModified |= G_GRADIENT_STARTPOINT_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_GRADIENT_STARTPOINT_MODIFIED);
}

void GGradientDesc::SetAuxPointModified(const GBool Modified) {

	if (Modified)
		gModified |= G_GRADIENT_AUXPOINT_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_GRADIENT_AUXPOINT_MODIFIED);
}

void GGradientDesc::SetColorKeysModified(const GBool Modified) {

	if (Modified)
		gModified |= G_GRADIENT_COLORKEYS_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_GRADIENT_COLORKEYS_MODIFIED);
}

void GGradientDesc::SetColorInterpolationModified(const GBool Modified) {

	if (Modified)
		gModified |= G_GRADIENT_COLORINTERPOLATION_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_GRADIENT_COLORINTERPOLATION_MODIFIED);
}

void GGradientDesc::SetSpreadModeModified(const GBool Modified) {

	if (Modified)
		gModified |= G_GRADIENT_COLORINTERPOLATION_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_GRADIENT_COLORINTERPOLATION_MODIFIED);
}

void GGradientDesc::SetMatrixModified(const GBool Modified) {

	if (Modified)
		gModified |= G_GRADIENT_MATRIX_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_GRADIENT_MATRIX_MODIFIED);
}

#undef G_GRADIENT_TYPE_MODIFIED
#undef G_GRADIENT_STARTPOINT_MODIFIED
#undef G_GRADIENT_AUXPOINT_MODIFIED
#undef G_GRADIENT_RADIUS_MODIFIED
#undef G_GRADIENT_COLORKEYS_MODIFIED
#undef G_GRADIENT_COLORINTERPOLATION_MODIFIED
#undef G_GRADIENT_SPREADMODE_MODIFIED
#undef G_GRADIENT_MATRIX_MODIFIED


// *********************************************************************
//                            GPatternDesc
// *********************************************************************

#define G_PATTERN_TILINGMODE_MODIFIED			2
#define G_PATTERN_MATRIX_MODIFIED				4
#define G_PATTERN_LOGICAL_WINDOW_MODIFIED		8

// constructor
GPatternDesc::GPatternDesc() {

	gModified = G_PATTERN_TILINGMODE_MODIFIED | G_PATTERN_MATRIX_MODIFIED;
	gTilingMode = G_REPEAT_TILE;
	gLogicalWindow.SetMinMax(GPoint2(0, 0), GPoint2(1, 1));
	// gMatrix constructor will set identity
}

GPatternDesc::~GPatternDesc() {
}

// set logical window
void GPatternDesc::SetLogicalWindow(const GPoint2& LowLeft, const GPoint2& UpperRight) {

	GAABox2 tmpBox(LowLeft, UpperRight);

	if (gLogicalWindow != tmpBox) {
		gModified |= G_PATTERN_LOGICAL_WINDOW_MODIFIED;
		gLogicalWindow = tmpBox;
	}
}

// set tiling mode
void GPatternDesc::SetTilingMode(const GTilingMode TilingMode) {

	if (TilingMode != gTilingMode) {
		gModified |= G_PATTERN_TILINGMODE_MODIFIED;
		gTilingMode = TilingMode;
	}
}

// set matrix
void GPatternDesc::SetMatrix(const GMatrix33& Matrix) {

	if (gMatrix != Matrix) {

		// calculate inverse pattern matrix
		GMatrix33 preTrans, tmpInv;
		GMatrix22 n, b;
		GBool ok;
		GReal det;

		TranslationToMatrix(preTrans, GPoint2(-Matrix[0][2], -Matrix[1][2]));
		n[0][0] = Matrix[0][0];
		n[0][1] = Matrix[0][1];
		n[1][0] = Matrix[1][0];
		n[1][1] = Matrix[1][1];
		ok = InvertFull_GJ(b, n, det);
		if (!ok) {
			G_DEBUG("GPatternDesc::SetMatrix, matrix is singular!");
			return;
		}
		tmpInv[0][0] = b[0][0];
		tmpInv[0][1] = b[0][1];
		tmpInv[1][0] = b[1][0];
		tmpInv[1][1] = b[1][1];
		tmpInv[0][2] = 0;
		tmpInv[1][2] = 0;
		gInverseMatrix = tmpInv * preTrans;

		gModified |= G_PATTERN_MATRIX_MODIFIED;
		gMatrix = Matrix;
	}
}

GBool GPatternDesc::LogicalWindowModified() const {

	if (gModified & G_PATTERN_LOGICAL_WINDOW_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GPatternDesc::TilingModeModified() const {

	if (gModified & G_PATTERN_TILINGMODE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GPatternDesc::MatrixModified() const {

	if (gModified & G_PATTERN_MATRIX_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

void GPatternDesc::SetLogicalWindowModified(const GBool Modified) {

	if (Modified)
		gModified |= G_PATTERN_LOGICAL_WINDOW_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_PATTERN_LOGICAL_WINDOW_MODIFIED);
}

void GPatternDesc::SetTilingModeModified(const GBool Modified) {

	if (Modified)
		gModified |= G_PATTERN_TILINGMODE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_PATTERN_TILINGMODE_MODIFIED);
}

void GPatternDesc::SetMatrixModified(const GBool Modified) {

	if (Modified)
		gModified |= G_PATTERN_MATRIX_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_PATTERN_MATRIX_MODIFIED);
}

#undef G_PATTERN_TILINGMODE_MODIFIED
#undef G_PATTERN_MATRIX_MODIFIED
#undef G_PATTERN_LOGICAL_WINDOW_MODIFIED

// *********************************************************************
//                            GDrawStyle
// *********************************************************************

#define G_DRAWSTYLE_STROKEWIDTH_MODIFIED			1
#define G_DRAWSTYLE_STROKEMITERLIMIT_MODIFIED		2
#define G_DRAWSTYLE_STROKESTARTCAPSTYLE_MODIFIED	4
#define G_DRAWSTYLE_STROKEENDCAPSTYLE_MODIFIED		8
#define G_DRAWSTYLE_STROKEJOINSTYLE_MODIFIED		16
#define G_DRAWSTYLE_STROKEPAINTTYPE_MODIFIED		32
#define G_DRAWSTYLE_STROKESTYLE_MODIFIED			64
#define G_DRAWSTYLE_STROKECOLOR_MODIFIED			128
#define G_DRAWSTYLE_STROKEDASHPHASE_MODIFIED		256
#define G_DRAWSTYLE_STROKEDASHPATTERN_MODIFIED		512
#define G_DRAWSTYLE_STROKECOMPOP_MODIFIED			1024
#define G_DRAWSTYLE_STROKEENABLED_MODIFIED			2048
#define G_DRAWSTYLE_FILLRULE_MODIFIED				4096
#define G_DRAWSTYLE_FILLPAINTTYPE_MODIFIED			8192
#define G_DRAWSTYLE_FILLCOLOR_MODIFIED				16384
#define G_DRAWSTYLE_FILLCOMPOP_MODIFIED				32768
#define G_DRAWSTYLE_FILLENABLED_MODIFIED			65536
#define G_DRAWSTYLE_MODELVIEW_MODIFIED				131072

GDrawStyle::GDrawStyle() {

	gModified = G_DRAWSTYLE_STROKEWIDTH_MODIFIED | G_DRAWSTYLE_STROKEMITERLIMIT_MODIFIED;
	gModified |= G_DRAWSTYLE_STROKESTARTCAPSTYLE_MODIFIED | G_DRAWSTYLE_STROKEENDCAPSTYLE_MODIFIED;
	gModified |= G_DRAWSTYLE_STROKEJOINSTYLE_MODIFIED | G_DRAWSTYLE_STROKEPAINTTYPE_MODIFIED;
	gModified |= G_DRAWSTYLE_STROKECOLOR_MODIFIED | G_DRAWSTYLE_STROKEDASHPHASE_MODIFIED;
	gModified |= G_DRAWSTYLE_STROKECOMPOP_MODIFIED;
	gModified |= G_DRAWSTYLE_STROKEDASHPATTERN_MODIFIED | G_DRAWSTYLE_STROKEENABLED_MODIFIED;
	gModified |= G_DRAWSTYLE_FILLRULE_MODIFIED | G_DRAWSTYLE_FILLPAINTTYPE_MODIFIED;
	gModified |= G_DRAWSTYLE_FILLCOMPOP_MODIFIED;
	gModified |= G_DRAWSTYLE_FILLCOLOR_MODIFIED | G_DRAWSTYLE_FILLENABLED_MODIFIED | G_DRAWSTYLE_MODELVIEW_MODIFIED;

	gStrokeWidth = (GReal)1;
	gThickness = (GReal)0.5;
	gStrokeMiterLimit = (GReal)4;
	gStrokeStartCapStyle = G_BUTT_CAP;
	gStrokeEndCapStyle = G_BUTT_CAP;
	gStrokeJoinStyle = G_MITER_JOIN;
	gStrokePaintType = G_COLOR_PAINT_TYPE;
	gStrokeColor.Set(0, 0, 0, 1);
	gStrokeStyle = G_SOLID_STROKE;
	// gStrokeDashPattern will start empty
	gStrokeDashPhase = 0;
	gStrokeOffsettedDashPatternEmpty = G_FALSE;
	gStrokeDashPatternSum = 0;
	gStrokeGradientDesc = NULL;
	gStrokePatternDesc = NULL;
	gStrokeCompOp = G_SRC_OVER_OP;
	gStrokeEnabled = G_TRUE;
	// fill parameters
	gFillRule = G_ODD_EVEN_FILLRULE;
	gFillPaintType = G_COLOR_PAINT_TYPE;
	gFillColor.Set(0, 0, 0, 1);
	gFillGradientDesc = NULL;
	gFillPatternDesc = NULL;
	gFillCompOp = G_SRC_OVER_OP;
	gFillEnabled = G_TRUE;
	// model-view matrix (and its inverse) will start as identities
}

// destructor
GDrawStyle::~GDrawStyle() {
}

// assignment operator
GDrawStyle& GDrawStyle::operator=(const GDrawStyle& Source) {

	SetStrokeWidth(Source.StrokeWidth());
	SetStrokeMiterLimit(Source.StrokeMiterLimit());
	SetStrokeStartCapStyle(Source.StrokeStartCapStyle());
	SetStrokeEndCapStyle(Source.StrokeEndCapStyle());
	SetStrokeJoinStyle(Source.StrokeJoinStyle());
	SetStrokePaintType(Source.StrokePaintType());
	SetStrokeColor(Source.StrokeColor());
	SetStrokeStyle(Source.StrokeStyle());
	SetStrokeDashPattern(Source.StrokeDashPattern());
	SetStrokeDashPhase(Source.StrokeDashPhase());
	SetStrokeEnabled(Source.StrokeEnabled());
	SetStrokeGradient(Source.StrokeGradient());
	SetStrokePattern(Source.StrokePattern());
	SetFillRule(Source.FillRule());
	SetFillColor(Source.FillColor());
	SetFillPaintType(Source.FillPaintType());
	SetFillEnabled(Source.FillEnabled());
	SetFillGradient(Source.FillGradient());
	SetFillPattern(Source.FillPattern());
	SetModelView(Source.ModelView());
	return *this;
}

// set stroke width
void GDrawStyle::SetStrokeWidth(const GReal Width) {

	GReal w = GMath::Abs(Width);
	if (w != gStrokeWidth) {
		gModified |= G_DRAWSTYLE_STROKEWIDTH_MODIFIED;
		gStrokeWidth = w;
		gThickness = w * (GReal)0.5;
	}
}

// set stroke miter limit
void GDrawStyle::SetStrokeMiterLimit(const GReal MiterLimit) {

	GReal m = GMath::Abs(MiterLimit);
	if (m != gStrokeMiterLimit) {
		gModified |= G_DRAWSTYLE_STROKEMITERLIMIT_MODIFIED;
		gStrokeMiterLimit = m;
	}
}

// set stroke start cap style
void GDrawStyle::SetStrokeStartCapStyle(const GCapStyle CapStyle) {

	if (CapStyle != gStrokeStartCapStyle) {
		gModified |= G_DRAWSTYLE_STROKESTARTCAPSTYLE_MODIFIED;
		gStrokeStartCapStyle = CapStyle;
	}
}

// set stroke end cap style
void GDrawStyle::SetStrokeEndCapStyle(const GCapStyle CapStyle) {

	if (CapStyle != gStrokeEndCapStyle) {
		gModified |= G_DRAWSTYLE_STROKEENDCAPSTYLE_MODIFIED;
		gStrokeEndCapStyle = CapStyle;
	}
}

// set stroke join style
void GDrawStyle::SetStrokeJoinStyle(const GJoinStyle JoinStyle) {

	if (JoinStyle != gStrokeJoinStyle) {
		gModified |= G_DRAWSTYLE_STROKEJOINSTYLE_MODIFIED;
		gStrokeJoinStyle = JoinStyle;
	}
}

// set stroke paint type
void GDrawStyle::SetStrokePaintType(const GPaintType Type) {

	if (Type != gStrokePaintType) {
		gModified |= G_DRAWSTYLE_STROKEPAINTTYPE_MODIFIED;
		gStrokePaintType = Type;
	}
}

// set stroke color
void GDrawStyle::SetStrokeColor(const GVectBase<GReal, 4>& Color) {

	if (Color != gStrokeColor) {
		gModified |= G_DRAWSTYLE_STROKECOLOR_MODIFIED;
		gStrokeColor = Color;
	}
}

// set stroke style
void GDrawStyle::SetStrokeStyle(const GStrokeStyle Style) {

	if (Style != gStrokeStyle) {
		gModified |= G_DRAWSTYLE_STROKESTYLE_MODIFIED;
		gStrokeStyle = Style;
	}
}

void GDrawStyle::UpdateOffsettedDashPattern() {

	gStrokeOffsettedDashPattern.clear();
	GUInt32 i = 0, j = (GUInt32)gStrokeDashPattern.size(), k;

	// no dash lengths, so exit
	if (j == 0)
		return;

	gStrokeDashPatternSum = 0;
	for (i = 0; i < j; i++)
		gStrokeDashPatternSum += (GInt32)gStrokeDashPattern[i];

	// all dash entries are 0, so exit
	if (gStrokeDashPatternSum == 0)
		return;

	// calculate phase modulo gStrokeDashPatternSum
	GReal phase;
	
	if (gStrokeDashPhase >= 0)
		phase = gStrokeDashPhase - gStrokeDashPatternSum * GMath::Floor(gStrokeDashPhase / gStrokeDashPatternSum);
	else
		phase = gStrokeDashPatternSum * GMath::Ceil(-gStrokeDashPhase / gStrokeDashPatternSum) + gStrokeDashPhase;

	i = 0;
	while (i < j && phase > 0) {
		phase -= gStrokeDashPattern[i];
		i++;
	}

	// calculate the flag that indicated if first gStrokeOffsettedDashPattern value will be an "off" value
	if (i & 1)
		gStrokeOffsettedDashPatternEmpty = G_FALSE;
	else
		gStrokeOffsettedDashPatternEmpty = G_TRUE;

	// this is the case when phase falls on a dash border
	if (phase == 0)
		gStrokeOffsettedDashPatternEmpty = !gStrokeOffsettedDashPatternEmpty;

	// we have fallen in the middle of a key
	if (phase < 0)
		gStrokeOffsettedDashPattern.push_back(-phase);

	for (k = 0; k < j - 1; k++) {
		gStrokeOffsettedDashPattern.push_back(gStrokeDashPattern[i % j]);
		i++;
	}

	gStrokeOffsettedDashPattern.push_back(gStrokeDashPattern[i % j] + phase);
}

// set stroke dash pattern
void GDrawStyle::SetStrokeDashPattern(const GDynArray<GReal> DashPattern) {

	GUInt32 i, j = (GUInt32)DashPattern.size();

	if (j == 0)
		return;

	// if an odd number of values is provided, then the list of values is repeated to yield an
	// even number of values
	if (j & 1) {
		GDynArray<GReal> repList;

		repList.reserve(j * 2);
		repList.insert(repList.begin(), DashPattern.begin(), DashPattern.end());
		repList.insert(repList.end(), DashPattern.begin(), DashPattern.end());

		gModified |= G_DRAWSTYLE_STROKEDASHPATTERN_MODIFIED;
		gStrokeDashPattern = repList;
	}
	else {
		gModified |= G_DRAWSTYLE_STROKEDASHPATTERN_MODIFIED;
		gStrokeDashPattern = DashPattern;
	}

	j = (GUInt32)gStrokeDashPattern.size();
	for (i = 0; i < j; i++)
		gStrokeDashPattern[i] = GMath::Abs(gStrokeDashPattern[i]);

	UpdateOffsettedDashPattern();
}

// set stroke dash phase
void GDrawStyle::SetStrokeDashPhase(const GReal DashPhase) {

	if (DashPhase != gStrokeDashPhase) {
		gModified |= G_DRAWSTYLE_STROKEDASHPHASE_MODIFIED;
		gStrokeDashPhase = DashPhase;
		UpdateOffsettedDashPattern();
	}
}

// set stroke compositing operation
void GDrawStyle::SetStrokeCompOp(const GCompositingOperation CompOp) {

	if (CompOp != gStrokeCompOp) {
		gModified |= G_DRAWSTYLE_STROKECOMPOP_MODIFIED;
		gStrokeCompOp = CompOp;
	}
}

// set stroke enabled
void GDrawStyle::SetStrokeEnabled(const GBool Enabled) {

	if (Enabled != gStrokeEnabled) {
		gModified |= G_DRAWSTYLE_STROKEENABLED_MODIFIED;
		gStrokeEnabled = Enabled;
	}
}

// set fill rule
void GDrawStyle::SetFillRule(const GFillRule Rule) {

	if (Rule != gFillRule) {
		gModified |= G_DRAWSTYLE_FILLRULE_MODIFIED;
		gFillRule = Rule;
	}
}

// set fill paint type
void GDrawStyle::SetFillPaintType(const GPaintType Type) {

	if (Type != gFillPaintType) {
		gModified |= G_DRAWSTYLE_FILLPAINTTYPE_MODIFIED;
		gFillPaintType = Type;
	}
}

// set fill color
void GDrawStyle::SetFillColor(const GVectBase<GReal, 4>& Color) {

	if (Color != gFillColor) {
		gModified |= G_DRAWSTYLE_FILLCOLOR_MODIFIED;
		gFillColor = Color;
	}
}

// set fill compositing operation
void GDrawStyle::SetFillCompOp(const GCompositingOperation CompOp) {

	if (CompOp != gFillCompOp) {
		gModified |= G_DRAWSTYLE_FILLCOMPOP_MODIFIED;
		gFillCompOp = CompOp;
	}
}

// set fill enabled
void GDrawStyle::SetFillEnabled(const GBool Enabled) {

	if (Enabled != gFillEnabled) {
		gModified |= G_DRAWSTYLE_FILLENABLED_MODIFIED;
		gFillEnabled = Enabled;
	}
}

// set model-view matrix
void GDrawStyle::SetModelView(const GMatrix33& Matrix) {

	if (gModelView != Matrix) {
		// calculate inverse model-view matrix
		GMatrix33 preTrans, tmpInv;
		GMatrix22 n, b;
		GBool ok;
		GReal det;

		TranslationToMatrix(preTrans, GPoint2(-Matrix[0][2], -Matrix[1][2]));
		n[0][0] = Matrix[0][0];
		n[0][1] = Matrix[0][1];
		n[1][0] = Matrix[1][0];
		n[1][1] = Matrix[1][1];
		ok = InvertFull_GJ(b, n, det);
		if (!ok) {
			G_DEBUG("GDrawStyle::SetModelView, matrix is singular!");
			return;
		}

		tmpInv[0][0] = b[0][0];
		tmpInv[0][1] = b[0][1];
		tmpInv[1][0] = b[1][0];
		tmpInv[1][1] = b[1][1];
		tmpInv[0][2] = 0;
		tmpInv[1][2] = 0;
		gInverseModelView = tmpInv * preTrans;

		gModified |= G_DRAWSTYLE_MODELVIEW_MODIFIED;
		gModelView = Matrix;
	}
}

// modified bit flags
GBool GDrawStyle::StrokeWidthModified() const {

	if (gModified & G_DRAWSTYLE_STROKEWIDTH_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeMiterLimitModified() const {

	if (gModified & G_DRAWSTYLE_STROKEMITERLIMIT_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeStartCapStyleModified() const {

	if (gModified & G_DRAWSTYLE_STROKESTARTCAPSTYLE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeEndCapStyleModified() const {

	if (gModified & G_DRAWSTYLE_STROKEENDCAPSTYLE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeJoinStyleModified() const {

	if (gModified & G_DRAWSTYLE_STROKEJOINSTYLE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokePaintTypeModified() const {

	if (gModified & G_DRAWSTYLE_STROKEPAINTTYPE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeStyleModified() const {

	if (gModified & G_DRAWSTYLE_STROKESTYLE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeColorModified() const {

	if (gModified & G_DRAWSTYLE_STROKECOLOR_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeDashPatternModified() const {

	if (gModified & G_DRAWSTYLE_STROKEDASHPATTERN_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeDashPhaseModified() const {

	if (gModified & G_DRAWSTYLE_STROKEDASHPHASE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeCompOpModified() const {

	if (gModified & G_DRAWSTYLE_STROKECOMPOP_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::StrokeEnabledModified() const {

	if (gModified & G_DRAWSTYLE_STROKEENABLED_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::FillRuleModified() const {

	if (gModified & G_DRAWSTYLE_FILLRULE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::FillPaintTypeModified() const {

	if (gModified & G_DRAWSTYLE_FILLPAINTTYPE_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::FillColorModified() const {

	if (gModified & G_DRAWSTYLE_FILLCOLOR_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::FillCompOpModified() const {

	if (gModified & G_DRAWSTYLE_FILLCOMPOP_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::FillEnabledModified() const {

	if (gModified & G_DRAWSTYLE_FILLENABLED_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

GBool GDrawStyle::ModelViewModified() const {

	if (gModified & G_DRAWSTYLE_MODELVIEW_MODIFIED)
		return G_TRUE;
	return G_FALSE;
}

void GDrawStyle::SetStrokeWidthModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKEWIDTH_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKEWIDTH_MODIFIED);
}

void GDrawStyle::SetStrokeMiterLimitModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKEMITERLIMIT_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKEMITERLIMIT_MODIFIED);
}

void GDrawStyle::SetStrokeStartCapStyleModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKESTARTCAPSTYLE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKESTARTCAPSTYLE_MODIFIED);
}

void GDrawStyle::SetStrokeEndCapStyleModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKEENDCAPSTYLE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKEENDCAPSTYLE_MODIFIED);
}

void GDrawStyle::SetStrokeJoinStyleModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKEJOINSTYLE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKEJOINSTYLE_MODIFIED);
}

void GDrawStyle::SetStrokePaintTypeModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKEPAINTTYPE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKEPAINTTYPE_MODIFIED);
}

void GDrawStyle::SetStrokeStyleModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKESTYLE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKESTYLE_MODIFIED);
}

void GDrawStyle::SetStrokeColorModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKECOLOR_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKECOLOR_MODIFIED);
}

void GDrawStyle::SetStrokeDashPatternModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKEDASHPATTERN_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKEDASHPATTERN_MODIFIED);
}

void GDrawStyle::SetStrokeDashPhaseModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKEDASHPHASE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKEDASHPHASE_MODIFIED);
}

void GDrawStyle::SetStrokeCompOpModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKECOMPOP_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKECOMPOP_MODIFIED);
}

void GDrawStyle::SetStrokeEnabledModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_STROKEENABLED_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_STROKEENABLED_MODIFIED);
}

void GDrawStyle::SetFillRuleModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_FILLRULE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_FILLRULE_MODIFIED);
}

void GDrawStyle::SetFillPaintTypeModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_FILLPAINTTYPE_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_FILLPAINTTYPE_MODIFIED);
}

void GDrawStyle::SetFillColorModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_FILLCOLOR_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_FILLCOLOR_MODIFIED);
}

void GDrawStyle::SetFillCompOpModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_FILLCOMPOP_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_FILLCOMPOP_MODIFIED);
}

void GDrawStyle::SetFillEnabledModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_FILLENABLED_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_FILLENABLED_MODIFIED);
}

void GDrawStyle::SetModelViewModified(const GBool Modified) {

	if (Modified)
		gModified |= G_DRAWSTYLE_MODELVIEW_MODIFIED;
	else
		gModified &= ((GUInt32)(~0) - G_DRAWSTYLE_MODELVIEW_MODIFIED);
}

#undef G_DRAWSTYLE_STROKEWIDTH_MODIFIED
#undef G_DRAWSTYLE_STROKEMITERLIMIT_MODIFIED
#undef G_DRAWSTYLE_STROKESTARTCAPSTYLE_MODIFIED
#undef G_DRAWSTYLE_STROKEENDCAPSTYLE_MODIFIED
#undef G_DRAWSTYLE_STROKEJOINSTYLE_MODIFIED
#undef G_DRAWSTYLE_STROKEPAINTTYPE_MODIFIED
#undef G_DRAWSTYLE_STROKECOLOR_MODIFIED
#undef G_DRAWSTYLE_STROKEDASHPHASE_MODIFIED
#undef G_DRAWSTYLE_STROKEDASHPATTERN_MODIFIED
#undef G_DRAWSTYLE_STROKECOMPOP_MODIFIED
#undef G_DRAWSTYLE_STROKEENABLED_MODIFIED
#undef G_DRAWSTYLE_FILLRULE_MODIFIED
#undef G_DRAWSTYLE_FILLPAINTTYPE_MODIFIED
#undef G_DRAWSTYLE_FILLCOLOR_MODIFIED
#undef G_DRAWSTYLE_FILLCOMPOP_MODIFIED
#undef G_DRAWSTYLE_FILLENABLED_MODIFIED
#undef G_DRAWSTYLE_MODELVIEW_MODIFIED

};	// end namespace Amanith
