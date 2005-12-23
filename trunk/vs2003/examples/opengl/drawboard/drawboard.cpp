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
bool doDraw = TRUE;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

// Amanith stuff
GKernel *gKernel = NULL;
GOpenglExt *gExtManager = NULL;	// OpenGL extensions manager
GOpenGLBoard *gDrawBoard = NULL;
GPixelMap *gImage = NULL;
GGradientDesc *gLinGrad1 = NULL, *gLinGrad2 = NULL, *gLinGrad3 = NULL;
GGradientDesc *gLinGradLogo1 = NULL, *gLinGradLogo2 = NULL, *gLinGradLogo3 = NULL;
GGradientDesc *gRadGrad1 = NULL, *gRadGrad2 = NULL, *gRadGrad3 = NULL, *gRadGrad4 = NULL;
GGradientDesc *gConGrad1 = NULL, *gConGrad2 = NULL, *gConGrad3 = NULL, *gConGrad4 = NULL;
GPatternDesc *gPattern = NULL;
GPatternDesc *gBackGround = NULL;
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
GUInt32 gTestSuite = 0;
GUInt32 gTestIndex = 0;
GBool gDrawBackGround = G_TRUE;
GReal gRandAngle = 0;
GReal gRandScaleX = 1;
GReal gRandScaleY = 1;
GRenderingQuality gRenderingQuality = G_HIGH_RENDERING_QUALITY;
GBool gUseShaders = G_TRUE;

#include "test_color.h"
#include "test_lineargradient.h"
#include "test_radialgradientin.h"
#include "test_radialgradientout.h"
#include "test_conicalgradientin.h"
#include "test_conicalgradientout.h"
#include "test_pattern.h"
#include "test_stroking.h"
#include "test_masks.h"
#include "test_geometries.h"

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
			WGL_ALPHA_BITS_ARB, 0,
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

void InitApp() {
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

	gKernel = new GKernel();
	gImage = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);

	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";

	// initialize random system
	GMath::SeedRandom();

	gDrawBoard->SetRenderingQuality(gRenderingQuality);

	GString s;
	GError err;
	GDynArray<GKeyValue> colKeys;

	// color gradients
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.4, (GReal)0.0, (GReal)0.5, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.25, GVector4((GReal)0.9, (GReal)0.5, (GReal)0.1, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.50, GVector4((GReal)0.8, (GReal)0.8, (GReal)0.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.75, GVector4((GReal)0.0, (GReal)0.3, (GReal)0.5, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.4, (GReal)0.0, (GReal)0.5, (GReal)1.0)));
	gLinGrad1 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.30, GVector4((GReal)0.540, (GReal)0.138, (GReal)0.757, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.60, GVector4((GReal)1.000, (GReal)0.500, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.70, GVector4((GReal)0.980, (GReal)0.950, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	gRadGrad1 = gDrawBoard->CreateRadialGradient(GPoint2(90, 58), GPoint2(150, 118), 110, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad3 = gDrawBoard->CreateRadialGradient(GPoint2(-90, -70), GPoint2(-130, -130), 100, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gConGrad1 = gDrawBoard->CreateConicalGradient(GPoint2(120, 88), GPoint2(180, 88), colKeys, G_HERMITE_COLOR_INTERPOLATION);
	gConGrad3 = gDrawBoard->CreateConicalGradient(GPoint2(0, 0), GPoint2(20, 0), colKeys, G_HERMITE_COLOR_INTERPOLATION);

	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.4, (GReal)0.0, (GReal)0.5, (GReal)1.00)));
	colKeys.push_back(GKeyValue((GReal)0.25, GVector4((GReal)0.9, (GReal)0.5, (GReal)0.1, (GReal)0.25)));
	colKeys.push_back(GKeyValue((GReal)0.50, GVector4((GReal)0.8, (GReal)0.8, (GReal)0.0, (GReal)0.50)));
	colKeys.push_back(GKeyValue((GReal)0.75, GVector4((GReal)0.0, (GReal)0.3, (GReal)0.5, (GReal)0.75)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.4, (GReal)0.0, (GReal)0.5, (GReal)1.00)));
	gLinGrad2 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	GDouble kd = GMath::AngleConversion(1.0, G_RADIAN_UNIT, G_DEGREE_UNIT);
	GFloat kf = GMath::AngleConversion(0.5f, G_RADIAN_UNIT, G_DEGREE_UNIT);

	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.30, GVector4((GReal)0.540, (GReal)0.138, (GReal)0.757, (GReal)0.7)));
	colKeys.push_back(GKeyValue((GReal)0.60, GVector4((GReal)1.000, (GReal)0.500, (GReal)0.000, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)0.70, GVector4((GReal)0.980, (GReal)0.950, (GReal)0.000, (GReal)0.5)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.171, (GReal)0.680, (GReal)0.800, (GReal)1.0)));
	gRadGrad2 = gDrawBoard->CreateRadialGradient(GPoint2(90, 58), GPoint2(150, 118), 110, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad4 = gDrawBoard->CreateRadialGradient(GPoint2(-90, -70), GPoint2(-130, -130), 100, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gConGrad2 = gDrawBoard->CreateConicalGradient(GPoint2(120, 88), GPoint2(180, 88), colKeys, G_HERMITE_COLOR_INTERPOLATION);
	gConGrad4 = gDrawBoard->CreateConicalGradient(GPoint2(0, 0), GPoint2(20, 0), colKeys, G_HERMITE_COLOR_INTERPOLATION);

	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.0, (GReal)0.0, (GReal)0.0, (GReal)0.0)));
	gLinGrad3 = gDrawBoard->CreateLinearGradient(GPoint2(560, 20), GPoint2(760, 20), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// background
	s = gDataPath + "background.png";
	err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err == G_NO_ERROR) {
		gBackGround = gDrawBoard->CreatePattern(gImage, G_LOW_IMAGE_QUALITY, G_REPEAT_TILE);
		gBackGround->SetLogicalWindow(GPoint2(0, 0), GPoint2(16, 16));
	}
	else
		gBackGround = NULL;

	// pattern
	s = gDataPath + "spiral.png";
	err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err == G_NO_ERROR) {
		gPattern = gDrawBoard->CreatePattern(gImage, G_HIGH_IMAGE_QUALITY, G_REPEAT_TILE);
		gPattern->SetLogicalWindow(GPoint2(-64, -64), GPoint2(64, 64));
	}
	else
		gPattern = NULL;

	// dashes
	gDrawBoard->SetStrokeDashPhase(0);
	GDynArray<GReal> pat;
	pat.push_back(10);
	pat.push_back(35);
	pat.push_back(30);
	pat.push_back(35);
	gDrawBoard->SetStrokeDashPattern(pat);

	// gradients for logo
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)1.0, (GReal)1.0, (GReal)0.44, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)1.0, (GReal)1.0, (GReal)1.00, (GReal)1.0)));
	gLinGradLogo1 = gDrawBoard->CreateLinearGradient(GPoint2(306, 280), GPoint2(460, 102), colKeys, G_LINEAR_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)1.0, (GReal)0.215, (GReal)0.172, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)1.0, (GReal)0.500, (GReal)0.000, (GReal)1.0)));
	gLinGradLogo2 = gDrawBoard->CreateLinearGradient(GPoint2(276, 438), GPoint2(580, 206), colKeys, G_LINEAR_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)1.0, (GReal)1.0, (GReal)1.0, (GReal)0.0)));
	gLinGradLogo3 = gDrawBoard->CreateLinearGradient(GPoint2(300, 460), GPoint2(417, 330), colKeys, G_LINEAR_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// now lets see if some bitmap file formats are present for load/save
	GDynArray<GImpExpFeature> features;
	err = gKernel->ImpExpFeatures(G_PIXELMAP_CLASSID, features);
	if (err == G_NO_ERROR) {
		// look for PNG support
		for (GUInt32 i = 0; i < (GUInt32)features.size(); i++) {
			if (features[i].FormatSupported("png", G_FALSE, G_TRUE))
				gScreenShotFileName = "./shot.png";
		}
		// look for JPEG support, if PNG format is not present
		if (gScreenShotFileName.length() <= 0) {
			for (GUInt32 i = 0; i < (GUInt32)features.size(); i++) {
				if (features[i].FormatSupported("jpeg", G_FALSE, G_TRUE))
					gScreenShotFileName = "./shot.jpg";
			}
		}
	}
	return TRUE;
}

int DrawGLScene(GLvoid)	{

	gDrawBoard->Clear(1.0, 1.0, 1.0, G_TRUE);

	GMatrix33 k;
	Identity(k);
	gDrawBoard->SetModelViewMatrix(k);

	if (gDrawBackGround) {

		gDrawBoard->SetStrokeEnabled(G_FALSE);
		gDrawBoard->SetFillEnabled(G_TRUE);
		gDrawBoard->SetFillPattern(gBackGround);
		gDrawBoard->SetFillColor(GVector4(0, 0, 0, 1));
		gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
		gDrawBoard->DrawRectangle(GPoint2(0, 0), GPoint2(800, 600));
	}

	switch (gTestSuite) {

		case 0:
			TestColor(gTestIndex);
			break;
		case 1:
			TestLinearGradient(gTestIndex, gRandAngle, gRandScaleX);
			break;
		case 2:
			TestRadialGradientIn(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 3:
			TestRadialGradientOut(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 4:
			TestConicalGradientIn(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 5:
			TestConicalGradientOut(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 6:
			TestPattern(gTestIndex, gRandAngle, gRandScaleX, gRandScaleY);
			break;
		case 7:
			TestStroke(gTestIndex);
			break;
		case 8:
			TestMasks(gTestIndex);
			break;
		case 9:
			TestGeometries(gTestIndex);
			break;
		default:
			TestColor(gTestIndex);
	}

	gDrawBoard->Flush();
	return TRUE;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) {

	GUInt32 x, y, w, h;

	gDrawBoard->Viewport(x, y, w, h);
	gDrawBoard->SetViewport(x, y, width, height);
	doDraw = TRUE;
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
		0,											// No Alpha Buffer
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

	gDrawBoard = new GOpenGLBoard(0, 0, width, height);

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
				doDraw = TRUE;
			}
			else {
				//active = FALSE;						// Program Is No Longer Active
				doDraw = TRUE;
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
		case WM_ERASEBKGND:
		{
			doDraw = TRUE;
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
	if (!CreateGLWindow("OpenGL drawboard example - Press F1 for help", 800, 600, 16, fullscreen))
		return 0;									// Quit If Window Was Not Created

	// init application
	InitApp();
	doDraw = TRUE;

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
				if (doDraw) {
					doDraw = FALSE;
					DrawGLScene();
					SwapBuffers(hDC);
				}
			}

			if (keys[VK_F1]) {						// Is F1 Being Pressed?
				keys[VK_F1] = FALSE;
				s = "F2: contextual example description\n";
				s += "0..9: Toggle draw test\n";
				s += "PageUp/PageDown: Switch draw sheet\n";
				s += "B: Toggle background\n";
				s += "R: Switch rendering quality (low/normal/high)\n";
				s += "S: Enable/Disable shaders (for gradients) if supported\n";
				s += "T: Take a screenshot\n";
				s += "Space: Change matrix (valid for gradient and pattern tests)\n";
				MessageBox(NULL, StrUtils::ToAscii(s), "Command keys", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
			}
			// F2 key, contextual description
			if (keys[VK_F2]) {
				keys[VK_F2] = FALSE;
				switch (gTestSuite) {
					case 0:
						s = "This board shows simple color filling with opaque and transparent colors.\n";
						s += "In the leftmost column colors are 100% opaque, in the middle column colors are ";
						s += "66% opaque\n and in the rightmost column colors are 33% opaque.\n";
						s += "Compositing is done drawing smaller rectangles over larger ones.";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 1:
						s = "This board shows simple linear gradient filling.\n\n";
						s += "In the rows different color interpolation schema are shown:\n";
						s += "Topmost row: CONSTANT interpolation.\n";
						s += "Middle row: LINEAR interpolation.\n";
						s += "Lower row: HERMITE interpolation.\n\n";
						s += "In the columns different spread methods are shown:\n";
						s += "Leftmost column: PAD spread method.\n";
						s += "Middle column: REPEAT spread method.\n";
						s += "Rightmost column: REFLECT spread method.\n\n";
						s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
						s += "100% opaque fill and 100% opaque color keys.\n";
						s += "50% opaque fill and 100% opaque color keys.\n";
						s += "100% opaque fill and some non 100% opaque color keys.\n";
						s += "50% opaque fill and some non 100% opaque color keys.";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 2:
						s = "This board shows simple radial gradient filling, with focus point inside filled shapes.\n\n";
						s += "In the rows different color interpolation schema are shown:\n";
						s += "Topmost row: CONSTANT interpolation.\n";
						s += "Middle row: LINEAR interpolation.\n";
						s += "Lower row: HERMITE interpolation (available only if fragment programs are supported).\n\n";
						s += "In the columns different spread methods are shown:\n";
						s += "Leftmost column: PAD spread method.\n";
						s += "Middle column: REPEAT spread method.\n";
						s += "Rightmost column: REFLECT spread method.\n\n";
						s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
						s += "100% opaque fill and 100% opaque color keys.\n";
						s += "50% opaque fill and 100% opaque color keys.\n";
						s += "100% opaque fill and some non 100% opaque color keys.\n";
						s += "50% opaque fill and some non 100% opaque color keys.";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 3:
						s = "This board shows simple radial gradient filling, with focus point outside filled shapes.\n\n";
						s += "In the rows different color interpolation schema are shown:\n";
						s += "Topmost row: CONSTANT interpolation.\n";
						s += "Middle row: LINEAR interpolation.\n";
						s += "Lower row: HERMITE interpolation (available only if fragment programs are supported).\n\n";
						s += "In the columns different spread methods are shown:\n";
						s += "Leftmost column: PAD spread method.\n";
						s += "Middle column: REPEAT spread method.\n";
						s += "Rightmost column: REFLECT spread method.\n\n";
						s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
						s += "100% opaque fill and 100% opaque color keys.\n";
						s += "50% opaque fill and 100% opaque color keys.\n";
						s += "100% opaque fill and some non 100% opaque color keys.\n";
						s += "50% opaque fill and some non 100% opaque color keys.";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 4:
						s = "This board shows simple conical gradient filling, with center point inside filled shapes.\n\n";
						s += "In the rows different color interpolation schema are shown:\n";
						s += "Topmost row: CONSTANT interpolation.\n";
						s += "Middle row: LINEAR interpolation.\n";
						s += "Lower row: HERMITE interpolation.\n\n";
						s += "In the columns different spread methods are shown:\n";
						s += "Leftmost column: PAD spread method.\n";
						s += "Middle column: REPEAT spread method.\n";
						s += "Rightmost column: REFLECT spread method.\n\n";
						s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
						s += "100% opaque fill and 100% opaque color keys.\n";
						s += "50% opaque fill and 100% opaque color keys.\n";
						s += "100% opaque fill and some non 100% opaque color keys.\n";
						s += "50% opaque fill and some non 100% opaque color keys.";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 5:
						s = "This board shows simple conical gradient filling, with center point outside filled shapes.\n\n";
						s += "In the rows different color interpolation schema are shown:\n";
						s += "Topmost row: CONSTANT interpolation.\n";
						s += "Middle row: LINEAR interpolation.\n";
						s += "Lower row: HERMITE interpolation.\n\n";
						s += "In the columns different spread methods are shown:\n";
						s += "Leftmost column: PAD spread method.\n";
						s += "Middle column: REPEAT spread method.\n";
						s += "Rightmost column: REFLECT spread method.\n\n";
						s += "Use PageUp/PageDown keys to switch to transparency modes:\n";
						s += "100% opaque fill and 100% opaque color keys.\n";
						s += "50% opaque fill and 100% opaque color keys.\n";
						s += "100% opaque fill and some non 100% opaque color keys.\n";
						s += "50% opaque fill and some non 100% opaque color keys.";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 6:
						s = "This board shows simple pattern filling.\n\n";
						s += "In the rows different opacity percentage are shown:\n";
						s += "Topmost row: 33% opaque filling.\n";
						s += "Middle row: 66% opaque filling.\n";
						s += "Lower row: 100% opaque filling.\n\n";
						s += "In the columns different pattern tiling modes are shown:\n";
						s += "Leftmost column: PAD tiling mode.\n";
						s += "Middle column: REPEAT tiling mode.\n";
						s += "Rightmost column: REFLECT tiling mode.\n\n";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 7:
						s = "This board shows all supported stroking features.\n\n";
						s += "Supported stroke styles are SOLID and DASHED (with also initial phase support).\n";
						s += "Supported join types are BEVEL, MITER and ROUND.\n";
						s += "Supported caps types (they could be used independently for start and end cap) are BUTT, SQUARE and ROUND.\n";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 8:
						s = "This board shows clip masks and group opacity (you can switch between them using PageUp/PageDown keys).\n\n";
						s += "Clip masks sheet description:\n";
						s += "Leftmost column: unclipped shapes.\n";
						s += "Middle column: clip masks (the darker color is the intersection of the masks).\n";
						s += "Rightmost column: clipped shapes, using masks in \"and\" (intersection).\n\n";
						s += "Group opacity sheet description:\n";
						s += "Leftmost column: shapes are drawn without group opacity.\n";
						s += "Middle column: shapes are drawn using (in different modes) group opacity.\n";
						s += "Rightmost column: shapes are drawn with group opacity (like in the middle column) over a background.\n";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
					case 9:
						s = "This board shows some supported geometric primitives (you can switch between them using PageUp/PageDown keys).\n\n";
						s += "Stroke primitives sheet description:\n";
						s += "Lower row: a line, a quadratic Bezier curve and a cubic Bezier curve.\n";
						s += "Middle row: 2 elliptical arcs (build using different constructors) and a polyline.\n";
						s += "Topmost row: a round rectangle, a circle and an ellipse.\n\n";
						s += "Amanith logo sheet description:\n";
						s += "Here's Amanith mushroom, it's constructed directly with SVG paths.";
						MessageBox(NULL, StrUtils::ToAscii(s), "Current board description", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
						break;
				}
			}
			// 1 key
			if (keys[49]) {
				keys[49] = FALSE;
				gTestSuite = 0;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 2 key
			if (keys[50]) {
				keys[50] = FALSE;
				gTestSuite = 1;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 3 key
			if (keys[51]) {
				keys[51] = FALSE;
				gTestSuite = 2;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 4 key
			if (keys[52]) {
				keys[52] = FALSE;
				gTestSuite = 3;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 5 key
			if (keys[53]) {
				keys[53] = FALSE;
				gTestSuite = 4;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 6 key
			if (keys[54]) {
				keys[54] = FALSE;
				gTestSuite = 5;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 7 key
			if (keys[55]) {
				keys[55] = FALSE;
				gTestSuite = 6;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 8 key
			if (keys[56]) {
				keys[56] = FALSE;
				gTestSuite = 7;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 9 key
			if (keys[57]) {
				keys[57] = FALSE;
				gTestSuite = 8;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// 0 key
			if (keys[48]) {
				keys[48] = FALSE;
				gTestSuite = 9;
				gTestIndex = 0;
				doDraw = TRUE;
			}
			// B key
			if (keys[66]) {
				keys[66] = FALSE;
				if (gDrawBackGround)
					gDrawBackGround = G_FALSE;
				else
					gDrawBackGround = G_TRUE;
				doDraw = TRUE;
			}
			// R key
			if (keys[82]) {
				keys[82] = FALSE;
				if (gRenderingQuality == G_LOW_RENDERING_QUALITY)
					gRenderingQuality = G_NORMAL_RENDERING_QUALITY;
				else
					if (gRenderingQuality == G_NORMAL_RENDERING_QUALITY)
						gRenderingQuality = G_HIGH_RENDERING_QUALITY;
					else
						gRenderingQuality = G_LOW_RENDERING_QUALITY;
				gDrawBoard->SetRenderingQuality(gRenderingQuality);
				doDraw = TRUE;
			}
			// S key
			if (keys[83]) {
				keys[83] = FALSE;
				if (gUseShaders) {
					gUseShaders = G_FALSE;
					gDrawBoard->DisableShaders(G_TRUE);
				}
				else {
					gUseShaders = G_TRUE;
					gDrawBoard->DisableShaders(G_FALSE);
				}
				doDraw = TRUE;
			}
			// T key (take a screenshot)
			if (keys[84]) {
				keys[84] = FALSE;
				if (gScreenShotFileName.length() > 0) {
					GPixelMap p(gKernel);
					gDrawBoard->ScreenShot(p);
					p.Save(StrUtils::ToAscii(gScreenShotFileName));
				}
			}

			// PageUp
			if (keys[VK_PRIOR]) {
				keys[VK_PRIOR] = FALSE;
				gTestIndex++;
				doDraw = TRUE;
			}
			// PageDown
			if (keys[VK_NEXT]) {
				keys[VK_NEXT] = FALSE;
				if (gTestIndex > 0)
					gTestIndex--;
				doDraw = TRUE;
			}
			// Space
			if (keys[VK_SPACE]) {
				keys[VK_SPACE] = FALSE;
				gRandAngle = GMath::RangeRandom((GReal)0, (GReal)G_2PI);
				if (gTestSuite == 6) {
					gRandScaleX = GMath::RangeRandom((GReal)0.1, (GReal)1.5);
					gRandScaleY = GMath::RangeRandom((GReal)0.1, (GReal)1.5);
				}
				else {
					gRandScaleX = GMath::RangeRandom((GReal)0.33, (GReal)3.0);
					gRandScaleY = GMath::RangeRandom((GReal)0.33, (GReal)3.0);
				}
				doDraw = TRUE;
			}
		}
	}
	// Shutdown
	KillGLWindow();									// Kill The Window
	KillApp();
	return (int)(msg.wParam);							// Exit The Program
}
