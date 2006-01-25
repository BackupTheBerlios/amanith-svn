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

#include "clock.h"
#include "amanith/geometry/gxformconv.h"
#include "amanith/2d/gpath2d.h"
#include "amanith/2d/gbeziercurve2d.h"
#include "amanith/2d/gbsplinecurve2d.h"
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
	#include <QTime>
#else
	#include <qdatetime.h>
#endif

// timer interval (millisec)
static int timer_interval = 0;

// constructor
QGLWidgetTest::QGLWidgetTest(const QGLFormat& Format, QWidget *parent) : QGLWidget(Format, parent) {

#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	gKernel = new GKernel();

	// set an 256x256 window
	this->setGeometry(50, 50, 256, 256);
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gKernel)
		delete gKernel;
	if (gDrawBoard)
		delete gDrawBoard;
}

//----- initializeGL -----------------------------------------
void QGLWidgetTest::initializeGL() {

	GDynArray<GKeyValue> colKeys;

	gDrawBoard = new GOpenGLBoard(0, 0, geometry().width(), geometry().height());
	gDrawBoard->SetProjection(0, 512, 0, 512);
	gDrawBoard->SetRenderingQuality(G_HIGH_RENDERING_QUALITY);

	// shadow gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.85, GVector4((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.95, GVector4((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)0.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)0.0)));
	gShadowGrad = gDrawBoard->CreateRadialGradient(GPoint2(260, 250), GPoint2(260, 250), 270, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// clock quadrant background gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.86, (GReal)0.87, (GReal)0.88, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.60, GVector4((GReal)0.76, (GReal)0.77, (GReal)0.78, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.36, (GReal)0.36, (GReal)0.40, (GReal)1.0)));
	gRadGrad1 = gDrawBoard->CreateRadialGradient(GPoint2(236, 276), GPoint2(235, 275), 256, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// outer/inner bevel ring gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)1.0, (GReal)1.00, (GReal)1.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.40, GVector4((GReal)0.9, (GReal)0.90, (GReal)0.92, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.80, GVector4((GReal)0.46, (GReal)0.47, (GReal)0.50, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.26, (GReal)0.27, (GReal)0.30, (GReal)1.0)));
	gRadGrad2 = gDrawBoard->CreateRadialGradient(GPoint2(120, 390), GPoint2(-121, 591), 400, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// milliseconds quadrant background gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.9, (GReal)0.95, (GReal)0.95, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.85, (GReal)0.85, (GReal)0.9, (GReal)0.5)));
	gRadGrad3 = gDrawBoard->CreateRadialGradient(GPoint2(340, 270), GPoint2(324, 280), 60, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// create a cache bank, and cache the clock
	gCacheBank = gDrawBoard->CreateCacheBank();
	CacheClock();

	startTimer(timer_interval);
}

void QGLWidgetTest::CacheBack() {

	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
	gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
	gDrawBoard->SetStrokeWidth(5);

	// shadow
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(270, 242, 270);
	// clock background
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 240);
	// milliseconds quadrant
	gDrawBoard->SetStrokeWidth(3);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(356, 256, 26);
	// inner bevel ring
	gDrawBoard->SetStrokeWidth(10);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 208);
	// outer bevel ring
	gDrawBoard->SetStrokeWidth(35);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 230);
	// seconds tag
	gDrawBoard->SetStrokeWidth(1);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 442, 256, 446);
	// hours tag
	gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeWidth(8);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 434, 256, 446);
	// 1000 / 12 milliseconds tag
	gDrawBoard->SetStrokeWidth(1);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(356, 274, 356, 276);
}

void QGLWidgetTest::CacheCursors() {

	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);

	// milliseconds	
	gDrawBoard->SetStrokeWidth(1);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(356, 250, 356, 254);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(356, 256, 2);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(356, 258, 356, 270);
	// seconds	
	gDrawBoard->SetStrokeWidth(2);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 223, 256, 248);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 8);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 264, 256, 410);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 415, 5);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 420, 256, 443);
	// minutes
	gDrawBoard->SetStrokeWidth(4);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 240, 256, 420);
	// hours
	gDrawBoard->SetStrokeWidth(8);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 246, 256, 380);
	// cursors screw
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 6);
}

void QGLWidgetTest::CacheGlass() {

	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
	gDrawBoard->SetStrokeWidth(1);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawPaths("M 162,447 C 365,540 564,309 419,118 Q 400,350 162,447 z");
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawPaths("M 256,43 C 83,45 4,231 62,340 Q 80,110 256,43 z");
}

void QGLWidgetTest::CacheClock() {

	gSlotsIndex = 0;
	gDrawBoard->SetTargetMode(G_CACHE_MODE);
	gDrawBoard->SetCacheBank(gCacheBank);
	gDrawBoard->SetRenderingQualityDeviation((GReal)0.01);
	CacheBack();
	CacheCursors();
	CacheGlass();
}

void QGLWidgetTest::DrawBack() {

	GUInt32 i;
	GReal kk;
	GMatrix33 rot, ks_rot;

	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetFillCompOp(G_SRC_OVER_OP);

	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
	gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);

	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetFillGradient(gShadowGrad);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);


	gDrawBoard->SetFillGradient(gRadGrad1);
	gDrawBoard->SetStrokeGradient(gRadGrad1);
	gRadGrad1->SetMatrix(rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);

	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(3);
	gDrawBoard->SetStrokeGradient(gRadGrad2);
	gDrawBoard->SetFillGradient(gRadGrad3);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);

	gDrawBoard->SetFillEnabled(G_FALSE);

	gDrawBoard->SetStrokeOpacity(1.0);
	gDrawBoard->SetStrokeGradient(gRadGrad2);
	gRadGrad2->SetMatrix(rot);
	gDrawBoard->SetStrokeWidth(10);
	gDrawBoard->SetStrokeCompOp(G_SRC_OP);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	RotationToMatrix(rot, (GReal)G_PI, GPoint2(256, 256));
	gRadGrad2->SetMatrix(rot);
	gDrawBoard->SetStrokeWidth(35);
	gDrawBoard->SetStrokeCompOp(G_SRC_OP);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);


	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
	Identity(rot);
	gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeColor((GReal)0.11, (GReal)0.12, (GReal)0.13, (GReal)0.5);
	gDrawBoard->SetStrokeWidth(1);
	for (i = 0; i < 60; ++i) {
		if ((i % 5) != 0) {
			kk = (GReal)G_2PI - (((GReal)i / 60) * (GReal)G_2PI);
			RotationToMatrix(rot, kk, GPoint2(256, 256));
			gDrawBoard->SetModelViewMatrix(rot);
			gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex]);
			Identity(rot);
		}
	}
	gSlotsIndex++;

	for (i = 0; i < 12; ++i) {
		kk = G_2PI - (((GReal)i / 12) * G_2PI);
		RotationToMatrix(rot, kk, GPoint2(256, 256));
		RotationToMatrix(ks_rot, kk, GPoint2(356, 256));
		gDrawBoard->SetModelViewMatrix(rot);
		gDrawBoard->SetStrokeWidth(10);
		gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex]);
		gDrawBoard->SetModelViewMatrix(ks_rot);
		gDrawBoard->SetStrokeWidth(1);
		gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex + 1]);
		Identity(rot);
	}
	gSlotsIndex += 2;
	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
}

void QGLWidgetTest::DrawCursors(const GUInt32 Hours, const GUInt32 Minutes, const GUInt32 Seconds, const GUInt32 Milliseconds) {

	GReal hh, mm, ss, ks;
	GMatrix33 rot, ks_rot;

	if (Hours < 13)
		hh = G_2PI - ( ((GReal)Hours / 12 + (GReal)Minutes / 720) * G_2PI);
	else 
		hh = G_2PI - ( ((GReal)(Hours - 12) / 12 + (GReal)Minutes / 720) * G_2PI);
	mm = G_2PI - (((GReal)Minutes / 60 + (GReal)Seconds / 3600) * G_2PI);
	ss = G_2PI - (((GReal)Seconds / 60 + (GReal)Milliseconds / 60000) * G_2PI);
	ks = G_2PI - (((GReal)Milliseconds / 1000) * G_2PI);

	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);

	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetFillColor((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)1.0);
	gDrawBoard->SetFillEnabled(G_FALSE);


	// milliseconds	
	gDrawBoard->SetStrokeWidth(1);
	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetStrokeColor((GReal)0.2, (GReal)0.4, (GReal)0.8, (GReal)1.0);
	RotationToMatrix(ks_rot, ks, GPoint2(356, 256));
	gDrawBoard->SetModelViewMatrix(ks_rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->SetFillEnabled(G_FALSE);


	// seconds	
	gDrawBoard->SetStrokeWidth(2);
	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetStrokeColor((GReal)0.8, (GReal)0.2, (GReal)0.1, (GReal)1.0);
	RotationToMatrix(rot, ss, GPoint2(256, 256));
	gDrawBoard->SetModelViewMatrix(rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->SetFillEnabled(G_FALSE);
	Identity(rot);

	// minutes
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetStrokeColor((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)1.0);
	RotationToMatrix(rot, mm, GPoint2(256, 256));
	gDrawBoard->SetModelViewMatrix(rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	Identity(rot);

	// hours
	gDrawBoard->SetStrokeWidth(8);
	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetStrokeColor((GReal)0.25, (GReal)0.25, (GReal)0.25, (GReal)1.0);
	RotationToMatrix(rot, hh, GPoint2(256, 256));
	gDrawBoard->SetModelViewMatrix(rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	Identity(rot);

	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetFillColor((GReal)0.25, (GReal)0.25, (GReal)0.25, (GReal)1.0);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
}

void QGLWidgetTest::DrawGlass() {

	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetModelViewMatrix(G_MATRIX_IDENTITY33);
	gDrawBoard->SetFillCompOp(G_PLUS_OP);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetFillColor((GReal)0.05, (GReal)0.05, (GReal)0.05, (GReal)1.0);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
}

void QGLWidgetTest::DrawClock() {

	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
	gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
	gDrawBoard->SetModelViewMatrix(G_MATRIX_IDENTITY33);

	gSlotsIndex = 0;
	DrawBack();

	QTime curTime = QTime::currentTime();

	DrawCursors(curTime.hour(), curTime.minute(), curTime.second(), curTime.msec());
	DrawGlass();
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	gDrawBoard->Clear(1.0, 1.0, 1.0, 0.0, G_TRUE);
	DrawClock();
	gDrawBoard->Flush();
}
//------------------------------------------------------------

//----- resizeGL ---------------------------------------------
void QGLWidgetTest::resizeGL(int width, int height) {

	GUInt32 x, y, w, h;

	gDrawBoard->Viewport(x, y, w, h);
	gDrawBoard->SetViewport(x, y, width, height);
}
//------------------------------------------------------------

// timer event
void QGLWidgetTest::timerEvent(QTimerEvent *e) {

	if (!e)
		return;

	updateGL();
}
