#ifndef TESTFONTS_H
#define TESTFONTS_H

#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#include <amanith/2d/gmesh2d.h>
#include <qgl.h>

using namespace Amanith;


class QGLWidgetTest;



class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GOpenglExt *gExtManager;	// extensions manager
	GMesh2D *gMesh;

	GString gDataPath;

	GLfloat	z, gX, gY;					// Depth Into The Screen

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void DrawMesh(GMesh2D* Mesh);
	void DrawMeshVertex(GMesh2D* Mesh);
	void DrawMeshFace(GMeshFace2D *Face);
	void DrawMeshFaces(GMesh2D* Mesh);
	void BuildNewMesh();

protected:
	void initializeGL();					// implementation for QGLWidget.initializeGL()
    void paintGL();							// implementation for QGLWidget.paintGL()
	void resizeGL(int width, int height);	// implementation for QGLWidget.resizeGL()
	void keyPressEvent(QKeyEvent *e);		// keyboard event handler

public:
	// constructor
	QGLWidgetTest(QWidget *parent = 0);
	// destructor
	~QGLWidgetTest();
	void timerEvent(QTimerEvent* e);
};

#endif
