/****************************************************************************
** $file: amanith/src/rendering/gopenglgroups.cpp   0.2.0.0   edited Dec, 12 2005
**
** OpenGL based draw board group functions implementation.
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

/*!
	\file gopenglgroups.cpp
	\brief OpenGL based draw board group functions implementation file.
*/

namespace Amanith {

void GOpenGLBoard::PushGLWindowMode() {

	GUInt32 x, y, w, h;
	Viewport(x, y, w, h);

	static const GLdouble s = 0.375;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-s, (GLdouble)w - s, -s, (GLdouble)h - s, (GLdouble)-1, (GLdouble)0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void GOpenGLBoard::PopGLWindowMode() {

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void GOpenGLBoard::DoGroupBegin(const GAABox2& LogicBox) {

	// if group opacity is not supported by hardware or group opacity is 1, ignore group as opaque
	if (GroupOpacity() >= 1 || GroupOpacity() <= 0 || TargetMode() == G_CLIP_MODE || !gGroupOpacitySupport)
		return;

	if (LogicBox.Volume() <= G_EPSILON) {
		gGLGroupRect.IsEmpty = G_TRUE;
		return;
	}
	else
		gGLGroupRect.IsEmpty = G_FALSE;

	GPoint<GInt32, 2> p0 = LogicalToPhysicalInt(LogicBox.Min());
	GPoint<GInt32, 2> p1 = LogicalToPhysicalInt(LogicBox.Max());

	GGenericAABox<GInt32, 2> physicBox(p0, p1);

	p0 = physicBox.Min();
	p1 = physicBox.Max();

	GrabFrameBuffer(p0, p1[G_X] - p0[G_X], p1[G_Y] - p0[G_Y], gGLGroupRect);

	PushGLWindowMode();
	glDisable(GL_DEPTH_TEST);

	// now intersect bounding box with current mask(s)
	if (ClipEnabled()) {
		StencilPush();
		glBegin(GL_POLYGON);
			glVertex2i(p0[G_X], p0[G_Y]);
			glVertex2i(p0[G_X], p1[G_Y]);
			glVertex2i(p1[G_X], p1[G_Y]);
			glVertex2i(p1[G_X], p0[G_Y]);
		glEnd();
		StencilEnableTop();
	}

	PopGLWindowMode();
}

void GOpenGLBoard::DoGroupEnd() {

	// if group opacity is not supported by hardware or group opacity is 1, ignore group as opaque
	if (GroupOpacity() >= 1 || GroupOpacity()<= 0 || gGLGroupRect.IsEmpty || TargetMode() == G_CLIP_MODE || !gGroupOpacitySupport)
		return;

	PushGLWindowMode();

	glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	if (ClipEnabled()) {
		gTopStencilValue++;
		glStencilFunc(GL_EQUAL, gTopStencilValue, gStencilMask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	}
	else {
		glStencilFunc(GL_EQUAL, (GLint)(~0), gStencilDualMask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	}

	glEnable(GL_BLEND);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	#if DOUBLE_REAL_TYPE
		glColor4d(1.0, 1.0, 1.0, GroupOpacity());
	#else
		glColor4f(1.0f, 1.0f, 1.0f, GroupOpacity());
	#endif
	ReplaceFrameBuffer(gGLGroupRect);
	glDisable(GL_BLEND);

	if (ClipEnabled()) {
		// delete stencil bounding box mask and drawn pixels
		StencilPop();
		glBegin(GL_POLYGON);
			glVertex2i(gGLGroupRect.X, gGLGroupRect.Y);
			glVertex2i(gGLGroupRect.X, gGLGroupRect.Y + gGLGroupRect.Height);
			glVertex2i(gGLGroupRect.X + gGLGroupRect.Width, gGLGroupRect.Y + gGLGroupRect.Height);
			glVertex2i(gGLGroupRect.X + gGLGroupRect.Width, gGLGroupRect.Y);
		glEnd();
		StencilPop();
		glBegin(GL_POLYGON);
			glVertex2i(gGLGroupRect.X, gGLGroupRect.Y);
			glVertex2i(gGLGroupRect.X, gGLGroupRect.Y + gGLGroupRect.Height);
			glVertex2i(gGLGroupRect.X + gGLGroupRect.Width, gGLGroupRect.Y + gGLGroupRect.Height);
			glVertex2i(gGLGroupRect.X + gGLGroupRect.Width, gGLGroupRect.Y);
		glEnd();
	}
	else {
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilFunc(GL_EQUAL, (GLint)(~0), gStencilDualMask);
		glStencilOp(GL_KEEP, GL_ZERO, GL_ZERO);
		glBegin(GL_POLYGON);
			glVertex2i(gGLGroupRect.X, gGLGroupRect.Y);
			glVertex2i(gGLGroupRect.X, gGLGroupRect.Y + gGLGroupRect.Height);
			glVertex2i(gGLGroupRect.X + gGLGroupRect.Width, gGLGroupRect.Y + gGLGroupRect.Height);
			glVertex2i(gGLGroupRect.X + gGLGroupRect.Width, gGLGroupRect.Y);
		glEnd();
	}

	PopGLWindowMode();
}

void GOpenGLBoard::GroupFirstPass() {

	// we must write only on stencil buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	if (ClipEnabled()) {
		glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
		glStencilFunc(GL_EQUAL, gTopStencilValue, gStencilMask);
	}
	else {
		glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, (GLint)(~0), gStencilDualMask);
	}
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
}

void GOpenGLBoard::GrabFrameBuffer(const GPoint<GInt32, 2>& LowLeft, const GUInt32 Width, const GUInt32 Height,
								   GLGrabbedRect& Shot) {

	if (gExtManager->IsRectTextureSupported()) {
		Shot.Target = GL_TEXTURE_RECTANGLE_EXT;
		Shot.TexWidth = Width;
		Shot.TexHeight = Height;
	}
	else {
		Shot.Target = GL_TEXTURE_2D;
		Shot.TexWidth = GOpenglExt::PowerOfTwo(Width);
		Shot.TexHeight = GOpenglExt::PowerOfTwo(Height);
	}
	//we must ensure that texture must not be larger than the maximum (hw)permitted size
	GUInt32 maxTexSize = gExtManager->MaxTextureSize();
	if (Shot.TexWidth > maxTexSize)
		Shot.TexWidth = maxTexSize;
	if (Shot.TexHeight > maxTexSize)
		Shot.TexHeight = maxTexSize;

	glGenTextures(1, &Shot.TexName);
	glBindTexture(Shot.Target, Shot.TexName);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	glTexParameteri(Shot.Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(Shot.Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(Shot.Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(Shot.Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glCopyTexImage2D(Shot.Target, 0, GL_RGB, LowLeft[G_X], LowLeft[G_Y], Shot.TexWidth, Shot.TexHeight, 0);

	Shot.X = LowLeft[G_X];
	Shot.Y = LowLeft[G_Y];
	Shot.Width = Width;
	Shot.Height = Height;

}

void GOpenGLBoard::ReplaceFrameBuffer(const GLGrabbedRect& Shot) {

	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_RECTANGLE_EXT);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();


	glEnable(Shot.Target);
	glBindTexture(Shot.Target, Shot.TexName);

	if (Shot.Target == GL_TEXTURE_2D) {

		GDouble u = (GDouble)Shot.Width / (GDouble)Shot.TexWidth;
		GDouble v = (GDouble)Shot.Height / (GDouble)Shot.TexHeight;

		glBegin(GL_POLYGON);
			glTexCoord2d(0, v);
			glVertex2i(Shot.X, Shot.Y + Shot.Height);
			glTexCoord2d(u, v);
			glVertex2i(Shot.X + Shot.Width, Shot.Y + Shot.Height);
			glTexCoord2d(u, 0);
			glVertex2i(Shot.X + Shot.Width, Shot.Y);
			glTexCoord2d(0, 0);
			glVertex2i(Shot.X, Shot.Y);
		glEnd();
	}
	else {
		glBegin(GL_POLYGON);
			glTexCoord2i(0, (GLint)Shot.TexHeight);
			glVertex2i(Shot.X, Shot.Y + Shot.Height);
			glTexCoord2i((GLint)Shot.TexWidth, (GLint)Shot.TexHeight);
			glVertex2i(Shot.X + Shot.Width, Shot.Y + Shot.Height);
			glTexCoord2i((GLint)Shot.TexWidth, 0);
			glVertex2i(Shot.X + Shot.Width, Shot.Y);
			glTexCoord2i(0, 0);
			glVertex2i(Shot.X, Shot.Y);
		glEnd();
	}

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glDisable(Shot.Target);
	glDeleteTextures(1, &Shot.TexName);
}

};	// end namespace Amanith
