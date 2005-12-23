/****************************************************************************
** $file: amanith/src/rendering/gopenglstyles.cpp   0.2.0.0   edited Dec, 12 2005
**
** OpenGL based draw board styles functions implementation.
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
	\file gopenglstyles.cpp
	\brief OpenGL based draw board styles functions implementation file.
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

void GOpenGLGradientDesc::GenerateTexture1D(const GInt32 Size, GPixelMap& Bitmap, const GBool ReflectKeys) {

	Bitmap.Create(Size, 1, G_A8R8G8B8);
	GUChar8 *pixels = Bitmap.Pixels();

	// hermite color interpolation
	if (ColorInterpolation() == G_HERMITE_COLOR_INTERPOLATION) {

		const GDynArray<GKeyValue>& keys = ColorKeys();
		GInt32 i, j = (GInt32)keys.size(), pixelOfs;
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

		// reflect keys, is specified
		if (ReflectKeys) {

			GReal maxTime = redKeys.back().Parameter;
			for (i = j - 2; i >= 0; i--) {

				redKeys.push_back(redKeys[i]);
				redKeys.back().Parameter = ((GReal)2 * maxTime - redKeys.back().Parameter);
				greenKeys.push_back(greenKeys[i]);
				greenKeys.back().Parameter = ((GReal)2 * maxTime - greenKeys.back().Parameter);
				blueKeys.push_back(blueKeys[i]);
				blueKeys.back().Parameter = ((GReal)2 * maxTime - blueKeys.back().Parameter);
				alphaKeys.push_back(alphaKeys[i]);
				alphaKeys.back().Parameter = ((GReal)2 * maxTime - alphaKeys.back().Parameter);
			}
		}

		redCurve.SetKeys(redKeys);
		greenCurve.SetKeys(greenKeys);
		blueCurve.SetKeys(blueKeys);
		alphaCurve.SetKeys(alphaKeys);

		step = redCurve.Domain().Length() / (GReal)(Size - 1);
		u = redCurve.DomainStart();
		pixelOfs = 0;
		for (i = 0; i < Size; i++) {
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
		GInt32 i, j = (GInt32)keys.size(), pixelOfs;
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

		// reflect keys, is specified
		if (ReflectKeys) {

			GReal maxTime = redKeys.back().Parameter;
		
			for (i = j - 2; i >= 0; i--) {

				redKeys.push_back(redKeys[i]);
				redKeys.back().Parameter = ((GReal)2 * maxTime - redKeys.back().Parameter);
				greenKeys.push_back(greenKeys[i]);
				greenKeys.back().Parameter = ((GReal)2 * maxTime - greenKeys.back().Parameter);
				blueKeys.push_back(blueKeys[i]);
				blueKeys.back().Parameter = ((GReal)2 * maxTime - blueKeys.back().Parameter);
				alphaKeys.push_back(alphaKeys[i]);
				alphaKeys.back().Parameter = ((GReal)2 * maxTime - alphaKeys.back().Parameter);
			}
		}

		redCurve.SetKeys(redKeys);
		greenCurve.SetKeys(greenKeys);
		blueCurve.SetKeys(blueKeys);
		alphaCurve.SetKeys(alphaKeys);

		step = redCurve.Domain().Length() / (GReal)(Size - 1);
		u = redCurve.DomainStart();
		pixelOfs = 0;
		for (i = 0; i < Size; i++) {
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
		GInt32 i, j = (GInt32)keys.size(), pixelOfs;
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

		//if (SpreadMode() == G_REFLECT_COLOR_RAMP_SPREAD) {
		// reflect keys, is specified
		if (ReflectKeys) {

			GReal maxTime = redKeys.back().TimePosition();
		
			for (i = j - 2; i >= 0; i--) {

				redKeys.push_back(redKeys[i]);
				redKeys.back().SetTimePosition((GReal)2 * maxTime - redKeys.back().TimePosition());
				greenKeys.push_back(greenKeys[i]);
				greenKeys.back().SetTimePosition((GReal)2 * maxTime - greenKeys.back().TimePosition());
				blueKeys.push_back(blueKeys[i]);
				blueKeys.back().SetTimePosition((GReal)2 * maxTime - blueKeys.back().TimePosition());
				alphaKeys.push_back(alphaKeys[i]);
				alphaKeys.back().SetTimePosition((GReal)2 * maxTime - alphaKeys.back().TimePosition());
			}
		}

		redCurve.SetKeys(redKeys);
		greenCurve.SetKeys(greenKeys);
		blueCurve.SetKeys(blueKeys);
		alphaCurve.SetKeys(alphaKeys);

		step = redCurve.Domain().Length() / (GReal)(Size - 1);
		u = redCurve.Domain().Start();
		pixelOfs = 0;
		for (i = 0; i < Size; i++) {
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

		// pad spread mode		
		if (SpreadMode() == G_PAD_COLOR_RAMP_SPREAD) {
			GVector4 v = keys[j - 1].Vect4Value();
			r8 = (GUChar8)(v[G_X] * (GReal)255);
			g8 = (GUChar8)(v[G_Y] * (GReal)255);
			b8 = (GUChar8)(v[G_Z] * (GReal)255);
			a8 = (GUChar8)(v[G_W] * (GReal)255);
			pixels[pixelOfs - 4] = r8;
			pixels[pixelOfs - 3] = g8;
			pixels[pixelOfs - 2] = b8;
			pixels[pixelOfs - 1] = a8;
		}
	}
}

void GOpenGLGradientDesc::UpdateOpenGLTextureLinRad(const GRenderingQuality Quality, const GUInt32 MaxTextureSize,
													const GBool MirroredRepeatSupported) {

	GInt32 size = 512;

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
	if (size > (GInt32)MaxTextureSize)
		size = (GInt32)MaxTextureSize;

	// create pixelmap
	GPixelMap pixMap;

	if (SpreadMode() == G_REFLECT_COLOR_RAMP_SPREAD) {

		if (ColorInterpolation() == G_CONSTANT_COLOR_INTERPOLATION || !MirroredRepeatSupported)
			GenerateTexture1D(size, pixMap, G_TRUE);
		else
			GenerateTexture1D(size, pixMap, G_FALSE);
	}
	else
		GenerateTexture1D(size, pixMap, G_FALSE);

	GUChar8 *pixels = pixMap.Pixels();

	// generate OpenGL texture
	if (gGradientTexture == 0) {
		glGenTextures(1, &gGradientTexture);
		G_ASSERT(gGradientTexture > 0);
	}
	glBindTexture(GL_TEXTURE_1D, gGradientTexture);
	SetGLGradientQuality(Quality);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, (GLsizei)size, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pixels);
}

void GOpenGLGradientDesc::UpdateOpenGLTextureCon(const GRenderingQuality Quality, const GUInt32 MaxTextureSize,
												const GInt32 Atan2LookupTableSize, const GFloat *gAtan2LookupTable) {


	GInt32 size = 512, i, j;

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
	if (size > (GInt32)MaxTextureSize)
		size = (GInt32)MaxTextureSize;

	// create pixelmap
	GPixelMap pixMap, texture;

	GenerateTexture1D(size, pixMap, G_FALSE);
	GUInt32 *pixelsSrc = (GUInt32 *)pixMap.Pixels();

	texture.Create(Atan2LookupTableSize, Atan2LookupTableSize, G_A8R8G8B8);
	GUInt32 *pixelsDst = (GUInt32 *)texture.Pixels();

	// atant2 table is square
	j = Atan2LookupTableSize * Atan2LookupTableSize;
	for (i = 0; i < j; i++) {

		GFloat atan2Val = gAtan2LookupTable[i];
		G_ASSERT(atan2Val >= 0 && atan2Val <= 1);
		GInt32 u = (GInt32)(atan2Val * size);
		G_ASSERT(u >= 0 && u < size);

		GUInt32 argb = pixelsSrc[u];
		pixelsDst[i] = argb;
	}

	// generate OpenGL texture
	if (gGradientTexture == 0) {
		glGenTextures(1, &gGradientTexture);
		G_ASSERT(gGradientTexture > 0);
	}

	glBindTexture(GL_TEXTURE_2D, gGradientTexture);
	// set texture min/mag filters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)Atan2LookupTableSize, (GLsizei)Atan2LookupTableSize,
				0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pixelsDst);
}

void GOpenGLGradientDesc::UpdateHermiteTangents() {

	// hermite color tangents are used only by conical gradients
	if (ColorInterpolation() != G_HERMITE_COLOR_INTERPOLATION || Type() != G_CONICAL_GRADIENT)
		return;


	const GDynArray<GKeyValue>& keys = ColorKeys();
	GUInt32 i, j = (GUInt32)keys.size();
	GHermiteCurve1D redCurve, greenCurve, blueCurve, alphaCurve;
	GDynArray<GHermiteKey1D> redKeys, greenKeys, blueKeys, alphaKeys;
	GHermiteKey1D redKey, greenKey, blueKey, alphaKey;

	gInTangents.resize(j);
	gOutTangents.resize(j);

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

	for (i = 0; i < j; i++) {
		redCurve.Key(i, redKey);
		greenCurve.Key(i, greenKey);
		blueCurve.Key(i, blueKey);
		alphaCurve.Key(i, alphaKey);
		gInTangents[i] = GVector4(redKey.InTangent, greenKey.InTangent, blueKey.InTangent, alphaKey.InTangent);
		gOutTangents[i] = GVector4(redKey.OutTangent, greenKey.OutTangent, blueKey.OutTangent, alphaKey.OutTangent);
	}
}

// set color keys
void GOpenGLGradientDesc::SetColorKeys(const GDynArray<GKeyValue>& ColorKeys) {

	GDynArray<GKeyValue>::const_iterator it = ColorKeys.begin();

	GDynArray<GKeyValue> tmpKeys;
	GKeyValue tmpKey;

	tmpKeys.reserve(ColorKeys.size());

	// clamp all color components in the range [0; 1]
	for (; it != ColorKeys.end(); ++it) {
		if (it->KeyType() == G_VECTOR3_KEY) {
			GVector3 v = it->Vect3Value();
			GReal t = it->TimePosition();
			v[G_X] = GMath::Clamp(v[G_X], (GReal)0, (GReal)1);
			v[G_Y] = GMath::Clamp(v[G_Y], (GReal)0, (GReal)1);
			v[G_Z] = GMath::Clamp(v[G_Z], (GReal)0, (GReal)1);
			tmpKey.SetValue(GVector4(v[G_X], v[G_Y], v[G_Z], (GReal)1.0));
			tmpKey.SetTimePosition(t);
			tmpKeys.push_back(tmpKey);
		}
		else
		if (it->KeyType() == G_VECTOR4_KEY) {
			GVector4 v = it->Vect4Value();
			GReal t = it->TimePosition();
			v[G_X] = GMath::Clamp(v[G_X], (GReal)0, (GReal)1);
			v[G_Y] = GMath::Clamp(v[G_Y], (GReal)0, (GReal)1);
			v[G_Z] = GMath::Clamp(v[G_Z], (GReal)0, (GReal)1);
			v[G_W] = GMath::Clamp(v[G_W], (GReal)0, (GReal)1);
			if (v[G_W] < (GReal)1)
				gAlphaKeys = G_TRUE;
			tmpKey.SetValue(v);
			tmpKey.SetTimePosition(t);
			tmpKeys.push_back(tmpKey);
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
	gPatternMirroredTexture = 0;
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

	GInt32 w = (GInt32)GOpenglExt::PowerOfTwo((GUInt32)Image->Width());
	GInt32 h = (GInt32)GOpenglExt::PowerOfTwo((GUInt32)Image->Height());

	GPixelMap tmpImage, mirroredImage;
	const GPixelMap *img = Image;

	// resize picture to the closest power of two dimensions
	if (w != Image->Width() || h != Image->Height()) {

		GInt32 wPrev = (w >> 1);
		GInt32 hPrev = (h >> 1);

		if (GMath::Abs(w - Image->Width()) > GMath::Abs(wPrev - Image->Width()))
			w = wPrev;

		if (GMath::Abs(h - Image->Height()) > GMath::Abs(hPrev - Image->Height()))
			h = hPrev;

		if (w > (GInt32)gMaxTextureSize)
			w = (GInt32)gMaxTextureSize;
		if (h > (GInt32)gMaxTextureSize)
			h = (GInt32)gMaxTextureSize;

		Image->Resize(w, h, tmpImage, G_RESIZE_CATMULLROM);
		img = (const GPixelMap *)&tmpImage;
	}

	// if mirrored repeat is not supported we must create a mirrored image by hand
	if (!gMirroredRepeatSupport) {
		img->ResizeCanvasMirror(0, img->Height(), 0, img->Width(), mirroredImage);
		if (mirroredImage.Width() > (GInt32)gMaxTextureSize || mirroredImage.Height() > (GInt32)gMaxTextureSize) {
			if (!mirroredImage.IsTrueColor())
				mirroredImage.SetPixelFormat(G_A8R8G8B8);
			mirroredImage.Resize(img->Width(), img->Height(), G_RESIZE_CATMULLROM);
		}
	}

	if (img->IsTrueColor() && (Quality == G_LOW_IMAGE_QUALITY || Quality == G_NORMAL_IMAGE_QUALITY)) {
		// generate "normal" OpenGL texture
		if (gPatternTexture == 0)
			glGenTextures(1, &gPatternTexture);
		glBindTexture(GL_TEXTURE_2D, gPatternTexture);
		SetGLImageQuality(Quality);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)img->Width(), (GLsizei)img->Height(),
					 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)img->Pixels());
		// generate mirrored OpenGL texture
		if (!gMirroredRepeatSupport) {
			if (gPatternMirroredTexture == 0)
				glGenTextures(1, &gPatternMirroredTexture);
			glBindTexture(GL_TEXTURE_2D, gPatternMirroredTexture);
			SetGLImageQuality(Quality);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)mirroredImage.Width(), (GLsizei)mirroredImage.Height(),
				0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)mirroredImage.Pixels());
		}
		return;
	}

	// copy image
	tmpImage.CopyFrom(*img);

	if (!tmpImage.IsTrueColor())
		tmpImage.SetPixelFormat(G_A8R8G8B8);

	// generate texture handle
	if (gPatternTexture == 0) {
		glGenTextures(1, &gPatternTexture);
		G_ASSERT(gPatternTexture > 0);
	}
	glBindTexture(GL_TEXTURE_2D, gPatternTexture);
	SetGLImageQuality(Quality);

	// for low and normal quality levels, we can upload texture directly
	if (Quality == G_LOW_IMAGE_QUALITY || Quality == G_NORMAL_IMAGE_QUALITY) {

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)tmpImage.Width(), (GLsizei)tmpImage.Height(),
					 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)tmpImage.Pixels());
		return;
	}

	// generate mipmaps
	GInt32 size = GMath::Max(tmpImage.Width(), tmpImage.Height());
	GInt32 level = 0;

	do {
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, (GLsizei)tmpImage.Width(), (GLsizei)tmpImage.Height(),
					 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)tmpImage.Pixels());

		if (size > 1) {
			GInt32 newW = tmpImage.Width() / 2;
			GInt32 newH = tmpImage.Height() / 2;

			if (newW == 0)
				newW = 1;
			if (newH == 0)
				newH = 1;

			tmpImage.Resize((GUInt32)newW, (GUInt32)newH, G_RESIZE_QUADRATIC);
		}

		size /= 2;
		level++;
	} while(size >= 1);


	// now repeat all steps for mirrored texture, if needed
	if (gMirroredRepeatSupport)
		return;

	if (!mirroredImage.IsTrueColor())
		mirroredImage.SetPixelFormat(G_A8R8G8B8);

	// generate texture handle for mirrored image
	if (gPatternMirroredTexture == 0) {
		glGenTextures(1, &gPatternMirroredTexture);
		G_ASSERT(gPatternTexture > 0);
	}
	glBindTexture(GL_TEXTURE_2D, gPatternMirroredTexture);
	SetGLImageQuality(Quality);

	// for low and normal quality levels, we can upload texture directly
	if (Quality == G_LOW_IMAGE_QUALITY || Quality == G_NORMAL_IMAGE_QUALITY) {

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)mirroredImage.Width(), (GLsizei)mirroredImage.Height(),
			0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)mirroredImage.Pixels());
		return;
	}

	// generate mipmaps
	size = GMath::Max(mirroredImage.Width(), mirroredImage.Height());
	level = 0;

	do {
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, (GLsizei)mirroredImage.Width(), (GLsizei)mirroredImage.Height(),
			0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)mirroredImage.Pixels());

		if (size > 1) {
			GInt32 newW = mirroredImage.Width() / 2;
			GInt32 newH = mirroredImage.Height() / 2;
			if (newW == 0)
				newW = 1;
			if (newH == 0)
				newH = 1;
			mirroredImage.Resize((GUInt32)newW, (GUInt32)newH, G_RESIZE_QUADRATIC);
		}
		size /= 2;
		level++;
	} while(size >= 1);
}

// *********************************************************************
//                          GOpenGLDrawStyle
// *********************************************************************
GOpenGLDrawStyle::GOpenGLDrawStyle() : GDrawStyle() {

	gMiterMulThickness = StrokeMiterLimit() * StrokeThickness();
}

void GOpenGLDrawStyle::SetStrokeMiterLimit(const GReal MiterLimit) {

	GDrawStyle::SetStrokeMiterLimit(MiterLimit);
	gMiterMulThickness = StrokeMiterLimit() * StrokeThickness();
}

void GOpenGLDrawStyle::SetStrokeWidth(const GReal Width) {

	GDrawStyle::SetStrokeWidth(Width);
	gMiterMulThickness = StrokeMiterLimit() * StrokeThickness();
}


void GOpenGLDrawStyle::SetStrokeColor(const GVectBase<GReal, 4>& Color) {

	GVector4 c(Color);

	c[G_X] = GMath::Clamp(c[G_X], (GReal)0, (GReal)1);
	c[G_Y] = GMath::Clamp(c[G_Y], (GReal)0, (GReal)1);
	c[G_Z] = GMath::Clamp(c[G_Z], (GReal)0, (GReal)1);
	c[G_W] = GMath::Clamp(c[G_W], (GReal)0, (GReal)1);

	GDrawStyle::SetStrokeColor(c);
}

void GOpenGLDrawStyle::SetFillColor(const GVectBase<GReal, 4>& Color) {

	GVector4 c(Color);

	c[G_X] = GMath::Clamp(c[G_X], (GReal)0, (GReal)1);
	c[G_Y] = GMath::Clamp(c[G_Y], (GReal)0, (GReal)1);
	c[G_Z] = GMath::Clamp(c[G_Z], (GReal)0, (GReal)1);
	c[G_W] = GMath::Clamp(c[G_W], (GReal)0, (GReal)1);

	GDrawStyle::SetFillColor(c);
}

// *********************************************************************
//                             GOpenGLBoard
// *********************************************************************

// create a draw style
GDrawStyle *GOpenGLBoard::CreateDrawStyle() const {

	GOpenGLDrawStyle *ds = new GOpenGLDrawStyle();

	GReal dev = GMath::Clamp((GReal)0.5, G_EPSILON, ds->StrokeThickness() - (G_EPSILON * ds->StrokeThickness()));
	ds->gRoundJoinAuxCoef = (GReal)1 / ((GReal)2 * GMath::Acos((GReal)1 - dev / ds->StrokeThickness()));

	return ds;
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
		g->SetRadius(Radius);
		g->SetAuxPoint(Focus);
		g->SetColorKeys(ColorKeys);
		g->SetColorInterpolation(Interpolation);
		g->SetSpreadMode(SpreadMode);
		g->SetMatrix(Matrix);
		gGradients.push_back(g);
	}
	return g;
}

GGradientDesc *GOpenGLBoard::CreateConicalGradient(const GPoint2& Center, const GPoint2& Target,
												   const GDynArray<GKeyValue>& ColorKeys,
												   const GColorRampInterpolation Interpolation,
												   const GMatrix33& Matrix) {

	GOpenGLGradientDesc *g = new(std::nothrow) GOpenGLGradientDesc();
	if (g) {
		g->SetType(G_CONICAL_GRADIENT);
		g->SetStartPoint(Center);
		g->SetAuxPoint(Target);
		g->SetColorKeys(ColorKeys);
		g->SetColorInterpolation(Interpolation);
		g->SetMatrix(Matrix);
		gGradients.push_back(g);
	}
	return g;
}

GPatternDesc *GOpenGLBoard::CreatePattern(const GPixelMap *Image, const GImageQuality Quality,
										  const GTilingMode TilingMode,
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
		// write internal data, useful to make decisions in independent way
		p->gMaxTextureSize = gExtManager->MaxTextureSize();
		p->gMirroredRepeatSupport = this->gMirroredRepeatSupport;
		p->SetImage(Image, Quality);
		gPatterns.push_back(p);
	}
	return p;
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

void GOpenGLBoard::SetGLModelViewMatrix(const GMatrix33& Matrix) {

	// convert an affine 3x3 matrix to its correspondent 4x4 matrix
	GMatrix44 m;

	m[0][0] = Matrix[0][0];
	m[0][1] = Matrix[0][1];
	m[1][0] = Matrix[1][0];
	m[1][1] = Matrix[1][1];
	m[0][3] = Matrix[0][2];
	m[1][3] = Matrix[1][2];

	m[2][2] = 1;
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

void GOpenGLBoard::UpdateStyle(GOpenGLDrawStyle& Style) {

	if (Style.StrokeEnabled()) {

		if (Style.StrokePaintType() == G_GRADIENT_PAINT_TYPE && Style.StrokeGradient()) {

			GOpenGLGradientDesc *g = (GOpenGLGradientDesc *)Style.StrokeGradient();
			if (g->Modified()) {
				if ((g->Type() == G_LINEAR_GRADIENT) ||
					(g->Type() == G_RADIAL_GRADIENT && gFragmentProgramsSupport)) {

					if (g->ColorKeysModified() || g->ColorInterpolationModified() || g->SpreadModeModified())
						// for linear gradients (or radial ones with fragment programs support) we must update texture
						g->UpdateOpenGLTextureLinRad(RenderingQuality(), MaxImageWidth(), gMirroredRepeatSupport);
				}
				else
				if (g->Type() == G_CONICAL_GRADIENT) {

					if (g->ColorKeysModified() || g->ColorInterpolationModified()) {
						// for conical gradients we must update color tangents
						g->UpdateHermiteTangents();
						// for shader version we must generate lookup texture
						if (gFragmentProgramsSupport)
							g->UpdateOpenGLTextureCon(RenderingQuality(), MaxImageWidth(),
													  gAtan2LookupTableSize, gAtan2LookupTable);
					}
				}
			}
			g->gModified = 0;
		}

		GOpenGLPatternDesc *p = (GOpenGLPatternDesc *)Style.StrokePattern();
		// pattern image/texture is rebuilt on the fly with SetImage method
		if (p)
			p->gModified = 0;

		GReal dev = GMath::Clamp(gFlateness, G_EPSILON, Style.StrokeThickness() - (G_EPSILON * Style.StrokeThickness()));
		Style.gRoundJoinAuxCoef = (GReal)1 / ((GReal)2 * GMath::Acos((GReal)1 - dev / Style.StrokeThickness()));
	}

	if (Style.FillEnabled()) {

		if (Style.FillPaintType() == G_GRADIENT_PAINT_TYPE && Style.FillGradient()) {

			GOpenGLGradientDesc *g = (GOpenGLGradientDesc *)Style.FillGradient();
			if (g->Modified()) {
				if ((g->Type() == G_LINEAR_GRADIENT) ||
					(g->Type() == G_RADIAL_GRADIENT && gFragmentProgramsSupport)) {

					if (g->ColorKeysModified() || g->ColorInterpolationModified() || g->SpreadModeModified())
						// for linear gradients (or radial ones with fragment programs support) we must update texture
						g->UpdateOpenGLTextureLinRad(RenderingQuality(), MaxImageWidth(), gMirroredRepeatSupport);
				}
				else
				if (g->Type() == G_CONICAL_GRADIENT) {

					if (g->ColorKeysModified() || g->ColorInterpolationModified()) {
						// for conical gradients we must update color tangents
						g->UpdateHermiteTangents();
						// for shader version we must generate lookup texture
						if (gFragmentProgramsSupport)
							g->UpdateOpenGLTextureCon(this->RenderingQuality(), this->MaxImageWidth(),
													  gAtan2LookupTableSize, gAtan2LookupTable);
					}
				}
			}
			g->gModified = 0;
		}

		GOpenGLPatternDesc *p = (GOpenGLPatternDesc *)Style.FillPattern();
		// pattern image/texture is rebuilt on the fly with SetImage method
		if (p)
			p->gModified = 0;
	}
}

GBool GOpenGLBoard::UseStyle(const GPaintType PaintType, const GVector4& Color,
							 const GOpenGLGradientDesc *Gradient, const GOpenGLPatternDesc *Pattern,
							 const GMatrix33& ModelView, const GMatrix33& InverseModelView, const GBool UseFill) {

	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	if (TargetMode() == G_CLIP_MODE)
		return G_FALSE;

	glMatrixMode(GL_MODELVIEW);
	SetGLModelViewMatrix(ModelView);

	GBool useDepthBuffer = G_FALSE;
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// color paint
	if ((PaintType == G_COLOR_PAINT_TYPE) || 
		(PaintType == G_GRADIENT_PAINT_TYPE && !Gradient) ||
		(PaintType == G_GRADIENT_PAINT_TYPE && Gradient && Gradient->ColorKeys().size() < 2) ||
		(PaintType == G_PATTERN_PAINT_TYPE && !Pattern)) {

		GVector4 col = Color;
		if (PaintType == G_GRADIENT_PAINT_TYPE && Gradient && Gradient->ColorKeys().size() == 1) {
			GKeyValue key = Gradient->ColorKeys().front();
			col = key.Vect4Value();
		}

		if (col[G_W] < (GReal)1) {
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glEnable(GL_BLEND);
			if (!UseFill)
				useDepthBuffer = G_TRUE;
		}
		else
			glDisable(GL_BLEND);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);

		SetGLColor(col);
	}
	else
	// gradient paint
	if (PaintType == G_GRADIENT_PAINT_TYPE) {

		// linear gradient
		if (Gradient->Type() == G_LINEAR_GRADIENT) {

			GVector4 col(Color);
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
						// this fix is due to some old hardware
						col[G_W] = (GReal)1;
						SetGLColor(col);
					}
				}
				// we have only stroke opacity
				else {
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
					SetGLColor(col);
				}
				glEnable(GL_BLEND);
				if (!UseFill)
					useDepthBuffer = G_TRUE;
			}
			else {
				glDisable(GL_BLEND);
				// this fix is due to some old hardware
				SetGLColor(col);
			}

			// select gradient texture for RGB channels
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);

			GMatrix33 genMatrix = InverseModelView * Gradient->Matrix();
			// affine transform end points
			GPoint2 sPoint = genMatrix * Gradient->StartPoint();
			GPoint2 ePoint = genMatrix * Gradient->AuxPoint();

			// calculate direction
			GVector2 n = ePoint - sPoint;
			n /= n.LengthSquared();
			if (Gradient->SpreadMode() == G_REFLECT_COLOR_RAMP_SPREAD) {
				if (Gradient->ColorInterpolation() == G_CONSTANT_COLOR_INTERPOLATION || !gMirroredRepeatSupport)
					n /= 2;
			}
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
					/*if (Gradient->ColorInterpolation() != G_CONSTANT_COLOR_INTERPOLATION) {
						if (gMirroredRepeatSupport)
							glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
						else
							glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					}
					else
						glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);*/
					if (Gradient->ColorInterpolation() == G_CONSTANT_COLOR_INTERPOLATION || !gMirroredRepeatSupport)
						glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					else
						glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
					break;
			}
		}
		// radial/conical gradient
		else {
			// geometrical version
			if (!gFragmentProgramsSupport) {
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);

				GVector4 col(Color);

				// set alpha pipeline, according to stroke and color keys opacity
				if (Gradient->gAlphaKeys || col[G_W] < (GLfloat)1) {

					if (Gradient->gAlphaKeys) {
						// we have both stroke and color keys opacity
						if (col[G_W] < (GLfloat)1) {

							GVect<GLfloat, 4> colf;
							colf[G_X] = colf[G_Y] = colf[G_Z] = (GLfloat)1;
							colf[G_W] = (GLfloat)Color[G_W];

							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
							glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
						}
						// we have only color keys opacity
						else {
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
							glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
						}
					}
					// we have only stroke opacity
					else {
						GVect<GLfloat, 4> colf;
						colf[G_X] = colf[G_Y] = colf[G_Z] = (GLfloat)1;
						colf[G_W] = (GLfloat)Color[G_W];

						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
					}
					glEnable(GL_BLEND);
				}
				else
					glDisable(GL_BLEND);
				// for radial gradients we use always depth buffer
				useDepthBuffer = G_TRUE;
			}
			// fragment program version (radial and conical)
			else {
				// for stroking, we must use zbuffer as mask also for conical and radial gradients (if they
				// are transparent)
				if ((Gradient->gAlphaKeys || Color[G_W] < (GReal)1) && !UseFill)
					useDepthBuffer = G_TRUE;

				// enable fragment program
				glEnable(GL_FRAGMENT_PROGRAM_ARB);

				// radial gradient
				if (Gradient->Type() == G_RADIAL_GRADIENT) {

					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gRadGradGLProgram);

					GPoint2 pc = Gradient->StartPoint();
					GPoint2 pf = Gradient->AuxPoint();
					GReal newRadius = Gradient->Radius();

					// if focus is outside gradient circle, we must reset it
					if (Distance(pf, pc) > newRadius)
						pf = pc;

					GVector2 fc = (pf - pc);

					float focuscenter[4] = { (float)fc[G_X], (float)fc[G_Y], 0.0f, 0.0f };

					// this fix a STRANGE bug on some GeForce boards (we have found it on a 7800GT) where
					// an x-aligned focus-center makes the fragment program run in an anomalous manner.
					if (GMath::Abs(focuscenter[0]) <= 1.1920928955078125e-07f)
						focuscenter[0] = (2.0f * 1.1920928955078125e-07f);

					float qCoef[4] = { (float)(fc.LengthSquared() - GMath::Sqr(newRadius)), 0.0f, 0.0f, 0.0f };

					GMatrix33 m = Gradient->InverseMatrix() * PhysicalToLogicalMatrix();
					m[0][2] -= (float)pf[G_X];
					m[1][2] -= (float)pf[G_Y];

					float mrs[4] = { (float)m[0][0], (float)m[0][1], (float)m[1][0], (float)m[1][1]};
					float mtr[4] = { (float)m[0][2], 0.0f, (float)m[1][2], 0.0f};

					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, focuscenter);	// (Focus - Center).xy, 0, 0
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, qCoef);		// (Focus-Center).LengthSquared - Radius^2, 0, 0, 0
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, mrs);	// InverseGradMatrix * PhysicalToLogical (affine part)
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, mtr); // InverseGradMatrix * PhysicalToLogical (translation part)

					// Color alpha
					GVect<float, 4> col(1, 1, 1, (float)Color[G_W]);
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, (const float *)col.Data());

					// u coordinate scaling (to support correct constant color interpolation and devices where
					// mirrored textures are not supported)
					float texMul[4];
					if (Gradient->SpreadMode() == G_REFLECT_COLOR_RAMP_SPREAD) {
						
						if (Gradient->ColorInterpolation() == G_CONSTANT_COLOR_INTERPOLATION || !gMirroredRepeatSupport)
							texMul[0] = texMul[1] = texMul[2] = texMul[3] = 0.5f;
						else
							texMul[0] = texMul[1] = texMul[2] = texMul[3] = 1.0f;
					}
					else
						texMul[0] = texMul[1] = texMul[2] = texMul[3] = 1.0f;
					// 5 = texture u coordinate scaling
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 5, texMul);

					// set alpha pipeline, according to stroke and color keys opacity
					if (Gradient->gAlphaKeys || col[G_W] < 1.0f)
						glEnable(GL_BLEND);
					else
						glDisable(GL_BLEND);

					// enable texture 1D and disable automatic texture coordinate generation
					glEnable(GL_TEXTURE_1D);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_TEXTURE_GEN_S);
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

							if (Gradient->ColorInterpolation() == G_CONSTANT_COLOR_INTERPOLATION || !gMirroredRepeatSupport)
								glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
							else
								glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
							/*if (Gradient->ColorInterpolation() != G_CONSTANT_COLOR_INTERPOLATION) {
								// just to be sure, if shaders are supported, it would be so for repeat mode too
								if (gMirroredRepeatSupport)
									glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
								else
									glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
							}
							else
								glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);*/
							break;
					}
				}
				// conical gradient
				else {
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gConGradGLProgram);

					const GPoint2& pc = Gradient->StartPoint();
					const GPoint2& pt = Gradient->AuxPoint();

					GVector2 dir = pt - pc;
					GReal l = dir.Length();

					if (l <= G_EPSILON)
						dir.Set(1, 0);
					else
						dir /= l;

					float rotM[4] = { (float)dir[G_X], (float)dir[G_Y], (float)(-dir[G_Y]), (float)dir[G_X] };
					float center[4] = { (float)pc[G_X], (float)pc[G_Y], 0.0f, 0.0f };
					static float texBias[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, rotM);	// 0 = [cos, sin, -sin, cos]
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, center);	// 1 = [Center.x, Center.y, 0, 0]
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, texBias);	// 2 = [0.5, 0.5, 0.5, 0.5]
					// Color alpha
					GVect<float, 4> col(1, 1, 1, (float)Color[G_W]);
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, (const float *)col.Data()); // 3 = [1, 1, 1, Alpha]
					// inverse gradient matrix
					GMatrix33 m = Gradient->InverseMatrix() * PhysicalToLogicalMatrix();

					float mrs[4] = { (float)m[0][0], (float)m[0][1], (float)m[1][0], (float)m[1][1]};
					float mtr[4] = { (float)m[0][2], 0.0f, (float)m[1][2], 0.0f};
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, mrs); // InverseGradMatrix (affine part)
					glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 5, mtr); // InverseGradMatrix (translation part)

					// set alpha pipeline, according to stroke and color keys opacity
					if (Gradient->gAlphaKeys || col[G_W] < 1.0f)
						glEnable(GL_BLEND);
					else
						glDisable(GL_BLEND);

					// enable texture 2D and disable automatic texture coordinate generation
					glDisable(GL_TEXTURE_1D);
					glEnable(GL_TEXTURE_2D);
					glDisable(GL_TEXTURE_GEN_S);
					glDisable(GL_TEXTURE_GEN_T);
					// bind gradient texture
					glBindTexture(GL_TEXTURE_2D, Gradient->GradientTexture());
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
			}
		}
	}
	// pattern paint
	else {
		
		GVector4 col = Color;
		col[G_X] = col[G_Y] = col[G_Z] = (GReal)1;

		// suppose we have both color and texture transparency
		if (col[G_W] < (GReal)1) {
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		}
		else {
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		}
		SetGLColor(col);

		glEnable(GL_BLEND);
		if (!UseFill)
			useDepthBuffer = G_TRUE;

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);

		// lets use texture coordinate generation in eye space which is in canvas coordinates
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

		const GAABox2& patWindow = Pattern->LogicalWindow();
		GReal xAxisLen = patWindow.Dimension(G_X);
		GReal yAxisLen = patWindow.Dimension(G_Y);

		GMatrix33 m, postTrans, postTrans2, scale;

		GVector4 planeS(InverseModelView[0][0], InverseModelView[1][0], 0, InverseModelView[0][2]);
		GVector4 planeT(InverseModelView[0][1], InverseModelView[1][1], 0, InverseModelView[1][2]);

	#ifdef DOUBLE_REAL_TYPE
		glTexGendv(GL_S, GL_EYE_PLANE, (const GLdouble *)planeS.Data());
		glTexGendv(GL_T, GL_EYE_PLANE, (const GLdouble *)planeT.Data());
	#else
		glTexGenfv(GL_S, GL_EYE_PLANE, (const GLfloat *)planeS.Data());
		glTexGenfv(GL_T, GL_EYE_PLANE, (const GLfloat *)planeT.Data());
	#endif

		TranslationToMatrix(postTrans2, -patWindow.Min());
		ScaleToMatrix(scale, GVector2(1 / xAxisLen, -1 / yAxisLen));
		TranslationToMatrix(postTrans, GPoint2(0, 1));

		if (Pattern->TilingMode() == G_REFLECT_TILE) {
			if (gMirroredRepeatSupport)
				m = (postTrans * (scale * (postTrans2 * Pattern->InverseMatrix())));
			else {
				GMatrix33 reflectFactors;
				ScaleToMatrix(reflectFactors, GVector3((GReal)0.5, (GReal)0.5, 1));
				m = (reflectFactors * (postTrans * (scale * (postTrans2 * Pattern->InverseMatrix()))));
			}
		}
		else
			m = (postTrans * (scale * (postTrans2 * Pattern->InverseMatrix())));

		// load texture matrix
		glMatrixMode(GL_TEXTURE);
		SetGLTextureMatrix(m);

		// enable texture 2D
		glDisable(GL_TEXTURE_1D);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);

		// bind gradient texture
		if (Pattern->TilingMode() == G_REFLECT_TILE) {
			if (gMirroredRepeatSupport)
				glBindTexture(GL_TEXTURE_2D, Pattern->PatternTexture());
			else
				glBindTexture(GL_TEXTURE_2D, Pattern->gPatternMirroredTexture);
		}
		else
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
				if (gMirroredRepeatSupport) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT_ARB);
				}
				else {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}
				break;
		}

	}
	return useDepthBuffer;
}

GBool GOpenGLBoard::UseStrokeStyle(const GDrawStyle& Style) {

	return UseStyle(Style.StrokePaintType(), Style.StrokeColor(), (const GOpenGLGradientDesc *)Style.StrokeGradient(),
					(const GOpenGLPatternDesc *)Style.StrokePattern(), Style.ModelView(), Style.InverseModelView(),
					G_FALSE);
}

GBool GOpenGLBoard::UseFillStyle(const GDrawStyle& Style) {

	return UseStyle(Style.FillPaintType(), Style.FillColor(), (const GOpenGLGradientDesc *)Style.FillGradient(),
					(const GOpenGLPatternDesc *)Style.FillPattern(), Style.ModelView(), Style.InverseModelView(),
					G_TRUE);
}

void GOpenGLBoard::PushDepthMask() {

	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);

	GReal left, right, top, bottom;
	GMatrix44 m;

	Projection(left, right, bottom, top);

	m[0][0] = (GReal)2 / (right - left);
	m[0][3] = -(right + left) / (right - left);

	m[1][1] = (GReal)2 / (top - bottom);
	m[1][3] = -(top + bottom) / (top - bottom);

	m[2][3] = (GReal)2e-7 - (GReal)1;

	// now all points (that have z = 0 because glVertex2dv/fv) will have a z-window value equal to 0.5
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	#ifdef DOUBLE_REAL_TYPE
		glLoadMatrixd((const GLdouble *)m.Data());
	#else
		glLoadMatrixf((const GLfloat *)m.Data());
	#endif
}

void GOpenGLBoard::DrawAndPopDepthMask(const GAABox2& Box, const GDrawStyle& Style, const GBool DrawFill) {

	glDisable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glDepthMask(GL_FALSE);

	GPoint2 p0 = Box.Min();
	GPoint2 p2 = Box.Max();
	GPoint2 p1(p0[G_X], p2[G_Y]);
	GPoint2 p3(p2[G_X], p0[G_Y]);

	GBool drawRadGrad = G_FALSE;
	GBool drawConGrad = G_FALSE;
	GOpenGLGradientDesc *g = NULL;

	if (DrawFill) {
		if (Style.FillPaintType() == G_GRADIENT_PAINT_TYPE && Style.FillGradient()) {
			g = (GOpenGLGradientDesc *)Style.FillGradient();
			if (g->Type() == G_RADIAL_GRADIENT && g->ColorKeys().size() > 1)
				drawRadGrad = G_TRUE;
			else
			if (g->Type() == G_CONICAL_GRADIENT && g->ColorKeys().size() > 1)
				drawConGrad = G_TRUE;
		}
	}
	else {
		if (Style.StrokePaintType() == G_GRADIENT_PAINT_TYPE && Style.StrokeGradient()) {
			g = (GOpenGLGradientDesc *)Style.StrokeGradient();
			if (g->Type() == G_RADIAL_GRADIENT && g->ColorKeys().size() > 1)
				drawRadGrad = G_TRUE;
			else
			if (g->Type() == G_CONICAL_GRADIENT && g->ColorKeys().size() > 1)
				drawConGrad = G_TRUE;
		}
	}


	if (drawRadGrad) {
		if (DrawFill)
			DrawRadialSector(g->StartPoint(), g->AuxPoint(), g->Radius(), Box, g->ColorKeys(),
							 g->ColorInterpolation(), g->SpreadMode(), Style.FillColor()[G_W],
							 g->Matrix(), g->InverseMatrix(), Style.ModelView());
		else
			DrawRadialSector(g->StartPoint(), g->AuxPoint(), g->Radius(), Box, g->ColorKeys(),
							 g->ColorInterpolation(), g->SpreadMode(), Style.StrokeColor()[G_W],
							 g->Matrix(), g->InverseMatrix(), Style.ModelView());
	}
	else
	if (drawConGrad) {
		if (DrawFill)
			DrawConicalSector(g->StartPoint(), g->AuxPoint(), Box, g->ColorKeys(), g->gInTangents, g->gOutTangents,
							  g->ColorInterpolation(), Style.FillColor()[G_W],
							  g->Matrix(), g->InverseMatrix(), Style.ModelView());
		else
			DrawConicalSector(g->StartPoint(), g->AuxPoint(), Box, g->ColorKeys(), g->gInTangents, g->gOutTangents,
							  g->ColorInterpolation(), Style.StrokeColor()[G_W],
							  g->Matrix(), g->InverseMatrix(), Style.ModelView());
	}
	else {
		// draw into color buffer
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

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// draw into depth buffer
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
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
}

};	// end namespace Amanith
