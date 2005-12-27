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
#include <amanith/gproperty.h>
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

void setLightAndTransform() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void setDefaultGlobalStates() {
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void BuildNewPalette(const GUInt32 KeysNum) {

	if (KeysNum < 2)
		return;

	GDynArray<GKeyValue> keys(KeysNum);

	keys[0] = GKeyValue(0, GMath::RangeRandom((GReal)0, (GReal)1));
	keys[1] = GKeyValue(1000, GMath::RangeRandom((GReal)0, (GReal)1));

	// red
	for (GUInt32 i = 2; i < KeysNum; ++i)
		keys[i] = GKeyValue(GMath::RangeRandom((GReal)10, (GReal)990), GMath::RangeRandom((GReal)0, (GReal)1));
	gRedAnimC->SetKeys(keys);
	gRedAnimL->SetKeys(keys);
	gRedAnimH->SetKeys(keys);
	// green
	for (GUInt32 i = 0; i < KeysNum; ++i)
		keys[i].SetValue(GMath::RangeRandom((GReal)0, (GReal)1));
	gGreenAnimC->SetKeys(keys);
	gGreenAnimL->SetKeys(keys);
	gGreenAnimH->SetKeys(keys);
	// blue
	for (GUInt32 i = 0; i < KeysNum; ++i)
		keys[i].SetValue(GMath::RangeRandom((GReal)0, (GReal)1));
	gBlueAnimC->SetKeys(keys);
	gBlueAnimL->SetKeys(keys);
	gBlueAnimH->SetKeys(keys);
}

void SetOORType(const GOORType NewOORType) {

	gRedAnimC->SetOORBefore(NewOORType);
	gRedAnimC->SetOORAfter(NewOORType);
	gGreenAnimC->SetOORBefore(NewOORType);
	gGreenAnimC->SetOORAfter(NewOORType);
	gBlueAnimC->SetOORBefore(NewOORType);
	gBlueAnimC->SetOORAfter(NewOORType);

	gRedAnimL->SetOORBefore(NewOORType);
	gRedAnimL->SetOORAfter(NewOORType);
	gGreenAnimL->SetOORBefore(NewOORType);
	gGreenAnimL->SetOORAfter(NewOORType);
	gBlueAnimL->SetOORBefore(NewOORType);
	gBlueAnimL->SetOORAfter(NewOORType);

	gRedAnimH->SetOORBefore(NewOORType);
	gRedAnimH->SetOORAfter(NewOORType);
	gGreenAnimH->SetOORBefore(NewOORType);
	gGreenAnimH->SetOORAfter(NewOORType);
	gBlueAnimH->SetOORBefore(NewOORType);
	gBlueAnimH->SetOORAfter(NewOORType);
}

void AddKey() {

	GUInt32 i;
	GBool b;
	GTimeValue t;

	t = GMath::RangeRandom((GReal)10, (GReal)990);
	gRedAnimC->AddKey(t, i, b);
	gGreenAnimC->AddKey(t, i, b);
	gBlueAnimC->AddKey(t, i, b);
	gRedAnimL->AddKey(t, i, b);
	gGreenAnimL->AddKey(t, i, b);
	gBlueAnimL->AddKey(t, i, b);
	gRedAnimH->AddKey(t, i, b);
	gGreenAnimH->AddKey(t, i, b);
	gBlueAnimH->AddKey(t, i, b);
}

void RemoveKey() {

	if (gRedAnimC->KeysCount() < 3)
		return;

	GInt32 i = GMath::RangeRandom(1, gRedAnimC->KeysCount() - 2);

	gRedAnimC->RemoveKey(i);
	gGreenAnimC->RemoveKey(i);
	gBlueAnimC->RemoveKey(i);
	gRedAnimL->RemoveKey(i);
	gGreenAnimL->RemoveKey(i);
	gBlueAnimL->RemoveKey(i);
	gRedAnimH->RemoveKey(i);
	gGreenAnimH->RemoveKey(i);
	gBlueAnimH->RemoveKey(i);
}

void InitApp() {

#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	gKernel = new GKernel();
	gRedAnimH = (GHermiteProperty1D *)gKernel->CreateNew(G_HERMITEPROPERTY1D_CLASSID);
	gGreenAnimH = (GHermiteProperty1D *)gKernel->CreateNew(G_HERMITEPROPERTY1D_CLASSID);
	gBlueAnimH = (GHermiteProperty1D *)gKernel->CreateNew(G_HERMITEPROPERTY1D_CLASSID);
	gRedAnimL = (GLinearProperty1D *)gKernel->CreateNew(G_LINEARPROPERTY1D_CLASSID);
	gGreenAnimL = (GLinearProperty1D *)gKernel->CreateNew(G_LINEARPROPERTY1D_CLASSID);
	gBlueAnimL = (GLinearProperty1D *)gKernel->CreateNew(G_LINEARPROPERTY1D_CLASSID);
	gRedAnimC = (GConstantProperty1D *)gKernel->CreateNew(G_CONSTANTPROPERTY1D_CLASSID);
	gGreenAnimC = (GConstantProperty1D *)gKernel->CreateNew(G_CONSTANTPROPERTY1D_CLASSID);
	gBlueAnimC = (GConstantProperty1D *)gKernel->CreateNew(G_CONSTANTPROPERTY1D_CLASSID);
	gOORType = G_LOOP_OOR;
	SetOORType(gOORType);
	// set time
	gCurrentTime = 0;
	gTimeStep = 1;
	gDrawPalette = G_TRUE;
	gInterpolationIndex = 0;  // start with hermite interpolation
	// initialize random system
	GMath::SeedRandom();
	// build a palette
	BuildNewPalette(6);
}

void KillApp() {

	if (gKernel)
		delete gKernel;
}

void DrawCursor(const GTimeValue TimePos, const GBool On) {

	GTimeValue t = gRedAnimC->OORTime(TimePos);
	GTimeInterval domainInterval = gRedAnimC->Domain();

	if (On)
		glColor3f(1.0f, 1.0f, 1.0f);
	else
		glColor3f(0.0f, 0.0f, 0.0f);

	GReal x = 2.0f * (t / domainInterval.Length()) - 1.0f;
	// hermite
	if (gInterpolationIndex == 0) {
		glBegin(GL_LINES);
			glVertex2f((GLfloat)x, 0.71f);
			glVertex2f((GLfloat)(x - 0.01), 0.75f);
			glVertex2f((GLfloat)(x - 0.01), 0.75f);
			glVertex2f((GLfloat)(x + 0.01), 0.75f);
			glVertex2f((GLfloat)(x + 0.01), 0.75f);
			glVertex2f((GLfloat)x, 0.71f);
		glEnd();
	}
	else
	// linear
	if (gInterpolationIndex == 1) {
		glBegin(GL_LINES);
			glVertex2f((GLfloat)x, 0.41f);
			glVertex2f((GLfloat)(x - 0.01), 0.45f);
			glVertex2f((GLfloat)(x - 0.01), 0.45f);
			glVertex2f((GLfloat)(x + 0.01), 0.45f);
			glVertex2f((GLfloat)(x + 0.01), 0.45f);
			glVertex2f((GLfloat)x, 0.41f);
		glEnd();
	}
	// constant
	else {
		glBegin(GL_LINES);
			glVertex2f((GLfloat)x, 0.11f);
			glVertex2f((GLfloat)(x - 0.01), 0.15f);
			glVertex2f((GLfloat)(x - 0.01), 0.15f);
			glVertex2f((GLfloat)(x + 0.01), 0.15f);
			glVertex2f((GLfloat)(x + 0.01), 0.15f);
			glVertex2f((GLfloat)x, 0.11f);
		glEnd();
	}
	if (!On)
		return;

	GKeyValue currentColorR, currentColorG, currentColorB;
	GTimeInterval validInterval;

	if (gInterpolationIndex == 0) {
		gRedAnimH->Value(currentColorR, validInterval, TimePos);
		gGreenAnimH->Value(currentColorG, validInterval, TimePos);
		gBlueAnimH->Value(currentColorB, validInterval, TimePos);
	}
	else
	if (gInterpolationIndex == 1) {
		gRedAnimL->Value(currentColorR, validInterval, TimePos);
		gGreenAnimL->Value(currentColorG, validInterval, TimePos);
		gBlueAnimL->Value(currentColorB, validInterval, TimePos);
	}
	else {
		gRedAnimC->Value(currentColorR, validInterval, TimePos);
		gGreenAnimC->Value(currentColorG, validInterval, TimePos);
		gBlueAnimC->Value(currentColorB, validInterval, TimePos);
	}
	glColor3f((GLfloat)currentColorR.RealValue(), (GLfloat)currentColorG.RealValue(), (GLfloat)currentColorB.RealValue());
	glBegin(GL_POLYGON);
		glVertex2f(-1.0f, -0.5f);
		glVertex2f(1.0f, -0.5f);
		glVertex2f(1.0f, -0.7f);
		glVertex2f(-1.0f, -0.7f);
	glEnd();
}

void DrawPalette() {

	#define X_SAMPLES 1000.0f
	GReal t, x, xStep, tStep;
	GKeyValue r, g, b;
	GInt32 i, j;
	GTimeInterval validInterval, domainInterval;

	// extract animation domain
	domainInterval = gRedAnimH->Domain();

	// calculate time and position steps
	x = -1.0f;
	xStep = 2.0f / X_SAMPLES;
	t = domainInterval.Start();
	tStep = domainInterval.Length() / X_SAMPLES;

	// draw color bars
	for (i = 0; i <= X_SAMPLES; ++i) {
		// hermite
		gRedAnimH->Value(r, validInterval, t);
		gGreenAnimH->Value(g, validInterval, t);
		gBlueAnimH->Value(b, validInterval, t);
		glColor3f((GLfloat)r.RealValue(), (GLfloat)g.RealValue(), (GLfloat)b.RealValue());
		glBegin(GL_POLYGON);
		glVertex2f((GLfloat)x, 0.5f);
		glVertex2f((GLfloat)(x + xStep), 0.5f);
		glVertex2f((GLfloat)(x + xStep), 0.7f);
		glVertex2f((GLfloat)x, 0.7f);
		glEnd();
		// linear
		gRedAnimL->Value(r, validInterval, t);
		gGreenAnimL->Value(g, validInterval, t);
		gBlueAnimL->Value(b, validInterval, t);
		glColor3f((GLfloat)r.RealValue(), (GLfloat)g.RealValue(), (GLfloat)b.RealValue());
		glBegin(GL_POLYGON);
		glVertex2f((GLfloat)x, 0.2f);
		glVertex2f((GLfloat)(x + xStep), 0.2f);
		glVertex2f((GLfloat)(x + xStep), 0.4f);
		glVertex2f((GLfloat)x, 0.4f);
		glEnd();
		// constant
		gRedAnimC->Value(r, validInterval, t);
		gGreenAnimC->Value(g, validInterval, t);
		gBlueAnimC->Value(b, validInterval, t);
		glColor3f((GLfloat)r.RealValue(), (GLfloat)g.RealValue(), (GLfloat)b.RealValue());
		glBegin(GL_POLYGON);
		glVertex2f((GLfloat)x, 0.1f);
		glVertex2f((GLfloat)(x + xStep), 0.1f);
		glVertex2f((GLfloat)(x + xStep), -0.1f);
		glVertex2f((GLfloat)x, -0.1f);
		glEnd();
		// next sample
		t += tStep;
		x += xStep;
	}

	// draw animation keys
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
	j = gRedAnimC->KeysCount();
	x = -1.0f;
	for (i = 0; i <= j; i++) {
		gRedAnimC->Key(i, r);
		gGreenAnimC->Key(i, g);
		gBlueAnimC->Key(i, b);

		x = (2.0f * (r.TimePosition() / domainInterval.Length())) - 1.0f;

		glColor3f((GLfloat)r.RealValue(), (GLfloat)g.RealValue(), (GLfloat)b.RealValue());
		glBegin(GL_LINES);
		glVertex2f((GLfloat)x, -0.2f);
		glVertex2f((GLfloat)x, -0.4f);
		glEnd();
	}
	setDefaultGlobalStates();
	#undef X_SAMPLES
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) {

	if (height == 0)
		height = 1;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.5, 1.5, -1.5, 1.5, -1.5, 1.5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gDrawPalette = G_TRUE;
}

int InitGL(GLvoid) {

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glDisable(GL_LIGHTING);
	setDefaultGlobalStates();
	return TRUE;
}

int DrawGLScene(GLvoid)	{

	setLightAndTransform();

	if (gDrawPalette) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawPalette();
		gDrawPalette = G_FALSE;
	}

	DrawCursor(gCurrentTime, G_FALSE);
	gCurrentTime += gTimeStep;
	DrawCursor(gCurrentTime, G_TRUE);
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
	if (!CreateGLWindow("Color animation example - Press F1 for help", 640, 480, 16, fullscreen))
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

			if (keys[VK_F1]) {						// Is F1 Being Pressed?
				keys[VK_F1] = FALSE;
				s = "A/Z: Add/Remove a color key +/-\n";
				s += "Space: Generate a new random palette\n";
				s += "I: Change interpolation type (constant/linear/hermite)\n";
				s += "O: Change 'out of range' behavior (constant/loop/ping-pong)";
				MessageBox(NULL, StrUtils::ToAscii(s), "Command keys", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
			}
			// A key
			if (keys[65]) {
				keys[65] = FALSE;
				AddKey();
				gDrawPalette = G_TRUE;
			}
			// Z key
			if (keys[90]) {
				RemoveKey();
				gDrawPalette = G_TRUE;
			}
			// I key
			if (keys[73]) {
				keys[73] = FALSE;
				gInterpolationIndex = (gInterpolationIndex + 1) % 3;
				gDrawPalette = G_TRUE;
			}
			// O key
			if (keys[79]) {
				keys[79] = FALSE;
				if (gOORType == G_CONSTANT_OOR)
					gOORType = G_LOOP_OOR;
				else
				if (gOORType == G_LOOP_OOR)
					gOORType = G_PINGPONG_OOR;
				else
				if (gOORType == G_PINGPONG_OOR)
					gOORType = G_CONSTANT_OOR;
				SetOORType(gOORType);
				gCurrentTime = 0;
				gDrawPalette = G_TRUE;
			}
			if (keys[VK_SPACE]) {
				keys[VK_SPACE] = FALSE;
				BuildNewPalette(gRedAnimC->KeysCount());
				gDrawPalette = G_TRUE;
			}
		}
	}
	// Shutdown
	KillGLWindow();									// Kill The Window
	KillApp();
	return (int)(msg.wParam);							// Exit The Program
}
