#ifndef DRAW_BSPLINE_H
#define DRAW_BSPLINE_H

#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#include <amanith/2d/gbsplinecurve2d.h>
#include <qgl.h>

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GOpenglExt *gExtManager;	// extensions manager
	GBSplineCurve2D *gBSplineCurve;
	GReal gDeviation;
	GDynArray<GPoint2> gVertices;
	GDynArray<GPoint2> gInterpolationPoints;

	GLfloat	gX, gY, gZ;								// Depth Into The Screen
	GDynArray<GVector2> gIntersectionPoints;
	GRay2 gIntersectionRay;

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void BuildFlatContour(const GBSplineCurve2D* Curve);
	void Draw(const GBSplineCurve2D* Curve);

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
