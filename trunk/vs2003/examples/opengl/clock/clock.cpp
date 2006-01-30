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
#include <windows.h>
#include <amanith/gkernel.h>
#include <amanith/2d/gpixelmap.h>
#include <amanith/rendering/gopenglboard.h>
#include <amanith/geometry/gxformconv.h>
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
GKernel *gKernel = NULL;
GOpenglExt *gExtManager = NULL;	// OpenGL extensions manager
GOpenGLBoard *gDrawBoard = NULL;

GGradientDesc *gRadGrad1 = NULL, *gRadGrad2 = NULL, *gRadGrad3 = NULL, *gShadowGrad = NULL;
GCacheBank *gCacheBank = NULL;
GInt32 gClockSlots[100];
GInt32 gSlotsIndex;

bool arbMultisampleSupported = false;
int arbMultisampleFormat = 0;
bool activateFSAA = true;

// InitMultisample: used to query the multisample frequencies
bool InitMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pfd) {  

	// using Amanith OpenGL extension manager is just a matter of test function pointer...
	if (!wglChoosePixelFormatARB) {
		arbMultisampleSupported = false;
		return false;
	}

	// get our current device context
	HDC hDC = GetDC(hWnd);

	int		pixelFormat;
	int		valid;
	UINT	numFormats;
	float	fAttributes[] = {0, 0};

	int iAttributes[] =	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 16,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 16,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 4,
		0, 0
	};
	// first we check to see if we can get a pixel format for 4 samples
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;	
		return arbMultisampleSupported;
	}

	// our pixel format with 4 samples failed, test for 2 samples
	iAttributes[19] = 2;
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
	// if we returned true, and our format count is greater than 1
	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;	
		return arbMultisampleSupported;
	}

	// return the valid format
	return arbMultisampleSupported;
}

void CacheBack() {

	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
	gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
	gDrawBoard->SetStrokeWidth(5);

	// shadow
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(270, 242, 270);
	// clock background
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 240);
	// milliseconds quadrant
	gDrawBoard->SetStrokeWidth(3);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(356, 256, 26);
	// inner bevel ring
	gDrawBoard->SetStrokeWidth(10);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 208);
	// outer bevel ring
	gDrawBoard->SetStrokeWidth(35);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 230);
	// seconds tag
	gDrawBoard->SetStrokeWidth(1);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 442, 256, 446);
	// hours tag
	gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeWidth(8);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 434, 256, 446);
	// 1000 / 12 milliseconds tag
	gDrawBoard->SetStrokeWidth(1);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(356, 274, 356, 276);
}

void CacheCursors() {

	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);

	// milliseconds	
	gDrawBoard->SetStrokeWidth(1);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(356, 250, 356, 254);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(356, 256, 2);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(356, 258, 356, 270);
	// seconds	
	gDrawBoard->SetStrokeWidth(2);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 223, 256, 248);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 8);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 264, 256, 410);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 415, 5);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 420, 256, 443);
	// minutes
	gDrawBoard->SetStrokeWidth(4);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 240, 256, 420);
	// hours
	gDrawBoard->SetStrokeWidth(8);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawLine(256, 246, 256, 380);
	// cursors screw
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawCircle(256, 256, 6);
}

void CacheGlass() {

	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
	gDrawBoard->SetStrokeWidth(1);
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawPaths("M 162,447 C 365,540 564,309 419,118 Q 400,350 162,447 z");
	gClockSlots[gSlotsIndex++] = gDrawBoard->DrawPaths("M 256,43 C 83,45 4,231 62,340 Q 80,110 256,43 z");
}

void CacheClock() {

	gSlotsIndex = 0;
	gDrawBoard->SetTargetMode(G_CACHE_MODE);
	gDrawBoard->SetCacheBank(gCacheBank);
	gDrawBoard->SetCustomRenderingQuality(0.01);
	CacheBack();
	CacheCursors();
	CacheGlass();
}

void KillApp() {

	if (gKernel)
		delete gKernel;
	if (gExtManager)
		delete gExtManager;
	if (gDrawBoard)
		delete gDrawBoard;
}

int InitGL(GLvoid) {

	GDynArray<GKeyValue> colKeys;
	gKernel = new GKernel();

	// shadow gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.85, GVector4((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.95, GVector4((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)0.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.3, (GReal)0.3, (GReal)0.3, (GReal)0.0)));
	gShadowGrad = gDrawBoard->CreateRadialGradient(GPoint2(260, 250), GPoint2(260, 250), 270, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// quadrant background gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.86, (GReal)0.87, (GReal)0.88, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.60, GVector4((GReal)0.76, (GReal)0.77, (GReal)0.78, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.36, (GReal)0.36, (GReal)0.40, (GReal)1.0)));
	gRadGrad1 = gDrawBoard->CreateRadialGradient(GPoint2(236, 276), GPoint2(235, 275), 256, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	// outer/inner bevel ring gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)1.0, (GReal)1.00, (GReal)1.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.40, GVector4((GReal)0.9, (GReal)0.90, (GReal)0.92, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.80, GVector4((GReal)0.46, (GReal)0.47, (GReal)0.50, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.26, (GReal)0.27, (GReal)0.30, (GReal)1.0)));
	gRadGrad2 = gDrawBoard->CreateRadialGradient(GPoint2(120, 390), GPoint2(-121, 591), 400, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	// milliseconds quadrant background gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.9, (GReal)0.95, (GReal)0.95, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.85, (GReal)0.85, (GReal)0.9, (GReal)0.5)));
	gRadGrad3 = gDrawBoard->CreateRadialGradient(GPoint2(340, 270), GPoint2(324, 280), 60, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	
	
	gCacheBank = gDrawBoard->CreateCacheBank();
	CacheClock();
	return TRUE;
}

void DrawBack() {

	GUInt32 i;
	GReal kk;
	GMatrix33 rot, ks_rot;

	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetFillCompOp(G_SRC_OVER_OP);

	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);
	gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);
	
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetFillGradient(gShadowGrad);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	

	gDrawBoard->SetFillGradient(gRadGrad1);
	gDrawBoard->SetStrokeGradient(gRadGrad1);
	gRadGrad1->SetMatrix(rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(3);
	gDrawBoard->SetStrokeGradient(gRadGrad2);
	gDrawBoard->SetFillGradient(gRadGrad3);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);

	gDrawBoard->SetFillEnabled(G_FALSE);
	
	gDrawBoard->SetStrokeOpacity(1.0);
	gDrawBoard->SetStrokeGradient(gRadGrad2);
	gRadGrad2->SetMatrix(rot);
	gDrawBoard->SetStrokeWidth(10);
	gDrawBoard->SetStrokeCompOp(G_SRC_OP);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	RotationToMatrix(rot, (GReal)G_PI, GPoint2(256, 256));
	gRadGrad2->SetMatrix(rot);
	gDrawBoard->SetStrokeWidth(35);
	gDrawBoard->SetStrokeCompOp(G_SRC_OP);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);


	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
	Identity(rot);
	gDrawBoard->SetStrokeEndCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeColor(0.11, 0.12, 0.13, 0.5);
	gDrawBoard->SetStrokeWidth(1);
	for (i = 0; i < 60; ++i) {
		if ((i % 5) != 0) {
			kk = (GReal)G_2PI - (((GReal)i / 60) * G_2PI);
			RotationToMatrix(rot, kk, GPoint2(256, 256));
			gDrawBoard->SetModelViewMatrix(rot);
			gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex]);
			Identity(rot);
		}
	}
	gSlotsIndex++;
	
	for (i = 0; i < 12; ++i) {
		kk = (GReal)G_2PI - (((GReal)i / 12) * G_2PI);
		RotationToMatrix(rot, kk, GPoint2(256, 256));
		RotationToMatrix(ks_rot, kk, GPoint2(356, 256));
		gDrawBoard->SetModelViewMatrix(rot);
		gDrawBoard->SetStrokeWidth(10);
		gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex]);
		gDrawBoard->SetModelViewMatrix(ks_rot);
		gDrawBoard->SetStrokeWidth(1);
		gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex + 1]);
		Identity(rot);
	}
	gSlotsIndex += 2;
	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
}

void DrawCursors(const GUInt32 Hours, const GUInt32 Minutes, const GUInt32 Seconds, const GUInt32 Milliseconds) {

	GReal hh, mm, ss, ks;
	GMatrix33 rot, ks_rot;

	if (Hours < 13)
		hh = G_2PI - ( ((GReal)Hours / 12 + (GReal)Minutes / 720) * G_2PI);
	else 
		hh = G_2PI - ( ((GReal)(Hours - 12) / 12 + (GReal)Minutes / 720) * G_2PI);
	mm = G_2PI - (((GReal)Minutes / 60 + (GReal)Seconds / 3600) * G_2PI);
	ss = G_2PI - (((GReal)Seconds / 60 + (GReal)Milliseconds / 60000) * G_2PI);
	ks = G_2PI - (((GReal)Milliseconds / 1000) * G_2PI);

	gDrawBoard->SetStrokePaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);

	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetFillColor(1.0, 1.0, 1.0, 1.0);
	gDrawBoard->SetFillEnabled(G_FALSE);


	// milliseconds	
	gDrawBoard->SetStrokeWidth(1);
	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetStrokeColor(0.2, 0.4, 0.8, 1.0);
	RotationToMatrix(ks_rot, ks, GPoint2(356,256));
	gDrawBoard->SetModelViewMatrix(ks_rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->SetFillEnabled(G_FALSE);


	// seconds	
	gDrawBoard->SetStrokeWidth(2);
	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetStrokeColor(0.8, 0.2, 0.1, 1.0);
	RotationToMatrix(rot, ss, GPoint2(256,256));
	gDrawBoard->SetModelViewMatrix(rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->SetFillEnabled(G_FALSE);
	Identity(rot);

	// minutes
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetStrokeColor(0.3, 0.3, 0.3, 1.0);
	RotationToMatrix(rot, mm, GPoint2(256,256));
	gDrawBoard->SetModelViewMatrix(rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	Identity(rot);

	// hours
	gDrawBoard->SetStrokeWidth(8);
	gDrawBoard->SetStrokeCompOp(G_SRC_OVER_OP);
	gDrawBoard->SetStrokeColor(0.25, 0.25, 0.25, 1.0);
	RotationToMatrix(rot, hh, GPoint2(256,256));
	gDrawBoard->SetModelViewMatrix(rot);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	Identity(rot);

	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetFillColor(0.25, 0.25, 0.25, 1.0);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
}

void DrawGlass() {

	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetModelViewMatrix(G_MATRIX_IDENTITY33);
	gDrawBoard->SetFillCompOp(G_PLUS_OP);
	gDrawBoard->SetFillPaintType(G_COLOR_PAINT_TYPE);
	gDrawBoard->SetFillColor(0.05, 0.05, 0.05, 1.0);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
	gDrawBoard->DrawCacheSlot(gClockSlots[gSlotsIndex++]);
}

void DrawClock() {

	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	gDrawBoard->SetStrokeStyle(G_SOLID_STROKE);
	gDrawBoard->SetStrokeJoinStyle(G_ROUND_JOIN);
	gDrawBoard->SetStrokeStartCapStyle(G_ROUND_CAP);
	gDrawBoard->SetStrokeEndCapStyle(G_BUTT_CAP);
	gDrawBoard->SetModelViewMatrix(G_MATRIX_IDENTITY33);

	gSlotsIndex = 0;
	DrawBack();

	SYSTEMTIME st;
	GetLocalTime(&st);

	DrawCursors(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	DrawGlass();
}

int DrawGLScene(GLvoid)	{

	gDrawBoard->Clear(1.0, 1.0, 1.0, 0.0, G_TRUE);
	DrawClock();
	gDrawBoard->Flush();
	return TRUE;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) {

	GUInt32 x, y, w, h;

	gDrawBoard->Viewport(x, y, w, h);
	gDrawBoard->SetViewport(x, y, width, height);
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
	
	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
	dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style

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
		8,											// 8bit Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		8,											// 8 bits Stencil Buffer
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

	if(!arbMultisampleSupported) {
		if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
			KillGLWindow();
			MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	else
		PixelFormat = arbMultisampleFormat;


	if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(hRC = wglCreateContext(hDC))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!wglMakeCurrent(hDC, hRC)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!arbMultisampleSupported) {
		if (!gExtManager)
			gExtManager = new GOpenglExt();

		if (InitMultisample(hInstance, hWnd, pfd)) {
			KillGLWindow();
			return CreateGLWindow(title, width, height, bits, fullscreenflag);
		}
	}

#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif
	gDrawBoard = new GOpenGLBoard(0, 0, width, height);
	gDrawBoard->SetProjection(0, 512, 0, 512);

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
			if (!HIWORD(wParam)) {
				active = TRUE;						// Program Is Active
			}
			else {
			}
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

	// Create Our OpenGL Window
	if (!CreateGLWindow("Amanith Clock", 256, 256, 16, fullscreen))
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
			if (keys[VK_ESCAPE])
				done = TRUE;
			else
			if (active) {
				DrawGLScene();
				SwapBuffers(hDC);
			}
		}
	}
	// Shutdown
	KillApp();
	KillGLWindow();				// Kill The Window
	return (int)(msg.wParam);	// Exit The Program
}
