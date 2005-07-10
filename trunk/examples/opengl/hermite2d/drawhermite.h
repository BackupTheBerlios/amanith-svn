#ifndef DRAW_HERMITE_H
#define DRAW_HERMITE_H

#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#include <amanith/2d/ghermitecurve2d.h>
#include <qgl.h>

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GOpenglExt *gExtManager;	// extensions manager
	GHermiteCurve2D *gHermCurve;
	GReal gDeviation;
	GLfloat	gX, gY, gZ;					// Depth Into The Screen
	GDynArray<GPoint2> gVertices;
	GDynArray<GVector2> gIntersectionPoints;
	GRay2 gIntersectionRay;

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void BuildFlatContour(const GHermiteCurve2D* Curve);
	void DrawHermite(const GHermiteCurve2D* Curve);

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
