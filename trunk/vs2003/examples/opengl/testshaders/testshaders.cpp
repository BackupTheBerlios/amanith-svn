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
#include <windows.h>
#include <amanith/gkernel.h>
#include <amanith/gopenglext.h>
#include <amanith/2d/gpixelmap.h>
#include "resource.h"

using namespace Amanith;

#define PrintOpenGLError() gExtManager->PrintOglError(__FILE__, __LINE__)

HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;	// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool keys[256];			// Array Used For The Keyboard Routine
bool active = TRUE;		// Window Active Flag Set To TRUE By Default
bool fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

// Amanith stuff
GKernel *gKernel = NULL;
GOpenglExt *gExtManager = NULL;	// extensions manager
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

void InitApp() {

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

	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";
}

void KillApp() {

	if (gExtManager)
		delete gExtManager;
	if (gKernel)
		delete gKernel;
}

// create a texture and return its ID
GLuint loadTexture(const GChar8 *fileName, const GChar8 *Options) {

	GError err;
	GLuint texture[1];
	GPixelMap *imgTexture;

	imgTexture = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	if (!imgTexture)
		return 0;

	err = imgTexture->Load(fileName, Options);
	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgTexture->Width(), imgTexture->Height(),
				 0, GL_BGRA, GL_UNSIGNED_BYTE, imgTexture->Pixels());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	delete imgTexture;
	return texture[0];
}
//------------------------------------------------------------

// create cubemap, and place it into textures slot [6]..[11]
void createCubeTex() {

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
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, 4, img5->Width(), img5->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img5->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, 4, img6->Width(), img6->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img6->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, 4, img3->Width(), img3->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img3->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, 4, img4->Width(), img4->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img4->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, 4, img1->Width(), img1->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img1->Pixels());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, 4, img2->Width(), img2->Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img2->Pixels());
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

//----- activate a texture unit -------------------------------
void activeTextureUnit(GLint unit) {

	if (glActiveTextureARB)
		glActiveTextureARB(unit);
	glEnable(GL_TEXTURE_2D);
}
//------------------------------------------------------------

//----- deactivate a texture unit ----------------------------
void deactiveTextureUnit(GLint unit) {

	if (glActiveTextureARB)
		glActiveTextureARB(unit);
	glDisable(GL_TEXTURE_2D);
}
//------------------------------------------------------------

void setDefaultTextureUnitStates(GLint unit) {

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
void setLightAndTransform() {

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

void setDefaultGlobalStates() {

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	// Warning message box is shown
	msgShown = false;
}

//----- loads specified shaders ---------------
GLhandleARB useShader(const GChar8 *vShader, const GChar8 *fShader) {

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

void SetTextureVertex(const GLint textureIndex, const GLfloat u, const GLfloat v) {

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
void drawCube(GBool vertNorm, GBool uvCoord, GUInt8 numMultiText, GDouble vertCol[8][4]) {

	GLfloat q = 1.0f/GMath::Sqrt(3.0f);

	glBegin(GL_QUADS);
	// Front Face
	if (!vertNorm)
		glNormal3f(0.0f, 0.0f, 1.0f);
	else
		glNormal3f(-q, -q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[0][0], (GLfloat)vertCol[0][1], (GLfloat)vertCol[0][2], (GLfloat)vertCol[0][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 0.0f);
	SetTextureVertex(numMultiText, 0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f,  1.0f);
	if (vertNorm)
		glNormal3f(q, -q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[1][0], (GLfloat)vertCol[1][1], (GLfloat)vertCol[1][2], (GLfloat)vertCol[1][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 0.0f);
	SetTextureVertex(numMultiText, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f,  1.0f);
	if (vertNorm)
		glNormal3f(q, q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[2][0], (GLfloat)vertCol[2][1], (GLfloat)vertCol[2][2], (GLfloat)vertCol[2][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 1.0f);
	SetTextureVertex(numMultiText, 1.0f, 1.0f);
	glVertex3f(1.0f,  1.0f,  1.0f);
	if (vertNorm)
		glNormal3f(-q,q,q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[3][0], (GLfloat)vertCol[3][1], (GLfloat)vertCol[3][2], (GLfloat)vertCol[3][3]);
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
		glColor4f((GLfloat)vertCol[4][0], (GLfloat)vertCol[4][1], (GLfloat)vertCol[4][2], (GLfloat)vertCol[4][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 0.0f);
	SetTextureVertex(numMultiText, 0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(-q, q, -q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[5][0], (GLfloat)vertCol[5][1], (GLfloat)vertCol[5][2], (GLfloat)vertCol[5][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 1.0f);
	SetTextureVertex(numMultiText, 1.0f, 0.0f);
	glVertex3f(-1.0f,  1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(q, q, -q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[6][0], (GLfloat)vertCol[6][1], (GLfloat)vertCol[6][2], (GLfloat)vertCol[6][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 1.0f);
	SetTextureVertex(numMultiText, 1.0f, 1.0f);
	glVertex3f(1.0f,  1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(q, -q, -q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[7][0], (GLfloat)vertCol[7][1], (GLfloat)vertCol[7][2], (GLfloat)vertCol[7][3]);
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
		glColor4f((GLfloat)vertCol[5][0], (GLfloat)vertCol[5][1], (GLfloat)vertCol[5][2], (GLfloat)vertCol[5][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 1.0f);
	SetTextureVertex(numMultiText, 0.0f, 0.0f);
	glVertex3f(-1.0f,  1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(-q, q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[3][0], (GLfloat)vertCol[3][1], (GLfloat)vertCol[3][2], (GLfloat)vertCol[3][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 0.0f);
	SetTextureVertex(numMultiText, 1.0f, 0.0f);
	glVertex3f(-1.0f,  1.0f,  1.0f);
	if (vertNorm)
		glNormal3f(q, q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[2][0], (GLfloat)vertCol[2][1], (GLfloat)vertCol[2][2], (GLfloat)vertCol[2][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 0.0f);
	SetTextureVertex(numMultiText, 1.0f, 1.0f);
	glVertex3f(1.0f,  1.0f,  1.0f);
	if (vertNorm)
		glNormal3f(q, q, -q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[6][0], (GLfloat)vertCol[6][1], (GLfloat)vertCol[6][2], (GLfloat)vertCol[6][3]);
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
		glColor4f((GLfloat)vertCol[4][0], (GLfloat)vertCol[4][1], (GLfloat)vertCol[4][2], (GLfloat)vertCol[4][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 1.0f);
	SetTextureVertex(numMultiText, 0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(q, -q, -q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[7][0], (GLfloat)vertCol[7][1], (GLfloat)vertCol[7][2], (GLfloat)vertCol[7][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 1.0f);
	SetTextureVertex(numMultiText, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(q, -q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[1][0], (GLfloat)vertCol[1][1], (GLfloat)vertCol[1][2], (GLfloat)vertCol[1][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 0.0f);
	SetTextureVertex(numMultiText, 1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f,  1.0f);
	if (vertNorm)
		glNormal3f(-q, -q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[0][0], (GLfloat)vertCol[0][1], (GLfloat)vertCol[0][2], (GLfloat)vertCol[0][3]);
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
		glColor4f((GLfloat)vertCol[7][0], (GLfloat)vertCol[7][1], (GLfloat)vertCol[7][2], (GLfloat)vertCol[7][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 0.0f);
	SetTextureVertex(numMultiText, 0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(q, q, -q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[6][0], (GLfloat)vertCol[6][1], (GLfloat)vertCol[6][2], (GLfloat)vertCol[6][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 1.0f);
	SetTextureVertex(numMultiText, 1.0f, 0.0f);
	glVertex3f(1.0f,  1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(q, q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[2][0], (GLfloat)vertCol[2][1], (GLfloat)vertCol[2][2], (GLfloat)vertCol[2][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 1.0f);
	SetTextureVertex(numMultiText, 1.0f, 1.0f);
	glVertex3f(1.0f,  1.0f,  1.0f);

	if (vertNorm)
		glNormal3f(q, -q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[1][0], (GLfloat)vertCol[1][1], (GLfloat)vertCol[1][2], (GLfloat)vertCol[1][3]);
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
		glColor4f((GLfloat)vertCol[4][0], (GLfloat)vertCol[4][1], (GLfloat)vertCol[4][2], (GLfloat)vertCol[4][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 0.0f);
	SetTextureVertex(numMultiText, 0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	if (vertNorm)
		glNormal3f(-q, -q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[0][0], (GLfloat)vertCol[0][1], (GLfloat)vertCol[0][2], (GLfloat)vertCol[0][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 0.0f);
	SetTextureVertex(numMultiText, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f,  1.0f);
	if (vertNorm)
		glNormal3f(-q, q, q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[3][0], (GLfloat)vertCol[3][1], (GLfloat)vertCol[3][2], (GLfloat)vertCol[3][3]);
	if (uvCoord)
		glTexCoord2f(1.0f, 1.0f);
	SetTextureVertex(numMultiText, 1.0f, 1.0f);
	glVertex3f(-1.0f,  1.0f,  1.0f);
	if (vertNorm)
		glNormal3f(-q, q, -q);
	if (vertCol != NULL)
		glColor4f((GLfloat)vertCol[5][0], (GLfloat)vertCol[5][1], (GLfloat)vertCol[5][2], (GLfloat)vertCol[5][3]);
	if (uvCoord)
		glTexCoord2f(0.0f, 1.0f);
	SetTextureVertex(numMultiText, 0.0f, 1.0f);
	glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();
}
//------------------------------------------------------------

// draw a cube with color specified at each vertex
void drawColorBlendingCube() {
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
void drawSimpleTexturedCube() {

	setLightAndTransform();
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	activeTextureUnit(GL_TEXTURE0_ARB);
	drawCube(true, true, 0, NULL);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// draw an alpha transparency cube (50% alpha)
void drawBlendingTexturedCube() {

	setLightAndTransform();
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glColor4f(1.0f, 1.0f, 1.0f, 0.4f);		// Full Brightness, 40% Alpha ( NEW )
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);		// Blending Function For Translucency Based On Source Alpha Value
	glEnable(GL_BLEND);						// Turn Blending On
	glDisable(GL_DEPTH_TEST);
	activeTextureUnit(GL_TEXTURE0_ARB);
	drawCube(true, true, 0, NULL);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// 3 pass multitexture with mask (here we have voluntary used a single texture unit, just for example)
void drawMultiTexturedCube() {

	if (!gExtManager->IsMultitextureSupported()) {
		if (!msgShown) {
			msgShown = true;
			MessageBox(NULL, "Multitexturing is not supported.", "Attention", MB_OK | MB_ICONINFORMATION);
		}
		vis = 1;
		return;
	}
	setLightAndTransform();
	activeTextureUnit(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, textures[0]); // base image
	drawCube(true, true, 0, NULL);

	setDefaultGlobalStates();
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, textures[2]); // multiply mask
	drawCube(true, true, 0, NULL);

	setDefaultGlobalStates();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	if (light)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	drawCube(true, true, 0, NULL);

	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// single pass multi textured cube
void drawMultiTexturedCube2() {

	if (!gExtManager->IsMultitextureSupported()) {
		if (msgShown == false) {
			msgShown = true;
			MessageBox(NULL, "Multitexturing is not supported.", "Attention", MB_OK | MB_ICONINFORMATION);
		}
		vis = 1;
		return;
	}
	GInt32 tunits = gExtManager->TextureUnitsCount();
	if (tunits < 2) {
		if (msgShown == false) {
			msgShown = true;
			GString msg = "Required 2 texture units. Sorry, your board supports up to ";
			msg += StrUtils::ToString(tunits) + " texture units";
			MessageBox(NULL, StrUtils::ToAscii(msg), "Attention", MB_OK | MB_ICONINFORMATION);
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
		drawCube(true, false, 3, NULL);
	}
	// in this case we have only 2 tunits, so we have to do 2 step
	else {
		drawCube(true, false, 2, NULL);
		glEnable(GL_BLEND);

		deactiveTextureUnit(GL_TEXTURE1_ARB);
		activeTextureUnit(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		glBlendFunc(GL_ONE, GL_ONE);
		drawCube(true, false, 1, NULL);
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
void drawReflectCube() {

	if (!gExtManager->IsMultitextureSupported()) {
		if (!msgShown) {
			msgShown = true;
			MessageBox(NULL, "Multitexturing is not supported.", "Attention", MB_OK | MB_ICONINFORMATION);
		}
		vis = 1;
		return;
	}
	GInt32 tunits = gExtManager->TextureUnitsCount();
	if (tunits < 2) {
		if (!msgShown) {
			msgShown = true;
			GString msg = "Required 2 texture units. Sorry, your board supports up to ";
			msg += StrUtils::ToString(tunits) + " texture units";
			MessageBox(NULL, StrUtils::ToAscii(msg), "Attention", MB_OK | MB_ICONINFORMATION);
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
	drawCube(false, false, 1, NULL);
	deactiveTextureUnit(GL_TEXTURE1_ARB);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE1_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// CUBE MAPPING test
void drawReflectCube2() {

	if (!gExtManager->IsCubemapSupported()) {
		if(msgShown == false) {
			msgShown = true;
			MessageBox(NULL, "Cubemaps are not supported.", "Attention", MB_OK | MB_ICONINFORMATION);
		}
		vis = 1;
		return;
	}
	GInt32 tunits = gExtManager->TextureUnitsCount();
	if (tunits < 2) {
		if (msgShown == false) {
			msgShown = true;
			GString msg = "Required 2 texture units. Sorry, your board supports up to ";
			msg += StrUtils::ToString(tunits) + " texture units";
			MessageBox(NULL, StrUtils::ToAscii(msg), "Attention", MB_OK | MB_ICONINFORMATION);
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
	drawCube(false, false, 1, NULL);

	deactiveTextureUnit(GL_TEXTURE1_ARB);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE1_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

// GLSL shaders
void drawCubeWithProgShader() {

	if(!gExtManager->IsArbShadersSupported()) {
		if (msgShown == false) {
			msgShown = true;
			MessageBox(NULL, "Programing shaders are not supported.", "Attention", MB_OK | MB_ICONINFORMATION);
		}
		vis = 1;
		return;
	}
	GInt32 tunits = gExtManager->TextureUnitsCount();
	if (tunits < 2) {
		if (!msgShown) {
			msgShown = true;
			GString msg = "Required 2 texture units. Sorry, your board supports up to ";
			msg += StrUtils::ToString(tunits) + " texture units";
			MessageBox(NULL, StrUtils::ToAscii(msg), "Attention", MB_OK | MB_ICONINFORMATION);
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

	drawCube(false, false, 2, NULL);

	glUseProgramObjectARB((GLhandleARB)0);
	deactiveTextureUnit(GL_TEXTURE1_ARB);
	deactiveTextureUnit(GL_TEXTURE0_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE1_ARB);
	setDefaultTextureUnitStates(GL_TEXTURE0_ARB);
	setDefaultGlobalStates();
}
//------------------------------------------------------------

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) {

	if (height == 0)
		height = 1;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

int InitGL(GLvoid) {

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
	textures[0] = loadTexture(StrUtils::ToAscii(fName), NULL);			// simple texture
	fName = gDataPath + GString("electrica.png");
	textures[1] = loadTexture(StrUtils::ToAscii(fName), "expandpalette=true");	// color image
	fName = gDataPath + GString("mask2.png");
	textures[2] = loadTexture(StrUtils::ToAscii(fName), NULL);			// grey alpha mask
	fName = gDataPath + GString("image2_plusmask.png");
	textures[3] = loadTexture(StrUtils::ToAscii(fName), NULL); // image with alpha mask
	fName = gDataPath + GString("sumu.png");
	textures[4] = loadTexture(StrUtils::ToAscii(fName), NULL);			// NeHe logo
	fName = gDataPath + GString("chromic.png");
	textures[5] = loadTexture(StrUtils::ToAscii(fName), NULL);			// simple metal envmap
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
	return TRUE;
}

int DrawGLScene(GLvoid)	{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (vis == 1)
		drawColorBlendingCube();
	else
	if (vis == 2)
		drawSimpleTexturedCube();
	else
	if (vis == 3)
		drawBlendingTexturedCube();
	else
	if (vis == 4)
		drawMultiTexturedCube();
	else
	if (vis == 5)
		drawMultiTexturedCube2();
	else
	if (vis == 6)
		drawReflectCube();
	else
	if (vis == 7)
		drawReflectCube2();
	else
	if (vis == 8)
		drawCubeWithProgShader();
	glFlush();
	return TRUE;
}

GLvoid KillGLWindow(GLvoid)	{

	if (hRC) {
		if (!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		if (!wglDeleteContext(hRC))
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hRC = NULL;
	}
	if (hDC && !ReleaseDC(hWnd, hDC)) {
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}
	if (hWnd && !DestroyWindow(hWnd)) {
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}
	if (!UnregisterClass("OpenGL", hInstance)) {
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag) {

	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	

	dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
	dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC = GetDC(hWnd))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	if (!SetPixelFormat(hDC, PixelFormat, &pfd))	{
		KillGLWindow();
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(hRC = wglCreateContext(hDC))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!wglMakeCurrent(hDC,hRC)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL()) {
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))
				active = TRUE;						// Program Is Active
			else
				active = FALSE;						// Program Is No Longer Active
			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
	}
	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done = FALSE;							// Bool Variable To Exit Loop
	GString s;
	GVector2 randomDir;
	fullscreen = FALSE;							// Windowed Mode

	// init application
	InitApp();

	// Create Our OpenGL Window
	if (!CreateGLWindow("OpenGL shaders example - Press F1 for help", 640, 480, 16, fullscreen))
		return 0;									// Quit If Window Was Not Created

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
			{
				done = TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (anim && !msgShown) {
				xrot += 0.7f;
				yrot -= 0.5f;
			}

			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
				done = TRUE;							// ESC or DrawGLScene Signalled A Quit
			else									// Not Time To Quit, Update Screen
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)

			if (keys[VK_F1]) {						// Is F1 Being Pressed?
				keys[VK_F1] = FALSE;
				s = "1..8: Toggle shader\n";
				s += "Space: Toggle animation\n";
				s += "A/Z: Zoom +/-\n";
				s += "L: Toggle light";
				MessageBox(NULL, StrUtils::ToAscii(s), "Command keys", MB_OK | MB_ICONINFORMATION);
			}
			// A
			if (keys[65]) {
				keys[65] = FALSE;
				z += 1.0f;
			}
			// Z
			if (keys[90]) {
				keys[90] = FALSE;
				z -= 1.0f;
			}
			if (keys[VK_SPACE]) {
				keys[VK_SPACE] = FALSE;
				anim = !anim;
			}
			// L
			if (keys[76]) {
				keys[76] = FALSE;
				light = !light;
			}
			// 1
			if (keys[49]) {
				keys[49] = FALSE;
				vis = 1;
			}
			// 2
			if (keys[50]) {
				keys[50] = FALSE;
				vis = 2;
			}
			// 3
			if (keys[51]) {
				keys[51] = FALSE;
				vis = 3;
			}
			// 4
			if (keys[52]) {
				keys[52] = FALSE;
				vis = 4;
			}
			// 5
			if (keys[53]) {
				keys[53] = FALSE;
				vis = 5;
			}
			// 6
			if (keys[54]) {
				keys[54] = FALSE;
				vis = 6;
			}
			// 7
			if (keys[55]) {
				keys[55] = FALSE;
				vis = 7;
			}
			// 8
			if (keys[56]) {
				keys[56] = FALSE;
				vis = 8;
			}
		}
	}
	// Shutdown
	KillGLWindow();									// Kill The Window
	KillApp();
	return (msg.wParam);							// Exit The Program
}
