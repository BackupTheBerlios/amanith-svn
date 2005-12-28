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

void QGLWidgetTest::TestStroke(const GUInt32 TestIndex) {
	
	GUInt32 idx = TestIndex & 0;

	gDrawBoard->SetCachingEnabled(G_FALSE);
	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetFillEnabled(G_FALSE);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeWidth(20);
	
	if (idx == 0) {
		// OPAQUE
		gDrawBoard->SetStrokeColor(GVector4((GReal)0.65, (GReal)0.65, (GReal)0.65, (GReal)1.0));
		// --------------------------------------------------------------
		
		gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
		gDrawBoard->SetStrokeJoinStyle(G_BEVEL_JOIN);
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(780, 582));
		gDrawBoard->DrawRectangle(GPoint2(50, 48), GPoint2(750, 552));
		gDrawBoard->DrawRectangle(GPoint2(80, 78), GPoint2(720, 522));
		gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
		gDrawBoard->DrawRectangle(GPoint2(110, 108), GPoint2(690, 492));
		gDrawBoard->DrawRectangle(GPoint2(140, 138), GPoint2(660, 462));
		gDrawBoard->DrawRectangle(GPoint2(170, 168), GPoint2(630, 432));
		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->DrawRectangle(GPoint2(200, 198), GPoint2(600, 402));
		gDrawBoard->DrawRectangle(GPoint2(230, 228), GPoint2(570, 372));
		gDrawBoard->DrawRectangle(GPoint2(260, 258), GPoint2(540, 342));
		
		gDrawBoard->SetStrokeColor(GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)0.5));
		
		gDrawBoard->SetStrokeStyle(G_DASHED_STROKE);
		
		gDrawBoard->SetStrokeJoinStyle(G_BEVEL_JOIN);
		gDrawBoard->SetStrokeStartCapStyle(G_SQUARE_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_SQUARE_CAP);
		gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(780, 582));
		gDrawBoard->SetStrokeStartCapStyle(G_BUTT_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
		gDrawBoard->DrawRectangle(GPoint2(50, 48), GPoint2(750, 552));	
		gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
		gDrawBoard->DrawRectangle(GPoint2(80, 78), GPoint2(720, 522));
		
		gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
		gDrawBoard->SetStrokeStartCapStyle(G_SQUARE_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_SQUARE_CAP);
		gDrawBoard->DrawRectangle(GPoint2(110, 108), GPoint2(690, 492));
		gDrawBoard->SetStrokeStartCapStyle(G_BUTT_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
		gDrawBoard->DrawRectangle(GPoint2(140, 138), GPoint2(660, 462));
		gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
		gDrawBoard->DrawRectangle(GPoint2(170, 168), GPoint2(630, 432));

		gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
		gDrawBoard->SetStrokeStartCapStyle(G_SQUARE_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_SQUARE_CAP);
		gDrawBoard->DrawRectangle(GPoint2(200, 198), GPoint2(600, 402));
		gDrawBoard->SetStrokeStartCapStyle(G_BUTT_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
		gDrawBoard->DrawRectangle(GPoint2(230, 228), GPoint2(570, 372));
		gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
		gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
		gDrawBoard->DrawRectangle(GPoint2(260, 258), GPoint2(540, 342));
	}
}
