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
GInt32 GOpenGLCacheBank::SlotsCount() const {

	return ((GInt32)gSlots.size());
}


// *********************************************************************
//                             GOpenGLBoard
// *********************************************************************

void GOpenGLBoard::DoDrawCacheSlot(const GDrawStyle& Style, const GOpenGLCacheSlot& CacheSlot) {

	// if we had to draw nothing, just exit
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

	GBool stencilPass = SetGLClipEnabled(TargetMode(), ClipOperation());

	if (TargetMode() == G_CLIP_MODE || TargetMode() == G_CLIP_AND_CACHE_MODE) {

		glMatrixMode(GL_MODELVIEW);
		SetGLModelViewMatrix(ModelViewMatrix());

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

		// calculate/update the shape box, according to model-view matrix
		GAABox2 mvBox;
		UpdateBox(tmpBox, ModelViewMatrix(), mvBox);

		if (!InsideGroup())
			gClipMasksBoxes.push_back(mvBox);
		else {
			// build initial group box
			if (gIsFirstGroupDrawing)
				gGroupBox = mvBox;
			else {
				// expand group box
				gGroupBox.ExtendToInclude(mvBox.Min());
				gGroupBox.ExtendToInclude(mvBox.Max());
			}
		}
		gIsFirstGroupDrawing = G_FALSE;
		return;
	}

	// if we are inside a group and the GroupCompOp() is DST_OP we have to draw nothing
	if (InsideGroup() && GroupCompOp() == G_DST_OP && gGroupOpacitySupport)
		return;

	// calculate/update the shape box, according to model-view matrix
	GAABox2 mvBox;
	UpdateBox(tmpBox, ModelViewMatrix(), mvBox);

	if (Style.FillEnabled() && Style.FillCompOp() != G_DST_OP) {

		// now count the number of passes needed by current compositing operation, and see if we have to grab framebuffer
		GUInt32 stylePassesCount = 0;
		GUInt32 fbPassesCount = 0;
		GBool needGrab = CompOpPassesCount(Style.FillCompOp(), stylePassesCount, fbPassesCount);

		// grab frame buffer to do compositing, if needed
		//GGenericAABox<GInt32, 2> physicBox;
		if (needGrab) {
			//physicBox.SetMinMax(LogicalToPhysicalInt(mvBox.Min()), LogicalToPhysicalInt(mvBox.Max()));
			// we must add a 1pixel of border just to avoid visual artifacts
			//GrabFrameBuffer(physicBox.Min() + GVect<GInt32, 2>(-1, -1),
			//				physicBox.Dimension(G_X) + 2, physicBox.Dimension(G_Y) + 2, gCompositingBuffer);
			GrabFrameBuffer(mvBox, gCompositingBuffer);
		}

		// set fill style using OpenGL
		GBool useDepthForFill = NeedDepthMask((const GOpenGLDrawStyle &)Style, G_TRUE);
		GBool depthPass = needGrab || useDepthForFill;

		// for those compositing operations that need a grab, we have to do a first pass on zbuffer; here we also
		// take care of group opacity and some compositing operations, first we have to write into stencil buffer
		if (stencilPass || depthPass) {

			GLDisableShaders();
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			if (stencilPass && !depthPass) {
				StencilNoDepthWrite();
				glMatrixMode(GL_MODELVIEW);
				SetGLModelViewMatrix(ModelViewMatrix());
				DRAW_FILL
			}
			else
			if (!stencilPass && depthPass) {
				StencilEnableTop();
				DepthNoStencilWrite();
				// use model-view matrix to draw the mask
				glMatrixMode(GL_MODELVIEW);
				SetGLModelViewMatrix(ModelViewMatrix());
				DRAW_FILL
			}
			else {
				StencilEnableTop();
				DepthNoStencilWrite();
				// use model-view matrix to draw the mask
				glMatrixMode(GL_MODELVIEW);
				SetGLModelViewMatrix(ModelViewMatrix());
				DRAW_FILL

				StencilWhereDepthEqual();
				// use identity to draw the logical box (already transformed with model-view matrix)
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				DrawGLBox(mvBox);
			}
		}

		if (depthPass)
			// geometric radial/conical gradient and transparent entities uses depth clip, so we must pop off clip mask
			// here we take into account compositing operations that need to grab framebuffer
			DrawAndPopDepthMask(mvBox, Style, G_TRUE, stylePassesCount, fbPassesCount, needGrab);		
		else {
			G_ASSERT(fbPassesCount == 0);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
			StencilEnableTop();

			glMatrixMode(GL_MODELVIEW);
			SetGLModelViewMatrix(ModelViewMatrix());
			for (GUInt32 ii = 0; ii < stylePassesCount; ++ii) {
				// draw fill, using specified style and model-view matrix
				UseFillStyle(Style, ii);
				DRAW_FILL
			}
		}
	}


	if (Style.StrokeEnabled() && Style.StrokeCompOp() != G_DST_OP) {

		// now count the number of passes needed by current compositing operation, and see if we have to grab framebuffer
		GUInt32 stylePassesCount = 0;
		GUInt32 fbPassesCount = 0;
		GBool needGrab = CompOpPassesCount(Style.StrokeCompOp(), stylePassesCount, fbPassesCount);

		// grab frame buffer to do compositing, if needed
		//GGenericAABox<GInt32, 2> physicBox;
		if (needGrab) {
			//physicBox.SetMinMax(LogicalToPhysicalInt(mvBox.Min()), LogicalToPhysicalInt(mvBox.Max()));
			// we must add a 1pixel of border just to avoid visual artifacts
			//GrabFrameBuffer(physicBox.Min() + GVect<GInt32, 2>(-1, -1),
			//				physicBox.Dimension(G_X) + 2, physicBox.Dimension(G_Y) + 2, gCompositingBuffer);
			GrabFrameBuffer(mvBox, gCompositingBuffer);
		}

		// set stroke style using OpenGL
		GBool useDepthForStroke = NeedDepthMask((const GOpenGLDrawStyle &)Style, G_FALSE);
		GBool depthPass = needGrab || useDepthForStroke;
		
		// for those compositing operations that need a grab, we have to do a first pass on zbuffer; here we also
		// take care of group opacity and some compositing operations, first we have to write into stencil buffer
		if (stencilPass || depthPass) {
			GLDisableShaders();
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			if (stencilPass && !depthPass) {
				StencilNoDepthWrite();
				glMatrixMode(GL_MODELVIEW);
				SetGLModelViewMatrix(ModelViewMatrix());
				DRAW_STROKE
			}
			else
			if (!stencilPass && depthPass) {
				StencilEnableTop();
				DepthNoStencilWrite();
				// use model-view matrix to draw the mask
				glMatrixMode(GL_MODELVIEW);
				SetGLModelViewMatrix(ModelViewMatrix());
				DRAW_STROKE
			}
			else {
				StencilEnableTop();
				DepthNoStencilWrite();
				// use model-view matrix to draw the mask
				glMatrixMode(GL_MODELVIEW);
				SetGLModelViewMatrix(ModelViewMatrix());
				DRAW_STROKE

				StencilWhereDepthEqual();
				// use identity to draw the logical box (already transformed with model-view matrix)
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				DrawGLBox(mvBox);
			}
		}

		if (depthPass)
			// geometric radial/conical gradient and transparent entities uses depth clip, so we must pop off clip mask
			// here we take into account compositing operations that need to grab framebuffer
			DrawAndPopDepthMask(mvBox, Style, G_FALSE, stylePassesCount, fbPassesCount, needGrab);
		else {
			G_ASSERT(fbPassesCount == 0);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
			StencilEnableTop();

			glMatrixMode(GL_MODELVIEW);
			SetGLModelViewMatrix(ModelViewMatrix());
			for (GUInt32 ii = 0; ii < stylePassesCount; ++ii) {
				// draw line segment, using specified style and model-view matrix
				UseStrokeStyle(Style, ii);
				DRAW_STROKE
			}
		}
	}
	#undef DRAW_STROKE
	#undef DRAW_FILL
}

void GOpenGLBoard::DoDrawCacheSlots(GDrawStyle& Style, const GInt32 FirstSlotIndex, const GInt32 LastSlotIndex) {

	if (!Style.StrokeEnabled() && !Style.FillEnabled())
		return;

	// update style
	UpdateStyle((GOpenGLDrawStyle&)Style);

	GOpenGLCacheBank *bank = (GOpenGLCacheBank *)CacheBank();
	G_ASSERT(bank != NULL);

	for (GInt32 i = FirstSlotIndex; i <= LastSlotIndex; ++i)
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
