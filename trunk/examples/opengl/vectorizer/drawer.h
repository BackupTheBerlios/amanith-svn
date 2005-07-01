#ifndef IMAGE_DRAWER_H
#define IMAGE_DRAWER_H

#include <amanith/gkernel.h>
#include <amanith/2d/gpixelmap.h>
#include <amanith/2d/gtracer2d.h>
#include <amanith/gopenglext.h>

#include <qscrollview.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qgl.h>

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GString gDataPath;
	GKernel *gKernel;
	GPixelMap *gPixMap1;
	GOpenglExt *gExtManager;	// extensions manager
	GReal gDeviation;
	GDynArray<GTracedContour> gPaths;
	GDynArray<GPoint2> gVertices;
	GDynArray<GInt32> gIndex;
	GDynArray<GPoint2> gTriangles;
	GBool gWireFrame;
	GBool gFillDraw;
	GReal gX, gY, gZ;

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void FlattenContours();

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
	void QGLWidgetTest::timerEvent(QTimerEvent *e);
};


#endif
