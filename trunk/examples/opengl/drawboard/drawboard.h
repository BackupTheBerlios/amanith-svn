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

#ifndef DRAW_BOARD_H
#define DRAW_BOARD_H

#include <amanith/gkernel.h>
#include <amanith/2d/gpixelmap.h>
#include <amanith/rendering/gopenglboard.h>
#include <qgl.h>

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GOpenGLBoard *gDrawBoard;
	GPixelMap *gImage;
	GGradientDesc *gLinGrad1, *gLinGrad2, *gLinGrad3;
	GGradientDesc *gRadGrad1, *gRadGrad2, *gRadGrad3, *gRadGrad4;
	GGradientDesc *gConGrad1, *gConGrad2, *gConGrad3, *gConGrad4;
	GPatternDesc *gPattern;
	GPatternDesc *gBackGround;
	GString gDataPath;

	// 0 = color
	// 1 = linear gradient
	// 2 = radial gradient (in)
	// 3 = radial gradient (out)
	// 4 = pattern
	// 5 = stroking
	GUInt32 gTestSuite;
	GUInt32 gTestIndex;
	GBool gDrawBackGround;
	GReal gRandAngle;
	GReal gRandScale;
	GRenderingQuality gRenderingQuality;
	GBool gUseShaders;

//	GLfloat	gX, gY, gZ;						// Depth Into The Screen
	GPath2D *gPath;

protected:
	void initializeGL();					// implementation for QGLWidget.initializeGL()
    void paintGL();							// implementation for QGLWidget.paintGL()
	void resizeGL(int width, int height);	// implementation for QGLWidget.resizeGL()
	void keyPressEvent(QKeyEvent *e);		// keyboard event handler

	void TestColor(const GUInt32 TestIndex);
	void TestLinearGradient(const GUInt32 TestIndex, const GReal RotAngle, const GReal Scale);
	void TestRadialGradientIn(const GUInt32 TestIndex, const GReal RotAngle, const GReal Scale);
	void TestRadialGradientOut(const GUInt32 TestIndex, const GReal RotAngle, const GReal Scale);
	void TestConicalGradientIn(const GUInt32 TestIndex, const GReal RotAngle, const GReal Scale);
	void TestConicalGradientOut(const GUInt32 TestIndex, const GReal RotAngle, const GReal Scale);
	void TestPattern(const GUInt32 TestIndex, const GReal RotAngle, const GReal Scale);
	void TestStroke(const GUInt32 TestIndex);
	void TestMasks(const GUInt32 TestIndex);

	void TestDebug();

public:
	// constructor
	QGLWidgetTest(QWidget *parent = 0);
	// destructor
	~QGLWidgetTest();
	//void timerEvent(QTimerEvent* e);
};

#endif
