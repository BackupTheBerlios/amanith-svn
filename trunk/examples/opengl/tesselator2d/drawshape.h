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

#ifndef DRAWSHAPE_H
#define DRAWSHAPE_H

#include <amanith/gkernel.h>
#include <amanith/2d/gpixelmap.h>
#include <amanith/gopenglext.h>
#include <qgl.h>

using namespace Amanith;

class QGLWidgetTest;

class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GPixelMap *gTexture;
	GDynArray< GPoint<GDouble, 2> > gTrianglesPts;
	GDynArray<GULong> gTrianglesIdx;
	GDynArray<GPoint2> gContoursPoints;
	GDynArray<GInt32> gContoursIndexes;

	GString gDataPath;
	GBool gWireFrame;
	GBool gAnim;
	GUInt32 gFillRule;
	GUInt32 gFillMode;
	GLfloat	gX, gY, gZ;								// Depth Into The Screen
	GFloat gAng1, gAng2, gAng3;						// Rotation angles
	GFloat gStepAng1, gStepAng2, gStepAng3;			// Rotation steps

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void DrawTriangles(const GDynArray< GPoint<GDouble, 2> >& Points, const GDynArray<GULong>& Indexes);
	void BuildShape(const GFloat RadAngle1, const GFloat RadAngle2, const GFloat RadAngle3);
	void GenerateTessellation();

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
