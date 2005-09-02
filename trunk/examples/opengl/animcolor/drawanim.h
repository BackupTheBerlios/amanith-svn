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

#ifndef DRAW_BEZIER_H
#define DRAW_BEZIER_H

#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#include <amanith/gproperty.h>
#include <qgl.h>

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;

	GHermiteProperty1D *gRedAnimH;
	GHermiteProperty1D *gGreenAnimH;
	GHermiteProperty1D *gBlueAnimH;
	GLinearProperty1D *gRedAnimL;
	GLinearProperty1D *gGreenAnimL;
	GLinearProperty1D *gBlueAnimL;
	GConstantProperty1D *gRedAnimC;
	GConstantProperty1D *gGreenAnimC;
	GConstantProperty1D *gBlueAnimC;

	GUInt32 gInterpolationIndex;
	GOORType gOORType;
	GTimeValue gCurrentTime;
	GTimeValue gTimeStep;
	GBool gDrawPalette;

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states
	void BuildNewPalette(const GUInt32 KeysNum);
	void AddKey();
	void RemoveKey();
	void SetOORType(const GOORType NewOORType);
	void DrawPalette();
	void DrawCursor(const GTimeValue TimePos, const GBool On);

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
