/****************************************************************************
** $file: amanith/src/rendering/gopenglcache.cpp   0.2.0.0   edited Dec, 12 2005
**
** OpenGL based draw board cache functions implementation.
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
#include "amanith/geometry/gxform.h"
#include "amanith/geometry/gxformconv.h"

/*!
	\file gopenglcache.cpp
	\brief OpenGL based draw board cache functions implementation file.
*/

namespace Amanith {


// *********************************************************************
//                        GOpenGLCacheBank
// *********************************************************************

// invalidate the cache, freeing associated (video) memory
void GOpenGLCacheBank::Invalidate() {

	GOpenGLCacheSlot slot;
	GUInt32 i, j = (GUInt32)gSlots.size();

	for (i = 0; i < j; ++i) {
		slot = gSlots[i];
		if (slot.FillDisplayList != 0)
			glDeleteLists(slot.FillDisplayList, 1);
		if (slot.StrokeDisplayList != 0)
			glDeleteLists(slot.StrokeDisplayList, 1);
	}
	gSlots.clear();
}

// constructor, it build an empty cache slot
GOpenGLCacheBank::GOpenGLCacheBank() {
}

// destructor, it invalidates cached shapes and frees memory
GOpenGLCacheBank::~GOpenGLCacheBank() {

	Invalidate();
}

// get the number of cached shapes
GUInt32 GOpenGLCacheBank::SlotsCount() const {

	return ((GUInt32)gSlots.size());
}


// *********************************************************************
//                             GOpenGLBoard
// *********************************************************************

void GOpenGLBoard::DoDrawCacheSlot(const GDrawStyle& Style, const GOpenGLCacheSlot& CacheSlot) {

	if (CacheSlot.FillDisplayList == 0 && CacheSlot.StrokeDisplayList == 0)
		return;
	// draw cached primitives on cache has no effects
	if (TargetMode() == G_CACHE_MODE)
		return;

#define DRAW_STROKE \
	G_ASSERT(CacheSlot.StrokeDisplayList != 0); \
	glCallList(CacheSlot.StrokeDisplayList);

#define DRAW_FILL \
	if (CacheSlot.FillDisplayList != 0) \
	glCallList(CacheSlot.FillDisplayList);

	// calculate bound box
	GAABox2 tmpBox(CacheSlot.Box);

	GBool doublePass = SetGLClipEnabled(TargetMode(), ClipOperation());

	if (TargetMode() == G_CLIP_MODE || TargetMode() == G_CLIP_AND_CACHE_MODE) {

		// draw fill
		if (Style.FillEnabled()) {
			DRAW_FILL
		}
		// draw stroke
		if (Style.StrokeEnabled()) {
			DRAW_STROKE
		}
		// take care of replace operation
		if (!InsideGroup())
			UpdateClipMasksState();

		if (!InsideGroup())
			gClipMasksBoxes.push_back(tmpBox);
		else {
			// build initial group box
			if (gIsFirstGroupDrawing)
				gGroupBox = tmpBox;
			else {
				// expand group box
				gGroupBox.ExtendToInclude(tmpBox.Min());
				gGroupBox.ExtendToInclude(tmpBox.Max());
			}
		}
		gIsFirstGroupDrawing = G_FALSE;
		return;
	}

	// in color mode, if we are inside a GroupBegin() / GroupEnd() constructor and group opacity is 0
	// do not draw anything
	if (InsideGroup() && GroupOpacity() <= 0 && gGroupOpacitySupport)
		return;

	if (Style.FillEnabled()) {
		// set fill style using OpenGL
		GBool useDepth = UseFillStyle(Style);
		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			// draw fill
			DRAW_FILL
				StencilEnableTop();
		}
		if (useDepth)
			PushDepthMask();
		// draw fill
		DRAW_FILL
			// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
			if (useDepth)
				DrawAndPopDepthMask(tmpBox, Style, G_TRUE);
	}

	// take care to enable stencil test if necessary
	SetGLClipEnabled(TargetMode(), ClipOperation());

	if (Style.StrokeEnabled()) {
		// set stroke style using OpenGL
		GBool useDepth = UseStrokeStyle(Style);
		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			// draw stroke
			DRAW_STROKE
				StencilEnableTop();
		}
		if (useDepth)
			PushDepthMask();
		// draw stroke
		DRAW_STROKE
			// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
			if (useDepth)
				DrawAndPopDepthMask(tmpBox, Style, G_FALSE);
	}

#undef DRAW_STROKE
#undef DRAW_FILL
}

void GOpenGLBoard::DoDrawCacheSlots(GDrawStyle& Style, const GUInt32 FirstSlotIndex, const GUInt32 LastSlotIndex) {

	if (!Style.StrokeEnabled() && !Style.FillEnabled())
		return;

	// update style
	UpdateStyle((GOpenGLDrawStyle&)Style);

	GOpenGLCacheBank *bank = (GOpenGLCacheBank *)CacheBank();
	G_ASSERT(bank != NULL);

	for (GUInt32 i = FirstSlotIndex; i <= LastSlotIndex; ++i)
		DoDrawCacheSlot(Style, bank->gSlots[i]);
}

GCacheBank *GOpenGLBoard::CreateCacheBank() {

	GOpenGLCacheBank *bank = new(std::nothrow) GOpenGLCacheBank();
	if (bank)
		gCacheBanks.push_back(bank);
	return (GCacheBank *)bank;
}

GCacheBank *GOpenGLBoard::CacheBank() const {

	return (GCacheBank *)gCacheBank;
}

void GOpenGLBoard::SetCacheBank(GCacheBank *Bank) {

	gCacheBank = (GOpenGLCacheBank *)Bank;
}

};	// end namespace Amanith
