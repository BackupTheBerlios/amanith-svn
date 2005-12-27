/****************************************************************************
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

#include "drawboard.h"
#include "amanith/geometry/gxformconv.h"

void QGLWidgetTest::TestMasks(const GUInt32 TestIndex) {

	GUInt32 idx = TestIndex % 2;
	GMatrix33 m;

	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeColor(GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)1.000));
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);

	gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)1.000));
	gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
	gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
	gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));
	gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
	gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
	gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));
	gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
	gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
	gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));

	if (idx == 0) {
		
		gLinGrad1->SetMatrix(m);
		gRadGrad1->SetMatrix(m);
		gRadGrad3->SetMatrix(m);
		gPattern->SetMatrix(m);

		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
		gDrawBoard->SetStrokeWidth(40);

		//------------------------------------------------------------------
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)0.9));
		gLinGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gLinGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillGradient(gLinGrad1);
		gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetStrokeGradient(gLinGrad1);
		gDrawBoard->DrawRectangle(GPoint2(50, 48), GPoint2(120, 87));
		gRadGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gDrawBoard->SetFillGradient(gRadGrad1);
		gDrawBoard->SetStrokeGradient(gRadGrad1);
		gDrawBoard->DrawRectangle(GPoint2(160, 48), GPoint2(230, 87));
		gRadGrad3->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad3->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gDrawBoard->SetFillGradient(gRadGrad3);
		gDrawBoard->SetStrokeGradient(gRadGrad3);
		gDrawBoard->DrawRectangle(GPoint2(50, 127), GPoint2(120, 165));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->SetStrokePattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(160, 127), GPoint2(230, 165));
		// ------------------------------------------------------------------------
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillColor(GVector4((GReal)0.6, (GReal)0.6, (GReal)0.6, (GReal)1.0));
		gDrawBoard->DrawRoundRectangle(GPoint2(50+260, 40), GPoint2(230+260, 172), GReal(20), GReal(20));
		// ------------------------------------------------------------------------
		gDrawBoard->SetTargetMode(G_CLIP_MODE);
		gDrawBoard->DrawRoundRectangle(GPoint2(50+520, 40), GPoint2(230+520, 172), GReal(20), GReal(20));
		TranslationToMatrix(m, GVector2(+520,+0));
		gDrawBoard->SetTargetMode(G_COLOR_MODE);
		gDrawBoard->SetStrokeEnabled(G_TRUE);
		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
		gDrawBoard->SetStrokeWidth(40);
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)0.9));
		gLinGrad1->SetMatrix(m);
		gLinGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gLinGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillGradient(gLinGrad1);
		gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetStrokeGradient(gLinGrad1);
		gDrawBoard->DrawRectangle(GPoint2(50+520, 48), GPoint2(120+520, 87));
		gRadGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad1->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad1);
		gDrawBoard->SetStrokeGradient(gRadGrad1);
		gDrawBoard->DrawRectangle(GPoint2(160+520, 48), GPoint2(230+520, 87));
		gRadGrad3->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad3->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad3->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad3);
		gDrawBoard->SetStrokeGradient(gRadGrad3);
		gDrawBoard->DrawRectangle(GPoint2(50+520, 127), GPoint2(120+520, 165));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_PATTERN_PAINT_TYPE);
		gPattern->SetMatrix(m);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->SetStrokePattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(160+520, 127), GPoint2(230+520, 165));
		gDrawBoard->PopClipMask();
		//------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------
		TranslationToMatrix(m, GVector2(+0,+194));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)0.9));
		gLinGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gLinGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gLinGrad1->SetMatrix(m);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillGradient(gLinGrad1);
		gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetStrokeGradient(gLinGrad1);
		gDrawBoard->DrawRectangle(GPoint2(50, 48+194), GPoint2(120, 87+194));
		gRadGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad1->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad1);
		gDrawBoard->SetStrokeGradient(gRadGrad1);
		gDrawBoard->DrawRectangle(GPoint2(160, 48+194), GPoint2(230, 87+194));
		gRadGrad3->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad3->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad3->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad3);
		gDrawBoard->SetStrokeGradient(gRadGrad3);
		gDrawBoard->DrawRectangle(GPoint2(50, 127+194), GPoint2(120, 165+194));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		gPattern->SetMatrix(m);
		gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->SetStrokePattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(160, 127+194), GPoint2(230, 165+194));
		// ------------------------------------------------------------------------
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillColor(GVector4((GReal)0.6, (GReal)0.6, (GReal)0.6, (GReal)1.0));
		gDrawBoard->DrawRoundRectangle(GPoint2(50+260, 40+194), GPoint2(195+260, 132+194), GReal(20), GReal(20));
		gDrawBoard->DrawRoundRectangle(GPoint2(85+260, 80+194), GPoint2(230+260, 172+194), GReal(20), GReal(20));
		// ------------------------------------------------------------------------
		gDrawBoard->SetTargetMode(G_CLIP_MODE);

		gDrawBoard->GroupBegin();
		gDrawBoard->DrawRoundRectangle(GPoint2(50+520, 40+194), GPoint2(195+520, 132+194), GReal(20), GReal(20));
		gDrawBoard->DrawRoundRectangle(GPoint2(85+520, 80+194), GPoint2(230+520, 172+194), GReal(20), GReal(20));
		gDrawBoard->GroupEnd();

		TranslationToMatrix(m, GVector2(+520,+194));
		gDrawBoard->SetStrokeEnabled(G_TRUE);
		gDrawBoard->SetTargetMode(G_COLOR_MODE);
		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
		gDrawBoard->SetStrokeWidth(40);
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)0.9));
		gLinGrad1->SetMatrix(m);
		gLinGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gLinGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillGradient(gLinGrad1);
		gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetStrokeGradient(gLinGrad1);
		gDrawBoard->DrawRectangle(GPoint2(50+520, 48+194), GPoint2(120+520, 87+194));
		gRadGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad1->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad1);
		gDrawBoard->SetStrokeGradient(gRadGrad1);
		gDrawBoard->DrawRectangle(GPoint2(160+520, 48+194), GPoint2(230+520, 87+194));
		gRadGrad3->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad3->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad3->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad3);
		gDrawBoard->SetStrokeGradient(gRadGrad3);
		gDrawBoard->DrawRectangle(GPoint2(50+520, 127+194), GPoint2(120+520, 165+194));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_PATTERN_PAINT_TYPE);
		gPattern->SetMatrix(m);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->SetStrokePattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(160+520, 127+194), GPoint2(230+520, 165+194));
		gDrawBoard->PopClipMask();
		//------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------
		TranslationToMatrix(m, GVector2(+0,+388));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)0.9));
		gLinGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gLinGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gLinGrad1->SetMatrix(m);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillGradient(gLinGrad1);
		gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetStrokeGradient(gLinGrad1);
		gDrawBoard->DrawRectangle(GPoint2(50, 48+388), GPoint2(120, 87+388));
		gRadGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad1->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad1);
		gDrawBoard->SetStrokeGradient(gRadGrad1);
		gDrawBoard->DrawRectangle(GPoint2(160, 48+388), GPoint2(230, 87+388));
		gRadGrad3->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad3->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad3->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad3);
		gDrawBoard->SetStrokeGradient(gRadGrad3);
		gDrawBoard->DrawRectangle(GPoint2(50, 127+388), GPoint2(120, 165+388));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		gPattern->SetMatrix(m);
		gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->SetStrokePattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(160, 127+388), GPoint2(230, 165+388));
		// ------------------------------------------------------------------------
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillColor(GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)0.4));
		gDrawBoard->DrawRoundRectangle(GPoint2(50+260, 40+388), GPoint2(195+260, 132+388), GReal(20), GReal(20));
		gDrawBoard->DrawRoundRectangle(GPoint2(85+260, 80+388), GPoint2(230+260, 172+388), GReal(20), GReal(20));
		// ------------------------------------------------------------------------
		gDrawBoard->SetTargetMode(G_CLIP_MODE);
		gDrawBoard->DrawRoundRectangle(GPoint2(50+520, 40+388), GPoint2(195+520, 132+388), GReal(20), GReal(20));
		gDrawBoard->DrawRoundRectangle(GPoint2(85+520, 80+388), GPoint2(230+520, 172+388), GReal(20), GReal(20));
		TranslationToMatrix(m, GVector2(+520,+388));
		gDrawBoard->SetStrokeEnabled(G_TRUE);
		gDrawBoard->SetTargetMode(G_COLOR_MODE);
		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
		gDrawBoard->SetStrokeWidth(40);
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)0.9));
		gLinGrad1->SetMatrix(m);
		gLinGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gLinGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillGradient(gLinGrad1);
		gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetStrokeGradient(gLinGrad1);
		gDrawBoard->DrawRectangle(GPoint2(50+520, 48+388), GPoint2(120+520, 87+388));
		gRadGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad1->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad1);
		gDrawBoard->SetStrokeGradient(gRadGrad1);
		gDrawBoard->DrawRectangle(GPoint2(160+520, 48+388), GPoint2(230+520, 87+388));
		gRadGrad3->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gRadGrad3->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		gRadGrad3->SetMatrix(m);
		gDrawBoard->SetFillGradient(gRadGrad3);
		gDrawBoard->SetStrokeGradient(gRadGrad3);
		gDrawBoard->DrawRectangle(GPoint2(50+520, 127+388), GPoint2(120+520, 165+388));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_PATTERN_PAINT_TYPE);
		gPattern->SetMatrix(m);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->SetStrokePattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(160+520, 127+388), GPoint2(230+520, 165+388));
		gDrawBoard->PopClipMask();
		gDrawBoard->PopClipMask();
	}
	else
	if (idx == 1) {
		gLinGrad3->SetMatrix(m);
		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
		gDrawBoard->SetStrokeWidth(40);
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)0.5));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)0.5));
		gDrawBoard->DrawRectangle(GPoint2(90, 60+18), GPoint2(195, 112+18));
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetGroupOpacity((GReal)0.5);
		gDrawBoard->GroupBegin(GAABox2(GPoint2(90+200, 40), GPoint2(195+300, 112+40)));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->DrawRectangle(GPoint2(90+260, 60+18), GPoint2(195+260, 112+18));
		gDrawBoard->GroupEnd();

		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillColor(GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillGradient(gLinGrad3);
		gDrawBoard->DrawRectangle(GPoint2(550, 28), GPoint2(770, 184));
		gDrawBoard->SetStrokeEnabled(G_TRUE);
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetGroupOpacity((GReal)0.5);
		gDrawBoard->GroupBegin(GAABox2(GPoint2(90+460, 40), GPoint2(195+560, 112+40)));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->DrawRectangle(GPoint2(90+520, 60+18), GPoint2(195+520, 112+18));
		gDrawBoard->GroupEnd();
		gDrawBoard->SetStrokeStyle(G_DASHED_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeWidth(20);
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)0.5));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)0.5));
		gDrawBoard->DrawRectangle(GPoint2(70, 45+194), GPoint2(175, 97+194));
		gDrawBoard->DrawRectangle(GPoint2(110, 70+194), GPoint2(210, 168+194));
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetGroupOpacity((GReal)0.5);
		gDrawBoard->GroupBegin(GAABox2(GPoint2(280, 212), GPoint2(520, 388)));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->DrawRectangle(GPoint2(70+260, 45+194), GPoint2(175+260, 97+194));
		gDrawBoard->DrawRectangle(GPoint2(110+260, 70+194), GPoint2(210+260, 168+194));
		gDrawBoard->GroupEnd();
		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillColor(GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillGradient(gLinGrad3);
		gDrawBoard->DrawRectangle(GPoint2(550, 28+194), GPoint2(770, 184+194));
		gDrawBoard->SetStrokeEnabled(G_TRUE);
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetGroupOpacity((GReal)0.5);
		gDrawBoard->GroupBegin(GAABox2(GPoint2(540, 212), GPoint2(780, 388)));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->DrawRectangle(GPoint2(70+520, 45+194), GPoint2(175+520, 97+194));
		gDrawBoard->DrawRectangle(GPoint2(110+520, 70+194), GPoint2(210+520, 168+194));
		gDrawBoard->GroupEnd();

		gDrawBoard->SetStrokeStyle(G_DASHED_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeWidth(20);
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)0.5));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)0.5));
		gDrawBoard->DrawRectangle(GPoint2(70, 45+388), GPoint2(175, 97+388));
		gDrawBoard->DrawRectangle(GPoint2(110, 70+388), GPoint2(210, 168+388));
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetGroupOpacity((GReal)0.5);
		gDrawBoard->GroupBegin(GAABox2(GPoint2(280, 406), GPoint2(520, 582)));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->DrawRectangle(GPoint2(70+260, 45+388), GPoint2(175+260, 97+388));
		gDrawBoard->GroupEnd();
		gDrawBoard->GroupBegin(GAABox2(GPoint2(280, 406), GPoint2(520, 582)));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->DrawRectangle(GPoint2(110+260, 70+388), GPoint2(210+260, 168+388));
		gDrawBoard->GroupEnd();
		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetFillColor(GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillGradient(gLinGrad3);
		gDrawBoard->DrawRectangle(GPoint2(550, 28+388), GPoint2(770, 184+388));
		gDrawBoard->SetStrokeEnabled(G_TRUE);
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetGroupOpacity((GReal)0.5);
		gDrawBoard->GroupBegin(GAABox2(GPoint2(540, 406), GPoint2(780, 582)));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->DrawRectangle(GPoint2(70+520, 45+388), GPoint2(175+520, 97+388));
		gDrawBoard->GroupEnd();
		gDrawBoard->GroupBegin(GAABox2(GPoint2(540, 406), GPoint2(780, 582)));
		gDrawBoard->SetStrokeColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
		gDrawBoard->DrawRectangle(GPoint2(110+520, 70+388), GPoint2(210+520, 168+388));
		gDrawBoard->GroupEnd();
	}
}
