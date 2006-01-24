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

#include "compositing.h"
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

// timer interval (millisec)
static int timer_interval = 0;

// constructor
#ifdef USE_QT4
QGLWidgetTest::QGLWidgetTest(const QGLFormat& Format, QWidget *parent) : QGLWidget(Format, parent, 0, Qt::Dialog) {
#else
QGLWidgetTest::QGLWidgetTest(const QGLFormat& Format, QWidget *parent) : QGLWidget(Format, parent, 0, 0,
							 Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu) {
#endif
#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	gKernel = new GKernel();
	gImage = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);

	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";

	gRotAngle = 0;
	gRotationVel = (GReal)0.05;
	gStrokeOpacity = 1;
	gFillOpacity = 1;
	gZoomFactor = 2;
	gAnim = G_FALSE;
	gTranslation.Set(256, 256);
	gStrokeCompOp = G_SRC_OVER_OP;
	gFillCompOp = G_SRC_OVER_OP;

	// set an 512x512 window
	this->setGeometry(50, 50, 512, 512);
	this->setMinimumSize(512, 512);
	this->setMaximumSize(512, 512);
	this->setMouseTracking(true);
	DrawTitle();
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
	gDrawBoard->SetRenderingQuality(G_HIGH_RENDERING_QUALITY);

	GString s;
	GError err;
	GDynArray<GKeyValue> colKeys;

	// color gradients
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.95, (GReal)0.92, (GReal)0.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.1, (GReal)0.3, (GReal)0.8, (GReal)0.7)));
	gLinGrad = gDrawBoard->CreateLinearGradient(GPoint2(-60, -44), GPoint2(60, 44), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	// background
	s = gDataPath + "compground.png";
	err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err == G_NO_ERROR) {
		gBackGround = gDrawBoard->CreatePattern(gImage, G_LOW_IMAGE_QUALITY, G_REPEAT_TILE);
		gBackGround->SetLogicalWindow(GPoint2(0, 0), GPoint2(512, 512));
	}
	else
		gBackGround = NULL;


	gDrawBoard->SetStrokeWidth(10);
	gDrawBoard->SetStrokeGradient(gLinGrad);
	gDrawBoard->SetFillGradient(gLinGrad);
	gDrawBoard->SetFillPattern(gBackGround);

	startTimer(timer_interval);
}

void QGLWidgetTest::BuildMatrices() {

	GMatrix33 scl, rot, trans;

	ScaleToMatrix(scl, gZoomFactor, GPoint2(0, 0));
	RotationToMatrix(rot, gRotAngle, GPoint2(0, 0));
	TranslationToMatrix(trans, gTranslation);

	gModelView = trans * (rot * scl);
	gGradientMatrix = gModelView;
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	gDrawBoard->Clear(1.0, 1.0, 1.0, 0.0, G_TRUE);
	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	BuildMatrices();

	// draw background
	gDrawBoard->SetModelViewMatrix(G_MATRIX_IDENTITY33);
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
	gDrawBoard->SetFillOpacity(1.0);
	gDrawBoard->SetFillCompOp(G_SRC_OP);
	gDrawBoard->DrawRectangle(GPoint2(0, 0), GPoint2(512, 512));

	// draw path
	gDrawBoard->SetStrokeOpacity(gStrokeOpacity);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);

	gDrawBoard->SetFillOpacity(gFillOpacity);
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);

	gDrawBoard->SetModelViewMatrix(gModelView);
	gLinGrad->SetMatrix(gGradientMatrix);

	gDrawBoard->SetStrokeCompOp(gStrokeCompOp);
	gDrawBoard->SetFillCompOp(gFillCompOp);

	gDrawBoard->DrawRoundRectangle(GPoint2(-64, -48), GPoint2(64, 48), 16, 16);

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

	if (gAnim)
		gRotAngle += gRotationVel;
	updateGL();
}

// keyboard event handler
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	QString s;

	switch(e->key()) {

		case Qt::Key_F1:
			s = "B: Toggle rotation.\n";
			s += "N/M: Decrease/Increation rotation velocity.\n";			
			s += "F/G: Decrease/Increase global fill opacity.\n";
			s += "S/D: Decrease/Increase global stroke opacity.\n";
			s += "Mouse left button: change fill compositing operation.\n";
			s += "Mouse right button: change stroke compositing operation.\n";
			s += "Mouse wheel or A/Z: change zoom factor.\n\n";
			s += "All 24 compositing operations are fully supported on gfx board with fragment programs.\n";
			s += "If they are absent, there are 9 unsupported compositing operations:\n";
			s += "Multiply, Overlay, Darken, Lighten, Color dodge, Color burn, Hard light, Soft light, Difference";
			QMessageBox::information(this, "Command keys", s);
			break;

		case Qt::Key_B:
			if (gAnim)
				gAnim = G_FALSE;
			else
				gAnim = G_TRUE;
			updateGL();
			break;

		case Qt::Key_S:
			if (gStrokeOpacity >= (GReal)0.1)
				gStrokeOpacity -= (GReal)0.1;
			DrawTitle();
			break;
		case Qt::Key_D:
			if (gStrokeOpacity < (GReal)1.0)
				gStrokeOpacity += (GReal)0.1;
			DrawTitle();
			break;
		case Qt::Key_F:
			if (gFillOpacity >= (GReal)0.1)
				gFillOpacity -= (GReal)0.1;
			DrawTitle();
			break;
		case Qt::Key_G:
			if (gFillOpacity < (GReal)1.0)
				gFillOpacity += (GReal)0.1;
			DrawTitle();
			break;
		case Qt::Key_A:
			if (gZoomFactor > (GReal)0.1)
				gZoomFactor *= (GReal)0.95;
			break;
		case Qt::Key_Z:
			if (gZoomFactor < (GReal)5.0)
				gZoomFactor /= (GReal)0.95;
			break;
		case Qt::Key_N:
			gRotationVel /= (GReal)1.2;
			break;
		case Qt::Key_M:
			gRotationVel *= (GReal)1.2;
			break;
	}
}

//------------------------------------------------------------

// mouse press event handler
void QGLWidgetTest::mousePressEvent(QMouseEvent * e) {

	GInt32 i;

	if (e->button() == Qt::LeftButton) {
		i = gFillCompOp;
		i++;
		if (i > G_EXCLUSION_OP)
			i = G_CLEAR_OP;
		gFillCompOp = (GCompositingOperation)i;
		DrawTitle();
	}
	if (e->button() == Qt::RightButton) {
		i = gStrokeCompOp;
		i++;
		if (i > G_EXCLUSION_OP)
			i = G_CLEAR_OP;
		gStrokeCompOp = (GCompositingOperation)i;
		DrawTitle();
	}
}

// mouse move event handler
void QGLWidgetTest::mouseMoveEvent(QMouseEvent * e) {

	GPoint<GInt32, 2> p(e->x(), geometry().height() - e->y());
	gTranslation = gDrawBoard->PhysicalToLogical(p);
}

// mouse wheel event handler
void QGLWidgetTest::wheelEvent(QWheelEvent * e) {

	if (e->delta() > 0) {
		if (gZoomFactor > (GReal)0.1)
			gZoomFactor *= (GReal)0.95;
	}
	else {
		if (gZoomFactor < (GReal)5.0)
			gZoomFactor /= (GReal)0.95;
	}
}

GString QGLWidgetTest::CompOpToString(const GCompositingOperation CompOp) {

	switch(CompOp) {

		case G_CLEAR_OP:
			return("Clear");
		case G_SRC_OP:
			return("Src");
		case G_DST_OP:
			return("Dst");
		case G_SRC_OVER_OP:
			return("SrcOver");
		case G_DST_OVER_OP:
			return("DstOver");
		case G_SRC_IN_OP:
			return("SrcIn");
		case G_DST_IN_OP:
			return("DstIn");
		case G_SRC_OUT_OP:
			return("SrcOut");
		case G_DST_OUT_OP:
			return("DstOut");
		case G_SRC_ATOP_OP:
			return("SrcATop");
		case G_DST_ATOP_OP:
			return("DstATop");
		case G_XOR_OP:
			return("Xor");
		case G_PLUS_OP:
			return("Plus");
		case G_MULTIPLY_OP:
			return("Multiply");
		case G_SCREEN_OP:
			return("Screen");
		case G_OVERLAY_OP:
			return("Overlay");
		case G_DARKEN_OP:
			return("Darken");
		case G_LIGHTEN_OP:
			return("Lighten");
		case G_COLOR_DODGE_OP:
			return("ColorDodge");
		case G_COLOR_BURN_OP:
			return("ColorBurn");
		case G_HARD_LIGHT_OP:
			return("HardLight");
		case G_SOFT_LIGHT_OP:
			return("SoftLight");
		case G_DIFFERENCE_OP:
			return("Difference");
		case G_EXCLUSION_OP:
			return("Exclusion");
		default:
			return("SrcOver");
	}
}

void QGLWidgetTest::DrawTitle() {

	GString s;

	s = "FILL: " + CompOpToString(gFillCompOp) + " (" + StrUtils::ToString(gFillOpacity * 100, "%3.0f") + "%)";
	s += " - STROKE: " +CompOpToString(gStrokeCompOp) + " (" + StrUtils::ToString(gStrokeOpacity * 100, "%3.0f") + "%)";
	s += " [F1 help]";

#ifdef USE_QT4
	setWindowTitle(QString(StrUtils::ToAscii(s)));
#else
	setCaption(QString(StrUtils::ToAscii(s)));
#endif
}
