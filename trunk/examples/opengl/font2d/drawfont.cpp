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

#include "drawfont.h"
#include <amanith/2d/gtesselator2d.h>
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

	GString s;

	gKernel = new GKernel();
	gFont = (GFont2D *)gKernel->CreateNew(G_FONT2D_CLASSID);

	// Depth Into The Screen
	gX = -0.4f;
	gY = -0.3f;
	gZ = -2.3f;

	// build path for data
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";

	s = StrUtils::OSFixPath(gDataPath + "crazk.ttf", G_FALSE);
	gFont->Load(StrUtils::ToAscii(s), "scale=0");

	gWireFrame = G_TRUE;
	gFillDraw = G_TRUE;
	gDeviation = (GReal)0.000003;
	gCurrentChar = 'S';
	gChar = gFont->CharByCode(gCurrentChar, 0);
	GenerateTessellation(gChar, gDeviation);
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gKernel)
		delete gKernel;
}

//------------------------------------------------------------

void QGLWidgetTest::DrawChar(const GFontChar2D* Char, const GReal Deviation, const GMatrix33& Transformation) {

	GUInt32 i, j;

	if (!Char->IsComposite()) {
		GDynArray<GPoint2> tmpPts;
		GDynArray<GInt32> tmpIndex;
		GUInt32 oldSize = 0;

		// extract all contours
		j = Char->ContoursCount();
		for (i = 0; i < j; ++i) {
			Char->Contour(i)->DrawContour(tmpPts, Deviation, Transformation);
			tmpIndex.push_back(tmpPts.size() - oldSize);
			oldSize = tmpPts.size();
			// insert generated point into internal arrays, used for the pure wireframe rendering mode
			gContoursPoints.insert(gContoursPoints.end(), tmpPts.begin(), tmpPts.end());
			gContoursIndexes.insert(gContoursIndexes.end(), tmpIndex.begin(), tmpIndex.end());
		}
		// triangulate contours
		GTesselator2D tesselator;
		tesselator.Tesselate(tmpPts, tmpIndex, gVertices, G_ODD_EVEN_RULE);
	}
	else {
		const GFontChar2D* subChar;
		GSubChar2D subCharInfo;
		GMatrix33 totalMatrix;
		// extract all sub characters
		j = Char->SubCharsCount();
		for (i = 0; i < j; ++i) {
			Char->SubChar(i, subCharInfo);
			// concatenate transformations
			totalMatrix = Transformation * subCharInfo.Transformation;
			subChar = gFont->CharByIndex(subCharInfo.GlyphIndex);
			// draw sub character
			DrawChar(subChar, Deviation, totalMatrix);
		}
	}
}

void QGLWidgetTest::GenerateTessellation(const GFontChar2D* Char, const GReal Deviation) {

	if (Char == NULL)
		return;

	// start with an identity transformation
	GMatrix33 idMatrix;

	gContoursPoints.clear();
	gContoursIndexes.clear();
	gVertices.clear();
	DrawChar(Char, Deviation, idMatrix);
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
	glDisable(GL_CULL_FACE);
	setDefaultGlobalStates();
	startTimer(timer_interval);
}
//------------------------------------------------------------

void QGLWidgetTest::DrawOutlines(const GDynArray<GPoint2>& ContoursPoints,
								 const GDynArray<GInt32>& ContoursIndexes) {

	GUInt32 i, j, k, w, ofs;
	GPoint2 p1, p2;

	ofs = 0;
	j = ContoursIndexes.size();
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.4f, 0.9f);
	for (i = 0; i < j; i++) {
		k = ContoursIndexes[i];
		for (w = 0; w < k - 1; w++) {
			p1 = ContoursPoints[ofs + w];
			p2 = ContoursPoints[ofs + w + 1];
			glVertex3f(p1[G_X], p1[G_Y], 1.0f);
			glVertex3f(p2[G_X], p2[G_Y], 1.0f);
		}
		p1 = ContoursPoints[ofs + k - 1];
		p2 = ContoursPoints[ofs];
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
		glVertex3f(p2[G_X], p2[G_Y], 1.0f);
		ofs += k;
	}
	glEnd();
}

//------------------------------------------------------------

void QGLWidgetTest::DrawTriangles(const GDynArray< GPoint<GDouble, 2> >& Points) {

	GUInt32 i, j = Points.size();
	GPoint<GDouble, 2> a, b, c;
	GPoint<GDouble, 3> col1(1.0f, 0.74f, 0.20f);
	GPoint<GDouble, 3> col2(0.72f, 0.0f, 0.0f);
	GPoint<GDouble, 3> col;

	glBegin(GL_TRIANGLES);
	for (i = 0; i < j; i+=3) {
		a = Points[i];
		b = Points[i + 1];
		c = Points[i + 2];
		// generate color for first point
		col = GMath::Lerp(GMath::Clamp(a[G_X], (GDouble)0, (GDouble)1), col1, col2);
		glColor3d(col[0], col[1], col[2]);
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		// generate color for second point
		col = GMath::Lerp(GMath::Clamp(b[G_X], (GDouble)0, (GDouble)1), col1, col2);
		glColor3d(col[0], col[1], col[2]);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		// generate color for third point
		col = GMath::Lerp(GMath::Clamp(c[G_X], (GDouble)0, (GDouble)1), col1, col2);
		glColor3d(col[0], col[1], col[2]);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
	}
	glEnd();

	if (!gWireFrame)
		return;

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (i = 0; i < j; i+=3) {
		a = Points[i];
		b = Points[i + 1];
		c = Points[i + 2];
		// draw edge lines
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
	}
	glEnd();
}


//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setLightAndTransform();

	if (gFillDraw)
		DrawTriangles(gVertices);
	else
		DrawOutlines(gContoursPoints, gContoursIndexes);
	glFlush();
}

//----- resizeGL ---------------------------------------------
void QGLWidgetTest::resizeGL(int width, int height) {

	if (height == 0)									// Prevent A Divide By Zero By
		height = 1;										// Making Height Equal One
	glViewport(0, 0, width, height);					// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 1500.0f);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	
}
//------------------------------------------------------------

// keyboard event handler
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	QString s;

	switch(e->key()) {
		case Qt::Key_F1:
			s = "A/Z: Zoom +/-\n";
			s += "Arrow keys: Move viewport\n";
			s += "Space: Toggle wireframe\n";
			s += "M/N: Fine/Rough adaptive flattening\n";
			s += "F: Toggle solid/outlines\n";
			s += "PgUp/PgDown: Previous/Next font letter";
			QMessageBox::information(this, "Command keys", s);
			break;

		case Qt::Key_F:
			if (gFillDraw)
				gFillDraw = G_FALSE;
			else
                gFillDraw = G_TRUE;
			break;
		case Qt::Key_Space:
			if (gWireFrame)
				gWireFrame = G_FALSE;
			else
				gWireFrame = G_TRUE;
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
		case Qt::Key_M:
			gDeviation /= 2;
			GenerateTessellation(gChar, gDeviation);
			break;
		case Qt::Key_N:
			gDeviation *= 2;
			GenerateTessellation(gChar, gDeviation);
			break;
		case Qt::Key_PageDown:
			gCurrentChar++;
			gChar = gFont->CharByCode(gCurrentChar, 0);
			GenerateTessellation(gChar, gDeviation);
			break;
		case Qt::Key_PageUp:
			if (gCurrentChar > 0) {
				gCurrentChar--;
				gChar = gFont->CharByCode(gCurrentChar, 0);
				GenerateTessellation(gChar, gDeviation);
			}
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

//------------------------------------------------------------
void QGLWidgetTest::setDefaultGlobalStates() {

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
//------------------------------------------------------------
