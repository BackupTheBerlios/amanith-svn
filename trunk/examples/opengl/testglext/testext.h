#ifndef TESTEXT_H
#define TESTEXT_H

#include <amanith/gopenglext.h>
#include <qgl.h>

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GOpenglExt *gExtManager;				// extensions manager

protected:
	void initializeGL();					// implementation for QGLWidget.initializeGL()
    void paintGL();							// implementation for QGLWidget.paintGL()
	void resizeGL(int width, int height);	// implementation for QGLWidget.resizeGL()
	void keyPressEvent(QKeyEvent *e);		// keyboard event handler

public:
	// constructor
	QGLWidgetTest(QWidget * parent = 0, const GChar8 *name = 0);
	// destructor
	~QGLWidgetTest();
};

#endif
