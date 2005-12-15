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

#include "testshaders.h"
#include <amanith/2d/gpixelmap.h>
#include <qapplication.h>
#include <qfile.h>
#include <qmessagebox.h>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QTextStream>
	#include <QKeyEvent>
#endif

#define PrintOpenGLError() gExtManager->PrintOglError(__FILE__, __LINE__)

static int timer_interval = 0;			// timer interval (millisec)

// constructor
#ifdef USE_QT4
QGLWidgetTest::QGLWidgetTest(const QGLFormat& Format, QWidget *parent) : QGLWidget(Format, parent) {
#else
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(parent) {
#endif

	GString s;
	gKernel = new GKernel();

	// Light Ambient init values
	lightAmbient[0] = 0.5f;
	lightAmbient[1] = 0.5f;
	lightAmbient[2] = 0.5f;
	lightAmbient[3] = 1.0f;
	// Light Diffuse init values
	lightDiffuse[0] = 1.0f;
	lightDiffuse[1] = 1.0f;
	lightDiffuse[2] = 1.0f;
	lightDiffuse[3] = 1.0f;
	// Light Position init values
	lightPosition[0] = 0.0f;
	lightPosition[1] = 0.0f;
	lightPosition[2] = 2.0f;
	lightPosition[3] = 1.0f;
	// Depth Into The Screen
	z = -5.0f;
	// Rotation init values
	xrot = 0.0f;
	yrot = 0.0f;
	// Rotation speed init values
	xspeed = 0.0f;
	yspeed = 0.0f;
	// Light ON
	light = true;
	// z light coord init
	z_light = 2.0f;
	// we start with shader number 1
	vis = 1;
	msgShown = G_FALSE;
	anim = G_TRUE;
	startTimer(timer_interval);
	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";
}
//------------------------------------------------------------

// destructor
QGLWidgetTest::~QGLWidgetTest() {

	if (gExtManager)
		delete gExtManager;
	if (gKernel)
		delete gKernel;
}

//------------------------------------------------------------

void QGLWidgetTest::timerEvent(QTimerEvent *e) {
	if (!e)
		return;
	if (anim && !msgShown) {
		xrot += 0.7f;
		yrot -= 0.5f;
		updateGL();
	}
}

//----- initializeGL -----------------------------------------
void QGLWidgetTest::initializeGL() {

	GString fName;
	// create extensions manager
	gExtManager = new GOpenglExt();

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);		// Setup The Ambient Light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);		// Setup The Diffuse Light
	glLightfv(GL_LIGHT1, GL_POSITION,lightPosition);	// Position The Light
	glEnable(GL_LIGHT1);
	// lets load textures
	fName = gDataPath + GString("spiral.png");
	textures[0] = loadTexture(StrUtils::ToAscii(fName));			// simple texture

	fName = gDataPath + GString("electrica.png");
	textures[1] = loadTexture(StrUtils::ToAscii(fName), "expandpalette=true");	// color image

	fName = gDataPath + GString("mask2.png");
	textures[2] = loadTexture(StrUtils::ToAscii(fName));			// grey alpha mask

	fName = gDataPath + GString("image2_plusmask.png");
	textures[3] = loadTexture(StrUtils::ToAscii(fName)); // image with alpha mask

	fName = gDataPath + GString("sumu.png");
	textures[4] = loadTexture(StrUtils::ToAscii(fName));			// NeHe logo

	fName = gDataPath + GString("chromic.png");
	textures[5] = loadTexture(StrUtils::ToAscii(fName));			// simple metal envmap

	// we create cubemap, if supported
	if (gExtManager->IsCubemapSupported())
		createCubeTex();
	// we create GLSL shaders, if supported
	if (gExtManager->IsArbShadersSupported()) {
		GString vs, fs;
		vs = gDataPath + GString("multitexture.vert");
		fs = gDataPath + GString("multitexture.frag");
		shader = useShader(StrUtils::ToAscii(vs), StrUtils::ToAscii(fs));
	}

	setDefaultGlobalStates();
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE1_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE2_ARB);
}
//------------------------------------------------------------

//----- paintGL ----------------------------------------------
void QGLWidgetTest::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (vis == 1)
		this->drawColorBlendingCube();
	else
	if (vis == 2)
		this->drawSimpleTexturedCube();
	else
	if (vis == 3)
		this->drawBlendingTexturedCube();
	else
	if (vis == 4)
		this->drawMultiTexturedCube();
	else
	if (vis == 5)
		this->drawMultiTexturedCube2();
	else
	if (vis == 6)
		this->drawReflectCube();
	else
	if (vis == 7)
		this->drawReflectCube2();
	else
	if (vis == 8)
		this->drawCubeWithProgShader();
	glFlush();
}
//------------------------------------------------------------

//----- resizeGL ---------------------------------------------
void QGLWidgetTest::resizeGL(int width, int height) {

	if (height == 0)									// Prevent A Divide By Zero By
		height = 1;										// Making Height Equal One
	glViewport(0, 0, width, height);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	
}
//------------------------------------------------------------

// keyboard event handler
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	QString s;

	switch(e->key()) {
		case Qt::Key_L:
			if (light)
				light = G_FALSE;
			else
				light = G_TRUE;
			break;
		case Qt::Key_A: z += 1.0f;
			break;
		case Qt::Key_Z:	z -= 1.0f;
			break;
		case Qt::Key_Space:	anim = !anim;
			break;
		case Qt::Key_1:	vis = 1;
			break;
		case Qt::Key_2:	vis = 2;
			break;
		case Qt::Key_3:	vis = 3;
			break;
		case Qt::Key_4:	vis = 4;
			break;
		case Qt::Key_5:	vis = 5;
			break;
		case Qt::Key_6:	vis = 6;
			break;
		case Qt::Key_7:	vis = 7;
			break;
		case Qt::Key_8:	vis = 8;
			break;
		case Qt::Key_F1:
			s = "1..8: Toggle shader\n";
			s += "Space: Toggle animation\n";
			s += "A/Z: Zoom +/-\n";
			s += "L: Toggle light";
			QMessageBox::information(this, "Command keys", s);
			break;
	}
	if (xrot > 360)
		xrot -= 360;
	else
	if (xrot < 0)
		xrot += 360;
	if (yrot > 360)
		yrot -= 360;
	else
	if (yrot < 0)
		yrot += 360;
	updateGL();
}
//------------------------------------------------------------

// create a texture and return its ID
GLuint QGLWidgetTest::loadTexture(const GChar8 *fileName, const GChar8 *Options) {

	GError err;
	GLuint texture[1];
	GPixelMap *imgTexture;

	imgTexture = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	if (!imgTexture)
		return 0;

	err = imgTexture->Load(fileName, Options);

	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgTexture->Width(), imgTexture->Height(),
		0, GL_BGRA, GL_UNSIGNED_BYTE, imgTexture->Pixels());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete imgTexture;
	return texture[0];
}
//------------------------------------------------------------

// create cubemap, and place it into textures slot [6]..[11]
void QGLWidgetTest::createCubeTex() {


	GPixelMap *img1 = NULL, *img2 = NULL, *img3 = NULL, *img4 = NULL, *img5 = NULL, *img6 = NULL;
	GError err1, err2, err3, err4, err5, err6;
	GString fName;

	img1 = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	img2 = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	img3 = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	img4 = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	img5 = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	img6 = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);

	if ((!img1) || (!img2) || (!img3) || (!img4) || (!img5) || (!img6)) {
		// even without this free step, memory would be retrieved by kernel
		// by the destructor (garbage collector)
		delete img1;
		delete img2;
		delete img3;
		delete img4;
		delete img5;
		delete img6;
		return;
	}
	// load faces
	fName = gDataPath + GString("skybox_rt.png");
	err1 = img1->Load(StrUtils::ToAscii(fName));
	fName = gDataPath + GString("skybox_lf.png");
	err2 = img2->Load(StrUtils::ToAscii(fName));

	fName = gDataPath + GString("skybox_gr.png");
	err3 = img3->Load(StrUtils::ToAscii(fName));
	fName = gDataPath + GString("skybox.png");
	err4 = img4->Load(StrUtils::ToAscii(fName));

	fName = gDataPath + GString("skybox_ft.png");
	err5 = img5->Load(StrUtils::ToAscii(fName));
	fName = gDataPath + GString("skybox_bk.png");
	err6 = img6->Load(StrUtils::ToAscii(fName));
	// bind textures
	if ((err1 == G_NO_ERROR) && (err2 == G_NO_ERROR) && (err3 == G_NO_ERROR) ||
		(err4 == G_NO_ERROR) && (err5 == G_NO_ERROR) && (err6 == G_NO_ERROR)) {
		glGenTextures(1, &cubeTex);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubeTex);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGBA8, img5->Width(), img5->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img5->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGBA8, img6->Width(), img6->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img6->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGBA8, img3->Width(), img3->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img3->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGBA8, img4->Width(), img4->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img4->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGBA8, img1->Width(), img1->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img1->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGBA8, img2->Width(), img2->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img2->Pixels());
	}
	// even without this free step, memory would be retrieved by kernel
	// by the destructor (garbage collector)
	delete img1;
	delete img2;
	delete img3;
	delete img4;
	delete img5;
	delete img6;
}
//------------------------------------------------------------

//----- loads specified shaders ---------------
GLhandleARB QGLWidgetTest::useShader(const GChar8 *vShader, const GChar8 *fShader) {

	// Reading the shaders from files
	GDynArray<GChar8> vShaderData;
	GDynArray<GChar8> fShaderData;

	FileUtils::ReadFile(vShader, vShaderData);
	FileUtils::ReadFile(fShader, fShaderData);

	GLhandleARB brickVS, brickFS, brickProg;   // handles to objects
	GLint       vertCompiled, fragCompiled;    // status values
	GLint       linked;

	// Create a vertex shader object and a fragment shader object
	brickVS = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	brickFS = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	// Load source code strings into shaders
	char *vS, *fS;
	vS = new char[vShaderData.size() + 1];
	fS = new char[fShaderData.size() + 1];

	std::memcpy(vS, &vShaderData[0], vShaderData.size());
	std::memcpy(fS, &fShaderData[0], fShaderData.size());
	vS[vShaderData.size()] = 0;
	fS[fShaderData.size()] = 0;
	glShaderSourceARB(brickVS, 1, (const GLcharARB**)(&(vS)), NULL);
	glShaderSourceARB(brickFS, 1, (const GLcharARB**)(&(fS)), NULL);

	// Compile the brick vertex shader, and print out
	// the compiler log file.
	glCompileShaderARB(brickVS);
	if (PrintOpenGLError())  // Check for OpenGL errors
		G_DEBUG("Error when compile vertex shader");
	glGetObjectParameterivARB(brickVS, GL_OBJECT_COMPILE_STATUS_ARB, &vertCompiled);
	gExtManager->PrintInfoLog(brickVS); // Check for errors

	// Compile the brick vertex shader, and print out
	// the compiler log file.
	glCompileShaderARB(brickFS);
	if (PrintOpenGLError())  // Check for OpenGL errors
		G_DEBUG("Error when compile fragment shader");
	glGetObjectParameterivARB(brickFS, GL_OBJECT_COMPILE_STATUS_ARB, &fragCompiled);
	gExtManager->PrintInfoLog(brickFS); // Check for errors
	delete [] vS;
	delete [] fS;
	if (!vertCompiled || !fragCompiled)
		return 0;
	// Create a program object and attach the two compiled shaders
	brickProg = glCreateProgramObjectARB();
	glAttachObjectARB(brickProg, brickVS);
	glAttachObjectARB(brickProg, brickFS);
	// Link the program object and print out the info log
	glLinkProgramARB(brickProg);
	if (PrintOpenGLError())  // Check for OpenGL errors
		G_DEBUG("Error when link shader program");
	glGetObjectParameterivARB(brickProg, GL_OBJECT_LINK_STATUS_ARB, &linked);
	gExtManager->PrintInfoLog(brickProg); // Check for errors
	if (!linked)
		return 0;
	return brickProg;
}
//------------------------------------------------------------

// draw a cube with color specified at each vertex
void QGLWidgetTest::drawColorBlendingCube() {
	// Vertex colors
	GDouble colors[8][4] = {{1.0f, 0.0f, 0.0f, 1.0f},
	                       {0.0f, 1.0f, 0.0f, 1.0f},
	                       {0.0f, 0.0f, 1.0f, 1.0f},
	                       {1.0f, 1.0f, 0.0f, 1.0f},
	                       {1.0f, 0.0f, 1.0f, 1.0f},
	                       {0.0f, 1.0f, 1.0f, 1.0f},
	                       {0.5f, 0.5f, 0.5f, 1.0f},
	                       {1.0f, 0.3f, 0.8f, 1.0f}};
	
	setLightAndTransform();
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	if (light) {
		drawCube(true, false, 0, colors);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		glEnable(GL_BLEND);
		light = false;
		setLightAndTransform();
		drawCube(true, false, 0, colors);
		light = true;
	}
	else
		drawCube(true, false, 0, colors);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// draw a textured cube
void QGLWidgetTest::drawSimpleTexturedCube() {

	setLightAndTransform();
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	activeTextureUnit(GL_TEXTURE0_ARB);
	drawCube(true, true);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// draw an alpha transparency cube (50% alpha)
void QGLWidgetTest::drawBlendingTexturedCube() {

	setLightAndTransform();
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glColor4f(1.0f, 1.0f, 1.0f, 0.4f);		// Full Brightness, 40% Alpha ( NEW )
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);		// Blending Function For Translucency Based On Source Alpha Value
	glEnable(GL_BLEND);						// Turn Blending On
	glDisable(GL_DEPTH_TEST);
	activeTextureUnit(GL_TEXTURE0_ARB);
	drawCube(true, true);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// 3 pass multitexture with mask (here we have voluntary used a single texture unit, just for example)
void QGLWidgetTest::drawMultiTexturedCube() {

	if (!gExtManager->IsMultitextureSupported()) {
		if (!msgShown) {
			msgShown = true;
			QMessageBox::information(this, "Attention", "Multitexturing is not supported.");
		}
		vis = 1;
		return;
	}
	setLightAndTransform();
	activeTextureUnit(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[0]); // base image
	drawCube(true, true);

	setDefaultGlobalStates();
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, textures[2]); // multiply mask
	drawCube(true, true);

	setDefaultGlobalStates();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	if (light)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	drawCube(true, true);

	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// single pass multi textured cube
void QGLWidgetTest::drawMultiTexturedCube2() {

	if (!gExtManager->IsMultitextureSupported()) {
		if (msgShown == false) {
			msgShown = true;
			QMessageBox::information(this, "Attention", "Multitexturing is not supported.");
		}
		vis = 1;
		return;
	}
	GInt32 tunits = gExtManager->TextureUnitsCount();
	if (tunits < 2) {
		if (msgShown == false) {
			msgShown = true;
			QString msg = "Required 2 texture units. Sorry, your board supports up to ";
			msg += QString::number(tunits) + " texture units";
			QMessageBox::information(this, "Attention", msg);
		}
		vis = 1;
		return;
	}

	setLightAndTransform();
	glDisable(GL_BLEND);

	// TEXTURE UNIT #0
	activeTextureUnit(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	// TEXTURE-UNIT #1
	activeTextureUnit(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);

	if (tunits >= 3) {
		// TEXTURE-UNIT #2
		activeTextureUnit(GL_TEXTURE2_ARB);
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
		drawCube(true, false, 3);
	}
	// in this case we have only 2 tunits, so we have to do 2 step
	else {
		drawCube(true, false, 2);
		glEnable(GL_BLEND);

		deactiveTextureUnit(GL_TEXTURE1_ARB);
		activeTextureUnit(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		glBlendFunc(GL_ONE, GL_ONE);
		drawCube(true, false, 1);
	}

	deactiveTextureUnit(GL_TEXTURE2_ARB);
	deactiveTextureUnit(GL_TEXTURE1_ARB);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE2_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE1_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------
// multitexture (2 texture units) and automatic texture coords generation
void QGLWidgetTest::drawReflectCube() {

	if (!gExtManager->IsMultitextureSupported()) {
		if (!msgShown) {
			msgShown = true;
			QMessageBox::information(this, "Attention", "Multitexturing is not supported.");
		}
		vis = 1;
		return;
	}
	GInt32 tunits = gExtManager->TextureUnitsCount();
	if (tunits < 2) {
		if (!msgShown) {
			msgShown = true;
			QString msg = "Required 2 texture units. Sorry, your board supports up to ";
			msg += QString::number(tunits) + " texture units";
			QMessageBox::information(this, "Attention", msg);
		}
		vis = 1;
		return;
	}

	setLightAndTransform();
	// TEXTURE UNIT #0
	activeTextureUnit(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	// TEXTURE-UNIT #1
	activeTextureUnit(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[5]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD_SIGNED_ARB);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	drawCube(false, false, 1);
	deactiveTextureUnit(GL_TEXTURE1_ARB);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE1_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// CUBE MAPPING test
void QGLWidgetTest::drawReflectCube2() {

	if (!gExtManager->IsCubemapSupported()) {
		if(msgShown == false) {
			msgShown = true;
			QMessageBox::information(this, "Attention", "Cubemaps are not supported.");
		}
		vis = 1;
		return;
	}
	GInt32 tunits = gExtManager->TextureUnitsCount();
	if (tunits < 2) {
		if (msgShown == false) {
			msgShown = true;
			QString msg = "Required 2 texture units. Sorry, your board supports up to ";
			msg += QString::number(tunits) + " texture units";
			QMessageBox::information(this, "Attention", msg);
		}
		vis = 1;
		return;
	}

	setLightAndTransform();
	// TEXTURE UNIT #0
	activeTextureUnit(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	// TEXTURE-UNIT #1
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubeTex);
	glEnable(GL_TEXTURE_CUBE_MAP_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	drawCube(false, false, 1);

	deactiveTextureUnit(GL_TEXTURE1_ARB);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE1_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// GLSL shaders
void QGLWidgetTest::drawCubeWithProgShader() {

	if(!gExtManager->IsArbShadersSupported()) {
		if(msgShown == false) {
			msgShown = true;
			QMessageBox::information(this, "Attention", "ARB shaders are not supported.");
		}
		vis = 1;
		return;
	}
	GInt32 tunits = gExtManager->TextureUnitsCount();
	if (tunits < 2) {
		if (!msgShown) {
			msgShown = true;
			QString msg = "Required 2 texture units. Sorry, your board supports up to ";
			msg += QString::number(tunits) + " texture units";
			QMessageBox::information(this, "Attention", msg);
		}
		vis = 1;
		return;
	}

	setLightAndTransform();
	glUseProgramObjectARB(shader);
	
	int loc0 = glGetUniformLocationARB(shader, "TextureUnit0");
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glUniform1iARB(loc0, 0);

	int loc1 = glGetUniformLocationARB(shader, "TextureUnit1");
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glUniform1iARB(loc1, 1);

	drawCube(false, false, 2);

	glUseProgramObjectARB((GLhandleARB)0);
	deactiveTextureUnit(GL_TEXTURE1_ARB);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE1_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

void QGLWidgetTest::SetTextureVertex(const GLint textureIndex, const GLfloat u, const GLfloat v) {

	if (!glMultiTexCoord2fARB)
		glTexCoord2f(u, v);
	else {
		switch(textureIndex) {
			case 3:	glMultiTexCoord2fARB(GL_TEXTURE2_ARB, u, v);
			case 2:	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u, v);
			case 1: glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
		}
	}
}

// main cube draw routine
void QGLWidgetTest::drawCube(GBool vertNorm, GBool uvCoord, GUInt8 numMultiText, GDouble vertCol[8][4]) {

	GLfloat q = 1.0/sqrt(3.0);

	glBegin(GL_QUADS);
		// Front Face
		if (!vertNorm)
			glNormal3f(0.0f, 0.0f, 1.0f);
		else
			glNormal3f(-q, -q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[0][0], vertCol[0][1], vertCol[0][2], vertCol[0][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 0.0f);
		SetTextureVertex(numMultiText, 0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		if (vertNorm)
			glNormal3f(q, -q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[1][0], vertCol[1][1], vertCol[1][2], vertCol[1][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 0.0f);
		SetTextureVertex(numMultiText, 1.0f, 0.0f);
		glVertex3f(1.0f, -1.0f,  1.0f);
		if (vertNorm)
			glNormal3f(q, q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[2][0], vertCol[2][1], vertCol[2][2], vertCol[2][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 1.0f);
		SetTextureVertex(numMultiText, 1.0f, 1.0f);
		glVertex3f(1.0f,  1.0f,  1.0f);
		if (vertNorm)
			glNormal3f(-q,q,q);
		if (vertCol != NULL)
			glColor4f(vertCol[3][0], vertCol[3][1], vertCol[3][2], vertCol[3][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 1.0f);
		SetTextureVertex(numMultiText, 0.0f, 1.0f);
		glVertex3f(-1.0f,  1.0f,  1.0f);
		// Back Face
		if (!vertNorm)
			glNormal3f(0.0f, 0.0f, -1.0f);
		else
			glNormal3f(-q, -q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[4][0], vertCol[4][1], vertCol[4][2], vertCol[4][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 0.0f);
		SetTextureVertex(numMultiText, 0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(-q, q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[5][0], vertCol[5][1], vertCol[5][2], vertCol[5][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 1.0f);
		SetTextureVertex(numMultiText, 1.0f, 0.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(q, q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[6][0], vertCol[6][1], vertCol[6][2], vertCol[6][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 1.0f);
		SetTextureVertex(numMultiText, 1.0f, 1.0f);
		glVertex3f(1.0f,  1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(q, -q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[7][0], vertCol[7][1], vertCol[7][2], vertCol[7][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 0.0f);
		SetTextureVertex(numMultiText, 0.0f, 1.0f);
		glVertex3f(1.0f, -1.0f, -1.0f);
		// Top Face
		if (!vertNorm)
			glNormal3f(0.0f, 1.0f, 0.0f);
		else
			glNormal3f(-q, q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[5][0], vertCol[5][1], vertCol[5][2], vertCol[5][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 1.0f);
		SetTextureVertex(numMultiText, 0.0f, 0.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(-q, q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[3][0], vertCol[3][1], vertCol[3][2], vertCol[3][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 0.0f);
		SetTextureVertex(numMultiText, 1.0f, 0.0f);
		glVertex3f(-1.0f,  1.0f,  1.0f);
		if (vertNorm)
			glNormal3f(q, q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[2][0], vertCol[2][1], vertCol[2][2], vertCol[2][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 0.0f);
		SetTextureVertex(numMultiText, 1.0f, 1.0f);
		glVertex3f(1.0f,  1.0f,  1.0f);
		if (vertNorm)
			glNormal3f(q, q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[6][0], vertCol[6][1], vertCol[6][2], vertCol[6][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 1.0f);
		SetTextureVertex(numMultiText, 0.0f, 1.0f);
		glVertex3f(1.0f,  1.0f, -1.0f);
		// Bottom Face
		if (!vertNorm)
			glNormal3f(0.0f, -1.0f, 0.0f);
		else
			glNormal3f(-q, -q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[4][0], vertCol[4][1], vertCol[4][2], vertCol[4][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 1.0f);
		SetTextureVertex(numMultiText, 0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(q, -q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[7][0], vertCol[7][1], vertCol[7][2], vertCol[7][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 1.0f);
		SetTextureVertex(numMultiText, 1.0f, 0.0f);
		glVertex3f(1.0f, -1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(q, -q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[1][0], vertCol[1][1], vertCol[1][2], vertCol[1][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 0.0f);
		SetTextureVertex(numMultiText, 1.0f, 1.0f);
		glVertex3f(1.0f, -1.0f,  1.0f);
		if (vertNorm)
			glNormal3f(-q, -q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[0][0], vertCol[0][1], vertCol[0][2], vertCol[0][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 0.0f);
		SetTextureVertex(numMultiText, 0.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		// Right face
		if (!vertNorm)
			glNormal3f(1.0f, 0.0f, 0.0f);
		else
			glNormal3f(q, -q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[7][0], vertCol[7][1], vertCol[7][2], vertCol[7][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 0.0f);
		SetTextureVertex(numMultiText, 0.0f, 0.0f);
		glVertex3f(1.0f, -1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(q, q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[6][0], vertCol[6][1], vertCol[6][2], vertCol[6][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 1.0f);
		SetTextureVertex(numMultiText, 1.0f, 0.0f);
		glVertex3f(1.0f,  1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(q, q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[2][0], vertCol[2][1], vertCol[2][2], vertCol[2][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 1.0f);
		SetTextureVertex(numMultiText, 1.0f, 1.0f);
		glVertex3f(1.0f,  1.0f,  1.0f);

		if (vertNorm)
			glNormal3f(q, -q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[1][0], vertCol[1][1], vertCol[1][2], vertCol[1][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 0.0f);
		SetTextureVertex(numMultiText, 0.0f, 1.0f);
		glVertex3f(1.0f, -1.0f,  1.0f);
		// Left Face
		if (!vertNorm)
			glNormal3f(-1.0f, 0.0f, 0.0f);
		else
			glNormal3f(-q, -q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[4][0], vertCol[4][1], vertCol[4][2], vertCol[4][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 0.0f);
		SetTextureVertex(numMultiText, 0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		if (vertNorm)
			glNormal3f(-q, -q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[0][0], vertCol[0][1], vertCol[0][2], vertCol[0][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 0.0f);
		SetTextureVertex(numMultiText, 1.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		if (vertNorm)
			glNormal3f(-q, q, q);
		if (vertCol != NULL)
			glColor4f(vertCol[3][0], vertCol[3][1], vertCol[3][2], vertCol[3][3]);
		if (uvCoord)
			glTexCoord2f(1.0f, 1.0f);
		SetTextureVertex(numMultiText, 1.0f, 1.0f);
		glVertex3f(-1.0f,  1.0f,  1.0f);
		if (vertNorm)
			glNormal3f(-q, q, -q);
		if (vertCol != NULL)
			glColor4f(vertCol[5][0], vertCol[5][1], vertCol[5][2], vertCol[5][3]);
		if (uvCoord)
			glTexCoord2f(0.0f, 1.0f);
		SetTextureVertex(numMultiText, 0.0f, 1.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();
}
//------------------------------------------------------------

//----- activate a texture unit -------------------------------
void QGLWidgetTest::activeTextureUnit(GLint unit) {

	if (glActiveTextureARB)
		glActiveTextureARB(unit);
	glEnable(GL_TEXTURE_2D);
}
//------------------------------------------------------------

//----- deactivate a texture unit ----------------------------
void QGLWidgetTest::deactiveTextureUnit(GLint unit) {

	if (glActiveTextureARB)
		glActiveTextureARB(unit);
	glDisable(GL_TEXTURE_2D);
}
//------------------------------------------------------------

//------------------------------------------------------------
void QGLWidgetTest::setDefaultTextureUnitStates(GLint unit) {

	if (glActiveTextureARB)
		glActiveTextureARB(unit);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PREVIOUS_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, GL_CONSTANT_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, GL_SRC_ALPHA);
	glTexEnvf (GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
	glTexEnvf (GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1.0f);
	glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
}
//------------------------------------------------------------

//------------------------------------------------------------
void QGLWidgetTest::setLightAndTransform() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	lightPosition[2] = z_light;
	glLightfv(GL_LIGHT1, GL_POSITION,lightPosition);
	glTranslatef(0.0f,0.0f,z);
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);
	if (light)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
}
//------------------------------------------------------------

//------------------------------------------------------------
void QGLWidgetTest::setDefaultGlobalStates() {

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	// Warning message box is shown
	msgShown = false;
}
//------------------------------------------------------------
