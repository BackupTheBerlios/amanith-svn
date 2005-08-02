/****************************************************************************
**
** Copyright (C) 2004-2005 Mazatech Inc. All rights reserved.
**
** This file is part of Amanith Framework.
**
** This file may be distributed and/or modified under the terms of the Q Public License
** as defined by Mazatech Inc. of Italy and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Amanith Professional Edition license may use this file in
** accordance with the Amanith Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.mazatech.com or email sales@mazatech.com for
** information about Amanith Commercial License Agreements.
** See http://www.amanith.org/ for opensource version, public forums and news.
**
** Contact info@mazatech.com if any conditions of this licensing are
** not clear to you.
**********************************************************************/

#include <amanith/support/gutilities.h>
#include <qmessagebox.h>
#include "testext.h"

// QT4 support
#ifdef USE_QT4
	#include <QKeyEvent>
#endif

// constructor
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(parent) {
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
				s += "glCopyTexSubImage3D is supported\n";
			else
				s += "glCopyTexSubImage3D isn't supported\n";

			if (gExtManager->IsGLVersionSupported("1.3"))
				s += "OpenGL \"1.3\" is supported\n";
			else
				s += "OpenGL \"1.3\" isn't supported\n";


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
				s += "OpenGL \"1.X\" is supported\n";
			else
				s += "OpenGL \"1.X\" isn't supported\n";

			if (gExtManager->IsGLVersionSupported("1.*"))
				s += "OpenGL \"1.*\" is supported\n";
			else
				s += "OpenGL \"1.*\" isn't supported\n";

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
