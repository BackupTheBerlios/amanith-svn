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
#include "amanith/geometry/gxform.h"
#include "amanith/geometry/gxformconv.h"

/*!
	\file gopenglboard.cpp
	\brief OpenGL based draw board implementation file.
*/

namespace Amanith {

// *********************************************************************
//                             GOpenGLBoard
// *********************************************************************

void GOpenGLBoard::DumpBuffers(const GChar8 *fNameZ, const GChar8 *fNameS) {

	GUInt32 x, y, w, h;
	Viewport(x, y, w, h);

	GLubyte *buf;
	std::FILE *f = NULL;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);

	buf = new GLubyte[w * h * 2];

	/*glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, buf);

	f = std::fopen(fNameZ, "wb");
	std::fwrite(buf, 1, w*h, f);
	std::fflush(f);
	std::fclose(f);*/

	std::memset(buf, 0, w*h);
	glReadPixels(0, 0, w, h, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buf);

	// open the file
#if defined(G_OS_WIN) && _MSC_VER >= 1400
	errno_t openErr = fopen_s(&f, fNameS, "wb");
	if (f && !openErr) {
		std::fwrite(buf, 1, w*h, f);
		std::fflush(f);
		std::fclose(f);
	}
#else
	f = std::fopen(fNameS, "wb");
	if (f) {
		std::fwrite(buf, 1, w*h, f);
		std::fflush(f);
		std::fclose(f);
	}
#endif
	delete [] buf;
}

/*
Radial fragment program
Parameters:	0 = [(Focus - Center).x, (Focus - Center).y, 0, 0]
			1 = [(F-C).LengthSquared - Radius^2, 0, 0, 0]
			2 = [a00, a01, a10, a11]
			3 = [a02, 0, a12, 0]
			4 = [1, 1, 1, Alpha]
			5 = texture u coordinate scaling
*/
static const char *const RadialProgram =
    "!!ARBfp1.0"
	"PARAM c[6] = { program.local[0..5] };"
    "TEMP R0;"
	"MAD R0, fragment.position.xyxy, c[2], c[3];"
	"ADD R0.x, R0.x, R0.y;"
	"ADD R0.y, R0.z, R0.w;"
	"MOV R0.zw, R0.xyxy;"
    "MUL R0.xy, R0, R0;"
    "MUL R0.zw, R0, c[0].xyxy;"
    "ADD R0.x, R0, R0.y;"
    "ADD R0.z, R0, R0.w;"
    "MOV R0.w, R0.z;"
    "MUL R0.w, R0, R0.w;"
	"MAD R0.w, -R0.x, c[1].x, R0.w;"
    "RSQ R0.w, R0.w;"
    "RCP R0.w, R0.w;"
	"ADD R0.z, -R0.z, R0.w;"
	"RCP R0.z, R0.z;"
	"MUL R0.x, R0, R0.z;"
	"MUL R0.x, R0, c[5];"
	"TEX R0, R0, texture[0], 1D;"
	"MUL result.color, R0, c[4];"
    "END"
    "\0";

/*
Conical fragment program
Parameters:	0 = [cos, sin, -sin, cos]
			1 = [Center.x, Center.y, 0, 0]
			2 = [0.5, 0.5, 0.5, 0.5]
			3 = [1, 1, 1, Alpha]
*/
static const char *const ConicalProgram =
    "!!ARBfp1.0"
	"PARAM c[4] = { program.local[0..3] };"
    "TEMP R0;"
	"SUB R0, fragment.position.xyxy, c[1].xyxy;"
	"MUL R0, R0, c[0];"
	"ADD R0.xy, R0.x, R0.y;"
	"ADD R0.zw, R0.z, R0.w;"
	"MUL R0.x, R0.x, R0.x;"
	"MAD R0.z, R0.z, R0.z, R0.x;"
	"RSQ R0.z, R0.z;"
	"MUL R0, R0, R0.z;"
	"MAD R0, R0, c[2], c[2];"
	"TEX R0, R0.ywyw, texture[0], 2D;"
	"MUL result.color, R0, c[3];"
    "END"
    "\0";

// atan2 lookup table
void GOpenGLBoard::GenerateAtan2LookupTable() {

	GInt32 size = gAtan2LookupTableSize;
	GFloat halfSizef = (GFloat)(size / 2);
	GInt32 sizeSqr = size * size;
	GInt32 ofs0 = 0;
	gAtan2LookupTable = new(std::nothrow) GFloat[sizeSqr];
	G_ASSERT(gAtan2LookupTable != NULL);

	for (GInt32 y = 0; y < size; y++) {
		GFloat ry = (GFloat)y - halfSizef;
		for (GInt32 x = 0; x < size; x++) {
			GFloat rx = (GFloat)x - halfSizef;
			GFloat atan2Val = GMath::Atan2(ry, rx);
			if (atan2Val < 0)
				atan2Val = (GFloat)G_2PI + atan2Val;
			GInt32 ofs = ofs0 + x;
			G_ASSERT(ofs >= 0 && ofs < sizeSqr);
			gAtan2LookupTable[ofs] = atan2Val / (GFloat)G_2PI;
		}
		ofs0 += size;
	}
}

void GOpenGLBoard::DisableShaders(const GBool Disable) {

	if (!gExtManager->IsArbProgramsSupported() || (!Disable == gFragmentProgramsSupport))
		return;

	gFragmentProgramsSupport = !Disable;
	// we have to mark all gradients as modified
	if (Disable == G_FALSE) {

		GUInt32 i, j = (GUInt32)gGradients.size();
		for (i = 0; i < j; i++) {
			GOpenGLGradientDesc *g = gGradients[i];
			g->SetColorInterpolationModified(G_TRUE);
		}
	}

}

GOpenGLBoard::GOpenGLBoard(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
						   const GUInt32 Width, const GUInt32 Height) : GDrawBoard() {

	gExtManager = new(std::nothrow) GOpenglExt();
	GUInt32 stencilBits = gExtManager->StencilBits();

	// verify if we can clip using stencil buffer
	if (stencilBits >= 8) {
		gClipByStencil = G_TRUE;
		// for example, if we have an 8bit stencil buffer, stencil mask will be 127
		gStencilMask = (1 << (stencilBits - 1)) - 1;
		gStencilDualMask = (~gStencilMask);
		// we must reserve 3 masks for internal use
		gMaxTopStencilValue = gStencilMask - 3;
	}
	else
		gClipByStencil = G_FALSE;

	gTopStencilValue = 0;
	// set "old" state of clipping operations
	gFirstClipMaskReplace = G_FALSE;

	// fragment programs support
	gFragmentProgramsSupport = gExtManager->IsArbProgramsSupported();
	gAtan2LookupTable = NULL;
	gAtan2LookupTableSize = 256;
	gRadGradGLProgram = 0;
	gConGradGLProgram = 0;
	if (gFragmentProgramsSupport) {
		// generate radial gradient fragment program
		glGenProgramsARB(1, &gRadGradGLProgram);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gRadGradGLProgram);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							std::strlen(RadialProgram), (const GLbyte *)RadialProgram);
		// check for errors
		if (GL_INVALID_OPERATION == glGetError()) {
			// find the error position
			GLint errPos;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
			const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			G_DEBUG((const GChar8 *)errString);
		}
		// generate atan2 lookup table
		GenerateAtan2LookupTable();
		// generate conical gradient fragment program
		glGenProgramsARB(1, &gConGradGLProgram);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gConGradGLProgram);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							std::strlen(ConicalProgram), (const GLbyte *)ConicalProgram);
		// check for errors
		if (GL_INVALID_OPERATION == glGetError()) {
			// find the error position
			GLint errPos;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
			const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			G_DEBUG((const GChar8 *)errString);
		}
	}

	SetViewport(LowLeftCornerX, LowLeftCornerY, Width, Height);
	SetProjection((GReal)LowLeftCornerX, (GReal)(LowLeftCornerX + Width), (GReal)LowLeftCornerY, (GReal)(LowLeftCornerY + Height)); 
	SetRenderingQuality(G_NORMAL_RENDERING_QUALITY);

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


	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
}

GOpenGLBoard::~GOpenGLBoard() {

	DeleteGradients();
	DeletePatterns();
	if (gAtan2LookupTable)
		delete [] gAtan2LookupTable;
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

	if (gFragmentProgramsSupport) {
		glDeleteProgramsARB(1, &gRadGradGLProgram);
		glDeleteProgramsARB(1, &gConGradGLProgram);
	}
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

void GOpenGLBoard::DoSetGroupOpacity(const GReal Opacity) {

	// just to avoid warning
	if (Opacity) {
	}
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

GBool GOpenGLBoard::SetGLClipEnabled(const GTargetMode Mode, const GClipOperation Operation) {

	if (gClipByStencil) {

		// write to the stencil using current clip operation
		if (Mode == G_CLIP_MODE) {
			switch (Operation) {
				case G_REPLACE_CLIP:
					StencilReplace();
					break;

				case G_INTERSECTION_CLIP:
					StencilPush();
					break;
			}
			return G_FALSE;
		}
		else {
			if (!InsideGroup()) {
				StencilEnableTop();
				return G_FALSE;
			}
			else {
				if (GroupOpacity() < 1 && GroupOpacity() > 0 && !gGLGroupRect.IsEmpty) {
					// we have to do a double-pass algorithm (first write into stencil, then into color buffer)
					return G_TRUE;
				}
				else {
					StencilEnableTop();
					return G_FALSE;
				}
			}
		}
	}
	return G_FALSE;
}

void GOpenGLBoard::DoSetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
								 const GUInt32 Width, const GUInt32 Height) {

	 glViewport(LowLeftCornerX, LowLeftCornerY, Width, Height);
	 UpdateDeviation(RenderingQuality());
}

void GOpenGLBoard::DoSetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top) {

/*	GMatrix44 m;

	m[0][0] = 2.0 / (Right - Left);
	m[0][3] = -(Right + Left) / (Right - Left);

	m[1][1] = 2.0 / (Top - Bottom);
	m[1][3] = -(Top + Bottom) / (Top - Bottom);

	GReal zNear = -1;
	GReal zFar = 0;

	m[2][2] = -2 / (zFar - zNear);
	m[2][3] = (-(zFar + zNear) / (zFar - zNear));*/

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho((GLdouble)Left, (GLdouble)Right, (GLdouble)Bottom, (GLdouble)Top, (GLdouble)-1, (GLdouble)0);

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

};	// end namespace Amanith
