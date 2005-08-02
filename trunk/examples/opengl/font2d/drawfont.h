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
	GFont2D *gFont;
	GString gDataPath;

	GDynArray< GPoint<GDouble, 2> > gVertices;
	GDynArray<GPoint2> gContoursPoints;
	GDynArray<GInt32> gContoursIndexes;
	const GFontChar2D *gChar;						// font character to be drawed
	GUInt32 gCurrentChar;
	GBool gFillDraw;
	GBool gWireFrame;
	GReal gDeviation;
	GLfloat	gX, gY, gZ;								// Depth Into The Screen

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void DrawTriangles(const GDynArray< GPoint<GDouble, 2> >& Points);
	void DrawOutlines(const GDynArray<GPoint2>& ContoursPoints, const GDynArray<GInt32>& ContoursIndexes);
	void GenerateTessellation(const GFontChar2D* Char, const GReal Deviation);
	void DrawChar(const GFontChar2D* Char, const GReal Deviation, const GMatrix33& Transformation);

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
