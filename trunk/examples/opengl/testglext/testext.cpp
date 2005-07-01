#include <amanith/support/gutilities.h>
#include <qmessagebox.h>
#include "testext.h"

// constructor
QGLWidgetTest::QGLWidgetTest(QWidget * parent, const GChar8 *name) : QGLWidget(parent, name) {
}

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gExtManager)
		delete gExtManager;
}


// implementation for QGLWidget.initializeGL()
void QGLWidgetTest::initializeGL() {

	// create extensions manager
	gExtManager = new GOpenglExt();

	glClear(GL_COLOR_BUFFER_BIT);

	GUInt32 numFunc = gExtManager->SupportedFunctions().size();
	G_DEBUG("Number of GL functions supported over OpenGL 1.1 standard: " + StrUtils::ToString(numFunc));

	G_DEBUG("TextureUnitsCount: " + StrUtils::ToString(gExtManager->TextureUnitsCount()));
        
	if (gExtManager->IsFunctionSupported("glcopYTexSubImage3D"))
		G_DEBUG("glcopYTexSubImage3D is supported");
	else
		G_DEBUG("glcopYTexSubImage3D isn't supported");

	if (gExtManager->IsGLVersionSupported("1.3"))
		G_DEBUG("OpenGL 1.3 is supported");
	else
		G_DEBUG("OpenGL 1.3 isn't supported");

	if (gExtManager->IsGLVersionSupported(1, 3))
		G_DEBUG("OpenGL (1, 3) is supported");
	else
		G_DEBUG("OpenGL (1, 3) isn't supported");

	if (gExtManager->IsGLVersionSupported(2, 0))
		G_DEBUG("OpenGL (2, 0) is supported");
	else
		G_DEBUG("OpenGL (2, 0) isn't supported");

	if (gExtManager->IsGLVersionSupported("1.X"))
		G_DEBUG("OpenGL 1.X is supported");
	else
		G_DEBUG("OpenGL 1.X isn't supported");

	if (gExtManager->IsGLVersionSupported("1.*"))
		G_DEBUG("OpenGL 1.* is supported");
	else
		G_DEBUG("OpenGL 1.* isn't supported");

	if (gExtManager->IsMultitextureSupported())
		G_DEBUG("Multitexture is supported");
	else
		G_DEBUG("Multitexture isn't supported");

	if (gExtManager->IsCubemapSupported())
		G_DEBUG("Cubemap is supported");
	else
		G_DEBUG("Cubemap isn't supported");

	if (gExtManager->IsArbProgramsSupported())
		G_DEBUG("ArbPrograms are supported");
	else
		G_DEBUG("ArbPrograms aren't supported");

	if (gExtManager->IsArbShadersSupported())
		G_DEBUG("ArbShaders are supported");
	else
		G_DEBUG("ArbShaders aren't supported");

	if (gExtManager->IsOcclusionQuerySupported())
		G_DEBUG("OcclusionQuery is supported");
	else
		G_DEBUG("OcclusionQuery isn't supported");
}

// keyboard event handler
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	GString s;
	GUInt32 numFunc = gExtManager->SupportedFunctions().size();

	switch(e->key()) {
		case Qt::Key_F1:
			s = "Number of GL functions supported over OpenGL 1.1 standard: " + StrUtils::ToString(numFunc) + "\n\n";
			s += "TextureUnitsCount: " + StrUtils::ToString(gExtManager->TextureUnitsCount()) + "\n";
			if (gExtManager->IsFunctionSupported("glcopYTexSubImage3D"))
				s += "glcopYTexSubImage3D is supported\n";
			else
				s += "glcopYTexSubImage3D isn't supported\n";

			if (gExtManager->IsGLVersionSupported("1.3"))
				s += "OpenGL 1.3 is supported\n";
			else
				s += "OpenGL 1.3 isn't supported\n";


			if (gExtManager->IsGLVersionSupported(1, 1))
				s += "OpenGL (1, 1) is supported\n";
			else
				s += "OpenGL (1, 1) isn't supported\n";

			if (gExtManager->IsGLVersionSupported(1, 2))
				s += "OpenGL (1, 2) is supported\n";
			else
				s += "OpenGL (1, 2) isn't supported\n";

			if (gExtManager->IsGLVersionSupported(1, 3))
				s += "OpenGL (1, 3) is supported\n";
			else
				s += "OpenGL (1, 3) isn't supported\n";

			if (gExtManager->IsGLVersionSupported(1, 4))
				s += "OpenGL (1, 4) is supported\n";
			else
				s += "OpenGL (1, 4) isn't supported\n";

			if (gExtManager->IsGLVersionSupported(1, 5))
				s += "OpenGL (1, 5) is supported\n";
			else
				s += "OpenGL (1, 5) isn't supported\n";

			if (gExtManager->IsGLVersionSupported("1.X"))
				s += "OpenGL 1.X is supported\n";
			else
				s += "OpenGL 1.X isn't supported\n";

			if (gExtManager->IsGLVersionSupported("1.*"))
				s += "OpenGL 1.* is supported\n";
			else
				s += "OpenGL 1.* isn't supported\n";

			if (gExtManager->IsMultitextureSupported())
				s += "Multitexture is supported\n";
			else
				s += "Multitexture isn't supported\n";

			if (gExtManager->IsCubemapSupported())
				s += "Cubemap is supported\n";
			else
				s += "Cubemap isn't supported\n";

			if (gExtManager->IsArbProgramsSupported())
				s += "ArbPrograms are supported\n";
			else
				s += "ArbPrograms aren't supported\n";

			if (gExtManager->IsArbShadersSupported())
				s += "ArbShaders are supported\n";
			else
				s += "ArbShaders aren't supported\n";

			if (gExtManager->IsOcclusionQuerySupported())
				s += "OcclusionQuery is supported\n";
			else
				s += "OcclusionQuery isn't supported\n";

			QMessageBox::information(this, "OpenGL Information", StrUtils::ToAscii(s));
			break;
	}
}

// implementation for QGLWidget.paintGL()
void QGLWidgetTest::paintGL() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFlush();
}

// implementation for QGLWidget.resizeGL()
void QGLWidgetTest::resizeGL(int width, int height) {

	glViewport(0, 0, width, height);					// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	
}
