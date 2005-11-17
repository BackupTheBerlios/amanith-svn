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

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

// Amanith stuff
GKernel *gKernel = NULL;
GOpenglExt *gExtManager = NULL;	// extensions manager
GOpenGLBoard *gDrawBoard = NULL;
GPixelMap *gImage = NULL;
GGradientDesc *gLinGrad1 = NULL, *gLinGrad2 = NULL, *gLinGrad3 = NULL;
GGradientDesc *gRadGrad1 = NULL, *gRadGrad2 = NULL, *gRadGrad3 = NULL, *gRadGrad4 = NULL;
GPatternDesc *gPattern = NULL;

bool arbMultisampleSupported = false;
int arbMultisampleFormat = 0;
bool activateFSAA = true;

// InitMultisample: Used To Query The Multisample Frequencies
bool InitMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pfd)
{  

	// using Amanith OpenGL extension manager is just a matter of test function pointer...
	if (!wglChoosePixelFormatARB) 
	{
		arbMultisampleSupported = false;
		return false;
	}

	// Get Our Current Device Context
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
			WGL_SAMPLES_ARB, 6,
			0, 0
	};
	// First We Check To See If We Can Get A Pixel Format For 6 Samples
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;	
		return arbMultisampleSupported;
	}

	// Our Pixel Format With 6 Samples Failed, Test For 4 Samples
	iAttributes[19] = 4;
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;	 
		return arbMultisampleSupported;
	}

	// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
	iAttributes[19] = 2;
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
	// If We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;	
		return arbMultisampleSupported;
	}

	// Return The Valid Format
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
	GString dataPath = SysUtils::AmanithPath();
	if (dataPath.length() > 0)
		dataPath += "data/";

	GString s = dataPath + "spiral.png";
	GError err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err != G_NO_ERROR)
		abort();


	gDrawBoard->SetRenderingQuality(G_HIGH_RENDERING_QUALITY);
	gDrawBoard->SetImageQuality(G_HIGH_IMAGE_QUALITY);

	gPattern = gDrawBoard->CreatePattern(gImage, G_REPEAT_TILE);
	gPattern->SetLogicalWindow(GPoint2(-64, -64), GPoint2(64, 64));

	GDynArray<GKeyValue> colKeys;

	// linear gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue(0.00, GVector4(0.4, 0.0, 0.5, 1.0)));
	colKeys.push_back(GKeyValue(0.25, GVector4(0.9, 0.5, 0.1, 1.0)));
	colKeys.push_back(GKeyValue(0.50, GVector4(0.8, 0.8, 0.0, 1.0)));
	colKeys.push_back(GKeyValue(0.75, GVector4(0.0, 0.3, 0.5, 1.0)));
	colKeys.push_back(GKeyValue(1.00, GVector4(0.4, 0.0, 0.5, 1.0)));
	gLinGrad1 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad1 = gDrawBoard->CreateRadialGradient(GPoint2(90, 58), GPoint2(150, 118), 110, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad3 = gDrawBoard->CreateRadialGradient(GPoint2(-160, -140), GPoint2(-200, -200), 100, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	colKeys.clear();
	colKeys.push_back(GKeyValue(0.00, GVector4(0.4, 0.0, 0.5, 1.00)));
	colKeys.push_back(GKeyValue(0.25, GVector4(0.9, 0.5, 0.1, 0.25)));
	colKeys.push_back(GKeyValue(0.50, GVector4(0.8, 0.8, 0.0, 0.50)));
	colKeys.push_back(GKeyValue(0.75, GVector4(0.0, 0.3, 0.5, 0.75)));
	colKeys.push_back(GKeyValue(1.00, GVector4(0.4, 0.0, 0.5, 1.00)));
	gLinGrad2 = gDrawBoard->CreateLinearGradient(GPoint2(80, 48), GPoint2(160, 128), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad2 = gDrawBoard->CreateRadialGradient(GPoint2(90, 58), GPoint2(150, 118), 110, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);
	gRadGrad4 = gDrawBoard->CreateRadialGradient(GPoint2(-160, -140), GPoint2(-200, -200), 100, colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	return TRUE;
}

int DrawGLScene(GLvoid)	{

	gDrawBoard->Clear(1.0, 1.0, 1.0, G_TRUE);
	gDrawBoard->SetTargetMode(G_COLOR_MODE);

	GMatrix33 m;

	//----- FILL LINEAR GRADIENT TEST SUITE ---------------------------------

	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);

	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokeWidth(4);
	gDrawBoard->SetStrokeColor(GVector4(0.0, 0.0, 0.0, 1.000));

	// --------------------------------------------------------------
	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 1.000));
	gPattern->SetTilingMode(G_PAD_TILE);
	TranslationToMatrix(m, GVector2(+140,+106));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(20, 18), GPoint2(260, 194));
	gPattern->SetTilingMode(G_REPEAT_TILE);
	TranslationToMatrix(m, GVector2(+400,+106));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(280, 18), GPoint2(520, 194));
	gPattern->SetTilingMode(G_REFLECT_TILE);
	TranslationToMatrix(m, GVector2(+660,+106));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(540, 18), GPoint2(780, 194));

	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.666));
	gPattern->SetTilingMode(G_PAD_TILE);
	TranslationToMatrix(m, GVector2(+140,+300));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(20, 212), GPoint2(260, 388));
	gPattern->SetTilingMode(G_REPEAT_TILE);
	TranslationToMatrix(m, GVector2(+400,+300));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(280, 212), GPoint2(520, 388));
	gPattern->SetTilingMode(G_REFLECT_TILE);
	TranslationToMatrix(m, GVector2(+660,+300));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(540, 212), GPoint2(780, 388));

	gDrawBoard->SetFillColor(GVector4(0.0, 0.0, 0.0, 0.333));
	gPattern->SetTilingMode(G_PAD_TILE);
	TranslationToMatrix(m, GVector2(+140,+494));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(20, 406), GPoint2(260, 582));
	gPattern->SetTilingMode(G_REPEAT_TILE);
	TranslationToMatrix(m, GVector2(+400,+494));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(280, 406), GPoint2(520, 582));
	gPattern->SetTilingMode(G_REFLECT_TILE);
	TranslationToMatrix(m, GVector2(+660,+494));
	gPattern->SetMatrix(m);
	gDrawBoard->SetFillPattern(gPattern);
	gDrawBoard->DrawRectangle(GPoint2(540, 406), GPoint2(780, 582));

	// --------------------------------------------------------------
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

	if(!arbMultisampleSupported)
	{
		if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
			KillGLWindow();
			MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	else
		PixelFormat = arbMultisampleFormat;


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

	if (!wglMakeCurrent(hDC, hRC)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!arbMultisampleSupported)
	{
		if (!gExtManager)
			gExtManager = new GOpenglExt();

		if (InitMultisample(hInstance, hWnd, pfd))
		{
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

	// Create Our OpenGL Window
	if (!CreateGLWindow("Color animation example - Press F1 for help", 1006, 768, 16, fullscreen))
		return 0;									// Quit If Window Was Not Created

	// init application
	InitApp();

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
		}
	}
	// Shutdown
	KillGLWindow();									// Kill The Window
	KillApp();
	return (int)(msg.wParam);							// Exit The Program
}
