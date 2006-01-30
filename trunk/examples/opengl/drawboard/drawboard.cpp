/****************************************************************************
**
** Copyright (C) 2004-2006 Mazatech Inc. All rights reserved.
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
/*#ifdef USE_QT4
QGLWidgetTest::QGLWidgetTest(const QGLFormat& Format, QWidget *parent) : QGLWidget(Format, parent) {
#else
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(parent) {
#endif
*/

QGLWidgetTest::QGLWidgetTest(const QGLFormat& Format, QWidget *parent) : QGLWidget(Format, parent) {
#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	gKernel = new GKernel();
	gImage = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);

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
	gRandScaleX = 1;
	gRandScaleY = 1;
	gRenderingQuality = G_HIGH_RENDERING_QUALITY;
	gUseShaders = G_TRUE;
	gDrawBackGround = G_TRUE;
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
	gLinGrad1 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.30, GVector4((GReal)0.540, (GReal)0.138, (GReal)0.757, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.60, GVector4((GReal)1.000, (GReal)0.500, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.70, GVector4((GReal)0.980, (GReal)0.950, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));

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
	gLinGrad3 = gDrawBoard->CreateLinearGradient(GPoint2(560, 20), GPoint2(760, 20), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

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

	// gradients for logo
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)1.0, (GReal)1.0, (GReal)0.44, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)1.0, (GReal)1.0, (GReal)1.00, (GReal)1.0)));
	gLinGradLogo1 = gDrawBoard->CreateLinearGradient(GPoint2(306, 280), GPoint2(460, 102), colKeys, G_LINEAR_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)1.0, (GReal)0.215, (GReal)0.172, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)1.0, (GReal)0.500, (GReal)0.000, (GReal)1.0)));
	gLinGradLogo2 = gDrawBoard->CreateLinearGradient(GPoint2(276, 438), GPoint2(580, 206), colKeys, G_LINEAR_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)0.0)));
	gLinGradLogo3 = gDrawBoard->CreateLinearGradient(GPoint2(300, 460), GPoint2(417, 330), colKeys, G_LINEAR_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// create a cache bank
	gCacheBank = gDrawBoard->CreateCacheBank();

	// now lets see if some bitmap file formats are present for load/save
	GDynArray<GImpExpFeature> features;
	err = gKernel->ImpExpFeatures(G_PIXELMAP_CLASSID, features);
	if (err == G_NO_ERROR) {
		// look for PNG support
		for (GUInt32 i = 0; i < (GUInt32)features.size(); i++) {
			if (features[i].FormatSupported("png", G_FALSE, G_TRUE))
				gScreenShotFileName = "./shot.png";
		}
		// look for JPEG support, if PNG format is not present
		if (gScreenShotFileName.length() <= 0) {
			for (GUInt32 i = 0; i < (GUInt32)features.size(); i++) {
				if (features[i].FormatSupported("jpeg", G_FALSE, G_TRUE))
					gScreenShotFileName = "./shot.jpg";
			}
		}
	}
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	gDrawBoard->Clear(1.0, 1.0, 1.0, 1.0, G_TRUE);
	gDrawBoard->SetModelViewMatrix(G_MATRIX_IDENTITY33);

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
			TestLinearGradient(gTestIndex, gRandAngle, gRandScaleX);
			break;
		case 2:
			TestRadialGradientIn(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 3:
			TestRadialGradientOut(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 4:
			TestConicalGradientIn(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 5:
			TestConicalGradientOut(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 6:
			TestPattern(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 7:
			TestStroke(gTestIndex);
			break;
		case 8:
			TestMasks(gTestIndex);
			break;
		case 9:
			TestGeometries(gTestIndex);
			break;
		case 10:
			TestCache(gTestIndex);
			break;
		default:
			TestColor(gTestIndex);
	}

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
			s = "F2: contextual example description\n";
			s += "0..9, C: Toggle draw test\n";
			s += "PageUp/PageDown: Switch draw sheet\n";
			s += "B: Toggle background\n";
			s += "R: Switch rendering quality (low/normal/high)\n";
			s += "S: Enable/Disable shaders (for gradients) if supported\n";
			s += "T: Take a screenshot\n";
			s += "Space: Change matrix (valid for gradient and pattern tests)\n";
			QMessageBox::information(this, "Command keys", s);
			break;

		case Qt::Key_F2:
			switch (gTestSuite) {
				case 0:
					s = "This board shows simple color filling with opaque and transparent colors.\n";
					s += "In the leftmost column colors are 100% opaque, in the middle column colors are ";
					s += "66% opaque\n and in the rightmost column colors are 33% opaque.\n";
					s += "Compositing is done drawing smaller rectangles over larger ones.";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 1:
					s = "This board shows simple linear gradient filling.\n\n";
					s += "In the rows different color interpolation schema are shown:\n";
					s += "Topmost row: CONSTANT interpolation.\n";
					s += "Middle row: LINEAR interpolation.\n";
					s += "Lower row: HERMITE interpolation.\n\n";
					s += "In the columns different spread methods are shown:\n";
					s += "Leftmost column: PAD spread method.\n";
					s += "Middle column: REPEAT spread method.\n";
					s += "Rightmost column: REFLECT spread method.\n\n";
					s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
					s += "100% opaque fill and 100% opaque color keys.\n";
					s += "50% opaque fill and 100% opaque color keys.\n";
					s += "100% opaque fill and some non 100% opaque color keys.\n";
					s += "50% opaque fill and some non 100% opaque color keys.";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 2:
					s = "This board shows simple radial gradient filling, with focus point inside filled shapes.\n\n";
					s += "In the rows different color interpolation schema are shown:\n";
					s += "Topmost row: CONSTANT interpolation.\n";
					s += "Middle row: LINEAR interpolation.\n";
					s += "Lower row: HERMITE interpolation (available only if fragment programs are supported).\n\n";
					s += "In the columns different spread methods are shown:\n";
					s += "Leftmost column: PAD spread method.\n";
					s += "Middle column: REPEAT spread method.\n";
					s += "Rightmost column: REFLECT spread method.\n\n";
					s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
					s += "100% opaque fill and 100% opaque color keys.\n";
					s += "50% opaque fill and 100% opaque color keys.\n";
					s += "100% opaque fill and some non 100% opaque color keys.\n";
					s += "50% opaque fill and some non 100% opaque color keys.";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 3:
					s = "This board shows simple radial gradient filling, with focus point outside filled shapes.\n\n";
					s += "In the rows different color interpolation schema are shown:\n";
					s += "Topmost row: CONSTANT interpolation.\n";
					s += "Middle row: LINEAR interpolation.\n";
					s += "Lower row: HERMITE interpolation (available only if fragment programs are supported).\n\n";
					s += "In the columns different spread methods are shown:\n";
					s += "Leftmost column: PAD spread method.\n";
					s += "Middle column: REPEAT spread method.\n";
					s += "Rightmost column: REFLECT spread method.\n\n";
					s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
					s += "100% opaque fill and 100% opaque color keys.\n";
					s += "50% opaque fill and 100% opaque color keys.\n";
					s += "100% opaque fill and some non 100% opaque color keys.\n";
					s += "50% opaque fill and some non 100% opaque color keys.";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 4:
					s = "This board shows simple conical gradient filling, with center point inside filled shapes.\n\n";
					s += "In the rows different color interpolation schema are shown:\n";
					s += "Topmost row: CONSTANT interpolation.\n";
					s += "Middle row: LINEAR interpolation.\n";
					s += "Lower row: HERMITE interpolation.\n\n";
					s += "In the columns different spread methods are shown:\n";
					s += "Leftmost column: PAD spread method.\n";
					s += "Middle column: REPEAT spread method.\n";
					s += "Rightmost column: REFLECT spread method.\n\n";
					s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
					s += "100% opaque fill and 100% opaque color keys.\n";
					s += "50% opaque fill and 100% opaque color keys.\n";
					s += "100% opaque fill and some non 100% opaque color keys.\n";
					s += "50% opaque fill and some non 100% opaque color keys.";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 5:
					s = "This board shows simple conical gradient filling, with center point outside filled shapes.\n\n";
					s += "In the rows different color interpolation schema are shown:\n";
					s += "Topmost row: CONSTANT interpolation.\n";
					s += "Middle row: LINEAR interpolation.\n";
					s += "Lower row: HERMITE interpolation.\n\n";
					s += "In the columns different spread methods are shown:\n";
					s += "Leftmost column: PAD spread method.\n";
					s += "Middle column: REPEAT spread method.\n";
					s += "Rightmost column: REFLECT spread method.\n\n";
					s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
					s += "100% opaque fill and 100% opaque color keys.\n";
					s += "50% opaque fill and 100% opaque color keys.\n";
					s += "100% opaque fill and some non 100% opaque color keys.\n";
					s += "50% opaque fill and some non 100% opaque color keys.";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 6:
					s = "This board shows simple pattern filling.\n\n";
					s += "In the rows different opacity percentage are shown:\n";
					s += "Topmost row: 33% opaque filling.\n";
					s += "Middle row: 66% opaque filling.\n";
					s += "Lower row: 100% opaque filling.\n\n";
					s += "In the columns different pattern tiling modes are shown:\n";
					s += "Leftmost column: PAD tiling mode.\n";
					s += "Middle column: REPEAT tiling mode.\n";
					s += "Rightmost column: REFLECT tiling mode.\n\n";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 7:
					s = "This board shows all supported stroking features.\n\n";
					s += "Supported stroke styles are SOLID and DASHED (with also initial phase support).\n";
					s += "Supported join types are BEVEL, MITER and ROUND.\n";
					s += "Supported caps types (they could be used independently for start and end cap) are BUTT, SQUARE and ROUND.\n";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 8:
					s = "This board shows clip masks and group opacity (you can switch between them using PageUp/PageDown keys).\n\n";
					s += "Clip masks sheet description:\n";
					s += "Leftmost column: unclipped shapes.\n";
					s += "Middle column: clip masks (the darker color is the intersection of the masks).\n";
					s += "Rightmost column: clipped shapes, using masks in \"and\" (intersection).\n\n";
					s += "Group opacity sheet description:\n";
					s += "Leftmost column: shapes are drawn without group opacity.\n";
					s += "Middle column: shapes are drawn using (in different modes) group opacity.\n";
					s += "Rightmost column: shapes are drawn with group opacity (like in the middle column) over a background.\n";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 9:
					s = "This board shows some supported geometric primitives (you can switch between them using PageUp/PageDown keys).\n\n";
					s += "Stroke primitives sheet description:\n";
					s += "Lower row: a line, a quadratic Bezier curve and a cubic Bezier curve.\n";
					s += "Middle row: 2 elliptical arcs (build using different constructors) and a polyline.\n";
					s += "Topmost row: a round rectangle, a circle and an ellipse.\n\n";
					s += "Amanith logo sheet description:\n";
					s += "Here's Amanith mushroom, it's constructed directly with SVG paths.";
					QMessageBox::information(this, "Current board description", s);
					break;
				case 10:
					s = "This board shows Amanith caching system.\n\n";
					s += "Lower row: first cache bank is filled (G_CACHE_MODE), then cached geometry is drawn (G_COLOR_MODE).\n";
					s += "Middle row: both cache and color are written at the same time (G_COLOR_AND_CACHE_MODE).\n";
					s += "Topmost row: cached geometry is drawn with a different paint style.\n\n";
					QMessageBox::information(this, "Current board description", s);
					break;
			}
			break;
		case Qt::Key_1:
			gTestSuite = 0;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_2:
			gTestSuite = 1;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_3:
			gTestSuite = 2;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_4:
			gTestSuite = 3;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_5:
			gTestSuite = 4;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_6:
			gTestSuite = 5;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_7:
			gTestSuite = 6;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_8:
			gTestSuite = 7;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_9:
			gTestSuite = 8;
			gTestIndex = 0;
			updateGL();
			break;
		case Qt::Key_0:
			gTestSuite = 9;
			gTestIndex = 0;
			updateGL();
			break;

		case Qt::Key_C:
			gTestSuite = 10;
			gTestIndex = 0;
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
				gDrawBoard->SetShadersEnabled(G_FALSE);
			}
			else {
				gUseShaders = G_TRUE;
				gDrawBoard->SetShadersEnabled(G_TRUE);
			}
			updateGL();
			break;

		case Qt::Key_T:
			if (gScreenShotFileName.length() > 0) {
				GPixelMap p(gKernel);
				gDrawBoard->ScreenShot(p);
				p.Save(StrUtils::ToAscii(gScreenShotFileName));
			}
			break;

		case Qt::Key_Space:
			gRandAngle = GMath::RangeRandom((GReal)0, (GReal)G_2PI);
			if (gTestSuite == 6) {
				gRandScaleX = GMath::RangeRandom((GReal)0.1, (GReal)1.5);
				gRandScaleY = GMath::RangeRandom((GReal)0.1, (GReal)1.5);
			}
			else {
				gRandScaleX = GMath::RangeRandom((GReal)0.33, (GReal)3.0);
				gRandScaleY = GMath::RangeRandom((GReal)0.33, (GReal)3.0);
			}
			updateGL();
			break;
	}
}

//------------------------------------------------------------
