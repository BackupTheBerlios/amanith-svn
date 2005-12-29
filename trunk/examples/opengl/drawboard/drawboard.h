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
#ifdef USE_QT4
#include <QGLWidget>
#else
#include <qgl.h>
#endif

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GOpenGLBoard *gDrawBoard;
	GPixelMap *gImage;
	GGradientDesc *gLinGrad1, *gLinGrad2, *gLinGrad3, *gLinGradLogo1, *gLinGradLogo2, *gLinGradLogo3;
	GGradientDesc *gRadGrad1, *gRadGrad2, *gRadGrad3, *gRadGrad4;
	GGradientDesc *gConGrad1, *gConGrad2, *gConGrad3, *gConGrad4;
	GPatternDesc *gPattern, *gBackGround;
	GCacheBank *gCacheBank;
	GString gDataPath;
	GString gScreenShotFileName;

	// 0 = color
	// 1 = linear gradient
	// 2 = radial gradient (in)
	// 3 = radial gradient (out)
	// 4 = conical gradient (in)
	// 5 = conical gradient (out)
	// 6 = pattern
	// 7 = stroking
	// 8 = masks and group opacity
	// 9 = shapes
	GUInt32 gTestSuite;
	GUInt32 gTestIndex;
	GBool gDrawBackGround;
	GReal gRandAngle;
	GReal gRandScaleX;
	GReal gRandScaleY;
	GRenderingQuality gRenderingQuality;
	GBool gUseShaders;

protected:
	void initializeGL();					// implementation for QGLWidget.initializeGL()
    void paintGL();							// implementation for QGLWidget.paintGL()
	void resizeGL(int width, int height);	// implementation for QGLWidget.resizeGL()
	void keyPressEvent(QKeyEvent *e);		// keyboard event handler

	void TestColor(const GUInt32 TestIndex);
	void TestLinearGradient(const GUInt32 TestIndex, const GReal RotAngle, const GReal Scale);
	void TestRadialGradientIn(const GUInt32 TestIndex, const GReal RotAngle, const GReal ScaleX, const GReal ScaleY);
	void TestRadialGradientOut(const GUInt32 TestIndex, const GReal RotAngle, const GReal ScaleX, const GReal ScaleY);
	void TestConicalGradientIn(const GUInt32 TestIndex, const GReal RotAngle, const GReal ScaleX, const GReal ScaleY);
	void TestConicalGradientOut(const GUInt32 TestIndex, const GReal RotAngle, const GReal ScaleX, const GReal ScaleY);
	void TestPattern(const GUInt32 TestIndex, const GReal RotAngle, const GReal ScaleX, const GReal ScaleY);
	void TestStroke(const GUInt32 TestIndex);
	void TestMasks(const GUInt32 TestIndex);
	void TestGeometries(const GUInt32 TestIndex);
	void TestCache(const GUInt32 TestIndex);

public:
	// constructor
#ifdef USE_QT4
	QGLWidgetTest(const QGLFormat& Format, QWidget *parent = 0);
#else
	QGLWidgetTest(QWidget *parent = 0);
#endif
	// destructor
	~QGLWidgetTest();
	//void timerEvent(QTimerEvent* e);
};

#endif
