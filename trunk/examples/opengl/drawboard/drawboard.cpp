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

//------------------------------------------------------------

void QGLWidgetTest::timerEvent(QTimerEvent *e) {

	if (!e)
		return;
	updateGL();
}

//static GLfloat feedBuffer[2000];

//----- initializeGL -----------------------------------------
void QGLWidgetTest::initializeGL() {

	gImage = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);

	// build path for data (textures)
	GString dataPath = SysUtils::AmanithPath();
	if (dataPath.length() > 0)
		dataPath += "data/";

	GString s = dataPath + "spiral.png";
	GError err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err != G_NO_ERROR)
		abort();
		
	
	gDrawBoard = new GOpenGLBoard(0, 0, geometry().width(), geometry().height());
	gDrawBoard->SetRenderingQuality(G_HIGH_RENDERING_QUALITY);
	gDrawBoard->SetImageQuality(G_HIGH_IMAGE_QUALITY);

	gPattern = gDrawBoard->CreatePattern(gImage, G_REPEAT_TILE);
	gPattern->SetLogicalWindow(GPoint2(-64, -64), GPoint2(64, 64));

	GDynArray<GKeyValue> colKeys;
	
	// linear gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue(0.00, GVector4(0.4, 0.0, 0.5, 1.0)));
	colKeys.push_back(GKeyValue(0.25, GVector4(0.9, 0.5, 0.1, 1.0)));
	colKeys.push_back(GKeyValue(0.50, GVector4(0.8, 0.8, 0.0, 1.0)));
	colKeys.push_back(GKeyValue(0.75, GVector4(0.0, 0.3, 0.5, 1.0)));
	colKeys.push_back(GKeyValue(1.00, GVector4(0.4, 0.0, 0.5, 1.0)));
	gLinGrad1 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad1 = gDrawBoard->CreateRadialGradient(GPoint2(90, 58), GPoint2(150, 118), 110, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad3 = gDrawBoard->CreateRadialGradient(GPoint2(-160, -140), GPoint2(-200, -200), 100, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	colKeys.clear();
	colKeys.push_back(GKeyValue(0.00, GVector4(0.4, 0.0, 0.5, 1.00)));
	colKeys.push_back(GKeyValue(0.25, GVector4(0.9, 0.5, 0.1, 0.25)));
	colKeys.push_back(GKeyValue(0.50, GVector4(0.8, 0.8, 0.0, 0.50)));
	colKeys.push_back(GKeyValue(0.75, GVector4(0.0, 0.3, 0.5, 0.75)));
	colKeys.push_back(GKeyValue(1.00, GVector4(0.4, 0.0, 0.5, 1.00)));
	gLinGrad2 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad2 = gDrawBoard->CreateRadialGradient(GPoint2(90, 58), GPoint2(150, 118), 110, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad4 = gDrawBoard->CreateRadialGradient(GPoint2(-160, -140), GPoint2(-200, -200), 100, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	

	/*
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
	pat.push_back(5);
	pat.push_back(20);
	pat.push_back(30);
	pat.push_back(20);
	gDrawBoard->SetStrokeDashPattern(pat);
	*/
	
}
//------------------------------------------------------------

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {
	
	gDrawBoard->Clear(1.0, 1.0, 1.0, G_TRUE);

	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	
	GMatrix33 m;
	
//----- FILL LINEAR GRADIENT TEST SUITE ---------------------------------
	
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
	
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.000));
	
	// --------------------------------------------------------------
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 1.000));
	gPattern->SetTilingMode(G_PAD_TILE);
	TranslationToMatrix(m, GVector2(+140,+106));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
	gPattern->SetTilingMode(G_REPEAT_TILE);
	TranslationToMatrix(m, GVector2(+400,+106));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
	gPattern->SetTilingMode(G_REFLECT_TILE);
	TranslationToMatrix(m, GVector2(+660,+106));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));
	
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.666));
	gPattern->SetTilingMode(G_PAD_TILE);
	TranslationToMatrix(m, GVector2(+140,+300));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
	gPattern->SetTilingMode(G_REPEAT_TILE);
	TranslationToMatrix(m, GVector2(+400,+300));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
	gPattern->SetTilingMode(G_REFLECT_TILE);
	TranslationToMatrix(m, GVector2(+660,+300));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));
	
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.333));
	gPattern->SetTilingMode(G_PAD_TILE);
	TranslationToMatrix(m, GVector2(+140,+494));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
	gPattern->SetTilingMode(G_REPEAT_TILE);
	TranslationToMatrix(m, GVector2(+400,+494));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
	gPattern->SetTilingMode(G_REFLECT_TILE);
	TranslationToMatrix(m, GVector2(+660,+494));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));
	
	// --------------------------------------------------------------
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
