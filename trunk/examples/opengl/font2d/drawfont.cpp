#include "drawfont.h"
#include <amanith/2d/gtesselator2d.h>
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
#endif

#define PrintOpenGLError() gExtManager->PrintOglError(__FILE__, __LINE__)

static int timer_interval = 0;			// timer interval (millisec)

// constructor
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(parent) {

	GString s;

	gKernel = new GKernel();
	gFont = (GFont2D *)gKernel->CreateNew(G_FONT2D_CLASSID);

	// Depth Into The Screen
	gX = -0.4f;
	gY = -0.3f;
	gZ = -2.3f;

	// build path for data (textures)
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

	if (gExtManager)
		delete gExtManager;
	if (gFont)
		delete gFont;
	if (gKernel)
		delete gKernel;
}

//------------------------------------------------------------

void QGLWidgetTest::GenerateTessellation(const GFontChar2D* Char, const GReal Deviation) {

	if (Char == NULL)
		return;

	GInt32 i, j;
	GDynArray<GPoint2> tmpPts;
	GDynArray<GInt32> tmpIndex;
	GTesselator2D tesselator;

	j = (GInt32)Char->OutlinesCount();
	gContoursPoints.clear();
	gContoursIndexes.clear();

	for (i = 0; i < j; i++) {
		tmpPts.clear();
		tmpIndex.clear();

		Char->DrawOutline(i, tmpPts, tmpIndex, Deviation);

		gContoursPoints.insert(gContoursPoints.end(), tmpPts.begin(), tmpPts.end());
		gContoursIndexes.insert(gContoursIndexes.end(), tmpIndex.begin(), tmpIndex.end());
	}
	gVertices.clear();
	tesselator.Tesselate(gContoursPoints, gContoursIndexes, gVertices, G_TRUE);
}

//------------------------------------------------------------

void QGLWidgetTest::timerEvent(QTimerEvent *e) {

	if (!e)
		return;
	updateGL();
}

//----- initializeGL -----------------------------------------
void QGLWidgetTest::initializeGL() {

	GString fName;
	// create extensions manager
	gExtManager = new GOpenglExt();
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);					// Set Line Antialiasing
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

void QGLWidgetTest::DrawTriangles(const GDynArray<GPoint2>& Points) {

	GUInt32 i, j = Points.size(), k;
	GPoint2 a, b, c;
	GPoint<GLfloat, 3> col1(1.0f, 0.74f, 0.20f);
	GPoint<GLfloat, 3> col2(0.72f, 0.0f, 0.0f);
	GPoint<GLfloat, 3> col;

	k = j / 3;
	glBegin(GL_TRIANGLES);
	for (i = 0; i < k; i++) {
		a = Points[i * 3];
		b = Points[i * 3 + 1];
		c = Points[i * 3 + 2];

		col = GMath::Lerp(GMath::Clamp(a[G_X], (GReal)0, (GReal)1), col1, col2);
		glColor3f(col[0], col[1], col[2]);
		glVertex3f(a[G_X], a[G_Y], 1.0f);

		col = GMath::Lerp(GMath::Clamp(b[G_X], (GReal)0, (GReal)1), col1, col2);
		glColor3f(col[0], col[1], col[2]);
		glVertex3f(b[G_X], b[G_Y], 1.0f);

		col = GMath::Lerp(GMath::Clamp(c[G_X], (GReal)0, (GReal)1), col1, col2);
		glColor3f(col[0], col[1], col[2]);
		glVertex3f(c[G_X], c[G_Y], 1.0f);
	}
	glEnd();

	if (!gWireFrame)
		return;

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (i = 0; i < k; i++) {
		a = Points[i * 3];
		b = Points[i * 3 + 1];
		c = Points[i * 3 + 2];
		glVertex3f(a[G_X], a[G_Y], 1.0f);
		glVertex3f(b[G_X], b[G_Y], 1.0f);
		glVertex3f(a[G_X], a[G_Y], 1.0f);
		glVertex3f(c[G_X], c[G_Y], 1.0f);
		glVertex3f(b[G_X], b[G_Y], 1.0f);
		glVertex3f(c[G_X], c[G_Y], 1.0f);
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
			gFillDraw = !gFillDraw;
		case Qt::Key_Space:
			gWireFrame = !gWireFrame;
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
			gDeviation *= 0.5;
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
