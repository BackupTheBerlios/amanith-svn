/****************************************************************************
** $file: amanith/src/rendering/gopenglmasks.cpp   0.2.0.0   edited Dec, 12 2005
**
** OpenGL based draw board clip masks functions implementation.
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
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	}
}

void GOpenGLBoard::StencilEnableTop() {

	if (!gClipMasksSupport)
		return;

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

	if (gTopStencilValue == 0 || !ClipEnabled())
		glDisable(GL_STENCIL_TEST);
	else {
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_LEQUAL, gTopStencilValue, gStencilMask);
		// do not change stencil buffer
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
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

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, gTopStencilValue, gStencilMask);
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
