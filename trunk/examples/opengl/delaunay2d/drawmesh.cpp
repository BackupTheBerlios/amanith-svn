#include "drawmesh.h"
#include <amanith/geometry/gxform.h>
#include <amanith/geometry/garea.h>
#include <amanith/geometry/gintersect.h>
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
#endif

#define PrintOpenGLError() gExtManager->PrintOglError(__FILE__, __LINE__)

static int timer_interval = 0;			// timer interval (millisec)


void QGLWidgetTest::BuildNewMesh() {

	GDynArray<GPoint2> pts;
	GPoint2 p;

	for (GUInt32 i = 0; i < 250; i++) {
		p[G_X] = GMath::RangeRandom(-8.0f, 8.0f);
		p[G_Y] = GMath::RangeRandom(-8.0f, 8.0f);
		pts.push_back(p);
	}
	gMesh->BuildFromPointsCloud(pts);
}

// constructor
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(parent) {

	gKernel = new GKernel();
	gMesh = (GMesh2D *)gKernel->CreateNew(G_MESH2D_CLASSID);

	GMath::SeedRandom();

	GDynArray<GPoint2> pts;
	GDynArray<GIndexList> idx;
	GIndexList list;

	BuildNewMesh();

	// Depth Into The Screen
	z = -25.0f;
	gX = -0.0f;
	gY = -0.0f;

	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gExtManager)
		delete gExtManager;
	if (gMesh)
		delete gMesh;
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
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);				// Set Line Antialiasing
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	setDefaultGlobalStates();
	startTimer(timer_interval);
}
//------------------------------------------------------------

void QGLWidgetTest::DrawMeshVertex(GMesh2D* Mesh) {

	GUInt32 i, j = Mesh->VerticesCount();
	GPoint2 p;

	glPointSize(4.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_POINTS);
	for (i = 0; i < j; i++) {
		p = Mesh->Vertex(i)->Position();
		glVertex3f(p[G_X], p[G_Y], 1.0f);
	}
	glEnd();
}

void QGLWidgetTest::DrawMeshFace(GMeshFace2D *Face) {

	if (!Face)
		return;

	GMeshFaceEdgeIterator2D it(Face);
	GMeshEdge2D *e = it.Next();
	GPoint2 o, d;

	while (e) {
		if (e->Org()->CustomData() && e->Dest()->CustomData())
			glColor3f(1.0f, 0.0f, 0.0f);
		else
			glColor3f(1.0f, 1.0f, 1.0f);
		o = e->Org()->Position();
		d = e->Dest()->Position();
		glVertex3f(o[G_X], o[G_Y], 1.0f);
		glVertex3f(d[G_X], d[G_Y], 1.0f);
		e = it.Next();
	}
}

void QGLWidgetTest::DrawMeshFaces(GMesh2D* Mesh) {

	GMeshFace2D *f;
	GUInt32 i, j = Mesh->FacesCount();
	GPoint2 p;

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	for (i = 0; i < j; i++) {
		f = Mesh->Face(i);
		DrawMeshFace(f);
	}
	glEnd();
}

void QGLWidgetTest::DrawMesh(GMesh2D* Mesh) {

	if (!Mesh)
		return;

	DrawMeshFaces(Mesh);
	DrawMeshVertex(Mesh);
	setDefaultGlobalStates();
}


//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setLightAndTransform();
	DrawMesh(gMesh);
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
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	
}
//------------------------------------------------------------

// keyboard event handler
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	switch(e->key()) {
		case Qt::Key_A:
			z -= 0.5f;
			break;
		case Qt::Key_Z:
			z += 0.5f;
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
		case Qt::Key_F1:
			QMessageBox::information(this, "Command keys", "A/Z: Zoom +/-\nArrow keys: Move viewport\nSpace: Build a new mesh");
			break;
		case Qt::Key_Space:
			BuildNewMesh();
			break;
	}
}
//------------------------------------------------------------
void QGLWidgetTest::setLightAndTransform() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(gX, gY, z);
}
//------------------------------------------------------------

//------------------------------------------------------------
void QGLWidgetTest::setDefaultGlobalStates() {

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
//------------------------------------------------------------
