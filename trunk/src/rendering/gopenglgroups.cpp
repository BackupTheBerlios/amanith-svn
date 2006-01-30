/****************************************************************************
** $file: amanith/src/rendering/gopenglgroups.cpp   0.3.0.0   edited Jan, 30 2006
**
** OpenGL based draw board group functions implementation.
**
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

#include "amanith/rendering/gopenglboard.h"

/*!
	\file gopenglgroups.cpp
	\brief OpenGL based draw board group functions implementation file.
*/

namespace Amanith {

void GOpenGLBoard::DoGroupBegin(const GAABox2& LogicBox) {


	gIsFirstGroupDrawing = G_TRUE;

	// if group opacity is not supported by hardware or group compositing operation is DST_OP just exit; the case of
	// CLEAR_OP is the same, a black box will be drawn inside DoGroupEnd()
	if (!gGroupOpacitySupport || GroupCompOp() == G_DST_OP)
		return;

	// group begin affects only color buffer
	if (TargetMode() == G_CACHE_MODE || TargetMode() == G_CLIP_MODE || TargetMode() == G_CLIP_AND_CACHE_MODE)
		return;

	GrabFrameBuffer(LogicBox, gGLGroupRect);

	GReal ll, rr, bb, tt;
	Projection(ll, rr, bb, tt);
	GMatrix44 m = GLProjectionMatrix(ll, rr, bb, tt, 1);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	#ifdef DOUBLE_REAL_TYPE 
		glLoadMatrixd((const GLdouble *)m.Data());
	#else
		glLoadMatrixf((const GLfloat *)m.Data());
	#endif
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	GLDisableShaders();
	SetGLColor(GVector4(1, 1, 1, 0));
	// enable texture 2D
	SELECT_AND_DISABLE_TUNIT(0)
	glDisable(GL_BLEND);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	
	DrawGLBox(gGLGroupRect.gExpandedLogicBox);

	// now intersect bounding box with current mask(s)
	if (ClipEnabled()) {
		StencilPush();
		DrawGLBox(gGLGroupRect.gExpandedLogicBox);
		// increment top stencil value because StencilPush checks for InsideGroup() flag; gTopStencilValue is
		// incremented only if we are not inside e group (here we are already in a group, because we have just
		// entered it)
		gTopStencilValue++;
	}

	// exit from window-mode
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void GOpenGLBoard::DoGroupEnd() {

	if (TargetMode() == G_CACHE_MODE)
		return;

	if ((TargetMode() == G_CLIP_MODE || TargetMode() == G_CLIP_AND_CACHE_MODE) && gIsFirstGroupDrawing == G_FALSE) {
		UpdateClipMasksState();
		gClipMasksBoxes.push_back(gGroupBox);
		if (gTopStencilValue < gMaxTopStencilValue) {
			if (ClipOperation() == G_INTERSECTION_CLIP)
				gTopStencilValue++;
			else {
				// in the case of replace operation, gTopStencilValue has been already incremented by
				// the first drawing operation
			}
		}
	}
	
	gIsFirstGroupDrawing = G_FALSE;

	// if group opacity is not supported by hardware or group compositing operation is DST_OP or group is
	// empty just exit
	if (gGLGroupRect.IsEmpty || !gGroupOpacitySupport || GroupCompOp() == G_DST_OP)
		return;
	// DoGroupEnd() affects only color buffer
	if (TargetMode() == G_CACHE_MODE || TargetMode() == G_CLIP_MODE || TargetMode() == G_CLIP_AND_CACHE_MODE)
		return;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_STENCIL_TEST);

	GrabFrameBuffer(gGLGroupRect.gNotExpandedLogicBox, gCompositingBuffer);

	// use SRC_OP just to disable blend and enable all 4 channels color mask
	ReplaceFrameBuffer(gGLGroupRect, G_SRC_OP, 0);

	glEnable(GL_STENCIL_TEST);
	if (ClipEnabled()) {
		gTopStencilValue++;
		glStencilFunc(GL_EQUAL, gTopStencilValue, gStencilMask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	}
	else {
		glStencilFunc(GL_EQUAL, (GLint)(0x7FFFFFFF), gStencilDualMask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	}

	if (GroupCompOp() == G_CLEAR_OP) {

		SELECT_AND_DISABLE_TUNIT(0)
		GLDisableShaders();
		glDisable(GL_BLEND);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		SetGLColor(GVector4(0, 0, 0, 0));
		DrawGLBox(gGLGroupRect.gExpandedLogicBox);
	}
	else {
		// simulate the drawing of a rectangle with GroupCompOp() and only fill
		GUInt32 stylePassesCount = 0;
		GUInt32 fbPassesCount = 0;
		CompOpPassesCount(GroupCompOp(), stylePassesCount, fbPassesCount);

		for (GUInt32 ii = 0; ii < stylePassesCount; ++ii) {
			UseGroupStyle(ii, gCompositingBuffer, gGLGroupRect);
			G_ASSERT(gCompositingBuffer.Width == gGLGroupRect.Width);
			G_ASSERT(gCompositingBuffer.Height == gGLGroupRect.Height);
			G_ASSERT(gCompositingBuffer.Target == gGLGroupRect.Target);
			DrawGrabbedRect(gCompositingBuffer, G_TRUE, G_TRUE, G_TRUE, G_FALSE);
		}
		for (GUInt32 ii = 0; ii < fbPassesCount; ++ii)
			ReplaceFrameBuffer(gGLGroupRect, GroupCompOp(), ii);
	}


	GLDisableShaders();
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	if (ClipEnabled()) {
		// delete stencil drawn pixels mask
		StencilPop();
		DrawGLBox(gGLGroupRect.gExpandedLogicBox);
		// delete stencil bounding box mask
		StencilPop();
		DrawGLBox(gGLGroupRect.gExpandedLogicBox);
	}
	else {
		glStencilFunc(GL_EQUAL, (GLint)(0x7FFFFFFF), gStencilDualMask);
		glStencilOp(GL_KEEP, GL_ZERO, GL_ZERO);
		glStencilMask(gStencilDualMask);
		DrawGLBox(gGLGroupRect.gExpandedLogicBox);
	}
}

void GOpenGLBoard::UpdateGrabBuffer(const GUInt32 Width, const GUInt32 Height, GLGrabbedRect& GrabRect) {

	G_ASSERT(Width > 0 && Height > 0);

	// check if we have to expand grabbing buffer
	if (Width > GrabRect.TexWidth || Height > GrabRect.TexHeight) {

		// check if we have to create OpenGL texture
		if (GrabRect.TexName == 0) {
			glGenTextures(1, &GrabRect.TexName);
			G_ASSERT(GrabRect.TexName > 0);
			if (gRectTexturesInUse)
				GrabRect.Target = GL_TEXTURE_RECTANGLE_EXT;
			else
				GrabRect.Target = GL_TEXTURE_2D;
		}

		// bind texture and calculate new dimensions
		SELECT_AND_DISABLE_TUNIT(1)
		SELECT_AND_DISABLE_TUNIT(0)
		glEnable(GrabRect.Target);
		glBindTexture(GrabRect.Target, GrabRect.TexName);
		if (GrabRect.Target == GL_TEXTURE_RECTANGLE_EXT) {
			GrabRect.TexWidth = Width;
			GrabRect.TexHeight = Height;
		}
		else {
			GrabRect.TexWidth = GOpenglExt::PowerOfTwo(Width);
			GrabRect.TexHeight = GOpenglExt::PowerOfTwo(Height);
		}
		// we must ensure that texture must not be larger than the maximum (hw)permitted size
		GUInt32 maxTexSize = gExtManager->MaxTextureSize();
		if (GrabRect.TexWidth > maxTexSize)
			GrabRect.TexWidth = maxTexSize;
		if (GrabRect.TexHeight > maxTexSize)
			GrabRect.TexHeight = maxTexSize;

		// set texture parameters
		glTexParameteri(GrabRect.Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
		glTexParameteri(GrabRect.Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GrabRect.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GrabRect.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// create/enlarge texture
		glTexImage2D(GrabRect.Target, 0, GL_RGBA8, GrabRect.TexWidth, GrabRect.TexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		GrabRect.Format = GL_RGBA8;
	}
}

void GOpenGLBoard::GrabFrameBuffer(const GAABox2& LogicBox, GLGrabbedRect& Shot) {

	GLDisableShaders();

	GReal left, right, bottom, top;
	Projection(left, right, bottom, top);

	if (LogicBox.Min()[G_X] > left)
		left = LogicBox.Min()[G_X];
	if (LogicBox.Max()[G_X] < right)
		right = LogicBox.Max()[G_X];
	if (LogicBox.Min()[G_Y] > bottom)
		bottom = LogicBox.Min()[G_Y];
	if (LogicBox.Max()[G_Y] < top)
		top = LogicBox.Max()[G_Y];

	GAABox2 tmpBox(GPoint2(left, bottom), GPoint2(right, top));

	GPoint<GInt32, 2> p0 = LogicalToPhysicalInt(tmpBox.Min());
	GPoint<GInt32, 2> p1 = LogicalToPhysicalInt(tmpBox.Max());
	p0[G_X] -= 1;
	p0[G_Y] -= 1;
	p1[G_X] += 1;
	p1[G_Y] += 1;
	GGenericAABox<GInt32, 2> intBox(p0, p1);

	GUInt32 width = (GUInt32)GMath::Abs(p1[G_X] - p0[G_X]);
	GUInt32 height = (GUInt32)GMath::Abs(p1[G_Y] - p0[G_Y]);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	UpdateGrabBuffer(width, height, Shot);

	G_ASSERT(Shot.TexName > 0);
	G_ASSERT(Shot.TexWidth > 0 && Shot.TexHeight > 0);
	G_ASSERT(Shot.TexWidth >= width && Shot.TexHeight >= height);

	SELECT_AND_DISABLE_TUNIT(1)
	SELECT_AND_DISABLE_TUNIT(0)
	glEnable(Shot.Target);
	glBindTexture(Shot.Target, Shot.TexName);
	glCopyTexSubImage2D(Shot.Target, 0, 0, 0, (GLint)intBox.Min()[G_X], (GLint)intBox.Min()[G_Y], (GLsizei)width, (GLsizei)height);

	Shot.Width = width;
	Shot.Height = height;
	Shot.IsEmpty = G_FALSE;

	Shot.gNotExpandedLogicBox = tmpBox;

	GPoint2 q0 = PhysicalToLogical(p0);
	GPoint2 q1 = PhysicalToLogical(p1);
	Shot.gExpandedLogicBox.SetMinMax(q0, q1);

	SELECT_AND_DISABLE_TUNIT(0)
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void GOpenGLBoard::DrawGrabbedRect(const GLGrabbedRect& GrabbedRect, const GBool TexUnit0, const GBool SubPixel0,
								   const GBool TexUnit1, const GBool SubPixel1) {

	GReal u, v;
	GReal subX0 = 0, subY0 = 0, subX1 = 0, subY1 = 0;

	#define ENLARGE_OFFSET (GReal)0.5

	if (GrabbedRect.Target == GL_TEXTURE_2D) {
		u = (GReal)((GDouble)GrabbedRect.Width / (GDouble)GrabbedRect.TexWidth);
		v = (GReal)((GDouble)GrabbedRect.Height / (GDouble)GrabbedRect.TexHeight);
		if (SubPixel0) {
			subX0 = ENLARGE_OFFSET / (GrabbedRect.TexWidth);
			subY0 = ENLARGE_OFFSET / (GrabbedRect.TexHeight);
		}
		if (SubPixel1) {
			subX1 = ENLARGE_OFFSET / (GrabbedRect.TexWidth);
			subY1 = ENLARGE_OFFSET / (GrabbedRect.TexHeight);
		}
	}
	else {
		u = (GReal)GrabbedRect.Width;
		v = (GReal)GrabbedRect.Height;
		if (SubPixel0) {
			subX0 = ENLARGE_OFFSET;
			subY0 = ENLARGE_OFFSET;
		}
		if (SubPixel1) {
			subX1 = ENLARGE_OFFSET;
			subY1 = ENLARGE_OFFSET;
		}
	}

	GPoint2 p0(GrabbedRect.gExpandedLogicBox.Min());
	GPoint2 p2(GrabbedRect.gExpandedLogicBox.Max());
	GPoint2 p1(p0[G_X], p2[G_Y]);
	GPoint2 p3(p2[G_X], p0[G_Y]);

	glBegin(GL_POLYGON);
		if (TexUnit0)
			SetTextureVertex(0, 0 + subX0, v - subY0);
		if (TexUnit1)
			SetTextureVertex(1, 0 + subX1, v - subY1);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(p1.Data());
		#else
			glVertex2fv(p1.Data());
		#endif

		if (TexUnit0)
			SetTextureVertex(0, u - subX0, v - subY0);
		if (TexUnit1)
			SetTextureVertex(1, u - subX1, v - subY1);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(p2.Data());
		#else
			glVertex2fv(p2.Data());
		#endif

		if (TexUnit0)
			SetTextureVertex(0, u - subX0, 0 + subY0);
		if (TexUnit1)
			SetTextureVertex(1, u - subX1, 0 + subY1);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(p3.Data());
		#else
			glVertex2fv(p3.Data());
		#endif

		if (TexUnit0)
			SetTextureVertex(0, 0 + subX0, 0 + subY0);
		if (TexUnit1)
			SetTextureVertex(1, 0 + subX1, 0 + subY1);
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(p0.Data());
		#else
			glVertex2fv(p0.Data());
		#endif
	glEnd();
}

};	// end namespace Amanith
