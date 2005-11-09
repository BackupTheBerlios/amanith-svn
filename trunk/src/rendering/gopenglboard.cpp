/****************************************************************************
** $file: amanith/src/rendering/gopenglboard.cpp   0.1.1.0   edited Sep 24 08:00
**
** OpenGL based draw board implementation.
**
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

#include "amanith/rendering/gopenglboard.h"
#include "amanith/1d/ghermitecurve1d.h"
#include "amanith/1d/gpolylinecurve1d.h"
#include "amanith/gmultiproperty.h"
#include "amanith/geometry/gxform.h"
#include "amanith/geometry/gxformconv.h"
#include <new>

/*!
	\file gopenglboard.cpp
	\brief OpenGL based draw board implementation file.
*/

namespace Amanith {

// *********************************************************************
//                          GOpenGLGradientDesc
// *********************************************************************

// default constructor
GOpenGLGradientDesc::GOpenGLGradientDesc() {

	gGradientTexture = 0;
	gAlphaKeys = G_FALSE;
}

// destructor
GOpenGLGradientDesc::~GOpenGLGradientDesc() {
}

void GOpenGLGradientDesc::SetGLGradientQuality(const GRenderingQuality Quality) {

	// set texture min/mag filters
	switch (Quality) {
		case G_LOW_RENDERING_QUALITY:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;

		case G_NORMAL_RENDERING_QUALITY:
		case G_HIGH_RENDERING_QUALITY:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
	}
}

void GOpenGLGradientDesc::UpdateOpenGLTexture(const GRenderingQuality Quality, const GUInt32 MaxTextureSize) {

	GUInt32 size = 512;

	// texture has sense for linear gradients only
	if (Type() != G_LINEAR_GRADIENT)
		return;

	// texture size depends on rendering quality
	switch (Quality) {
		case G_LOW_RENDERING_QUALITY:
			size = 256;
			break;
		case G_NORMAL_RENDERING_QUALITY:
			size = 512;
			break;
		case G_HIGH_RENDERING_QUALITY:
			size = 1024;
			break;
	}
	if (size > MaxTextureSize)
		size = MaxTextureSize;

	// create pixelmap
	GPixelMap pixMap;
	pixMap.Create(size, 1, G_A8R8G8B8);
	GUChar8 *pixels = pixMap.Pixels();

	// hermite color interpolation
	if (ColorInterpolation() == G_HERMITE_COLOR_INTERPOLATION) {

		const GDynArray<GKeyValue>& keys = ColorKeys();
		GUInt32 i, j = (GUInt32)keys.size(), pixelOfs;
		GHermiteCurve1D redCurve, greenCurve, blueCurve, alphaCurve;
		GDynArray<GHermiteKey1D> redKeys, greenKeys, blueKeys, alphaKeys;
		GReal u, step;
		GUChar8 r8, g8, b8, a8;

		for (i = 0; i < j; i++) {
			GVector4 v = keys[i].Vect4Value();
			redKeys.push_back(GHermiteKey1D(keys[i].TimePosition(), v[G_X]));
			greenKeys.push_back(GHermiteKey1D(keys[i].TimePosition(), v[G_Y]));
			blueKeys.push_back(GHermiteKey1D(keys[i].TimePosition(), v[G_Z]));
			alphaKeys.push_back(GHermiteKey1D(keys[i].TimePosition(), v[G_W]));
		}
		redCurve.SetKeys(redKeys);
		greenCurve.SetKeys(greenKeys);
		blueCurve.SetKeys(blueKeys);
		alphaCurve.SetKeys(alphaKeys);

		step = redCurve.Domain().Length() / (GReal)(size - 1);
		u = redCurve.DomainStart();
		pixelOfs = 0;
		for (i = 0; i < size; i++) {
			r8 = (GUChar8)(redCurve.Evaluate(u) * (GReal)255);
			g8 = (GUChar8)(greenCurve.Evaluate(u) * (GReal)255);
			b8 = (GUChar8)(blueCurve.Evaluate(u) * (GReal)255);
			a8 = (GUChar8)(alphaCurve.Evaluate(u) * (GReal)255);
			pixels[pixelOfs++] = r8;
			pixels[pixelOfs++] = g8;
			pixels[pixelOfs++] = b8;
			pixels[pixelOfs++] = a8;
			u += step;
		}		
	}
	else
	// linear color interpolation
	if (ColorInterpolation() == G_LINEAR_COLOR_INTERPOLATION) {

		const GDynArray<GKeyValue>& keys = ColorKeys();
		GUInt32 i, j = (GUInt32)keys.size(), pixelOfs;
		GPolyLineCurve1D redCurve, greenCurve, blueCurve, alphaCurve;
		GDynArray<GPolyLineKey1D> redKeys, greenKeys, blueKeys, alphaKeys;
		GReal u, step;
		GUChar8 r8, g8, b8, a8;

		for (i = 0; i < j; i++) {
			GVector4 v = keys[i].Vect4Value();
			redKeys.push_back(GPolyLineKey1D(keys[i].TimePosition(), v[G_X]));
			greenKeys.push_back(GPolyLineKey1D(keys[i].TimePosition(), v[G_Y]));
			blueKeys.push_back(GPolyLineKey1D(keys[i].TimePosition(), v[G_Z]));
			alphaKeys.push_back(GPolyLineKey1D(keys[i].TimePosition(), v[G_W]));
		}
		redCurve.SetKeys(redKeys);
		greenCurve.SetKeys(greenKeys);
		blueCurve.SetKeys(blueKeys);
		alphaCurve.SetKeys(alphaKeys);

		step = redCurve.Domain().Length() / (GReal)(size - 1);
		u = redCurve.DomainStart();
		pixelOfs = 0;
		for (i = 0; i < size; i++) {
			r8 = (GUChar8)(redCurve.Evaluate(u) * (GReal)255);
			g8 = (GUChar8)(greenCurve.Evaluate(u) * (GReal)255);
			b8 = (GUChar8)(blueCurve.Evaluate(u) * (GReal)255);
			a8 = (GUChar8)(alphaCurve.Evaluate(u) * (GReal)255);
			pixels[pixelOfs++] = r8;
			pixels[pixelOfs++] = g8;
			pixels[pixelOfs++] = b8;
			pixels[pixelOfs++] = a8;
			u += step;
		}
	}
	// constant color interpolation
	else {
		const GDynArray<GKeyValue>& keys = ColorKeys();
		GUInt32 i, j = (GUInt32)keys.size(), pixelOfs;
		GConstantProperty1D redCurve, greenCurve, blueCurve, alphaCurve;
		GDynArray<GKeyValue> redKeys, greenKeys, blueKeys, alphaKeys;
		GKeyValue val;
		GTimeInterval valid;
		GReal u, step;
		GUChar8 r8, g8, b8, a8;

		for (i = 0; i < j; i++) {
			GVector4 v = keys[i].Vect4Value();
			redKeys.push_back(GKeyValue(keys[i].TimePosition(), v[G_X]));
			greenKeys.push_back(GKeyValue(keys[i].TimePosition(), v[G_Y]));
			blueKeys.push_back(GKeyValue(keys[i].TimePosition(), v[G_Z]));
			alphaKeys.push_back(GKeyValue(keys[i].TimePosition(), v[G_W]));
		}
		redCurve.SetKeys(redKeys);
		greenCurve.SetKeys(greenKeys);
		blueCurve.SetKeys(blueKeys);
		alphaCurve.SetKeys(alphaKeys);

		step = redCurve.Domain().Length() / (GReal)(size - 1);
		u = redCurve.Domain().Start();
		pixelOfs = 0;
		for (i = 0; i < size; i++) {
			redCurve.Value(val, valid, u, G_ABSOLUTE_VALUE);
			r8 = (GUChar8)(val.RealValue() * (GReal)255);
			greenCurve.Value(val, valid, u, G_ABSOLUTE_VALUE);
			g8 = (GUChar8)(val.RealValue() * (GReal)255);
			blueCurve.Value(val, valid, u, G_ABSOLUTE_VALUE);
			b8 = (GUChar8)(val.RealValue() * (GReal)255);
			alphaCurve.Value(val, valid, u, G_ABSOLUTE_VALUE);
			a8 = (GUChar8)(val.RealValue() * (GReal)255);
			pixels[pixelOfs++] = r8;
			pixels[pixelOfs++] = g8;
			pixels[pixelOfs++] = b8;
			pixels[pixelOfs++] = a8;
			u += step;
		}
	}
	// generate OpenGL texture
	glGenTextures(1, &gGradientTexture);
	glBindTexture(GL_TEXTURE_1D, gGradientTexture);
	SetGLGradientQuality(Quality);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, (GLsizei)size, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pixels);
}

// set color keys
void GOpenGLGradientDesc::SetColorKeys(const GDynArray<GKeyValue>& ColorKeys) {

	GDynArray<GKeyValue>::const_iterator it = ColorKeys.begin();

	GDynArray<GKeyValue> tmpKeys;
	GKeyValue tmpKey;

	for (; it != ColorKeys.end(); ++it) {
		if (it->KeyType() == G_VECTOR3_KEY) {
			GVector3 v = it->Vect3Value();
			tmpKey.SetValue(GVector4(v[G_X], v[G_Y], v[G_Z], (GReal)1.0));
			tmpKeys.push_back(tmpKey);
		}
		else
		if (it->KeyType() == G_VECTOR4_KEY) {

			if (it->Vect4Value()[G_W] < (GReal)1.0)
				gAlphaKeys = G_TRUE;
			tmpKeys.push_back(*it);
		}
	}

	GGradientDesc::SetColorKeys(tmpKeys);
}

// *********************************************************************
//                          GOpenGLPatternDesc
// *********************************************************************

// default constructor
GOpenGLPatternDesc::GOpenGLPatternDesc() {

	gPatternTexture = 0;
}

// destructor
GOpenGLPatternDesc::~GOpenGLPatternDesc() {
}

void GOpenGLPatternDesc::SetGLImageQuality(const GImageQuality Quality) {

	// set texture min/mag filters
	switch (Quality) {
		case G_LOW_IMAGE_QUALITY:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;

		case G_NORMAL_IMAGE_QUALITY:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;

		case G_HIGH_IMAGE_QUALITY:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
	}
}

void GOpenGLPatternDesc::SetImage(const GPixelMap *Image, const GImageQuality Quality) {

	if (!Image)
		return;

	if (Image->IsTrueColor() && (Quality == G_LOW_IMAGE_QUALITY || Quality == G_NORMAL_IMAGE_QUALITY)) {
		glGenTextures(1, &gPatternTexture);
		glBindTexture(GL_TEXTURE_2D, gPatternTexture);
		SetGLImageQuality(Quality);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)Image->Width(), (GLsizei)Image->Height(),
					 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)Image->Pixels());
		return;
	}

	// copy image
	GPixelMap tmpImage;
	tmpImage.CopyFrom(*Image);

	if (!tmpImage.IsTrueColor())
		tmpImage.SetPixelFormat(G_A8R8G8B8);

	// generate texture handle
	glGenTextures(1, &gPatternTexture);
	glBindTexture(GL_TEXTURE_2D, gPatternTexture);
	SetGLImageQuality(Quality);

	// for low and normal quality levels, we can upload texture directly
	if (Quality == G_LOW_IMAGE_QUALITY || Quality == G_NORMAL_IMAGE_QUALITY) {

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)tmpImage.Width(), (GLsizei)tmpImage.Height(),
					 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)tmpImage.Pixels());
		return;
	}

	// generate mipmaps
	GInt32 size = GMath::Min(tmpImage.Width(), tmpImage.Height());
	GInt32 level = 0;

	do {
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, (GLsizei)tmpImage.Width(), (GLsizei)tmpImage.Height(),
					 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)tmpImage.Pixels());

		if (size > 1)
			tmpImage.Resize(tmpImage.Width() / 2, tmpImage.Height() / 2, G_RESIZE_CATMULLROM);

		size /= 2;
		level++;
	} while(size >= 1);
}

// *********************************************************************
//                             GOpenGLBoard
// *********************************************************************

GOpenGLBoard::GOpenGLBoard(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
						   const GUInt32 Width, const GUInt32 Height) : GDrawBoard() {

	gExtManager = new(std::nothrow) GOpenglExt();

	// verify if we can clip using stencil buffer
	if (gExtManager->StencilBits() >= 8) {
		gClipByStencil = G_TRUE;
		glDisable(GL_DEPTH_TEST);
	}
	else
		gClipByStencil = G_FALSE;
	gTopStencilValue = 0;
	// set "old" state of clipping operations
	gFirstClipMaskReplace = G_FALSE;
	// GLSL support
	gShaderSupport = gExtManager->IsArbShadersSupported();

	SetViewport(LowLeftCornerX, LowLeftCornerY, Width, Height);
	SetProjection((GReal)LowLeftCornerX, (GReal)(LowLeftCornerX + Width), (GReal)LowLeftCornerY, (GReal)(LowLeftCornerY + Height)); 
	SetRenderingQuality(G_NORMAL_RENDERING_QUALITY);
	SetImageQuality(G_NORMAL_IMAGE_QUALITY);

	glDisable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
}

GOpenGLBoard::~GOpenGLBoard() {

	DeleteGradients();
	DeletePatterns();
	if (gExtManager)
		delete gExtManager;
}

void GOpenGLBoard::DeleteGradients() {

	GDynArray<GOpenGLGradientDesc *>::iterator it = gGradients.begin();

	for (; it != gGradients.end(); ++it) {
		GOpenGLGradientDesc *gradient = *it;
		G_ASSERT(gradient);
		delete gradient;
	}
	gGradients.clear();
}

void GOpenGLBoard::DeletePatterns() {

	GDynArray<GOpenGLPatternDesc *>::iterator it = gPatterns.begin();

	for (; it != gPatterns.end(); ++it) {
		GOpenGLPatternDesc *pattern = *it;
		G_ASSERT(pattern);
		delete pattern;
	}
	gPatterns.clear();
}

// read only parameters
GUInt32 GOpenGLBoard::MaxDashCount() const {

	return G_MAX_UINT16;
}

GUInt32 GOpenGLBoard::MaxKernelSize() const {

	if (gExtManager)
		return GMath::Min(gExtManager->MaxConvolutionWidth(), gExtManager->MaxConvolutionHeight());
	else
		return 0;
}

GUInt32 GOpenGLBoard::MaxSeparableKernelSize() const {

	return MaxKernelSize();
}

GUInt32 GOpenGLBoard::MaxColorKeys() const {

	return MaxImageWidth();
}

GUInt32 GOpenGLBoard::MaxImageWidth() const {

	if (gExtManager)
		return gExtManager->MaxTextureSize();
	else
		return 0;
}

GUInt32 GOpenGLBoard::MaxImageHeight() const {

	if (gExtManager)
		return gExtManager->MaxTextureSize();
	else
		return 0;
}

GUInt32 GOpenGLBoard::MaxImageBytes() const {

	return (MaxImageWidth() * MaxImageHeight() * 4);
}


// paint resources
GGradientDesc *GOpenGLBoard::CreateLinearGradient(const GPoint2& StartPoint, const GPoint2& EndPoint,
												  const GDynArray<GKeyValue>& ColorKeys,
												  const GColorRampInterpolation Interpolation,
												  const GColorRampSpreadMode SpreadMode,
												  const GMatrix33& Matrix) {

	GOpenGLGradientDesc *g = new(std::nothrow) GOpenGLGradientDesc();
	if (g) {
		g->SetType(G_LINEAR_GRADIENT);
		g->SetStartPoint(StartPoint);
		g->SetAuxPoint(EndPoint);
		g->SetColorKeys(ColorKeys);
		g->SetColorInterpolation(Interpolation);
		g->SetSpreadMode(SpreadMode);
		g->SetMatrix(Matrix);
		gGradients.push_back(g);
	}
	return g;
}

GGradientDesc *GOpenGLBoard::CreateRadialGradient(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
												  const GDynArray<GKeyValue>& ColorKeys,
												  const GColorRampInterpolation Interpolation,
												  const GColorRampSpreadMode SpreadMode,
												  const GMatrix33& Matrix) {

	GOpenGLGradientDesc *g = new(std::nothrow) GOpenGLGradientDesc();
	if (g) {
		g->SetType(G_RADIAL_GRADIENT);
		g->SetStartPoint(Center);
		g->SetAuxPoint(Focus);
		g->SetRadius(Radius);
		g->SetColorKeys(ColorKeys);
		g->SetColorInterpolation(Interpolation);
		g->SetSpreadMode(SpreadMode);
		g->SetMatrix(Matrix);
		gGradients.push_back(g);
	}
	return g;
}

GPatternDesc *GOpenGLBoard::CreatePattern(const GPixelMap *Image, const GTilingMode TilingMode,
										  const GAABox2 *LogicalWindow, const GMatrix33& Matrix) {

 	GOpenGLPatternDesc *p = new(std::nothrow) GOpenGLPatternDesc();
	if (p) {
		p->SetTilingMode(TilingMode);
		p->SetMatrix(Matrix);
		if (LogicalWindow)
			p->SetLogicalWindow(LogicalWindow->Min(), LogicalWindow->Max());
		else {
			GAABox2 tmpBox;
			GReal left, right, bottom, top;
			GReal imageRatio = (GReal)Image->Width() / (GReal)Image->Height();

			Projection(left, right, bottom, top);

			GReal logWidth = GMath::Abs(left - right);
			GReal logHeight = GMath::Abs(top - bottom);

			GReal newLogWidth = logHeight * imageRatio;
			GReal newLogHeight = logWidth / imageRatio;

			if (newLogWidth > logWidth)
				newLogWidth = logWidth;
			else
				newLogHeight = logHeight;

			// calculate box according to current logical coordinate system and preserving image ratio
			tmpBox.SetMinMax(GPoint2(left, bottom), GPoint2(left + newLogWidth, bottom + newLogHeight));
			p->SetLogicalWindow(tmpBox.Min(), tmpBox.Max());
		}
		p->SetImage(Image, ImageQuality());
		gPatterns.push_back(p);
	}
	return p;
}

void GOpenGLBoard::UpdateDeviation(const GRenderingQuality Quality) {

	#define LOW_QUALITY_PIXEL_DEVIATION (1.5 * 1.5)
	#define NORMAL_QUALITY_PIXEL_DEVIATION (0.75 * 0.75)
	#define HIGH_QUALITY_PIXEL_DEVIATION (0.25 * 0.25)

	switch (Quality) {

		case G_LOW_RENDERING_QUALITY:
			gDeviation = CalcDeviation(LOW_QUALITY_PIXEL_DEVIATION);
			break;

		case G_NORMAL_RENDERING_QUALITY:
			gDeviation = CalcDeviation(NORMAL_QUALITY_PIXEL_DEVIATION);
			break;

		case G_HIGH_RENDERING_QUALITY:
			gDeviation = CalcDeviation(HIGH_QUALITY_PIXEL_DEVIATION);
			break;
	}
	G_ASSERT(gDeviation > 0);
	gFlateness = GMath::Sqrt(gDeviation);

	#undef LOW_QUALITY_PIXEL_DEVIATION
	#undef NORMAL_QUALITY_PIXEL_DEVIATION
	#undef HIGH_QUALITY_PIXEL_DEVIATION
}

void GOpenGLBoard::DoSetRenderingQuality(const GRenderingQuality Quality) {

	UpdateDeviation(Quality);
	if (Quality == G_LOW_RENDERING_QUALITY)
		glDisable(GL_MULTISAMPLE_ARB);
	else
		glEnable(GL_MULTISAMPLE_ARB);
}

void GOpenGLBoard::DoSetImageQuality(const GImageQuality Quality) {

	// just to avoid warning
	if (Quality) {
	}
}

void GOpenGLBoard::DoSetTargetMode(const GTargetMode Mode) {

	// just to avoid warning
	if (Mode) {
	}
}

void GOpenGLBoard::DoSetClipOperation(const GClipOperation Operation) {

	// just to avoid warning
	if (Operation) {
	}
//	gClipMaskDrawed = G_FALSE;
}

void GOpenGLBoard::DoSetClipEnabled(const GBool Enabled) {

	// just to avoid warning
	if (Enabled) {
	}
}

void GOpenGLBoard::DoPopClipMask() {

	if (gClipMasksBoxes.size() == 0)
		return;

	// setup stencil operation
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	if (gClipMasksBoxes.size() == 1 && gFirstClipMaskReplace) {
		gTopStencilValue = 0;
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
		gClipMasksBoxes.clear();
	}
	else {
		
		GAABox2 lastClipMaskBox(gClipMasksBoxes.back());
		gClipMasksBoxes.pop_back();

		GPoint2 p0 = lastClipMaskBox.Min();
		GPoint2 p2 = lastClipMaskBox.Max();
		GPoint2 p1(p0[G_X], p2[G_Y]);
		GPoint2 p3(p2[G_X], p0[G_Y]);

		glStencilFunc(GL_EQUAL, (GLint)gTopStencilValue, (GLuint)(~0));
		if (gTopStencilValue > 0) {
			gTopStencilValue--;
			glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
		}
		else
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// pop the mask, drawing a bounding box
		glBegin(GL_POLYGON);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(p0.Data());
			glVertex2dv(p1.Data());
			glVertex2dv(p2.Data());
			glVertex2dv(p3.Data());
		#else
			glVertex2fv(p0.Data());
			glVertex2fv(p1.Data());
			glVertex2fv(p2.Data());
			glVertex2fv(p3.Data());
		#endif
		glEnd();
	}
}

void GOpenGLBoard::DoSetGroupOpacity(const GReal Opacity) {

	// just to avoid warning
	if (Opacity) {
	}
}

void GOpenGLBoard::DoGroupBegin() {
	// to do : grab frame buffer and go on
}

void GOpenGLBoard::DoGroupEnd() {
	// to do : blend grabbed framebuffer according to group opacity
}

void GOpenGLBoard::DoFlush() {

	glFlush();
}

void GOpenGLBoard::DoFinish() {

	glFinish();
}

void GOpenGLBoard::DoClear(const GReal Red, const GReal Green, const GReal Blue, const GBool ClearClipMasks) {

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if (gClipByStencil) {
		glClearColor((GLclampf)Red, (GLclampf)Green, (GLclampf)Blue, 1.0f);
		glClearDepth(1.0);
		if (ClearClipMasks) {
			glClearStencil((GLint)0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			gTopStencilValue = 0;
			gClipMasksBoxes.clear();
		}
		else
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else {
		glClearColor((GLclampf)Red, (GLclampf)Green, (GLclampf)Blue, 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void GOpenGLBoard::SetGLClipEnabled(const GTargetMode Mode, const GClipOperation Operation) {

	if (gClipByStencil) {

		// write to the stencil using current clip operation
		if (Mode == G_CLIP_MODE) {

			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glEnable(GL_STENCIL_TEST);

			switch (Operation) {

				case G_REPLACE_CLIP:
					if (gTopStencilValue > 254) {
						glClearStencil((GLint)0);
						glClear(GL_STENCIL_BUFFER_BIT);
						gTopStencilValue = 0;
					}
					gTopStencilValue++;
					glStencilFunc(GL_ALWAYS, (GLint)gTopStencilValue, (GLuint)(~0));
					glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
					break;

				case G_INTERSECTION_CLIP:

					glStencilFunc(GL_EQUAL, (GLint)gTopStencilValue, (GLuint)(~0));
					if (gTopStencilValue > 254)
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					else {
						gTopStencilValue++;
						glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
					}
					break;

				case G_UNION_CLIP:
					if (gShaderSupport) {
					/*
					if (gTopStencilValue == 0) {
						gTopStencilValue++;
						glStencilFunc(GL_ALWAYS, (GLint)gTopStencilValue, (GLuint)(~0));
						glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
						
					}
					else {
						glStencilFunc(GL_EQUAL, (GLint)gTopStencilValue, (GLuint)(~0));

						if (gTopStencilValue > 254)
							glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
						else
							glStencilOp(GL_REPLACE, GL_INCR, GL_INCR);
					}*/
					}
					else
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					break;
			}
			return;
		}

		// we are drawing in color mode
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		// enable masks clipping
		if (ClipEnabled()) {
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_LEQUAL, (GLint)gTopStencilValue, (GLuint)(~0));
			// do not change stencil buffer
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		}
		// clip masks not enabled
		else
			glDisable(GL_STENCIL_TEST);
	}
	// to do : zbuffer version
	else {
	}
}

void GOpenGLBoard::SetGLColor(const GVectBase<GReal, 4>& Color) {

#ifdef DOUBLE_REAL_TYPE
	glColor4d(Color[G_X], Color[G_Y], Color[G_Z], Color[G_W]);
#else
	glColor4f(Color[G_X], Color[G_Y], Color[G_Z], Color[G_W]);
#endif
}

void GOpenGLBoard::SetGLColor(const GVectBase<GReal, 3>& Color) {

#ifdef DOUBLE_REAL_TYPE
	glColor3d(Color[G_X], Color[G_Y], Color[G_Z]);
#else
	glColor3f(Color[G_X], Color[G_Y], Color[G_Z]);
#endif
}

void GOpenGLBoard::SetGLMatrix(const GMatrix33& Matrix) {

	// convert an affine 3x3 matrix to its correspondent 4x4 matrix
	GMatrix44 m;

	m[0][0] = Matrix[0][0];
	m[0][1] = Matrix[0][1];
	m[1][0] = Matrix[1][0];
	m[1][1] = Matrix[1][1];
	m[0][3] = Matrix[0][2];
	m[1][3] = Matrix[1][2];

	m[2][2] = 0;
	m[3][0] = Matrix[2][0];
	m[3][1] = Matrix[2][1];

#ifdef DOUBLE_REAL_TYPE
	glLoadMatrixd((GLdouble *)m.Data());
#else
	glLoadMatrixf((GLfloat *)m.Data());
#endif
}

void GOpenGLBoard::SetGLTextureMatrix(const GMatrix33& Matrix) {

	// convert an affine 3x3 matrix to its correspondent 4x4 matrix
	GMatrix44 m;

	m[0][0] = Matrix[0][0];
	m[0][1] = Matrix[0][1];
	m[1][0] = Matrix[1][0];
	m[1][1] = Matrix[1][1];
	m[0][3] = Matrix[0][2];
	m[1][3] = Matrix[1][2];

#ifdef DOUBLE_REAL_TYPE
	glLoadMatrixd((GLdouble *)m.Data());
#else
	glLoadMatrixf((GLfloat *)m.Data());
#endif
}

void GOpenGLBoard::UpdateStyle(GDrawStyle& Style) {

	if (Style.StrokeEnabled()) {

		if (Style.StrokePaintType() == G_GRADIENT_PAINT_TYPE && Style.StrokeGradient()) {

			GOpenGLGradientDesc *g = (GOpenGLGradientDesc *)Style.StrokeGradient();
			if (Style.StrokeGradient()->Modified() && Style.StrokeGradient()->Type() == G_LINEAR_GRADIENT) {
				// for linear gradients we must update texture
				g->UpdateOpenGLTexture(this->RenderingQuality(), this->MaxImageWidth());
			}
			g->gModified = 0;
		}

		GOpenGLPatternDesc *p = (GOpenGLPatternDesc *)Style.StrokePattern();
		// pattern image/texture is rebuilt on the fly with SetImage method
		if (p)
			p->gModified = 0;
	}

	if (Style.FillEnabled()) {

		if (Style.FillPaintType() == G_GRADIENT_PAINT_TYPE && Style.FillGradient()) {

			GOpenGLGradientDesc *g = (GOpenGLGradientDesc *)Style.FillGradient();
			if (Style.FillGradient()->Modified() && Style.FillGradient()->Type() == G_LINEAR_GRADIENT) {
				// for linear gradients we must update texture
				g->UpdateOpenGLTexture(this->RenderingQuality(), this->MaxImageWidth());
			}
			g->gModified = 0;
		}

		GOpenGLPatternDesc *p = (GOpenGLPatternDesc *)Style.FillPattern();
		// pattern image/texture is rebuilt on the fly with SetImage method
		if (p)
			p->gModified = 0;
	}
}

void GOpenGLBoard::UseStyle(const GPaintType PaintType, const GVector4& Color,
							const GOpenGLGradientDesc *Gradient, const GOpenGLPatternDesc *Pattern,
							const GMatrix33& ModelView, const GImageQuality ImageQuality) {

	if (TargetMode() == G_CLIP_MODE) {
		/*glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glMatrixMode(GL_MODELVIEW);
		SetGLMatrix(ModelView);*/
		return;
	}

	// color paint
	if (PaintType == G_COLOR_PAINT_TYPE) {

		if (Color[G_W] < (GReal)1) {
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glEnable(GL_BLEND);
		}
		else
			glDisable(GL_BLEND);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		SetGLColor(Color);
	}
	else
	// gradient paint
	if (PaintType == G_GRADIENT_PAINT_TYPE && Gradient) {

		// linear gradient
		if (Gradient->Type() == G_LINEAR_GRADIENT) {

			GVector4 col = Color;
			col[G_X] = col[G_Y] = col[G_Z] = (GReal)1;

			// set alpha pipeline, according to stroke and color keys opacity
			if (Gradient->gAlphaKeys || col[G_W] < (GReal)1) {

				if (Gradient->gAlphaKeys) {
					// we have both stroke and color keys opacity
					if (col[G_W] < (GReal)1) {
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
						SetGLColor(col);
					}
					// we have only color keys opacity
					else {
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
					}
				}
				// we have only stroke opacity
				else {
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
					SetGLColor(col);
				}
				glEnable(GL_BLEND);
			}
			else
				glDisable(GL_BLEND);

			// select gradient texture for RGB channels
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);

			// affine transform end points
			GPoint2 sPoint = Gradient->Matrix() * Gradient->StartPoint();
			GPoint2 ePoint = Gradient->Matrix() * Gradient->AuxPoint();

			// calculate direction
			GVector2 n = ePoint - sPoint;
			n /= n.LengthSquared();
			// calculate translation factor
			GReal q = -Dot(n, static_cast< const GVect<GReal, 2>& >(sPoint));

			// generate the S coordinate using the calculated object plane and place the transformation (in
			// the direction of the normal) in the texture matrix
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();

			#ifdef DOUBLE_REAL_TYPE
				glTranslated(q, 0, 0);
			#else
				glTranslatef(q, 0, 0);
			#endif

			GVector4 plane(n[G_X], n[G_Y], 0, 0);
			// lets use texture coordinate generation in eye space which is in canvas coordinates
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			#ifdef DOUBLE_REAL_TYPE
				glTexGendv(GL_S, GL_EYE_PLANE, (const GLdouble *)plane.Data());
			#else
				glTexGenfv(GL_S, GL_EYE_PLANE, (const GLfloat *)plane.Data());
			#endif

			// enable texture 1D and automatic texture coordinate generation
			glEnable(GL_TEXTURE_1D);
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);

			// bind gradient texture
			glBindTexture(GL_TEXTURE_1D, Gradient->GradientTexture());

			// set spread mode
			switch (Gradient->SpreadMode()) {
				case G_PAD_COLOR_RAMP_SPREAD:
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					break;
				case G_REPEAT_COLOR_RAMP_SPREAD:
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					break;
				case G_REFLECT_COLOR_RAMP_SPREAD:
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
					break;
			}
		}
		// radial gradient
		else {
			glDisable(GL_TEXTURE_1D);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
	}
	// pattern paint
	else {
		
		if (Pattern) {

			GVector4 col = Color;
			col[G_X] = col[G_Y] = col[G_Z] = (GReal)1;

			if (col[G_W] < (GReal)1) {
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				glEnable(GL_BLEND);
			}
			else
				glDisable(GL_BLEND);

			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			SetGLColor(col);

			// lets use texture coordinate generation in eye space which is in canvas coordinates
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

			const GAABox2& patWindow = Pattern->LogicalWindow();
			GReal xAxisLen = patWindow.Dimension(G_X);
			GReal yAxisLen = patWindow.Dimension(G_Y);

			GVector4 planeS(1 / xAxisLen, 0, 0, 0);
			GVector4 planeT(0, -1 / yAxisLen, 0, 0);

			#ifdef DOUBLE_REAL_TYPE
				glTexGendv(GL_S, GL_EYE_PLANE, (const GLdouble *)planeS.Data());
				glTexGendv(GL_T, GL_EYE_PLANE, (const GLdouble *)planeT.Data());
			#else
				glTexGenfv(GL_S, GL_EYE_PLANE, (const GLfloat *)planeS.Data());
				glTexGenfv(GL_T, GL_EYE_PLANE, (const GLfloat *)planeT.Data());
			#endif

			GMatrix33 m, preTrans, postTrans;
			GPoint2 p = -patWindow.Min();
			p[G_X] /= xAxisLen;
			p[G_Y] /= -yAxisLen;
			TranslationToMatrix(preTrans, p);

			TranslationToMatrix(postTrans, GPoint2(0, 1));

			m = (postTrans * (Pattern->Matrix() * preTrans));
			// load texture matrix
			glMatrixMode(GL_TEXTURE);
			SetGLTextureMatrix(m);

			// enable texture 2D
			glDisable(GL_TEXTURE_1D);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);

			// bind gradient texture
			glBindTexture(GL_TEXTURE_2D, Pattern->PatternTexture());

			// set tiling mode
			switch (Pattern->TilingMode()) {
				case G_PAD_TILE:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					break;
				case G_REPEAT_TILE:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					break;
				case G_REFLECT_TILE:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT_ARB);
					break;
			}
		}
	}

	glMatrixMode(GL_MODELVIEW);
	SetGLMatrix(ModelView);
}

void GOpenGLBoard::UseStrokeStyle(const GDrawStyle& Style) {

	UseStyle(Style.StrokePaintType(), Style.StrokeColor(), (const GOpenGLGradientDesc *)Style.StrokeGradient(),
			(const GOpenGLPatternDesc *)Style.StrokePattern(), Style.ModelView(), ImageQuality());
}

void GOpenGLBoard::UseFillStyle(const GDrawStyle& Style) {

	UseStyle(Style.FillPaintType(), Style.FillColor(), (const GOpenGLGradientDesc *)Style.FillGradient(),
			(const GOpenGLPatternDesc *)Style.FillPattern(), Style.ModelView(), ImageQuality());
}

void GOpenGLBoard::DoSetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
								 const GUInt32 Width, const GUInt32 Height) {

	 glViewport(LowLeftCornerX, LowLeftCornerY, Width, Height);
	 UpdateDeviation(RenderingQuality());
}

void GOpenGLBoard::DoSetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top) {

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho((GLdouble)Left, (GLdouble)Right, (GLdouble)Bottom, (GLdouble)Top, -1.0, 1.0);
	UpdateDeviation(RenderingQuality());
}

// calculate (squared) deviation given a (squared) pixel deviation
GReal GOpenGLBoard::CalcDeviation(const GReal PixelDeviation) {

	GReal rX = GMath::Abs(gProjection[G_X] - gProjection[G_Y]) / gViewport[G_Z];
	GReal rY = GMath::Abs(gProjection[G_Z] - gProjection[G_W]) / gViewport[G_W];

	return (PixelDeviation * GMath::Sqr(GMath::Min(rX, rY)));
}

// calculate (squared) pixel deviation given a (squared) deviation
GReal GOpenGLBoard::CalcPixelDeviation(const GReal Deviation) {

	GReal rX = gViewport[G_Z] / GMath::Abs(gProjection[G_X] - gProjection[G_Y]);
	GReal rY = gViewport[G_W] / GMath::Abs(gProjection[G_Z] - gProjection[G_W]);

	return (Deviation * GMath::Sqr(GMath::Min(rX, rY)));
}

void GOpenGLBoard::DrawGLCapsLine(const GBool DoStartCap, const GCapStyle StartCapStyle,
								  const GBool DoEndCap, const GCapStyle EndCapStyle,
								  const GPoint2& P0, const GPoint2& P1, const GReal Thickness) {

	GVector2 dirSeg = P1 - P0;
	// perpendicular direction in CCW respect to dirSeg
	GVector2 perpSeg(-dirSeg[G_Y], dirSeg[G_X]);
	perpSeg *= (Thickness / perpSeg.Length());


	glBegin(GL_POLYGON);

	// calculate fundamentals points
	GPoint2 j0 = P0 + perpSeg;
	GPoint2 j1 = P0 - perpSeg;
	GPoint2 l0 = P1 + perpSeg;
	GPoint2 l1 = P1 - perpSeg;
	GReal l;

	if (StartCapStyle == G_SQUARE_CAP || EndCapStyle == G_SQUARE_CAP)
		l = dirSeg.Normalize() + Thickness;

	// end cap
	if (!DoEndCap) {
		// behavior is like we would have a butt cap
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(l0.Data());
			glVertex2dv(l1.Data());
		#else
			glVertex2fv(l0.Data());
			glVertex2fv(l1.Data());
		#endif
	}
	else {
		GReal l;
		GPoint2 a, b;
		switch (EndCapStyle) {

			case G_BUTT_CAP:
				#ifdef DOUBLE_REAL_TYPE
					glVertex2dv(l0.Data());
					glVertex2dv(l1.Data());
				#else
					glVertex2fv(l0.Data());
					glVertex2fv(l1.Data());
				#endif
				break;

			case G_ROUND_CAP:
				DrawGLCircleSlice(P1, Thickness, l0, l1, G_PI, G_FALSE);
				break;

			case G_SQUARE_CAP:
				a = j0 + l * dirSeg;
				b = j1 + l * dirSeg;
				#ifdef DOUBLE_REAL_TYPE
					glVertex2dv(a.Data());
					glVertex2dv(b.Data());
				#else
					glVertex2fv(a.Data());
					glVertex2fv(b.Data());
				#endif
				break;
		}
	}


	// start cap
	if (!DoStartCap) {
		// behavior is like we would have a butt cap
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j1.Data());
			glVertex2dv(j0.Data());
		#else
			glVertex2fv(j1.Data());
			glVertex2fv(j0.Data());
		#endif
	}
	else {
		GReal l;
		GPoint2 a, b;

		switch (StartCapStyle) {

			case G_BUTT_CAP:
				#ifdef DOUBLE_REAL_TYPE
					glVertex2dv(j1.Data());
					glVertex2dv(j0.Data());
				#else
					glVertex2fv(j1.Data());
					glVertex2fv(j0.Data());
				#endif
				break;

			case G_ROUND_CAP:
				DrawGLCircleSlice(P0, Thickness, j1, j0, G_PI, G_FALSE);
				break;

			case G_SQUARE_CAP:
				a = l0 - l * dirSeg;
				b = l1 - l * dirSeg;
				#ifdef DOUBLE_REAL_TYPE
					glVertex2dv(b.Data());
					glVertex2dv(a.Data());
				#else
					glVertex2fv(b.Data());
					glVertex2fv(a.Data());
				#endif
				break;
		}
	}

	glEnd();
}

void GOpenGLBoard::DrawGLJoinLine(const GJoinStyle JoinStyle, const GReal MiterLimit,
								  const GPoint2& Previous, const GPoint2& P0, const GPoint2& P1,
								  const GReal Thickness) {

	GVector2 dirPrev = P0 - Previous;
	GVector2 dirSeg = P1 - P0;

	// calculate perpendicular vectors (taking care of Thickness of the line)
	GVector2 normPerpPrev(dirPrev[G_Y], -dirPrev[G_X]);
	normPerpPrev.Normalize();
	GVector2 perpPrev(Thickness * normPerpPrev[G_X], Thickness * normPerpPrev[G_Y]);

	GVector2 normPerpSeg(dirSeg[G_Y], -dirSeg[G_X]);
	normPerpSeg.Normalize();
	GVector2 perpSeg(Thickness * normPerpSeg[G_X], Thickness * normPerpSeg[G_Y]);

	// make sure the normal vectors are pointing outwards
	if (Cross(perpPrev, perpSeg) < 0) {
		perpPrev = -perpPrev;
		perpSeg = -perpSeg;
	}


	glBegin(GL_POLYGON);

	// calculate fundamentals points
	GPoint2 j0 = P0 + perpPrev;
	GPoint2 j1 = P0 + perpSeg;
	GPoint2 l0 = j1 + dirSeg;
	GPoint2 l2 = P0 - perpSeg;
	GPoint2 l1 = l2 + dirSeg;

	// round join
	if (JoinStyle == G_ROUND_JOIN) {
		GReal roundJoinAngle = GMath::Acos(Dot(normPerpPrev, normPerpSeg));
		DrawGLCircleSlice(P0, Thickness, j0, j1, roundJoinAngle, CounterClockWise(P0, j0, j1));
	}
	else
	// miter join
	if (JoinStyle == G_MITER_JOIN) {

		GRay2 prevRay, segRay;
		GUInt32 intFlags;
		GReal intParam[2];
		GBool intFound;

		GPoint2 o = Previous + perpPrev;
		prevRay.SetOrigin(o);
		prevRay.SetDirection(dirPrev);

		segRay.SetOrigin(l0);
		segRay.SetDirection(-dirSeg);

		intFound = Intersect(prevRay, segRay, intFlags, intParam);
		if (intFound) {

			GPoint2 intPoint = prevRay.Origin() + (prevRay.Direction() * intParam[0]);
			GVector2 intDir = intPoint - P0;
			GReal intDirLen = intDir.Normalize();

			if (intDirLen > MiterLimit * Thickness)
				intPoint = P0 + (MiterLimit * Thickness) * intDir;

			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(j0.Data());
				glVertex2dv(intPoint.Data());
				glVertex2dv(j1.Data());
			#else
				glVertex2fv(j0.Data());
				glVertex2fv(intPoint.Data());
				glVertex2fv(j1.Data());
			#endif
		}
	}
	// bevel join
	else {
		G_ASSERT(JoinStyle == G_BEVEL_JOIN);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j0.Data());
			glVertex2dv(j1.Data());
		#else
			glVertex2fv(j0.Data());
			glVertex2fv(j1.Data());
		#endif
	}

	#ifdef DOUBLE_REAL_TYPE
		glVertex2dv(l0.Data());
		glVertex2dv(P1.Data());
		glVertex2dv(l1.Data());
		glVertex2dv(l2.Data());
	#else
		glVertex2fv(l0.Data());
		glVertex2fv(P1.Data());
		glVertex2fv(l1.Data());
		glVertex2fv(l2.Data());
	#endif

	glEnd();
}

void GOpenGLBoard::DrawGLJoinLineCap(const GJoinStyle JoinStyle, const GReal MiterLimit,
									 const GPoint2& Previous, const GPoint2& P0, const GPoint2& P1,
									 const GReal Thickness, const GCapStyle EndCapStyle) {


	GVector2 dirPrev = P0 - Previous;
	GVector2 dirSeg = P1 - P0;

	// calculate perpendicular vectors (taking care of Thickness of the line)
	GVector2 normPerpPrev(dirPrev[G_Y], -dirPrev[G_X]);
	normPerpPrev.Normalize();
	GVector2 perpPrev(Thickness * normPerpPrev[G_X], Thickness * normPerpPrev[G_Y]);

	GVector2 normPerpSeg(dirSeg[G_Y], -dirSeg[G_X]);
	normPerpSeg.Normalize();
	GVector2 perpSeg(Thickness * normPerpSeg[G_X], Thickness * normPerpSeg[G_Y]);

	// make sure the normal vectors are pointing outwards
	if (Cross(perpPrev, perpSeg) < 0) {
		perpPrev = -perpPrev;
		perpSeg = -perpSeg;
	}


	glBegin(GL_POLYGON);

	// calculate fundamentals points
	GPoint2 j0 = P0 + perpPrev;
	GPoint2 j1 = P0 + perpSeg;
	GPoint2 l0 = j1 + dirSeg;
	GPoint2 l2 = P0 - perpSeg;
	GPoint2 l1 = l2 + dirSeg;

	// round join
	if (JoinStyle == G_ROUND_JOIN) {
		GReal roundJoinAngle = GMath::Acos(Dot(normPerpPrev, normPerpSeg));
		DrawGLCircleSlice(P0, Thickness, j0, j1, roundJoinAngle, CounterClockWise(P0, j0, j1));
	}
	else
	// miter join
	if (JoinStyle == G_MITER_JOIN) {

		GRay2 prevRay, segRay;
		GUInt32 intFlags;
		GReal intParam[2];
		GBool intFound;

		GPoint2 o = Previous + perpPrev;
		prevRay.SetOrigin(o);
		prevRay.SetDirection(dirPrev);

		segRay.SetOrigin(l0);
		segRay.SetDirection(-dirSeg);

		intFound = Intersect(prevRay, segRay, intFlags, intParam);
		if (intFound) {

			GPoint2 intPoint = prevRay.Origin() + (prevRay.Direction() * intParam[0]);
			GVector2 intDir = intPoint - P0;
			GReal intDirLen = intDir.Normalize();

			if (intDirLen > MiterLimit * Thickness)
				intPoint = P0 + (MiterLimit * Thickness) * intDir;

			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(j0.Data());
				glVertex2dv(intPoint.Data());
				glVertex2dv(j1.Data());
			#else
				glVertex2fv(j0.Data());
				glVertex2fv(intPoint.Data());
				glVertex2fv(j1.Data());
			#endif
		}
	}
	// bevel join
	else {
		G_ASSERT(JoinStyle == G_BEVEL_JOIN);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j0.Data());
			glVertex2dv(j1.Data());
		#else
			glVertex2fv(j0.Data());
			glVertex2fv(j1.Data());
		#endif
	}

	GReal l;
	switch (EndCapStyle) {

		case G_BUTT_CAP:
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(l0.Data());
				glVertex2dv(l1.Data());
				glVertex2dv(l2.Data());
			#else
				glVertex2fv(l0.Data());
				glVertex2fv(l1.Data());
				glVertex2fv(l2.Data());
			#endif
			break;

		case G_ROUND_CAP:
			DrawGLCircleSlice(P1, Thickness, l0, l1, G_PI, CounterClockWise(P0, l0, l1));
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(l2.Data());
			#else
				glVertex2fv(l2.Data());
			#endif
			break;

		case G_SQUARE_CAP:
			l = dirSeg.Normalize() + Thickness;
			l0 = j1 + l * dirSeg;
			l1 = l2 + l * dirSeg;
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(l0.Data());
				glVertex2dv(l1.Data());
				glVertex2dv(l2.Data());
			#else
				glVertex2fv(l0.Data());
				glVertex2fv(l1.Data());
				glVertex2fv(l2.Data());
			#endif
			break;
	}
	glEnd();
}

void GOpenGLBoard::DrawGLCircleSlice(const GPoint2& Center, const GReal Radius, const GPoint2& Start,
									 const GPoint2& End, const GReal SpanAngle, const GBool CCW) {

	 // MaxDeviation is a squared chordal distance, we must report to this value to a linear chordal distance
	GReal dev = GMath::Clamp(gFlateness, G_EPSILON, Radius - (G_EPSILON * Radius)), deltaAngle;
	GUInt32 n = 2;

	GReal n1 = SpanAngle / ((GReal)2 * GMath::Acos((GReal)1 - dev / Radius));
	if (n1 > 2 && n1 >= (SpanAngle * G_ONE_OVER_PI)) {
		if (n1 > n)
			n = (GUInt32)GMath::Ceil(n1);
	}
	G_ASSERT(n >= 1);

	// generate points
	if (CCW)
		deltaAngle = (SpanAngle / n);
	else
		deltaAngle = (-SpanAngle / n);
	GReal cosDelta = GMath::Cos(deltaAngle);
	GReal sinDelta = GMath::Sin(deltaAngle);

	GPoint2 p(Start - Center), q, r;
	// draw cap

#ifdef DOUBLE_REAL_TYPE
	glVertex2dv(Start.Data());
#else
	glVertex2fv(Start.Data());
#endif
	for (GUInt32 i = 0; i < n - 1; ++i) {
		q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
		r = q + Center;
#ifdef DOUBLE_REAL_TYPE
		glVertex2dv(r.Data());
#else
		glVertex2fv(r.Data());
#endif
		p = q;
	}
#ifdef DOUBLE_REAL_TYPE
	glVertex2dv(End.Data());
#else
	glVertex2fv(End.Data());
#endif
}

void GOpenGLBoard::DrawGLJoin(const GPoint2& JoinCenter, const GVector2& InDirection, const GReal InDistance,
							  const GVector2& OutDirection, const GReal OutDistance, const GJoinStyle JoinStyle,
							  const GReal MiterLimit, const GCapStyle StartCapStyle, const GCapStyle EndCapStyle,
							  const GReal Thickness) {

	if (InDistance < G_EPSILON && OutDistance < G_EPSILON)
		return;

	GVector2 normPerpInExt(InDirection[G_Y], -InDirection[G_X]);
	GVector2 normPerpOutExt(OutDirection[G_Y], -OutDirection[G_X]);

	// make sure the normal vectors are pointing outwards
	if (Cross(normPerpInExt, normPerpOutExt) < 0) {
		  normPerpInExt = -normPerpInExt;
		  normPerpOutExt = -normPerpOutExt;
	}

	GVector2 perpInExt(normPerpInExt[G_X] * Thickness, normPerpInExt[G_Y] * Thickness);
	GVector2 perpOutExt(normPerpOutExt[G_X] * Thickness, normPerpOutExt[G_Y] * Thickness);

	GVector2 vTmp = (normPerpInExt + normPerpOutExt) * (GReal)0.5;
	vTmp.Normalize();

	GPoint2 j0 = JoinCenter + perpInExt;
	GPoint2 j1 = JoinCenter + perpOutExt;
	GPoint2 t0 = JoinCenter - perpInExt;
	GPoint2 t1 = JoinCenter - perpOutExt;
	GPoint2 v0 = t0 - InDistance * InDirection;
	GPoint2 v1 = t1 + OutDistance * OutDirection;
	GPoint2 k0 = j0 - InDistance * InDirection;
	GPoint2 k1 = j1 + OutDistance * OutDirection;

	// we must find vc using t0 and t1 that are length independent
	GUInt32 intFlags;
	GReal intParam[2];
	GRay2 r0(t0, -InDirection);
	GRay2 r1(t1, OutDirection);
	GBool intFound = Intersect(r0, r1, intFlags, intParam);
	// this could be the case of 2 collinear and opposite directions
	if (!intFound)
		return;

	// calculate fundamentals points
	GPoint2 a, b, jc;

	GPoint2 vc = t0 - InDirection * intParam[0];
	GPoint2 w0 = vc + ((GReal)2 * perpInExt);
	GPoint2 w1 = vc + ((GReal)2 * perpOutExt);

	GReal halfRoundJoinAngle = GMath::Acos(Dot(normPerpInExt, normPerpOutExt)) * (GReal)0.5;

	// handle degenerative distance cases
	GVector2 n = v0 - vc;
	if (InDistance < G_EPSILON || Dot(n, InDirection) > 0) {
		k0 = w0;
		v0 = vc;
	}
	GPoint2 s0 = (k0 + v0) * (GReal)0.5;
	// calculate ccw/cw direction
	GBool ccw = CounterClockWise(j0, t0, s0);

	glBegin(GL_POLYGON);
	// start cap
	switch (StartCapStyle) {

		case G_BUTT_CAP:
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(v0.Data());
				glVertex2dv(k0.Data());
			#else
				glVertex2fv(v0.Data());
				glVertex2fv(k0.Data());
			#endif
			break;

		case G_ROUND_CAP:
			DrawGLCircleSlice(s0, Thickness, v0, k0, G_PI, ccw);
			break;

		case G_SQUARE_CAP:
			a = k0 - Thickness * InDirection;
			b = v0 - Thickness * InDirection;
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(b.Data());
				glVertex2dv(a.Data());
			#else
				glVertex2fv(b.Data());
				glVertex2fv(a.Data());
			#endif
			break;
	}

	// join
	if (JoinStyle == G_BEVEL_JOIN) {
		jc = (j0 + j1) * (GReal)0.5;
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j0.Data());
			glVertex2dv(jc.Data());
		#else
			glVertex2fv(j0.Data());
			glVertex2fv(jc.Data());
		#endif
	}
	else
	if (JoinStyle == G_ROUND_JOIN) {
		jc = JoinCenter + Thickness * vTmp;
		DrawGLCircleSlice(JoinCenter, Thickness, j0, jc, halfRoundJoinAngle, ccw);
	}
	else {
		G_ASSERT(JoinStyle == G_MITER_JOIN);

		GVector2 intDir = JoinCenter - vc;
		jc = JoinCenter + intDir;
		GReal intDirLen = intDir.Normalize();

		if (intDirLen > MiterLimit * Thickness)
			jc = JoinCenter + (MiterLimit * Thickness) * intDir;

		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(j0.Data());
			glVertex2dv(jc.Data());
		#else
			glVertex2fv(j0.Data());
			glVertex2fv(jc.Data());
		#endif
	}

	#ifdef DOUBLE_REAL_TYPE
		glVertex2dv(w1.Data());
		glVertex2dv(vc.Data());
	#else
		glVertex2fv(w1.Data());
		glVertex2fv(vc.Data());
	#endif
	glEnd();


	// handle degenerative distance cases
	n = vc - v1;
	if (OutDistance < G_EPSILON || Dot(n, OutDirection) > 0) {
		k1 = w1;
		v1 = vc;
	}
	GPoint2 s1 = (k1 + v1) * (GReal)0.5;


	glBegin(GL_POLYGON);
	// end cap
	switch (EndCapStyle) {

		case G_BUTT_CAP:
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(k1.Data());
				glVertex2dv(v1.Data());
			#else
				glVertex2fv(k1.Data());
				glVertex2fv(v1.Data());
			#endif
			break;

		case G_ROUND_CAP:
			DrawGLCircleSlice(s1, Thickness, k1, v1, G_PI, ccw);
			break;

		case G_SQUARE_CAP:
			a = k1 + Thickness * OutDirection;
			b = v1 + Thickness * OutDirection;
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(a.Data());
				glVertex2dv(b.Data());
			#else
				glVertex2fv(a.Data());
				glVertex2fv(b.Data());
			#endif
			break;
	}

	#ifdef DOUBLE_REAL_TYPE
		glVertex2dv(vc.Data());
		glVertex2dv(w0.Data());
	#else
		glVertex2fv(vc.Data());
		glVertex2fv(w0.Data());
	#endif

	switch (JoinStyle) {

		case G_BEVEL_JOIN:
		case G_MITER_JOIN:
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(jc.Data());
				glVertex2dv(j1.Data());
			#else
				glVertex2fv(jc.Data());
				glVertex2fv(j1.Data());
			#endif
			break;

		case G_ROUND_JOIN:
			DrawGLCircleSlice(JoinCenter, Thickness, jc, j1, halfRoundJoinAngle, ccw);
			break;
	}
	glEnd();
}

void GOpenGLBoard::IntersectRaysDisk(const GRay2& Ray0, const GRay2& Ray1, const GPoint2& Center, const GReal Radius,
									  GPoint2& P0, GPoint2& P1, GReal& SpannedAngle) const {

	GUInt32 intFlags;
	GReal intParams[2];
	GBool intFound;
	GSphere2 sph(Center, Radius);

	intFound = Intersect(Ray0, sph, intFlags, intParams);
	G_ASSERT(intFound == G_TRUE);
	G_ASSERT(intFlags & SINGLE_SOLUTION);
	P0 = Ray0.Origin() + intParams[0] * Ray0.Direction();

	intFound = Intersect(Ray1, sph, intFlags, intParams);
	G_ASSERT(intFound == G_TRUE);
	G_ASSERT(intFlags & SINGLE_SOLUTION);
	P1 = Ray1.Origin() + intParams[0] * Ray1.Direction();

	GVector2 d0(P0 - Center);
	GVector2 d1(P1 - Center);
	GReal cr = Cross(d0, d1);

	// smaller angle in CCW, going from P0 to P1
	if (cr > 0)
		SpannedAngle = GMath::Acos(Dot(d0, d1) / (d0.Length() * d1.Length()));
	else
		SpannedAngle = G_2PI - GMath::Acos(Dot(d0, d1) / (d0.Length() * d1.Length()));

	G_ASSERT(SpannedAngle >= 0);
}

void GOpenGLBoard::DrawGLShadedSector(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
									  const GReal Time0, const GReal Time1,
									  const GPoint2& P0, const GPoint2& P1, const GBool WholeDisk,
									  const GDynArray<GKeyValue>& ColorKeys, const GColorRampInterpolation Interpolation,
									  const GColorRampSpreadMode SpreadMode) const {

	/*
		NB: color keys must be sorted by TimePosition() !!!
	*/
	if (Time0 >= Time1)
		return;

	G_ASSERT(Time0 >= 0 && Time1 >= 0);

	GPoint2 e0, e1;
	GVector2 normDirFC = Center - Focus;
	GVector2 dirFC = normDirFC;
	GReal distFC = normDirFC.Normalize();

	// handle the case where Center == Focus
	if (distFC <= G_EPSILON) {
		distFC = 0;
		normDirFC.Set(1, 0);
	}

	GRay2 rayP0(Focus, P0 - Focus);
	GRay2 rayP1(Focus, P1 - Focus);

	// calculate radius corresponding to Time1
	GReal maxRadius = (distFC + Radius) * Time1;
	GPoint2 maxCenter = Focus + Time1 * dirFC;
	GReal deltaAngle;

	if (!WholeDisk)
		IntersectRaysDisk(rayP0, rayP1, maxCenter, maxRadius, e0, e1, deltaAngle);
	else
		deltaAngle = G_2PI;

	// MaxDeviation is a squared chordal distance, we must report to this value to a linear chordal distance
	// now calculate the number of segments to produce (number of times we have to subdivide angle) that
	// permit to have a squared chordal distance less than gDeviation
	GReal dev = GMath::Clamp(gFlateness, G_EPSILON, maxRadius - (G_EPSILON * maxRadius));
	GInt32 n = 1;
	GReal n1 = deltaAngle / (2 * GMath::Acos(1 - dev / maxRadius));
	if (n1 > 1 && n1 >= (deltaAngle * G_ONE_OVER_PI)) {
		if (n1 > n)
			n = (GUInt32)GMath::Ceil(n1);
	}
	G_ASSERT(n >= 1);


	// calculate a new array of color keys, taking care of spread mode and color interpolation
	GInt32 i, w, k, keyDir, j = (GInt32)ColorKeys.size();
	GTimeValue t, tOld, tSum;
	GDynArray<GKeyValue> tmpKeys;
	GReal ratio;

	// now we are sure that we have at least 2 keys
	switch (SpreadMode) {

		case G_PAD_COLOR_RAMP_SPREAD:

			// skip all keys <= Time0
			k = 0;
			while (k < j && ColorKeys[k].TimePosition() <= Time0)
				k++;

			// push a new key at Time0
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - 1]);
					tmpKeys.back().SetTimePosition(Time0);
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:

					if (k == j) {
						tmpKeys.push_back(ColorKeys[k - 1]);
						tmpKeys.back().SetTimePosition(Time0);
					}
					else {
						ratio = (Time0 - ColorKeys[k - 1].TimePosition()) / (ColorKeys[k].TimePosition() - ColorKeys[k - 1].TimePosition());
						tmpKeys.push_back(GKeyValue(Time0, GMath::Lerp(ratio, ColorKeys[k - 1].Vect4Value(), ColorKeys[k].Vect4Value())));
					}
					break;
			}

			// push all intermediate keys ( > Time0 and < Time1)
			while (k < j && ColorKeys[k].TimePosition() < Time1) {
				tmpKeys.push_back(ColorKeys[k]);
				k++;
			}

			// push a new key at Time1
			// interpolate color value between key k-1 and k
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - 1]);
					tmpKeys.back().SetTimePosition(Time1);
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					if (k == j) {
						tmpKeys.push_back(ColorKeys[k - 1]);
						tmpKeys.back().SetTimePosition(Time1);
					}
					else {
						ratio = (Time1 - ColorKeys[k - 1].TimePosition()) / (ColorKeys[k].TimePosition() - ColorKeys[k - 1].TimePosition());
						tmpKeys.push_back(GKeyValue(Time1, GMath::Lerp(ratio, ColorKeys[k - 1].Vect4Value(), ColorKeys[k].Vect4Value())));
					}
					break;
			}
			break;

		case G_REPEAT_COLOR_RAMP_SPREAD:

			k = 0;
			t = tOld = ColorKeys[0].TimePosition();
			tSum = 0;

			// skip all keys <= Time0
			while (tSum <= Time0) {
				tOld = t;
				k++;
				t = ColorKeys[k % j].TimePosition();
				tSum += (t - tOld);
			}

			// push a new key at Time0
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - 1]);
					tmpKeys.back().SetTimePosition(Time0);
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					ratio = (Time0 - tSum + (t - tOld)) / (t - tOld);
					tmpKeys.push_back(GKeyValue(Time0, GMath::Lerp(ratio, ColorKeys[(k - 1) % j].Vect4Value(), ColorKeys[k % j].Vect4Value())));
					break;
			}

			// push all intermediate keys ( > Time0 and < Time1)
			while (tSum < Time1) {

				tOld = t;
				k++;
				if (k >= j) {
					tmpKeys.push_back(ColorKeys[(k - 1) % j]);
					tmpKeys.back().SetTimePosition(tSum);
					k = 0;
					t = tOld = ColorKeys[0].TimePosition();
				}
				else {
					tmpKeys.push_back(ColorKeys[(k - 1) % j]);
					tmpKeys.back().SetTimePosition(tSum);
					t = ColorKeys[k % j].TimePosition();
					tSum += (t - tOld);
				}
			}

			// push a new key at Time1
			// interpolate color value between key k-1 and k
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[(k - 1) % j]);
					tmpKeys.back().SetTimePosition(Time1);
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					ratio = (Time1 - tSum + (t - tOld)) / (t - tOld);
					tmpKeys.push_back(GKeyValue(Time1, GMath::Lerp(ratio, ColorKeys[(k - 1) % j].Vect4Value(), ColorKeys[k % j].Vect4Value())));
					break;
			}
			break;

		case G_REFLECT_COLOR_RAMP_SPREAD:

			keyDir = 1;

			// skip all keys <= Time0
			k = 0;
			t = tOld = ColorKeys[0].TimePosition();
			tSum = 0;

			// skip all keys <= Time0
			while (tSum <= Time0) {

				tOld = t;

				k += keyDir;
				if (k < 0) {
					keyDir = 1;
					k = 0;
				}
				else
				if (k >= j) {
					keyDir = -1;
					k = j - 1;
				}

				t = ColorKeys[k].TimePosition();
				tSum += GMath::Abs(t - tOld);
			}

			// push a new key at Time0
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - keyDir]);
					tmpKeys.back().SetTimePosition(Time0);
					break;
				
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					ratio = (Time0 - tSum + GMath::Abs(t - tOld)) / GMath::Abs(t - tOld);
					tmpKeys.push_back(GKeyValue(Time0, GMath::Lerp(ratio, ColorKeys[(k - keyDir)].Vect4Value(), ColorKeys[k].Vect4Value())));
					break;
			}

			// push all intermediate keys ( > Time0 and < Time1)
			while (tSum < Time1) {

				tOld = t;
				
				k += keyDir;
				if (k < 0) {
					keyDir = 1;
					k = 1;
				}
				else
				if (k >= j) {
					keyDir = -1;
					k = j - 2;
				}

				tmpKeys.push_back(ColorKeys[(k - keyDir)]);
				tmpKeys.back().SetTimePosition(tSum);

				t = ColorKeys[k].TimePosition();
				tSum += GMath::Abs(t - tOld);
			}

			// push a new key at Time1
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - keyDir]);
					tmpKeys.back().SetTimePosition(Time1);
					break;

				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					ratio = (Time1 - tSum + GMath::Abs(t - tOld)) / GMath::Abs(t - tOld);
					tmpKeys.push_back(GKeyValue(Time1, GMath::Lerp(ratio, ColorKeys[(k - keyDir)].Vect4Value(), ColorKeys[k].Vect4Value())));
					break;
			}

			break;
	}

	
	GDynArray< GVectBase<GReal, 2> > ptsCache(n + 1);
	GReal cosDelta;
	GReal sinDelta;
	GReal r;
	GPoint2 p, q, m, c;
	GVector4 col, oldCol;

	if (Time0 == 0) {
		i = 1;
		t = tmpKeys[i].TimePosition();
		oldCol = tmpKeys[0].Vect4Value();
		r = t * Radius;
		c = Focus + t * dirFC;

		if (!WholeDisk) {
			IntersectRaysDisk(rayP0, rayP1, c, r, e0, e1, deltaAngle);
			p = e0 - c;
			m = e0;
		}
		else {
			p = r * normDirFC;
			m = c + p;
		}

		cosDelta = GMath::Cos(deltaAngle / n);
		sinDelta = GMath::Sin(deltaAngle / n);

		ptsCache[0] = m;

		// first draw triangle fan
		glBegin(GL_TRIANGLE_FAN);

		#ifdef DOUBLE_REAL_TYPE
			glColor4dv(oldCol.Data());
			glVertex2dv(Focus.Data());
		#else
			glColor4fv(oldCol.Data());
			glVertex2fv(Focus.Data());
		#endif

		if (Interpolation != G_CONSTANT_COLOR_INTERPOLATION)
			oldCol = tmpKeys[1].Vect4Value();

		#ifdef DOUBLE_REAL_TYPE
			glColor4dv(oldCol.Data());
			glVertex2dv(m.Data());
		#else
			glColor4fv(oldCol.Data());
			glVertex2fv(m.Data());
		#endif

		for (k = 0; k < n - 1; ++k) {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
			ptsCache[k + 1] = m;
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(m.Data());
			#else
				glVertex2fv(m.Data());
			#endif
			p = q;
		}

		// push last point (avoiding precision errors for the case 2PI)
		if (deltaAngle == G_2PI)
			m = c + r * normDirFC;
		else {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
		}
		ptsCache[k + 1] = m;
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(m.Data());
		#else
			glVertex2fv(m.Data());
		#endif

		glEnd();

		if (Interpolation == G_CONSTANT_COLOR_INTERPOLATION)
			oldCol = tmpKeys[1].Vect4Value();
	}
	else {
		i = 0;
		t = tmpKeys[i].TimePosition();
		oldCol = tmpKeys[i].Vect4Value();
		r = t * Radius;
		c = Focus + t * dirFC;

		if (!WholeDisk) {
			IntersectRaysDisk(rayP0, rayP1, c, r, e0, e1, deltaAngle);
			p = e0 - c;
			m = e0;
		}
		else {
			p = r * normDirFC;
			m = c + p;
		}

		ptsCache[0] = m;
		cosDelta = GMath::Cos(deltaAngle / n);
		sinDelta = GMath::Sin(deltaAngle / n);

		for (k = 0; k < n - 1; ++k) {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
			ptsCache[k + 1] = m;
			p = q;
		}
		// push last point (avoiding precision errors for the case 2PI)
		if (deltaAngle == G_2PI)
			m = c + r * normDirFC;
		else {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
		}
		ptsCache[k + 1] = m;
	}

	i++;
	k = (GInt32)tmpKeys.size();

	for (; i < k; ++i) {

		t = tmpKeys[i].TimePosition();
		if (Interpolation == G_CONSTANT_COLOR_INTERPOLATION)
			col = oldCol;
		else
			col = tmpKeys[i].Vect4Value();

		r = t * Radius;
		c = Focus + t * dirFC;

		if (!WholeDisk) {
			IntersectRaysDisk(rayP0, rayP1, c, r, e0, e1, deltaAngle);
			p = e0 - c;
			m = e0;
		}
		else {
			p = r * normDirFC;
			m = c + p;
		}

		cosDelta = GMath::Cos(deltaAngle / n);
		sinDelta = GMath::Sin(deltaAngle / n);


		glBegin(GL_TRIANGLE_STRIP);

		// push first point
		#ifdef DOUBLE_REAL_TYPE
			glColor4dv(oldCol.Data());
			glVertex2dv(ptsCache[0].Data());
			glColor4dv(col.Data());
			glVertex2dv(m.Data());
		#else
			glColor4dv(oldCol.Data());
			glVertex2fv(ptsCache[0].Data());
			glColor4dv(col.Data());
			glVertex2fv(m.Data());
		#endif
		ptsCache[0] = m;

		for (w = 0; w < n - 1; ++w) {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
			#ifdef DOUBLE_REAL_TYPE
				glColor4dv(oldCol.Data());
				glVertex2dv(ptsCache[w + 1].Data());
				glColor4dv(col.Data());
				glVertex2dv(m.Data());
			#else
				glColor4dv(oldCol.Data());
				glVertex2fv(ptsCache[w + 1].Data());
				glColor4dv(col.Data());
				glVertex2fv(m.Data());
			#endif
			ptsCache[w + 1] = m;
			p = q;
		}
		// push last point (avoiding precision errors for the case 2PI)
		if (deltaAngle == G_2PI)
			m = c + r * normDirFC;
		else {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
		}
		#ifdef DOUBLE_REAL_TYPE
			glColor4dv(oldCol.Data());
			glVertex2dv(ptsCache[w + 1].Data());
			glColor4dv(col.Data());
			glVertex2dv(m.Data());
		#else
			glColor4dv(oldCol.Data());
			glVertex2fv(ptsCache[w + 1].Data());
			glColor4dv(col.Data());
			glVertex2fv(m.Data());
		#endif

		ptsCache[w + 1] = m;

		if (Interpolation == G_CONSTANT_COLOR_INTERPOLATION)
			oldCol = tmpKeys[i].Vect4Value();
		else
			oldCol = col;

		glEnd();
	}

}

GInt32 GOpenGLBoard::SignBoxDisk(const GAABox2& Box, const GPoint2& Center, const GReal Radius) {

	GVector2 deltaCenter = Center - Box.Center();
	GReal distX = GMath::Abs(deltaCenter[G_X]) - Box.HalfDimension(G_X);
	GReal distY = GMath::Abs(deltaCenter[G_Y]) - Box.HalfDimension(G_Y);
	// if both distances are negative, then Distance = Max(distX, distY)
	// if one distance is negative, then Distance = the positive one
	// else Arvo's algorithm
	if (distX <= 0) {
		if (distY <= 0)
			return GMath::Sign(GMath::Max(distX, distY) - Radius);
		else
			return GMath::Sign(distY - Radius);
	}
	else {
		if (distY <= 0)
			return GMath::Sign(distX - Radius);
		else
			// Arvo's algorithm
			return GMath::Sign((distX * distX) + (distY * distY) - Radius * Radius);
	}
}

void GOpenGLBoard::DrawShadedSector(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
									const GAABox2& BoundingBox,
									const GDynArray<GKeyValue>& ColorKeys, const GColorRampInterpolation Interpolation,
									const GColorRampSpreadMode SpreadMode) const {


	GPoint2 p0 = BoundingBox.Min();
	GPoint2 p2 = BoundingBox.Max();
	GPoint2 p1(p2[G_X], p0[G_Y]);
	GPoint2 p3(p0[G_X], p2[G_Y]);

	GPoint2 pMax, pMin;
	GVector2 v;
	GReal dMax, tMax, tMin;
	GSphere2 sph(Center, Radius);
	GBool intFound;
	GUInt32 intFlags;
	GReal intParams[2];
	GRay2 ray;
	GBool wholeDisk = G_FALSE;

	ray.SetOrigin(Focus);
	tMax = 0;

	// calculate the maximum radial value (distance between point and focus divided by the length of the
	// line segment starting at focus, passing through the point, and ending on the circumference of the
	// gradient circle)	of box corners
	ray.SetDirection(p0 - Focus);
	intFound = Intersect(ray, sph, intFlags, intParams);
	if (intFound) {

		G_ASSERT(intFlags & SINGLE_SOLUTION);
		v = (ray.Origin() + intParams[0] * ray.Direction()) - Focus;
		dMax = ray.Direction().LengthSquared() / v.LengthSquared();
		if (dMax > tMax)
			tMax = dMax;
	}
	ray.SetDirection(p1 - Focus);
	intFound = Intersect(ray, sph, intFlags, intParams);
	if (intFound) {
		G_ASSERT(intFlags & SINGLE_SOLUTION);
		v = (ray.Origin() + intParams[0] * ray.Direction()) - Focus;
		dMax = ray.Direction().LengthSquared() / v.LengthSquared();
		if (dMax > tMax)
			tMax = dMax;
	}
	ray.SetDirection(p2 - Focus);
	intFound = Intersect(ray, sph, intFlags, intParams);
	if (intFound) {
		G_ASSERT(intFlags & SINGLE_SOLUTION);
		v = (ray.Origin() + intParams[0] * ray.Direction()) - Focus;
		dMax = ray.Direction().LengthSquared() / v.LengthSquared();
		if (dMax > tMax)
			tMax = dMax;
	}
	ray.SetDirection(p3 - Focus);
	intFound = Intersect(ray, sph, intFlags, intParams);
	if (intFound) {
		G_ASSERT(intFlags & SINGLE_SOLUTION);
		v = (ray.Origin() + intParams[0] * ray.Direction()) - Focus;
		dMax = ray.Direction().LengthSquared() / v.LengthSquared();
		if (dMax > tMax)
			tMax = dMax;
	}
	// the final tMax
	tMax = GMath::Sqrt(tMax);


	// if the focus is inside the box, whole disk must be rendered (and in this case tMin is 0)
	if ((Focus[G_X] > BoundingBox.Min()[G_X] && Focus[G_X] < BoundingBox.Max()[G_X]) &&
		(Focus[G_Y] > BoundingBox.Min()[G_Y] && Focus[G_Y] < BoundingBox.Max()[G_Y])) {
		wholeDisk = G_TRUE;
		tMin = 0;
	}
	else {
		GUInt32 curSteps = 0;
		GReal tTail = 0;
		GReal tHead = tMax;

		GVector2 dirFC = Center - Focus;
		GReal lenFC = dirFC.Length();

		// it handles the case when focus and center are the same 
		if (lenFC <= G_EPSILON) {
			dirFC = BoundingBox.Center() - Focus;
			lenFC = dirFC.Length();
		}

		// now use a bisection iterative method to find a good bound for tMin
		do {
			GReal tPivot = (tTail + tHead) * (GReal)0.5;
			GInt32 signPivot = SignBoxDisk(BoundingBox, Focus + tPivot * dirFC, tPivot * Radius);

			if (signPivot == 0) {
				tTail = tPivot;
				break;
			}
			else
			if (signPivot < 0)
				tHead = tPivot;
			else
				tTail = tPivot;
			curSteps++;
		} while(curSteps < 5);
		tMin = tTail;

		// if we have identified a sector we must calculate "external" points
		dirFC /= lenFC;

		// calculate axes transformation (transform box corners int the coordinate system given by Focus-Center
		// direction
		GMatrix22 A;
		A.Set(dirFC[G_X], dirFC[G_Y], -dirFC[G_Y], dirFC[G_X]);

		GReal angles[4];
		GPoint2 pts[4];

		pts[0] = A * (p0 - Focus);
		pts[1] = A * (p1 - Focus);
		pts[2] = A * (p2 - Focus);
		pts[3] = A * (p3 - Focus);
		angles[0] = GMath::Atan2(pts[0][G_Y], pts[0][G_X]);
		angles[1] = GMath::Atan2(pts[1][G_Y], pts[1][G_X]);
		angles[2] = GMath::Atan2(pts[2][G_Y], pts[2][G_X]);
		angles[3] = GMath::Atan2(pts[3][G_Y], pts[3][G_X]);

		// sort by angles (ascending order)
		for (GUInt32 i = 0; i < 3; ++i) {
			for (GUInt32 j = i + 1; j < 4; ++j) {
				if (angles[j] < angles[i]) {
					GReal angle = angles[j];
					angles[j] = angles[i];
					angles[i] = angle;
					p0 = pts[j];
					pts[j] = pts[i];
					pts[i] = p0;
				}
			}
		}
		pMin = pts[0];
		pMax = pts[3];
		// it handles the case when the box intersect x-axis at the focus side (behind the focus); in this case
		// we must choose other points couple
		if (angles[0] < 0 && angles[3] > 0) {
			if (Cross(pMin, pMax) < 0) {
				pMax = pts[1];
				pMin = pts[2];
			}
		}
		// anti-transform "external" point into the original coordinate system
		Transpose(A, A);
		pMin = (A * pMin) + Focus;
		pMax = (A * pMax) + Focus;
	}

	// draw the shaded sector
	DrawGLShadedSector(Center, Focus, Radius, tMin, tMax, pMin, pMax, wholeDisk, ColorKeys, Interpolation, SpreadMode);
}

void GOpenGLBoard::DrawSolidStroke(const GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed,
								   const GReal Thickness) {


	GDynArray<GPoint2>::const_iterator it0 = Points.begin(), it1, it2;

	it1 = it0;
	it1++;
	it2 = it1;
	it2++;

	// a single line contour (2 points)
	if (it2 == Points.end())
		DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), *it0, *it1, Thickness);
	else {
		GReal dist = Distance(Points.front(), Points.back());

		if (Points.size() == 3 && (dist < G_EPSILON))
			DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), *it0, *it1, Thickness);
		// contour made at least by 3 (different) points
		else {
			if (!Closed) {
				// draw start cap and line segment
				DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_FALSE, Style.StrokeEndCapStyle(), *it0, *it1, Thickness);

				GDynArray<GPoint2>::const_iterator itEnd = Points.end();
				itEnd--;
				// draw intermediate join-line couples
				while (it2 != itEnd)	{
					DrawGLJoinLine(Style.StrokeJoinStyle(), Style.StrokeMiterLimit(), *it0, *it1, *it2, Thickness);
					it0 = it1;
					it1 = it2;
					it2++;
				}
				// draw last join-line-endcap
				DrawGLJoinLineCap(Style.StrokeJoinStyle(), Style.StrokeMiterLimit(), *it0, *it1, *it2, Thickness, Style.StrokeEndCapStyle());
			}
			else {
				if (dist > G_EPSILON)
					DrawGLJoinLine(Style.StrokeJoinStyle(), Style.StrokeMiterLimit(), Points.back(), *it0, *it1, Thickness);
				else {
					GDynArray<GPoint2>::const_iterator it4 = Points.end();
					it4 -= 2;
					DrawGLJoinLine(Style.StrokeJoinStyle(), Style.StrokeMiterLimit(), *it4, *it0, *it1, Thickness);
				}

				// draw other segments
				while (it2 != Points.end())	{
					DrawGLJoinLine(Style.StrokeJoinStyle(), Style.StrokeMiterLimit(), *it0, *it1, *it2, Thickness);
					it0 = it1;
					it1 = it2;
					it2++;
				}
				// check that first and last point must be geometrically different
				if (dist > G_EPSILON) {
					// line segment and join back to the start
					it2 = Points.begin();
					DrawGLJoinLine(Style.StrokeJoinStyle(), Style.StrokeMiterLimit(), *it0, *it1, *it2, Thickness);
				}
			}
		}
	}
}

void GOpenGLBoard::DrawDashedStroke(const GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed,
									const GReal Thickness) {

	GDynArray<GPoint2>::const_iterator it0 = Points.begin(), it1, it2;

	it1 = it0;
	it1++;
	it2 = it1;
	it2++;

	const GDynArray<GReal>& OfsDashPat = Style.StrokeOffsettedDashPattern();
	GBool empty = Style.StrokeOffsettedDashPatternEmpty();
	GBool startDrawed = !empty;
	GReal PatSum = Style.StrokeDashPatternSum();
	GUInt32 patSize = (GUInt32)OfsDashPat.size();
	GVector2 v, w;
	GReal dashPatVal;
	GPoint2 p0, p1;

	if (PatSum <= G_EPSILON)
		return;

	GUInt32 dashIdx = 0;
	GReal lv, lvOld, lw;

	p0 = *it0;
	v = (*it1) - (*it0);
	lv = v.Normalize();

	if (!Closed) {

		while (lv >= 0 && it1 != Points.end()) {

			// eat a dash entry
			dashPatVal = OfsDashPat[dashIdx++];
			lvOld = lv;
			lv -= dashPatVal;

			if (!empty) {
				// draw just a line
				if (lv >= 0) {
					p1 = p0 + dashPatVal * v;
					DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), p0, p1, Thickness);
					p0 = p1;
				}
				else {
					if (it2 != Points.end()) {

						GCapStyle tmpStyle = Style.StrokeStartCapStyle();
						GReal tmpLen = lvOld;
						GVector2 oldDir = v;

						// draw remaining piece of current segment, then draw also whole "eaten" segments
						do {
							w = (*it2) - (*it1);
							lw = w.Normalize();
							DrawGLJoin(*it1, v, tmpLen, w, 0, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
										tmpStyle, G_BUTT_CAP, Thickness);
							if (tmpLen != 0)
								tmpStyle = G_BUTT_CAP;
							tmpLen = lw;
							lvOld = lv;
							lv += lw;
							it0 = it1;
							it1 = it2;
							it2++;
							oldDir = v;
							v = w;
						} while(lv < 0 && it2 != Points.end());

						// draw remained piece of last segment (so we can take care of end cap style)
						if (it2 != Points.end()) {
							DrawGLJoin(*it0, oldDir, 0, v, -lvOld, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
										tmpStyle, Style.StrokeEndCapStyle(), Thickness);
							p0 = (*it0) - lvOld * v;
						}
						else {
							if (lw < -lvOld) {
								DrawGLJoin(*it0, oldDir, 0, v, lw, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
											G_BUTT_CAP, Style.StrokeEndCapStyle(), Thickness);
								// now exit
								it1++;
							}
							else {
								DrawGLJoin(*it0, oldDir, 0, v, -lvOld, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
											tmpStyle, Style.StrokeEndCapStyle(), Thickness);
								p0 = (*it0) - lvOld * v;
							}
						}
					}
					// last segment
					else {
						DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(),
										p0, *it1, Thickness);
						// now exit
						it1++;
					}
				}
			}
			else {
				if (lv >= 0)
					p0 = p0 + dashPatVal * v;
				else {
					if (it2 != Points.end()) {
						// skip remaining piece of current segment, then skip also whole "eaten" segments
						do {
							w = (*it2) - (*it1);
							lw = w.Normalize();
							lvOld = lv;
							lv += lw;
							it0 = it1;
							it1 = it2;
							it2++;
						} while(lv <= 0 && it2 != Points.end());

						if (lw < -lvOld)
							// now exit
							it1++;
						else {
							p0 = (*it0) - lvOld * w;
							v = w;
						}
					}
					// last segment, now exit
					else
						it1++;
				}
			}
			// next dash pattern entry
			if (dashIdx >= patSize) {
				dashIdx = 0;
				empty = Style.StrokeOffsettedDashPatternEmpty();
			}
			else
				empty = !empty;
		}
	}
	// closed contour
	else {
		GDynArray<GPoint2>::const_iterator itEnd = Points.end();
		GBool recycle = G_TRUE;
		GBool endDrawed;
		GCapStyle tmpStartCapStyle = G_BUTT_CAP;

recycleLabel:
		while (lv >= 0 && it1 != itEnd) {
			// eat a dash entry
			dashPatVal = OfsDashPat[dashIdx++];
			lvOld = lv;
			lv -= dashPatVal;

			if (!empty) {
				// draw just a line
				if (lv >= 0) {
					p1 = p0 + dashPatVal * v;
					DrawGLCapsLine(G_TRUE, tmpStartCapStyle, G_TRUE, Style.StrokeEndCapStyle(),	p0, p1, Thickness);
					p0 = p1;
				}
				else {
					if (it2 != itEnd) {

						GCapStyle tmpStyle = tmpStartCapStyle;
						GReal tmpLen = lvOld;
						GVector2 oldDir = v;
						// draw remaining piece of current segment, then draw also whole "eaten" segments
						do {
							w = (*it2) - (*it1);
							lw = w.Normalize();

							DrawGLJoin(*it1, v, tmpLen, w, 0, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
										tmpStyle, G_BUTT_CAP, Thickness);

							if (tmpLen != 0)
								tmpStyle = G_BUTT_CAP;

							tmpLen = lw;
							lvOld = lv;
							lv += lw;
							it0 = it1;
							it1 = it2;
							it2++;
							oldDir = v;
							v = w;
						} while(lv < 0 && it2 != itEnd);
						// draw remained piece of last segment (so we can take care of end cap style)
						if (it2 != itEnd) {
							DrawGLJoin(*it0, oldDir, 0, v, -lvOld, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
										tmpStyle, Style.StrokeEndCapStyle(), Thickness);
							p0 = (*it0) - lvOld * v;
						}
						else {
							if (lw < -lvOld)
								// now exit
								it1++;
							else {
								DrawGLJoin(*it0, oldDir, 0, v, -lvOld, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
											tmpStyle, Style.StrokeEndCapStyle(), Thickness);
								p0 = (*it0) - lvOld * v;
							}
						}
					}
					// last segment, now exit
					else
						it1++;
				}
			}
			else {
				if (lv >= 0)
					p0 = p0 + dashPatVal * v;
				else {
					if (it2 != itEnd) {
						// skip remaining piece of current segment, then skip also whole "eaten" segments
						do {
							w = (*it2) - (*it1);
							lw = w.Normalize();
							lvOld = lv;
							lv += lw;
							it0 = it1;
							it1 = it2;
							it2++;
						} while(lv <= 0 && it2 != itEnd);
						if (lw < -lvOld)
							// now exit
							it1++;
						else {
							p0 = (*it0) - lvOld * w;
							v = w;
						}
					}
					// last segment, now exit
					else
						it1++;
				}
			}
			tmpStartCapStyle = Style.StrokeStartCapStyle();
			// next dash pattern entry
			if (dashIdx >= patSize) {
				dashIdx = 0;
				empty = Style.StrokeOffsettedDashPatternEmpty();
			}
			else
				empty = !empty;
		}

		it0 = Points.end();
		it0--;
		it1 = it2 = Points.begin();
		it2++;
		endDrawed = empty;

		if (recycle) {

			itEnd = it2;

			w = (*it1) - (*it0);
			lw = w.Normalize();
			p0 = (*it0) - lv * w;

			// take care of last connection
			if (endDrawed) {
				if (lw > -lv)
					DrawGLJoin(*it0, v, dashPatVal + lv, w, -lv, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
								Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), Thickness);
				else {
					if (startDrawed)
						DrawGLJoin(*it0, v, dashPatVal + lv, w, lw, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
									Style.StrokeStartCapStyle(), G_BUTT_CAP, Thickness);
					else
						DrawGLJoin(*it0, v, dashPatVal + lv, w, lw, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
									Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), Thickness);
				}
			}
			lv += lw;
			v = w;
			// now draw the segment that connect last point to the first one (it's the same result that we can
			// obtain if we could push a point into specified Points array)
			recycle = G_FALSE;
			goto recycleLabel;
		}
		else {
			v = (*it1) - (*it0);
			v.Normalize();
			w = (*it2) - (*it1);
			w.Normalize();

			// take care of first connection
			if (endDrawed) {
				if (startDrawed) {
					dashPatVal = OfsDashPat[0];
					//dashPatVal = 0;
					DrawGLJoin(*it1, v, lvOld, w, dashPatVal, Style.StrokeJoinStyle(), Style.StrokeMiterLimit(),
								Style.StrokeStartCapStyle(), Style.StrokeEndCapStyle(), Thickness);
				}
				else
					DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), p0, *it1, Thickness);
			}
			else {
				if (startDrawed) {
					dashPatVal = OfsDashPat[0];
					p1 = (*it1) + dashPatVal * w;
					DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_FALSE, G_BUTT_CAP, *it1, p1, Thickness);				
				}
			}
		}
	}
}

GBool GOpenGLBoard::GeometricRadialGradient(const GDrawStyle& Style, const GBool TestFill) {

	GBool res;

	if (TestFill) {

		if ((Style.FillPaintType() == G_GRADIENT_PAINT_TYPE) &&
			(Style.FillGradient()) &&
			(Style.FillGradient()->Type() == G_RADIAL_GRADIENT) &&
			(!gShaderSupport))
			res = G_TRUE;
		else
		// just until a full shader implementation is not available
		if ((Style.FillPaintType() == G_GRADIENT_PAINT_TYPE) &&
			(Style.FillGradient()) &&
			(Style.FillGradient()->Type() == G_RADIAL_GRADIENT) &&
			(gShaderSupport))
			res = G_TRUE;
		else
			res = G_FALSE;
	}
	else {

		if ((Style.StrokePaintType() == G_GRADIENT_PAINT_TYPE) &&
			(Style.StrokeGradient()) &&
			(Style.StrokeGradient()->Type() == G_RADIAL_GRADIENT) &&
			(!gShaderSupport))
			res = G_TRUE;
		else
		// just until a full shader implementation is not available
		if ((Style.StrokePaintType() == G_GRADIENT_PAINT_TYPE) &&
			(Style.StrokeGradient()) &&
			(Style.StrokeGradient()->Type() == G_RADIAL_GRADIENT) &&
			(gShaderSupport))
			res = G_TRUE;
		else
			res = G_FALSE;
	}

	if (res)
		// geometric radial gradient uses stencil clip
		SetGLClipEnabled(G_CLIP_MODE, G_INTERSECTION_CLIP);
	else
		SetGLClipEnabled(TargetMode(), ClipOperation());

	return res;
}

void GOpenGLBoard::UpdateClipMasksState() {

	if (ClipOperation() == G_REPLACE_CLIP) {
		gClipMasksBoxes.clear();
		gFirstClipMaskReplace = G_TRUE;
	}
	else {
		if (gClipMasksBoxes.empty())
			gFirstClipMaskReplace = G_FALSE;
	}
}

void GOpenGLBoard::DoDrawLine(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1) {

	if (Distance(P0, P1) <= G_EPSILON)
		return;

	// update style
	UpdateStyle(Style);

	if (TargetMode() == G_CLIP_MODE) {

		SetGLClipEnabled(TargetMode(), ClipOperation());
		// draw line segment
		if (Style.StrokeStyle() == G_SOLID_STROKE)
			DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), P0, P1, Style.StrokeThickness());
		else {
			GDynArray<GPoint2> pts(2);
			pts[0] = P0;
			pts[1] = P1;
			DrawDashedStroke(Style, pts, G_FALSE, Style.StrokeThickness());
		}
		// take care of replace operation
		UpdateClipMasksState();
		// calculate bound box of the drawn clip mask
		GAABox2 tmpBox(GPoint2(P0[G_X] - Style.StrokeThickness(), P0[G_Y] - Style.StrokeThickness()),
						GPoint2(P1[G_X] + Style.StrokeThickness(), P1[G_Y] + Style.StrokeThickness()));
		gClipMasksBoxes.push_back(tmpBox);
		return;
	}

	// set stroke style using OpenGL
	UseStrokeStyle(Style);

	GBool geomRadialGradient = GeometricRadialGradient(Style, G_FALSE);

	// draw line segment
	if (Style.StrokeStyle() == G_SOLID_STROKE)
		DrawGLCapsLine(G_TRUE, Style.StrokeStartCapStyle(), G_TRUE, Style.StrokeEndCapStyle(), P0, P1, Style.StrokeThickness());
	else {
		GDynArray<GPoint2> pts(2);
		pts[0] = P0;
		pts[1] = P1;
		DrawDashedStroke(Style, pts, G_FALSE, Style.StrokeThickness());
	}

	// geometric radial gradient uses stencil clip, so we must pop off clip mask
	if (geomRadialGradient) {

		GOpenGLGradientDesc *g = (GOpenGLGradientDesc *)Style.StrokeGradient();

		GAABox2 tmpBox(GPoint2(P0[G_X] - Style.StrokeThickness(), P0[G_Y] - Style.StrokeThickness()),
						GPoint2(P1[G_X] + Style.StrokeThickness(), P1[G_Y] + Style.StrokeThickness()));
		gClipMasksBoxes.push_back(tmpBox);

		SetGLClipEnabled(TargetMode(), ClipOperation());
		DrawShadedSector(g->StartPoint(), g->AuxPoint(), g->Radius(), tmpBox, g->ColorKeys(), g->ColorInterpolation(), g->SpreadMode());
		PopClipMask();
	}
}

void GOpenGLBoard::DoDrawRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner) {

	if (Distance(MinCorner, MaxCorner) <= G_EPSILON)
		return;


	GDynArray<GPoint2> pts(4);
	/*
		p1------p2
		|        |
		|        |
		p0------p3
	*/
	pts[0] = MinCorner;
	pts[1].Set(MinCorner[G_X], MaxCorner[G_Y]);
	pts[2] = MaxCorner;
	pts[3].Set(MaxCorner[G_X], MinCorner[G_Y]);

	// update style
	UpdateStyle(Style);

	if (TargetMode() == G_CLIP_MODE) {

		SetGLClipEnabled(TargetMode(), ClipOperation());

		// draw fill
		if (Style.FillEnabled()) {
			glBegin(GL_POLYGON);
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(pts[0].Data());
				glVertex2dv(pts[1].Data());
				glVertex2dv(pts[2].Data());
				glVertex2dv(pts[3].Data());
			#else
				glVertex2fv(pts[0].Data());
				glVertex2fv(pts[1].Data());
				glVertex2fv(pts[2].Data());
				glVertex2fv(pts[3].Data());
			#endif
			glEnd();
		}
		// draw stroke
		if (Style.StrokeEnabled()) {
			if (Style.StrokeStyle() == G_SOLID_STROKE)
				DrawSolidStroke(Style, pts, G_TRUE, Style.StrokeThickness());
			else
				DrawDashedStroke(Style, pts, G_TRUE, Style.StrokeThickness());
		}
		// take care of replace operation
		UpdateClipMasksState();
		// calculate and push bounding box
		GReal expandLength = 0;
		if (Style.StrokeEnabled())
			expandLength = GMath::Max(Style.StrokeThickness(), Style.StrokeThickness() * Style.StrokeMiterLimit());

		GAABox2 tmpBox(GPoint2(MinCorner[G_X] - expandLength, MinCorner[G_Y] - expandLength),
						GPoint2(MaxCorner[G_X] + expandLength, MaxCorner[G_Y] + expandLength));
		gClipMasksBoxes.push_back(tmpBox);
		return;
	}

	// set fill style using OpenGL
	UseFillStyle(Style);

	if (Style.FillEnabled()) {

		GBool geomRadialGradient = GeometricRadialGradient(Style, G_TRUE);

		// draw fill
		glBegin(GL_POLYGON);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(pts[0].Data());
			glVertex2dv(pts[1].Data());
			glVertex2dv(pts[2].Data());
			glVertex2dv(pts[3].Data());
		#else
			glVertex2fv(pts[0].Data());
			glVertex2fv(pts[1].Data());
			glVertex2fv(pts[2].Data());
			glVertex2fv(pts[3].Data());
		#endif
		glEnd();

		// geometric radial gradient uses stencil clip, so we must pop off clip mask
		if (geomRadialGradient) {

			GOpenGLGradientDesc *g = (GOpenGLGradientDesc *)Style.FillGradient();

			GAABox2 tmpBox(GPoint2(MinCorner[G_X] - Style.StrokeThickness(), MinCorner[G_Y] - Style.StrokeThickness()),
							GPoint2(MaxCorner[G_X] + Style.StrokeThickness(), MaxCorner[G_Y] + Style.StrokeThickness()));
			gClipMasksBoxes.push_back(tmpBox);

			SetGLClipEnabled(TargetMode(), ClipOperation());
			DrawShadedSector(g->StartPoint(), g->AuxPoint(), g->Radius(), tmpBox, g->ColorKeys(), g->ColorInterpolation(), g->SpreadMode());
			PopClipMask();
		}
	}


	// draw stroke
	UseStrokeStyle(Style);

	if (Style.StrokeEnabled()) {
		
		GBool geomRadialGradient = GeometricRadialGradient(Style, G_FALSE);

		// solid stroke
		if (Style.StrokeStyle() == G_SOLID_STROKE)
			DrawSolidStroke(Style, pts, G_TRUE, Style.StrokeThickness());
		else
			DrawDashedStroke(Style, pts, G_TRUE, Style.StrokeThickness());

		// geometric radial gradient uses stencil clip, so we must pop off clip mask
		if (geomRadialGradient) {

			GOpenGLGradientDesc *g = (GOpenGLGradientDesc *)Style.StrokeGradient();

			GAABox2 tmpBox(GPoint2(MinCorner[G_X] - Style.StrokeThickness(), MinCorner[G_Y] - Style.StrokeThickness()),
							GPoint2(MaxCorner[G_X] + Style.StrokeThickness(), MaxCorner[G_Y] + Style.StrokeThickness()));
			gClipMasksBoxes.push_back(tmpBox);

			SetGLClipEnabled(TargetMode(), ClipOperation());
			DrawShadedSector(g->StartPoint(), g->AuxPoint(), g->Radius(), tmpBox, g->ColorKeys(), g->ColorInterpolation(), g->SpreadMode());
			PopClipMask();
		}
	}

}

void GOpenGLBoard::DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2) {

	if (Style.StrokeWidth() && P0[0] && P1[0] && P2[0]) {
	}
}

void GOpenGLBoard::DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3) {

	if (Style.StrokeWidth() && P0[0] && P1[0] && P2[0] && P3[0]) {
	}
}

void GOpenGLBoard::DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength,
									const GReal YSemiAxisLength, const GReal OffsetRotation,
									const GReal StartAngle, const GReal EndAngle, const GBool CCW) {

	if (Style.StrokeWidth() && Center[0] && XSemiAxisLength && YSemiAxisLength && OffsetRotation && StartAngle && EndAngle && CCW) {
	}
}

void GOpenGLBoard::DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
									const GReal OffsetRotation, const GBool LargeArc, const GBool CCW) {
	if (Style.StrokeWidth() && P0[0] && P1[0] && XSemiAxisLength && YSemiAxisLength && OffsetRotation && LargeArc && CCW) {
	}
}

void GOpenGLBoard::DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed) {

	// empty contours, or 1 point contour, lets exit immediately
	if (Points.size() < 2)
		return;

	SetGLClipEnabled(TargetMode(), ClipOperation());

	// update and use style
	UpdateStyle(Style);
	UseStrokeStyle(Style);

	// draw fill
	if (Style.FillEnabled()) {
	}

	// draw stroke
	if (Style.StrokeEnabled()) {
		// solid stroke
		if (Style.StrokeStyle() == G_SOLID_STROKE)
			DrawSolidStroke(Style, Points, Closed, Style.StrokeThickness());
		else {
			if (Points.size() <= 2)
				DrawDashedStroke(Style, Points, G_FALSE, Style.StrokeThickness());
			else
				DrawDashedStroke(Style, Points, Closed, Style.StrokeThickness());
		}
	}

	if (TargetMode() == G_CLIP_MODE) {

		if (ClipOperation() == G_REPLACE_CLIP) {
			gClipMasksBoxes.clear();
			gFirstClipMaskReplace = G_TRUE;
		}
		else {
			if (gClipMasksBoxes.empty())
				gFirstClipMaskReplace = G_FALSE;
		}

		GReal expandLength = 0;
		if (Style.StrokeEnabled())
			expandLength = GMath::Max(Style.StrokeThickness(), Style.StrokeThickness() * Style.StrokeMiterLimit());

		GAABox2 tmpBox(Points);
		GPoint2 newMin = tmpBox.Min();
		GPoint2 newMax = tmpBox.Max();
		newMin[G_X] -= expandLength;
		newMin[G_Y] -= expandLength;
		newMax[G_X] += expandLength;
		newMax[G_Y] += expandLength;
		tmpBox.SetMinMax(newMin, newMax);
		gClipMasksBoxes.push_back(tmpBox);
	}
}

};	// end namespace Amanith
