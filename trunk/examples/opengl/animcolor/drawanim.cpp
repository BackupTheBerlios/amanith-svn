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

#include "drawanim.h"
#include <amanith/geometry/gxform.h>
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
#endif

static int timer_interval = 0;			// timer interval (millisec)

// constructor
#ifdef USE_QT4
QGLWidgetTest::QGLWidgetTest(const QGLFormat& Format, QWidget *parent) : QGLWidget(Format, parent) {
#else
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(parent) {
#endif

#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	gKernel = new GKernel();

	gRedAnimH = (GHermiteProperty1D *)gKernel->CreateNew(G_HERMITEPROPERTY1D_CLASSID);
	gGreenAnimH = (GHermiteProperty1D *)gKernel->CreateNew(G_HERMITEPROPERTY1D_CLASSID);
	gBlueAnimH = (GHermiteProperty1D *)gKernel->CreateNew(G_HERMITEPROPERTY1D_CLASSID);

	gRedAnimL = (GLinearProperty1D *)gKernel->CreateNew(G_LINEARPROPERTY1D_CLASSID);
	gGreenAnimL = (GLinearProperty1D *)gKernel->CreateNew(G_LINEARPROPERTY1D_CLASSID);
	gBlueAnimL = (GLinearProperty1D *)gKernel->CreateNew(G_LINEARPROPERTY1D_CLASSID);

	gRedAnimC = (GConstantProperty1D *)gKernel->CreateNew(G_CONSTANTPROPERTY1D_CLASSID);
	gGreenAnimC = (GConstantProperty1D *)gKernel->CreateNew(G_CONSTANTPROPERTY1D_CLASSID);
	gBlueAnimC = (GConstantProperty1D *)gKernel->CreateNew(G_CONSTANTPROPERTY1D_CLASSID);

	gOORType = G_LOOP_OOR;
	SetOORType(gOORType);

	// set time
	gCurrentTime = 0;
	gTimeStep = 1;
	gDrawPalette = G_TRUE;
	gInterpolationIndex = 0;  // start with hermite interpolation
	// initialize random system
	GMath::SeedRandom();
	// build a palette
	BuildNewPalette(6);
}

//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gKernel)
		delete gKernel;
}

//------------------------------------------------------------

void QGLWidgetTest::BuildNewPalette(const GUInt32 KeysNum) {

	if (KeysNum < 2)
		return;

	GDynArray<GKeyValue> keys(KeysNum);

	keys[0] = GKeyValue(0, GMath::RangeRandom((GReal)0, (GReal)1));
	keys[1] = GKeyValue(1000, GMath::RangeRandom((GReal)0, (GReal)1));

	// red
	for (GUInt32 i = 2; i < KeysNum; ++i)
		keys[i] = GKeyValue(GMath::RangeRandom((GReal)10, (GReal)990), GMath::RangeRandom((GReal)0, (GReal)1));
	gRedAnimC->SetKeys(keys);
	gRedAnimL->SetKeys(keys);
	gRedAnimH->SetKeys(keys);

	// green
	for (GUInt32 i = 0; i < KeysNum; ++i)
		keys[i].SetValue(GMath::RangeRandom((GReal)0, (GReal)1));
	gGreenAnimC->SetKeys(keys);
	gGreenAnimL->SetKeys(keys);
	gGreenAnimH->SetKeys(keys);

	// blue
	for (GUInt32 i = 0; i < KeysNum; ++i)
		keys[i].SetValue(GMath::RangeRandom((GReal)0, (GReal)1));
	gBlueAnimC->SetKeys(keys);
	gBlueAnimL->SetKeys(keys);
	gBlueAnimH->SetKeys(keys);

}

//------------------------------------------------------------

void QGLWidgetTest::timerEvent(QTimerEvent *e) {

	if (!e)
		return;

	updateGL();
}

//----- initializeGL -----------------------------------------
void QGLWidgetTest::initializeGL() {

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);				// Set Line Antialiasing
	glDisable(GL_LIGHTING);
	setDefaultGlobalStates();
	startTimer(timer_interval);
}
//------------------------------------------------------------

void QGLWidgetTest::DrawCursor(const GTimeValue TimePos, const GBool On) {

	GTimeValue t = gRedAnimC->OORTime(TimePos);
	GReal x;
	GTimeInterval domainInterval = gRedAnimC->Domain();

	if (On)
		glColor3f(1.0f, 1.0f, 1.0f);
	else
		glColor3f(0.0f, 0.0f, 0.0f);

	x = 2.0f * (t / domainInterval.Length()) - 1.0f;
	// hermite
	if (gInterpolationIndex == 0) {
		glBegin(GL_LINES);
			glVertex2f(x, 0.71f);
			glVertex2f(x - 0.01, 0.75f);
			glVertex2f(x - 0.01, 0.75f);
			glVertex2f(x + 0.01, 0.75f);
			glVertex2f(x + 0.01, 0.75f);
			glVertex2f(x, 0.71f);
		glEnd();
	}
	else
	// linear
	if (gInterpolationIndex == 1) {
		glBegin(GL_LINES);
			glVertex2f(x, 0.41f);
			glVertex2f(x - 0.01, 0.45f);
			glVertex2f(x - 0.01, 0.45f);
			glVertex2f(x + 0.01, 0.45f);
			glVertex2f(x + 0.01, 0.45f);
			glVertex2f(x, 0.41f);
		glEnd();
	}
	// constant
	else {
		glBegin(GL_LINES);
			glVertex2f(x, 0.11f);
			glVertex2f(x - 0.01, 0.15f);
			glVertex2f(x - 0.01, 0.15f);
			glVertex2f(x + 0.01, 0.15f);
			glVertex2f(x + 0.01, 0.15f);
			glVertex2f(x, 0.11f);
		glEnd();
	}

	if (!On)
		return;

	GKeyValue currentColorR, currentColorG, currentColorB;
	GTimeInterval validInterval;

	if (gInterpolationIndex == 0) {
		gRedAnimH->Value(currentColorR, validInterval, TimePos);
		gGreenAnimH->Value(currentColorG, validInterval, TimePos);
		gBlueAnimH->Value(currentColorB, validInterval, TimePos);
	}
	else
	if (gInterpolationIndex == 1) {
		gRedAnimL->Value(currentColorR, validInterval, TimePos);
		gGreenAnimL->Value(currentColorG, validInterval, TimePos);
		gBlueAnimL->Value(currentColorB, validInterval, TimePos);
	}
	else {
		gRedAnimC->Value(currentColorR, validInterval, TimePos);
		gGreenAnimC->Value(currentColorG, validInterval, TimePos);
		gBlueAnimC->Value(currentColorB, validInterval, TimePos);
	}

	glColor3f(currentColorR.RealValue(), currentColorG.RealValue(), currentColorB.RealValue());
	glBegin(GL_POLYGON);
		glVertex2f(-1.0f, -0.5f);
		glVertex2f(1.0f, -0.5f);
		glVertex2f(1.0f, -0.7f);
		glVertex2f(-1.0f, -0.7f);
	glEnd();
}

void QGLWidgetTest::DrawPalette() {

	#define X_SAMPLES 1000.0f
	GReal t, x, xStep, tStep;
	GKeyValue r, g, b;
	GInt32 i, j;
	GTimeInterval validInterval, domainInterval;

	// extract animation domain
	domainInterval = gRedAnimH->Domain();

	// calculate time and position steps
	x = -1.0f;
	xStep = 2.0f / X_SAMPLES;
	t = domainInterval.Start();
	tStep = domainInterval.Length() / X_SAMPLES;

	// draw color bars
	for (i = 0; i <= X_SAMPLES; ++i) {
		// hermite
		gRedAnimH->Value(r, validInterval, t);
		gGreenAnimH->Value(g, validInterval, t);
		gBlueAnimH->Value(b, validInterval, t);
		glColor3f(r.RealValue(), g.RealValue(), b.RealValue());
		glBegin(GL_POLYGON);
			glVertex2f(x, 0.5f);
			glVertex2f(x + xStep, 0.5f);
			glVertex2f(x + xStep, 0.7f);
			glVertex2f(x, 0.7f);
		glEnd();
		// linear
		gRedAnimL->Value(r, validInterval, t);
		gGreenAnimL->Value(g, validInterval, t);
		gBlueAnimL->Value(b, validInterval, t);
		glColor3f(r.RealValue(), g.RealValue(), b.RealValue());
		glBegin(GL_POLYGON);
			glVertex2f(x, 0.2f);
			glVertex2f(x + xStep, 0.2f);
			glVertex2f(x + xStep, 0.4f);
			glVertex2f(x, 0.4f);
		glEnd();
		// constant
		gRedAnimC->Value(r, validInterval, t);
		gGreenAnimC->Value(g, validInterval, t);
		gBlueAnimC->Value(b, validInterval, t);
		glColor3f(r.RealValue(), g.RealValue(), b.RealValue());
		glBegin(GL_POLYGON);
			glVertex2f(x, 0.1f);
			glVertex2f(x + xStep, 0.1f);
			glVertex2f(x + xStep, -0.1f);
			glVertex2f(x, -0.1f);
		glEnd();
		// next sample
		t += tStep;
		x += xStep;
	}

	// draw animation keys
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
	j = gRedAnimC->KeysCount();
	x = -1.0f;
	for (i = 0; i <= j; i++) {
		gRedAnimC->Key(i, r);
		gGreenAnimC->Key(i, g);
		gBlueAnimC->Key(i, b);

		x = (2.0f * (r.TimePosition() / domainInterval.Length())) - 1.0f;

		glColor3f(r.RealValue(), g.RealValue(), b.RealValue());
		glBegin(GL_LINES);
			glVertex2f(x, -0.2f);
			glVertex2f(x, -0.4f);
		glEnd();
	}
	setDefaultGlobalStates();
	#undef X_SAMPLES
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	setLightAndTransform();

	if (gDrawPalette) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawPalette();
		gDrawPalette = G_FALSE;
	}

	DrawCursor(gCurrentTime, G_FALSE);
	gCurrentTime += gTimeStep;
	DrawCursor(gCurrentTime, G_TRUE);
	glFlush();
}
//------------------------------------------------------------

//----- resizeGL ---------------------------------------------
void QGLWidgetTest::resizeGL(int width, int height) {

	if (height == 0)									// Prevent A Divide By Zero By
		height = 1;										// Making Height Equal One
	glViewport(0, 0, width, height);					// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glOrtho(-1.5, 1.5, -1.5, 1.5, -1.5, 1.5);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();
	gDrawPalette = G_TRUE;
}
//------------------------------------------------------------

void QGLWidgetTest::SetOORType(const GOORType NewOORType) {

	gRedAnimC->SetOORBefore(NewOORType);
	gRedAnimC->SetOORAfter(NewOORType);
	gGreenAnimC->SetOORBefore(NewOORType);
	gGreenAnimC->SetOORAfter(NewOORType);
	gBlueAnimC->SetOORBefore(NewOORType);
	gBlueAnimC->SetOORAfter(NewOORType);

	gRedAnimL->SetOORBefore(NewOORType);
	gRedAnimL->SetOORAfter(NewOORType);
	gGreenAnimL->SetOORBefore(NewOORType);
	gGreenAnimL->SetOORAfter(NewOORType);
	gBlueAnimL->SetOORBefore(NewOORType);
	gBlueAnimL->SetOORAfter(NewOORType);

	gRedAnimH->SetOORBefore(NewOORType);
	gRedAnimH->SetOORAfter(NewOORType);
	gGreenAnimH->SetOORBefore(NewOORType);
	gGreenAnimH->SetOORAfter(NewOORType);
	gBlueAnimH->SetOORBefore(NewOORType);
	gBlueAnimH->SetOORAfter(NewOORType);

}

void QGLWidgetTest::AddKey() {

	GUInt32 i;
	GBool b;
	GTimeValue t;

	t = GMath::RangeRandom((GReal)10, (GReal)990);
	gRedAnimC->AddKey(t, i, b);
	gGreenAnimC->AddKey(t, i, b);
	gBlueAnimC->AddKey(t, i, b);
	gRedAnimL->AddKey(t, i, b);
	gGreenAnimL->AddKey(t, i, b);
	gBlueAnimL->AddKey(t, i, b);
	gRedAnimH->AddKey(t, i, b);
	gGreenAnimH->AddKey(t, i, b);
	gBlueAnimH->AddKey(t, i, b);
}

void QGLWidgetTest::RemoveKey() {

	if (gRedAnimC->KeysCount() < 3)
		return;

	GInt32 i = GMath::RangeRandom(1, gRedAnimC->KeysCount() - 2);

	gRedAnimC->RemoveKey(i);
	gGreenAnimC->RemoveKey(i);
	gBlueAnimC->RemoveKey(i);
	gRedAnimL->RemoveKey(i);
	gGreenAnimL->RemoveKey(i);
	gBlueAnimL->RemoveKey(i);
	gRedAnimH->RemoveKey(i);
	gGreenAnimH->RemoveKey(i);
	gBlueAnimH->RemoveKey(i);
}

// keyboard event handler
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	QString s;

	switch(e->key()) {
		case Qt::Key_F1:
			s = "A/Z: Add/Remove a color key +/-\n";
			s += "Space: Generate a new random palette\n";
			s += "I: Change interpolation type (constant/linear/hermite)\n";
			s += "O: Change 'out of range' behavior (constant/loop/ping-pong)";
			QMessageBox::information(this, "Command keys", s);
			break;
		case Qt::Key_A:
			AddKey();
			gDrawPalette = G_TRUE;
			break;
		case Qt::Key_Z:
			RemoveKey();
			gDrawPalette = G_TRUE;
			break;
		case Qt::Key_I:
			gInterpolationIndex = (gInterpolationIndex + 1) % 3;
			gDrawPalette = G_TRUE;
			break;
		case Qt::Key_O:
			if (gOORType == G_CONSTANT_OOR)
				gOORType = G_LOOP_OOR;
			else
			if (gOORType == G_LOOP_OOR)
				gOORType = G_PINGPONG_OOR;
			else
			if (gOORType == G_PINGPONG_OOR)
				gOORType = G_CONSTANT_OOR;
			SetOORType(gOORType);
			gCurrentTime = 0;
			gDrawPalette = G_TRUE;
			break;
		case Qt::Key_Space:
			BuildNewPalette(gRedAnimC->KeysCount());
			gDrawPalette = G_TRUE;
			break;
	}
}
//------------------------------------------------------------
void QGLWidgetTest::setLightAndTransform() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
//------------------------------------------------------------

//------------------------------------------------------------
void QGLWidgetTest::setDefaultGlobalStates() {

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
//------------------------------------------------------------
