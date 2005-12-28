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

void TestConicalGradientIn(const GUInt32 TestIndex, const GReal RotAngle, const GReal ScaleX, const GReal ScaleY) {
	
	GUInt32 idx = (TestIndex % 4);

	GMatrix33 ofsTransf, m;
	GMatrix33 rot, scl, preTrans, postTrans;

	TranslationToMatrix(preTrans, GPoint2(-120, -88));
	TranslationToMatrix(postTrans, GPoint2(120, 88));
	RotationToMatrix(rot, RotAngle);

	ScaleToMatrix(scl, GVector2(ScaleX, ScaleY));
	ofsTransf = (postTrans * (rot * (scl * preTrans)));

	gDrawBoard->SetCachingEnabled(G_FALSE);
	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.000));
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
	
	// OPAQUE
	if (idx == 0) {
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 1.0));
		// --------------------------------------------------------------
		gConGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gConGrad1->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+18));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
		gConGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+18));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
		gConGrad1->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+18));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));
		// --------------------------------------------------------------
		gConGrad1->SetColorInterpolation(G_LINEAR_COLOR_INTERPOLATION);
		gConGrad1->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+212));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
		gConGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+212));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
		gConGrad1->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+212));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));
		// --------------------------------------------------------------
		gConGrad1->SetColorInterpolation(G_CONSTANT_COLOR_INTERPOLATION);
		gConGrad1->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+406));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
		gConGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+406));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
		gConGrad1->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+406));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);	
		gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));
		// --------------------------------------------------------------
	}
	else
	if (idx == 1) {
		// TRANSPARENT
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.5));
		// --------------------------------------------------------------
		gConGrad1->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gConGrad1->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+18));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
		gConGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+18));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
		gConGrad1->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+18));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));
		// --------------------------------------------------------------
		gConGrad1->SetColorInterpolation(G_LINEAR_COLOR_INTERPOLATION);
		gConGrad1->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+212));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
		gConGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+212));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
		gConGrad1->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+212));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));
		// --------------------------------------------------------------
		gConGrad1->SetColorInterpolation(G_CONSTANT_COLOR_INTERPOLATION);
		gConGrad1->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+406));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
		gConGrad1->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+406));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);
		gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
		gConGrad1->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+406));
		gConGrad1->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad1);	
		gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));
		// --------------------------------------------------------------
	}
	else
	if (idx == 2) {
		// TRANSPARENT IN KEYS
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 1.0));
		// --------------------------------------------------------------
		gConGrad2->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gConGrad2->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+18));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
		gConGrad2->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+18));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
		gConGrad2->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+18));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));
		// --------------------------------------------------------------
		gConGrad2->SetColorInterpolation(G_LINEAR_COLOR_INTERPOLATION);
		gConGrad2->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+212));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
		gConGrad2->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+212));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
		gConGrad2->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+212));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));
		// --------------------------------------------------------------
		gConGrad2->SetColorInterpolation(G_CONSTANT_COLOR_INTERPOLATION);
		gConGrad2->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+406));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
		gConGrad2->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+406));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
		gConGrad2->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+406));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);	
		gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));
		// --------------------------------------------------------------
	}
	else {
		// TRANSPARENT IN KEYS AND COLOR
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.5));
		// --------------------------------------------------------------
		gConGrad2->SetColorInterpolation(G_HERMITE_COLOR_INTERPOLATION);
		gConGrad2->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+18));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
		gConGrad2->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+18));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
		gConGrad2->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+18));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));
		// --------------------------------------------------------------
		gConGrad2->SetColorInterpolation(G_LINEAR_COLOR_INTERPOLATION);
		gConGrad2->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+212));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
		gConGrad2->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+212));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
		gConGrad2->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+212));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));
		// --------------------------------------------------------------
		gConGrad2->SetColorInterpolation(G_CONSTANT_COLOR_INTERPOLATION);
		gConGrad2->SetSpreadMode(G_PAD_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+20,+406));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
		gConGrad2->SetSpreadMode(G_REPEAT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+280,+406));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);
		gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
		gConGrad2->SetSpreadMode(G_REFLECT_COLOR_RAMP_SPREAD);
		TranslationToMatrix(m, GVector2(+540,+406));
		gConGrad2->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillGradient(gConGrad2);	
		gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));
		// --------------------------------------------------------------
	}
}
