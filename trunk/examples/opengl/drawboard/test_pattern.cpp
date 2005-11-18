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

void QGLWidgetTest::TestPattern(const GUInt32 TestIndex, const GReal RotAngle, const GReal Scale) {
	
	GUInt32 idx = TestIndex & 0;//(TestIndex % 4);

	GMatrix33 ofsTransf, m;
	GMatrix33 rot, scl;

	RotationToMatrix(rot, RotAngle);
	ScaleToMatrix(scl, GVector2(Scale, Scale));
	ofsTransf = (rot * scl);

	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.000));
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
	
	if (idx == 0) {
		// --------------------------------------------------------------
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 1.000));
		gPattern->SetTilingMode(G_PAD_TILE);
		TranslationToMatrix(m, GVector2(+140,+106));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
		gPattern->SetTilingMode(G_REPEAT_TILE);
		TranslationToMatrix(m, GVector2(+400,+106));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		TranslationToMatrix(m, GVector2(+660,+106));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));
		
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.666));
		gPattern->SetTilingMode(G_PAD_TILE);
		TranslationToMatrix(m, GVector2(+140,+300));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
		gPattern->SetTilingMode(G_REPEAT_TILE);
		TranslationToMatrix(m, GVector2(+400,+300));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		TranslationToMatrix(m, GVector2(+660,+300));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));
		
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.333));
		gPattern->SetTilingMode(G_PAD_TILE);
		TranslationToMatrix(m, GVector2(+140,+494));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
		gPattern->SetTilingMode(G_REPEAT_TILE);
		TranslationToMatrix(m, GVector2(+400,+494));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
		gPattern->SetTilingMode(G_REFLECT_TILE);
		TranslationToMatrix(m, GVector2(+660,+494));
		gPattern->SetMatrix(m * ofsTransf);
		gDrawBoard->SetFillPattern(gPattern);
		gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));
	}
}
