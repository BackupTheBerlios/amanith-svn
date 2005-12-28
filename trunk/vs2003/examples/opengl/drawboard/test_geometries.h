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

void TestGeometries(const GUInt32 TestIndex) {

	GUInt32 idx = TestIndex % 2;
	
	GDynArray<GPoint2> pts;

	gDrawBoard->SetCachingEnabled(G_FALSE);
	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeColor(GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)1.000));
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);

	if (idx == 0) {

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
		gDrawBoard->SetFillEnabled(G_FALSE);
		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeWidth(10);
		gDrawBoard->DrawLine(GPoint2(40,38),GPoint2(240,174));
		gDrawBoard->DrawBezier(GPoint2(300,38),GPoint2(500,38),GPoint2(500,174));
		gDrawBoard->DrawBezier(GPoint2(560,38),GPoint2(960,38),GPoint2(360,174),GPoint2(760,174));
		gDrawBoard->DrawEllipseArc(GPoint2(55,282),GPoint2(240,368),(GReal)100,(GReal)50, (GReal)0.79, G_TRUE, G_TRUE);
		gDrawBoard->DrawEllipseArc(GPoint2(400,300), (GReal)90, (GReal)70, (GReal)0.0,(GReal)0.0, (GReal)3.14, G_TRUE);
		
		pts.push_back(GPoint2(560,232));
		pts.push_back(GPoint2(660,282));
		pts.push_back(GPoint2(600,322));
		pts.push_back(GPoint2(680,360));
		pts.push_back(GPoint2(760,232));

		gDrawBoard->DrawRoundRectangle(GPoint2(50, 436), GPoint2(230, 552), (GReal)20, (GReal)20);
		gDrawBoard->DrawCircle(GPoint2(400, 494), (GReal)70);
		gDrawBoard->DrawEllipse(GPoint2(660, 494), (GReal)90, (GReal)70);

		gDrawBoard->DrawPolygon(pts,G_FALSE);
	
	}
	else
	if (idx == 1) {

		gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)1.000));
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(780, 582));

		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
		gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);

		gDrawBoard->SetStrokeEnabled(G_TRUE);
		gDrawBoard->SetFillEnabled(G_FALSE);
		gDrawBoard->SetStrokeWidth(48);
		gDrawBoard->DrawPaths("M 407,77 C 292,71 294,128 325,222 C 281,226 240,228 208,215 C 256,448 358,511 407,512 C 454,511 545,450 593,215 C 564,204 527,203 486,206 C 508,122 507,72 407,77 z");

		gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->SetStrokeWidth(12);
		gDrawBoard->SetFillGradient(gLinGradLogo1);
		gDrawBoard->SetFillEnabled(G_TRUE);

		gDrawBoard->DrawPaths("M 406,74 C 257,67 305,163 359,313 Q 406,361 454,314 C 496,164 544,67 406,74 z");
		gDrawBoard->SetFillGradient(gLinGradLogo2);
		gDrawBoard->DrawPaths("M 208,211 C 257,446 358,508 406,508 C 455,508 545,446 593,211 C 467,173 305,250 208,211 z");

		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillGradient(gLinGradLogo3);
		gDrawBoard->DrawPaths("M 250,266 C 250,326 336,482 406,482 C 455,482 478,422 478,422 C 478,374 263,218 250,266 z");
	}
}
