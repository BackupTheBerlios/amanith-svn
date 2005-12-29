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

void TestCache(const GUInt32 TestIndex) {

	//GUInt32 idx = TestIndex % 2;
	GUInt32 idx = TestIndex & 0;
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

		GDynArray<GPoint2> pts;
		pts.push_back(GPoint2(420, 38));
		pts.push_back(GPoint2(500, 38));
		pts.push_back(GPoint2(420, 174));
		pts.push_back(GPoint2(500, 174));

		// invalidate cache bank
		gCacheBank->Invalidate();
		gDrawBoard->SetCacheBank(gCacheBank);
		gDrawBoard->SetFillEnabled(G_FALSE);
		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeWidth(10);

		// enable writing only on cache
		gDrawBoard->SetTargetMode(G_CACHE_MODE);

		gDrawBoard->DrawLine(GPoint2(40, 38), GPoint2(240, 174));
		gDrawBoard->DrawRoundRectangle(GPoint2(300, 38), GPoint2(400, 174), 20, 20);
		gDrawBoard->DrawPolygon(pts, G_TRUE);
		gDrawBoard->DrawPaths("M 560,106 C 560,38 760,38 760,106 C 760,174 560,174 560,106 M 660,140 C 560,48 760,48 660,140");
		// now draw cached primitives on screen
		gDrawBoard->SetTargetMode(G_COLOR_MODE);
		gDrawBoard->SetFillEnabled(G_TRUE);
		gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
		gDrawBoard->SetFillColor(1, 0, 0, 1);
		gDrawBoard->DrawCacheBank();

		// draw on both cache and screen
		gDrawBoard->SetTargetMode(G_COLOR_AND_CACHE_MODE);
		TranslationToMatrix(m, GVector2(0, 194));
		gDrawBoard->SetModelViewMatrix(m);
		gDrawBoard->DrawLine(GPoint2(40, 38), GPoint2(240, 174));
		gDrawBoard->DrawRoundRectangle(GPoint2(300, 38), GPoint2(400, 174), 20, 20);
		gDrawBoard->DrawPolygon(pts, G_TRUE);
		gDrawBoard->DrawPaths("M 560,106 C 560,38 760,38 760,106 C 760,174 560,174 560,106 M 660,140 C 560,48 760,48 660,140");

		// change drawing style and draw some cached primitives using a translation on top of the windows
		gDrawBoard->SetTargetMode(G_COLOR_MODE);
		TranslationToMatrix(m, GVector2(0, 388));
		gDrawBoard->SetModelViewMatrix(m);

		gDrawBoard->SetStrokeColor(0, 0, 0, (GReal)0.5);
		gDrawBoard->SetFillColor(0, 0, 1, (GReal)0.5);
		// draw cached slots [4..7]
		gDrawBoard->DrawCacheSlots(4, 7);
		// reset modelview matrix to identity
		gDrawBoard->SetModelViewMatrix(G_MATRIX_IDENTITY33);
	}
}
