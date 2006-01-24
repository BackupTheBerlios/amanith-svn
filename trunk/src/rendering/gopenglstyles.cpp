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

	if (gGradientTexture)
		glDeleteTextures(1, &gGradientTexture);
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
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_1D, gGradientTexture);
	SetGLGradientQuality(Quality);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, (GLsizei)size, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pixels);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, gGradientTexture);
	// set texture min/mag filters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)Atan2LookupTableSize, (GLsizei)Atan2LookupTableSize,
				0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pixelsDst);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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

	if (gPatternTexture)
		glDeleteTextures(1, &gPatternTexture);

	if (gPatternMirroredTexture)
		glDeleteTextures(1, &gPatternMirroredTexture);
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

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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
	if (gMirroredRepeatSupport) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		return;
	}

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
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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

void GOpenGLBoard::DrawGLBox(const GAABox2& Box) {

	glBegin(GL_POLYGON);
	#ifdef DOUBLE_REAL_TYPE
		glVertex2d(Box.Min()[G_X], Box.Min()[G_Y]);
		glVertex2d(Box.Min()[G_X], Box.Max()[G_Y]);
		glVertex2d(Box.Max()[G_X], Box.Max()[G_Y]);
		glVertex2d(Box.Max()[G_X], Box.Min()[G_Y]);
	#else
		glVertex2f(Box.Min()[G_X], Box.Min()[G_Y]);
		glVertex2f(Box.Min()[G_X], Box.Max()[G_Y]);
		glVertex2f(Box.Max()[G_X], Box.Max()[G_Y]);
		glVertex2f(Box.Max()[G_X], Box.Min()[G_Y]);
	#endif
	glEnd();
}

void GOpenGLBoard::DrawGLBox(const GGenericAABox<GInt32, 2>& Box) {

	glBegin(GL_POLYGON);
		glVertex2i(Box.Min()[G_X], Box.Min()[G_Y]);
		glVertex2i(Box.Min()[G_X], Box.Max()[G_Y]);
		glVertex2i(Box.Max()[G_X], Box.Max()[G_Y]);
		glVertex2i(Box.Max()[G_X], Box.Min()[G_Y]);
	glEnd();
}

void GOpenGLBoard::UpdateStyle(GOpenGLDrawStyle& Style) {

	if (Style.StrokeEnabled()) {

		if (Style.StrokePaintType() == G_GRADIENT_PAINT_TYPE && Style.StrokeGradient()) {

			GOpenGLGradientDesc *g = (GOpenGLGradientDesc *)Style.StrokeGradient();
			if (g->Modified()) {
				if ((g->Type() == G_LINEAR_GRADIENT) ||
					(g->Type() == G_RADIAL_GRADIENT && gFragmentProgramsInUse)) {

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
						if (gFragmentProgramsInUse)
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
					(g->Type() == G_RADIAL_GRADIENT && gFragmentProgramsInUse)) {

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
						if (gFragmentProgramsInUse)
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

GBool GOpenGLBoard::NeedDepthMask(const GOpenGLDrawStyle& Style, const GBool Fill) const {

	// graphic style has sense only for color buffer
	if (TargetMode() == G_CLIP_MODE || TargetMode() == G_CLIP_AND_CACHE_MODE || TargetMode() == G_CACHE_MODE)
		return G_FALSE;

	GBool useDepthBuffer;

	GPaintType paintType;
	GReal colorAlpha;
	GOpenGLGradientDesc *gradient = NULL;
	GOpenGLPatternDesc *pattern = NULL;
	GCompositingOperation compOp;

	if (Fill) {
		paintType = Style.FillPaintType();
		colorAlpha = Style.FillColor()[G_W];
		gradient = (GOpenGLGradientDesc *)Style.FillGradient();
		pattern = (GOpenGLPatternDesc *)Style.FillPattern();
		compOp = Style.FillCompOp();
	}
	else {
		paintType = Style.StrokePaintType();
		colorAlpha = Style.StrokeColor()[G_W];
		gradient = (GOpenGLGradientDesc *)Style.StrokeGradient();
		pattern = (GOpenGLPatternDesc *)Style.StrokePattern();
		compOp = Style.StrokeCompOp();
	}

	switch (compOp) {

		case G_CLEAR_OP:
		case G_SRC_OP:
		case G_DST_OP:
		case G_SRC_OVER_OP:
		case G_DST_IN_OP:
		case G_DST_OUT_OP:
		case G_SRC_ATOP_OP:
			useDepthBuffer = G_FALSE;
			break;

		case G_DST_OVER_OP:
		case G_SRC_IN_OP:
		case G_SRC_OUT_OP:
		case G_DST_ATOP_OP:
		case G_XOR_OP:
		case G_PLUS_OP:
		case G_MULTIPLY_OP:
		case G_SCREEN_OP:
		case G_EXCLUSION_OP:
		case G_OVERLAY_OP:
		case G_DARKEN_OP:
		case G_LIGHTEN_OP:
		case G_COLOR_DODGE_OP:
		case G_COLOR_BURN_OP:
		case G_HARD_LIGHT_OP:
		case G_SOFT_LIGHT_OP:
		case G_DIFFERENCE_OP:
			useDepthBuffer = G_TRUE;
			break;
	}


	// color paint
	if ((paintType == G_COLOR_PAINT_TYPE) || 
		(paintType == G_GRADIENT_PAINT_TYPE && !gradient) ||
		(paintType == G_GRADIENT_PAINT_TYPE && gradient && gradient->ColorKeys().size() < 2) ||
		(paintType == G_PATTERN_PAINT_TYPE && !pattern)) {

		if (colorAlpha < (GReal)1 && !Fill)
			useDepthBuffer = G_TRUE;
	}
	else
	// gradient paint
	if (paintType == G_GRADIENT_PAINT_TYPE) {

		// linear gradient
		if (gradient->Type() == G_LINEAR_GRADIENT) {
			if ((gradient->gAlphaKeys || colorAlpha < (GReal)1) && !Fill)
				useDepthBuffer = G_TRUE;
		}
		// radial/conical gradient
		else {
			// geometrical version
			if (!gFragmentProgramsInUse)
				// for radial gradients we use always depth buffer
				useDepthBuffer = G_TRUE;
			// fragment program version (radial and conical)
			else {
				// for stroking, we must use zbuffer as mask also for conical and radial gradients (if they
				// are transparent)
				if ((gradient->gAlphaKeys || colorAlpha < (GReal)1) && !Fill)
					useDepthBuffer = G_TRUE;
			}
		}
	}
	// pattern paint
	else {
		if (!Fill)
			useDepthBuffer = G_TRUE;
	}
	return useDepthBuffer;
}

void GOpenGLBoard::UseStyle(const GPaintType PaintType, const GCompositingOperation CompOp, const GUInt32 PassIndex,
							 const GVector4& Color,
							 const GOpenGLGradientDesc *Gradient, const GOpenGLPatternDesc *Pattern,
							 const GMatrix33& ModelView, const GMatrix33& InverseModelView) {

	#define COLOR_PRG	0
	#define LINGRAD_PRG	1
	#define RADGRAD_PRG	2
	#define CONGRAD_PRG	3
	#define PATTERN_PRG	4
	
	#define TEXTURE_POW2 0
	#define TEXTURE_RECT 1

	// graphic style has sense only for color buffer
	if (TargetMode() == G_CLIP_MODE || TargetMode() == G_CLIP_AND_CACHE_MODE || TargetMode() == G_CACHE_MODE)
		return;

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	// set compositing operation
	GBool screenGrabbed = SetGLStyleCompOp(CompOp, PassIndex);

	if (gFragmentProgramsInUse) {

		// enable fragment program
		glEnable(GL_FRAGMENT_PROGRAM_ARB);

		// color paint
		if ((PaintType == G_COLOR_PAINT_TYPE) || 
			(PaintType == G_GRADIENT_PAINT_TYPE && !Gradient) ||
			(PaintType == G_GRADIENT_PAINT_TYPE && Gradient && Gradient->ColorKeys().size() < 2) ||
			(PaintType == G_PATTERN_PAINT_TYPE && !Pattern)	||
			(CompOp == G_CLEAR_OP)) {

			GVector4 col = Color;
			if (PaintType == G_GRADIENT_PAINT_TYPE && Gradient && Gradient->ColorKeys().size() == 1) {
				GKeyValue key = Gradient->ColorKeys().front();
				col = key.Vect4Value();
			}

			if (CompOp == G_CLEAR_OP)
				col.Set(0, 0, 0, 0);

			if (gAlphaBufferPresent) {
				if (gRectTexturesInUse)
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][COLOR_PRG][TEXTURE_RECT]);
				else
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][COLOR_PRG][TEXTURE_POW2]);
			}
			else {
				if (gRectTexturesInUse)
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][COLOR_PRG][TEXTURE_RECT]);
				else
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][COLOR_PRG][TEXTURE_POW2]);
			}

			// select texture unit 0
			SELECT_AND_DISABLE_TUNIT(0)

			if (screenGrabbed) {
				glEnable(gCompositingBuffer.Target);
				G_ASSERT(gCompositingBuffer.TexName > 0);
				glBindTexture(gCompositingBuffer.Target, gCompositingBuffer.TexName);
				glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			SetGLColor(col);
		}
		else
		// gradient paint
		if (PaintType == G_GRADIENT_PAINT_TYPE) {

			// linear gradient
			if (Gradient->Type() == G_LINEAR_GRADIENT) {

				GMatrix33 genMatrix = Gradient->Matrix();
				GMatrix33 p2lMatrix = PhysicalToLogicalMatrix();

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

				// bind the program
				if (gAlphaBufferPresent) {
					if (gRectTexturesInUse)
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][LINGRAD_PRG][TEXTURE_RECT]);
					else
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][LINGRAD_PRG][TEXTURE_POW2]);
				}
				else {
					if (gRectTexturesInUse)
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][LINGRAD_PRG][TEXTURE_RECT]);
					else
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][LINGRAD_PRG][TEXTURE_POW2]);
				}

				// pass uv generation-plane to program
				float c0[4] = { (float)n[G_X], (float)n[G_Y], 0.0f, (float)q};
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, c0);	// 0 = [n.x, n.y, 0, q]
				// pass color alpha to program
				float c1[4] = { 1.0f, 1.0f, 1.0f, (float)Color[G_W] };
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, c1); // 1 = [1, 1, 1, Alpha]
				// pass physical to logical matrix
				float c2[4] = { (float)p2lMatrix[0][0], (float)p2lMatrix[0][1], 0.0f, (float)p2lMatrix[0][2]};
				float c3[4] = { (float)p2lMatrix[1][0], (float)p2lMatrix[1][1], 0.0f, (float)p2lMatrix[1][2]};
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, c2); // 2 = [a00, a01, 0, a02]
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, c3); // 3 = [a10, a11, 0, a12]

				// select texture unit 0
				SELECT_AND_DISABLE_TUNIT(1)
				SELECT_AND_DISABLE_TUNIT(0)
				glEnable(GL_TEXTURE_1D);

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
						break;
				}
				// bind the grabbed portion of screen buffer, if needed
				if (screenGrabbed) {
					SELECT_AND_DISABLE_TUNIT(1)
					glEnable(gCompositingBuffer.Target);
					G_ASSERT(gCompositingBuffer.TexName > 0);
					glBindTexture(gCompositingBuffer.Target, gCompositingBuffer.TexName);
					glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
			}
			else
			// radial gradient
			if (Gradient->Type() == G_RADIAL_GRADIENT) {

				// bind the program
				if (gAlphaBufferPresent) {
					if (gRectTexturesInUse)
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][RADGRAD_PRG][TEXTURE_RECT]);
					else
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][RADGRAD_PRG][TEXTURE_POW2]);
				}
				else {
					if (gRectTexturesInUse)
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][RADGRAD_PRG][TEXTURE_RECT]);
					else
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][RADGRAD_PRG][TEXTURE_POW2]);
				}

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

				// select texture unit 0
				SELECT_AND_DISABLE_TUNIT(1)
				SELECT_AND_DISABLE_TUNIT(0)
				glEnable(GL_TEXTURE_1D);

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
						break;
				}
				// bind the grabbed portion of screen buffer, if needed
				if (screenGrabbed) {
					SELECT_AND_DISABLE_TUNIT(1)
					glEnable(gCompositingBuffer.Target);
					G_ASSERT(gCompositingBuffer.TexName > 0);
					glBindTexture(gCompositingBuffer.Target, gCompositingBuffer.TexName);
					glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
			}
			// conical gradient
			else {
				// bind the program
				if (gAlphaBufferPresent) {
					if (gRectTexturesInUse)
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][CONGRAD_PRG][TEXTURE_RECT]);
					else
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][CONGRAD_PRG][TEXTURE_POW2]);
				}
				else {
					if (gRectTexturesInUse)
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][CONGRAD_PRG][TEXTURE_RECT]);
					else
						glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][CONGRAD_PRG][TEXTURE_POW2]);
				}

				const GPoint2& pc = Gradient->StartPoint();
				const GPoint2& pt = Gradient->AuxPoint();

				GVector2 dir = pt - pc;
				GReal l = dir.Length();

				if (l <= G_EPSILON)
					dir.Set(1, 0);
				else
					dir /= l;

				float rotM[4] = { (float)dir[G_X], (float)dir[G_Y], (float)(-dir[G_Y]), (float)dir[G_X] };
				static float texBias[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, rotM);	// 0 = [cos, sin, -sin, cos]
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, texBias);	// 1 = [0.5, 0.5, 0.5, 0.5]
				// Color alpha
				GVect<float, 4> col(1, 1, 1, (float)Color[G_W]);
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, (const float *)col.Data()); // 2 = [1, 1, 1, Alpha]
				// inverse gradient matrix
				GMatrix33 m = Gradient->InverseMatrix() * PhysicalToLogicalMatrix();

				float mrs[4] = { (float)m[0][0], (float)m[0][1], (float)m[1][0], (float)m[1][1]};
				float mtr[4] = { (float)m[0][2], 0.0f, (float)m[1][2], 0.0f};
				float center[4] = { (float)pc[G_X], (float)pc[G_Y], 0.0f, 0.0f };

				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, mrs); // InverseGradMatrix (affine part)
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, mtr); // InverseGradMatrix (translation part)
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 5, center); // center.x, center.y, 0, 0

				// select texture unit 0
				SELECT_AND_DISABLE_TUNIT(1)
				SELECT_AND_DISABLE_TUNIT(0)
				glEnable(GL_TEXTURE_2D);

				// bind gradient texture
				glBindTexture(GL_TEXTURE_2D, Gradient->GradientTexture());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				// bind the grabbed portion of screen buffer, if needed
				if (screenGrabbed) {
					SELECT_AND_DISABLE_TUNIT(1)
					glEnable(gCompositingBuffer.Target);
					G_ASSERT(gCompositingBuffer.TexName > 0);
					glBindTexture(gCompositingBuffer.Target, gCompositingBuffer.TexName);
					glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
			}
		}
		// pattern paint
		else {
			// bind the program
			if (gAlphaBufferPresent) {
				if (gRectTexturesInUse)
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][PATTERN_PRG][TEXTURE_RECT]);
				else
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGBA[CompOp][PATTERN_PRG][TEXTURE_POW2]);
			}
			else {
				if (gRectTexturesInUse)
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][PATTERN_PRG][TEXTURE_RECT]);
				else
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompProgramsRGB[CompOp][PATTERN_PRG][TEXTURE_POW2]);
			}

			const GAABox2& patWindow = Pattern->LogicalWindow();
			GReal xAxisLen = patWindow.Dimension(G_X);
			GReal yAxisLen = patWindow.Dimension(G_Y);

			GMatrix33 m, postTrans, postTrans2, scale;

			TranslationToMatrix(postTrans2, -patWindow.Min());
			ScaleToMatrix(scale, GVector2(1 / xAxisLen, -1 / yAxisLen));
			TranslationToMatrix(postTrans, GPoint2(0, 1));

			if (Pattern->TilingMode() == G_REFLECT_TILE) {
				if (gMirroredRepeatSupport)
					m = (postTrans * (scale * (postTrans2 * (Pattern->InverseMatrix() * PhysicalToLogicalMatrix()))));
				else {
					GMatrix33 reflectFactors;
					ScaleToMatrix(reflectFactors, GVector3((GReal)0.5, (GReal)0.5, 1));
					m = reflectFactors * (postTrans * (scale * (postTrans2 * (Pattern->InverseMatrix() * PhysicalToLogicalMatrix()))));
				}
			}
			else
				m = (postTrans * (scale * (postTrans2 * (Pattern->InverseMatrix() * PhysicalToLogicalMatrix()))));

			float mr0[4] = { (float)m[0][0], (float)m[0][1], 0.0f, (float)m[0][2] };
			float mt1[4] = { (float)m[1][0], (float)m[1][1], 0.0f, (float)m[1][2] };

			glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, mr0); // 0 = [a00, a01, 0, a02]
			glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, mt1); // 1 = [a10, a11, 0, a12]

			// Color alpha
			GVect<float, 4> col(1, 1, 1, (float)Color[G_W]);
			glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, (const float *)col.Data()); // 2 = [1, 1, 1, Alpha]

			// select texture unit 0
			SELECT_AND_DISABLE_TUNIT(1)
			SELECT_AND_DISABLE_TUNIT(0)
			glEnable(GL_TEXTURE_2D);

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
			// bind the grabbed portion of screen buffer, if needed
			if (screenGrabbed) {
				SELECT_AND_DISABLE_TUNIT(1)
				glEnable(gCompositingBuffer.Target);
				G_ASSERT(gCompositingBuffer.TexName > 0);
				glBindTexture(gCompositingBuffer.Target, gCompositingBuffer.TexName);
				glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(gCompositingBuffer.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}
	else {

		GLDisableShaders();

		// color paint
		if ((PaintType == G_COLOR_PAINT_TYPE) || 
			(PaintType == G_GRADIENT_PAINT_TYPE && !Gradient) ||
			(PaintType == G_GRADIENT_PAINT_TYPE && Gradient && Gradient->ColorKeys().size() < 2) ||
			(PaintType == G_PATTERN_PAINT_TYPE && !Pattern) ||
			(CompOp == G_CLEAR_OP)) {

				GVector4 col = Color;
				if (PaintType == G_GRADIENT_PAINT_TYPE && Gradient && Gradient->ColorKeys().size() == 1) {
					GKeyValue key = Gradient->ColorKeys().front();
					col = key.Vect4Value();
				}

				if (CompOp == G_CLEAR_OP)
					col.Set(0, 0, 0, 0);

				SELECT_AND_DISABLE_TUNIT(0)
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
				SetGLColor(col);
		}
		else
		// gradient paint
		if (PaintType == G_GRADIENT_PAINT_TYPE) {

			// linear gradient
			if (Gradient->Type() == G_LINEAR_GRADIENT) {

				// enable texture 1D and automatic texture coordinate generation
				SELECT_AND_DISABLE_TUNIT(1)
				SELECT_AND_DISABLE_TUNIT(0)
				glEnable(GL_TEXTURE_1D);
				glEnable(GL_TEXTURE_GEN_S);

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
				}
				else {
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
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

				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
				#ifdef DOUBLE_REAL_TYPE
					glTexGendv(GL_S, GL_OBJECT_PLANE, (const GLdouble *)plane.Data());
				#else
					glTexGenfv(GL_S, GL_OBJECT_PLANE, (const GLfloat *)plane.Data());
				#endif

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
						break;
				}
			}
			// radial/conical gradient
			else {
				// geometrical version
				SELECT_AND_DISABLE_TUNIT(1)
				SELECT_AND_DISABLE_TUNIT(0)
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
				}
				else {
					GVect<GLfloat, 4> colf;
					colf[G_X] = colf[G_Y] = colf[G_Z] = (GLfloat)1;
					colf[G_W] = (GLfloat)Color[G_W];
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				}
			}
		}
		// pattern paint
		else {
			GVector4 col = Color;
			col[G_X] = col[G_Y] = col[G_Z] = (GReal)1;

			// enable texture 2D
			SELECT_AND_DISABLE_TUNIT(1)
			SELECT_AND_DISABLE_TUNIT(0)
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);

			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			SetGLColor(col);

			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);

			// lets use texture coordinate generation in eye space which is in canvas coordinates
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

			const GAABox2& patWindow = Pattern->LogicalWindow();
			GReal xAxisLen = patWindow.Dimension(G_X);
			GReal yAxisLen = patWindow.Dimension(G_Y);

			GMatrix33 m, postTrans, postTrans2, scale;

			GVector4 planeS(1, 0, 0, 0);
			GVector4 planeT(0, 1, 0, 0);

			#ifdef DOUBLE_REAL_TYPE
				glTexGendv(GL_S, GL_OBJECT_PLANE, (const GLdouble *)planeS.Data());
				glTexGendv(GL_T, GL_OBJECT_PLANE, (const GLdouble *)planeT.Data());
			#else
				glTexGenfv(GL_S, GL_OBJECT_PLANE, (const GLfloat *)planeS.Data());
				glTexGenfv(GL_T, GL_OBJECT_PLANE, (const GLfloat *)planeT.Data());
			#endif

			TranslationToMatrix(postTrans2, -patWindow.Min());
			ScaleToMatrix(scale, GVector2(1 / xAxisLen, -1 / yAxisLen));
			TranslationToMatrix(postTrans, GPoint2(0, 1));

			if (Pattern->TilingMode() == G_REFLECT_TILE) {
				if (gMirroredRepeatSupport)
					m = (postTrans * (scale * (postTrans2 * (Pattern->InverseMatrix() * ModelView))));
				else {
					GMatrix33 reflectFactors;
					ScaleToMatrix(reflectFactors, GVector3((GReal)0.5, (GReal)0.5, 1));
					m = (reflectFactors * (postTrans * (scale * (postTrans2 * (Pattern->InverseMatrix() * ModelView)))));
				}
			}
			else
				m = (postTrans * (scale * (postTrans2 * (Pattern->InverseMatrix() * ModelView))));

			// load texture matrix
			glMatrixMode(GL_TEXTURE);
			SetGLTextureMatrix(m);

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
	}
	#undef COLOR_PRG
	#undef LINGRAD_PRG
	#undef RADGRAD_PRG
	#undef CONGRAD_PRG
	#undef PATTERN_PRG

	#undef TEXTURE_POW2
	#undef TEXTURE_RECT
}

void GOpenGLBoard::UseGroupStyle(const GUInt32 PassIndex, const GLGrabbedRect& GroupColorRect,
								 const GLGrabbedRect& BackgroundRect) {

	#define TEXTURE_POW2 0
	#define TEXTURE_RECT 1

	// graphic style has sense only for color buffer
	if (TargetMode() == G_CLIP_MODE || TargetMode() == G_CLIP_AND_CACHE_MODE || TargetMode() == G_CACHE_MODE)
		return;

	// set compositing operation
	GBool screenGrabbed = SetGLStyleCompOp(GroupCompOp(), PassIndex);

	if (gFragmentProgramsInUse) {

		// enable fragment program
		glEnable(GL_FRAGMENT_PROGRAM_ARB);

		// bind the program
		if (gAlphaBufferPresent) {
			if (gRectTexturesInUse)
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompGroupProgramsRGBA[GroupCompOp()][TEXTURE_RECT]);
			else
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompGroupProgramsRGBA[GroupCompOp()][TEXTURE_POW2]);
		}
		else {
			if (gRectTexturesInUse)
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompGroupProgramsRGB[GroupCompOp()][TEXTURE_RECT]);
			else
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gCompGroupProgramsRGB[GroupCompOp()][TEXTURE_POW2]);
		}

		// Color alpha
		GVect<float, 4> col(1, 1, 1, (float)GroupOpacity());
		glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, (const float *)col.Data()); // 0 = [1, 1, 1, Alpha]

		// select texture unit 0
		SELECT_AND_DISABLE_TUNIT(1)
		SELECT_AND_DISABLE_TUNIT(0)
		glEnable(GroupColorRect.Target);

		// bind gradient texture
		glBindTexture(GroupColorRect.Target, GroupColorRect.TexName);

		// set tiling mode
		glTexParameteri(GroupColorRect.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GroupColorRect.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// bind the grabbed portion of screen buffer, if needed
		if (screenGrabbed) {
			SELECT_AND_DISABLE_TUNIT(1)
			glEnable(BackgroundRect.Target);
			G_ASSERT(BackgroundRect.TexName > 0);
			glBindTexture(BackgroundRect.Target, BackgroundRect.TexName);
			glTexParameteri(BackgroundRect.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(BackgroundRect.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
	}
	else {

		GLDisableShaders();

		GVector4 col(1, 1, 1, GroupOpacity());

		// enable texture 2D
		SELECT_AND_DISABLE_TUNIT(1)
		SELECT_AND_DISABLE_TUNIT(0)
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glEnable(GroupColorRect.Target);

		SetGLColor(col);

		// bind gradient texture
		glBindTexture(GroupColorRect.Target, GroupColorRect.TexName);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);

		// load texture matrix
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		// set tiling mode
		glTexParameteri(GroupColorRect.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GroupColorRect.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	#undef TEXTURE_POW2
	#undef TEXTURE_RECT
}

void GOpenGLBoard::UseStrokeStyle(const GDrawStyle& Style, const GUInt32 PassIndex) {

	UseStyle(Style.StrokePaintType(), Style.StrokeCompOp(), PassIndex,
			 Style.StrokeColor(), (const GOpenGLGradientDesc *)Style.StrokeGradient(),
			 (const GOpenGLPatternDesc *)Style.StrokePattern(), Style.ModelView(), Style.InverseModelView());
}

void GOpenGLBoard::UseFillStyle(const GDrawStyle& Style, const GUInt32 PassIndex) {

	UseStyle(Style.FillPaintType(), Style.FillCompOp(), PassIndex,
			Style.FillColor(), (const GOpenGLGradientDesc *)Style.FillGradient(),
			(const GOpenGLPatternDesc *)Style.FillPattern(), Style.ModelView(), Style.InverseModelView());
}

void GOpenGLBoard::ReplaceFrameBuffer(const GLGrabbedRect& GrabbedRect, const GCompositingOperation CompOp,
									  const GUInt32 PassIndex) {

	GLDisableShaders();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	SELECT_AND_DISABLE_TUNIT(1)
	SELECT_AND_DISABLE_TUNIT(0)
	glEnable(GrabbedRect.Target);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	// fixes old hardware issues (irix for example)
	SetGLColor(GVector4(1, 1, 1, 1));

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glBindTexture(GrabbedRect.Target, GrabbedRect.TexName);

	switch (CompOp) {

		case G_CLEAR_OP:
		case G_SRC_OP:
		case G_DST_OP:
		case G_SRC_OVER_OP:
		case G_DST_IN_OP:
		case G_DST_OUT_OP:
		case G_PLUS_OP:
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDisable(GL_BLEND);
			break;

		case G_DST_OVER_OP:
			G_ASSERT(PassIndex == 0);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			break;
		case G_SRC_IN_OP:
			G_ASSERT(PassIndex == 0);
			glEnable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
			break;
		case G_SRC_OUT_OP:
			G_ASSERT(PassIndex == 0);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			break;
		case G_SRC_ATOP_OP:
			G_ASSERT(PassIndex == 0 || PassIndex == 1);
			if (PassIndex == 0) {
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
				glEnable(GL_BLEND);
			}
			else {
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				glDisable(GL_BLEND);
			}
			break;
		case G_DST_ATOP_OP:
			G_ASSERT(PassIndex == 0);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
			glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			break;
		case G_XOR_OP:
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case G_SCREEN_OP:
			if (gAlphaBufferPresent) {
				G_ASSERT(PassIndex == 0 || PassIndex == 1);
				if (PassIndex == 0) {
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
					glEnable(GL_BLEND);
				}
				else {
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					glEnable(GL_BLEND);
				}
			}
			else {
				G_ASSERT(PassIndex == 0);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
				glEnable(GL_BLEND);
			}
			break;
		case G_EXCLUSION_OP:
			if (gAlphaBufferPresent) {
				G_ASSERT(PassIndex == 0 || PassIndex == 1);
				if (PassIndex == 0) {
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
					glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
					glEnable(GL_BLEND);
				}
				else {
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
					glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
					glEnable(GL_BLEND);
				}
			}
			else {
				G_ASSERT(PassIndex == 0);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
				glEnable(GL_BLEND);
			}
			break;

		case G_MULTIPLY_OP:
		case G_OVERLAY_OP:
		case G_DARKEN_OP:
		case G_LIGHTEN_OP:
		case G_COLOR_DODGE_OP:
		case G_COLOR_BURN_OP:
		case G_HARD_LIGHT_OP:
		case G_SOFT_LIGHT_OP:
		case G_DIFFERENCE_OP:
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDisable(GL_BLEND);
			break;

		default:
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDisable(GL_BLEND);
			break;
	}

	glTexParameteri(GrabbedRect.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GrabbedRect.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	DrawGrabbedRect(GrabbedRect, G_TRUE, G_FALSE, G_FALSE, G_FALSE);

	glDisable(GrabbedRect.Target);
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

	// now all points (that have z = 0 because glVertex2dv/fv) will have a z-window value equal to +epsilon
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	#ifdef DOUBLE_REAL_TYPE
		glLoadMatrixd((const GLdouble *)m.Data());
	#else
		glLoadMatrixf((const GLfloat *)m.Data());
	#endif
}

void GOpenGLBoard::SelectTextureUnit(const GLint Unit) {

	if (!gMultiTextureSupport)
		return;

	switch (Unit) {
		case 0:
			glActiveTextureARB(GL_TEXTURE0_ARB);
			break;
		case 1:
			glActiveTextureARB(GL_TEXTURE1_ARB);
			break;
		default:
			glActiveTextureARB(GL_TEXTURE0_ARB);
			break;
	}
}

void GOpenGLBoard::SetTextureVertex(const GUInt32 TextureIndex, const GReal u, const GReal v) {

	if (!gMultiTextureSupport) {
		#ifdef DOUBLE_REAL_TYPE
			glTexCoord2d(u, v);
		#else
			glTexCoord2f(u, v);
		#endif
	}
	else {
		switch (TextureIndex) {

			case 0:
				#ifdef DOUBLE_REAL_TYPE
					glMultiTexCoord2dARB(GL_TEXTURE0_ARB, u, v);
				#else
					glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
				#endif
				break;

			case 1:
				#ifdef DOUBLE_REAL_TYPE
					glMultiTexCoord2dARB(GL_TEXTURE1_ARB, u, v);
				#else
					glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u, v);
				#endif
				break;

			default:
				G_DEBUG("SetTextureVertex, TextureIndex greater than 1");
				break;
		}
	}
}

void GOpenGLBoard::DrawAndPopDepthMask(const GAABox2& Box, const GDrawStyle& Style, const GBool DrawFill,
									   const GUInt32 StylePassesCount, const GUInt32 FrameBufferPassesCount,
									   const GBool ScreenGrabbed) {

	// do not touch stencil buffer
	glDisable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	// enable depth test (onto zbuffer there is a "geometry mask", we are going to pop it using this test)
	glEnable(GL_DEPTH_TEST);
	// enable color writing only for those pixels inside "geometry mask"
	glDepthFunc(GL_EQUAL);
	// disable zwrite
	glDepthMask(GL_FALSE);

	GMatrix44 m;
	GReal ll, rr, bb, tt;
	GBool drawGeometricRadGrad = G_FALSE;
	GBool drawGeometricConGrad = G_FALSE;
	GOpenGLGradientDesc *g = NULL;
	GUInt32 tUnit;

	if (DrawFill) {
		if (Style.FillPaintType() == G_GRADIENT_PAINT_TYPE && Style.FillGradient()) {
			g = (GOpenGLGradientDesc *)Style.FillGradient();
			if (g->Type() == G_RADIAL_GRADIENT && g->ColorKeys().size() > 1 && !gFragmentProgramsInUse)
				drawGeometricRadGrad = G_TRUE;
			else
			if (g->Type() == G_CONICAL_GRADIENT && g->ColorKeys().size() > 1 && !gFragmentProgramsInUse)
				drawGeometricConGrad = G_TRUE;
		}
	}
	else {
		if (Style.StrokePaintType() == G_GRADIENT_PAINT_TYPE && Style.StrokeGradient()) {
			g = (GOpenGLGradientDesc *)Style.StrokeGradient();
			if (g->Type() == G_RADIAL_GRADIENT && g->ColorKeys().size() > 1 && !gFragmentProgramsInUse)
				drawGeometricRadGrad = G_TRUE;
			else
			if (g->Type() == G_CONICAL_GRADIENT && g->ColorKeys().size() > 1 && !gFragmentProgramsInUse)
				drawGeometricConGrad = G_TRUE;
		}
	}

	if (drawGeometricRadGrad) {

		if (DrawFill) {
			// repeat color drawing according to number of steps required by current compositing operation
			for (GUInt32 ii = 0; ii < StylePassesCount; ++ii) {
				UseFillStyle(Style, ii);
				DrawRadialSector(g->StartPoint(), g->AuxPoint(), g->Radius(), Box, g->ColorKeys(),
								 g->ColorInterpolation(), g->SpreadMode(), Style.FillColor()[G_W],
								 g->Matrix(), g->InverseMatrix());
			}
		}
		else {
			// repeat color drawing according to number of steps required by current compositing operation
			for (GUInt32 ii = 0; ii < StylePassesCount; ++ii) {
				UseStrokeStyle(Style, ii);
				DrawRadialSector(g->StartPoint(), g->AuxPoint(), g->Radius(), Box, g->ColorKeys(),
								 g->ColorInterpolation(), g->SpreadMode(), Style.StrokeColor()[G_W],
								 g->Matrix(), g->InverseMatrix());
			}
		}
	}
	else
	if (drawGeometricConGrad) {
		if (DrawFill) {
			// repeat color drawing according to number of steps required by current compositing operation
			for (GUInt32 ii = 0; ii < StylePassesCount; ++ii) {
				UseFillStyle(Style, ii);
				DrawConicalSector(g->StartPoint(), g->AuxPoint(), Box, g->ColorKeys(), g->gInTangents, g->gOutTangents,
								  g->ColorInterpolation(), Style.FillColor()[G_W],
								  g->Matrix(), g->InverseMatrix());
			}
		}
		else {
			// repeat color drawing according to number of steps required by current compositing operation
			for (GUInt32 ii = 0; ii < StylePassesCount; ++ii) {
				UseStrokeStyle(Style, ii);
				DrawConicalSector(g->StartPoint(), g->AuxPoint(), Box, g->ColorKeys(), g->gInTangents, g->gOutTangents,
								  g->ColorInterpolation(), Style.StrokeColor()[G_W],
								  g->Matrix(), g->InverseMatrix());
			}
		}
	}
	else {

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// here we are in one of these cases:
		// no pixel shaders (color, lingrad, pattern)
		// pixel shaders (color, lingrad, radgrad, congrad, pattern)
		for (GUInt32 ii = 0; ii < StylePassesCount; ++ii) {
			if (DrawFill)
				UseFillStyle(Style, ii);
			else
				UseStrokeStyle(Style, ii);

			if (!ScreenGrabbed) {
				DrawGLBox(Box);
			}
			else {
				tUnit = 1;
				if ((DrawFill && Style.FillPaintType() == G_COLOR_PAINT_TYPE) ||
					(!DrawFill && Style.StrokePaintType() == G_COLOR_PAINT_TYPE))
					tUnit = 0;

				// to place grabbed screen portion we have to enter in window-mode
				//m = GLWindowModeMatrix((GReal)1e-7);

				Projection(ll, rr, bb, tt);
				m = GLProjectionMatrix(ll, rr, bb, tt, (GReal)1e-7);
				glMatrixMode(GL_PROJECTION);
				#ifdef DOUBLE_REAL_TYPE
					glLoadMatrixd((const GLdouble *)m.Data());
				#else
					glLoadMatrixf((const GLfloat *)m.Data());
				#endif

				if (tUnit == 0)
					DrawGrabbedRect(gCompositingBuffer, G_TRUE, G_FALSE, G_FALSE, G_FALSE);
				else
					DrawGrabbedRect(gCompositingBuffer, G_FALSE, G_FALSE, G_TRUE, G_FALSE);
			}
		}
	}

	// unbind texture on tUnit (tUnit is always used to store grabbed rect for compositing)
	if (gFragmentProgramsInUse) {
		G_ASSERT(FrameBufferPassesCount == 0);
		if (ScreenGrabbed) {
			SELECT_AND_DISABLE_TUNIT(tUnit);
			glBindTexture(gCompositingBuffer.Target, 0);
		}
	}

	GCompositingOperation compOp;
	if (DrawFill)
		compOp = Style.FillCompOp();
	else
		compOp = Style.StrokeCompOp();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// put grabbed frame buffer onto screen to complete compositing operation
	if (FrameBufferPassesCount > 0) {

		Projection(ll, rr, bb, tt);
		m = GLProjectionMatrix(ll, rr, bb, tt, (GReal)1e-7);
		glMatrixMode(GL_PROJECTION);
		#ifdef DOUBLE_REAL_TYPE
			glLoadMatrixd((const GLdouble *)m.Data());
		#else
			glLoadMatrixf((const GLfloat *)m.Data());
		#endif

		// do the drawing
		for (GUInt32 ii = 0; ii < FrameBufferPassesCount; ++ii)
			ReplaceFrameBuffer(gCompositingBuffer, compOp, ii);
	}

	// pop "geometry mask" out of zbuffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	Projection(ll, rr, bb, tt);
	m = GLProjectionMatrix(ll, rr, bb, tt, 1);
	glMatrixMode(GL_PROJECTION);
	#ifdef DOUBLE_REAL_TYPE
		glLoadMatrixd((GLdouble *)m.Data());
	#else
		glLoadMatrixf((GLfloat *)m.Data());
	#endif

	// draw into depth buffer a rectangle
	DrawGLBox(Box);

	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

};	// end namespace Amanith
