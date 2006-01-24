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

#include "drawpath.h"
#include <amanith/geometry/gxform.h>
#include <amanith/2d/gbeziercurve2d.h>
#include <amanith/2d/gbsplinecurve2d.h>
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
#endif

// timer interval (millisec)
static int timer_interval = 150;

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
	gPath = (GPath2D *)gKernel->CreateNew(G_PATH2D_CLASSID);

	GBezierCurve2D bezCurve;
	GBSplineCurve2D bsplineCurve;

	// bezier segment
	gVertices.push_back(GPoint2(2, 8));
	gVertices.push_back(GPoint2(5, 13));
	gVertices.push_back(GPoint2(8, 11));
	bezCurve.SetPoints(gVertices);
	bezCurve.SetDomain(0, (GReal)0.2);
	gPath->AppendCurve(bezCurve);
	// another bezier segment
	gVertices.clear();
	gVertices.push_back(GPoint2(8, 11));
	gVertices.push_back(GPoint2(11, 15));
	gVertices.push_back(GPoint2(15, 10));
	gVertices.push_back(GPoint2(13, 5));
	bezCurve.SetPoints(gVertices);
	bezCurve.SetDomain((GReal)0.2, (GReal)0.5);
	gPath->AppendCurve(bezCurve);
	// b-spline curve segment
	gVertices.clear();
	gVertices.push_back(GPoint2(13, 5));
	gVertices.push_back(GPoint2(5, 3));
	gVertices.push_back(GPoint2(10, 8));
	gVertices.push_back(GPoint2(8, 3));
	bsplineCurve.SetPoints(gVertices, 3, (GReal)0.5, 1);
	gPath->AppendCurve(bsplineCurve);
	// close the path
	gPath->ClosePath();

	// Depth Into The Screen
	gX = -8;
	gY = -9;
	gZ = -16.0f;

	gIntersectionRay.SetOrigin(GPoint2(0, 0));
	gIntersectionRay.SetDirection(GPoint2(0, 0));
	GMath::SeedRandom();

	gDeviation = (GReal)1e-5;
	BuildFlatContour(gPath);
}
//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setLightAndTransform();
	Draw(gPath);

	glFlush();
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gKernel)
		delete gKernel;
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
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);					// Set Line Antialiasing
	glDisable(GL_LIGHTING);
	setDefaultGlobalStates();
	startTimer(timer_interval);
}
//------------------------------------------------------------

void QGLWidgetTest::BuildFlatContour(const GPath2D* Path) {

	gVertices.clear();
	Path->Flatten(gVertices, gDeviation);
}

//------------------------------------------------------------
void QGLWidgetTest::Draw(const GPath2D* Path) {

	GInt32 numSegs, i, j;
	GPoint2 p1, p2;

	if (gVertices.size() > 0) {
		glDisable(GL_LINE_SMOOTH);
		glLineWidth(2.0f);
		glColor3f(1.0f, 0.7f, 0.25f);
		glBegin(GL_LINES);
		numSegs = (GInt32)gVertices.size() - 1;
		for (i = 0; i < numSegs; i++) {
			p1 = gVertices[i];
			p2 = gVertices[i + 1];
			glVertex3f(p1[G_X], p1[G_Y], 1.0f);
			glVertex3f(p2[G_X], p2[G_Y], 1.0f);
		}
		// we have to draw last segment, spanned by last point and first point
		if (Path->IsClosed()) {
			p1 = p2;
			p2 = gVertices[0];
			glVertex3f(p1[G_X], p1[G_Y], 1.0f);
			glVertex3f(p2[G_X], p2[G_Y], 1.0f);
		}
		glEnd();
	}
	// draw control polygon
	glLineWidth(1.0f);
	glColor3f(0.0f, 0.5f, 1.0f);
	glBegin(GL_LINES);

	numSegs = Path->PointsCount();
	for (i = 0; i < numSegs - 1; i++) {
		p1 = Path->Point(i);
		p2 = Path->Point(i + 1);
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
		glVertex3f(p2[G_X], p2[G_Y], 1.0f);
	}
	// draw last control polygon segment
	if (Path->IsClosed()) {
		p1 = p2;
		p2 = Path->Point(0);
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
		glVertex3f(p2[G_X], p2[G_Y], 1.0f);
	}

	// draw ray used for intersection test
	glColor3f(1.0f, 0.3f, 0.1f);
	glVertex3f(gIntersectionRay.Origin()[G_X], gIntersectionRay.Origin()[G_Y], 1.0f);
	glVertex3f(gIntersectionRay.Origin()[G_X] + 20 * gIntersectionRay.Direction()[G_X],
			gIntersectionRay.Origin()[G_Y] + 20 * gIntersectionRay.Direction()[G_Y], 1.0f);
	glEnd();

	// draw intersection points
	glPointSize(5.0);
	glColor3f(0.0f, 1.0f, 0.3f);
	glBegin(GL_POINTS);
	j = gIntersectionPoints.size();
	for (i = 0; i < j; i++) {
		p1 = Path->Evaluate(gIntersectionPoints[i][G_X]);
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
	}
	glEnd();
}

//------------------------------------------------------------

//----- resizeGL ---------------------------------------------
void QGLWidgetTest::resizeGL(int width, int height) {

	if (height == 0)									// Prevent A Divide By Zero By
		height = 1;										// Making Height Equal One
	glViewport(0, 0, width, height);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	
}
//------------------------------------------------------------

// keyboard event handler
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	QString s;
	GBool b;
	GVector2 randomDir;

	switch(e->key()) {
		case Qt::Key_F1:
			s = "A/Z: Zoom +/-\n";
			s += "Arrow keys: Move viewport\n";
			s += "Space: Generate a random ray and intersect the curve\n";
			s += "M/N: Fine/Rough adaptive flattening\n";
			QMessageBox::information(this, "Command keys", s);
			break;
		case Qt::Key_A:
			gZ -= 0.5f;
			break;
		case Qt::Key_Z:
			gZ += 0.5f;
			break;
		case Qt::Key_Up:
			gY += 0.5f;
			break;
		case Qt::Key_Down:
			gY -= 0.5f;
			break;
		case Qt::Key_Right:
			gX += 0.5f;
			break;
		case Qt::Key_Left:
			gX -= 0.5f;
			break;
		case Qt::Key_M:
			gDeviation *= 0.5;
			BuildFlatContour(gPath);
			break;
		case Qt::Key_N:
			gDeviation *= 2;
			BuildFlatContour(gPath);
			break;
		case Qt::Key_Space:
			gIntersectionRay.SetOrigin(GPoint2(8.5, 8.5));
			randomDir[G_X] = GMath::RangeRandom(-1.0, 1.0);
			randomDir[G_Y] = GMath::RangeRandom(-1.0, 1.0);
			gIntersectionRay.SetDirection(randomDir);
			gIntersectionRay.Normalize();
			gIntersectionPoints.clear();
			b = gPath->IntersectRay(gIntersectionRay, gIntersectionPoints);
			break;
	}
}
//------------------------------------------------------------
void QGLWidgetTest::setLightAndTransform() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(gX, gY, gZ);
}

//------------------------------------------------------------
void QGLWidgetTest::setDefaultGlobalStates() {

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
//------------------------------------------------------------
