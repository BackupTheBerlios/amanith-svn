#ifndef DRAW_POLYLINE_H
#define DRAW_POLYLINE_H

#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#include <amanith/2d/gpolylinecurve2d.h>
#include <qgl.h>

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GOpenglExt *gExtManager;	// extensions manager
	GPolyLineCurve2D *gPolyCurve;
	GReal gDeviation;
	GDynArray<GVector2> gIntersectionPoints;
	GRay2 gIntersectionRay;
	GLfloat	gX, gY, gZ;					// Depth Into The Screen

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void DrawPolyLine(const GPolyLineCurve2D* Curve);

protected:
	void initializeGL();					// implementation for QGLWidget.initializeGL()
    void paintGL();							// implementation for QGLWidget.paintGL()
	void resizeGL(int width, int height);	// implementation for QGLWidget.resizeGL()
	void keyPressEvent(QKeyEvent *e);		// keyboard event handler

public:
	// constructor
	QGLWidgetTest(QWidget *parent = 0, const char *name = 0);
	// destructor
	~QGLWidgetTest();
	void timerEvent(QTimerEvent* e);
};

#endif
