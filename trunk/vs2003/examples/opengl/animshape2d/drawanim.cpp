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
#include <amanith/2d/ganimtrsnode2d.h>
#include <amanith/gmultiproperty.h>
#include <amanith/2d/gtesselator2d.h>
#include <amanith/2d/gpixelmap.h>
#include <amanith/2d/gfont2d.h>
#include <new>
#include "tinyxml.h"
#include "resource.h"

using namespace Amanith;
#define LOGIC_SIZE 1.3

// *********************************************************************
//                           GTesselatedGlyph
// *********************************************************************
class GTesselatedGlyph {

private:
	GDynArray<GPoint2> gPoints;
	GLuint gDisplayList;

public:
	GTesselatedGlyph(const GFontChar2D* FontChar, const GReal Deviation) {

		if (!FontChar || FontChar->IsComposite())
			return;

		GDynArray<GPoint2> tmpPts;
		GDynArray<GInt32> tmpIndex;
		GUInt32 i, j = FontChar->ContoursCount(), oldSize = 0;
		// extract all contours
		for (i = 0; i < j; ++i) {
			FontChar->Contour(i)->DrawContour(tmpPts, Deviation);
			tmpIndex.push_back((GInt32)tmpPts.size() - (GInt32)oldSize);
			oldSize = (GUInt32)tmpPts.size();
		}
		// triangulate contours
		GTesselator2D tesselator;
		tesselator.Tesselate(tmpPts, tmpIndex, gPoints, G_ODD_EVEN_RULE);
	}
	inline const GDynArray<GPoint2>& Points() const {
		return gPoints;
	}
	inline GLuint DisplayList() const {
		return gDisplayList;
	}
	void GenDisplayList() {
		// generate display list
		gDisplayList = glGenLists(1);
		glNewList(gDisplayList, GL_COMPILE);

		GUInt32 i, j = (GUInt32)gPoints.size();
		glBegin(GL_TRIANGLES);
		for (i = 0; i < j; i+=3) {
			#ifdef DOUBLE_REAL_TYPE
				glVertex2d(gPoints[i + 0][G_X], gPoints[i + 0][G_Y]);
				glVertex2d(gPoints[i + 1][G_X], gPoints[i + 1][G_Y]);
				glVertex2d(gPoints[i + 2][G_X], gPoints[i + 2][G_Y]);
			#else
				glVertex2f(gPoints[i + 0][G_X], gPoints[i + 0][G_Y]);
				glVertex2f(gPoints[i + 1][G_X], gPoints[i + 1][G_Y]);
				glVertex2f(gPoints[i + 2][G_X], gPoints[i + 2][G_Y]);
			#endif
		}
		glEnd();
		glEndList();
	}
};

// *********************************************************************
//                              GGlyphStyle
// *********************************************************************
class GGlyphStyle {

private:
	GBool gOpaque;
	GReal gOpacity;
	GVector3 gColor;
	GInt32 gGlyphIndex;

public:
	GGlyphStyle() {
		// full opaque
		gOpaque = G_TRUE;
		gOpacity = 1;
		// white color
		gColor.Set(1, 1, 1);
		gGlyphIndex = -1;
	}
	// get glyph index
	inline GInt32 GlyphIndex() const {
		return gGlyphIndex;
	}
	// set glyph index
	inline void SetGlyphIndex(const GInt32 NewIndex) {
		gGlyphIndex = NewIndex;
	}
	// get a flag that indicates if this glyph is opaque
	inline GBool Opaque() const {
		return gOpaque;
	}
	// set transparent flag
	inline void SetOpaque(const GBool NewValue) {
		gOpaque = NewValue;
	}
	// get glyph opacity
	inline GReal Opacity() const {
		return gOpacity;
	}
	inline void SetOpacity(const GReal NewValue) {
		gOpacity = NewValue;
	}
	// get glyph color
	inline const GVectBase<GReal, 3>& Color() const {
		return gColor;
	}
	// set glyph color
	inline void SetColor(const GVectBase<GReal, 3>& NewColor) {
		gColor = NewColor;
	}
};

// *********************************************************************
//                              GAnimCard
// *********************************************************************
struct GAnimCard {

public:
	GReal LifeTicks;
	GReal ElapsedTicks;
	const GAnimTRSNode2D *Card;
	GAnimTRSNode2D *Animation;

	GAnimCard() {
		ElapsedTicks = 1;
		LifeTicks = 0;
		Card = NULL;
		Animation = 0;
	}
	inline GBool IsAlive() const {
		if (ElapsedTicks >= LifeTicks)
			return G_FALSE;
		return G_TRUE;
	}
};

// *********************************************************************
//                           Main Application
// *********************************************************************
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
TiXmlDocument gXMLDoc;
GDynArray<GTesselatedGlyph *> gTesselatedGlyphs; // triangulated glyphs
GDynArray<GAnimTRSNode2D *> gCards; // compound poker cards
GDynArray<GAnimTRSNode2D *> gAnimations; // cards animations
GUInt32 gMaxCards; // number of max cards
GReal gLogicTick;
GDynArray<GAnimCard> gAnimCards;
GOpenglExt *gExtManager = NULL;	// extensions manager
// background stuff (texture, time, color animation, texture animation)
GPixelMap *gBackGround;
GAnimTRSNode2D *gBackGroundAnimUV;
GAnimTRSNode2D *gAnimLogo;
GThreeHermiteProperty1D *gBackGroundAnimColor;
GReal gBackGroundTime;
GReal gBackGroundTimeStep;
GLuint gBindedTexture;
GBool gBlockAnim = G_FALSE;

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
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 24,
		WGL_ALPHA_BITS_ARB, 8,
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
}

void setDefaultGlobalStates() {

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// convert an affine 3x3 matrix to its correspondent 4x4 matrix
void Matrix33To44(const GMatrix33& Matrix33, GMatrix44& Matrix44) {

	Identity(Matrix44);
	Matrix44[0][0] = Matrix33[0][0];
	Matrix44[0][1] = Matrix33[0][1];
	Matrix44[1][0] = Matrix33[1][0];
	Matrix44[1][1] = Matrix33[1][1];
	Matrix44[0][3] = Matrix33[0][2];
	Matrix44[1][3] = Matrix33[1][2];
	Matrix44[2][3] = Matrix33[2][2];
}

GError LoadAnimFromXML(const TiXmlNode *XMLNode, GAnimTRSNode2D *TRSNode) {

	const TiXmlElement *trackElem;
	const TiXmlElement *keyElem;
	GHermiteProperty1D *p;
	const GChar8 *strVal;
	GVector2 tmpVect;
	GTimeValue keyTime;
	GError err;

	// load position track (if exists)
	trackElem = (const TiXmlElement *)XMLNode->FirstChild("position");
	if (trackElem) {
		// find first key
		keyElem = (const TiXmlElement *)trackElem->FirstChild("key");
		while (keyElem) {
			keyTime = 0;
			// read time position
			strVal = keyElem->Attribute("time");
			if (strVal)
				keyTime = (GReal)StrUtils::ToDouble(strVal);
			// read position value
			strVal = keyElem->Attribute("value");
			if (strVal) {
				err = StrUtils::FromString(strVal, tmpVect);
				if (err == G_NO_ERROR)
					TRSNode->SetPosition(keyTime, tmpVect);
			}
			// next key
			keyElem = (const TiXmlElement *)keyElem->NextSibling("key");
		}
		p = (GHermiteProperty1D *)TRSNode->Property("transform")->Property("position")->Property("x");
		p->RecalcSmoothTangents(G_FALSE);
		p = (GHermiteProperty1D *)TRSNode->Property("transform")->Property("position")->Property("y");
		p->RecalcSmoothTangents(G_FALSE);
	}
	// load rotation track (if exists)
	trackElem = (const TiXmlElement *)XMLNode->FirstChild("rotation");
	if (trackElem) {
		// find first key
		keyElem = (const TiXmlElement *)trackElem->FirstChild("key");
		while (keyElem) {
			keyTime = 0;
			// read time position
			strVal = keyElem->Attribute("time");
			if (strVal)
				keyTime = (GReal)StrUtils::ToDouble(strVal);
			// read rotation value
			strVal = keyElem->Attribute("value");
			if (strVal) {
				tmpVect[G_X] = (GReal)StrUtils::ToDouble(strVal);
				TRSNode->SetRotation(keyTime, tmpVect[G_X]);
			}
			// next key
			keyElem = (const TiXmlElement *)keyElem->NextSibling("key");
		}
		p = (GHermiteProperty1D *)TRSNode->Property("transform")->Property("rotation");
		p->RecalcSmoothTangents(G_FALSE);
	}
	// load scale track (if exists)
	trackElem = (const TiXmlElement *)XMLNode->FirstChild("scale");
	if (trackElem) {
		// find first key
		keyElem = (const TiXmlElement *)trackElem->FirstChild("key");
		while (keyElem) {
			keyTime = 0;
			// read time position
			strVal = keyElem->Attribute("time");
			if (strVal)
				keyTime = (GReal)StrUtils::ToDouble(strVal);
			// read scale value
			strVal = keyElem->Attribute("value");
			if (strVal) {
				err = StrUtils::FromString(strVal, tmpVect);
				if (err == G_NO_ERROR)
					TRSNode->SetScale(keyTime, tmpVect);
			}
			// next key
			keyElem = (const TiXmlElement *)keyElem->NextSibling("key");
		}
		p = (GHermiteProperty1D *)TRSNode->Property("transform")->Property("scale")->Property("x");
		p->RecalcSmoothTangents(G_FALSE);
		p = (GHermiteProperty1D *)TRSNode->Property("transform")->Property("scale")->Property("y");
		p->RecalcSmoothTangents(G_FALSE);
	}
	return G_NO_ERROR;
}

GError LoadCardGlyphFromXML(const TiXmlElement *XMLElem, GAnimTRSNode2D *Father) {

	GError err;
	GVector3 tmpColor;
	GVector2 tmpVect;

	// allocate a new style
	GGlyphStyle *newStyle = new(std::nothrow) GGlyphStyle();
	if (!newStyle)
		return G_MEMORY_ERROR;
	// allocate a new TRS node
	GAnimTRSNode2D *trsNode = (GAnimTRSNode2D *)gKernel->CreateNew(G_ANIMTRSNODE2D_CLASSID);
	if (!trsNode) {
		delete newStyle;
		return G_MEMORY_ERROR;
	}

	// fill glyph style, reading settings from XML
	const GChar8 *strVal;
	// 'id' attribute
	strVal = XMLElem->Attribute("id");
	if (strVal)
		newStyle->SetGlyphIndex(StrUtils::ToInt(strVal));
	// 'color' attribute
	strVal = XMLElem->Attribute("color");
	if (strVal){
		err = StrUtils::FromString(GString(strVal), tmpColor);
		if (err == G_NO_ERROR)
			newStyle->SetColor(tmpColor);
	}
	// 'isopaque' attribute
	strVal = XMLElem->Attribute("isopaque");
	if (strVal){
		if (StrUtils::SameText(strVal, "false"))
			newStyle->SetOpaque(G_FALSE);
	}
	// 'opacity' attribute
	strVal = XMLElem->Attribute("opacity");
	if (strVal)
		newStyle->SetOpacity((GReal)StrUtils::ToDouble(strVal));
	// if root node push it into internal card list
	if (!Father)
		gCards.push_back(trsNode);
	else {
		G_ASSERT(Father != NULL);
		trsNode->SetFather(Father, G_FALSE);
	}
	// 'pivot' attribute
	strVal = XMLElem->Attribute("pivot");
	if (strVal) {
		err = StrUtils::FromString(GString(strVal), tmpVect);
		if (err == G_NO_ERROR)
			trsNode->SetPivotPosition(tmpVect, G_FALSE);
	}
	// 'pos' attribute
	strVal = XMLElem->Attribute("pos");
	if (strVal) {
		err = StrUtils::FromString(GString(strVal), tmpVect);
		if (err == G_NO_ERROR)
			trsNode->SetPosition(0, tmpVect);
	}
	// 'rot' attribute
	strVal = XMLElem->Attribute("rot");
	if (strVal)
		trsNode->SetRotation(0, (GReal)StrUtils::ToDouble(strVal));
	// 'scl' attribute
	strVal = XMLElem->Attribute("scl");
	if (strVal) {
		err = StrUtils::FromString(GString(strVal), tmpVect);
		if (err == G_NO_ERROR)
			trsNode->SetScale(0, tmpVect);
	}
	// link glyph style to trs node
	trsNode->SetCustomData((void *)newStyle);
	return G_NO_ERROR;
}

//------------------------------------------------------------
GError LoadCardFromXML(const TiXmlElement *XMLElem) {

	const TiXmlElement *glyphElem = NULL;
	const GChar8 *strVal;
	GAnimTRSNode2D *rootCard = NULL;
	GInt32 rootID = -1, glyphID;

	// read root glyph id
	strVal = XMLElem->Attribute("rootglyph");
	if (strVal)
		rootID = StrUtils::ToInt(strVal);
	// find root glyph
	glyphElem = (const TiXmlElement *)XMLElem->FirstChild();
	while (glyphElem) {
		glyphID = -1;
		strVal = glyphElem->Attribute("id");
		if (strVal) {
			glyphID = StrUtils::ToInt(strVal);
			// i've found root glyph
			if (rootID == -1 || glyphID == rootID) {
				// load the root glyph
				LoadCardGlyphFromXML(glyphElem, NULL);
				rootCard = gCards.back();
				break;
			}
		}
	}
	// load child glyphs
	glyphElem = (const TiXmlElement *)XMLElem->FirstChild();
	while (glyphElem) {
		glyphID = -1;
		strVal = glyphElem->Attribute("id");
		if (strVal) {
			glyphID = StrUtils::ToInt(strVal);
			if (glyphID != -1 && glyphID != rootID)
				// load a child glyph
				LoadCardGlyphFromXML(glyphElem, rootCard);
		}
		glyphElem = (const TiXmlElement *)glyphElem->NextSibling();
	}
	return G_NO_ERROR;
}

GError LoadAndTesselateGlyphs(const GString& FileName, const GReal Deviation) {

	if (!FileUtils::FileExists(StrUtils::ToAscii(FileName)))
		return G_FILE_NOT_FOUND;

	// create the font
	GFont2D *font = (GFont2D *)gKernel->CreateNew(G_FONT2D_CLASSID);
	if (!font)
		return G_MEMORY_ERROR;
	// load the font, normalizing points into [0;1]x[0;1] range (scale=0 option)
	GError err = font->Load(StrUtils::ToAscii(FileName), "scale=0");
	if (err != G_NO_ERROR)
		return err;
	// flatten and tessellate every character
	GInt32 i, j = font->CharsCount();
	for (i = 0; i < j; ++i) {
		const GFontChar2D *fontChar = font->CharByIndex(i);
		G_ASSERT(fontChar != NULL);
		if (!fontChar->IsComposite()) {
			GTesselatedGlyph *tessGlyph = new(std::nothrow) GTesselatedGlyph(fontChar, Deviation);
			if (tessGlyph) {
				gTesselatedGlyphs.push_back(tessGlyph);
			}
		}
	}
	// delete font from memory and return
	delete font;
	return G_NO_ERROR;
}

GError LoadCardsAndAnimations(const GString& FileName) {

	GError err;
	GAnimTRSNode2D *trsNode = NULL;

	// check if the file exists
	if (!FileUtils::FileExists(StrUtils::ToAscii(FileName)))
		return G_FILE_NOT_FOUND;
	// load XML document
	GBool ok = gXMLDoc.LoadFile(StrUtils::ToAscii(FileName));
	if (!ok)
		return G_INVALID_FORMAT;

	// extract root element
	const TiXmlElement *root = gXMLDoc.RootElement();
	if (!root)
		return G_INVALID_FORMAT;

	// load all cards
	const TiXmlElement *n = (const TiXmlElement *)root->FirstChild("card");
	while (n) {
		err = LoadCardFromXML(n);
		// jump to next card
		n = (const TiXmlElement *)n->NextSibling("card");
	}

	// load animations
	n = (const TiXmlElement *)root->FirstChild("animation");
	while (n) {
		// allocate a trs node
		trsNode = (GAnimTRSNode2D *)gKernel->CreateNew(G_ANIMTRSNODE2D_CLASSID);
		// read animation
		if (trsNode) {
			err = LoadAnimFromXML(n, trsNode);
			if (err != G_NO_ERROR)
				delete trsNode;
			else
				gAnimations.push_back(trsNode);
		}
		// jump to next animation
		n = (const TiXmlElement *)n->NextSibling("animation");
	}
	return G_NO_ERROR;
}

void LoadBackGraoundAndLogo(const GString& FileName) {

	GError err;

	// create and load background texture
	gBackGround = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	if (!gBackGround)  {
		perror("Fail to create background GPixelMap");
		abort();
	}
	// use 'expandpalette' option just to ensure full color format (so artists can manipulate starts.png as
	// they want to do
	err = gBackGround->Load(StrUtils::ToAscii(FileName), "expandpalette=true");
	if (err != G_NO_ERROR) {
		perror("Fail to load stars.png");
		abort();
	}

	gBackGroundAnimUV = (GAnimTRSNode2D *)gKernel->CreateNew(G_ANIMTRSNODE2D_CLASSID);
	gBackGroundAnimColor = (GThreeHermiteProperty1D *)gKernel->CreateNew(G_THREEHERMITEPROPERTY1D_CLASSID);
	if (!gBackGroundAnimUV || !gBackGroundAnimColor) {
		perror("Fail to create background animations");
		abort();
	}
	// set uv animation
	gBackGroundAnimUV->SetPivotPosition(GPoint2(0.5, 0.5), G_FALSE);
	GHermiteProperty1D *p;
	GDynArray<GKeyValue> tmpKeys;

	// x-position
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("position")->Property("x");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)-2));
	tmpKeys.push_back(GKeyValue(0.33, (GReal)2));
	tmpKeys.push_back(GKeyValue(0.67, (GReal)0));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)-2));
	p->SetKeys(tmpKeys);
	// y-position
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("position")->Property("y");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)1));
	tmpKeys.push_back(GKeyValue(0.33, (GReal)2));
	tmpKeys.push_back(GKeyValue(0.67, (GReal)-2));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)1));
	p->SetKeys(tmpKeys);
	// rotation
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("rotation");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)0));
	tmpKeys.push_back(GKeyValue(0.25, (GReal)1.57 * 2));
	tmpKeys.push_back(GKeyValue(0.50, (GReal)3.14 * 2));
	tmpKeys.push_back(GKeyValue(0.75, (GReal)4.71 * 2));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)6.28 * 2));
	p->SetKeys(tmpKeys);
	// x-scale
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("scale")->Property("x");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)1));
	tmpKeys.push_back(GKeyValue(0.33, (GReal)3));
	tmpKeys.push_back(GKeyValue(0.67, (GReal)0.9));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)1));
	p->SetKeys(tmpKeys);
	// y-scale
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("scale")->Property("y");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)1));
	tmpKeys.push_back(GKeyValue(0.33, (GReal)3));
	tmpKeys.push_back(GKeyValue(0.67, (GReal)0.9));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)1));
	p->SetKeys(tmpKeys);

	// red color
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimColor->Property("x");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)0.7));
	tmpKeys.push_back(GKeyValue(0.50, (GReal)0.0));
	tmpKeys.push_back(GKeyValue(0.75, (GReal)0.0));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)0.7));
	p->SetKeys(tmpKeys);
	// green color
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimColor->Property("y");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)0.0));
	tmpKeys.push_back(GKeyValue(0.50, (GReal)0.1));
	tmpKeys.push_back(GKeyValue(0.75, (GReal)0.7));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)0.0));
	p->SetKeys(tmpKeys);
	// blue color
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimColor->Property("z");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)0.5));
	tmpKeys.push_back(GKeyValue(0.50, (GReal)0.6));
	tmpKeys.push_back(GKeyValue(0.75, (GReal)0.3));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)0.5));
	p->SetKeys(tmpKeys);

	// now set up logo
	gAnimLogo = (GAnimTRSNode2D *)gKernel->CreateNew(G_ANIMTRSNODE2D_CLASSID);	
	gAnimLogo->SetPivotPosition(GPoint2(0.550, 0.540), G_FALSE);

	gAnimLogo->Property("transform")->Property("position")->Property("x")->SetDefaultValue(GKeyValue(0.6));
	gAnimLogo->Property("transform")->Property("position")->Property("y")->SetDefaultValue(GKeyValue(-1.23));

	// x-scale
	tmpKeys.clear();
	gAnimLogo->Property("transform")->Property("scale")->Property("y")->SetDefaultValue(GKeyValue(1.5));
	p = (GHermiteProperty1D *)gAnimLogo->Property("transform")->Property("scale")->Property("x");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue(0.00, (GReal)1.5));
	tmpKeys.push_back(GKeyValue(0.5, (GReal)-1.5));
	tmpKeys.push_back(GKeyValue(1.00, (GReal)1.5));
	p->SetKeys(tmpKeys);
}


void InitApp() {

#ifdef _DEBUG
	SysUtils::RedirectIOToConsole();
#endif

	GError err;

	// create a new kernel
	gKernel = new GKernel();

	// build path for data
	GString dataPath = SysUtils::AmanithPath();
	if (dataPath.length() > 0)
		dataPath += "data/";

	// load glyphs and triangulate them
	err = LoadAndTesselateGlyphs(dataPath + "cards.ttf", (GReal)1e-7);
	if (err != G_NO_ERROR) {
		perror("Fail to load cards.ttf");
		abort();
	}
	// load cards and animations
	err = LoadCardsAndAnimations(dataPath + "cards.xml");
	if (err != G_NO_ERROR) {
		perror("Fail to load cards.xml");
		abort();
	}
	// load and initialize background
	LoadBackGraoundAndLogo(dataPath + "stars.png");
	// initialize random generator
	GMath::SeedRandom();
	// begin with 7 cards
	gMaxCards = 7;
	gLogicTick = 0.13;
	gBackGroundTime = 0;
	gBackGroundTimeStep = 0.0003;
	gAnimCards.resize(gMaxCards);
}

void DeleteCard(GAnimTRSNode2D *Node) {

	G_ASSERT(Node != NULL);

	// delete TRS nodes in inverse order (from children to root) to avoid too many expensive SetFather()
	GInt32 i, j = (GInt32)Node->ChildrenCounts();
	for (i = j - 1; i >= 0; i--)
		DeleteCard(Node->Child(i));
	// delete style
	GGlyphStyle *style = (GGlyphStyle *)Node->CustomData();
	if (style)
		delete style;
	delete Node;
}

void BirthAnimCard(const GUInt32 Index) {

	if (Index >= (GUInt32)gAnimCards.size())
		return;

	// select a random card
	GInt32 i = GMath::RangeRandom(0, (GInt32)gCards.size() - 1);
	gAnimCards[Index].Card = gCards[i];

	if ((i >= 7 && i <= 9) || (i >= 17 && i <= 19) || (i >= 27 && i <= 29) || (i >= 37 && i <= 39)) {
		// select a random animation
		i = GMath::RangeRandom(0, (GInt32)gAnimations.size() - 1);
	}
	else {
		// select a random animation
		i = GMath::RangeRandom(0, (GInt32)gAnimations.size() - 2);
	}
	gAnimCards[Index].Animation = gAnimations[i];

	// calculate a random lifeticks
	gAnimCards[Index].LifeTicks = (GReal)GMath::RangeRandom(130.0, 180.0);
	// reset life
	gAnimCards[Index].ElapsedTicks = 0;
}

void KillApp() {

	GUInt32 i, j;

	if (gExtManager)
		delete gExtManager;

	// delete tessellated glyphs
	j = (GUInt32)gTesselatedGlyphs.size();
	for (i = 0; i < j; ++i) {
		GTesselatedGlyph *g = gTesselatedGlyphs[i];
		G_ASSERT(g != NULL);
		delete g;
	}
	// delete poker cards
	j = (GUInt32)gCards.size();
	for (i = 0; i < j; ++i)
		DeleteCard(gCards[i]);
	// now delete kernel (and animations)
	if (gKernel)
		delete gKernel;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) {

	if (height == 0)
		height = 1;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-LOGIC_SIZE, LOGIC_SIZE, -LOGIC_SIZE, LOGIC_SIZE, -LOGIC_SIZE, LOGIC_SIZE);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int InitGL(GLvoid) {

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.4f, 0.4f, 0.4f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);				// Set Line Antialiasing
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE);
	glDisable(GL_CULL_FACE);
	setDefaultGlobalStates();
	// generate display lists of all glyphs
	GUInt32 i, j = (GUInt32)gTesselatedGlyphs.size();
	for (i = 0; i < j; i++)
		gTesselatedGlyphs[i]->GenDisplayList();
	// bind background texture to OpenGL
	glGenTextures(1, &gBindedTexture);
	glBindTexture(GL_TEXTURE_2D, gBindedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gBackGround->Width(), gBackGround->Height(),
				 0, GL_BGRA, GL_UNSIGNED_BYTE, gBackGround->Pixels());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return TRUE;
}

void DrawGlyph(const GAnimTRSNode2D *Node, const GMatrix33 AnimMatrix) {

	G_ASSERT(Node);

	GUInt32 i, j;
	GTimeInterval validInterval;
	GMatrix33 worldTransform, pivotMatrix, totalMatrix;
	GMatrix44 oglMatrix;

	GGlyphStyle *style = (GGlyphStyle *)Node->CustomData();
	G_ASSERT(style != NULL);
	const GTesselatedGlyph *tessGlyph = (const GTesselatedGlyph *)gTesselatedGlyphs[style->GlyphIndex()];
	G_ASSERT(tessGlyph != NULL);

	// extract world transformation
	worldTransform = Node->Matrix(0, G_WORLD_SPACE, validInterval);
	pivotMatrix = Node->PivotMatrix();
	totalMatrix = (AnimMatrix * (worldTransform * pivotMatrix));
	Matrix33To44(totalMatrix, oglMatrix);

	glMatrixMode(GL_MODELVIEW);
#ifdef DOUBLE_REAL_TYPE
	glLoadMatrixd((GLdouble *)oglMatrix.Data());
	glColor3d(style->Color()[G_X], style->Color()[G_Y], style->Color()[G_Z]);
#else
	glLoadMatrixf((GLfloat *)oglMatrix.Data());
	glColor3f(style->Color()[G_X], style->Color()[G_Y], style->Color()[G_Z]);
#endif
	// draw the display list
	glCallList(tessGlyph->DisplayList());
	// draw children
	j = Node->ChildrenCounts();
	for (i = 0; i < j; ++i)
		DrawGlyph(Node->Child(i), AnimMatrix);
}

void DrawLogo(const GVectBase<GReal, 3>& Color, const GMatrix33 AnimMatrix) {

	GMatrix44 oglMatrix;

	Matrix33To44(AnimMatrix, oglMatrix);
	glMatrixMode(GL_MODELVIEW);
#ifdef DOUBLE_REAL_TYPE
	glLoadMatrixd((GLdouble *)oglMatrix.Data());
	glColor3d(Color[G_X], Color[G_Y], Color[G_Z]);
#else
	glLoadMatrixf((GLfloat *)oglMatrix.Data());
	glColor3d(Color[G_X], Color[G_Y], Color[G_Z]);
#endif
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	// draw the logo
	glCallList(gTesselatedGlyphs[70]->DisplayList());
	glDisable(GL_BLEND);
}

void DrawBackGround(const GVectBase<GReal, 3>& Color, const GMatrix33 TextureMatrix) {

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, gBindedTexture);

	GMatrix44 oglMatrix;
	Matrix33To44(TextureMatrix, oglMatrix);
#ifdef DOUBLE_REAL_TYPE
	glColor3d(Color[G_X], Color[G_Y], Color[G_Z]);
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixd((GLdouble *)oglMatrix.Data());
#else
	glColor3f(Color[G_X], Color[G_Y], Color[G_Z]);
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf((GLfloat *)oglMatrix.Data());
#endif
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f((GLfloat)-LOGIC_SIZE, (GLfloat)LOGIC_SIZE);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((GLfloat)LOGIC_SIZE, (GLfloat)LOGIC_SIZE);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((GLfloat)LOGIC_SIZE, (GLfloat)-LOGIC_SIZE);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f((GLfloat)-LOGIC_SIZE, (GLfloat)-LOGIC_SIZE);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void DrawAnimCard(const GAnimCard& AnimCard) {

	G_ASSERT(AnimCard.IsAlive());

	GTimeInterval validInterval;
	GMatrix33 worldMatrix;
	GReal t = AnimCard.ElapsedTicks / AnimCard.LifeTicks;

	worldMatrix = AnimCard.Animation->Matrix(t, G_WORLD_SPACE, validInterval);
	DrawGlyph(AnimCard.Card, worldMatrix);
}

int DrawGLScene(GLvoid)	{

	if (arbMultisampleSupported) {
		if (activateFSAA)
			glEnable(GL_MULTISAMPLE_ARB);
		else
			glDisable(GL_MULTISAMPLE_ARB);
	}

	setLightAndTransform();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// first draw background
	GMatrix33 uvMatrix;
	GKeyValue colorVal;
	GVector3 color;
	GTimeInterval validInterval;

	// extract update background animations and draw it
	uvMatrix = gBackGroundAnimUV->Matrix(gBackGroundTime, G_WORLD_SPACE, validInterval);
	gBackGroundAnimColor->Property("x")->Value(colorVal, validInterval, gBackGroundTime);
	color[G_X] = colorVal.RealValue();
	gBackGroundAnimColor->Property("y")->Value(colorVal, validInterval, gBackGroundTime);
	color[G_Y] = colorVal.RealValue();
	gBackGroundAnimColor->Property("z")->Value(colorVal, validInterval, gBackGroundTime);
	color[G_Z] = colorVal.RealValue();
	DrawBackGround(color, uvMatrix);

	GUInt32 i, j = (GUInt32)gAnimCards.size();
	// time passes also for cards...
	if (!gBlockAnim)
		for (i = 0; i < j; i++)
			gAnimCards[i].ElapsedTicks += gLogicTick;

	// lets arise new cards
	for (i = 0; i < gMaxCards; i++) {
		if (!gAnimCards[i].IsAlive())
			BirthAnimCard(i);
	}
	// draw alive cards
	for (i = 0; i < j; i++) {
		if (gAnimCards[i].IsAlive())
			DrawAnimCard(gAnimCards[i]);
	}
	// draw logo
	uvMatrix = gAnimLogo->Matrix(gBackGroundTime, G_WORLD_SPACE, validInterval);
	DrawLogo(GPoint3(0.6, 0.6, 0.6), (uvMatrix * gAnimLogo->PivotMatrix()));
	if (!gBlockAnim)
		gBackGroundTime += gBackGroundTimeStep;
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
	if (!CreateGLWindow("Shape animation example - Press F1 for help", 720, 720, 16, fullscreen))
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
				s = "A/Z: Add/Remove a poker card.\n";
				s += "B: Toggle animation On/Off.\n";
				s += "O: Toggle antialias (if supported) On/Off.\n";
				s += "N/M: Increase/Decrease animation speed.";
				MessageBox(NULL, StrUtils::ToAscii(s), "Command keys", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
			}
			// A key
			if (keys[65]) {
				keys[65] = FALSE;
				gMaxCards++;
				gAnimCards.push_back(GAnimCard());
			}
			// Z key
			if (keys[90]) {
				keys[90] = FALSE;
				if (gMaxCards > 0) {
					gMaxCards--;
					gAnimCards.pop_back();
				}
			}
			// B key
			if (keys[66]) {
				keys[66] = FALSE;
				gBlockAnim = !gBlockAnim;
			}
			// M key
			if (keys[77]) {
				keys[77] = FALSE;
				gLogicTick *= (GReal)1.2;
				gBackGroundTimeStep *= (GReal)1.2;
			}
			// N key
			if (keys[78]) {
				keys[78] = FALSE;
				gLogicTick /= (GReal)1.2;
				gBackGroundTimeStep /= (GReal)1.2;
			}
			// O key
			if (keys[79]) {
				keys[79] = FALSE;
				activateFSAA = !activateFSAA;
			}
		}
	}
	// Shutdown
	KillGLWindow();									// Kill The Window
	KillApp();
	return (int)(msg.wParam);							// Exit The Program
}
