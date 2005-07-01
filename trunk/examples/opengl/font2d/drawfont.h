#ifndef TESTFONTS_H
#define TESTFONTS_H

#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#include <amanith/2d/gfont2d.h>
#include <qgl.h>

using namespace Amanith;


class QGLWidgetTest;


class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GOpenglExt *gExtManager;	// extensions manager
	GFont2D *gFont;
	GString gDataPath;

	GDynArray<GPoint2> gVertices;
	GDynArray<GPoint2> gContoursPoints;
	GDynArray<GInt32> gContoursIndexes;

	const GFontChar2D *gChar;
	GUInt32 gCurrentChar;
	GBool gFillDraw;
	GBool gWireFrame;

	GReal gDeviation;
	GLfloat	gX, gY, gZ;								// Depth Into The Screen

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void DrawTriangles(const GDynArray<GPoint2>& Points);
	void DrawOutlines(const GDynArray<GPoint2>& ContoursPoints, const GDynArray<GInt32>& ContoursIndexes);
	void GenerateTessellation(const GFontChar2D* Char, const GReal Deviation);

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
