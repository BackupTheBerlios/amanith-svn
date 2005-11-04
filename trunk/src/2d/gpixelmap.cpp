/****************************************************************************
** $file: amanith/src/2d/gpixelmap.cpp   0.1.1.0   edited Sep 24 08:00
**
** 2D Pixelmap implementation.
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

#include "amanith/2d/gpixelmap.h"
#include "amanith/support/gutilities.h"
#include "amanith/gkernel.h"
#include "amanith/numerics/gfilter.h"
#include <new> // for nothrow
#include <cstring>  // for memcpy function

/*!
	\file gpixelmap.cpp
	\brief Implementation of GPixelMap class.
*/

namespace Amanith {


struct GColorNode {
	// G_TRUE if node has no children
	GBool gIsLeaf;
	// number of pixels represented by this leaf
	GUInt32 gPixelCount;
	// sum of red components
	GULong gRedSum;
	// sum of green components
	GUInt32 gGreenSum;
	// sum of blue components
	GUInt32 gBlueSum;
	// pointers to child nodes
	GColorNode* gChildren[8];
	// pointer to next reducible node
	GColorNode* gNext;

	// constructor
	GColorNode() {
		gIsLeaf = G_FALSE;
		gPixelCount = 0;
		gRedSum = 0;
		gGreenSum = 0;
		gBlueSum = 0;
		gChildren[0] = NULL;
		gChildren[1] = NULL;
		gChildren[2] = NULL;
		gChildren[3] = NULL;
		gChildren[4] = NULL;
		gChildren[5] = NULL;
		gChildren[6] = NULL;
		gChildren[7] = NULL;
		gNext = NULL;
	}
};

static GColorNode* CreateNode(GUInt32 nLevel, GUInt32 nColorBits, GUInt32* pLeafCount, GColorNode** pReducibleNodes) {

	GColorNode* pNode = NULL;

	pNode = new(std::nothrow) GColorNode();
	if (!pNode)
		return NULL;

	pNode->gIsLeaf = (nLevel == nColorBits) ? G_TRUE : G_FALSE;
	if (pNode->gIsLeaf)
		(*pLeafCount)++;
	else {
		// add the node to the reducible list for this level
		pNode->gNext = pReducibleNodes[nLevel];
		pReducibleNodes[nLevel] = pNode;
	}
	return pNode;
}

static void AddColor(GColorNode** ppNode, GUChar8 r, GUChar8 g, GUChar8 b, GUInt32 nColorBits,
					GUInt32 nLevel, GUInt32* pLeafCount, GColorNode** pReducibleNodes) {
	
	GInt32 nIndex, shift;
	static GUChar8 mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

	// if the node doesn't exist, create it
	if (*ppNode == NULL)
		*ppNode = CreateNode(nLevel, nColorBits, pLeafCount, pReducibleNodes);

	// update color information if it's a leaf node
	if ((*ppNode)->gIsLeaf) {
		(*ppNode)->gPixelCount++;
		(*ppNode)->gRedSum += r;
		(*ppNode)->gGreenSum += g;
		(*ppNode)->gBlueSum += b;
	}
	// recurse a level deeper if the node is not a leaf
	else {
		shift = 7 - nLevel;
		nIndex = (((r & mask[nLevel]) >> shift) << 2) |	(((g & mask[nLevel]) >> shift) << 1) |
					((b & mask[nLevel]) >> shift);
		AddColor(&((*ppNode)->gChildren[nIndex]), r, g, b, nColorBits, nLevel + 1, pLeafCount, pReducibleNodes);
	}
}

static void ReduceTree(GUInt32 nColorBits, GUInt32* pLeafCount, GColorNode** pReducibleNodes) {

	GInt32 i;
	GColorNode* pNode;
	GUInt32 nRedSum, nGreenSum, nBlueSum, nChildren;

	// Find the deepest level containing at least one reducible node
	for (i = nColorBits - 1; (i > 0) && (pReducibleNodes[i] == NULL); i--);

	// Reduce the node most recently added to the list at level i
	pNode = pReducibleNodes[i];
	pReducibleNodes[i] = pNode->gNext;

	nRedSum = nGreenSum = nBlueSum = nChildren = 0;
	for (i = 0; i < 8; i++) {
		if (pNode->gChildren[i] != NULL) {
			nRedSum += pNode->gChildren[i]->gRedSum;
			nGreenSum += pNode->gChildren[i]->gGreenSum;
			nBlueSum += pNode->gChildren[i]->gBlueSum;
			pNode->gPixelCount += pNode->gChildren[i]->gPixelCount;
			delete pNode->gChildren[i];
			pNode->gChildren[i] = NULL;
			nChildren++;
		}
	}
	pNode->gIsLeaf = G_TRUE;
	pNode->gRedSum = nRedSum;
	pNode->gGreenSum = nGreenSum;
	pNode->gBlueSum = nBlueSum;
	*pLeafCount -= (nChildren - 1);
}

void DeleteTree(GColorNode** ppNode) {

	GInt32 i;

	for (i = 0; i < 8; i++) {
		if ((*ppNode)->gChildren[i] != NULL)
			DeleteTree(&((*ppNode)->gChildren[i]));
	}
	delete (*ppNode);
	*ppNode = NULL;
}

static void GetPaletteColors(GColorNode* pTree, GUChar8* pPalEntries, GUInt32* pIndex) {

	GInt32 i;

	if (pTree->gIsLeaf) {
		pPalEntries[(*pIndex) * 4] = (GUChar8)((pTree->gBlueSum) / (pTree->gPixelCount));
		pPalEntries[(*pIndex) * 4 + 1] = (GUChar8)((pTree->gGreenSum) / (pTree->gPixelCount));
		pPalEntries[(*pIndex) * 4 + 2] = (GUChar8)((pTree->gRedSum) / (pTree->gPixelCount));
		pPalEntries[(*pIndex) * 4 + 3] = 0;
		(*pIndex)++;
	}
	else {
		for (i = 0; i < 8; i++) {
			if (pTree->gChildren[i] != NULL)
				GetPaletteColors(pTree->gChildren[i], pPalEntries, pIndex);
		}
	}
}

static void MapColor(GColorNode* pNode, GUChar8 r, GUChar8 g, GUChar8 b, GUInt32 nLevel,
					 GUChar8 *OutR, GUChar8 *OutG, GUChar8 *OutB) {
	
	GInt32 nIndex, shift;
	static GUChar8 mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

	G_ASSERT(pNode != NULL);

	// update color information if it's a leaf node
	if (pNode->gIsLeaf) {
		*OutB = (GUChar8)((pNode->gBlueSum) / (pNode->gPixelCount));
		*OutG = (GUChar8)((pNode->gGreenSum) / (pNode->gPixelCount));
		*OutR = (GUChar8)((pNode->gRedSum) / (pNode->gPixelCount));
	}
	// recurse a level deeper if the node is not a leaf
	else {
		shift = 7 - nLevel;
		nIndex = (((r & mask[nLevel]) >> shift) << 2) |	(((g & mask[nLevel]) >> shift) << 1) |
					((b & mask[nLevel]) >> shift);
		MapColor(pNode->gChildren[nIndex], r, g, b, nLevel + 1, OutR, OutG, OutB);
	}
}

static GError OctreeQuantization(const GPixelMap& Image, const GUInt32 MaxColors, const GUInt32 ColorBits,
								  GUInt32 *Palette, const GUInt32 PalEntries, GUInt32& CreatedEntries,
								  GUChar8 *NewPixels) {

	GInt32 i, j, k;
	GUChar8 *pbBits;
	GUInt16* pwBits;
	GUInt32* pdwBits;
	GUChar8 r, g, b, r2, g2, b2;
	GUInt16 wColor;
	GUInt32 dwColor;
	GColorNode* pTree;
	GUInt32 nLeafCount, nIndex;
	GUInt32 maxCols;
	GColorNode* pReducibleNodes[9];

	// image must not be already paletted
	G_ASSERT(Image.IsPaletted() == G_FALSE);

	// check given parameters
	j = Image.PixelsCount();
	if (!NewPixels || !Palette || ColorBits > 8 || j <= 0)
		return G_INVALID_PARAMETER;

	maxCols = GMath::Min(MaxColors, PalEntries);

	// initialize octree variables
	pTree = NULL;
	nLeafCount = 0;
	for (i = 0; i <= (GInt32)ColorBits; i++)
		pReducibleNodes[i] = NULL;

	switch (Image.PixelFormat()) {
		// just to make gcc happy...
		case G_RGB_PALETTE:
			return G_NO_ERROR;
			break;

		// in this case we can simply generate a standard 256 entries palette
		case G_GRAYSCALE:
			for (i = 0; i < (GInt32)maxCols; i++) {
				dwColor = i | (i << 8) | (i << 16);
				Palette[i] = dwColor;
			}
			pbBits = Image.Pixels();
			for (i = 0; i < j; ++i)
				NewPixels[i] = pbBits[i];
			return G_NO_ERROR;
			break;

		case G_A1R5G5B5:
			pwBits = (GUInt16 *)Image.Pixels();
			for (i = 0; i < j; ++i) {
				wColor = *pwBits++;
				r = (GUChar8)(((wColor >> 10) & 31) << 3);
				g = (GUChar8)(((wColor >> 5) & 31) << 3);
				b = (GUChar8)((wColor & 31) << 3);
				AddColor(&pTree, r, g, b, ColorBits, 0, &nLeafCount, pReducibleNodes);
				while (nLeafCount > maxCols)
					ReduceTree(ColorBits, &nLeafCount, pReducibleNodes);
			}
			break;

		case G_R5G6B5:
			pwBits = (GUInt16 *)Image.Pixels();
			for (i = 0; i < j; ++i) {
				wColor = *pwBits++;
				r = (GUChar8)(((wColor >> 11) & 31) << 3);
				g = (GUChar8)(((wColor >> 5) & 63) << 2);
				b = (GUChar8)((wColor & 31) << 3);
				AddColor(&pTree, r, g, b, ColorBits, 0, &nLeafCount, pReducibleNodes);
				while (nLeafCount > maxCols)
					ReduceTree(ColorBits, &nLeafCount, pReducibleNodes);
			}
			break;

		// full color pixel format
		case G_R8G8B8:
		case G_A8R8G8B8:
			pdwBits = (GUInt32 *)Image.Pixels();
			for (i = 0; i < j; ++i) {
				dwColor = *pdwBits++;
				b = (GUChar8)((dwColor & 0x000000FF));
				g = (GUChar8)((dwColor & 0x0000FF00) >> 8);
				r = (GUChar8)((dwColor & 0x00FF0000) >> 16);
				AddColor(&pTree, r, g, b, ColorBits, 0, &nLeafCount, pReducibleNodes);
				while (nLeafCount > maxCols)
					ReduceTree(ColorBits, &nLeafCount, pReducibleNodes);
			}
			break;
	}
	// sanity check
	if (nLeafCount > maxCols) {
		// free temporary octree
		DeleteTree(&pTree);
		return G_UNKNOWN_ERROR;
	}
	CreatedEntries = nLeafCount;
	// fill palette
	nIndex = 0;
	GetPaletteColors(pTree, (GUChar8 *)Palette, &nIndex);

	// now map each pixel in the new created palette
	j = Image.PixelsCount();
	switch (Image.PixelFormat()) {
		// just to make gcc happy...
		case G_RGB_PALETTE:
		case G_GRAYSCALE:
			break;

		case G_A1R5G5B5:
			pwBits = (GUInt16 *)Image.Pixels();
			for (i = 0; i < j; ++i) {
				wColor = *pwBits++;
				r = (GUChar8)(((wColor >> 10) & 31) << 3);
				g = (GUChar8)(((wColor >> 5) & 31) << 3);
				b = (GUChar8)((wColor & 31) << 3);
				MapColor(pTree, r, g, b, 0, &r2, &g2, &b2);
				dwColor = (r2 << 16) | (g2 << 8) | b2;
				for (k = 0; k < (GInt32)maxCols; ++k) {
					if (Palette[k] == dwColor) {
						NewPixels[i] = (GUChar8)k;
						break;
					}
				}
			}
			break;

		case G_R5G6B5:
			pwBits = (GUInt16 *)Image.Pixels();
			for (i = 0; i < j; ++i) {
				wColor = *pwBits++;
				r = (GUChar8)(((wColor >> 11) & 31) << 3);
				g = (GUChar8)(((wColor >> 5) & 63) << 2);
				b = (GUChar8)((wColor & 31) << 3);
				MapColor(pTree, r, g, b, 0, &r2, &g2, &b2);
				dwColor = (r2 << 16) | (g2 << 8) | b2;
				for (k = 0; k < (GInt32)maxCols; ++k) {
					if (Palette[k] == dwColor) {
						NewPixels[i] = (GUChar8)k;
						break;
					}
				}
			}
			break;

		// full color pixel format
		case G_R8G8B8:
		case G_A8R8G8B8:
			pdwBits = (GUInt32 *)Image.Pixels();
			for (i = 0; i < j; ++i) {
				dwColor = *pdwBits++;
				b = (GUChar8)((dwColor & 0x000000FF));
				g = (GUChar8)((dwColor & 0x0000FF00) >> 8);
				r = (GUChar8)((dwColor & 0x00FF0000) >> 16);
				MapColor(pTree, r, g, b, 0, &r2, &g2, &b2);
				dwColor = (r2 << 16) | (g2 << 8) | b2;
				for (k = 0; k < (GInt32)maxCols; ++k) {
					if (Palette[k] == dwColor) {
						NewPixels[i] = (GUChar8)k;
						break;
					}
				}
			}
			break;
	}
	// free temporary octree
	DeleteTree(&pTree);
	return G_NO_ERROR;
}



// *********************************************************************
//                            GPixelMap
// *********************************************************************

// constructor
GPixelMap::GPixelMap() : GElement() {

	gPixels = NULL;
	gPalette = NULL;
	gPixelFormat = G_A8R8G8B8;
	gWidth = 0;
	gHeight = 0;
}

// constructor
GPixelMap::GPixelMap(const GElement* Owner) : GElement(Owner) {

	gPixels = NULL;
	gPalette = NULL;
	gPixelFormat = G_A8R8G8B8;
	gWidth = 0;
	gHeight = 0;
}

// destructor
GPixelMap::~GPixelMap() {
	Reset();
}

void GPixelMap::Reset() {

	if (gPixels)
		delete [] gPixels;
	if (gPalette)
		delete [] gPalette;
	gPixels = NULL;
	gPalette = NULL;
	gPixelFormat = G_A8R8G8B8;
	gWidth = 0;
	gHeight = 0;
}

GError GPixelMap::Reset(const GInt32 NewWidth, const GInt32 NewHeight, const GPixelFormat NewPixelFormat) {

	GUChar8 *p1 = NULL;
	GUInt32 *p2 = NULL;
	GInt32 palSize, pixelsSize;

	// clear memory / reset image
	if ((NewWidth <= 0) || (NewHeight <= 0)) {
		Reset();
		return G_NO_ERROR;
	}
	// in this case nothing has to be done
	if (((GInt32)gWidth == NewWidth) && ((GInt32)gHeight == NewHeight) && (gPixelFormat == NewPixelFormat))
		return G_NO_ERROR;

	pixelsSize = NeededBytes(NewWidth, NewHeight, NewPixelFormat);
	palSize = PaletteSize(NewPixelFormat);
	// allocate new pixels
	p1 = new(std::nothrow) GUChar8[pixelsSize];
	if (!p1)
		return G_MEMORY_ERROR;
	// allocate new palette
	if (palSize > 0) {
		p2 = new(std::nothrow) GUInt32[palSize];
		if (!p2) {
			delete [] p1;
			return G_MEMORY_ERROR;
		}
	}
	// clear memory
	Reset();
	// assign members
	gWidth = NewWidth;
	gHeight = NewHeight;
	gPixelFormat = NewPixelFormat;
	gPixels = p1;
	gPalette = p2;
	return G_NO_ERROR;
}

// Reset the image, setting new dimensions and pixel format. All pixels will be filled in black.
GError GPixelMap::Create(const GInt32 NewWidth, const GInt32 NewHeight, const GPixelFormat NewPixelFormat) {

	if (NewWidth <= 0 || NewHeight <= 0)
		return G_INVALID_PARAMETER;

	GError err = G_NO_ERROR;
	// in this case nothing has to be done, but clearing all pixels to black
	if (((GInt32)gWidth == NewWidth) && ((GInt32)gHeight == NewHeight) && (gPixelFormat == NewPixelFormat)) {
	}
	else
		err = Reset(NewWidth, NewHeight, NewPixelFormat);

	if (err == G_NO_ERROR) {
		// clear pixels
		std::memset((void *)gPixels, 0, this->Size());
		// clear palette
		if (IsPaletted())
			std::memset((void *)gPalette, 0, this->PaletteSize());
	}
	return err;
}

GError GPixelMap::BaseClone(const GElement& Source) {

	const GPixelMap& k = (const GPixelMap&)Source;
	GError err;

	err = Reset(k.gWidth, k.gHeight, k.gPixelFormat);
	if (err != G_NO_ERROR)
		return err;
	// lets copy pixels
	if ((gPixels) && (k.gPixels))
		std::memcpy(gPixels, k.gPixels, k.Size());
	else
		return G_MEMORY_ERROR;
	// lets copy palette
	if (k.gPalette) {
		if (!gPalette)
			return G_MEMORY_ERROR;
		std::memcpy(gPalette, k.gPalette, PaletteSize(k.gPixelFormat));
	}
	return GElement::BaseClone(Source);
}

// number of bytes occupied by the image data
GInt32 GPixelMap::Size() const {
	return NeededBytes(gWidth, gHeight, gPixelFormat);
}

// number of bytes occupied by the colors palette
GInt32 GPixelMap::PaletteSize() const {
	if (!IsPaletted())
		return 0;
	return PaletteSize(gPixelFormat);
}

// number of pixels
GInt32 GPixelMap::PixelsCount() const {
	return gWidth * gHeight;
}

// number of bits used to encode a single pixel, also called bits per pixel (bpp)
// or bit planes of an image
GInt32 GPixelMap::BitsPerPixel() const {

	switch (gPixelFormat) {
		case G_GRAYSCALE:
		case G_RGB_PALETTE:
			return 8;
		case G_R8G8B8:
		case G_A8R8G8B8:
			return 32;
		case G_A1R5G5B5:
		case G_R5G6B5:
			return 16;
		default:
			return -1;
	}
}

// number of bytes used to encode a single pixel
GInt32 GPixelMap::BytesPerPixel() const {

	switch (gPixelFormat) {
		case G_GRAYSCALE:
		case G_RGB_PALETTE:
			return 1;
		case G_R8G8B8:
		case G_A8R8G8B8:
			return 4;
		case G_A1R5G5B5:
		case G_R5G6B5:
			return 2;
		default:
			return -1;
	}
}

// number of bytes per image scanline
GInt32 GPixelMap::BytesPerLine() const {
	return gWidth * BytesPerPixel();
}


// returns if image has an alpha channel
GBool GPixelMap::HasAlphaChannel() const {

	switch (gPixelFormat) {
		case G_GRAYSCALE:
		case G_R5G6B5:
		case G_RGB_PALETTE:
		case G_R8G8B8:
			return G_FALSE;
		case G_A8R8G8B8:
		case G_A1R5G5B5:
			return G_TRUE;
		default:
			return G_FALSE;
	}
}

// returns if image is paletted (it has a palette associated
GBool GPixelMap::IsPaletted() const {

	if (gPixelFormat == G_RGB_PALETTE)
		return G_TRUE;
	return G_FALSE;
}

// returns if image is gray scale
GBool GPixelMap::IsGrayScale() const {

	if (gPixelFormat == G_GRAYSCALE)
		return G_TRUE;
	return G_FALSE;
}

// returns if image is high color (15 or 16bit)
GBool GPixelMap::IsHighColor() const {

	if ((gPixelFormat == G_A1R5G5B5) || (gPixelFormat == G_R5G6B5))
		return G_TRUE;
	return G_FALSE;
}

// returns if image is truecolor (24 or more)
GBool GPixelMap::IsTrueColor() const {

	if ((gPixelFormat == G_A8R8G8B8) || (gPixelFormat == G_R8G8B8))
		return G_TRUE;
	return G_FALSE;
}

GInt32 GPixelMap::NeededBytes(const GInt32 _Width, const GInt32 _Height, const GPixelFormat PixelFormat) {

	GInt32 i;

	i = _Width * _Height;
	if (i <= 0)
		return -1;
	switch (PixelFormat) {
		case G_GRAYSCALE:
		case G_RGB_PALETTE:
			return i;
		case G_R5G6B5:
		case G_A1R5G5B5:
			return i * 2;
		case G_R8G8B8:
		case G_A8R8G8B8:
			return i * 4;
		default:
			return -1;
	}
}

GInt32 GPixelMap::PaletteSize(const GPixelFormat PixelFormat) {

	if (PixelFormat == G_RGB_PALETTE)
		return 1024; // 256 RGBA color = 1024 bytes
	else
		return 0;
}

void GPixelMap::ReferenceMove(GPixelMap& Source) {

	gWidth = Source.gWidth;
	gHeight = Source.gHeight;
	gPixels = Source.gPixels;
	gPalette = Source.gPalette;
	gPixelFormat = Source.gPixelFormat;

	Source.gWidth = 0;
	Source.gHeight = 0;
	Source.gPixels = NULL;
	Source.gPalette = NULL;
}

// negative image
GError GPixelMap::Negative() {

	if (IsPaletted())
		return G_INVALID_OPERATION;

	GInt32 i, j = PixelsCount();
	GUChar8 *pixels8;
	GUInt32 *pixels32;
	GUInt16 *pixels16;

	if (j <= 0)
		return G_NO_ERROR;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
			// just to make gcc happy with warnings..
			break;
		case G_GRAYSCALE:
			pixels8 = gPixels;
			for (i = 0; i < j; i++)
				*pixels8++ ^= 0xFF;
			break;
		case G_R8G8B8:
		case G_A8R8G8B8:
			pixels32 = (GUInt32 *)gPixels;
			for (i = 0; i < j; i++)
				*pixels32++ ^= 0x00FFFFFF;
			break;
		case G_A1R5G5B5:
			pixels16 = (GUInt16 *)gPixels;
			for (i = 0; i < j; i++)
				*pixels16++ ^= 0x7FFF;
			break;
		case G_R5G6B5:
			pixels16 = (GUInt16 *)gPixels;
			for (i = 0; i < j; i++)
				*pixels16++ ^= 0xFFFF;
			break;
	}
	return G_NO_ERROR;
}

// negative image specifying a destination pixelmap
GError GPixelMap::Negative(GPixelMap& NegativePixelMap) const {

	if (IsPaletted())
		return G_INVALID_OPERATION;

	GInt32 i, j = PixelsCount();
	GUChar8 *pixels8, *pixels8Dst;
	GUInt32 *pixels32, *pixels32Dst;
	GUInt16 *pixels16, *pixels16Dst;
	GError err;

	if (j <= 0)
		return G_NO_ERROR;

	// resize destination image
	err = NegativePixelMap.Reset(gWidth, gHeight, gPixelFormat);
	if (err != G_NO_ERROR)
		return err;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
			// just to make gcc happy with warnings..
			break;
		case G_GRAYSCALE:
			pixels8 = gPixels;
			pixels8Dst = NegativePixelMap.gPixels;
			for (i = 0; i < j; i++)
				*pixels8++ ^= 0xFF;
			break;
		case G_R8G8B8:
		case G_A8R8G8B8:
			pixels32 = (GUInt32 *)gPixels;
			pixels32Dst = (GUInt32 *)NegativePixelMap.gPixels;
			for (i = 0; i < j; i++)
				*pixels32++ ^= 0x00FFFFFF;
			break;
		case G_A1R5G5B5:
			pixels16 = (GUInt16 *)gPixels;
			pixels16Dst = (GUInt16 *)NegativePixelMap.gPixels;
			for (i = 0; i < j; i++)
				*pixels16++ ^= 0x7FFF;
			break;
		case G_R5G6B5:
			pixels16 = (GUInt16 *)gPixels;
			pixels16Dst = (GUInt16 *)NegativePixelMap.gPixels;
			for (i = 0; i < j; i++)
				*pixels16++ ^= 0xFFFF;
			break;
	}
	return G_NO_ERROR;
}

// swap RGBA -> ARGB
GError GPixelMap::ReverseChannels(const GBool ReverseAlphaToo) {

	if (IsGrayScale() || IsPaletted())
		return G_INVALID_OPERATION;

	GInt32 i, j = PixelsCount();
	GUInt32 *pixels32, argb32, a32, r32, g32, b32;
	GUInt16 *pixels16, argb16, a16, r16, g16, b16;

	if (j <= 0)
		return G_NO_ERROR;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
		case G_GRAYSCALE:
			// just to make gcc happy with warnings..
			break;
		case G_A8R8G8B8:
		case G_R8G8B8:
			pixels32 = (GUInt32 *)gPixels;
			if (ReverseAlphaToo) {
				for (i = 0; i < j; i++) {
					argb32 = *pixels32;
					a32 = (argb32 >> 24) & 0xFF;
					r32 = (argb32 >> 16) & 0xFF;
					g32 = (argb32 >> 8) & 0xFF;
					b32 = argb32 & 0xFF;
					argb32 = (b32 << 24) | (g32 << 16) | (r32 << 8) | a32;
					*pixels32++ = argb32;
				}
			}
			else {
				for (i = 0; i < j; i++) {
					argb32 = *pixels32;
					a32 = (argb32 >> 24) & 0xFF;
					r32 = (argb32 >> 16) & 0xFF;
					g32 = (argb32 >> 8) & 0xFF;
					b32 = argb32 & 0xFF;
					argb32 = (a32 << 24) | (b32 << 16) | (g32 << 8) | r32;
					*pixels32++ = argb32;
				}
			}
			break;
		case G_A1R5G5B5:
			pixels16 = (GUInt16 *)gPixels;
			if (ReverseAlphaToo) {
				for (i = 0; i < j; i++) {
					argb16 = *pixels16;
					a16 = (argb16 >> 15) & 1;
					r16 = (argb16 >> 10) & 31;
					g16 = (argb16 >> 5) & 31;
					b16 = argb16 & 31;
					argb16 = (b16 << 11) | (g16 << 6) | (r16 << 1) | a16;
					*pixels16++ = argb16;
				}
			}
			else {
				for (i = 0; i < j; i++) {
					argb16 = *pixels16;
					a16 = (argb16 >> 15) & 1;
					r16 = (argb16 >> 10) & 31;
					g16 = (argb16 >> 5) & 31;
					b16 = argb16 & 31;
					argb16 = (a16 << 15) | (b16 << 10) | (g16 << 5) | r16;
					*pixels16++ = argb16;
				}
			}
			break;

		case G_R5G6B5:
			pixels16 = (GUInt16 *)gPixels;
			for (i = 0; i < j; i++) {
				argb16 = *pixels16;
				r16 = (argb16 >> 11) & 31;
				g16 = (argb16 >> 5) & 63;
				b16 = argb16 & 31;
				argb16 = (b16 << 11) | (g16 << 5) | r16;
				*pixels16++ = argb16;
			}
			break;
	}
	return G_NO_ERROR;
}

// swap RGBA -> ARGB specifying destination
GError GPixelMap::ReverseChannels(GPixelMap& ReversedImage, const GBool ReverseAlphaToo) const {

	if (IsGrayScale() || IsPaletted())
		return G_INVALID_OPERATION;

	GInt32 i, j = PixelsCount();
	GUInt32 *pixels32, argb32, a32, r32, g32, b32, *pixels32Dst;
	GUInt16 *pixels16, argb16, a16, r16, g16, b16, *pixels16Dst;

	if (j <= 0)
		return G_NO_ERROR;

	GError err = ReversedImage.Reset(gWidth, gHeight, gPixelFormat);
	if (err != G_NO_ERROR)
		return err;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
		case G_GRAYSCALE:
			// just to make gcc happy with warnings..
			break;

		case G_A8R8G8B8:
		case G_R8G8B8:
			pixels32 = (GUInt32 *)gPixels;
			pixels32Dst = (GUInt32 *)ReversedImage.gPixels;
			if (ReverseAlphaToo) {
				for (i = 0; i < j; i++) {
					argb32 = *pixels32++;
					a32 = (argb32 >> 24) & 0xFF;
					r32 = (argb32 >> 16) & 0xFF;
					g32 = (argb32 >> 8) & 0xFF;
					b32 = argb32 & 0xFF;
					argb32 = (b32 << 24) | (g32 << 16) | (r32 << 8) | a32;
					*pixels32Dst++ = argb32;
				}
			}
			else {
				for (i = 0; i < j; i++) {
					argb32 = *pixels32++;
					a32 = (argb32 >> 24) & 0xFF;
					r32 = (argb32 >> 16) & 0xFF;
					g32 = (argb32 >> 8) & 0xFF;
					b32 = argb32 & 0xFF;
					argb32 = (a32 << 24) | (b32 << 16) | (g32 << 8) | r32;
					*pixels32Dst++ = argb32;
				}
			}
			break;

		case G_A1R5G5B5:
			pixels16 = (GUInt16 *)gPixels;
			pixels16Dst = (GUInt16 *)ReversedImage.gPixels;
			if (ReverseAlphaToo) {
				for (i = 0; i < j; i++) {
					argb16 = *pixels16++;
					a16 = (argb16 >> 15) & 1;
					r16 = (argb16 >> 10) & 31;
					g16 = (argb16 >> 5) & 31;
					b16 = argb16 & 31;
					argb16 = (b16 << 11) | (g16 << 6) | (r16 << 1) | a16;
					*pixels16Dst++ = argb16;
				}
			}
			else {
				for (i = 0; i < j; i++) {
					argb16 = *pixels16++;
					a16 = (argb16 >> 15) & 1;
					r16 = (argb16 >> 10) & 31;
					g16 = (argb16 >> 5) & 31;
					b16 = argb16 & 31;
					argb16 = (a16 << 15) | (b16 << 10) | (g16 << 5) | r16;
					*pixels16Dst++ = argb16;
				}
			}
			break;
		case G_R5G6B5:
			pixels16 = (GUInt16 *)gPixels;
			pixels16Dst = (GUInt16 *)ReversedImage.gPixels;
			for (i = 0; i < j; i++) {
				argb16 = *pixels16++;
				r16 = (argb16 >> 11) & 31;
				g16 = (argb16 >> 5) & 63;
				b16 = argb16 & 31;
				argb16 = (b16 << 11) | (g16 << 5) | r16;
				*pixels16Dst++ = argb16;
			}
			break;
	}
	return G_NO_ERROR;
}

GError GPixelMap::Resize(const GUInt32 NewWidth, const GUInt32 NewHeight, const GResizeOp ResizeOp) {

	GError err = G_NO_ERROR;
	GInt32 numNewPixels = NewWidth * NewHeight;
	GPixelFormat oldFormat = gPixelFormat;

	// we don't wanna expand to a degenerated image
	if (numNewPixels <= 0)
		return G_INVALID_PARAMETER;

	if (IsPaletted())
		return G_INVALID_OPERATION;

	GPixelMap tmpImage;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
			// just to make gcc happy with warnings..
			break;
		case G_GRAYSCALE:
			err = CopyTo(tmpImage);
			if (err == G_NO_ERROR)
				err = ResizeGray(NewWidth, NewHeight, tmpImage, *this, ResizeOp);
			break;

		case G_A1R5G5B5:
		case G_R5G6B5:
			err = CopyTo(tmpImage);
			if (err == G_NO_ERROR) {
				err = tmpImage.SetPixelFormat(G_A8R8G8B8);
				if (err == G_NO_ERROR) {
					err = ResizeRGB(NewWidth, NewHeight, tmpImage, *this, ResizeOp);
					if (err == G_NO_ERROR)
						err = SetPixelFormat(oldFormat);
				}
			}
			break;

		case G_A8R8G8B8:
		case G_R8G8B8:
			err = CopyTo(tmpImage);
			if (err == G_NO_ERROR) {
				err = ResizeRGB(NewWidth, NewHeight, tmpImage, *this, ResizeOp);
				if (err == G_NO_ERROR)
					gPixelFormat = oldFormat;
			}
			break;
	}
	return err;
}

GError GPixelMap::Resize(const GUInt32 NewWidth, const GUInt32 NewHeight, GPixelMap& ResizedPixelMap,
						 const GResizeOp ResizeOp) {

	GError err = G_NO_ERROR;
	GInt32 numNewPixels = NewWidth * NewHeight;
	GPixelFormat oldFormat = gPixelFormat;

	// we don't wanna expand to a degenerated image
	if (numNewPixels <= 0)
		return G_INVALID_PARAMETER;

	if (IsPaletted())
		return G_INVALID_OPERATION;

	GPixelMap tmpImage;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
			// just to make gcc happy with warnings..
			break;
		case G_GRAYSCALE:
			err = CopyTo(tmpImage);
			if (err == G_NO_ERROR)
				err = ResizeGray(NewWidth, NewHeight, *this, ResizedPixelMap, ResizeOp);
			break;

		case G_A1R5G5B5:
		case G_R5G6B5:
			err = CopyTo(tmpImage);
			if (err == G_NO_ERROR) {
				err = tmpImage.SetPixelFormat(G_A8R8G8B8);
				if (err == G_NO_ERROR) {
					err = ResizeRGB(NewWidth, NewHeight, tmpImage, ResizedPixelMap, ResizeOp);
					if (err == G_NO_ERROR)
						err = ResizedPixelMap.SetPixelFormat(oldFormat);
				}
			}
			break;

		case G_A8R8G8B8:
		case G_R8G8B8:
			err = CopyTo(tmpImage);
			if (err == G_NO_ERROR) {
				err = ResizeRGB(NewWidth, NewHeight, *this, ResizedPixelMap, ResizeOp);
				if (err == G_NO_ERROR)
					ResizedPixelMap.gPixelFormat = oldFormat;
			}
			break;
	}
	return err;
}

struct Filter1D {
	GFloat (*FilterFunction)(const GFloat);
	GFloat Support;
};

static Filter1D FilterTable[] = {
	{ BoxFilter, 0.5f },
	{ TriangleFilter, 1.0f },
	{ QuadraticFilter, 1.5f },
	{ CubicFilter, 2.0f },
	{ CatmullRomFilter, 2.0f },
	{ GaussianFilter, 1.25f },
	{ SincFilter, 4.0f },
	{ BesselFilter, 3.2383f },
	{ BellFilter, 1.5f },
	{ HanningFilter, 1.0f },
	{ HammingFilter, 1.0f },
	{ BlackmanFilter, 1.0f },
	{ NormalFilter, 2.50f }
};

struct PixelContribute {
	GInt32 PixelIndex;
	GFloat Weight;
	PixelContribute() : PixelIndex(0), Weight(0.0f) {
	}
	PixelContribute(const GInt32 _PixelIndex, const GFloat _Weight) : PixelIndex(_PixelIndex), Weight(_Weight) {
	}
};

struct PixelWeightsTable {
	GDynArray<PixelContribute> Contributes;
	GFloat InvSumWeights;
};

GError GPixelMap::ResizeGray(const GUInt32 NewWidth, const GUInt32 NewHeight, const GPixelMap& GraySource,
							 GPixelMap& ResizedPixelMap, const GResizeOp ResizeOp) {

	G_ASSERT(GraySource.gPixelFormat == G_GRAYSCALE);
	G_ASSERT(&GraySource != &ResizedPixelMap);

	if (NewWidth == 0 || NewHeight == 0)
		return G_INVALID_PARAMETER;

	GFloat (*filter_function)(const GFloat);
	GFloat support;

	// Pick a filter function & support.
	filter_function = FilterTable[ResizeOp].FilterFunction;
	support = FilterTable[ResizeOp].Support;

	GFloat xscale, yscale;		// zoom scale factors
	GInt32 i, j, k, w;			// loop variables
	GInt32 n;				// pixel number
	GFloat center;
	GInt32 left, right;	// filter calculation variables
	GFloat width, fscale, weight;	// filter calculation variables
	GFloat sumWeights;
	GFloat in_x0, in_x1, in_y0, in_y1;
	GInt32 out_x0, out_x1, out_y0, out_y1;
	GError err;
	GDynArray<GUChar8> raster;
	GDynArray<PixelWeightsTable> contrib;

	in_x0 = 0;
	in_x1 = (GFloat)GraySource.gWidth - 1;
	in_y0 = 0;
	in_y1 = (GFloat)GraySource.gHeight - 1;

	out_x0 = 0;
	out_x1 = NewWidth - 1;
	out_y0 = 0;
	out_y1 = NewHeight - 1;


	GInt32 out_width = out_x1 - out_x0 + 1;
	GInt32 out_height = out_y1 - out_y0 + 1;

	GFloat in_width = in_x1 - in_x0;
	GFloat in_height = in_y1 - in_y0;

	GInt32 in_window_w = GInt32(GMath::Ceil(in_x1) - GMath::Floor(in_x0) + 1);
	GInt32 in_window_h = GInt32(GMath::Ceil(in_y1) - GMath::Floor(in_y0) + 1);

	// create intermediate image to hold horizontal zoom
	GPixelMap tmp;

	err = tmp.Reset(out_width, in_window_h, GraySource.PixelFormat());
	if (err != G_NO_ERROR)
		return err;

	xscale = (GFloat)(out_width) / (in_width + 1);
	yscale = (GFloat)(out_height) / (in_height + 1);

	// protect against division by 0
	if (yscale == 0)
		yscale = 1.0f;
	if (xscale == 0)
		xscale = 1.0f;

	// pre-calculate filter contributions for a row
	contrib.resize(tmp.gWidth);
	if (xscale < 1.0f) {
		width = support / xscale;
		fscale = 1.0f / xscale;
		for (i = 0; i < (GInt32)tmp.gWidth; ++i) {
			contrib[i].Contributes.clear();
			contrib[i].InvSumWeights = 0;
			center = (GFloat)i / xscale;
			left = GInt32(GMath::Ceil(center - width));
			right = GInt32(GMath::Floor(center + width));
			for (j = left; j <= right; ++j) {
				weight = center - (GFloat)j;
				weight = (*filter_function)(weight / fscale);
				n = GMath::Clamp(j, 0, in_window_w - 1);
				contrib[i].Contributes.push_back(PixelContribute(n, weight));
				contrib[i].InvSumWeights += weight;
			}
			contrib[i].InvSumWeights = 1.0f / contrib[i].InvSumWeights;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; j++)
				contrib[i].Contributes[j].Weight *= contrib[i].InvSumWeights;
		}
	}
	else {
		for (i = 0; i < (GInt32)tmp.gWidth; ++i) {
			contrib[i].Contributes.clear();
			contrib[i].InvSumWeights = 0;
			center = (GFloat)i / xscale;
			left = GInt32(GMath::Ceil(center - support));
			right = GInt32(GMath::Floor(center + support));
			for (j = left; j <= right; ++j) {
				weight = center - (GFloat) j;
				weight = (*filter_function)(weight);
				n = GMath::Clamp(j, 0, in_window_w - 1);
				contrib[i].Contributes.push_back(PixelContribute(n, weight));
				contrib[i].InvSumWeights += weight;
			}
			contrib[i].InvSumWeights = 1.0f / contrib[i].InvSumWeights;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; j++)
				contrib[i].Contributes[j].Weight *= contrib[i].InvSumWeights;
		}
	}

	// apply filter to zoom horizontally from src to tmp
	for (k = 0; k < (GInt32)tmp.gHeight; ++k) {
		GraySource.ExtractRow(k, GInt32(GMath::Floor(in_x0)), GInt32(GMath::Floor(in_x0)) + in_window_w - 1, raster);
		for (i = 0; i < (GInt32)tmp.gWidth; ++i) {
			GFloat gray = 0.0f;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; ++j) {
				GInt32 pixel = contrib[i].Contributes[j].PixelIndex;
				gray += raster[pixel] * contrib[i].Contributes[j].Weight;
			}
			sumWeights = contrib[i].InvSumWeights;
			G_ASSERT(sumWeights > 0);

			gray = GMath::Clamp(gray, 0.0f, 255.0f);
			tmp.SetPixel(i, k, (GUChar8)gray);
		}
	}

	contrib.resize(out_height);

	if (yscale < 1.0f) {
		width = support / yscale;
		fscale = 1.0f / yscale;
		for (i = 0; i < out_height; ++i) {
			contrib[i].Contributes.clear();
			contrib[i].InvSumWeights = 0;
			center = (GFloat)i / yscale;
			left = GInt32(GMath::Ceil(center - width));
			right = GInt32(GMath::Floor(center + width));
			for (j = left; j <= right; ++j) {
				weight = center - (GFloat)j;
				weight = (*filter_function)(weight / fscale);
				n = GMath::Clamp(j, 0, (GInt32)tmp.gHeight - 1);
				contrib[i].Contributes.push_back(PixelContribute(n, weight));
				contrib[i].InvSumWeights += weight;
			}
			contrib[i].InvSumWeights = 1.0f / contrib[i].InvSumWeights;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; j++)
				contrib[i].Contributes[j].Weight *= contrib[i].InvSumWeights;
		}
	}
	else {
		for (i = 0; i < out_height; ++i) {
			contrib[i].Contributes.clear();
			contrib[i].InvSumWeights = 0;
			center = (GFloat)i / yscale;
			left = GInt32(GMath::Ceil(center - support));
			right = GInt32(GMath::Floor(center + support));
			for (j = left; j <= right; ++j) {
				weight = center - (GFloat)j;
				weight = (*filter_function)(weight);
				n = GMath::Clamp(j, 0, (GInt32)tmp.gHeight - 1);
				contrib[i].Contributes.push_back(PixelContribute(n, weight));
				contrib[i].InvSumWeights += weight;
			}
			contrib[i].InvSumWeights = 1.0f / contrib[i].InvSumWeights;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; j++)
				contrib[i].Contributes[j].Weight *= contrib[i].InvSumWeights;
		}
	}

	// apply filter to zoom vertically from tmp to dst
	err = ResizedPixelMap.Reset(NewWidth, NewHeight, G_GRAYSCALE);
	if (err != G_NO_ERROR)
		return err;

	for (k = 0; k < (GInt32)tmp.gWidth; ++k) {
		tmp.ExtractColumn(k, 0, tmp.gHeight - 1, raster);
		for (i = 0; i < out_height; ++i) {
			GFloat gray = 0.0f;

			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; ++j) {
				GInt32 pixel = contrib[i].Contributes[j].PixelIndex;
				gray += raster[pixel] * contrib[i].Contributes[j].Weight;
			}
			sumWeights = contrib[i].InvSumWeights;
			G_ASSERT(sumWeights > 0);

			gray = GMath::Clamp(gray, 0.0f, 255.0f);
			ResizedPixelMap.SetPixel(k + out_x0, i + out_y0, (GUChar8)gray);
		}
	}
	return G_NO_ERROR;
}


GError GPixelMap::ResizeRGB(const GUInt32 NewWidth, const GUInt32 NewHeight, const GPixelMap& FullColorSource,
							GPixelMap& ResizedPixelMap,	const GResizeOp ResizeOp) {

	G_ASSERT(FullColorSource.gPixelFormat == G_A8R8G8B8 || FullColorSource.gPixelFormat == G_R8G8B8);
	G_ASSERT(&FullColorSource != &ResizedPixelMap);
	if (NewWidth == 0 || NewHeight == 0)
		return G_INVALID_PARAMETER;

	GFloat (*filter_function)(const GFloat);
	GFloat support;

	// Pick a filter function & support.
	filter_function = FilterTable[ResizeOp].FilterFunction;
	support = FilterTable[ResizeOp].Support;

	GFloat xscale, yscale;		// zoom scale factors
	GInt32 i, j, k, w;			// loop variables
	GInt32 n;				// pixel number
	GFloat center;
	GInt32 left, right;	// filter calculation variables
	GFloat width, fscale, weight;	// filter calculation variables
	GFloat sumWeights;
	GFloat in_x0, in_x1, in_y0, in_y1;
	GInt32 out_x0, out_x1, out_y0, out_y1;
	GError err;
	GDynArray<GUChar8> raster;
	GDynArray<PixelWeightsTable> contrib;

	in_x0 = 0;
	in_x1 = (GFloat)FullColorSource.gWidth - 1;
	in_y0 = 0;
	in_y1 = (GFloat)FullColorSource.gHeight - 1;

	out_x0 = 0;
	out_x1 = NewWidth - 1;
	out_y0 = 0;
	out_y1 = NewHeight - 1;


	GInt32 out_width = out_x1 - out_x0 + 1;
	GInt32 out_height = out_y1 - out_y0 + 1;

	GFloat in_width = in_x1 - in_x0;
	GFloat in_height = in_y1 - in_y0;

	GInt32 in_window_w = GInt32(GMath::Ceil(in_x1) - GMath::Floor(in_x0) + 1);
	GInt32 in_window_h = GInt32(GMath::Ceil(in_y1) - GMath::Floor(in_y0) + 1);

	// create intermediate image to hold horizontal zoom
	GPixelMap tmp;

	err = tmp.Reset(out_width, in_window_h, FullColorSource.PixelFormat());
	if (err != G_NO_ERROR)
		return err;

	xscale = (GFloat)(out_width) / (in_width + 1);
	yscale = (GFloat)(out_height) / (in_height + 1);

	// protect against division by 0
	if (yscale == 0)
		yscale = 1.0f;
	if (xscale == 0)
		xscale = 1.0f;

	// pre-calculate filter contributions for a row
	contrib.resize(tmp.gWidth);
	if (xscale < 1.0f) {
		width = support / xscale;
		fscale = 1.0f / xscale;
		for (i = 0; i < (GInt32)tmp.gWidth; ++i) {
			contrib[i].Contributes.clear();
			contrib[i].InvSumWeights = 0;
			center = (GFloat)i / xscale;
			left = GInt32(GMath::Ceil(center - width));
			right = GInt32(GMath::Floor(center + width));
			for (j = left; j <= right; ++j) {
				weight = center - (GFloat)j;
				weight = (*filter_function)(weight / fscale);
				n = GMath::Clamp(j, 0, in_window_w - 1);
				contrib[i].Contributes.push_back(PixelContribute(n, weight));
				contrib[i].InvSumWeights += weight;
			}
			contrib[i].InvSumWeights = 1.0f / contrib[i].InvSumWeights;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; j++)
				contrib[i].Contributes[j].Weight *= contrib[i].InvSumWeights;
		}
	}
	else {
		for (i = 0; i < (GInt32)tmp.gWidth; ++i) {
			contrib[i].Contributes.clear();
			contrib[i].InvSumWeights = 0;
			center = (GFloat)i / xscale;
			left = GInt32(GMath::Ceil(center - support));
			right = GInt32(GMath::Floor(center + support));
			for (j = left; j <= right; ++j) {
				weight = center - (GFloat) j;
				weight = (*filter_function)(weight);
				n = GMath::Clamp(j, 0, in_window_w - 1);
				contrib[i].Contributes.push_back(PixelContribute(n, weight));
				contrib[i].InvSumWeights += weight;
			}
			contrib[i].InvSumWeights = 1.0f / contrib[i].InvSumWeights;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; j++)
				contrib[i].Contributes[j].Weight *= contrib[i].InvSumWeights;
		}
	}

	// apply filter to zoom horizontally from src to tmp
	for (k = 0; k < (GInt32)tmp.gHeight; ++k) {
		FullColorSource.ExtractRow(k, GInt32(GMath::Floor(in_x0)), GInt32(GMath::Floor(in_x0)) + in_window_w - 1, raster);
		for (i = 0; i < (GInt32)tmp.gWidth; ++i) {
			GFloat alpha = 0.0f;
			GFloat red = 0.0f;
			GFloat green = 0.0f;
			GFloat blue = 0.0f;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; ++j) {
				GInt32 pixel = contrib[i].Contributes[j].PixelIndex;
				blue += raster[pixel * 4 + 0] * contrib[i].Contributes[j].Weight;
				green += raster[pixel * 4 + 1] * contrib[i].Contributes[j].Weight;
				red += raster[pixel * 4 + 2] * contrib[i].Contributes[j].Weight;
				alpha += raster[pixel * 4 + 3] * contrib[i].Contributes[j].Weight;
			}
			sumWeights = contrib[i].InvSumWeights;
			G_ASSERT(sumWeights > 0);

			red = GMath::Clamp(red, 0.0f, 255.0f);
			green = GMath::Clamp(green, 0.0f, 255.0f);
			blue = GMath::Clamp(blue, 0.0f, 255.0f);
			alpha = GMath::Clamp(alpha, 0.0f, 255.0f);

			GUInt32 r32 = ((GUInt32)red);
			GUInt32 g32 = ((GUInt32)green);
			GUInt32 b32 = ((GUInt32)blue);
			GUInt32 a32 = ((GUInt32)alpha);
			tmp.SetPixel(i, k, (a32 << 24) | (r32 << 16) | (g32 << 8) | b32);
		}
	}

	contrib.resize(out_height);

	if (yscale < 1.0f) {
		width = support / yscale;
		fscale = 1.0f / yscale;
		for (i = 0; i < out_height; ++i) {
			contrib[i].Contributes.clear();
			contrib[i].InvSumWeights = 0;
			center = (GFloat)i / yscale;
			left = GInt32(GMath::Ceil(center - width));
			right = GInt32(GMath::Floor(center + width));
			for (j = left; j <= right; ++j) {
				weight = center - (GFloat)j;
				weight = (*filter_function)(weight / fscale);
				n = GMath::Clamp(j, 0, (GInt32)tmp.gHeight - 1);
				contrib[i].Contributes.push_back(PixelContribute(n, weight));
				contrib[i].InvSumWeights += weight;
			}
			contrib[i].InvSumWeights = 1.0f / contrib[i].InvSumWeights;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; j++)
				contrib[i].Contributes[j].Weight *= contrib[i].InvSumWeights;
		}
	}
	else {
		for (i = 0; i < out_height; ++i) {
			contrib[i].Contributes.clear();
			contrib[i].InvSumWeights = 0;
			center = (GFloat)i / yscale;
			left = GInt32(GMath::Ceil(center - support));
			right = GInt32(GMath::Floor(center + support));
			for (j = left; j <= right; ++j) {
				weight = center - (GFloat)j;
				weight = (*filter_function)(weight);
				n = GMath::Clamp(j, 0, (GInt32)tmp.gHeight - 1);
				contrib[i].Contributes.push_back(PixelContribute(n, weight));
				contrib[i].InvSumWeights += weight;
			}
			contrib[i].InvSumWeights = 1.0f / contrib[i].InvSumWeights;
			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; j++)
				contrib[i].Contributes[j].Weight *= contrib[i].InvSumWeights;
		}
	}

	// apply filter to zoom vertically from tmp to dst
	err = ResizedPixelMap.Reset(NewWidth, NewHeight, G_A8R8G8B8);
	if (err != G_NO_ERROR)
		return err;

	for (k = 0; k < (GInt32)tmp.gWidth; ++k) {
		tmp.ExtractColumn(k, 0, tmp.gHeight - 1, raster);
		for (i = 0; i < out_height; ++i) {
			GFloat red = 0.0f;
			GFloat green = 0.0f;
			GFloat blue = 0.0f;
			GFloat alpha = 0.0f;

			w = (GInt32)contrib[i].Contributes.size();
			for (j = 0; j < w; ++j) {
				GInt32 pixel = contrib[i].Contributes[j].PixelIndex;
				blue += raster[pixel * 4 + 0] * contrib[i].Contributes[j].Weight;
				green += raster[pixel * 4 + 1] * contrib[i].Contributes[j].Weight;
				red += raster[pixel * 4 + 2] * contrib[i].Contributes[j].Weight;
				alpha += raster[pixel * 4 + 3] * contrib[i].Contributes[j].Weight;
			}
			sumWeights = contrib[i].InvSumWeights;
			G_ASSERT(sumWeights > 0);

			red = GMath::Clamp(red, 0.0f, 255.0f);
			green = GMath::Clamp(green, 0.0f, 255.0f);
			blue = GMath::Clamp(blue, 0.0f, 255.0f);
			alpha = GMath::Clamp(alpha, 0.0f, 255.0f);

			GUInt32 r32 = ((GUInt32)red);
			GUInt32 g32 = ((GUInt32)green);
			GUInt32 b32 = ((GUInt32)blue);
			GUInt32 a32 = ((GUInt32)alpha);
			ResizedPixelMap.SetPixel(k + out_x0, i + out_y0, (a32 << 24) | (r32 << 16) | (g32 << 8) | b32);
		}
	}
	return G_NO_ERROR;
}

void GPixelMap::Clear(const GUInt32& Index_Or_A8R8G8B8) {

	GUInt32 gray, i, j, *pixels32;
	GUInt16 rgb16, *pixels16, r16, b16, g16, a16;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
		case G_GRAYSCALE:
			gray = Index_Or_A8R8G8B8;
			std::memset(gPixels, gray, PixelsCount());
			break;

		case G_R5G6B5:
			r16 = (GUInt16)(Index_Or_A8R8G8B8 >> 19) & 0x1F;
			g16 = (GUInt16)(Index_Or_A8R8G8B8 >> 10) & 0x3F;
			b16 = (GUInt16)(Index_Or_A8R8G8B8 >> 3) & 0x1F;
			rgb16 = b16 | (g16 << 5) | (r16 << 11);
			pixels16 = (GUInt16 *)gPixels;
			j = PixelsCount();
			for (i = 0; i < j; i++) {
				*pixels16 = rgb16;
				pixels16++;
			}
			break;

		case G_A1R5G5B5:
			r16 = (GUInt16)(Index_Or_A8R8G8B8 >> 19) & 0x1F;
			g16 = (GUInt16)(Index_Or_A8R8G8B8 >> 11) & 0x1F;
			b16 = (GUInt16)(Index_Or_A8R8G8B8 >> 3) & 0x1F;
			a16 = (GUInt16)(Index_Or_A8R8G8B8 >> 31) & 0x01;
			rgb16 = b16 | (g16 << 5) | (r16 << 10) | (a16 << 15);
			pixels16 = (GUInt16 *)gPixels;
			j = PixelsCount();
			for (i = 0; i < j; i++) {
				*pixels16 = rgb16;
				pixels16++;
			}
			break;

		case G_R8G8B8:
		case G_A8R8G8B8:
			pixels32 = (GUInt32 *)gPixels;
			j = PixelsCount();
			for (i = 0; i < j; i++) {
				*pixels32 = Index_Or_A8R8G8B8;
				pixels32++;
			}
			break;
	}
}

GError GPixelMap::ResizeCanvas(const GInt32 Top, const GInt32 Bottom, const GInt32 Left, const GInt32 Right,
							   const GUInt32 Index_Or_A8R8G8B8) {

	GError err;
	GPixelMap tmpImage;

	err = ResizeCanvas(Top, Bottom, Left, Right, tmpImage, Index_Or_A8R8G8B8);
	if (err == G_NO_ERROR) {
		Reset();
		ReferenceMove(tmpImage);
	}
	return err;
}

GError GPixelMap::ResizeCanvas(const GInt32 Top, const GInt32 Bottom, const GInt32 Left, const GInt32 Right,
							GPixelMap& ResizedPixelMap, const GUInt32 Index_Or_A8R8G8B8) const {

	GInt32 newWidth, newHeight, i, lineBytes, rowsCount, colsCount;
	GInt32 xSrc, ySrc, ofsSrc, xDst, yDst, ofsDst;
	GChar8 *pixelSrc, *pixelDst;
	void *src, *dst;
	GError err;

	// calculate new dimensions and return an error if negative
	newWidth = gWidth + Left + Right;
	newHeight = gHeight + Top + Bottom;
	if ((newWidth < 0) || (newHeight < 0))
		return G_INVALID_PARAMETER;
	// resize output image
	err = ResizedPixelMap.Reset(newWidth, newHeight, gPixelFormat);
	if (err != G_NO_ERROR)
		return err;
	
	if ((Top > 0) || (Bottom > 0) || (Left > 0) || (Right > 0))
		ResizedPixelMap.Clear(Index_Or_A8R8G8B8);

	// locate the first point in the source image where we have to start to read pixels
	xSrc = GMath::Max(0, -Left);
	ySrc = GMath::Max(0, -Top);
	ofsSrc = (ySrc * gWidth + xSrc) * BytesPerPixel();
	pixelSrc = (GChar8 *)gPixels;
	// locate the first point in the destination image where we have to start to write pixels
	xDst = GMath::Max(0, Left);
	yDst = GMath::Max(0, Top);
	ofsDst = (yDst * ResizedPixelMap.gWidth + xDst) * ResizedPixelMap.BytesPerPixel();
	pixelDst = (GChar8 *)ResizedPixelMap.gPixels;
	rowsCount = gHeight;
	if (Top < 0)
		rowsCount += Top;
	if (Bottom < 0)
		rowsCount += Bottom;
	colsCount = gWidth;
	if (Left < 0)
		colsCount += Left;
	if (Right < 0)
		colsCount += Right;
	lineBytes = colsCount * BytesPerPixel();
	for (i = 0; i < rowsCount; i++) {
		src = (void *)(&pixelSrc[ofsSrc]);
		dst = (void *)(&pixelDst[ofsDst]);
		std::memcpy(dst, src, lineBytes);
		ofsSrc += BytesPerLine();
		ofsDst += ResizedPixelMap.BytesPerLine();
	}
	return G_NO_ERROR;
}

GError GPixelMap::ResizeCanvasMirror(const GInt32 Top, const GInt32 Bottom,
									 const GInt32 Left, const GInt32 Right) {

	GError err;
	GPixelMap tmpImage;

	err = ResizeCanvasMirror(Top, Bottom, Left, Right, tmpImage);
	if (err == G_NO_ERROR) {
		Reset();
		ReferenceMove(tmpImage);
	}
	return err;
}

GError GPixelMap::ResizeCanvasMirror(const GInt32 Top, const GInt32 Bottom, const GInt32 Left, const GInt32 Right,
									GPixelMap& ResizedPixelMap) {

	GInt32 newWidth, newHeight, i, ii, iii, j, k, l, lineBytes, rowsCount, colsCount;
	GInt32 xSrc, ySrc, ofsSrc, xDst, yDst, ofsDst;
	GChar8 *pixelSrc, *pixelDst, *tmpSrc, *tmpDst;
	void *src, *dst;
	GError err;

	// calculate new dimensions and return an error if negative
	newWidth = gWidth + Left + Right;
	newHeight = gHeight + Top + Bottom;
	if ((newWidth < 0) || (newHeight < 0))
		return G_INVALID_PARAMETER;
	// resize output image
	err = ResizedPixelMap.Reset(newWidth, newHeight, gPixelFormat);
	if (err != G_NO_ERROR)
		return err;
	
	// locate the first point in the source image where we have to start to read pixels
	xSrc = GMath::Max(0, -Left);
	ySrc = GMath::Max(0, -Top);
	ofsSrc = (ySrc * gWidth + xSrc) * BytesPerPixel();
	pixelSrc = (GChar8 *)gPixels;
	// locate the first point in the destination image where we have to start to write pixels
	xDst = GMath::Max(0, Left);
	yDst = GMath::Max(0, Top);
	ofsDst = (yDst * ResizedPixelMap.gWidth + xDst) * ResizedPixelMap.BytesPerPixel();
	pixelDst = (GChar8 *)ResizedPixelMap.gPixels;
	rowsCount = gHeight;
	if (Top < 0)
		rowsCount += Top;
	if (Bottom < 0)
		rowsCount += Bottom;
	colsCount = gWidth;
	if (Left < 0)
		colsCount += Left;
	if (Right < 0)
		colsCount += Right;
	lineBytes = colsCount * BytesPerPixel();
	// copy central part
	for (i = 0; i < rowsCount; i++) {
		src = (void *)(&pixelSrc[ofsSrc]);
		dst = (void *)(&pixelDst[ofsDst]);
		std::memcpy(dst, src, lineBytes);
		ofsSrc += BytesPerLine();
		ofsDst += ResizedPixelMap.BytesPerLine();
	}

	// now copy mirrored border, if any
	if (Top > 0) {
		for (i = 0; i < Top; i++) {
			// in the case where source image is 'k' pixel height and requested Top border
			// is greater that 'k'
			ii = (i % gHeight);

			GUInt32 flip = i / gHeight;
			if ((flip & 1) == 0)
				j = ii;
			else
				j = gHeight - ii - 1;

			tmpSrc = pixelSrc + j * gWidth * BytesPerPixel();

			j = Top - 1 - i;
			tmpDst = pixelDst + (j * newWidth + xDst) * ResizedPixelMap.BytesPerPixel();

			std::memcpy(tmpDst, tmpSrc, lineBytes);
		}
	}
	if (Bottom > 0) {
		for (i = 0; i < Bottom; i++) {
			// in the case where source image is 'k' pixel height and requested Top border
			// is greater that 'k'
			ii = i % gHeight;
			// source offset must care of "cycling"
			GUInt32 flip = i / gHeight;
			if ((flip & 1) == 0)
				j = gHeight - ii - 1;
			else
				j = ii;
			tmpSrc = pixelSrc + j * gWidth * BytesPerPixel();

			j = newHeight - Bottom + i;
			tmpDst = pixelDst + (j * newWidth + xDst) * ResizedPixelMap.BytesPerPixel();
			std::memcpy(tmpDst, tmpSrc, lineBytes);
		}
	}
	if (Right > 0) {
		for (i = 0; i < Right; i++) {
			// source offset must care of "cycling"
			for (j = 0; j < newHeight; j++) {
				ii = ((j + 1) * newWidth - Right + i) * ResizedPixelMap.BytesPerPixel();
				iii = (j + 1) * newWidth - Right;
				
				// source offset must care of "cycling"
				GUInt32 flip = i / gWidth;
				if ((flip & 1) == 0)
					iii = iii - (i % gWidth) - 1;
				else
					iii = iii - gWidth + (i % gWidth);

				k = iii * ResizedPixelMap.BytesPerPixel();
				for (l = 0; l < ResizedPixelMap.BytesPerPixel(); l++)
					pixelDst[ii + l] = pixelDst[k + l];
			}
		}
	}
	if (Left > 0) {
		for (i = 0; i < Left; i++) {
			for (j = 0; j < newHeight; j++) {
				ii = (j * newWidth + Left - i - 1) * ResizedPixelMap.BytesPerPixel();

				iii = j * newWidth + Left;
				// source offset must care of "cycling"
				GUInt32 flip = i / gWidth;
				if ((flip & 1) == 0)
					iii = iii + (i % gWidth);
				else
					iii = iii + gWidth - (i % gWidth) - 1;

				k = iii * ResizedPixelMap.BytesPerPixel();

				for (l = 0; l < ResizedPixelMap.BytesPerPixel(); l++)
					pixelDst[ii + l] = pixelDst[k + l];
			}
		}
	}
	return G_NO_ERROR;
}

void GPixelMap::ExtractColumn(const GUInt32 Index, const GUInt32 StartY, const GUInt32 EndY,
							  GDynArray<GUChar8>& Pixels) const {


	G_ASSERT(Index < gWidth);
	G_ASSERT(StartY <= EndY);
	G_ASSERT(EndY < gHeight);

	Pixels.resize((EndY - StartY + 1) * BytesPerPixel());

	GUChar8 *p = gPixels;
	GUInt32 i, ofs, j, ofsDst;

	ofs = StartY * BytesPerLine() + Index * BytesPerPixel();
	ofsDst = 0;
	for (i = StartY; i <= EndY; i++) {
		for (j = 0; j < (GUInt32)BytesPerPixel(); j++)
			Pixels[ofsDst++] = p[ofs + j];
		ofs += BytesPerLine();
	}
}

void GPixelMap::ExtractRow(const GUInt32 Index, const GUInt32 StartX, const GUInt32 EndX,
						   GDynArray<GUChar8>& Pixels) const {

	GInt32 extraPixels = EndX - gWidth + 1;
	if (extraPixels > 0) {
		G_ASSERT(0 == 1);
	}
	G_ASSERT(Index < gHeight);
	G_ASSERT(StartX <= EndX);
	G_ASSERT(EndX < gWidth);

	Pixels.resize((EndX - StartX + 1) * BytesPerPixel());

	GUChar8 *p = gPixels;
	GUInt32 ofs = Index * BytesPerLine() + StartX * BytesPerPixel();

	std::memcpy(&Pixels[0], p + ofs, BytesPerLine());
}

// flip image
GError GPixelMap::Flip(const GBool Horizontal, const GBool Vertical) {

	if (!Horizontal && !Vertical)
		return G_NO_ERROR;

	GUInt32 i, j, k;
	GUInt8 *pixel8Src, *pixel8Dest, swap8;
	GUInt16 *pixel16Src, *pixel16Dest, swap16;
	GUInt32 *pixel32Src, *pixel32Dest, swap32;

	j = gWidth / 2;
	if (Horizontal) {
		switch (gPixelFormat) {
			case G_GRAYSCALE:
			case G_RGB_PALETTE:
				for (i = 0; i < j; i++) {	
					pixel8Src = (GUInt8 *)gPixels + i;
					pixel8Dest = (GUInt8 *)gPixels + (gWidth - i - 1);
					for (k = 0; k < gHeight; k++) {
						swap8 = *pixel8Src;
						*pixel8Src = *pixel8Dest;
						*pixel8Dest = swap8;
						pixel8Src += gWidth;
						pixel8Dest += gWidth;
					}
				}
				break;
			case G_R8G8B8:
			case G_A8R8G8B8:
				for (i = 0; i < j; i++) {	
					pixel32Src = (GUInt32 *)gPixels + i;
					pixel32Dest = (GUInt32 *)gPixels + (gWidth - i - 1);
					for (k = 0; k < gHeight; k++) {
						swap32 = *pixel32Src;
						*pixel32Src = *pixel32Dest;
						*pixel32Dest = swap32;
						pixel32Src += gWidth;
						pixel32Dest += gWidth;
					}
				}
				break;
			case G_A1R5G5B5:
			case G_R5G6B5:
				for (i = 0; i < j; i++) {	
					pixel16Src = (GUInt16 *)gPixels + i;
					pixel16Dest = (GUInt16 *)gPixels + (gWidth - i - 1);
					for (k = 0; k < gHeight; k++) {
						swap16 = *pixel16Src;
						*pixel16Src = *pixel16Dest;
						*pixel16Dest = swap16;
						pixel16Src += gWidth;
						pixel16Dest += gWidth;
					}
				}
				break;
		}
	}

	j = gHeight / 2;
	if (Vertical) {
		switch (gPixelFormat) {
			case G_GRAYSCALE:
			case G_RGB_PALETTE:
				for (i = 0; i < j; i++) {	
					pixel8Src = (GUInt8 *)gPixels + i * gWidth;
					pixel8Dest = (GUInt8 *)gPixels + (gHeight - i - 1) * gWidth;
					for (k = 0; k < gWidth; k++) {
						swap8 = *pixel8Src;
						*pixel8Src = *pixel8Dest;
						*pixel8Dest = swap8;
						pixel8Src++;
						pixel8Dest++;
					}
				}
				break;
			case G_R8G8B8:
			case G_A8R8G8B8:
				for (i = 0; i < j; i++) {	
					pixel32Src = (GUInt32 *)gPixels + i * gWidth;
					pixel32Dest = (GUInt32 *)gPixels + (gHeight - i - 1) * gWidth;
					for (k = 0; k < gWidth; k++) {
						swap32 = *pixel32Src;
						*pixel32Src = *pixel32Dest;
						*pixel32Dest = swap32;
						pixel32Src++;
						pixel32Dest++;
					}
				}
				break;
			case G_A1R5G5B5:
			case G_R5G6B5:
				for (i = 0; i < j; i++) {	
					pixel16Src = (GUInt16 *)gPixels + i * gWidth;
					pixel16Dest = (GUInt16 *)gPixels + (gHeight - i - 1) * gWidth;
					for (k = 0; k < gWidth; k++) {
						swap16 = *pixel16Src;
						*pixel16Src = *pixel16Dest;
						*pixel16Dest = swap16;
						pixel16Src++;
						pixel16Dest++;
					}
				}
				break;
		}
	}
	return G_NO_ERROR;
}

GError GPixelMap::SetPixelFormat(const GPixelFormat NewPixelFormat) {

	if (gPixelFormat == NewPixelFormat)
		return G_NO_ERROR;
	// these 2 special cases, for performance reasons, can be managed easily
	if (gPixelFormat == G_R8G8B8 && NewPixelFormat == G_A8R8G8B8)
		return G_NO_ERROR;
	if (gPixelFormat == G_A8R8G8B8 && NewPixelFormat == G_R8G8B8)
		return G_NO_ERROR;

	GPixelMap tmpImage;
	GError err;

	err = SetPixelFormat(NewPixelFormat, tmpImage);
	if (err == G_NO_ERROR) {
		Reset();
		ReferenceMove(tmpImage);
	}
	return err;
}

GError GPixelMap::SetPixelFormat(const GPixelFormat NewPixelFormat, GPixelMap& ConvertedImage) const {

	GError err;
	GUInt32 i, j, gray32, *pixels32, red32, green32, blue32, alpha32, rgb32;
	GUChar8 *pixels8, *pix8;
	GUInt16 *pixels16, rgb16, gray16, red16, green16, blue16, alpha16, *pixels16dst;

	if ((NewPixelFormat == gPixelFormat) || (gPixelFormat == G_R8G8B8 && NewPixelFormat == G_A8R8G8B8) ||
		(gPixelFormat == G_A8R8G8B8 && NewPixelFormat == G_R8G8B8)) {
		err = CopyTo(ConvertedImage);
		return err;
	}

	err = ConvertedImage.Reset(gWidth, gHeight, NewPixelFormat);
	if (err != G_NO_ERROR)
		return err;

	j = PixelsCount();
	switch (NewPixelFormat) {
		case G_RGB_PALETTE:
			// octree color quantization
			err = OctreeQuantization(*this, 256, 8, ConvertedImage.gPalette, 256, j, ConvertedImage.gPixels);
			break;
		case G_R8G8B8:
		case G_A8R8G8B8:
			// palette to full 24/32 bit color conversion
			if (gPixelFormat == G_RGB_PALETTE) {
				pixels8 = gPixels;
				pixels32 = (GUInt32 *)ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					rgb32 = gPalette[*pixels8++];
					*pixels32++ = rgb32;
				}
			}
			else
			// grayscale to full 24bit color conversion
			if (gPixelFormat == G_GRAYSCALE) {
				pixels8 = (GUChar8 *)gPixels;
				pixels32 = (GUInt32 *)ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					gray32 = (GUInt32)(*pixels8);
					*pixels32 = gray32 | (gray32 << 8) | (gray32 << 16);
					pixels32++;
					pixels8++;
				}
			}
			else
			// A1R5G5B5 to full 24bit color conversion
			if (gPixelFormat == G_A1R5G5B5) {
				pixels16 = (GUInt16 *)gPixels;
				pixels32 = (GUInt32 *)ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					rgb16 = *pixels16;
					blue32 = rgb16 & 31;
					green32 = (rgb16 >> 5) & 31;
					red32 = (rgb16 >> 10) & 31;
					alpha32 = (rgb16 >> 15) & 1;
					*pixels32 = (alpha32 << 31) | (red32 << 19) | (green32 << 11) | (blue32 << 3);
					pixels32++;
					pixels16++;
				}
			}
			else
			// R5G6B5 to full 24bit color conversion
			if (gPixelFormat == G_R5G6B5) {
				pixels16 = (GUInt16 *)gPixels;
				pixels32 = (GUInt32 *)ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					rgb16 = *pixels16;
					blue32 = rgb16 & 31;
					green32 = (rgb16 >> 5) & 63;
					red32 = (rgb16 >> 11) & 31;
					*pixels32 = (red32 << 19) | (green32 << 10) | (blue32 << 3);
					pixels32++;
					pixels16++;
				}
			}
			break;

		case G_A1R5G5B5:
			// palette to 15 bit high color conversion
			if (gPixelFormat == G_RGB_PALETTE) {
				pixels8 = gPixels;
				pixels16 = (GUInt16 *)ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					rgb32 = gPalette[*pixels8++];
					red16 = (GUInt16)((rgb32 >> 19) & 31);
					green16 = (GUInt16)((rgb32 >> 11) & 31);
					blue16 = (GUInt16)((rgb32 >> 3)& 31);
					*pixels16++ = (red16 << 10) | (green16 << 5) | blue16;
				}
			}
			else
			// grayscale to 16bit color conversion
			if (gPixelFormat == G_GRAYSCALE) {
				pixels8 = (GUChar8 *)gPixels;
				pixels16 = (GUInt16 *)ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					gray16 = (GUInt16)(*pixels8);
					gray16 = (gray16 >> 2) & 63;
					*pixels16 = (((gray16 >> 1) & 31) << 11) | (gray16 << 5) | ((gray16 >> 1) & 31);
					pixels16++;
					pixels8++;
				}
			}
			else
			// full 24bit to 16bit color conversion
			if (gPixelFormat == G_R8G8B8 || gPixelFormat ==  G_A8R8G8B8) {
				pixels32 = (GUInt32 *)gPixels;
				pixels16 = (GUInt16 *)ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					rgb32 = *pixels32;
					alpha16 = (rgb32 >> 31) & 1;
					red16 = (rgb32 >> 19) & 31;
					green16 = (rgb32 >> 11) & 31;
					blue16 = (rgb32 >> 3) & 31;
					*pixels16 = (alpha16 << 15) | (red16 << 10) | (green16 << 5) | blue16;
					pixels16++;
					pixels32++;
				}
			}
			else
			if (gPixelFormat == G_R5G6B5) {
				pixels16 = (GUInt16 *)gPixels;
				pixels16dst = (GUInt16 *)ConvertedImage.Pixels();
				for (i = 0; i < j; i++) {
					rgb16 = *pixels16;
					red16 = (rgb16 >> 11) & 31;
					green16 = (rgb16 >> 6) & 31;
					blue16 = rgb16 & 31;
					*pixels16dst = (red16 << 10) | (green16 << 5) | blue16;
					pixels16++;
					pixels16dst++;
				}
			}
			break;

		case G_R5G6B5:
			// palette to 15 bit high color conversion
			if (gPixelFormat == G_RGB_PALETTE) {
				pixels8 = gPixels;
				pixels16 = (GUInt16 *)ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					rgb32 = gPalette[*pixels8++];
					red16 = (GUInt16)((rgb32 >> 19) & 31);
					green16 = (GUInt16)((rgb32 >> 10) & 63);
					blue16 = (GUInt16)((rgb32 >> 3)& 31);
					*pixels16++ = (red16 << 11) | (green16 << 5) | blue16;
				}
			}
			else
			// grayscale to 16bit color conversion
			if (gPixelFormat == G_GRAYSCALE) {
				pixels8 = (GUChar8 *)gPixels;
				pixels16 = (GUInt16 *)ConvertedImage.Pixels();
				for (i = 0; i < j; i++) {
					gray16 = (GUInt16)(*pixels8);
					gray16 = (gray16 >> 2) & 63;
					*pixels16 = (((gray16 >> 1) & 31) << 11) | (gray16 << 5) | ((gray16 >> 1) & 31);
					pixels16++;
					pixels8++;
				}
			}
			else
			// full 24bit to 16bit color conversion
			if (gPixelFormat == G_R8G8B8 || gPixelFormat ==  G_A8R8G8B8) {
				pixels32 = (GUInt32 *)gPixels;
				pixels16 = (GUInt16 *)ConvertedImage.Pixels();
				for (i = 0; i < j; i++) {
					rgb32 = *pixels32;
					red16 = (rgb32 >> 19) & 31;
					green16 = (rgb32 >> 10) & 63;
					blue16 = (rgb32 >> 3) & 31;
					*pixels16 = (red16 << 11) | (green16 << 5) | blue16;
					pixels16++;
					pixels32++;
				}
			}
			else
			if (gPixelFormat == G_A1R5G5B5) {
				pixels16 = (GUInt16 *)gPixels;
				pixels16dst = (GUInt16 *)ConvertedImage.Pixels();
				for (i = 0; i < j; i++) {
					rgb16 = *pixels16;
					red16 = (rgb16 >> 10) & 31;
					green16 = (rgb16 >> 5) & 31;
					blue16 = rgb16 & 31;
					*pixels16dst = (red16 << 11) | (green16 << 6) | blue16;
					pixels16++;
					pixels16dst++;
				}
			}
			break;

		case G_GRAYSCALE:
			// palette to grayscale conversion
			if (gPixelFormat == G_RGB_PALETTE) {
				pixels8 = gPixels;
				pix8 = ConvertedImage.Pixels();
				for (i = 0; i < j; ++i) {
					rgb32 = gPalette[*pixels8++];
					*pix8++ = RGBToGray(rgb32);
				}
			}
			else
			// full 24bit color to grayscale conversion
			if ((gPixelFormat == G_R8G8B8) || (gPixelFormat == G_A8R8G8B8)) {
				pixels32 = (GUInt32 *)gPixels;
				pixels8 = (GUChar8 *)ConvertedImage.Pixels();
				for (i = 0; i < j; i++) {
					gray32 = RGBToGray(*pixels32);
					*pixels8 = (GUChar8)gray32;
					pixels32++;
					pixels8++;
				}
			}
			else
			// 16bit color to grayscale conversion
			if (gPixelFormat == G_A1R5G5B5) {
				pixels16 = (GUInt16 *)gPixels;
				pixels8 = (GUChar8 *)ConvertedImage.Pixels();
				for (i = 0; i < j; i++) {
					rgb16 = *pixels16;
					red32 = ((rgb16 >> 10) & 31) << 3;
					green32 = ((rgb16 >> 5) & 31) << 3;
					blue32 = (rgb16 & 31) << 3;
					gray32 = RGBToGray(red32, green32, blue32);
					*pixels8 = (GUChar8)gray32;
					pixels16++;
					pixels8++;
				}
			}
			else
			if (gPixelFormat == G_R5G6B5) {
				pixels16 = (GUInt16 *)gPixels;
				pixels8 = (GUChar8 *)ConvertedImage.Pixels();
				for (i = 0; i < j; i++) {
					rgb16 = *pixels16;
					red32 = ((rgb16 >> 11) & 31) << 3;
					green32 = ((rgb16 >> 5) & 63) << 2;
					blue32 = (rgb16 & 31) << 3;
					gray32 = RGBToGray(red32, green32, blue32);
					*pixels8 = (GUChar8)gray32;
					pixels16++;
					pixels8++;
				}
			}
			break;
	}
	return err;
}

void GPixelMap::BuildFiltersBrightnessLUT(GUChar8 *LUT, const GUInt32 Threshold, const GBool SixDeg) {

	GUChar8 *tmpLUT = LUT;
	GReal tmp, realThres = (GReal)Threshold;

	G_ASSERT(LUT != NULL);

	// center position
	tmpLUT += 258;

	if (SixDeg) {
		for (GInt32 k = -256; k < 257; k++) {
			tmp=((GReal)k) / realThres;
			tmp = tmp * tmp;
			tmp = tmp * tmp * tmp;
			tmp = (GReal)100.0 * GMath::Exp(-tmp);
			*(tmpLUT + k) = (GUChar8)tmp;
		}
	}
	else {
		for (GInt32 k = -256; k < 257; k++) {
			tmp=((GReal)k) / realThres;
			tmp = tmp * tmp;
			tmp = (GReal)100.0 * GMath::Exp(-tmp);
			*(tmpLUT + k) = (GUChar8)tmp;
		}
	}
}

GError GPixelMap::EdgePreservingSmoothMono(const GInt32 DistThres, const GInt32 BrighThres, const GBool Fast) {

	if (!IsGrayScale())
		return G_INVALID_FORMAT;

	GFloat temp;
	GInt32 n_max, increment, mask_size, i, j, x, y, area, brightness, tmp, centre;
	GUChar8 *ip, *dp, *dpt, *cp, *out, *in, *bp, *brighTable;
	GFloat total;
	GError err;
	GInt32 p[8], k, l, aux, XSize, YSize;
	GPixelMap tmpPixmap;

	if (!Fast)
		mask_size = ((GInt32)((GFloat)1.5 * DistThres)) + 1;
	else
		mask_size = 1;

	total = (GFloat)0.1;
	if ((DistThres > 15) && (total == 0))
		return G_INVALID_PARAMETER;

	XSize = gWidth;
	YSize = gHeight;

	if ((2 * mask_size + 1 > XSize) || (2 * mask_size + 1 > YSize))
		return G_INVALID_PARAMETER;

	// enlarge image by mask_size
	err = ResizeCanvasMirror(mask_size, mask_size, mask_size, mask_size, tmpPixmap);
	if (err != G_NO_ERROR)
		return err;

	// build brightness table
	brighTable = new GUChar8[516];
	BuildFiltersBrightnessLUT(brighTable, BrighThres, G_FALSE);
	bp = brighTable + 258;

	out = (GUChar8 *)gPixels;
	in = (GUChar8 *)tmpPixmap.Pixels();
	XSize = tmpPixmap.Width();
	YSize = tmpPixmap.Height();

	// large Gaussian masks
	if (!Fast) {     
		// setup distance lut (look up table)
		n_max = (mask_size * 2) + 1;
		increment = XSize - n_max;
		dp = new GUChar8[n_max * n_max];
		dpt = dp;
		temp = (GFloat)(-(DistThres * DistThres));
		for (i = -mask_size; i <= mask_size; i++)
			for (j = -mask_size; j <= mask_size; j++) {
				x = (GInt32)((GFloat)100 * GMath::Exp(((GFloat)((i * i) + (j * j))  ) / temp));
				*dpt++ = (GUChar8)x;
			}

			for (i = mask_size; i < YSize - mask_size; i++) {
				for (j = mask_size; j < XSize - mask_size; j++) {
					area = 0;
					total = 0;
					dpt = dp;
					ip = in + ((i - mask_size) * XSize) + j - mask_size;
					centre = in[i * XSize + j];
					cp = bp + centre;
					for (y = -mask_size; y <= mask_size; y++) {
						for (x = -mask_size; x <= mask_size; x++) {
							brightness = *ip++;
							tmp = *dpt++ * (*(cp - brightness));
							area += tmp;
							total += tmp * brightness;
						}
						ip += increment;
					}
					tmp = area - 10000;
					if (tmp == 0) {
						// lets do a median filter
						p[0] = in[(i - 1) * XSize + j - 1];
						p[1] = in[(i - 1) * XSize + j];
						p[2] = in[(i - 1) * XSize + j + 1];
						p[3] = in[i * XSize + j - 1];
						p[4] = in[i * XSize + j + 1];
						p[5] = in[(i + 1) * XSize + j - 1];
						p[6] = in[(i + 1) * XSize + j];
						p[7] = in[(i + 1) * XSize + j + 1];
						for (k = 0; k < 7; k++)
							for (l = 0; l < (7 - k); l++)
								if (p[l] > p[l + 1]) {
									aux = p[l];
									p[l] = p[l + 1];
									p[l + 1] = aux;
								}
								*out++ = (GUChar8)((p[3] + p[4]) / 2);
					}
					else
						*out++ = (GUChar8)((total - (centre * 10000)) / tmp);
				}
			}
			// delete distance table
			delete [] dp;
	}
	// 3x3 constant mask
	else {     
		for (i = 1; i < YSize - 1; i++) {
			for (j = 1; j < XSize - 1; j++){
				area = 0;
				total = 0;
				ip = in + ((i - 1) * XSize) + j - 1;
				centre = in[i * XSize + j];
				cp = bp + centre;
				brightness = *ip++;
				tmp = *(cp - brightness);
				area += tmp;
				total += tmp * brightness;
				brightness = *ip++;
				tmp =*(cp - brightness);
				area += tmp;
				total += tmp * brightness;
				brightness = *ip;
				tmp = *(cp - brightness);
				area += tmp;
				total += tmp * brightness;
				ip += XSize - 2;
				brightness = *ip++;
				tmp = *(cp - brightness);
				area += tmp;
				total += tmp * brightness;
				brightness = *ip++;
				tmp = *(cp - brightness);
				area += tmp;
				total += tmp * brightness;
				brightness = *ip;
				tmp = *(cp - brightness);
				area += tmp;
				total += tmp * brightness;
				ip += XSize - 2;
				brightness = *ip++;
				tmp = *(cp - brightness);
				area += tmp;
				total += tmp * brightness;
				brightness = *ip++;
				tmp = *(cp - brightness);
				area += tmp;
				total += tmp * brightness;
				brightness = *ip;
				tmp = *(cp - brightness);
				area += tmp;
				total += tmp * brightness;

				tmp = area - 100;
				if (tmp == 0) {
					// lets do a median filter
					p[0] = in[(i - 1) * XSize + j - 1];
					p[1] = in[(i - 1) * XSize + j];
					p[2] = in[(i - 1) * XSize + j + 1];
					p[3] = in[i * XSize + j - 1];
					p[4] = in[i * XSize + j + 1];
					p[5] = in[(i + 1) * XSize + j - 1];
					p[6] = in[(i + 1) * XSize + j];
					p[7] = in[(i + 1) * XSize + j + 1];
					for (k = 0; k < 7; k++)
						for (l = 0; l < (7 - k); l++)
							if (p[l] > p[l + 1]) {
								aux = p[l];
								p[l] = p[l + 1];
								p[l + 1] = aux;
							}
							*out++ = (GUChar8)((p[3] + p[4]) / 2);
				}
				else
					*out++ = (GUChar8)((total - (centre * 100)) / tmp);
			}
		}
	}
	// free brightness table
	delete [] brighTable;
	return G_NO_ERROR;
}

GError GPixelMap::EdgePreservingSmooth(const GInt32 DistThres, const GInt32 BrighThres, const GBool Fast) {

	GError err = G_NO_ERROR;

	if (IsPaletted())
		return G_INVALID_OPERATION;

	if (IsGrayScale())
		err = EdgePreservingSmoothMono(DistThres, BrighThres, Fast);
	else {
		GPixelMap alphaChannel, redChannel, greenChannel, blueChannel;
		switch (gPixelFormat) {
			case G_RGB_PALETTE:
			case G_GRAYSCALE:
				// just to make gcc happy with warnings..
				break;
			case G_A8R8G8B8:
			case G_A1R5G5B5:
				err = SplitChannels(&alphaChannel, &redChannel, &greenChannel, &blueChannel);
				if (err == G_NO_ERROR) {
					err = alphaChannel.EdgePreservingSmoothMono(DistThres, BrighThres, Fast);
					if (err == G_NO_ERROR) {
						err = redChannel.EdgePreservingSmoothMono(DistThres, BrighThres, Fast);
						if (err == G_NO_ERROR) {
							err = greenChannel.EdgePreservingSmoothMono(DistThres, BrighThres, Fast);
							if (err == G_NO_ERROR) {
								err = blueChannel.EdgePreservingSmoothMono(DistThres, BrighThres, Fast);
								if (err == G_NO_ERROR)
									err = MergeChannels(redChannel, greenChannel, blueChannel, &alphaChannel);
							}
						}
					}
				}
				break;

			case G_R8G8B8:
			case G_R5G6B5:
				err = SplitChannels(NULL, &redChannel, &greenChannel, &blueChannel);
				if (err == G_NO_ERROR) {
					err = redChannel.EdgePreservingSmoothMono(DistThres, BrighThres, Fast);
					if (err == G_NO_ERROR) {
						err = greenChannel.EdgePreservingSmoothMono(DistThres, BrighThres, Fast);
						if (err == G_NO_ERROR) {
							err = blueChannel.EdgePreservingSmoothMono(DistThres, BrighThres, Fast);
							if (err == G_NO_ERROR)
								err = MergeChannels(redChannel, greenChannel, blueChannel, NULL);
						}
					}
				}
				break;
		}
	}
	return err;
}

GError GPixelMap::TraceContoursFast(GUChar8 *in, GInt32 *r, GUChar8 *mid, GUChar8 *bp,
									GInt32 max_no, GInt32 x_size, GInt32 y_size) {

	
	GFloat z;
	GInt32 do_symmetry, i, j, m, n, a, b, x, y, w;
	GUChar8 c, *p, *cp;

	std::memset(r, 0, x_size * y_size * sizeof(GInt32));
	max_no = 730;
	for (i = 1; i < y_size - 1; i++)
		for (j = 1; j < x_size - 1; j++) {
			n = 100;
			p = in + (i - 1) * x_size + j - 1;
			cp = bp + in[i * x_size + j];
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			p += x_size - 2; 
			n += *(cp - *p);
			p += 2;
			n += *(cp - *p);
			p += x_size - 2;
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			if (n <= max_no)
				r[i * x_size + j] = max_no - n;
		}

	for (i = 2; i < y_size - 2; i++)
		for (j = 2; j < x_size - 2; j++) {
			if (r[i * x_size + j] > 0) {
				m = r[i * x_size + j];
				n = max_no - m;
				cp = bp + in[i * x_size + j];
				if (n > 250) {
					p = in + (i - 1) * x_size + j - 1;
					x = 0;
					y = 0;
					c = *(cp - *p++);
					x -= c;
					y -= c;
					c = *(cp - *p++);
					y -= c;
					c = *(cp - *p);
					x += c;
					y -= c;
					p += x_size - 2; 
					c = *(cp - *p);
					x -= c;
					p += 2;
					c = *(cp - *p);
					x += c;
					p += x_size - 2;
					c = *(cp - *p++);
					x -= c;
					y += c;
					c = *(cp - *p++);
					y += c;
					c = *(cp - *p);
					x += c;
					y += c;
					z = GMath::Sqrt((GFloat)((x * x) + (y * y)));
					if (z > ((GFloat)0.4*(GFloat)n)) { // 0.6
						do_symmetry = 0;
						if (x == 0)
							z = (GFloat)1000000.0;
						else
							z =((GFloat)y) / ((GFloat)x);
						if (z < 0) {
							z = -z;
							w = -1;
						}
						else
							w = 1;
						if (z < (GFloat)0.5) {/* vert_edge */
							a = 0;
							b = 1;
						}
						else {
							if (z > (GFloat)2.0) {/* hor_edge */
								a = 1;
								b = 0;
							}
							else {
								// diag_edge
								if (w > 0) {
									a = 1;
									b = 1;
								}
								else {
									a = -1;
									b = 1;
								}
							}
						}
						if ((m > r[(i + a) * x_size + j + b]) && (m >= r[(i - a) * x_size + j - b]))
							mid[i * x_size + j] = 1;
					}
					else
						do_symmetry = 1;
				}
				else
					do_symmetry = 1;

				if (do_symmetry == 1) { 
					p = in + (i - 1) * x_size + j - 1;
					x = 0;
					y = 0;
					w = 0;
					c = *(cp - *p++);
					x += c;
					y += c;
					w += c;
					c = *(cp - *p++);
					y += c;
					c = *(cp - *p);
					x += c;
					y += c;
					w -= c;
					p += x_size - 2;
					c = *(cp - *p);
					x += c;
					p += 2;
					c = *(cp - *p);
					x += c;
					p += x_size - 2;
					c = *(cp - *p++);
					x += c;
					y += c;
					w -= c;
					c = *(cp - *p++);
					y += c;
					c = *(cp - *p);
					x += c;
					y += c;
					w += c;
					if (y == 0)
						z = (GFloat)1000000.0;
					else
						z = ((GFloat)x) / ((GFloat)y);
					if (z < (GFloat)0.5) {/* vertical */
						a = 0;
						b = 1;
					}
					else {
						if (z > (GFloat)2.0) {/* horizontal */
							a = 1;
							b = 0;
						}
						else {/* diagonal */
							if (w > 0) {
								a = -1;
								b = 1;
							}
							else {
								a = 1;
								b = 1;
							}
						}
					}
					if ((m > r[(i + a) * x_size + j + b]) && (m >= r[(i - a) * x_size + j - b]))
						mid[i * x_size + j] = 2;	
				}
			}
		}
	return G_NO_ERROR;
}

GError GPixelMap::TraceContoursAccurate(GUChar8 *in, GInt32 *r, GUChar8 *mid, GUChar8 *bp,
										GInt32 max_no, GInt32 x_size, GInt32 y_size) {

	GFloat z;
	GInt32 do_symmetry, i, j, m, n, a, b, x, y, w;
	GUChar8 c,*p,*cp;

	std::memset(r, 0, x_size * y_size * sizeof(GInt32));

	for (i = 3; i < y_size - 3; i++)
		for (j = 3; j < x_size - 3; j++) {
			n = 100;
			p = in + (i - 3) * x_size + j - 1;
			cp = bp + in[i * x_size + j];
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			p += x_size - 3; 
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			p += x_size - 5;
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			p += x_size - 6;
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			p += 2;
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			p += x_size - 6;
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			p += x_size - 5;
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			p += x_size - 3;
			n += *(cp - *p++);
			n += *(cp - *p++);
			n += *(cp - *p);
			if (n <= max_no)
				r[i * x_size + j] = max_no - n;
		}

	for (i = 4;i < y_size - 4;i++)
		for (j = 4;j < x_size - 4;j++) {
			if (r[i * x_size + j]>0) {
				m = r[i * x_size + j];
				n = max_no - m;
				cp = bp + in[i * x_size + j];
				if (n > 600) {
					p = in + (i - 3) * x_size + j - 1;
					x = 0;
					y = 0;
					c = *(cp - *p++);
					x -= c;
					y -= 3 * c;
					c = *(cp - *p++);
					y -= 3 * c;
					c = *(cp - *p);
					x += c;
					y -= 3 * c;
					p += x_size - 3; 
			    
					c = *(cp - *p++);
					x -= 2 * c;
					y -= 2 * c;
					c = *(cp - *p++);
					x -= c;
					y -= 2 * c;
					c = *(cp - *p++);
					y -= 2 * c;
					c = *(cp - *p++);
					x += c;
					y -= 2 * c;
					c = *(cp - *p);
					x += 2 * c;
					y -= 2 * c;
					p += x_size - 5;
					c = *(cp - *p++);
					x -= 3 * c;
					y -= c;
					c = *(cp - *p++);
					x -= 2 * c;
					y -= c;
					c = *(cp - *p++);
					x -= c;
					y -= c;
					c = *(cp - *p++);
					y -= c;
					c = *(cp - *p++);
					x += c;
					y -= c;
					c = *(cp - *p++);
					x += 2 * c;
					y -= c;
					c = *(cp - *p);
					x += 3 * c;
					y -= c;
					p += x_size - 6;
					c = *(cp - *p++);
					x -= 3 * c;
					c = *(cp - *p++);
					x -= 2 * c;
					c = *(cp - *p);
					x -= c;
					p += 2;
					c = *(cp - *p++);
					x += c;
					c = *(cp - *p++);
					x += 2 * c;
					c = *(cp - *p);
					x += 3 * c;
					p += x_size - 6;
					c = *(cp - *p++);
					x -= 3 * c;
					y += c;
					c = *(cp - *p++);
					x -= 2 * c;
					y += c;
					c = *(cp - *p++);
					x -= c;
					y += c;
					c = *(cp - *p++);
					y += c;
					c = *(cp - *p++);
					x += c;
					y += c;
					c = *(cp - *p++);
					x += 2 * c;
					y += c;
					c = *(cp - *p);
					x += 3 * c;
					y += c;
					p += x_size - 5;
					c = *(cp - *p++);
					x -= 2 * c;
					y += 2 * c;
					c = *(cp - *p++);
					x -= c;
					y += 2 * c;
					c = *(cp-*p++);
					y += 2 * c;
					c = *(cp - *p++);
					x += c;
					y += 2 * c;
					c = *(cp - *p);
					x += 2 * c;
					y += 2 * c;
					p += x_size - 3;
					c = *(cp - *p++);
					x -= c;
					y += 3 * c;
					c = *(cp - *p++);
					y += 3 * c;
					c = *(cp - *p);
					x += c;
					y += 3 * c;
					z = GMath::Sqrt((GFloat)((x * x) + (y * y)));
					if (z > ((GFloat)0.9*(GFloat)n)) { // 0.5
						do_symmetry = 0;
						if (x == 0)
							z = (GFloat)1000000.0;
						else
							z = ((GFloat)y) / ((GFloat)x);
						if (z < 0) {
							z = -z;
							w = -1;
						}
						else
							w = 1;
						if (z < (GFloat)0.5) {/* vert_edge */
							a = 0;
							b = 1;
						}
						else {
							if (z > (GFloat)2.0) {/* hor_edge */
								a = 1;
								b = 0;
							}
							else {/* diag_edge */
								if (w > 0) {
									a = 1;
									b = 1;
								}
                                else {
									a = -1;
									b = 1;
								}
							}
						}
						if ((m > r[(i + a) * x_size + j + b]) && (m >= r[(i - a) * x_size + j - b]) &&
							(m > r[(i + (2 * a)) * x_size + j + (2 * b)]) && (m >= r[(i - (2 * a)) * x_size + j - (2 * b)]))
							mid[i * x_size + j] = 1;
					}
					else
						do_symmetry = 1;
				}
				else 
					do_symmetry = 1;

				if (do_symmetry == 1) { 
					p = in + (i - 3) * x_size + j - 1;
					x = 0;
					y = 0;
					w = 0;
					c = *(cp - *p++);
					x += c;
					y += 9 * c;
					w += 3 * c;
					c = *(cp - *p++);
					y += 9 * c;
					c = *(cp - *p);
					x += c;
					y += 9 * c;
					w -= 3 * c;
					p += x_size - 3;
			  		c = *(cp - *p++);
					x += 4 * c;
					y += 4 * c;
					w += 4 * c;
					c = *(cp - *p++);
					x +=c ;
					y += 4 * c;
					w += 2 * c;
					c = *(cp - *p++);
					y += 4 * c;
					c = *(cp - *p++);
					x += c;
					y += 4 * c;
					w -= 2 * c;
					c = *(cp - *p);
					x += 4 * c;
					y += 4 * c;
					w -= 4 * c;
					p += x_size - 5;
					c = *(cp - *p++);
					x += 9 * c;
					y += c;
					w += 3 * c;
					c = *(cp - *p++);
					x += 4 * c;
					y += c;
					w += 2 * c;
					c = *(cp - *p++);
					x += c;
					y += c;
					w += c;
					c = *(cp - *p++);
					y += c;
					c = *(cp - *p++);
					x += c;
					y += c;
					w -= c;
					c = *(cp - *p++);
					x += 4 * c;
					y += c;
					w -= 2 * c;
					c = *(cp - *p);
					x += 9 * c;
					y += c;
					w -= 3 * c;
					p += x_size - 6;
					c = *(cp - *p++);
					x += 9 * c;
					c = *(cp - *p++);
					x += 4 * c;
					c = *(cp - *p);
					x += c;
					p += 2;
					c = *(cp - *p++);
					x += c;
					c = *(cp - *p++);
					x += 4 * c;
					c = *(cp - *p);
					x += 9 * c;
					p += x_size - 6;
					c = *(cp - *p++);
					x += 9 * c;
					y += c;
					w -= 3 * c;
					c = *(cp - *p++);
					x += 4 * c;
					y += c;
					w -= 2 * c;
					c = *(cp - *p++);
					x += c;
					y += c;
					w -= c;
					c = *(cp - *p++);
					y += c;
					c = *(cp - *p++);
					x += c;
					y += c;
					w += c;
					c = *(cp - *p++);
					x += 4 * c;
					y += c;
					w += 2 * c;
					c = *(cp - *p);
					x += 9 * c;
					y += c;
					w += 3 * c;
					p += x_size - 5;
					c = *(cp - *p++);
					x += 4 * c;
					y += 4 * c;
					w -= 4 * c;
					c = *(cp - *p++);
					x += c;
					y += 4 * c;
					w -= 2 * c;
					c = *(cp - *p++);
					y += 4 * c;
					c = *(cp - *p++);
					x += c;
					y += 4 * c;
					w += 2 * c;
					c = *(cp - *p);
					x += 4 * c;
					y += 4 * c;
					w += 4 * c;
					p += x_size - 3;
					c = *(cp - *p++);
					x += c;
					y += 9 * c;
					w -= 3 * c;
					c = *(cp - *p++);
					y += 9 * c;
					c = *(cp - *p);
					x += c;
					y += 9 * c;
					w += 3 * c;
					if (y == 0)
						z = (GFloat)1000000.0;
					else
						z = ((GFloat)x) / ((GFloat)y);
					if (z < (GFloat)0.5) {/* vertical */
						a = 0;
						b = 1;
					}
					else {
						if (z > (GFloat)2.0) {/* horizontal */
							a = 1;
							b = 0;
						}
						else {/* diagonal */
							if (w > 0) {
								a = -1;
								b = 1;
							}
                            else {
								a = 1;
								b = 1;
							}
						}
					}
					if ((m > r[(i + a) * x_size + j + b]) && (m >= r[(i - a) * x_size + j - b]) &&
						(m > r[(i + (2 * a)) * x_size + j + (2 * b)]) && (m >= r[(i - (2 * a)) * x_size +j - (2 * b)]))
						mid[i * x_size + j] = 2;	
				}
			}
		}
	return G_NO_ERROR;
}

GError GPixelMap::ThinContours(GInt32 *r, GUChar8 *mid, GInt32 x_size, GInt32 y_size) {

	GInt32 l[9], centre, b01, b12, b21, b10;
	GInt32 p1, p2, p3, p4, b00, b02, b20, b22, m, n, a = 0, b = 0, x, y, i, j;
	GUChar8 *mp;

	for (i = 4; i < y_size - 4; i++)
		for (j = 4; j < x_size - 4; j++)
			if (mid[i * x_size + j] < 8) {
				centre = r[i * x_size + j];
				// count number of neighbors
				mp = mid + (i - 1) * x_size + j - 1;
				n = (*mp < 8) + (*(mp + 1) < 8) + (*(mp + 2) < 8) + (*(mp + x_size) < 8) +
					(*(mp + x_size + 2) < 8) + (*(mp + x_size + x_size) < 8) +
					(*(mp + x_size + x_size + 1) < 8) + (*(mp + x_size + x_size + 2) < 8);

				// n==0 no neighbors - remove point
				if (n == 0)
					mid[i * x_size + j] = 100;

				// n==1 - extend line if possible
				// extension is only allowed a few times - the value of mid is used to control this
				if ((n == 1) && (mid[i * x_size + j] < 6)) {
					// find maximum neighbor weighted in direction opposite the
					// neighbor already present. e.g.
					// have: O O O  weight r by 0 2 3
					//       X X O              0 0 4
					//       O O O              0 2 3
					l[0] = r[(i - 1) * x_size + j - 1];
					l[1] = r[(i - 1) * x_size + j];
					l[2] = r[(i - 1) * x_size + j + 1];
					l[3] = r[i * x_size + j - 1];
					l[4] = 0;
					l[5] = r[i * x_size + j + 1];
					l[6] = r[(i + 1) * x_size + j - 1];
					l[7] = r[(i + 1) * x_size + j];
					l[8] = r[(i + 1) * x_size + j + 1];
					if (mid[(i - 1) * x_size + j - 1] < 8) {
						l[0] = 0;
						l[1] = 0;
						l[3] = 0;
						l[2] *= 2; 
                        l[6] *= 2;
						l[5] *= 3;
						l[7] *= 3;
						l[8] *= 4;
					}
					else {
						if (mid[(i - 1) * x_size + j] < 8) {
							l[1] = 0;
							l[0] = 0;
							l[2] = 0;
							l[3] *= 2; 
                            l[5] *= 2;
							l[6] *= 3;
							l[8] *= 3;
							l[7] *= 4;
						}
						else {
							if (mid[(i - 1) * x_size + j + 1] < 8) {
								l[2] = 0;
								l[1] = 0;
								l[5] = 0;
								l[0] *= 2; 
                                l[8] *= 2;
								l[3] *= 3;
								l[7] *= 3;
								l[6] *= 4;
							}
							else {
								if (mid[(i) * x_size + j - 1] < 8) {
									l[3] = 0;
									l[0] = 0;
									l[6] = 0;
									l[1] *= 2; 
                                    l[7] *= 2;
									l[2] *= 3;
									l[8] *= 3;
									l[5] *= 4;
								}
								else {
									if (mid[i * x_size + j + 1] < 8) {
										l[5] = 0;
										l[2] = 0;
										l[8] = 0;
										l[1] *= 2; 
                                        l[7] *= 2;
										l[0] *= 3;
										l[6] *= 3;
										l[3] *= 4;
									}
									else {
										if (mid[(i + 1) * x_size + j - 1] < 8) {
											l[6] = 0;
											l[3] = 0;
											l[7] = 0;
											l[0] *= 2; 
                                            l[8] *= 2;
											l[1] *= 3;
											l[5] *= 3;
											l[2] *= 4;
										}
										else {
											if (mid[(i + 1) * x_size + j] < 8) {
												l[7] = 0;
												l[6] = 0;
												l[8] = 0;
												l[3] *= 2; 
                                                l[5] *= 2;
												l[0] *= 3;
												l[2] *= 3;
												l[1] *= 4;
											}
											else {
												if (mid[(i + 1) * x_size + j + 1] < 8) {
													l[8] = 0;
													l[5] = 0;
													l[7] = 0;
													l[6] *= 2; 
                                                    l[2] *= 2;
													l[1] *= 3;
													l[3] *= 3;
													l[0] *= 4;
												}
											}
										}
									}
								}
							}
						}
					}
					// find the highest point
					m = 0;
					for (y = 0; y < 3; y++)
						for(x = 0; x < 3; x++)
							if (l[y + y + y + x] > m) {
								m = l[y +y +y + x];
								a = y;
								b = x;
							}

					if (m > 0) {
						if (mid[i * x_size + j] < 4)
							mid[(i + a - 1) * x_size + j + b - 1] = 4;
						else
							mid[(i + a - 1) * x_size + j + b - 1] = mid[i * x_size + j] + 1;
						if ((a + a + b) < 3) {// need to jump back in image
							i += a - 1;
							j += b - 2;
							if (i < 4)
								i = 4;
							if (j < 4)
								j = 4;
						}
					}
				}
				if (n == 2) {
					// put in a bit here to straighten edges
					b00 = mid[(i - 1) * x_size +j - 1] < 8; /* corners of 3x3 */
					b02 = mid[(i - 1) * x_size +j + 1] < 8;
					b20 = mid[(i + 1) * x_size +j - 1] < 8;
					b22 = mid[(i + 1) * x_size +j + 1] < 8;
					if (((b00 + b02 + b20 + b22) == 2) && ((b00 | b22) & (b02 | b20))) {
						// case: move a point back into line.
						// e.g. X O X  CAN  become X X X
						//      O X O              O O O
						//      O O O              O O O    */
						if (b00) {
							if (b02) {
								x = 0;
								y = -1;
							}
							else {
								x = -1;
								y = 0;
							}
						}
						else {
							if (b02) {
								x = 1;
								y = 0;
							}
							else {
								x = 0;
								y = 1;
							}
						}
						if (((GFloat)r[(i + y) * x_size + j + x] / (GFloat)centre) > (GFloat)0.7) {
							if (((x == 0) && (mid[(i + (2 * y)) * x_size + j] > 7) && (mid[(i + (2 * y)) * x_size + j - 1] > 7) && (mid[(i + (2 * y)) * x_size + j + 1] > 7)) ||
								((y == 0) && (mid[i * x_size + j + (2 * x)] > 7) && (mid[(i + 1) * x_size + j + (2 * x)] > 7) && (mid[(i - 1) * x_size + j + (2 * x)] > 7))) {
								mid[i * x_size + j] = 100;
								mid[(i + y) * x_size + j + x] = 3;  /* no jumping needed */
							}
						}
					}
					else {
						b01 = mid[(i - 1) * x_size + j] < 8;
						b12 = mid[i * x_size + j + 1] < 8;
						b21 = mid[(i + 1) * x_size + j] < 8;
						b10 = mid[i * x_size + j - 1] < 8;
						if (((b01 + b12 + b21 + b10) == 2) && ((b10 | b12) & (b01 | b21)) &&
							((b01 & ((mid[(i - 2) * x_size + j - 1] < 8) | (mid[(i - 2) * x_size + j + 1] < 8))) | (b10 & ((mid[(i - 1) * x_size + j - 2] < 8) | (mid[(i + 1) * x_size + j - 2] < 8))) |
							(b12 & ((mid[(i - 1) * x_size + j + 2] < 8) | (mid[(i + 1) * x_size + j + 2] < 8))) | (b21 & ((mid[(i + 2) * x_size+ j - 1] < 8) | (mid[(i + 2) * x_size + j + 1] < 8))))) {
							// case; clears odd right angles.
							// e.g.; O O O  becomes O O O
							//       X X O          X O O
							//       O X O          O X O
							mid[i * x_size + j]=100;
							// jump back
							i--;
							j -= 2;
							if (i < 4)
								i = 4;
							if (j < 4)
								j = 4;
						}
					}
				}

				// n>2 the thinning is done here without breaking connectivity
				if (n > 2) {
					b01 = mid[(i - 1) * x_size + j] < 8;
					b12 = mid[i * x_size + j + 1] < 8;
					b21 = mid[(i + 1) * x_size + j] < 8;
					b10 = mid[i * x_size + j - 1] < 8;
					if ((b01 +b12 +b21 + b10) > 1) {
						b00 = mid[(i - 1) * x_size +j - 1] < 8;
						b02 = mid[(i - 1) * x_size +j + 1] < 8;
						b20 = mid[(i + 1) * x_size +j - 1] < 8;
						b22 = mid[(i + 1) * x_size +j + 1] < 8;
						p1 = b00 | b01;
						p2 = b02 | b12;
						p3 = b22 | b21;
						p4 = b20 | b10;
						if (((p1 + p2 + p3 + p4) - ((b01 & p2) + (b12 & p3) + (b21 & p4) + (b10 & p1))) < 2) {
							mid[i * x_size + j] = 100;
							i--;
							j-=2;
							if (i < 4)
								i = 4;
							if (j < 4)
								j = 4;
						}
				}
			}
		}
	return G_NO_ERROR;
}

GError GPixelMap::TraceContoursMono(const GInt32 BrighThres, const GBool Thinning, const GBool Fast) {

	GInt32 *r, x_size, y_size, i, j;
	GUChar8 *bp, *brighTable, *mid, *in, *midp;
	GInt32 max_no_edges = 2650;
	GError err;

	x_size = gWidth;
	y_size = gHeight;
	r = new GInt32[x_size * y_size];

	// build brightness table
	brighTable = new GUChar8[516];
	BuildFiltersBrightnessLUT(brighTable, BrighThres, G_TRUE);
	bp = brighTable + 258;

    mid = new GUChar8[x_size * y_size];
    memset(mid, 100, x_size * y_size);

	in = (GUChar8 *)gPixels;
	if (Fast)
        err = TraceContoursFast(in, r, mid, bp, max_no_edges, x_size, y_size);
	else
        err = TraceContoursAccurate(in, r, mid, bp, max_no_edges, x_size, y_size);

	if (err == G_NO_ERROR) {
		err = G_NO_ERROR;
		if (Thinning)
			err = ThinContours(r, mid, x_size, y_size);

		if (err == G_NO_ERROR) {
			midp = mid;
			j = x_size * y_size;
			for (i = 0; i < j; i++) {
				if (*midp < 8)
					*(in + (midp - mid)) = 0;
				else
					*(in + (midp - mid)) = 255;
				midp++;
			}
		}
	}
	delete [] brighTable;
	delete [] mid;
	delete [] r;
	return G_NO_ERROR;
}

GError GPixelMap::TraceContours(const GInt32 BrighThres, const GBool Thinning, const GBool Fast) {

	GError err = G_NO_ERROR;

	if (IsPaletted())
		return G_INVALID_OPERATION;
	if (IsGrayScale())
		err = TraceContoursMono(BrighThres, Thinning, Fast);
	else {
		GPixelMap alphaChannel, redChannel, greenChannel, blueChannel;
		switch (gPixelFormat) {
			case G_RGB_PALETTE:
			case G_GRAYSCALE:
				// just to make gcc happy with warnings..
				break;
			case G_A8R8G8B8:
			case G_A1R5G5B5:
				err = SplitChannels(&alphaChannel, &redChannel, &greenChannel, &blueChannel);
				if (err == G_NO_ERROR) {
					err = alphaChannel.TraceContoursMono(BrighThres, Thinning, Fast);
					if (err == G_NO_ERROR) {
						err = redChannel.TraceContoursMono(BrighThres, Thinning, Fast);
						if (err == G_NO_ERROR) {
							err = greenChannel.TraceContoursMono(BrighThres, Thinning, Fast);
							if (err == G_NO_ERROR) {
								err = blueChannel.TraceContoursMono(BrighThres, Thinning, Fast);
								if (err == G_NO_ERROR)
									err = MergeChannels(redChannel, greenChannel, blueChannel, &alphaChannel);
							}
						}
					}
				}
				break;

			case G_R8G8B8:
			case G_R5G6B5:
				err = SplitChannels(NULL, &redChannel, &greenChannel, &blueChannel);
				if (err == G_NO_ERROR) {
					err = redChannel.TraceContoursMono(BrighThres, Thinning, Fast);
					if (err == G_NO_ERROR) {
						err = greenChannel.TraceContoursMono(BrighThres, Thinning, Fast);
						if (err == G_NO_ERROR) {
							err = blueChannel.TraceContoursMono(BrighThres, Thinning, Fast);
							if (err == G_NO_ERROR)
								err = MergeChannels(redChannel, greenChannel, blueChannel, NULL);
						}
					}
				}
				break;
		}
	}
	return err;
}

GError GPixelMap::EdgeEnhanceMono(const GInt32 Divisor, const GBool Strong) {

	G_ASSERT(IsGrayScale());

	GPixelMap tmpPixmap;
	GUChar8 *in, *out;
	GInt32 i, j, xs, ys, border;
	GInt32 topHat[9], bottomHat[9], p0, p1, p2, p3, p4, p5, p6, p7, p8;
	GInt32 topAcc, bottomAcc, pixelOut;

	if (!Strong) {
		bottomHat[0] = 1; bottomHat[1] = 0;  bottomHat[2] = 1;
		bottomHat[3] = 0; bottomHat[4] = -4; bottomHat[5] = 0;
		bottomHat[6] = 1; bottomHat[7] = 0;  bottomHat[8] = 1;
		topHat[0] = -1; topHat[1] = 0;  topHat[2] = -1;
		topHat[3] = 0;  topHat[4] = 4;  topHat[5] = 0;
		topHat[6] = -1; topHat[7] = 0;  topHat[8] = -1;
	}
	else {
		bottomHat[0] = 1; bottomHat[1] = 1;  bottomHat[2] = 1;
		bottomHat[3] = 1; bottomHat[4] = -8; bottomHat[5] = 1;
		bottomHat[6] = 1; bottomHat[7] = 1;  bottomHat[8] = 1;
		topHat[0] = -1;  topHat[1] = -1;  topHat[2] = -1;
		topHat[3] = -1;  topHat[4] = 8;   topHat[5] = -1;
		topHat[6] = -1;  topHat[7] = -1;  topHat[8] = -1;
	}

	// enlarge image by 1
	border = 1;
	ResizeCanvasMirror(border, border, border, border, tmpPixmap);

	xs = tmpPixmap.Width();
	ys = tmpPixmap.Height();
	out = (GUChar8 *)gPixels;
	in = (GUChar8 *)tmpPixmap.Pixels();
	for (i = border; i < ys - border; i++) {
		for (j = border; j < xs - border; j++) {
			p0 = in[(i - 1) * xs + (j - 1)];
			p1 = in[(i - 1) * xs + (j)];
			p2 = in[(i - 1) * xs + (j + 1)];
			p3 = in[(i) * xs + (j - 1)];
			p4 = in[(i) * xs + (j)];
			p5 = in[(i) * xs + (j + 1)];
			p6 = in[(i + 1) * xs + (j - 1)];
			p7 = in[(i + 1) * xs + (j)];
			p8 = in[(i + 1) * xs + (j + 1)];
			pixelOut = p4;
			bottomAcc = p0 * bottomHat[0] + p1 * bottomHat[1] + p2 * bottomHat[2] +
						p3 * bottomHat[3] + p4 * bottomHat[4] + p5 * bottomHat[5] +
						p6 * bottomHat[6] + p7 * bottomHat[7] + p8 * bottomHat[8];
			bottomAcc /= Divisor;
			topAcc = p0 * topHat[0] + p1 * topHat[1] + p2 * topHat[2] +
					 p3 * topHat[3] + p4 * topHat[4] + p5 * topHat[5] +
					 p6 * topHat[6] + p7 * topHat[7] + p8 * topHat[8];
			topAcc /= Divisor;
			bottomAcc = GMath::Clamp(bottomAcc, 0, 255);
			topAcc = GMath::Clamp(topAcc, 0, 255);
			pixelOut = (pixelOut + topAcc - bottomAcc);
			pixelOut = GMath::Clamp(pixelOut, 0, 255);
			*out++ = (GUChar8)pixelOut; 
		}
	}
	return G_NO_ERROR;
}

GError GPixelMap::EdgeEnhance(const GInt32 Divisor, const GBool Strong) {

	GError err = G_NO_ERROR;

	if (IsPaletted())
		return G_INVALID_OPERATION;

	if (IsGrayScale())
		err = EdgeEnhanceMono(Divisor, Strong);
	else {
		GPixelMap alphaChannel, redChannel, greenChannel, blueChannel;
		switch (gPixelFormat) {
			case G_RGB_PALETTE:
			case G_GRAYSCALE:
				// just to make gcc happy with warnings..
				break;
			case G_A8R8G8B8:
			case G_A1R5G5B5:
				err = SplitChannels(&alphaChannel, &redChannel, &greenChannel, &blueChannel);
				if (err == G_NO_ERROR) {
					err = alphaChannel.EdgeEnhanceMono(Divisor, Strong);
					if (err == G_NO_ERROR) {
						err = redChannel.EdgeEnhanceMono(Divisor, Strong);
						if (err == G_NO_ERROR) {
							err = greenChannel.EdgeEnhanceMono(Divisor, Strong);
							if (err == G_NO_ERROR) {
								err = blueChannel.EdgeEnhanceMono(Divisor, Strong);
								if (err == G_NO_ERROR)
									err = MergeChannels(redChannel, greenChannel, blueChannel, &alphaChannel);
							}
						}
					}
				}
				break;

			case G_R8G8B8:
			case G_R5G6B5:
				err = SplitChannels(NULL, &redChannel, &greenChannel, &blueChannel);
				if (err == G_NO_ERROR) {
					err = redChannel.EdgeEnhanceMono(Divisor, Strong);
					if (err == G_NO_ERROR) {
						err = greenChannel.EdgeEnhanceMono(Divisor, Strong);
						if (err == G_NO_ERROR) {
							err = blueChannel.EdgeEnhanceMono(Divisor, Strong);
							if (err == G_NO_ERROR)
								err = MergeChannels(redChannel, greenChannel, blueChannel, NULL);
						}
					}
				}
				break;
		}
	}
	return err;
}


GError GPixelMap::Load(const GChar8 *FileName, const GChar8 *Options, const GChar8 *FormatName) {

	if (Owner()) {
		GKernel *k = (GKernel *)Owner();
		return k->Load(FileName, *this, Options, FormatName);
	}
	return G_MISSING_KERNEL;
}

GError GPixelMap::Save(const GChar8 *FileName, const GChar8 *Options, const GChar8 *FormatName) {

	if (Owner()) {
		GKernel *k = (GKernel *)Owner();
		return k->Save(FileName, *this, Options, FormatName);
	}
	return G_MISSING_KERNEL;
}

// CMY -> KCMY
GUInt32 GPixelMap::CMYToKCMY(const GUInt32 CMY) {

	GUInt32 C = (CMY >> 16) & 0xFF;
	GUInt32 M = (CMY >> 8) & 0xFF;
	GUInt32 Y = CMY & 0xFF;
	GUInt32 K = 255;

	if (C < K)
		K = C;
	if (M < K)
		K = M;
	if (Y < K)
		K = Y;
	C = (256 * (C - K)) / (256 - K);
	M = (256 * (M - K)) / (256 - K);
	Y = (256 * (Y - K)) / (256 - K);
	return (Y | (M << 8) | (C << 16) | (K << 24));
}

// KCMY -> CMY
GUInt32 GPixelMap::KCMYToCMY(const GUInt32 KCMY) {

	GUInt32 Y = KCMY & 0xFF;
	GUInt32 M = (KCMY >> 8) & 0xFF;
	GUInt32 C = (KCMY >> 16) & 0xFF;
	GUInt32 K = (KCMY >> 24) & 0xFF;

	C = (C * ( 256 - K )) / 256 + K;
	M = (M * ( 256 - K )) / 256 + K;
	Y = (Y * ( 256 - K )) / 256 + K;
	return (Y | (M << 8) | (C << 16));
}

// RGB -> HSV
GUInt32 GPixelMap::RGBToHSV(const GUInt32 RGB) {

	GInt32 R = (GInt32)((RGB >> 16) & 0xFF);
	GInt32 G = (GInt32)((RGB >> 8) & 0xFF);
	GInt32 B = (GInt32)(RGB & 0xFF);

	GInt32 var_Min = (GInt32)(GMath::Min(R, G, B));
	GInt32 var_Max = (GInt32)(GMath::Max(R, G, B));
	GInt32 del_Max = var_Max - var_Min;
	GInt32 V = var_Max;
	GInt32 H = 0, S, del_R, del_G, del_B;
	// this is a gray, no chroma
	if (del_Max == 0) {
		H = 0;
		S = 0;
	}
	else {
		S = (256 * del_Max) / var_Max;
		del_R = (256 * (var_Max - R + 3 * del_Max)) / del_Max;
		del_G = (256 * (var_Max - G + 3 * del_Max)) / del_Max;
		del_B = (256 * (var_Max - B + 3 * del_Max)) / del_Max;
		if (R == var_Max)
			H = (del_B - del_G) / 6;
		else
		if (G == var_Max)
			H = 85 + (del_R - del_B) / 6;
		else
		if (B == var_Max)
			H = 170 + (del_G - del_R) / 6;
		if (H < 0)
			H += 255;
		if (H > 255)
			H -= 255;
	}
	return (V | (S << 8) | (H << 16));
}

// RGB -> HSL
GUInt32 GPixelMap::RGBToHSL(const GUInt32 RGB) {

	GInt32 R = (GInt32)((RGB >> 16) & 0xFF);
	GInt32 G = (GInt32)((RGB >> 8) & 0xFF);
	GInt32 B = (GInt32)(RGB & 0xFF);
	GInt32 var_Min = (GInt32)(GMath::Min(R, G, B));
	GInt32 var_Max = (GInt32)(GMath::Max(R, G, B));
	GInt32 del_Max = var_Max - var_Min;

	GInt32 H = 0, S, del_R, del_G, del_B;
	GInt32 L = (var_Max + var_Min) / 2;
	// this is a gray, no chroma
	if (del_Max == 0 ) {
		H = 0;
		S = 0;
	}
	else {
		if (L < 128)
			S = (256 * del_Max) / (var_Max + var_Min);
		else
			S = (256 * del_Max) / (511 - var_Max - var_Min);
		del_R = (256 * (var_Max - R + 3 * del_Max)) / del_Max;
		del_G = (256 * (var_Max - G + 3 * del_Max)) / del_Max;
		del_B = (256 * (var_Max - B + 3 * del_Max)) / del_Max;
		if (R == var_Max)
			H = (del_B - del_G) / 6;
		else
		if (G == var_Max)
			H = 85 + (del_R - del_B) / 6;
		else
		if (B == var_Max)
			H = 170 + (del_G - del_R) / 6;
		if (H < 0)
			H += 255;
		if (H > 255)
			H -= 255;
	}
	return (L | (S << 8) | (H << 16));
}

GUInt32 GPixelMap::HSLToRGB(const GUInt32 HSL) {

	GUInt32 H = (HSL >> 16) & 0xFF;
	GUInt32 S = (HSL >> 8) & 0xFF;
	GUInt32 L = HSL & 0xFF;
	GInt32 vH, R = 0, G = 0, B = 0, var_1, var_2;

	if (S == 0) {
		R = L;
		G = L;
		B = L;
	}
	else {
		if (L < 128)
			var_2 = L * (255 + S);
		else
			var_2 = (L + S) - (S * L);

		var_1 = 512 * L - var_2;
		// red component
		vH = H + 85;
		if (vH < 0)
			vH += 255;
		if (vH > 255)
			vH -= 255;
		if (6 * vH  < 255)
			R = (var_1 + (((var_2 - var_1) * 6 * vH) / 256)) / 256;
		else
		if (2 * vH < 255)
			R = var_2 / 256;
		else
		if (3 * vH < 512)
			R = (var_1 + ((var_2 - var_1) * (170 - vH) * 6) / 256) / 256;
		// green component
		vH = H;
		if (vH < 0)
			vH += 255;
		if (vH > 255)
			vH -= 255;
		if (6 * vH  < 255)
			G = (var_1 + (((var_2 - var_1) * 6 * vH) / 256)) / 256;
		else
		if (2 * vH < 255)
			G = var_2 / 256;
		else
		if (3 * vH < 512)
			G = (var_1 + ((var_2 - var_1) * (170 - vH) * 6) / 256) / 256;
		// blue component
		vH = H - 85;
		if (vH < 0)
			vH += 255;
		if (vH > 255)
			vH -= 255;
		if (6 * vH  < 255)
			B = (var_1 + (((var_2 - var_1) * 6 * vH) / 256)) / 256;
		else
		if (2 * vH < 255)
			B = var_2 / 256;
		else
		if (3 * vH < 512)
			B = (var_1 + ((var_2 - var_1) * (170 - vH) * 6) / 256) / 256;
	}
	return (B | (G << 8) | (R << 16));
}


GUInt32 GPixelMap::HSVToRGB(const GUInt32 HSV) {

	GInt32 H = (GInt32)((HSV >> 16) & 0xFF);
	GInt32 S = (GInt32)((HSV >> 8) & 0xFF);
	GInt32 V = (GInt32)(HSV & 0xFF);
	GInt32 R, G, B, var_1, var_2, var_3, var_i, var_h, delta;

	if (S == 0) {
		R = V;
		G = V;
		B = V;
	}
	else {
		var_h = (H * 6 * 65536) / 256;
		var_i = var_h & 0xFFFF0000;
		// delta is floor(var_h) in [0; 255]
		delta = (var_h - var_i) / 256;
		var_1 = V * (255 - S);
		var_2 = V * (255 - ((S * delta)) / 256);
		var_3 = V * (255 - ((S * (255 - delta)) / 256));
		if (var_i == 0) {
			R = V;
			G = var_3 / 256;
			B = var_1 / 256;
		}
		else
		if (var_i == 65536) {
			R = var_2 / 256;
			G = V;
			B = var_1 / 256;
		}
		else
		if (var_i == 2 * 65536) {
			R = var_1 / 256;
			G = V;
			B = var_3 / 256;
		}
		else
		if (var_i == 3 * 65536) {
			R = var_1 / 256;
			G = var_2 / 256;
			B = V;
		}
		else
		if (var_i == 4 * 65536) {
			R = var_3 / 256;
			G = var_1 / 256;
			B = V;
		}
		else {
			R = V;
			G = var_1 / 256;
			B = var_2 / 256;
		}
	}
	return (B | (G << 8) | (R << 16));
}

// calculate hue distance (caring of hue wrapping)
GInt32 GPixelMap::HueDistance(const GUChar8 Hue1, const GUChar8 Hue2) {

	GInt32 d1 = (GInt32)255 - (GInt32)Hue1;
	GInt32 d2 = (GInt32)255 - (GInt32)Hue2;
	GInt32 d, dOK;

	if (d1 < d2)
		d = d1 + Hue2;
	else
		d = Hue1 + d2;

	dOK = GMath::Min(d, 255 - d);

	if (((Hue2 + dOK) % 255) == Hue1)
		return -dOK;
	else
		return dOK;
}

// get a pixel
GError GPixelMap::Pixel(const GUInt32 X, const GUInt32 Y, GUInt32& Index_Or_A8R8G8B8) const {

	if (X >= gWidth || Y >= gHeight) {
		Index_Or_A8R8G8B8 = 0;
		return G_OUT_OF_RANGE;
	}

	GUChar8 *pixel8;
	GUInt16 *pixel16;
	GUInt32 *pixel32;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
		case G_GRAYSCALE:
			pixel8 = (GUChar8 *)gPixels;
			Index_Or_A8R8G8B8 = (GUInt32)(pixel8[Y * gWidth + X]);
			break;
		case G_R8G8B8:
		case G_A8R8G8B8:
			pixel32 = (GUInt32 *)gPixels;
			Index_Or_A8R8G8B8 = pixel32[(Y * gWidth + X)];
			break;
		case G_A1R5G5B5:
		case G_R5G6B5:
			pixel16 = (GUInt16 *)gPixels;
			Index_Or_A8R8G8B8 = pixel16[(Y * gWidth + X)];
			break;
	}
	return G_NO_ERROR;
}

// set a pixel color
GError GPixelMap::SetPixel(const GUInt32 X, const GUInt32 Y, const GUInt32 Index_Or_A8R8G8B8) {

	if (X >= gWidth || Y >= gHeight)
		return G_OUT_OF_RANGE;

	GUChar8 *pixel8;
	GUInt16 *pixel16;
	GUInt32 *pixel32;

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
		case G_GRAYSCALE:
			pixel8 = (GUChar8 *)gPixels;
			pixel8[Y * gWidth + X] = (GUChar8)Index_Or_A8R8G8B8;
			break;
		case G_R8G8B8:
		case G_A8R8G8B8:
			pixel32 = (GUInt32 *)gPixels;
			pixel32[(Y * gWidth + X)] = Index_Or_A8R8G8B8;
			break;
		case G_A1R5G5B5:
		case G_R5G6B5:
			pixel16 = (GUInt16 *)gPixels;
			pixel16[(Y * gWidth + X)] = (GUInt16)Index_Or_A8R8G8B8;
			break;
	}
	return G_NO_ERROR;
}

// split RGB channels into single images
GError GPixelMap::SplitChannels(GPixelMap *AlphaImage, GPixelMap *RedImage, GPixelMap *GreenImage,
								GPixelMap *BlueImage) const {

	GUInt16 *pixels16, argb16;
	GUInt32 *pixels32;
	GUChar8 a8, r8, g8, b8;
	GError err;
	GUChar8 *alphaPixels = NULL, *redPixels = NULL, *greenPixels = NULL, *bluePixels = NULL;
	GInt32 i, j, argb32;

	if (IsGrayScale() || IsPaletted())
		return G_INVALID_OPERATION;

	j = PixelsCount();
	if (j <= 0)
		return G_NO_ERROR;

	if (AlphaImage) {
		err = AlphaImage->Reset(gWidth, gHeight, G_GRAYSCALE);
		if (err != G_NO_ERROR)
			return err;
		alphaPixels = (GUChar8 *)AlphaImage->gPixels;
	}
	if (RedImage) {
		err = RedImage->Reset(gWidth, gHeight, G_GRAYSCALE);
		if (err != G_NO_ERROR)
			return err;
		redPixels = (GUChar8 *)RedImage->gPixels;
	}
	if (GreenImage) {
		err = GreenImage->Reset(gWidth, gHeight, G_GRAYSCALE);
		if (err != G_NO_ERROR)
			return err;
		greenPixels = (GUChar8 *)GreenImage->gPixels;
	}
	if (BlueImage) {
		err = BlueImage->Reset(gWidth, gHeight, G_GRAYSCALE);
		if (err != G_NO_ERROR)
			return err;
		bluePixels = (GUChar8 *)BlueImage->gPixels;
	}

	switch (gPixelFormat) {
		case G_RGB_PALETTE:
		case G_GRAYSCALE:
			// just to make gcc happy with warnings..
			break;
		case G_R8G8B8:
		case G_A8R8G8B8:
			pixels32 = (GUInt32 *)gPixels;
			for (i = 0; i < j; i++) {
				argb32 = *pixels32++;
				if (AlphaImage) {
					a8 = (GUChar8)((argb32 >> 24) & 0xFF);
					*alphaPixels++ = a8;
				}
				if (RedImage) {
					r8 = (GUChar8)((argb32 >> 16) & 0xFF);
					*redPixels++ = r8;
				}
				if (GreenImage) {
					g8 = (GUChar8)((argb32 >> 8) & 0xFF);
					*greenPixels++ = g8;
				}
				if (BlueImage) {
					b8 = (GUChar8)(argb32 & 0xFF);
					*bluePixels++ = b8;
				}
			}
			break;

		case G_A1R5G5B5:
			pixels16 = (GUInt16 *)gPixels;
			for (i = 0; i < j; i++) {
				argb16 = *pixels16++;
				if (AlphaImage) {
					a8 = (GUChar8)((argb16 >> 15) & 1);
					*alphaPixels++ = (a8 << 7);
				}
				if (RedImage) {
					r8 = (GUChar8)((argb16 >> 10) & 31);
					*redPixels++ = (r8 << 3);
				}
				if (GreenImage) {
					g8 = (GUChar8)((argb16 >> 5) & 31);
					*greenPixels++ = (g8 << 3);
				}
				if (BlueImage) {
					b8 = (GUChar8)(argb16 & 31);
					*bluePixels++ = (b8 << 3);
				}
			}
			break;

		case G_R5G6B5:
			pixels16 = (GUInt16 *)gPixels;
			for (i = 0; i < j; i++) {
				argb16 = *pixels16++;
				if (AlphaImage)
					*alphaPixels++ = 0;
				if (RedImage) {
					r8 = (GUChar8)((argb16 >> 11) & 31);
					*redPixels++ = (r8 << 3);
				}
				if (GreenImage) {
					g8 = (GUChar8)((argb16 >> 5) & 63);
					*greenPixels++ = (g8 << 2);
				}
				if (BlueImage) {
					b8 = (GUChar8)(argb16 & 31);
					*bluePixels++ = (b8 << 3);
				}
			}
			break;
	}
	return G_NO_ERROR;
}

// merge RGB and Alpha channels to build final image
GError GPixelMap::MergeChannels(const GPixelMap& RedImage, const GPixelMap& GreenImage, const GPixelMap& BlueImage,
								const GPixelMap *AlphaImage) {

	GUInt32 *pixels32;
	GUInt32 a32, r32, g32, b32;
	GError err;
	GUChar8 *alphaPixels = NULL, *redPixels, *greenPixels, *bluePixels;
	GInt32 i, j, width, height;

	if (!RedImage.IsGrayScale() || !GreenImage.IsGrayScale() || !BlueImage.IsGrayScale())
		return G_INVALID_PARAMETER;

	width = RedImage.Width();
	height = RedImage.Height();
	if (width != GreenImage.Width() || height != GreenImage.Height())
		return G_INVALID_PARAMETER;
	if (width != BlueImage.Width() || height != BlueImage.Height())
		return G_INVALID_PARAMETER;

	if (AlphaImage) {
		if (!AlphaImage->IsGrayScale())
			return G_INVALID_PARAMETER;
		if (width != AlphaImage->Width() || height != AlphaImage->Height())
			return G_INVALID_PARAMETER;
	}

	if (AlphaImage)
		err = Reset(width, height, G_A8R8G8B8);
	else
		err = Reset(width, height, G_R8G8B8);

	if (err != G_NO_ERROR)
		return err;

	redPixels = (GUChar8 *)RedImage.gPixels;
	greenPixels = (GUChar8 *)GreenImage.gPixels;
	bluePixels = (GUChar8 *)BlueImage.gPixels;
	if (AlphaImage)
		alphaPixels = (GUChar8 *)AlphaImage->gPixels;
	pixels32 = (GUInt32 *)gPixels;

	j = PixelsCount();
	if (AlphaImage) {
		for (i = 0; i < j; i++) {
			a32 = (GUInt32)*alphaPixels;
			r32 = (GUInt32)*redPixels;
			g32 = (GUInt32)*greenPixels;
			b32 = (GUInt32)*bluePixels;
			*pixels32++ = (a32 << 24) | (r32 << 16) | (g32 << 8) | b32;
			alphaPixels++;
			redPixels++;
			greenPixels++;
			bluePixels++;
		}
	}
	else {
		for (i = 0; i < j; i++) {
			r32 = (GUInt32)*redPixels;
			g32 = (GUInt32)*greenPixels;
			b32 = (GUInt32)*bluePixels;
			*pixels32++ = (r32 << 16) | (g32 << 8) | b32;
			redPixels++;
			greenPixels++;
			bluePixels++;
		}
	}
	return G_NO_ERROR;
}

}
