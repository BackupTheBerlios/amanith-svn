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

#include "drawanim.h"
#include <amanith/2d/gtesselator2d.h>
#include <amanith/geometry/gxformconv.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <new>

// QT4 support
#ifdef USE_QT4
	#include <QTimerEvent>
	#include <QKeyEvent>
#endif

static int timer_interval = 0;			// timer interval (millisec)
#define LOGIC_SIZE 1.3

// convert an affine 3x3 matrix to its correspondent 4x4 matrix
static void Matrix33To44(const GMatrix33& Matrix33, GMatrix44& Matrix44) {

	Identity(Matrix44);
	Matrix44[0][0] = Matrix33[0][0];
	Matrix44[0][1] = Matrix33[0][1];
	Matrix44[1][0] = Matrix33[1][0];
	Matrix44[1][1] = Matrix33[1][1];
	Matrix44[0][3] = Matrix33[0][2];
	Matrix44[1][3] = Matrix33[1][2];
	Matrix44[2][3] = Matrix33[2][2];
}

//------------------------------------------------------------
GTesselatedGlyph::GTesselatedGlyph(const GFontChar2D* FontChar, const GReal Deviation) {

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

//------------------------------------------------------------
void GTesselatedGlyph::GenDisplayList() {

	// generate display list
	gDisplayList = glGenLists(1);
	glNewList(gDisplayList, GL_COMPILE);

	GUInt32 i, j = (GUInt32)gPoints.size();
	glBegin(GL_TRIANGLES);
	for (i = 0; i < j; i+=3) {
		glVertex2d(gPoints[i + 0][G_X], gPoints[i + 0][G_Y]);
		glVertex2d(gPoints[i + 1][G_X], gPoints[i + 1][G_Y]);
		glVertex2d(gPoints[i + 2][G_X], gPoints[i + 2][G_Y]);
	}
	glEnd();
	glEndList();
}

//------------------------------------------------------------
QGLWidgetTest::QGLWidgetTest(QWidget * parent) : QGLWidget(parent) {

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
	// begin with 9 cards
	gMaxCards = 9;
	gBlockAnim = G_FALSE;
	gLogicTick = 0.5;
	gAnimCards.resize(gMaxCards);
	// resize the window
	QWidget *d = (QWidget *)QApplication::desktop();
	GInt32 winSize = GMath::Min(d->width(), d->height());
	this->resize(winSize-64, winSize-64);
}

//------------------------------------------------------------
void QGLWidgetTest::LoadBackGraoundAndLogo(const GString& FileName) {

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
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)-2));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.33, (GReal)2));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.67, (GReal)0));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)-2));
	p->SetKeys(tmpKeys);
	// y-position
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("position")->Property("y");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)1));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.33, (GReal)2));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.67, (GReal)-2));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)1));
	p->SetKeys(tmpKeys);
	// rotation
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("rotation");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)0));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.25, (GReal)1.57 * 2));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.50, (GReal)3.14 * 2));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.75, (GReal)4.71 * 2));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)6.28 * 2));
	p->SetKeys(tmpKeys);
	// x-scale
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("scale")->Property("x");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)1));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.33, (GReal)3));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.67, (GReal)0.9));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)1));
	p->SetKeys(tmpKeys);
	// y-scale
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimUV->Property("transform")->Property("scale")->Property("y");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)1));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.33, (GReal)3));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.67, (GReal)0.9));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)1));
	p->SetKeys(tmpKeys);

	// red color
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimColor->Property("x");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)0.7));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.50, (GReal)0.0));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.75, (GReal)0.0));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)0.7));
	p->SetKeys(tmpKeys);
	// green color
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimColor->Property("y");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)0.0));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.50, (GReal)0.1));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.75, (GReal)0.7));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)0.0));
	p->SetKeys(tmpKeys);
	// blue color
	tmpKeys.clear();
	p = (GHermiteProperty1D *)gBackGroundAnimColor->Property("z");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)0.5));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.50, (GReal)0.6));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.75, (GReal)0.3));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)0.5));
	p->SetKeys(tmpKeys);

	// now set up logo
	gAnimLogo = (GAnimTRSNode2D *)gKernel->CreateNew(G_ANIMTRSNODE2D_CLASSID);	
	gAnimLogo->SetPivotPosition(GPoint2((GReal)0.550, (GReal)0.540), G_FALSE);

	gAnimLogo->Property("transform")->Property("position")->Property("x")->SetDefaultValue(GKeyValue((GReal)0.6));
	gAnimLogo->Property("transform")->Property("position")->Property("y")->SetDefaultValue(GKeyValue((GReal)-1.23));

	// x-scale
	tmpKeys.clear();
	gAnimLogo->Property("transform")->Property("scale")->Property("y")->SetDefaultValue(GKeyValue((GReal)1.5));
	p = (GHermiteProperty1D *)gAnimLogo->Property("transform")->Property("scale")->Property("x");
	p->SetOORAfter(G_LOOP_OOR);
	tmpKeys.push_back(GKeyValue((GTimeValue)0.00, (GReal)1.5));
	tmpKeys.push_back(GKeyValue((GTimeValue)0.50, (GReal)-1.5));
	tmpKeys.push_back(GKeyValue((GTimeValue)1.00, (GReal)1.5));
	p->SetKeys(tmpKeys);

	gBackGroundTime = 0;
	gBackGroundTimeStep = (GReal)0.001;
}

//------------------------------------------------------------
void QGLWidgetTest::DeleteCard(GAnimTRSNode2D *Node) {

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

//------------------------------------------------------------
QGLWidgetTest::~QGLWidgetTest() {

	GUInt32 i, j;

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
	// now delete kernel (background pixelmap and animations)
	if (gKernel)
		delete gKernel;
}

//------------------------------------------------------------
void QGLWidgetTest::timerEvent(QTimerEvent *e) {

	if (!e)
		return;
	updateGL();
}

//------------------------------------------------------------
void QGLWidgetTest::initializeGL() {

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.4f, 0.4f, 0.4f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);				// Set Line Antialiasing
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE);
	glDisable(GL_CULL_FACE);
	setDefaultGlobalStates();
	startTimer(timer_interval);
	// generate display lists of all glyphs
	GUInt32 i, j = (GUInt32)gTesselatedGlyphs.size();
	for (i = 0; i < j; i++)
		gTesselatedGlyphs[i]->GenDisplayList();
	// bind background texture to OpenGL
	glGenTextures(1, &gBindedTexture);
	glBindTexture(GL_TEXTURE_2D, gBindedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gBackGround->Width(), gBackGround->Height(),
				 0, GL_BGRA, GL_UNSIGNED_BYTE, gBackGround->Pixels());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

//------------------------------------------------------------
void QGLWidgetTest::DrawLogo(const GVectBase<GReal, 3>& Color, const GMatrix33 AnimMatrix) {

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

//------------------------------------------------------------
void QGLWidgetTest::DrawBackGround(const GVectBase<GReal, 3>& Color, const GMatrix33 TextureMatrix) {

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

//------------------------------------------------------------
void QGLWidgetTest::DrawAnimCard(const GAnimCard& AnimCard) {

	GTimeInterval validInterval;
	GMatrix33 worldMatrix;
	GReal t = AnimCard.ElapsedTicks / AnimCard.LifeTicks;

	worldMatrix = AnimCard.Animation->Matrix(t, G_WORLD_SPACE, validInterval);
	DrawGlyph(AnimCard.Card, worldMatrix);
}

//------------------------------------------------------------
void QGLWidgetTest::paintGL() {

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
	DrawLogo(GPoint3((GReal)0.6, (GReal)0.6, (GReal)0.6), (uvMatrix * gAnimLogo->PivotMatrix()));
	if (!gBlockAnim)
		gBackGroundTime += gBackGroundTimeStep;
	glFlush();
}

//------------------------------------------------------------
void QGLWidgetTest::resizeGL(int width, int height) {

	

	if (height == 0)									// Prevent A Divide By Zero By
		height = 1;										// Making Height Equal One
	glViewport(0, 0, width, height);					// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glOrtho(-LOGIC_SIZE, LOGIC_SIZE, -LOGIC_SIZE, LOGIC_SIZE, -LOGIC_SIZE, LOGIC_SIZE);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();
}

//------------------------------------------------------------
void QGLWidgetTest::keyPressEvent(QKeyEvent *e) {

	QString s;

	switch(e->key()) {
		case Qt::Key_F1:
			s = "A/Z: Add/Remove a poker card.\n";
			s += "B: Toggle animation On/Off.\n";
			s += "N/M: Increase/Decrease animation speed.";
			QMessageBox::information(this, "Command keys", s);
			break;
		case Qt::Key_A:
			gMaxCards++;
			gAnimCards.push_back(GAnimCard());
			break;
		case Qt::Key_Z:
			if (gMaxCards > 0) {
				gMaxCards--;
				gAnimCards.pop_back();
			}
			break;
		case Qt::Key_B:
			gBlockAnim = !gBlockAnim;
			break;
		case Qt::Key_N:
			gLogicTick /= (GReal)1.2;
			gBackGroundTimeStep /= (GReal)1.2;
			break;
		case Qt::Key_M:
			gLogicTick *= (GReal)1.2;
			gBackGroundTimeStep *= (GReal)1.2;
			break;
	}
}

//------------------------------------------------------------
void QGLWidgetTest::setLightAndTransform() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//------------------------------------------------------------
void QGLWidgetTest::setDefaultGlobalStates() {

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

//------------------------------------------------------------
void QGLWidgetTest::BirthAnimCard(const GUInt32 Index) {

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

//------------------------------------------------------------
GError QGLWidgetTest::LoadAndTesselateGlyphs(const GString& FileName, const GReal Deviation) {

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

//------------------------------------------------------------
void QGLWidgetTest::DrawGlyph(const GAnimTRSNode2D *Node, const GMatrix33 AnimMatrix) const {

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

//------------------------------------------------------------
GError QGLWidgetTest::LoadAnimFromXML(const TiXmlNode *XMLNode, GAnimTRSNode2D *TRSNode) {

	const TiXmlElement *trackElem;
	const TiXmlElement *keyElem;
	const GChar8 *strVal;
	GVector2 tmpVect;
	GTimeValue keyTime;
	GHermiteProperty1D *p;
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

//------------------------------------------------------------
GError QGLWidgetTest::LoadCardGlyphFromXML(const TiXmlElement *XMLElem, GAnimTRSNode2D *Father) {

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
GError QGLWidgetTest::LoadCardFromXML(const TiXmlElement *XMLElem) {

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

//------------------------------------------------------------
GError QGLWidgetTest::LoadCardsAndAnimations(const GString& FileName) {

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
