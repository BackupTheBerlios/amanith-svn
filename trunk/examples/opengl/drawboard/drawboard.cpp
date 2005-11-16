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
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
 #include <QTimerEvent>
 #include <QKeyEvent>
#endif

static int timer_interval = 0;   // timer interval (milliseconds)

// constructor
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(QGLFormat(StencilBuffer), parent, 0) {

 /*if (!format().stencil())
  qWarning("Could not get stencil buffer; results will be suboptimal");

 if (!format().alpha())
  qWarning("Could not get alpha channel; results will be suboptimal");*/

 gKernel = new GKernel();
 gImage = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);

 // build path for data (textures)
 GString dataPath = SysUtils::AmanithPath();
 if (dataPath.length() > 0)
  dataPath += "data/";

 GString s = dataPath + "stars.png";
 GError err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err != G_NO_ERROR)
		abort();

	this->setGeometry(50, 50, 1006, 768);
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gKernel)
		delete gKernel;
	if (gDrawBoard)
		delete gDrawBoard;
}

//------------------------------------------------------------

void QGLWidgetTest::timerEvent(QTimerEvent *e) {

	if (!e)
		return;
	updateGL();
}

//static GLfloat feedBuffer[2000];

//----- initializeGL -----------------------------------------
void QGLWidgetTest::initializeGL() {

	gDrawBoard = new GOpenGLBoard(0, 0, geometry().width(), geometry().height());
	gDrawBoard->SetRenderingQuality(G_HIGH_RENDERING_QUALITY);
	gDrawBoard->SetImageQuality(G_HIGH_IMAGE_QUALITY);
/*
	gPattern = gDrawBoard->CreatePattern(gImage, G_REPEAT_TILE);
	gPattern->SetLogicalWindow(GPoint2(100, 100), GPoint2(200, 200));
*/


	
	//startTimer(timer_interval);
}
//------------------------------------------------------------

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {
	
	GDynArray<GKeyValue> colKeys;

	// caps
	gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
	// joins
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetFillEnabled(G_TRUE);
	// dashes
	gDrawBoard->SetStrokeDashPhase(0);
	GDynArray<GReal> pat;
	pat.push_back(1);
	pat.push_back(25);
	pat.push_back(30);
	pat.push_back(25);
	gDrawBoard->SetStrokeDashPattern(pat);
	
	gDrawBoard->Clear(1.0, 1.0, 1.0, G_TRUE);

	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	//gPattern->SetMatrix(G_MATRIX_IDENTITY33);
	
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(8);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gDrawBoard->SetFillColor(GVector4(0.9, 0.9, 0.9, 1.0));
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetFillGradient(gRadGrad);
	gDrawBoard->DrawRectangle(GPoint2(15, 399), GPoint2(490, 753));
	gDrawBoard->DrawRectangle(GPoint2(15, 15), GPoint2(490, 369));
	gDrawBoard->DrawRectangle(GPoint2(520, 399), GPoint2(989, 753));
	gDrawBoard->DrawRectangle(GPoint2(520, 15), GPoint2(989, 369));
	
	
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gDrawBoard->SetFillColor(GVector4(0.7, 0.0, 0.0, 1.0));
	gDrawBoard->DrawRectangle(GPoint2(30, 591), GPoint2(130, 738));
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 0.5));
	gDrawBoard->SetFillColor(GVector4(0.7, 0.0, 0.0, 0.5));
	gDrawBoard->DrawRectangle(GPoint2(30, 414), GPoint2(130, 576));
	
	
	colKeys.clear();
	colKeys.push_back(GKeyValue(0.00, GVector4(0.0, 0.0, 0.7, 1.0)));
	colKeys.push_back(GKeyValue(1.00, GVector4(0.7, 0.0, 0.0, 1.0)));
	gLinGrad = gDrawBoard->CreateLinearGradient(GPoint2(145, 591), GPoint2(245, 738), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gDrawBoard->SetFillGradient(gLinGrad);
	gDrawBoard->DrawRectangle(GPoint2(145, 591), GPoint2(245, 738));
	gLinGrad = gDrawBoard->CreateLinearGradient(GPoint2(145, 414), GPoint2(245, 576), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 0.5));
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.5));
	gDrawBoard->SetFillGradient(gLinGrad);
	gDrawBoard->DrawRectangle(GPoint2(145, 414), GPoint2(245, 576));
	
	
	colKeys.clear();
	colKeys.push_back(GKeyValue(0.00, GVector4(0.4, 0.0, 0.3, 1.0)));
	colKeys.push_back(GKeyValue(0.50, GVector4(0.2, 0.5, 0.4, 1.0)));
	colKeys.push_back(GKeyValue(1.00, GVector4(0.7, 0.8, 0.0, 1.0)));
	gRadGrad = gDrawBoard->CreateRadialGradient(GPoint2(310, 664), GPoint2(300, 630), 50, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_REFLECT_COLOR_RAMP_SPREAD);
	gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gDrawBoard->SetFillGradient(gRadGrad);
	gDrawBoard->DrawRectangle(GPoint2(260, 591), GPoint2(360, 738));
	gRadGrad = gDrawBoard->CreateRadialGradient(GPoint2(310, 495), GPoint2(300, 465), 50, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_REFLECT_COLOR_RAMP_SPREAD);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 0.5));
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.5));
	gDrawBoard->SetFillGradient(gRadGrad);
	gDrawBoard->DrawRectangle(GPoint2(260, 414), GPoint2(360, 576));
	
	
	gPattern = gDrawBoard->CreatePattern(gImage, G_REPEAT_TILE);
	gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gPattern->SetLogicalWindow(GPoint2(100, 100), GPoint2(200, 200));
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(375, 591), GPoint2(475, 738));
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 0.5));
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.5));
	gDrawBoard->DrawRectangle(GPoint2(375, 414), GPoint2(475, 576));

	
	gDrawBoard->SetFillEnabled(G_FALSE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.0));
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->DrawRectangle(GPoint2(30, 207), GPoint2(130, 354));
	gDrawBoard->SetStrokeJoinStyle(G_BEVEL_JOIN);
	gDrawBoard->DrawRectangle(GPoint2(145, 207), GPoint2(245, 354));
	gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
	gDrawBoard->DrawRectangle(GPoint2(260, 207), GPoint2(360, 354));
	
	gDrawBoard->DrawRectangle(GPoint2(375, 207), GPoint2(475, 354));
	
	
	gDrawBoard->SetStrokeStyle(G_DASHED_STROKE);
	
	
	gDrawBoard->SetStrokeJoinStyle(G_MITER_JOIN);
	gDrawBoard->SetStrokeStartCapStyle(G_SQUARE_CAP);
	gDrawBoard->SetStrokeEndCapStyle(G_SQUARE_CAP);
	gDrawBoard->DrawRectangle(GPoint2(30, 30), GPoint2(130, 192));
	/*gDrawBoard->SetStrokeJoinStyle(G_BEVEL_JOIN);
	gDrawBoard->SetStrokeStartCapStyle(G_BUTT_CAP);
	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
	gDrawBoard->DrawRectangle(GPoint2(145, 30), GPoint2(245, 192));
	gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
	//gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
	//gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
	gDrawBoard->DrawRectangle(GPoint2(260, 30), GPoint2(360, 192));
	
	gDrawBoard->DrawRectangle(GPoint2(375, 30), GPoint2(475, 192));
	
*/

/*
	// group 1
	gDrawBoard->SetStrokeColor(GVector4(0.2509, 0.6411, 0.1882, 0.6));
	//gDrawBoard->SetStrokePaintType(G_PATTERN_PAINT_TYPE);
	gDrawBoard->SetStrokeWidth(50);

	gDrawBoard->SetGroupOpacity(0.5);
	gDrawBoard->GroupBegin(GAABox2(GPoint2(200, 180), GPoint2(700, 510)));
		gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->DrawLine(GPoint2(300, 234), GPoint2(601, 434));
		gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
		gDrawBoard->DrawLine(GPoint2(300, 434), GPoint2(601, 234));
	gDrawBoard->GroupEnd();

	//gDrawBoard->DumpBuffers("buf_Z.raw", "buf_S.raw");

	// group 2
	gDrawBoard->SetStrokeColor(GVector4(0.9411, 0.2509, 0.1882, 1.0));
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);

	gDrawBoard->SetStrokeWidth(30);

	gDrawBoard->SetGroupOpacity(0.5);
	gDrawBoard->GroupBegin(GAABox2(GPoint2(200, 180), GPoint2(700, 510)));
		gDrawBoard->DrawLine(GPoint2(360, 300), GPoint2(530, 380));
		gDrawBoard->DrawLine(GPoint2(250, 240), GPoint2(500, 300));
		gDrawBoard->DrawLine(GPoint2(450, 200), GPoint2(450, 380));
	gDrawBoard->GroupEnd();
*/
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
			/*
			s = "";
			s += "";
			s += "";
			s += "";
			s += "";
			QMessageBox::information(this, "Command keys", s);
			*/
			break;
		case Qt::Key_PageUp:
			//resize(1186, 911);
			break;
		case Qt::Key_PageDown:
			break;
		case Qt::Key_A:
			break;
		case Qt::Key_Z:
			break;
		case Qt::Key_Up:
			break;
		case Qt::Key_Down:
			break;
		case Qt::Key_Right:
			break;
		case Qt::Key_Left:
			break;
		case Qt::Key_M:
			break;
		case Qt::Key_N:
			break;
		case Qt::Key_Space:
			break;
	}
}

//------------------------------------------------------------
