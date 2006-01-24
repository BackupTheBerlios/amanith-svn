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
#include <amanith/gkernel.h>
#include <amanith/2d/gpixelmap.h>
#include <amanith/rendering/gopenglboard.h>
#include <amanith/geometry/gxformconv.h>
#include <windows.h>
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
GGradientDesc *gLinGrad = NULL;
GPatternDesc *gBackGround = NULL;
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


void KillApp() {

	if (gKernel) {
		delete gKernel;
		gKernel = NULL;
	}
	if (gExtManager) {
		delete gExtManager;
		gExtManager = NULL;
	}
	if (gDrawBoard) {
		delete gDrawBoard;
		gDrawBoard = NULL;
	}
}

void BuildMatrices() {

	GMatrix33 scl, rot, trans;

	ScaleToMatrix(scl, gZoomFactor, GPoint2(0, 0));
	RotationToMatrix(rot, gRotAngle, GPoint2(0, 0));
	TranslationToMatrix(trans, gTranslation);

	gModelView = trans * (rot * scl);
	gGradientMatrix = gModelView;
}

GString CompOpToString(const GCompositingOperation CompOp) {

	switch(CompOp) {

		case G_CLEAR_OP:
			return("Clear");
		case G_SRC_OP:
			return("Src");
		case G_DST_OP:
			return("Dst");
		case G_SRC_OVER_OP:
			return("SrcOver");
		case G_DST_OVER_OP:
			return("DstOver");
		case G_SRC_IN_OP:
			return("SrcIn");
		case G_DST_IN_OP:
			return("DstIn");
		case G_SRC_OUT_OP:
			return("SrcOut");
		case G_DST_OUT_OP:
			return("DstOut");
		case G_SRC_ATOP_OP:
			return("SrcATop");
		case G_DST_ATOP_OP:
			return("DstATop");
		case G_XOR_OP:
			return("Xor");
		case G_PLUS_OP:
			return("Plus");
		case G_MULTIPLY_OP:
			return("Multiply");
		case G_SCREEN_OP:
			return("Screen");
		case G_OVERLAY_OP:
			return("Overlay");
		case G_DARKEN_OP:
			return("Darken");
		case G_LIGHTEN_OP:
			return("Lighten");
		case G_COLOR_DODGE_OP:
			return("ColorDodge");
		case G_COLOR_BURN_OP:
			return("ColorBurn");
		case G_HARD_LIGHT_OP:
			return("HardLight");
		case G_SOFT_LIGHT_OP:
			return("SoftLight");
		case G_DIFFERENCE_OP:
			return("Difference");
		case G_EXCLUSION_OP:
			return("Exclusion");
		default:
			return("SrcOver");
	}
}

void DrawTitle() {

	GString s;

	s = "FILL: " + CompOpToString(gFillCompOp) + " (" + StrUtils::ToString(gFillOpacity * 100, "%3.0f") + "%)";
	s += " - STROKE: " +CompOpToString(gStrokeCompOp) + " (" + StrUtils::ToString(gStrokeOpacity * 100, "%3.0f") + "%)";
	s += " [F1 help]";

	SetWindowText(hWnd, StrUtils::ToAscii(s));
}

int InitGL(GLvoid) {

#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	gKernel = new GKernel();
	gImage = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);

	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";

	gRotAngle = 0;
	gRotationVel = (GReal)0.05;
	gStrokeOpacity = 1;
	gFillOpacity = 1;
	gZoomFactor = 2;
	gAnim = G_FALSE;
	gTranslation.Set(256, 256);
	gStrokeCompOp = G_SRC_OVER_OP;
	gFillCompOp = G_SRC_OVER_OP;

	gDrawBoard->SetRenderingQuality(G_HIGH_RENDERING_QUALITY);

	GString s;
	GError err;
	GDynArray<GKeyValue> colKeys;

	// color gradient
	colKeys.clear();
	colKeys.push_back(GKeyValue((GReal)0.00, GVector4((GReal)0.95, (GReal)0.92, (GReal)0.0, (GReal)1.0)));
	colKeys.push_back(GKeyValue((GReal)1.00, GVector4((GReal)0.1, (GReal)0.3, (GReal)0.8, (GReal)0.7)));
	gLinGrad = gDrawBoard->CreateLinearGradient(GPoint2(-60, -44), GPoint2(60, 44), colKeys, G_HERMITE_COLOR_INTERPOLATION, G_PAD_COLOR_RAMP_SPREAD);

	// background
	s = gDataPath + "compground.png";
	err = gImage->Load(StrUtils::ToAscii(s), "expandpalette=true");
	if (err == G_NO_ERROR) {
		gBackGround = gDrawBoard->CreatePattern(gImage, G_LOW_IMAGE_QUALITY, G_REPEAT_TILE);
		gBackGround->SetLogicalWindow(GPoint2(0, 0), GPoint2(512, 512));
	}
	else
		gBackGround = NULL;

	gDrawBoard->SetStrokeWidth(10);
	gDrawBoard->SetStrokeGradient(gLinGrad);
	gDrawBoard->SetFillGradient(gLinGrad);
	gDrawBoard->SetFillPattern(gBackGround);
	DrawTitle();

	return TRUE;
}


int DrawGLScene(GLvoid)	{

	if (gAnim)
		gRotAngle += gRotationVel;

	gDrawBoard->Clear(1.0, 1.0, 1.0, 0.0, G_TRUE);
	gDrawBoard->SetTargetMode(G_COLOR_MODE);
	BuildMatrices();

	// draw background
	gDrawBoard->SetModelViewMatrix(G_MATRIX_IDENTITY33);
	gDrawBoard->SetStrokeEnabled(G_FALSE);
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_PATTERN_PAINT_TYPE);
	gDrawBoard->SetFillOpacity(1.0);
	gDrawBoard->SetFillCompOp(G_SRC_OP);
	gDrawBoard->DrawRectangle(GPoint2(0, 0), GPoint2(512, 512));

	// draw path
	gDrawBoard->SetStrokeOpacity(gStrokeOpacity);
	gDrawBoard->SetStrokeEnabled(G_TRUE);
	gDrawBoard->SetStrokePaintType(G_GRADIENT_PAINT_TYPE);

	gDrawBoard->SetFillOpacity(gFillOpacity);
	gDrawBoard->SetFillEnabled(G_TRUE);
	gDrawBoard->SetFillPaintType(G_GRADIENT_PAINT_TYPE);

	gDrawBoard->SetModelViewMatrix(gModelView);
	gLinGrad->SetMatrix(gGradientMatrix);

	gDrawBoard->SetStrokeCompOp(gStrokeCompOp);
	gDrawBoard->SetFillCompOp(gFillCompOp);

	gDrawBoard->DrawRoundRectangle(GPoint2(-64, -48), GPoint2(64, 48), 16, 16);

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
	
	// Window Extended Style
	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle = WS_OVERLAPPED | WS_SYSMENU;// Windows Style

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
								NULL)))								// Don't Pass Anything To WM_CREATE
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
		case WM_LBUTTONDOWN: {

			GInt32 i = gFillCompOp;
			i++;
			if (i > G_EXCLUSION_OP)
				i = G_CLEAR_OP;
			gFillCompOp = (GCompositingOperation)i;
			DrawTitle();
			return 0;								// Jump Back
		}
		case WM_RBUTTONDOWN: {

			GInt32 i = gStrokeCompOp;
			i++;
			if (i > G_EXCLUSION_OP)
				i = G_CLEAR_OP;
			gStrokeCompOp = (GCompositingOperation)i;
			DrawTitle();
			return 0;								// Jump Back
		}
		case WM_MOUSEWHEEL: {

			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (zDelta > 0) {
				if (gZoomFactor > (GReal)0.1)
					gZoomFactor *= (GReal)0.95;
			}
			else {
				if (gZoomFactor < (GReal)5.0)
					gZoomFactor /= (GReal)0.95;
			}
			return 0;								// Jump Back
		}
		case WM_MOUSEMOVE: {

			int xPos = ((int)(short)LOWORD(lParam)); 
			int yPos = ((int)(short)HIWORD(lParam)); 

			GPoint<GInt32, 2> p(xPos, 512 - 20 - yPos);
			gTranslation = gDrawBoard->PhysicalToLogical(p);

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
	if (!CreateGLWindow("Amanith compositing example - Press F1 for help", 512, 512, 16, fullscreen))
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

			if (keys[VK_F1]) {						// Is F1 Being Pressed?
				keys[VK_F1] = FALSE;
				s = "B: Toggle rotation.\n";
				s += "N/M: Decrease/Increation rotation velocity.\n";			
				s += "F/G: Decrease/Increase global fill opacity.\n";
				s += "S/D: Decrease/Increase global stroke opacity.\n";
				s += "Mouse left button: change fill compositing operation.\n";
				s += "Mouse right button: change stroke compositing operation.\n";
				s += "Mouse wheel: change zoom factor.\n\n";
				s += "All 24 compositing operations are fully supported on gfx board with fragment programs.\n";
				s += "If they are absent, there are 9 unsupported compositing operations:\n";
				s += "Multiply, Overlay, Darken, Lighten, Color dodge, Color burn, Hard light, Soft light, Difference";
				MessageBox(NULL, StrUtils::ToAscii(s), "Command keys", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
			}
			// B key
			if (keys[66]) {
				keys[66] = FALSE;
				if (gAnim)
					gAnim = G_FALSE;
				else
					gAnim = G_TRUE;
			}
			// S key
			if (keys[83]) {
				keys[83] = FALSE;
				if (gStrokeOpacity > (GReal)0)
					gStrokeOpacity -= (GReal)0.1;
				DrawTitle();
			}
			// D key
			if (keys[68]) {
				keys[68] = FALSE;
				if (gStrokeOpacity < (GReal)1.0)
					gStrokeOpacity += (GReal)0.1;
				DrawTitle();
			}
			// F key
			if (keys[70]) {
				keys[70] = FALSE;
				if (gFillOpacity > (GReal)0)
					gFillOpacity -= (GReal)0.1;
				DrawTitle();
			}
			// G key
			if (keys[71]) {
				keys[71] = FALSE;
				if (gFillOpacity < (GReal)1.0)
					gFillOpacity += (GReal)0.1;
				DrawTitle();
			}
			// N key
			if (keys[78]) {
				keys[78] = FALSE;
				gRotationVel /= (GReal)1.2;
			}
			// M key
			if (keys[77]) {
				keys[77] = FALSE;
				gRotationVel *= (GReal)1.2;
			}
		}
	}
	// Shutdown
	KillApp();
	KillGLWindow();									// Kill The Window
	return (int)(msg.wParam);							// Exit The Program
}
