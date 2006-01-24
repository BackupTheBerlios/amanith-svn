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

#include "drawshape.h"
#include <amanith/2d/gtesselator2d.h>
#include <amanith/geometry/gxformconv.h>
#include <amanith/geometry/gxform.h>
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
#endif

// timer interval (millisec)
static int timer_interval = 0;
static GLuint texture = 0;

#if defined(G_OS_WIN) && !defined(__CYGWIN__) && !defined(__GNUC__)
	HDC myhdc = 0;
#endif

GLuint  base; // Base Display List For The Font Set
void BuildFont(void) {

#if defined(G_OS_WIN) && !defined(__CYGWIN__) && !defined(__GNUC__)

	myhdc = GetDC(0);
	HFONT  font;									// Windows Font ID
	base = glGenLists(96);							// Storage For 96 Characters

	font = CreateFontA(-10,							// Height Of Font
						0,							// Width Of Font
						0,							// Angle Of Escapement
						0,							// Orientation Angle
						FW_BOLD,					// Font Weight
						FALSE,						// Italic
						FALSE,						// Underline
						FALSE,						// Strikeout
						ANSI_CHARSET,				// Character Set Identifier
						OUT_TT_PRECIS,				// Output Precision
						CLIP_DEFAULT_PRECIS,		// Clipping Precision
						NONANTIALIASED_QUALITY,		// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,	// Family And Pitch
						"Arial");					// Font Name
	SelectObject(myhdc, font);						// Selects The Font We Want
	wglUseFontBitmaps(myhdc, 32, 96, base);
#endif
}

void glMyPrint(const char *fmt) {

#if defined(G_OS_WIN) && !defined(__CYGWIN__) && !defined(__GNUC__)
	char  text[256]; // Holds Our String
	va_list ap;      // Pointer To List Of Arguments

	if (fmt == NULL)
		return;					// Do Nothing
	va_start(ap, fmt);			// Parses The String For Variables
	vsprintf(text, fmt, ap);	// And Converts Symbols To Actual Numbers
	va_end(ap);					// Results Are Stored In Text
	glPushAttrib(GL_LIST_BIT);  // Pushes The Display List Bits
	glListBase(base - 32);		// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
#else
	// just to avoid warnings on non-Windows platforms...
	if (fmt) {
	}
#endif
}

void DeleteFont(void) {
#if defined(G_OS_WIN) && !defined(__CYGWIN__) && !defined(__GNUC__)
	glDeleteLists(base, 96);
#endif 
}

// constructor
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

	// Depth Into The Screen
	gX = -8.0f;
	gY = -8.0f;
	gZ = -22.0f;

	gKernel = new GKernel();

	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";

	gWireFrame = G_TRUE;
	gAnim = G_TRUE;
	gAng1 = gAng2 = gAng3 = 0;

	gFillRule = G_ODD_EVEN_RULE;
	gFillMode = 1;
	gStepAng1 = 0.004f;
	gStepAng2 = -0.002f;
	gStepAng3 = 0.001f;
	gAng1 = gAng2 = gAng3 = 0;

	// load the texture
	gTexture = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	if (!gTexture)
		abort();

	s = gDataPath + "metal05.png";
	err = gTexture->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err != G_NO_ERROR)
		abort();
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gKernel)
		delete gKernel;
	DeleteFont();
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setLightAndTransform();

	if (gAnim) {
		gAng1 += gStepAng1;
		gAng2 += gStepAng2;
		gAng3 += gStepAng3;
	}
	// build the shape
	BuildShape(gAng1, gAng2, gAng3);
	// triangulate
	GenerateTessellation();
	DrawTriangles(gTrianglesPts, gTrianglesIdx);
	glFlush();
}

//------------------------------------------------------------

void QGLWidgetTest::BuildShape(const GFloat RadAngle1, const GFloat RadAngle2, const GFloat RadAngle3) {

	GMatrix33 preTrans, postTrans, rot;
	GMatrix33 finalMatrix;

	// rotation pivot is at (8, 8)
	TranslationToMatrix(preTrans, GVector2(-8, -8));
	TranslationToMatrix(postTrans, GVector2(8, 8));

	gContoursPoints.clear();
	gContoursIndexes.clear();

	// little cross
	RotationToMatrix(rot, (GReal)RadAngle1);
	finalMatrix = (postTrans * rot) * preTrans;
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 7));
	gContoursPoints.push_back(finalMatrix * GPoint2(7, 8));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 9));
	gContoursPoints.push_back(finalMatrix * GPoint2(7, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(8, 9));
	gContoursPoints.push_back(finalMatrix * GPoint2(9, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 9));
	gContoursPoints.push_back(finalMatrix * GPoint2(9, 8));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 7));
	gContoursPoints.push_back(finalMatrix * GPoint2(9, 6));
	gContoursPoints.push_back(finalMatrix * GPoint2(8, 7));
	gContoursPoints.push_back(finalMatrix * GPoint2(7, 6));
	gContoursIndexes.push_back(12);

	// big cross
	RotationToMatrix(rot, (GReal)RadAngle2);
	finalMatrix = (postTrans * rot) * preTrans;

	gContoursPoints.push_back(finalMatrix * GPoint2(2, 6));
	gContoursPoints.push_back(finalMatrix * GPoint2(2, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 14));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 14));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(14, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(14, 6));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 6));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 2));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 2));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 6));
	gContoursIndexes.push_back(12);

	// star
	RotationToMatrix(rot, (GReal)RadAngle3);
	finalMatrix = (postTrans * rot) * preTrans;
	gContoursPoints.push_back(finalMatrix * GPoint2(2, 2));
	gContoursPoints.push_back(finalMatrix * GPoint2(5, 8));
	gContoursPoints.push_back(finalMatrix * GPoint2(2, 14));
	gContoursPoints.push_back(finalMatrix * GPoint2(8, 11));
	gContoursPoints.push_back(finalMatrix * GPoint2(14, 14));
	gContoursPoints.push_back(finalMatrix * GPoint2(11, 8));
	gContoursPoints.push_back(finalMatrix * GPoint2(14, 2));
	gContoursPoints.push_back(finalMatrix * GPoint2(8, 5));
	gContoursIndexes.push_back(8);

	// square
	gContoursPoints.push_back(GPoint2(1, 1));
	gContoursPoints.push_back(GPoint2(1, 15));
	gContoursPoints.push_back(GPoint2(15, 15));
	gContoursPoints.push_back(GPoint2(15, 1));
	gContoursIndexes.push_back(4);
}

void QGLWidgetTest::GenerateTessellation() {

	GTesselator2D tesselator;

	gTrianglesPts.clear();
	gTrianglesIdx.clear();
	tesselator.Tesselate(gContoursPoints, gContoursIndexes, gTrianglesPts, gTrianglesIdx, (GFillBehavior)gFillRule);
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
	glClearColor(0.6f, 0.6f, 0.6f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);					// Set Line Antialiasing
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	// bind the texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gTexture->Width(), gTexture->Height(),
				 0, GL_BGRA, GL_UNSIGNED_BYTE, gTexture->Pixels());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setDefaultGlobalStates();
	startTimer(timer_interval);
	BuildFont();
}

//------------------------------------------------------------

void QGLWidgetTest::DrawTriangles(const GDynArray< GPoint<GDouble, 2> >& Points,
								  const GDynArray<GULong>& Indexes) {

	GUInt32 i, j = Indexes.size();
	GPoint<GDouble, 2> a, b, c;
	GPoint<GDouble, 3> col;
	GLfloat texPlaneGen[4];

	if (gFillMode == 2 || gFillMode == 3) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		texPlaneGen[0] = 0.05f;
		texPlaneGen[1] = 0;
		texPlaneGen[2] = 0;
		texPlaneGen[3] = 0;
		glTexGenfv(GL_S, GL_EYE_PLANE, texPlaneGen);
		texPlaneGen[0] = 0;
		texPlaneGen[1] = 0.05f;
		texPlaneGen[2] = 0;
		texPlaneGen[3] = 0;
		glTexGenfv(GL_T, GL_EYE_PLANE, texPlaneGen);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
	}

	// debug stuff
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.2f, 0.4f);
	a[G_X] = -2;
	for (i = 0; i < 21; i++) {
		glVertex3d(a[G_X], 0.0f, 1.0f);
		glVertex3d(a[G_X], 16.0f, 1.0f);
		a[G_X] += 1.0;
	}
	a[G_Y] = 0;
	for (i = 0; i < 17; i++) {
		glVertex3d(-2, a[G_Y], 1.0f);
		glVertex3d(18.0, a[G_Y], 1.0f);
		a[G_Y] += 1.0;
	}

	glColor3f(0.6f, 0.0f, 0.3f);
	a[G_X] = -1.5;
	for (i = 0; i < 20; i++) {
		glVertex3d(a[G_X], 0.0f, 1.0f);
		glVertex3d(a[G_X], 16.0f, 1.0f);
		a[G_X] += 1.0;
	}
	a[G_Y] = 0.5;
	for (i = 0; i < 16; i++) {
		glVertex3d(-2, a[G_Y], 1.0f);
		glVertex3d(18.0, a[G_Y], 1.0f);
		a[G_Y] += 1.0;
	}
	glEnd();


	glEnable(GL_BLEND);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	glBlendFunc(GL_ONE, GL_ONE);

	glBegin(GL_TRIANGLES);
	if (gFillMode == 0) {
		glColor3f(0.2f, 0.2f, 0.2f);
		for (i = 0; i < j; i+=3) {
			a = Points[Indexes[i]];
			b = Points[Indexes[i + 1]];
			c = Points[Indexes[i + 2]];
			glVertex3d(a[G_X], a[G_Y], 1.0);
			glVertex3d(b[G_X], b[G_Y], 1.0);
			glVertex3d(c[G_X], c[G_Y], 1.0);
		}
	}
	else
	if (gFillMode == 1) {
		GPoint<GDouble, 3> col1(0.0f, 0.1f, 0.2f);
		GPoint<GDouble, 3> col2(0.3f, 0.2f, 1.0f);
		for (i = 0; i < j; i+=3) {
			a = Points[Indexes[i]];
			b = Points[Indexes[i + 1]];
			c = Points[Indexes[i + 2]];
			col = GMath::BarycentricConvexSum(a[G_X] + 1.0, col1, 18.0 - a[G_X], col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(a[G_X], a[G_Y], 1.0);
			col = GMath::BarycentricConvexSum(b[G_X] + 1.0, col1, 18.0 - b[G_X], col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(b[G_X], b[G_Y], 1.0);
			col = GMath::BarycentricConvexSum(c[G_X] + 1.0, col1, 18.0 - c[G_X], col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(c[G_X], c[G_Y], 1.0);
		}
	}
	else
	if (gFillMode == 2) {
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 0; i < j; i+=3) {
			a = Points[Indexes[i]];
			b = Points[Indexes[i + 1]];
			c = Points[Indexes[i + 2]];
			glVertex3d(a[G_X], a[G_Y], 1.0);
			glVertex3d(b[G_X], b[G_Y], 1.0);
			glVertex3d(c[G_X], c[G_Y], 1.0);
		}
	}
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);
	if (!gWireFrame)
		return;

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (i = 0; i < j; i+=3) {
		a = Points[Indexes[i]];
		b = Points[Indexes[i + 1]];
		c = Points[Indexes[i + 2]];
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
	}
	glEnd();

	GString s, s2;
	j = Points.size();
	glColor3f(0.1f, 0.1f, 0.1f);
	for (i = 0; i < j; ++i) {
		a = Points[i];
		s = StrUtils::ToString(a, ";", "%5.2f");
		s = StrUtils::Purge(s, " ");
		glRasterPos2d(a[G_X], a[G_Y]);
		glMyPrint(StrUtils::ToAscii(s));
	}
}

//------------------------------------------------------------

//----- resizeGL ---------------------------------------------
void QGLWidgetTest::resizeGL(int width, int height) {

	if (height == 0)									// Prevent A Divide By Zero By
		height = 1;										// Making Height Equal One
	glViewport(0, 0, width, height);						// Reset The Current Viewport
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
			s += "M/N: Accelerate/Decelerate rotation\n";
			s += "F: Change fill rule\n";
			s += "C: Change fill mode\n";
			s += "B: Toggle animation\n";
			QMessageBox::information(this, "Command keys", s);
			break;

		case Qt::Key_Space:
			if (gWireFrame)
				gWireFrame = G_FALSE;
			else
				gWireFrame = G_TRUE;
			break;
		case Qt::Key_A:
			gZ -= 0.5f;
			break;
		case Qt::Key_Z:
			gZ += 0.5f;
			break;
		case Qt::Key_S:
			gZ -= 0.025f;
			break;
		case Qt::Key_X:
			gZ += 0.025f;
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
		case Qt::Key_8:
			gY += 0.025f;
			break;
		case Qt::Key_2:
			gY -= 0.025f;
			break;
		case Qt::Key_6:
			gX += 0.025f;
			break;
		case Qt::Key_4:
			gX -= 0.025f;
			break;	
		case Qt::Key_B:
			if (gAnim)
				gAnim = G_FALSE;
			else
				gAnim = G_TRUE;
			break;
		case Qt::Key_M:
			gStepAng1 *= 1.25f;
			gStepAng2 *= 1.25f;
			gStepAng3 *= 1.25f;
			break;
		case Qt::Key_N:
			gStepAng1 /= 1.25f;
			gStepAng2 /= 1.25f;
			gStepAng3 /= 1.25f;
			break;
		case Qt::Key_F:
			if (gFillRule == G_ODD_EVEN_RULE)
				gFillRule = G_EVEN_ODD_RULE;
			else
			if (gFillRule == G_EVEN_ODD_RULE)
				gFillRule = G_ANY_RULE;
			else
				gFillRule = G_ODD_EVEN_RULE;
			break;
		case Qt::Key_C:
			gFillMode = (gFillMode + 1) % 3;
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
