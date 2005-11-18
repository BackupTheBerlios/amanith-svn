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

void QGLWidgetTest::TestColor(const GUInt32 TestIndex) {

	GUInt32 idx = TestIndex & 0;//(TestIndex % 4);

	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.000));
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);

	if (idx == 0) {
		// --------------------------------------------------------------
		gDrawBoard->SetFillColor(GVector4(1.0, 0.0, 0.0, 1.000));
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
		gDrawBoard->SetFillColor(GVector4(0.0, 1.0, 1.0, 1.000));
		gDrawBoard->DrawRectangle(GPoint2(80, 78), GPoint2(200, 134));
		gDrawBoard->SetFillColor(GVector4(1.0, 0.0, 0.0, 0.666));
		gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
		gDrawBoard->SetFillColor(GVector4(0.0, 1.0, 1.0, 0.666));
		gDrawBoard->DrawRectangle(GPoint2(340, 78), GPoint2(460, 134));
		gDrawBoard->SetFillColor(GVector4(1.0, 0.0, 0.0, 0.333));
		gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));
		gDrawBoard->SetFillColor(GVector4(0.0, 1.0, 1.0, 0.333));
		gDrawBoard->DrawRectangle(GPoint2(600, 78), GPoint2(720, 134));
		// --------------------------------------------------------------
		gDrawBoard->SetFillColor(GVector4(0.0, 1.0, 0.0, 1.000));
		gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
		gDrawBoard->SetFillColor(GVector4(1.0, 0.0, 1.0, 1.000));
		gDrawBoard->DrawRectangle(GPoint2(80, 272), GPoint2(200, 328));
		gDrawBoard->SetFillColor(GVector4(0.0, 1.0, 0.0, 0.666));
		gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
		gDrawBoard->SetFillColor(GVector4(1.0, 0.0, 1.0, 0.666));
		gDrawBoard->DrawRectangle(GPoint2(340, 272), GPoint2(460, 328));
		gDrawBoard->SetFillColor(GVector4(0.0, 1.0, 0.0, 0.333));
		gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));
		gDrawBoard->SetFillColor(GVector4(1.0, 0.0, 1.0, 0.333));
		gDrawBoard->DrawRectangle(GPoint2(600, 272), GPoint2(720, 328));
		// --------------------------------------------------------------
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 1.0, 1.000));
		gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
		gDrawBoard->SetFillColor(GVector4(1.0, 1.0, 0.0, 1.000));
		gDrawBoard->DrawRectangle(GPoint2(80, 466), GPoint2(200, 522));
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 1.0, 0.666));
		gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
		gDrawBoard->SetFillColor(GVector4(1.0, 1.0, 0.0, 0.666));
		gDrawBoard->DrawRectangle(GPoint2(340, 466), GPoint2(460, 522));
		gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 1.0, 0.333));
		gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));
		gDrawBoard->SetFillColor(GVector4(1.0, 1.0, 0.0, 0.333));
		gDrawBoard->DrawRectangle(GPoint2(600, 466), GPoint2(720, 522));
	}
}
