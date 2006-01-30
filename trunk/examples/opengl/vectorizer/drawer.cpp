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

#include "drawer.h"
#include <amanith/geometry/gvect.h>
#include <amanith/2d/gtracer2d.h>
#include <amanith/2d/gtesselator2d.h>
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
#endif

// timer interval (millisec)
static int timer_interval = 150;

#ifdef USE_QT4
QGLWidgetTest::QGLWidgetTest(const QGLFormat& Format, QWidget *parent) : QGLWidget(Format, parent) {
#else
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(parent) {
#endif

#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	GString s;
	GError err;

	gKernel = new GKernel();

	gPixMap1 = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	if (!gPixMap1)
		abort();

	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";

	s = gDataPath + "class_dancer.png";
	err = gPixMap1->Load(StrUtils::ToAscii(s));
	if (err != G_NO_ERROR)
		abort();

	gX = (GReal)-0.3;
	gY = (GReal)-0.5;
	gZ = (GReal)-1.7;

	GTracer2D::Trace(*gPixMap1, gPaths, 255, G_CONNECT_NOT_DOMINANT, 5, 2, (GReal)0.7655);

	gDeviation = (GReal)1e-7;
	
	gWireFrame = G_FALSE;
	gFillDraw = G_TRUE;
	FlattenContours();
}

QGLWidgetTest::~QGLWidgetTest() {

	if (gKernel)
		delete gKernel;
}

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

void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	QString s;

	switch(e->key()) {
		case Qt::Key_N:
			gDeviation *= 2;
			FlattenContours();
			break;
		case Qt::Key_M:
			gDeviation /= 2;
			FlattenContours();
			break;

		case Qt::Key_F1:
			s = "A/Z: Zoom +/-\n";
			s += "Arrow keys: Move viewport\n";
			s += "F: Toggle solid/outlines\n";
			s += "Space: Toggle wireframe (just for solid filling)\n";
			s += "M/N: Fine/Rough adaptive flattening\n";
			QMessageBox::information(this, "Command keys", s);
			break;

		case Qt::Key_Space:
			if (gWireFrame)
				gWireFrame = G_FALSE;
			else
				gWireFrame = G_TRUE;
			break;
		case Qt::Key_F:
			if (gFillDraw)
				gFillDraw = G_FALSE;
			else
				gFillDraw = G_TRUE;
			break;
		case Qt::Key_A:
			gZ -= 0.05f;
			break;
		case Qt::Key_Z:
			gZ += 0.05f;
			break;
		case Qt::Key_Up:
			gY += 0.05f;
			break;
		case Qt::Key_Down:
			gY -= 0.05f;
			break;
		case Qt::Key_Right:
			gX += 0.05f;
			break;
		case Qt::Key_Left:
			gX -= 0.05f;
			break;
	}
}

void QGLWidgetTest::timerEvent(QTimerEvent *e) {

	if (!e)
		return;

	updateGL();
}

void QGLWidgetTest::FlattenContours() {

	GUInt32 i, j, k0, k1;

	gVertices.clear();
	gIndex.clear();
	gTriangles.clear();

	j = gPaths.size();
	k0 = k1 = 0;
	for (i = 0; i < j; i++) {
		gPaths[i].DrawContour(gVertices, gDeviation);

		k1 = gVertices.size();
		gIndex.push_back((GInt32)k1 - (GInt32)k0);
		k0 = k1;
	}

	GTesselator2D tesselator;
	tesselator.Tesselate(gVertices, gIndex, gTriangles, G_ODD_EVEN_RULE);
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	GInt32 i, j, k, w, ofs;
	GPoint<GDouble, 2> a, b, c;
	GPoint2 p1, p2;
	GPoint<GDouble, 3> col1(1.0f, 0.74f, 0.2f);
	GPoint<GDouble, 3> col2(0.4f, 0.1f, 0.6f);
	GPoint<GDouble, 3> col;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setLightAndTransform();

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	if (gFillDraw) {
		k = gTriangles.size() / 3;
		glBegin(GL_TRIANGLES);
		for (i = 0; i < k; i++) {
			a = gTriangles[i * 3];
			b = gTriangles[i * 3 + 1];
			c = gTriangles[i * 3 + 2];

			col = GMath::Lerp(GMath::Clamp(a[G_Y], (GDouble)0, (GDouble)1), col1, col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(a[G_X], a[G_Y], 1.0f);

			col = GMath::Lerp(GMath::Clamp(b[G_Y], (GDouble)0, (GDouble)1), col1, col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(b[G_X], b[G_Y], 1.0f);

			col = GMath::Lerp(GMath::Clamp(c[G_Y], (GDouble)0, (GDouble)1), col1, col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(c[G_X], c[G_Y], 1.0f);
		}
		glEnd();

		if (!gWireFrame)
			return;

		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 0; i < k; i++) {
			a = gTriangles[i * 3];
			b = gTriangles[i * 3 + 1];
			c = gTriangles[i * 3 + 2];
			glVertex3d(a[G_X], a[G_Y], 1.0f);
			glVertex3d(b[G_X], b[G_Y], 1.0f);
			glVertex3d(a[G_X], a[G_Y], 1.0f);
			glVertex3d(c[G_X], c[G_Y], 1.0f);
			glVertex3d(b[G_X], b[G_Y], 1.0f);
			glVertex3d(c[G_X], c[G_Y], 1.0f);
		}
		glEnd();
	}
	else {
		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);

		j = gIndex.size();
		ofs = 0;
		for (i = 0; i < j; i++) {
			k = gIndex[i];
			for (w = 0; w < k - 1; w++) {
				p1 = gVertices[ofs + w];
				p2 = gVertices[ofs + w + 1];
				glVertex3f(p1[G_X], p1[G_Y], 1.0f);
				glVertex3f(p2[G_X], p2[G_Y], 1.0f);
			}
			p1 = gVertices[ofs + k - 1];
			p2 = gVertices[ofs];
			glVertex3f(p1[G_X], p1[G_Y], 1.0f);
			glVertex3f(p2[G_X], p2[G_Y], 1.0f);
			ofs += k;
		}
		glEnd();
	}

	glFlush();
}
//------------------------------------------------------------

//----- resizeGL ---------------------------------------------
void QGLWidgetTest::resizeGL(int width, int height) {

	if (height == 0)									// Prevent A Divide By Zero By
		height = 1;										// Making Height Equal One
	glViewport(0, 0, width, height);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	
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
