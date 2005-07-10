#include "drawhermite.h"
#include <amanith/geometry/gxform.h>
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


	GDynArray<GPoint2> points;
	GDynArray<GHermiteKey> keys;

	gKernel = new GKernel();
	gHermCurve = (GHermiteCurve2D *)gKernel->CreateNew(G_HERMITECURVE2D_CLASSID);


	keys.push_back(GHermiteKey(0, GPoint2(2, 7), GVector2(8, 4), GVector2(8, 4)));
	keys.push_back(GHermiteKey(1, GPoint2(7, 6), GVector2(-4, -8), GVector2(8, -4)));
	keys.push_back(GHermiteKey(2, GPoint2(6, 5), GVector2(-4, 8), GVector2(-4, -8)));
	keys.push_back(GHermiteKey(3, GPoint2(4, 6), GVector2(-4, -4), GVector2(-4, 4)));
	keys.push_back(GHermiteKey(4, GPoint2(3, 3), GVector2(4, -8), GVector2(4, -8)));
	keys.push_back(GHermiteKey(5, GPoint2(8, 2), GVector2(4, -1), GVector2(4, 0)));
	gHermCurve->SetKeys(keys);

	gX = -5.5;
	gY = -5;
	gZ = -12;

	gIntersectionRay.SetOrigin(GPoint2(0, 0));
	gIntersectionRay.SetDirection(GPoint2(0, 0));
	GMath::SeedRandom();

	gDeviation = (GReal)0.0001;
	BuildFlatContour(gHermCurve);
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gExtManager)
		delete gExtManager;
	if (gHermCurve)
		delete gHermCurve;
	if (gKernel)
		delete gKernel;
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setLightAndTransform();
	DrawHermite(gHermCurve);
	glFlush();
}
//------------------------------------------------------------

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

	setDefaultGlobalStates();
	startTimer(timer_interval);
}
//------------------------------------------------------------

void QGLWidgetTest::BuildFlatContour(const GHermiteCurve2D* Curve) {

	gVertices.clear();
	Curve->Flatten(gVertices, gDeviation);
}

//------------------------------------------------------------

void QGLWidgetTest::DrawHermite(const GHermiteCurve2D* Curve) {

	GInt32 numSegs, i, j;
	GPoint2 p1, p2;
	GHermiteKey tmpKey;

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
	glEnd();

	// draw keys tangents
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	j = Curve->PointsCount();
	for (i = 0; i < j; i++) {
		Curve->Key(i, tmpKey);
		p1 = tmpKey.Value;
		p2 = p1 + (tmpKey.InTangent * 0.1);
		glColor3f(0.1f, 0.4f, 1.0f);
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
		glVertex3f(p2[G_X], p2[G_Y], 1.0f);

		p2 = p1 + (tmpKey.OutTangent * 0.1);
		glColor3f(0.1f, 1.0f, 0.4f);
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
		glVertex3f(p2[G_X], p2[G_Y], 1.0f);
	}
	glEnd();

	// draw keys points
	glPointSize(5.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POINTS);
	j = Curve->PointsCount();
	for (i = 0; i < j; i++) {
		p1 = Curve->Point(i);
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
	}
	glEnd();

	// draw ray used for intersection test
	glLineWidth(1.0f);
	glColor3f(1.0f, 0.3f, 0.1f);
	glBegin(GL_LINES);
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
		p1 = Curve->Evaluate(gIntersectionPoints[i][G_X]);
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
	}
	glEnd();

	setDefaultGlobalStates();
}

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
			BuildFlatContour(gHermCurve);
			break;
		case Qt::Key_N:
			gDeviation *= 2;
			BuildFlatContour(gHermCurve);
			break;
		case Qt::Key_Space:
			gIntersectionRay.SetOrigin(GPoint2(5.5, 5.0));
			randomDir[G_X] = GMath::RangeRandom(-1.0, 1.0);
			randomDir[G_Y] = GMath::RangeRandom(-1.0, 1.0);
			gIntersectionRay.SetDirection(randomDir);
			gIntersectionRay.Normalize();
			gIntersectionPoints.clear();
			b = gHermCurve->IntersectRay(gIntersectionRay, gIntersectionPoints);
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
