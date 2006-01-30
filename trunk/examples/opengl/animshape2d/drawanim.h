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

#ifndef DRAW_BEZIER_H
#define DRAW_BEZIER_H

#include <amanith/gkernel.h>
#include <amanith/2d/ganimtrsnode2d.h>
#include <amanith/gmultiproperty.h>
#include <amanith/gopenglext.h>
#include <amanith/2d/gpixelmap.h>
#include <amanith/2d/gfont2d.h>
#ifdef USE_QT4
#include <QGLWidget>
#else
#include <qgl.h>
#endif
#include "tinyxml.h"

using namespace Amanith;

// a flattened glyph
class GTesselatedGlyph {

private:
	GDynArray< GPoint<GDouble, 2> > gPoints;
	GLuint gDisplayList;

public:
	GTesselatedGlyph(const GFontChar2D* FontChar, const GReal Deviation);
	inline const GDynArray< GPoint<GDouble, 2> >& Points() const {
		return gPoints;
	}
	inline GLuint DisplayList() const {
		return gDisplayList;
	}
	void GenDisplayList();
};

// glyph drawing style (material)
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


// animated card
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


class QGLWidgetTest : public QGLWidget {

private:
	GKernel *gKernel;
	TiXmlDocument gXMLDoc;
	// triangulated glyphs
	GDynArray<GTesselatedGlyph *> gTesselatedGlyphs;
	// compound poker cards
	GDynArray<GAnimTRSNode2D *> gCards;
	// cards animations
	GDynArray<GAnimTRSNode2D *> gAnimations;
	// number of max cards
	GUInt32 gMaxCards;
	GReal gLogicTick;
	GDynArray<GAnimCard> gAnimCards;
	GBool gBlockAnim;

	// background stuff (texture, time, color animation, texture animation)
	GPixelMap *gBackGround;
	GAnimTRSNode2D *gBackGroundAnimUV;
	GAnimTRSNode2D *gAnimLogo;
	GThreeHermiteProperty1D *gBackGroundAnimColor;
	GReal gBackGroundTime;
	GReal gBackGroundTimeStep;
	GLuint gBindedTexture;

	void setLightAndTransform();					// Set initial light and transform
	void setDefaultGlobalStates();					// Set initial states

	void TesselateChar(const GFontChar2D *FontChar, const GReal Deviation);
	GError LoadAndTesselateGlyphs(const GString& FileName, const GReal Deviation);
	void DeleteCard(GAnimTRSNode2D *Node);
	GError LoadAnimFromXML(const TiXmlNode *XMLNode, GAnimTRSNode2D *TRSNode);
	GError LoadCardGlyphFromXML(const TiXmlElement *XMLElem, GAnimTRSNode2D *Father);
	GError LoadCardFromXML(const TiXmlElement *XMLElem);
	GError LoadCardsAndAnimations(const GString& FileName);
	void DrawGlyph(const GAnimTRSNode2D *Node, const GMatrix33 AnimMatrix) const;
	void DrawBackGround(const GVectBase<GReal, 3>& Color, const GMatrix33 TextureMatrix);
	void DrawLogo(const GVectBase<GReal, 3>& Color, const GMatrix33 AnimMatrix);

	void LoadBackGraoundAndLogo(const GString& FileName);
	void DrawAnimCard(const GAnimCard& AnimCard);
	void BirthAnimCard(const GUInt32 Index);

protected:
	void initializeGL();					// implementation for QGLWidget.initializeGL()
    void paintGL();							// implementation for QGLWidget.paintGL()
	void resizeGL(int width, int height);	// implementation for QGLWidget.resizeGL()
	void keyPressEvent(QKeyEvent *e);		// keyboard event handler

public:
	// constructor
#ifdef USE_QT4
	QGLWidgetTest(const QGLFormat& Format, QWidget *parent = 0);
#else
	QGLWidgetTest(QWidget *parent = 0);
#endif
	// destructor
	~QGLWidgetTest();
	// timer event
	void timerEvent(QTimerEvent* e);
};

#endif
