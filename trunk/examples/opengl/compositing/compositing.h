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

#ifndef COMPOSITING_H
#define COMPOSITING_H

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
	GGradientDesc *gLinGrad;
	GPatternDesc *gBackGround;
	GString gDataPath;

	GReal gRotAngle;
	GReal gRotationVel;
	GReal gStrokeOpacity;
	GReal gFillOpacity;
	GReal gZoomFactor;
	GPoint2 gTranslation;
	GMatrix33 gModelView;
	GMatrix33 gGradientMatrix;
	GBool gAnim;
	GCompositingOperation gStrokeCompOp;
	GCompositingOperation gFillCompOp;

protected:
	void initializeGL();					// implementation for QGLWidget.initializeGL()
    void paintGL();							// implementation for QGLWidget.paintGL()
	void resizeGL(int width, int height);	// implementation for QGLWidget.resizeGL()
	void keyPressEvent(QKeyEvent *e);		// keyboard event handler
	void mousePressEvent(QMouseEvent * e);	// mouse press event handler
	void mouseMoveEvent(QMouseEvent * e);	// mouse move event handler
	void wheelEvent(QWheelEvent * e);		// mouse wheel event handler

	void BuildMatrices();
	void DrawTitle();
	GString CompOpToString(const GCompositingOperation CompOp);

public:
	// constructor
	QGLWidgetTest(const QGLFormat& Format, QWidget *parent = 0);
	// destructor
	~QGLWidgetTest();
	// timer event
	void timerEvent(QTimerEvent* e);
};

#endif
