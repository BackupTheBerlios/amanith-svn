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
#include <amanith/2d/gfont2d.h>
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

bool arbMultisampleSupported = false;
int arbMultisampleFormat = 0;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

// Amanith stuff
GKernel *gKernel;
GOpenglExt *gExtManager = NULL;	// extensions manager
GFont2D *gFont;
GString gDataPath;
GDynArray< GPoint<GDouble, 2> > gVertices;
GDynArray<GPoint2> gContoursPoints;
GDynArray<GInt32> gContoursIndexes;
const GFontChar2D *gChar;
GUInt32 gCurrentChar;
GBool gFillDraw;
GBool gWireFrame;
GReal gDeviation;
GLfloat	gX, gY, gZ;								// Depth Into The Screen

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
			WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 16,
			WGL_ALPHA_BITS_ARB, 0,
			WGL_DEPTH_BITS_ARB, 16,
			WGL_STENCIL_BITS_ARB, 0,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, 4,
			0, 0
	};
	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);

	// If We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;	
		return arbMultisampleSupported;
	}
	// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
	iAttributes[19] = 2;
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;	 
		return arbMultisampleSupported;
	}
	// Return The Valid Format
	return arbMultisampleSupported;
}

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

void DrawTriangles(const GDynArray< GPoint<GDouble, 2> >& Points) {

	GUInt32 i, j = (GUInt32)Points.size();
	GPoint<GDouble, 2> a, b, c;
	GPoint<GDouble, 3> col1(1.0f, 0.74f, 0.20f);
	GPoint<GDouble, 3> col2(0.72f, 0.0f, 0.0f);
	GPoint<GDouble, 3> col;

	glBegin(GL_TRIANGLES);
	for (i = 0; i < j; i+=3) {
		a = Points[i];
		b = Points[i + 1];
		c = Points[i + 2];
		// generate color for first point
		col = GMath::Lerp(GMath::Clamp(a[G_X], (GDouble)0, (GDouble)1), col1, col2);
		glColor3d(col[0], col[1], col[2]);
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		// generate color for second point
		col = GMath::Lerp(GMath::Clamp(b[G_X], (GDouble)0, (GDouble)1), col1, col2);
		glColor3d(col[0], col[1], col[2]);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		// generate color for third point
		col = GMath::Lerp(GMath::Clamp(c[G_X], (GDouble)0, (GDouble)1), col1, col2);
		glColor3d(col[0], col[1], col[2]);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
	}
	glEnd();

	if (!gWireFrame)
		return;

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (i = 0; i < j; i+=3) {
		a = Points[i];
		b = Points[i + 1];
		c = Points[i + 2];
		// draw edge lines
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		glVertex3d(a[G_X], a[G_Y], 1.0f);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
		glVertex3d(b[G_X], b[G_Y], 1.0f);
		glVertex3d(c[G_X], c[G_Y], 1.0f);
	}
	glEnd();
}

void DrawOutlines(const GDynArray<GPoint2>& ContoursPoints, const GDynArray<GInt32>& ContoursIndexes) {

	GUInt32 i, j, k, w, ofs;
	GPoint2 p1, p2;

	ofs = 0;
	j = (GUInt32)ContoursIndexes.size();
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.4f, 0.9f);
	for (i = 0; i < j; i++) {
		k = ContoursIndexes[i];
		for (w = 0; w < k - 1; w++) {
			p1 = ContoursPoints[ofs + w];
			p2 = ContoursPoints[ofs + w + 1];
			glVertex3f((GLfloat)p1[G_X], (GLfloat)p1[G_Y], 1.0f);
			glVertex3f((GLfloat)p2[G_X], (GLfloat)p2[G_Y], 1.0f);
		}
		p1 = ContoursPoints[ofs + k - 1];
		p2 = ContoursPoints[ofs];
		glVertex3f((GLfloat)p1[G_X], (GLfloat)p1[G_Y], 1.0f);
		glVertex3f((GLfloat)p2[G_X], (GLfloat)p2[G_Y], 1.0f);
		ofs += k;
	}
	glEnd();
}

void DrawChar(const GFontChar2D* Char, const GReal Deviation, const GMatrix33& Transformation) {

	GUInt32 i, j;

	if (!Char->IsComposite()) {
		GDynArray<GPoint2> tmpPts;
		GDynArray<GInt32> tmpIndex;
		GUInt32 oldSize = 0;

		// extract all contours
		j = Char->ContoursCount();
		for (i = 0; i < j; ++i) {
			Char->Contour(i)->DrawContour(tmpPts, Deviation, Transformation);
			tmpIndex.push_back((GInt32)tmpPts.size() - (GInt32)oldSize);
			oldSize = (GUInt32)tmpPts.size();
			// insert generated point into internal arrays, used for the pure wireframe rendering mode
			gContoursPoints.insert(gContoursPoints.end(), tmpPts.begin(), tmpPts.end());
			gContoursIndexes.insert(gContoursIndexes.end(), tmpIndex.begin(), tmpIndex.end());
		}
		// triangulate contours
		GTesselator2D tesselator;
		tesselator.Tesselate(tmpPts, tmpIndex, gVertices, G_ODD_EVEN_RULE);
	}
	else {
		const GFontChar2D* subChar;
		GSubChar2D subCharInfo;
		GMatrix33 totalMatrix;
		// extract all sub characters
		j = Char->SubCharsCount();
		for (i = 0; i < j; ++i) {
			Char->SubChar(i, subCharInfo);
			// concatenate transformations
			totalMatrix = Transformation * subCharInfo.Transformation;
			subChar = gFont->CharByIndex(subCharInfo.GlyphIndex);
			// draw sub character
			DrawChar(subChar, Deviation, totalMatrix);
		}
	}
}

void GenerateTessellation(const GFontChar2D* Char, const GReal Deviation) {

	if (Char == NULL)
		return;

	// start with an identity transformation
	GMatrix33 idMatrix;

	gContoursPoints.clear();
	gContoursIndexes.clear();
	gVertices.clear();
	DrawChar(Char, Deviation, idMatrix);
}

void InitApp() {

#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	GString s;

	gKernel = new GKernel();
	gFont = (GFont2D *)gKernel->CreateNew(G_FONT2D_CLASSID);
	// Depth Into The Screen
	gX = -0.4f;
	gY = -0.3f;
	gZ = -2.3f;
	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";
	s = StrUtils::OSFixPath(gDataPath + "crazk.ttf", G_FALSE);
	gFont->Load(StrUtils::ToAscii(s), "scale=0");
	gWireFrame = G_TRUE;
	gFillDraw = G_TRUE;
	gDeviation = (GReal)0.000003;
	gCurrentChar = 'S';
	gChar = gFont->CharByCode(gCurrentChar, 0);
	GenerateTessellation(gChar, gDeviation);
}

void KillApp() {

	if (gKernel)
		delete gKernel;
	if (gExtManager)
		delete gExtManager;
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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setLightAndTransform();

	if (gFillDraw)
		DrawTriangles(gVertices);
	else
		DrawOutlines(gContoursPoints, gContoursIndexes);
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

	if (!arbMultisampleSupported) {
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
		// create extensions manager
		if (!gExtManager)
			gExtManager = new GOpenglExt();

		if (InitMultisample(hInstance, hWnd, pfd)) {
			KillGLWindow();
			return CreateGLWindow(title, width, height, bits, fullscreenflag);
		}
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
	if (!CreateGLWindow("Font drawing example - Press F1 for help", 640, 480, 16, fullscreen))
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
				s += "M/N: Fine/Rough adaptive flattening\n";
				s += "F: Toggle solid/outlines\n";
				s += "PgUp/PgDown: Previous/Next font letter";
				MessageBox(NULL, StrUtils::ToAscii(s), "Command keys", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
			}
			if (keys[VK_PRIOR]) {
				keys[VK_PRIOR] = FALSE;
				if (gCurrentChar > 0) {
					gCurrentChar--;
					gChar = gFont->CharByCode(gCurrentChar, 0);
					GenerateTessellation(gChar, gDeviation);
				}
			}
			if (keys[VK_NEXT]) {
				keys[VK_NEXT] = FALSE;
				gCurrentChar++;
				gChar = gFont->CharByCode(gCurrentChar, 0);
				GenerateTessellation(gChar, gDeviation);
			}
			// A key
			if (keys[65]) {
				keys[65] = FALSE;
				gZ -= 0.05f;
			}
			// Z key
			if (keys[90]) {
				keys[90] = FALSE;
				gZ += 0.05f;
			}
			if (keys[VK_UP]) {
				keys[VK_UP] = FALSE;
				gY += 0.05f;
			}
			if (keys[VK_DOWN]) {
				keys[VK_DOWN] = FALSE;
				gY -= 0.05f;
			}
			if (keys[VK_RIGHT]) {
				keys[VK_RIGHT] = FALSE;
				gX += 0.05f;
			}
			if (keys[VK_LEFT]) {
				keys[VK_LEFT] = FALSE;
				gX -= 0.05f;
			}
			// M key
			if (keys[77]) {
				keys[77] = FALSE;
				gDeviation /= 2;
				GenerateTessellation(gChar, gDeviation);
			}
			// N key
			if (keys[78]) {
				keys[78] = FALSE;
				gDeviation *= 2;
				GenerateTessellation(gChar, gDeviation);
			}
			if (keys[VK_SPACE]) {
				keys[VK_SPACE] = FALSE;
				gWireFrame = !gWireFrame;
			}
			// F
			if (keys[70]) {
				keys[70] = FALSE;
				gFillDraw = !gFillDraw;
			}
		}
	}
	// Shutdown
	KillGLWindow();									// Kill The Window
	KillApp();
	return (int)(msg.wParam);							// Exit The Program
}
