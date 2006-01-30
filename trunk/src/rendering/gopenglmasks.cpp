/****************************************************************************
** $file: amanith/src/rendering/gopenglmasks.cpp   0.3.0.0   edited Jan, 30 2006
**
** OpenGL based draw board clip masks functions implementation.
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
	\file gopenglmasks.cpp
	\brief OpenGL based draw board clip masks functions implementation file.
*/

namespace Amanith {

void GOpenGLBoard::StencilPush() {

	if (!gClipMasksSupport)
		return;

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_STENCIL_TEST);

	if (gTopStencilValue > gMaxTopStencilValue) {
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		G_DEBUG("Stencil overflow");
	}
	else {
		glStencilFunc(GL_EQUAL, gTopStencilValue, gStencilMask);
		glStencilMask(gStencilMask);
		if (!InsideGroup())
			gTopStencilValue++;
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	}
}

void GOpenGLBoard::StencilPop() {

	if (!gClipMasksSupport)
		return;

	if (gTopStencilValue == 0) {
		G_DEBUG("Stencil underflow");
		return;
	}
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_LEQUAL, gTopStencilValue, gStencilMask);
	glStencilMask(gStencilMask);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	gTopStencilValue--;
}

void GOpenGLBoard::StencilReplace() {

	if (!gClipMasksSupport)
		return;

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_STENCIL_TEST);

	if (gTopStencilValue <= gMaxTopStencilValue) {

		if (!InsideGroup() || gIsFirstGroupDrawing)
			gTopStencilValue++;

		glStencilFunc(GL_ALWAYS, gTopStencilValue, gStencilMask);
		glStencilMask(gStencilMask);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	}
}

void GOpenGLBoard::StencilEnableTop() {

	if (!gClipMasksSupport)
		return;

	if (gTopStencilValue == 0 || !ClipEnabled())
		glDisable(GL_STENCIL_TEST);
	else {
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_LEQUAL, gTopStencilValue, gStencilMask);
		// do not change stencil buffer
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	}
}

void GOpenGLBoard::DepthNoStencilWrite() {

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);

	GReal left, right, top, bottom;
	Projection(left, right, bottom, top);
	GMatrix44 m = GLProjectionMatrix(left, right, bottom, top, (GReal)1e-7);
	// now all points (that have z = 0 because glVertex2dv/fv) will have a z-window value equal to +epsilon
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	#ifdef DOUBLE_REAL_TYPE
		glLoadMatrixd((const GLdouble *)m.Data());
	#else
		glLoadMatrixf((const GLfloat *)m.Data());
	#endif
}


void GOpenGLBoard::StencilNoDepthWrite() {

	glEnable(GL_STENCIL_TEST);
	if (ClipEnabled()) {
		glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
		glStencilFunc(GL_EQUAL, gTopStencilValue, gStencilMask);
		glStencilMask(gStencilMask);
	}
	else {
		glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, (GLint)(0x7FFFFFFF), gStencilDualMask);
		glStencilMask(gStencilDualMask);
	}

	// we are inside a group, write geometry on stencil, without touch zbuffer (and disabling z-test)
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void GOpenGLBoard::StencilWhereDepthEqual() {

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glEnable(GL_STENCIL_TEST);
	if (ClipEnabled()) {
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		glStencilFunc(GL_EQUAL, gTopStencilValue, gStencilMask);
		glStencilMask(gStencilMask);
	}
	else {
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, (GLint)(0x7FFFFFFF), gStencilDualMask);
		glStencilMask(gStencilDualMask);
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

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, gTopStencilValue, gStencilMask);
		glStencilMask(gStencilMask);
		if (gTopStencilValue > 0) {
			gTopStencilValue--;
			glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
		}
		else
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// pop the mask, drawing a bounding box
		DrawGLBox(lastClipMaskBox);
	}
}

void GOpenGLBoard::UpdateClipMasksState() {

	if (ClipOperation() == G_REPLACE_CLIP) {
		gClipMasksBoxes.clear();
		// sign the flag, so popping a mask written using replace operation will be done as a clear of the entire
		// stencil buffer
		gFirstClipMaskReplace = G_TRUE;
	}
	else {
		if (gClipMasksBoxes.empty())
			gFirstClipMaskReplace = G_FALSE;
	}
}

};	// end namespace Amanith
