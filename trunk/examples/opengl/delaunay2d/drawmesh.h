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

#ifndef DELAUNAY2D_H
#define DELAUNAY2D_H

#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#include <amanith/2d/gmesh2d.h>
#ifdef USE_QT4
#include <QGLWidget>
#else
#include <qgl.h>
#endif

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:

	GMesh2D<GReal> *gMesh;
	GLfloat	gZ, gX, gY;								// Depth Into The Screen

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void DrawMesh(GMesh2D<GReal>* Mesh);
	void DrawMeshVertex(GMesh2D<GReal>* Mesh);
	void DrawMeshFace(GMeshFace2D<GReal> *Face);
	void DrawMeshFaces(GMesh2D<GReal>* Mesh);
	void BuildNewMesh();

protected:
	void initializeGL();					// implementation for QGLWidget.initializeGL()
    void paintGL();							// implementation for QGLWidget.paintGL()
	void resizeGL(int width, int height);	// implementation for QGLWidget.resizeGL()
	void keyPressEvent(QKeyEvent *e);		// keyboard event handler

public:
	// constructor
#ifdef USE_QT4
	QGLWidgetTest(const QGLFormat& Format, QWidget *parent = 0);
#else
	QGLWidgetTest(QWidget *parent = 0);
#endif
	// destructor
	~QGLWidgetTest();
	void timerEvent(QTimerEvent* e);
};

#endif
