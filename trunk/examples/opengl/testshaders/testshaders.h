/****************************************************************************
**
** Copyright (C) 2004-2006 Mazatech Inc. All rights reserved.
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

#ifndef TESTSHADERS_H
#define TESTSHADERS_H

#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#ifdef USE_QT4
#include <QGLWidget>
#else
#include <qgl.h>
#endif

using namespace Amanith;

class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	GOpenglExt *gExtManager;	// extensions manager
	GString gDataPath;

	GInt32 vis;					// Test to show (1-8)
	GLuint textures[12];		// Textures
	GLuint cubeTex;				// Cubemap
	GLhandleARB shader;
	GBool light;				// Lighting ON/OFF
	GBool anim;					// Animation ON/OFF
	GBool msgShown;				// Information message shown
	GLfloat	xrot;				// X Rotation
	GLfloat	yrot;				// Y Rotation
	GLfloat xspeed;				// X Rotation Speed
	GLfloat yspeed;				// Y Rotation Speed
	GLfloat	z;					// Depth Into The Screen
	GLfloat z_light;			// Light z
	GLfloat lightAmbient[4];	// Ambient Light Values
	GLfloat lightDiffuse[4];	// Diffuse Light Values
	GLfloat lightPosition[4];	// Light Position

	void drawColorBlendingCube();				// Display a cube with vertex colors
	void drawSimpleTexturedCube();				// Display a cube with a texture
	void drawBlendingTexturedCube();			// Display a cube with transparent texture
	void drawMultiTexturedCube();				// Display a cube with 2 color textures and 1 mask
	void drawMultiTexturedCube2();				// Display a cube with 2 color textures using 3 texture units
	void drawReflectCube();						// Display a cube with spherical EnvMapping
	void drawReflectCube2();					// Display a cube with CUBE_MAP
	void drawCubeWithProgShader();				// Display a cube with programming shaders
	GLuint loadTexture(const GChar8 *fileName, const GChar8 *Options = NULL);	// Load a texture and return ID
	void createCubeTex();						// Create a cubemap
	GLhandleARB useShader(const GChar8 *vShader, const GChar8 *fShader); // Load shaders passed as param

	// Draw a cube:
	// vertNorm = true, normals are per vertex else per face
	// uvCoord = true, create uv coords
	// numMultiText, number of texture units for whose gen coords (max = 3) 
	// vertCol, array of colors for vertex
	void drawCube(GBool vertNorm = true, GBool uvCoord = false, GUInt8 numMultiText = 0, GDouble vertCol[8][4] = NULL);
	void SetTextureVertex(const GLint textureIndex, const GLfloat u, const GLfloat v);

	void activeTextureUnit(GLint unit);				// Turn on the texture unit
	void deactiveTextureUnit(GLint unit);			// Turn off the texture unit
	void setDefaultTextureUnitStates(GLint unit);	// Set initial texture unit state
	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states

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
