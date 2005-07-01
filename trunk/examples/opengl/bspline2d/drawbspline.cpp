#include "drawbspline.h"
#include <amanith/geometry/gxform.h>
#include <qmessagebox.h>

#define PrintOpenGLError() gExtManager->PrintOglError(__FILE__, __LINE__)

static int timer_interval = 0;			// timer interval (millisec)

// constructor
QGLWidgetTest::QGLWidgetTest(QWidget * parent, const char * name) : QGLWidget(parent, name) {

	gKernel = new GKernel();
	gBSplineCurve = (GBSplineCurve2D *)gKernel->CreateNew(G_BSPLINECURVE2D_CLASSID);

	gInterpolationPoints.push_back(GPoint2(4, 1));
	gInterpolationPoints.push_back(GPoint2(2, 3));
	gInterpolationPoints.push_back(GPoint2(6, 8));
	gInterpolationPoints.push_back(GPoint2(8, 4));
	gInterpolationPoints.push_back(GPoint2(5, 3));
	gInterpolationPoints.push_back(GPoint2(2, 10));

	GError err = gBSplineCurve->GlobalFit(3, gInterpolationPoints, 0, 1);
	if (err != G_NO_ERROR)
		exit(-1);

	// Depth Into The Screen
	gX = -5.4f;
	gY = -6.3f;
	gZ = -18.0f;

	gIntersectionRay.SetOrigin(GPoint2(0, 0));
	gIntersectionRay.SetDirection(GPoint2(0, 0));
	GMath::SeedRandom();
	gDeviation = (GReal)0.0001;
	BuildFlatContour(gBSplineCurve);
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	this->killTimers();

	if (gExtManager)
		delete gExtManager;
	if (gBSplineCurve)
		delete gBSplineCurve;
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

void QGLWidgetTest::BuildFlatContour(const GBSplineCurve2D* Curve) {

	gVertices.clear();
	Curve->Flatten(gVertices, gDeviation);
}

//------------------------------------------------------------
void QGLWidgetTest::Draw(const GBSplineCurve2D* Curve) {

	GInt32 numSegs, i, j;
	GPoint2 p1, p2;

	// draw curve
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.7f, 0.25f);
	numSegs = (GInt32)gVertices.size() - 1;
	for (i = 0; i < numSegs; i++) {
		p1 = gVertices[i];
		p2 = gVertices[i + 1];
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
		glVertex3f(p2[G_X], p2[G_Y], 1.0f);
	}
	glEnd();

	// draw control polygon
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.5f, 1.0f);
	numSegs = Curve->PointsCount() - 1;
	for (i = 0; i < numSegs; i++) {
		p1 = Curve->Point(i);
		p2 = Curve->Point(i + 1);
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
	glBegin(GL_POINTS);
	glColor3f(0.0f, 1.0f, 0.3f);
	j = gIntersectionPoints.size();
	for (i = 0; i < j; i++) {
		p1 = Curve->Evaluate(gIntersectionPoints[i][G_X]);
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
	}
	glEnd();

	// draw interpolated point
	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(6.0f);
    glBegin(GL_POINTS);
	numSegs = gInterpolationPoints.size() - 1;
	for (i = 0; i <= numSegs; i++) {
		p1 = gInterpolationPoints[i];
		glVertex3f(p1[G_X], p1[G_Y], 1.0f);
	}
	glEnd();
}

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setLightAndTransform();
	Draw(gBSplineCurve);

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
			s += "PgUp/PgDown: Degree elevation/reduction";
			QMessageBox::information(this, "Command keys", s);
			break;
		case Qt::Key_PageUp:
			gBSplineCurve->HigherDegree(1);
			BuildFlatContour(gBSplineCurve);
			break;
		case Qt::Key_PageDown:
			gBSplineCurve->LowerDegree();
			BuildFlatContour(gBSplineCurve);
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
			BuildFlatContour(gBSplineCurve);
			break;
		case Qt::Key_N:
			gDeviation *= 2;
			BuildFlatContour(gBSplineCurve);
			break;
		case Qt::Key_Space:
			gIntersectionRay.SetOrigin(GPoint2(5.5, 5.5));
			randomDir[G_X] = GMath::RangeRandom(-1.0, 1.0);
			randomDir[G_Y] = GMath::RangeRandom(-1.0, 1.0);
			gIntersectionRay.SetDirection(randomDir);
			gIntersectionRay.Normalize();
			gIntersectionPoints.clear();
			b = gBSplineCurve->IntersectRay(gIntersectionRay, gIntersectionPoints);
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
