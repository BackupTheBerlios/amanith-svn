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
#include "amanith/2d/gpath2d.h"
#include "amanith/2d/gbeziercurve2d.h"
#include "amanith/2d/gbsplinecurve2d.h"
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
#endif

// constructor
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(QGLFormat(QGL::StencilBuffer), parent) {

	gKernel = new GKernel();
	gImage = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	gPath = (GPath2D *)gKernel->CreateNew(G_PATH2D_CLASSID);

	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";

	// initialize random system
	GMath::SeedRandom();
	// start with color test suite
	gTestSuite = 0;
	gTestIndex = 0;
	// start with not using random matrix
	gRandAngle = 0;
	gRandScale = 1;
	gRenderingQuality = G_HIGH_RENDERING_QUALITY;
	gUseShaders = G_TRUE;
	// set an 800x600 window
	this->setGeometry(50, 50, 800, 600);
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

	gDrawBoard = new GOpenGLBoard(0, 0, geometry().width(), geometry().height());
	gDrawBoard->SetRenderingQuality(gRenderingQuality);

	GString s;
	GError err;
	GDynArray<GKeyValue> colKeys;

	// color gradients
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.4, (GReal)0.0, (GReal)0.5, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.25, GVector4((GReal)0.9, (GReal)0.5, (GReal)0.1, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.50, GVector4((GReal)0.8, (GReal)0.8, (GReal)0.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.75, GVector4((GReal)0.0, (GReal)0.3, (GReal)0.5, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.4, (GReal)0.0, (GReal)0.5, (GReal)1.0)));
	//colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0)));
	gLinGrad1 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.30, GVector4((GReal)0.540, (GReal)0.138, (GReal)0.757, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.60, GVector4((GReal)1.000, (GReal)0.500, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.70, GVector4((GReal)0.980, (GReal)0.950, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	//colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.171, (GReal)0.200, (GReal)0.400, (GReal)1.0)));

	gRadGrad1 = gDrawBoard->CreateRadialGradient(GPoint2(90, 58), GPoint2(150, 118), 110, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad3 = gDrawBoard->CreateRadialGradient(GPoint2(-90, -70), GPoint2(-130, -130), 100, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gConGrad1 = gDrawBoard->CreateConicalGradient(GPoint2(120, 88), GPoint2(180, 88), colKeys, G_HERMITE_COLOR_INTERPOLATION);
	gConGrad3 = gDrawBoard->CreateConicalGradient(GPoint2(0, 0), GPoint2(20, 0), colKeys, G_HERMITE_COLOR_INTERPOLATION);

	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.4, (GReal)0.0, (GReal)0.5, (GReal)1.00)));
	colKeys.push_back(GKeyValue((GReal)0.25, GVector4((GReal)0.9, (GReal)0.5, (GReal)0.1, (GReal)0.25)));
	colKeys.push_back(GKeyValue((GReal)0.50, GVector4((GReal)0.8, (GReal)0.8, (GReal)0.0, (GReal)0.50)));
	colKeys.push_back(GKeyValue((GReal)0.75, GVector4((GReal)0.0, (GReal)0.3, (GReal)0.5, (GReal)0.75)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.4, (GReal)0.0, (GReal)0.5, (GReal)1.00)));
	gLinGrad2 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.30, GVector4((GReal)0.540, (GReal)0.138, (GReal)0.757, (GReal)0.7)));
	colKeys.push_back(GKeyValue((GReal)0.60, GVector4((GReal)1.000, (GReal)0.500, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.70, GVector4((GReal)0.980, (GReal)0.950, (GReal)0.000, (GReal)0.5)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	gRadGrad2 = gDrawBoard->CreateRadialGradient(GPoint2(90, 58), GPoint2(150, 118), 110, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad4 = gDrawBoard->CreateRadialGradient(GPoint2(-90, -70), GPoint2(-130, -130), 100, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gConGrad2 = gDrawBoard->CreateConicalGradient(GPoint2(120, 88), GPoint2(180, 88), colKeys, G_HERMITE_COLOR_INTERPOLATION);
	gConGrad4 = gDrawBoard->CreateConicalGradient(GPoint2(0, 0), GPoint2(20, 0), colKeys, G_HERMITE_COLOR_INTERPOLATION);


	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)0.0)));
	//colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0)));
	gLinGrad3 = gDrawBoard->CreateLinearGradient(GPoint2(560, 20), GPoint2(760, 20), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// conical gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.30, GVector4((GReal)0.540, (GReal)0.138, (GReal)0.757, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.60, GVector4((GReal)1.000, (GReal)0.500, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.70, GVector4((GReal)0.980, (GReal)0.950, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	
	// background
	s = gDataPath + "background.png";
	err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err == G_NO_ERROR) {
		gBackGround = gDrawBoard->CreatePattern(gImage, G_LOW_IMAGE_QUALITY, G_REPEAT_TILE);
		gBackGround->SetLogicalWindow(GPoint2(0, 0), GPoint2(16, 16));
	}
	else
		gBackGround = NULL;

	// pattern
	s = gDataPath + "spiral.png";
	err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err == G_NO_ERROR) {
		gPattern = gDrawBoard->CreatePattern(gImage, G_HIGH_IMAGE_QUALITY, G_REPEAT_TILE);
		gPattern->SetLogicalWindow(GPoint2(-64, -64), GPoint2(64, 64));
	}
	else
		gPattern = NULL;


	// dashes
	gDrawBoard->SetStrokeDashPhase(0);
	GDynArray<GReal> pat;
	pat.push_back(10);
	pat.push_back(35);
	pat.push_back(30);
	pat.push_back(35);
	gDrawBoard->SetStrokeDashPattern(pat);
}
//------------------------------------------------------------

void QGLWidgetTest::TestDebug() {

	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetFillColor(GVector4((GReal)1.0, (GReal)0.0, (GReal)0.0, (GReal)1.0));
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokeColor(GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)0.5));
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokeWidth(20);
	gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
	//gDrawBoard->SetStrokeJoinStyle(G_BEVEL_JOIN);
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);

	gDrawBoard->SetFillEnabled(G_FALSE);
	gDrawBoard->SetStrokeEnabled(G_TRUE);

	//gDrawBoard->DrawRectangle(GPoint2(200, 200), GPoint2(400, 400));
	//gDrawBoard->DrawRoundRectangle(GPoint2(200, 200), GPoint2(600, 400), 500, 3000);

	gDrawBoard->DrawBezier(GPoint2(100, 100), GPoint2(500, 500), GPoint2(100, 400), GPoint2(400, 100));

	long double t0, t1, dt;

	t0 = GetTickCount();

	for (GUInt32 i = 0; i < 100000; i++)
		gDrawBoard->DrawBezier(GPoint2(100, 100), GPoint2(500, 500), GPoint2(100, 400), GPoint2(400, 100));

	t1 = GetTickCount();
	dt = t1 - t0;
	if (dt < 0)
		dt = 0;

	GString s;
	s = StrUtils::ToString((double)dt);
	MessageBoxA(0, StrUtils::ToAscii(s), "Elapsed time in ms", MB_OK);

	//gDrawBoard->DrawBezier(GPoint2(200, 200), GPoint2(400, 600), GPoint2(600, 200));

	/*GDynArray<GPoint2> pts;

	GBezierCurve2D bezCurve;
	GBSplineCurve2D bsplineCurve;

	// bezier segment
	pts.push_back(GPoint2(20, 80));
	pts.push_back(GPoint2(50, 130));
	pts.push_back(GPoint2(80, 110));
	bezCurve.SetPoints(pts);
	bezCurve.SetDomain(0, (GReal)0.2);
	gPath->AppendCurve(bezCurve);
	// another bezier segment
	pts.clear();
	pts.push_back(GPoint2(80, 110));
	pts.push_back(GPoint2(110, 150));
	pts.push_back(GPoint2(150, 100));
	pts.push_back(GPoint2(130, 50));
	bezCurve.SetPoints(pts);
	bezCurve.SetDomain((GReal)0.2, (GReal)0.5);
	gPath->AppendCurve(bezCurve);
	// b-spline curve segment
	pts.clear();
	pts.push_back(GPoint2(130, 50));
	pts.push_back(GPoint2(50, 30));
	pts.push_back(GPoint2(100, 80));
	pts.push_back(GPoint2(80, 30));
	bsplineCurve.SetPoints(pts, 3, (GReal)0.5, 1);
	gPath->AppendCurve(bsplineCurve);
	// close the path
	gPath->ClosePath();

	//gDrawBoard->DrawPath(*gPath);

	/*pts.push_back(GPoint2(100, 100));
	pts.push_back(GPoint2(300, 300));
	pts.push_back(GPoint2(300, 300));
	pts.push_back(GPoint2(100, 500));*/

	//gDrawBoard->DrawPolygon(pts, G_FALSE);
	//gDrawBoard->DrawEllipseArc(GPoint2(400, 200), GPoint2(500, 300), 250, 150, 0.75, G_TRUE, G_FALSE);

	//gDrawBoard->DrawCircle(GPoint2(300, 300), 200);
	//gDrawBoard->SetStrokeStyle(G_DASHED_STROKE);
	//gDrawBoard->DrawEllipse(GPoint2(300, 300), 200, 120);
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	gDrawBoard->Clear(1.0, 1.0, 1.0, G_TRUE);

	if (gDrawBackGround) {

		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillEnabled(G_TRUE);
		gDrawBoard->SetFillPattern(gBackGround);
		gDrawBoard->SetFillColor(GVector4(0, 0, 0, 1));
		gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->DrawRectangle(GPoint2(0, 0), GPoint2(800, 600));
	}

	switch (gTestSuite) {

		case 0:
			TestColor(gTestIndex);
			break;
		case 1:
			TestLinearGradient(gTestIndex, gRandAngle, gRandScale);
			break;
		case 2:
			TestRadialGradientIn(gTestIndex, gRandAngle, gRandScale);
			break;
		case 3:
			TestRadialGradientOut(gTestIndex, gRandAngle, gRandScale);
			break;
		case 4:
			TestConicalGradientIn(gTestIndex, gRandAngle, gRandScale);
			break;
		case 5:
			TestConicalGradientOut(gTestIndex, gRandAngle, gRandScale);
			break;
		case 6:
			TestPattern(gTestIndex, gRandAngle, gRandScale);
			break;
		case 7:
			TestStroke(gTestIndex);
			break;
		case 8:
			TestMasks(gTestIndex);
			break;
		default:
			TestColor(gTestIndex);
	}

	//TestDebug();

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

// keyboard event handler
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	QString s;

	switch(e->key()) {
		case Qt::Key_F1:
			s = "1..9: Toggle draw test\n";
			s += "PageUp/PageDown: Switch transparency modes\n";
			s += "B: Toggle background\n";
			s += "R: Switch rendering quality (low/normal/high)\n";
			s += "S: Enable/Disable shaders (for gradients) if supported\n";
			s += "Space: Change matrix (valid for gradient and pattern tests)\n";
			QMessageBox::information(this, "Command keys", s);
			break;

		case Qt::Key_1:
			gTestSuite = 0;
			updateGL();
			break;
		case Qt::Key_2:
			gTestSuite = 1;
			updateGL();
			break;
		case Qt::Key_3:
			gTestSuite = 2;
			updateGL();
			break;
		case Qt::Key_4:
			gTestSuite = 3;
			updateGL();
			break;
		case Qt::Key_5:
			gTestSuite = 4;
			updateGL();
			break;
		case Qt::Key_6:
			gTestSuite = 5;
			updateGL();
			break;
		case Qt::Key_7:
			gTestSuite = 6;
			updateGL();
			break;
		case Qt::Key_8:
			gTestSuite = 7;
			updateGL();
			break;
		case Qt::Key_9:
			gTestSuite = 8;
			updateGL();
			break;
		case Qt::Key_B:
			if (gDrawBackGround)
				gDrawBackGround = G_FALSE;
			else
				gDrawBackGround = G_TRUE;
			updateGL();
			break;
		case Qt::Key_R:
			if (gRenderingQuality == G_LOW_RENDERING_QUALITY)
				gRenderingQuality = G_NORMAL_RENDERING_QUALITY;
			else
			if (gRenderingQuality == G_NORMAL_RENDERING_QUALITY)
				gRenderingQuality = G_HIGH_RENDERING_QUALITY;
			else
				gRenderingQuality = G_LOW_RENDERING_QUALITY;
			gDrawBoard->SetRenderingQuality(gRenderingQuality);
			updateGL();
			break;

		case Qt::Key_PageUp:
			gTestIndex++;
			updateGL();
			break;

		case Qt::Key_PageDown:
			if (gTestIndex > 0) {
				gTestIndex--;
				updateGL();
			}
			break;

		case Qt::Key_S:
			if (gUseShaders) {
				gUseShaders = G_FALSE;
				gDrawBoard->DisableShaders(G_TRUE);
			}
			else {
				gUseShaders = G_TRUE;
				gDrawBoard->DisableShaders(G_FALSE);
			}
			
			updateGL();
			break;

		case Qt::Key_Space:
			gRandAngle = GMath::RangeRandom((GReal)0, (GReal)G_2PI);
			if (gTestSuite == 6)
				gRandScale = GMath::RangeRandom((GReal)0.1, (GReal)1.5);
			else
				gRandScale = GMath::RangeRandom((GReal)0.33, (GReal)3.0);
			updateGL();
			break;
	}
}

//------------------------------------------------------------
