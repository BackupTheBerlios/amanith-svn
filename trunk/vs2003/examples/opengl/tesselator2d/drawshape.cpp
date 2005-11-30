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
#include <amanith/geometry/gxformconv.h>
#include <amanith/geometry/gxform.h>
#include <amanith/2d/gtesselator2d.h>
#include "resource.h"

using namespace Amanith;

HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;	// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool keys[256];			// Array Used For The Keyboard Routine
bool active = TRUE;		// Window Active Flag Set To TRUE By Default
bool fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

// Amanith stuff
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
GLuint texture = 0;
GLuint base; // Base Display List For The Font Set

void setLightAndTransform() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(gX, gY, gZ);
}

void setDefaultGlobalStates() {
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void BuildFont(void) {

	HFONT  font;					       // Windows Font ID
	base = glGenLists(96);			       // Storage For 96 Characters
	font = CreateFontA(	-10,			       // Height Of Font
		0,			       // Width Of Font
		0,			       // Angle Of Escapement
		0,			       // Orientation Angle
		FW_BOLD,		       // Font Weight
		FALSE,		       // Italic
		FALSE,		       // Underline
		FALSE,		       // Strikeout
		ANSI_CHARSET,		       // Character Set Identifier
		OUT_TT_PRECIS,	       // Output Precision
		CLIP_DEFAULT_PRECIS,	       // Clipping Precision
		NONANTIALIASED_QUALITY,	       // Output Quality
		FF_DONTCARE|DEFAULT_PITCH,   // Family And Pitch
		"Arial");	       // Font Name

	SelectObject(hDC, font);							// Selects The Font We Want
	wglUseFontBitmaps(hDC, 32, 96, base);
}

void glMyPrint(const char *fmt, ...) {

	char  text[256]; // Holds Our String
	va_list ap;      // Pointer To List Of Arguments

	if (fmt == NULL) return;											// Do Nothing
	va_start(ap, fmt);	// Parses The String For Variables
	vsprintf(text, fmt, ap);  // And Converts Symbols To Actual Numbers
	va_end(ap); // Results Are Stored In Text
	glPushAttrib(GL_LIST_BIT);  // Pushes The Display List Bits
	glListBase(base - 32); // Sets The Base Character to 32
	glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib(); // Pops The Display List Bits
}

void DeleteFont(void) {
	glDeleteLists(base, 96);
}

void BuildShape(const GFloat RadAngle1, const GFloat RadAngle2, const GFloat RadAngle3) {

	GMatrix33 preTrans, postTrans, rot;
	GMatrix33 finalMatrix;

	// rotation pivot is at (8, 8)
	TranslationToMatrix(preTrans, GVector2(-8, -8));
	TranslationToMatrix(postTrans, GVector2(8, 8));

	gContoursPoints.clear();
	gContoursIndexes.clear();

	// little cross
	RotationToMatrix(rot, RadAngle1);
	finalMatrix = (postTrans * rot) * preTrans;
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 7));
	gContoursPoints.push_back(finalMatrix * GPoint2(7, 8));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 9));
	gContoursPoints.push_back(finalMatrix * GPoint2(7, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(8, 9));
	gContoursPoints.push_back(finalMatrix * GPoint2(9, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 9));
	gContoursPoints.push_back(finalMatrix * GPoint2(9, 8));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 7));
	gContoursPoints.push_back(finalMatrix * GPoint2(9, 6));
	gContoursPoints.push_back(finalMatrix * GPoint2(8, 7));
	gContoursPoints.push_back(finalMatrix * GPoint2(7, 6));
	gContoursIndexes.push_back(12);

	// big cross
	RotationToMatrix(rot, RadAngle2);
	finalMatrix = (postTrans * rot) * preTrans;

	gContoursPoints.push_back(finalMatrix * GPoint2(2, 6));
	gContoursPoints.push_back(finalMatrix * GPoint2(2, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 14));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 14));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(14, 10));
	gContoursPoints.push_back(finalMatrix * GPoint2(14, 6));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 6));
	gContoursPoints.push_back(finalMatrix * GPoint2(10, 2));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 2));
	gContoursPoints.push_back(finalMatrix * GPoint2(6, 6));
	gContoursIndexes.push_back(12);

	// star
	RotationToMatrix(rot, RadAngle3);
	finalMatrix = (postTrans * rot) * preTrans;
	gContoursPoints.push_back(finalMatrix * GPoint2(2, 2));
	gContoursPoints.push_back(finalMatrix * GPoint2(5, 8));
	gContoursPoints.push_back(finalMatrix * GPoint2(2, 14));
	gContoursPoints.push_back(finalMatrix * GPoint2(8, 11));
	gContoursPoints.push_back(finalMatrix * GPoint2(14, 14));
	gContoursPoints.push_back(finalMatrix * GPoint2(11, 8));
	gContoursPoints.push_back(finalMatrix * GPoint2(14, 2));
	gContoursPoints.push_back(finalMatrix * GPoint2(8, 5));
	gContoursIndexes.push_back(8);

	// square
	gContoursPoints.push_back(GPoint2(1, 1));
	gContoursPoints.push_back(GPoint2(1, 15));
	gContoursPoints.push_back(GPoint2(15, 15));
	gContoursPoints.push_back(GPoint2(15, 1));
	gContoursIndexes.push_back(4);
}

void GenerateTessellation() {

	GTesselator2D tesselator;

	gTrianglesPts.clear();
	gTrianglesIdx.clear();
	tesselator.Tesselate(gContoursPoints, gContoursIndexes, gTrianglesPts, gTrianglesIdx, (GFillBehavior)gFillRule);
}

void InitApp() {

	GString s;
	GError err;

	// Depth Into The Screen
	gX = -8.0f;
	gY = -8.0f;
	gZ = -22.0f;
	gKernel = new GKernel();
	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";
	gWireFrame = G_TRUE;
	gAnim = G_TRUE;
	gAng1 = gAng2 = gAng3 = 0;

	gFillRule = G_ODD_EVEN_RULE;
	gFillMode = 1;
	gStepAng1 = 0.004f;
	gStepAng2 = -0.002f;
	gStepAng3 = 0.001f;
	gAng1 = gAng2 = gAng3 = 0;

	// load the texture
	gTexture = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	if (!gTexture)
		abort();
	s = gDataPath + "metal05.png";
	err = gTexture->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err != G_NO_ERROR)
		abort();
}

void KillApp() {

	if (gKernel)
		delete gKernel;
	DeleteFont();
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) {

	if (height == 0)
		height = 1;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 1500.0f);
	glMatrixMode(GL_MODELVIEW);
}

int InitGL(GLvoid) {

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.6f, 0.6f, 0.6f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);					// Set Line Antialiasing
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	// bind the texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gTexture->Width(), gTexture->Height(),
				 0, GL_BGRA, GL_UNSIGNED_BYTE, gTexture->Pixels());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setDefaultGlobalStates();
	BuildFont();
	return TRUE;
}

void DrawTriangles(const GDynArray< GPoint<GDouble, 2> >& Points, const GDynArray<GULong>& Indexes) {

	GUInt32 i, j = (GUInt32)Indexes.size();
	GPoint<GDouble, 2> a, b, c;
	GPoint<GDouble, 3> col;
	GLfloat texPlaneGen[4];

	if (gFillMode == 2 || gFillMode == 3) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		texPlaneGen[0] = 0.05f;
		texPlaneGen[1] = 0;
		texPlaneGen[2] = 0;
		texPlaneGen[3] = 0;
		glTexGenfv(GL_S, GL_EYE_PLANE, texPlaneGen);
		texPlaneGen[0] = 0;
		texPlaneGen[1] = 0.05f;
		texPlaneGen[2] = 0;
		texPlaneGen[3] = 0;
		glTexGenfv(GL_T, GL_EYE_PLANE, texPlaneGen);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
	}

	// debug stuff
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.2f, 0.4f);
	a[G_X] = -2;
	for (i = 0; i < 21; i++) {
		glVertex3d(a[G_X], 0.0f, 1.0f);
		glVertex3d(a[G_X], 16.0f, 1.0f);
		a[G_X] += 1.0;
	}
	a[G_Y] = 0;
	for (i = 0; i < 17; i++) {
		glVertex3d(-2, a[G_Y], 1.0f);
		glVertex3d(18.0, a[G_Y], 1.0f);
		a[G_Y] += 1.0;
	}

	glColor3f(0.6f, 0.0f, 0.3f);
	a[G_X] = -1.5;
	for (i = 0; i < 20; i++) {
		glVertex3d(a[G_X], 0.0f, 1.0f);
		glVertex3d(a[G_X], 16.0f, 1.0f);
		a[G_X] += 1.0;
	}
	a[G_Y] = 0.5;
	for (i = 0; i < 16; i++) {
		glVertex3d(-2, a[G_Y], 1.0f);
		glVertex3d(18.0, a[G_Y], 1.0f);
		a[G_Y] += 1.0;
	}
	glEnd();


	glEnable(GL_BLEND);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	glBlendFunc(GL_ONE, GL_ONE);

	glBegin(GL_TRIANGLES);
	if (gFillMode == 0) {
		glColor3f(0.2f, 0.2f, 0.2f);
		for (i = 0; i < j; i+=3) {
			a = Points[Indexes[i]];
			b = Points[Indexes[i + 1]];
			c = Points[Indexes[i + 2]];
			glVertex3d(a[G_X], a[G_Y], 1.0);
			glVertex3d(b[G_X], b[G_Y], 1.0);
			glVertex3d(c[G_X], c[G_Y], 1.0);
		}
	}
	else
	if (gFillMode == 1) {
		GPoint<GDouble, 3> col1(0.0f, 0.1f, 0.2f);
		GPoint<GDouble, 3> col2(0.3f, 0.2f, 1.0f);
		for (i = 0; i < j; i+=3) {
			a = Points[Indexes[i]];
			b = Points[Indexes[i + 1]];
			c = Points[Indexes[i + 2]];
			col = GMath::BarycentricConvexSum(a[G_X] + 1.0, col1, 18.0 - a[G_X], col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(a[G_X], a[G_Y], 1.0);
			col = GMath::BarycentricConvexSum(b[G_X] + 1.0, col1, 18.0 - b[G_X], col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(b[G_X], b[G_Y], 1.0);
			col = GMath::BarycentricConvexSum(c[G_X] + 1.0, col1, 18.0 - c[G_X], col2);
			glColor3d(col[0], col[1], col[2]);
			glVertex3d(c[G_X], c[G_Y], 1.0);
		}
	}
	else
	if (gFillMode == 2) {
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 0; i < j; i+=3) {
			a = Points[Indexes[i]];
			b = Points[Indexes[i + 1]];
			c = Points[Indexes[i + 2]];
			glVertex3d(a[G_X], a[G_Y], 1.0);
			glVertex3d(b[G_X], b[G_Y], 1.0);
			glVertex3d(c[G_X], c[G_Y], 1.0);
		}
	}
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);
	if (!gWireFrame)
		return;

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (i = 0; i < j; i+=3) {
		a = Points[Indexes[i]];
		b = Points[Indexes[i + 1]];
		c = Points[Indexes[i + 2]];
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
	}
	glEnd();

	GString s, s2;
	j = (GUInt32)Points.size();
	glColor3f(0.1f, 0.1f, 0.1f);
	for (i = 0; i < j; ++i) {
		a = Points[i];
		s = StrUtils::ToString(a, ";", "%5.2f");
		s = StrUtils::Purge(s, " ");
		glRasterPos2d(a[G_X], a[G_Y]);
		glMyPrint(StrUtils::ToAscii(s));
	}
}

int DrawGLScene(GLvoid)	{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setLightAndTransform();

	if (gAnim) {
		gAng1 += gStepAng1;
		gAng2 += gStepAng2;
		gAng3 += gStepAng3;
	}
	// build the shape
	BuildShape(gAng1, gAng2, gAng3);
	// triangulate
	GenerateTessellation();

	DrawTriangles(gTrianglesPts, gTrianglesIdx);
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
	if (!CreateGLWindow("Realtime tesselator example - Press F1 for help", 640, 480, 16, fullscreen))
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
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
				done = TRUE;							// ESC or DrawGLScene Signalled A Quit
			else									// Not Time To Quit, Update Screen
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)

			if (keys[VK_F1]) {						// Is F1 Being Pressed?
				keys[VK_F1] = FALSE;
				s = "A/Z: Zoom +/-\n";
				s += "Arrow keys: Move viewport\n";
				s += "Space: Toggle wireframe\n";
				s += "M/N: Accelerate/Decelerate rotation\n";
				s += "F: Change fill rule\n";
				s += "C: Change fill mode\n";
				s += "B: Toggle animation\n";
				MessageBox(NULL, StrUtils::ToAscii(s), "Command keys", MB_OK | MB_ICONINFORMATION);
			}
			// A key
			if (keys[65]) {
				keys[65] = FALSE;
				gZ -= 0.5f;
			}
			// Z key
			if (keys[90]) {
				keys[90] = FALSE;
				gZ += 0.5f;
			}
			if (keys[VK_UP]) {
				keys[VK_UP] = FALSE;
				gY += 0.5f;
			}
			if (keys[VK_DOWN]) {
				keys[VK_DOWN] = FALSE;
				gY -= 0.5f;
			}
			if (keys[VK_RIGHT]) {
				keys[VK_RIGHT] = FALSE;
				gX += 0.5f;
			}
			if (keys[VK_LEFT]) {
				keys[VK_LEFT] = FALSE;
				gX -= 0.5f;
			}
			// M key
			if (keys[77]) {
				keys[77] = FALSE;
				gStepAng1 *= 1.25f;
				gStepAng2 *= 1.25f;
				gStepAng3 *= 1.25f;
			}
			// N key
			if (keys[78]) {
				keys[78] = FALSE;
				gStepAng1 /= 1.25f;
				gStepAng2 /= 1.25f;
				gStepAng3 /= 1.25f;
			}
			// F
			if (keys[70]) {
				keys[70] = FALSE;
				gFillRule = (gFillRule + 1) % 3;
			}
			// B
			if (keys[66]) {
				keys[66] = FALSE;
				gAnim = !gAnim;
			}
			// C
			if (keys[67]) {
				keys[67] = FALSE;
				gFillMode = (gFillMode + 1) % 3;
			}
			if (keys[VK_SPACE]) {
				keys[VK_SPACE] = FALSE;
				gWireFrame = !gWireFrame;
			}
			if (keys[VK_F7]) {
				keys[VK_F7] = FALSE;
				gAng1 -= gStepAng1;
				gAng2 -= gStepAng2;
				gAng3 -= gStepAng3;
			}
			if (keys[VK_F8]) {
				keys[VK_F8] = FALSE;
				gAng1 += gStepAng1;
				gAng2 += gStepAng2;
				gAng3 += gStepAng3;
			}
		}
	}
	// Shutdown
	KillGLWindow();									// Kill The Window
	KillApp();
	return (int)(msg.wParam);							// Exit The Program
}
