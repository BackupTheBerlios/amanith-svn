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

/*!
	\file gopenglcompositing.cpp
	\brief OpenGL based draw board compositing functions implementation file.
*/

namespace Amanith {


// *********************************************************************
//                             GOpenGLBoard
// *********************************************************************

/*
	Simple color fragment program
*/
static const GChar8 *const ColorProgram =
    "!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
	"TEMP R0;"
	"MUL R0, fragment.color, fragment.color.w;"
	"MOV R0.w, fragment.color;";

/*
	Linear gradient fragment program
	Parameters:	0 = [n.x, n.y, 0, q]
				1 = [1, 1, 1, Alpha]
				2 = [a00, a01, 0, a02]
				3 = [a10, a11, 0, a12]
*/
static const GChar8 *const LinearProgram =
	"!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
	"PARAM c[4] = { program.local[0..3] };"
	"TEMP R0;"
	"DPH R0.x, fragment.position, c[2];"
	"DPH R0.y, fragment.position, c[3];"
	"MOV R0.zw, fragment.position;"
	"DPH R0, R0, c[0];"
	"TEX R0, R0.x, texture[0], 1D;"
	"MUL R0, R0, c[1];"
	"MUL R0.xyz, R0, R0.w;";


/*
	Radial fragment program
	Parameters:	0 = [(Focus - Center).x, (Focus - Center).y, 0, 0]
				1 = [(F-C).LengthSquared - Radius^2, 0, 0, 0]
				2 = [a00, a01, a10, a11]
				3 = [a02, 0, a12, 0]
				4 = [1, 1, 1, Alpha]
				5 = texture u coordinate scaling
*/
static const GChar8 *const RadialProgram =
    "!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
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
	"MUL R0, R0, c[4];"
	"MUL R0.xyz, R0, R0.w;";

/*
	Conical fragment program
	Parameters:	0 = [cos, sin, -sin, cos]
				1 = [0.5, 0.5, 0.5, 0.5]
				2 = [1, 1, 1, Alpha]
				3 = [a00, a01, a10, a11]
				4 = [a02, 0, a12, 0]
				5 = [center.x, center.y, 0, 0]
*/
static const GChar8 *const ConicalProgram =
    "!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
	"PARAM c[6] = { program.local[0..5] };"
    "TEMP R0;"
	"MAD R0, fragment.position.xyxy, c[3], c[4];"
	"ADD R0.x, R0.x, R0.y;"
	"ADD R0.y, R0.z, R0.w;"
	"SUB R0, R0.xyxy, c[5].xyxy;"
	"MUL R0, R0, c[0];"
	"ADD R0.xy, R0.x, R0.y;"
	"ADD R0.zw, R0.z, R0.w;"
	"MUL R0.x, R0.x, R0.x;"
	"MAD R0.z, R0.z, R0.z, R0.x;"
	"RSQ R0.z, R0.z;"
	"MUL R0, R0, R0.z;"
	"MAD R0, R0, c[1], c[1];"
	"TEX R0, R0.ywyw, texture[0], 2D;"
	"MUL R0, R0, c[2];"
	"MUL R0.xyz, R0, R0.w;";

/*
	Pattern fragment program
	Parameters:	0 = [a00, a01, 0, a02]
				1 = [a10, a11, 0, a12]
				2 = [1, 1, 1, Alpha]
*/
static const GChar8 *const PatternProgram =
	"!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
	"PARAM c[3] = { program.local[0..2] };"
	"TEMP R0;"
	"DPH R0.z, fragment.position, c[0];"
	"DPH R0.w, fragment.position, c[1];"
	"TEX R0, R0.zwzw, texture[0], 2D;"
	"MUL R0, R0, c[2];"
	"MUL R0.xyz, R0, R0.w;";

/*
	Simple single power-of-two texture fragment program
	Parameters:	0 = [1, 1, 1, Alpha]
*/
static const GChar8 *const PutTextureProgram_POW2 =
	"!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
	"PARAM c[1] = { program.local[0] };"
	"TEMP R0;"
	"TEX R0, fragment.texcoord[0].xyxy, texture[0], 2D;"
	"MUL R0.w, R0, c[0];"
	"MUL R0.xyz, R0, R0.w;";

/*
	Simple single rectangular texture fragment program
	Parameters:	0 = [1, 1, 1, Alpha]
*/
static const GChar8 *const PutTextureProgram_RECT =
	"!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
	"PARAM c[1] = { program.local[0] };"
	"TEMP R0;"
	"TEX R0, fragment.texcoord[0].xyxy, texture[0], RECT;"
	"MUL R0.w, R0, c[0];"
	"MUL R0.xyz, R0, R0.w;";

// inside R0 there is (Sca, Sa) couple
#define IDENTITY_OP_RGBA "MOV result.color, R0;"

// read from first power-of-two texture unit in R1
#define READ_TEX0_POW2 \
	"TEMP R1;" \
	"TEX R1, fragment.texcoord[0].xyxy, texture[0], 2D;"

// read from first rectangular texture unit in R1
#define READ_TEX0_RECT \
	"TEMP R1;" \
	"TEX R1, fragment.texcoord[0].xyxy, texture[0], RECT;"

// read from second power-of-two texture unit in R1
#define READ_TEX1_POW2 \
	"TEMP R1;" \
	"TEX R1, fragment.texcoord[1].xyxy, texture[1], 2D;"

// read from second rectangular texture unit in R1
#define READ_TEX1_RECT \
	"TEMP R1;" \
	"TEX R1, fragment.texcoord[1].xyxy, texture[1], RECT;"

// G_DST_ATOP_OP
#define DST_ATOP_OP_RGBA \
	"MUL R1.xyz, R1, R0.w;" \
	"MAD R0.xyz, R0, -R1.w, R0;" \
	"ADD R0.xyz, R0, R1;" \
	"MOV result.color, R0;"

// G_MULTIPLY_OP
#define MULTIPLY_OP_RGBA \
	"TEMP R2;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"MUL R2, R0, R1;" \
	"MAD R0.xyz, R0, -R1.w, R0;" \
	"MAD R1.xyz, R1, -R0.w, R1;" \
	"ADD R0.xyz, R0, R1;" \
	"ADD R0.xyz, R0, R2;" \
	"LRP R0.w, R1.w, ones, R0.w;" \
	"MOV result.color, R0;"

// G_SCREEN_OP
#define SCREEN_OP_RGBA \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"LRP R0.xyz, R1, ones, R0;" \
	"LRP R0.w, R1.w, ones, R0;" \
	"MOV result.color, R0;"

// G_OVERLAY_OP
#define OVERLAY_OP_RGBA \
	"TEMP R2;" \
	"TEMP R3;" \
	"TEMP R4;" \
	"TEMP R5;" \
	"TEMP R6;" \
	"TEMP R7;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"ADD R2, R1, R1;" \
	"SLT R3.xyz, R2, R1.w;" \
	"SGE R4.xyz, R2, R1.w;" \
	"MAD R5.xyz, R0, -R1.w, R0;" \
	"MAD R6.xyz, R1, -R0.w, R1;" \
	"ADD R5, R5, R6;" \
	"MUL R6.xyz, R0, R1;" \
	"ADD R6, R6, R6;" \
	"MUL R7.xyz, R1, R0.w;" \
	"MAD R7.xyz, R0, R1.w, R7;" \
	"ADD R7, R7, R7;" \
	"MAD R7.xyz, R0.w, -R1.w, R7;" \
	"SUB R7.xyz, R7, R6;" \
	"ADD R7.xyz, R7, R5;" \
	"ADD R6.xyz, R5, R6;" \
	"MUL R6.xyz, R6, R3;" \
	"MAD R6.xyz, R7, R4, R6;" \
	"LRP R6.w, R1.w, ones, R0.w;" \
	"MOV result.color, R6;"

// G_DARKEN_OP
#define DARKEN_OP_RGBA \
	"TEMP R2;" \
	"TEMP R3;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"MUL R2.xyz, R0, R1.w;" \
	"MUL R3.xyz, R1, R0.w;" \
	"MIN R2, R2, R3;" \
	"MAD R0.xyz, R0, -R1.w, R0;" \
	"MAD R1.xyz, R1, -R0.w, R1;" \
	"ADD R0.xyz, R0, R1;" \
	"ADD R0.xyz, R0, R2;" \
	"LRP R0.w, R1.w, ones, R0.w;" \
	"MOV result.color, R0;"

// G_LIGHTEN_OP
#define LIGHTEN_OP_RGBA \
	"TEMP R2;" \
	"TEMP R3;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"MUL R2.xyz, R0, R1.w;" \
	"MUL R3.xyz, R1, R0.w;" \
	"MAX R2, R2, R3;" \
	"MAD R0.xyz, R0, -R1.w, R0;" \
	"MAD R1.xyz, R1, -R0.w, R1;" \
	"ADD R0.xyz, R0, R1;" \
	"ADD R0.xyz, R0, R2;" \
	"LRP R0.w, R1.w, ones, R0.w;" \
	"MOV result.color, R0;"

// G_COLOR_DODGE_OP
#define COLOR_DODGE_OP_RGBA \
	"TEMP R2;" \
	"TEMP R3;" \
	"TEMP R4;" \
	"TEMP R5;" \
	"TEMP R6;" \
	"TEMP R7;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"MUL R2.xyz, R0, R1.w;" \
	"MAD R2.xyz, R1, R0.w, R2;" \
	"MUL R2.w, R0.w, R1.w;" \
	"SGE R3.xyz, R2, R2.w;" \
	"SLT R4.xyz, R2, R2.w;" \
	"ADD R5.xyz, R0, R1;" \
	"SUB R6.xyz, R5, R2;" \
	"ADD R5.xyz, R6, R2.w;" \
	"SUB R7.xyz, R0.w, R0;" \
	"RCP R7.x, R7.x;" \
	"RCP R7.y, R7.y;" \
	"RCP R7.z, R7.z;" \
	"MUL R7.xyz, R7, R1;" \
	"MUL R7.xyz, R7, R0.w;" \
	"MUL R7.xyz, R7, R0.w;" \
	"ADD R6.xyz, R7, R6;" \
	"MUL R5.xyz, R5, R3;" \
	"MAD R5.xyz, R6, R4, R5;" \
	"LRP R5.w, R1.w, ones, R0.w;" \
	"MOV result.color, R5;"

// G_COLOR_BURN_OP
#define COLOR_BURN_OP_RGBA \
	"TEMP R2;" \
	"TEMP R3;" \
	"TEMP R4;" \
	"TEMP R5;" \
	"TEMP R6;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"MUL R2.xyz, R0, R1.w;" \
	"MAD R2.xyz, R1, R0.w, R2;" \
	"MUL R2.w, R0.w, R1.w;" \
	"SGE R3.xyz, R2.w, R2;" \
	"SLT R4.xyz, R2.w, R2;" \
	"ADD R5.xyz, R0, R1;" \
	"SUB R5.xyz, R5, R2;" \
	"MOV R6.xyz, R5;" \
	"SUB R2.xyz, R2, R2.w;" \
	"MUL R2.xyz, R2, R0.w;" \
	"RCP R0.x, R0.x;" \
	"RCP R0.y, R0.y;" \
	"RCP R0.z, R0.z;" \
	"MAD R2.xyz, R2, R0, R6;" \
	"MUL R3.xyz, R3, R5;" \
	"MAD R3.xyz, R4, R2, R3;" \
	"LRP R3.w, R1.w, ones, R0.w;" \
	"MOV result.color, R3;"

// G_HARD_LIGHT_OP
#define HARD_LIGHT_OP_RGBA \
	"TEMP R2;" \
	"TEMP R3;" \
	"TEMP R4;" \
	"TEMP R5;" \
	"TEMP R6;" \
	"TEMP R7;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"ADD R2, R0, R0;" \
	"SLT R3.xyz, R2, R0.w;" \
	"SGE R4.xyz, R2, R0.w;" \
	"MAD R5.xyz, R0, -R1.w, R0;" \
	"MAD R6.xyz, R1, -R0.w, R1;" \
	"ADD R5, R5, R6;" \
	"MUL R6.xyz, R0, R1;" \
	"ADD R6, R6, R6;" \
	"MUL R7.xyz, R1, R0.w;" \
	"MAD R7.xyz, R0, R1.w, R7;" \
	"ADD R7, R7, R7;" \
	"MAD R7.xyz, R0.w, -R1.w, R7;" \
	"SUB R7.xyz, R7, R6;" \
	"ADD R7.xyz, R7, R5;" \
	"ADD R6.xyz, R5, R6;" \
	"MUL R6.xyz, R6, R3;" \
	"MAD R6.xyz, R7, R4, R6;" \
	"LRP R6.w, R1.w, ones, R0.w;" \
	"MOV result.color, R6;"

// G_SOFT_LIGHT_OP
#define SOFT_LIGHT_OP_RGBA \
	"TEMP R2;" \
	"TEMP R3;" \
	"TEMP R4;" \
	"TEMP R5;" \
	"TEMP R6;" \
	"TEMP R7;" \
	"TEMP R8;" \
	"TEMP R9;" \
	"TEMP R10;" \
	"PARAM halfs = { 0.5, 0.5, 0.5, 0.5 };" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"PARAM threes = { 3.0, 3.0, 3.0, 3.0 };" \
	"PARAM eights = { 8.0, 8.0, 8.0, 8.0 };" \
	"ADD R2.xyz, R0, R0;" \
	"MUL R3.xyz, R1, eights;" \
	"SGE R5, R1.w, R3;" \
	"SLT R6, R1.w, R3;" \
	"SLT R4, R2, R0.w;" \
	"SGE R7, R2, R0.w;" \
	"MUL R5, R5, R7;" \
	"MUL R6, R6, R7;" \
	"MAD R7.xyz, R0, -R1.w, R0;" \
	"MAD R8.xyz, R1, -R0.w, R1;" \
	"ADD R7, R7, R8;" \
	"SUB R2.xyz, R2, R0.w;" \
	"RCP R8, R1.w;" \
	"MAD R9.xyz, R8, -R1, ones;" \
	"MUL R9.xyz, R2, R9;" \
	"MAD R3.xyz, R3, -R8, threes;" \
	"SUB R10.xyz, R0.w, R9;" \
	"MAD R10.xyz, R10, R1, R7;" \
	"MAD R9.xyz, R9, -R3, R0.w;" \
	"MAD R9.xyz, R9, R1, R7;" \
	"MUL R3.xyz, R1, R8;" \
	"POW R3.x, R3.x, halfs.x;" \
	"POW R3.y, R3.y, halfs.y;" \
	"POW R3.z, R3.z, halfs.z;" \
	"MAD R3.xyz, R3, R1.w, -R1;" \
	"MUL R3.xyz, R3, R2;" \
	"MAD R8.xyz, R1, R0.w, R3;" \
	"ADD R8.xyz, R8, R7;" \
	"MUL R8.xyz, R8, R6;" \
	"MAD R8.xyz, R9, R5, R8;" \
	"MAD R8.xyz, R10, R4, R8;" \
	"LRP R8.w, R1.w, ones, R0.w;" \
	"MOV result.color, R8;"

// G_DIFFERENCE_OP
#define DIFFERENCE_OP_RGBA \
	"TEMP R2;" \
	"TEMP R3;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"MUL R2.xyz, R0, R1.w;" \
	"MUL R3.xyz, R1, R0.w;" \
	"MIN R2, R2, R3;" \
	"ADD R2, R2, R2;" \
	"ADD R0.xyz, R0, R1;" \
	"SUB R0.xyz, R0, R2;" \
	"LRP R0.w, R1.w, ones, R0.w;" \
	"MOV result.color, R0;"

// G_EXCLUSION_OP
#define EXCLUSION_OP_RGBA \
	"TEMP R2;" \
	"PARAM ones = { 1.0, 1.0, 1.0, 1.0 };" \
	"MUL R2, R0, R1;" \
	"ADD R2, R2, R2;" \
	"ADD R0.xyz, R0, R1;" \
	"ADD R0.xyz, R0, -R2;" \
	"LRP R0.w, R0.w, ones, R1.w;" \
	"MOV result.color, R0;"

#define END_PROGRAM_RGBA "END\0"


GLuint GOpenGLBoard::GLGenerateProgram(const GChar8 *ProgramString) {

	if (!ProgramString)
		return 0;

	// generate and bind the program
	GLuint progID;
	glGenProgramsARB(1, &progID);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progID);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)std::strlen(ProgramString), (const GLbyte *)ProgramString);

	// if the program is over the hardware's limits, print out some information
	GLint isUnderNativeLimits;
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isUnderNativeLimits);
	if (isUnderNativeLimits != 1) {

		GChar8 errStr[1024];
		G_DEBUG("GenerateProgram: Fragment program is beyond hardware limits:\n");

		GLint aluInstructions, maxAluInstructions;
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_ALU_INSTRUCTIONS_ARB, &aluInstructions);
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB, &maxAluInstructions);
		if (aluInstructions > maxAluInstructions) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				sprintf_s(errStr, 1024, " - Compiles to too many ALU instructions (%d, limit is %d)\n", aluInstructions, maxAluInstructions);
			#else
				sprintf(errStr, " - Compiles to too many ALU instructions (%d, limit is %d)\n", aluInstructions, maxAluInstructions);
			#endif
			G_DEBUG(errStr);
		}

		GLint textureInstructions, maxTextureInstructions;
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_TEX_INSTRUCTIONS_ARB, &textureInstructions);
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB, &maxTextureInstructions);
		if (textureInstructions > maxTextureInstructions) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				sprintf_s(errStr, 1024, " - Compiles to too many texture instructions (%d, limit is %d)\n", textureInstructions, maxTextureInstructions);
			#else
				sprintf(errStr, " - Compiles to too many texture instructions (%d, limit is %d)\n", textureInstructions, maxTextureInstructions);
			#endif
			G_DEBUG(errStr);
		}

		GLint textureIndirections, maxTextureIndirections;
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_TEX_INDIRECTIONS_ARB, &textureIndirections);
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB, &maxTextureIndirections);
		if (textureIndirections > maxTextureIndirections) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				sprintf_s(errStr, 1024, " - Compiles to too many texture indirections (%d, limit is %d)\n", textureIndirections, maxTextureIndirections);
			#else
				sprintf(errStr, " - Compiles to too many texture indirections (%d, limit is %d)\n", textureIndirections, maxTextureIndirections);
			#endif
			G_DEBUG(errStr);
		}

		GLint nativeTextureIndirections, maxNativeTextureIndirections;
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, &nativeTextureIndirections);
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, &maxNativeTextureIndirections);
		if (nativeTextureIndirections > maxNativeTextureIndirections) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				sprintf_s(errStr, 1024, " - Compiles to too many native texture indirections (%d, limit is %d)\n", nativeTextureIndirections, maxNativeTextureIndirections);
			#else
				sprintf(errStr, " - Compiles to too many native texture indirections (%d, limit is %d)\n", nativeTextureIndirections, maxNativeTextureIndirections);
			#endif
			G_DEBUG(errStr);
		}

		GLint nativeAluInstructions, maxNativeAluInstructions;
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, &nativeAluInstructions);
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, &maxNativeAluInstructions);
		if (nativeAluInstructions > maxNativeAluInstructions) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				sprintf_s(errStr, 1024, "Compiles to too many native ALU instructions (%d, limit is %d)\n", nativeAluInstructions, maxNativeAluInstructions);
			#else
				sprintf(errStr, "Compiles to too many native ALU instructions (%d, limit is %d)\n", nativeAluInstructions, maxNativeAluInstructions);
			#endif
			G_DEBUG(errStr);
		}
		glDeleteProgramsARB(1, &progID);
		progID = 0;
	}
	else
	// see if a syntax error was found
	if (GL_INVALID_OPERATION == glGetError()) {
		GLint errPos;
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
		const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		if (errString) {
			G_DEBUG((const GChar8 *)errString);
		}
		glDeleteProgramsARB(1, &progID);
		progID = 0;
	}
	return progID;
}

GLuint GOpenGLBoard::GenerateGroupProgram(const GUInt32 TextureTarget, const GChar8 *TexString, const GChar8 *OpString) {

	#define TEXTURE_POW2 0
	#define TEXTURE_RECT 1

	G_ASSERT(TextureTarget == TEXTURE_POW2 || TextureTarget == TEXTURE_RECT);

	GChar8 *prg = new (std::nothrow) char[4096];

	if (!prg)
		return 0;

	// clear program string
	std::memset((void *)prg, 0, 4096);

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (TextureTarget == TEXTURE_POW2)
		strcpy_s(prg, 4096, PutTextureProgram_POW2);
	else
		strcpy_s(prg, 4096, PutTextureProgram_RECT);

	if (TexString)
		strcat_s(prg, 4096, TexString);
	if (OpString)
		strcat_s(prg, 4096, OpString);
	strcat_s(prg, 4096, END_PROGRAM_RGBA);
#else
	if (TextureTarget == TEXTURE_POW2)
		std::strcpy(prg, PutTextureProgram_POW2);
	else
		std::strcpy(prg, PutTextureProgram_RECT);

	if (TexString)
		std::strcat(prg, TexString);
	if (OpString)
		std::strcat(prg, OpString);
	std::strcat(prg, END_PROGRAM_RGBA);
#endif

	// generate and bind the program
	GLuint progID = GLGenerateProgram(prg);

	delete [] prg;
	return progID;
	#undef TEXTURE_POW2
	#undef TEXTURE_RECT
}

GLuint GOpenGLBoard::GenerateProgram(const GUInt32 PaintType, const GChar8 *TexString, const GChar8 *OpString) {

	#define COLOR 0
	#define LINGRAD 1
	#define RADGRAD 2
	#define CONGRAD 3
	#define PATTERN 4

	GChar8 *prg = new (std::nothrow) char[4096];

	if (!prg)
		return 0;

	// clear program string
	std::memset((void *)prg, 0, 4096);

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	switch (PaintType) {
		case COLOR:
			strcpy_s(prg, 4096, ColorProgram);
			break;
		case LINGRAD:
			strcpy_s(prg, 4096, LinearProgram);
			break;
		case RADGRAD:
			strcpy_s(prg, 4096, RadialProgram);
			break;
		case CONGRAD:
			strcpy_s(prg, 4096, ConicalProgram);
			break;
		case PATTERN:
			strcpy_s(prg, 4096, PatternProgram);
			break;
		default:
			strcpy_s(prg, 4096, ColorProgram);
			break;
	}
	if (TexString)
		strcat_s(prg, 4096, TexString);
	if (OpString)
		strcat_s(prg, 4096, OpString);
	strcat_s(prg, 4096, END_PROGRAM_RGBA);
#else
	switch (PaintType) {
		case COLOR:
			std::strcpy(prg, ColorProgram);
			break;
		case LINGRAD:
			std::strcpy(prg, LinearProgram);
			break;
		case RADGRAD:
			std::strcpy(prg, RadialProgram);
			break;
		case CONGRAD:
			std::strcpy(prg, ConicalProgram);
			break;
		case PATTERN:
			std::strcpy(prg, PatternProgram);
			break;
		default:
			std::strcpy(prg, ColorProgram);
			break;
	}
	if (TexString)
		std::strcat(prg, TexString);
	if (OpString)
		std::strcat(prg, OpString);
	std::strcat(prg, END_PROGRAM_RGBA);
#endif


	// generate and bind the program
	GLuint progID = GLGenerateProgram(prg);

	delete [] prg;
	return progID;
	#undef COLOR
	#undef LINGRAD
	#undef RADGRAD
	#undef CONGRAD
	#undef PATTERN
}

void GOpenGLBoard::GenerateShadersTable() {

	GUInt32 i, j;
	GUInt32 uniqueID;

	#define TEXTURE_POW2 0
	#define TEXTURE_RECT 1

	#define COLOR_PAINT		0
	#define LINGRAD_PAINT	1
	#define RADGRAD_PAINT	2
	#define CONGRAD_PAINT	3
	#define PATTERN_PAINT	4
	#define PAINT_TYPES_COUNT (PATTERN_PAINT - COLOR_PAINT + 1)

	#define DST_ATOP_OFFSET		5
	#define MULTIPLY_OFFSET		15
	#define SCREEN_OFFSET		25
	#define OVERLAY_OFFSET		35
	#define DARKEN_OFFSET		45
	#define LIGHTEN_OFFSET		55
	#define COLOR_DODGE_OFFSET	65
	#define COLOR_BURN_OFFSET	75
	#define HARD_LIGHT_OFFSET	85
	#define SOFT_LIGHT_OFFSET	95
	#define DIFFERENCE_OFFSET	105
	#define EXCLUSION_OFFSET	115

	// clear tables
	for (i = 0; i < 5 * (1 + 12 * 2); ++i)
		gUniqueProgramsID[i] = 0;

	for (i = 0; i < (1 + 12); ++i) {
		gUniqueGroupProgramsID[i][TEXTURE_POW2] = 0;
		gUniqueGroupProgramsID[i][TEXTURE_RECT] = 0;
	}

	for (i = 0; i < 24; ++i) {
		gCompGroupProgramsRGBA[i][TEXTURE_POW2] = 0;
		gCompGroupProgramsRGBA[i][TEXTURE_RECT] = 0;
		gCompGroupProgramsRGB[i][TEXTURE_POW2] = 0;
		gCompGroupProgramsRGB[i][TEXTURE_RECT] = 0;
		for (j = 0; j < 5; ++j) {
			gCompProgramsRGB[i][j][TEXTURE_POW2] = 0;
			gCompProgramsRGB[i][j][TEXTURE_RECT] = 0;
			gCompProgramsRGBA[i][j][TEXTURE_POW2] = 0;
			gCompProgramsRGBA[i][j][TEXTURE_RECT] = 0;
		}
	}


	uniqueID = 0;
	// identity op, it's valid for CLEAR, SRC, SRC_OVER, DST_OVER, SRC_IN, DST_IN, SRC_OUT, SRC_ATOP, XOR
	// PLUS, SCREEN (just for RGB), EXCLUSION (just for RGB)
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, NULL, IDENTITY_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, NULL, IDENTITY_OP_RGBA);

	for (i = TEXTURE_POW2; i <= TEXTURE_RECT; i++) {
		gCompGroupProgramsRGBA[G_CLEAR_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_CLEAR_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_SRC_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_SRC_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_SRC_OVER_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_SRC_OVER_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_DST_OVER_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_DST_OVER_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_SRC_IN_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_SRC_IN_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_DST_IN_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_DST_IN_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_SRC_OUT_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_SRC_OUT_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_SRC_ATOP_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_SRC_ATOP_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_XOR_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_XOR_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGBA[G_PLUS_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_PLUS_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_SCREEN_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
		gCompGroupProgramsRGB[G_EXCLUSION_OP][i] = gUniqueGroupProgramsID[uniqueID][i];
	}
	uniqueID++;

	// DST_ATOP
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, DST_ATOP_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, DST_ATOP_OP_RGBA);
	gCompGroupProgramsRGBA[G_DST_ATOP_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_DST_ATOP_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_DST_ATOP_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_DST_ATOP_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// MULTIPLY
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, MULTIPLY_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, MULTIPLY_OP_RGBA);
	gCompGroupProgramsRGBA[G_MULTIPLY_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_MULTIPLY_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_MULTIPLY_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_MULTIPLY_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// SCREEN
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, SCREEN_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, SCREEN_OP_RGBA);
	gCompGroupProgramsRGBA[G_SCREEN_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_SCREEN_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	uniqueID++;
	// OVERLAY
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, OVERLAY_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, OVERLAY_OP_RGBA);
	gCompGroupProgramsRGBA[G_OVERLAY_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_OVERLAY_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_OVERLAY_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_OVERLAY_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// DARKEN
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, DARKEN_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, DARKEN_OP_RGBA);
	gCompGroupProgramsRGBA[G_DARKEN_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_DARKEN_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_DARKEN_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_DARKEN_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// LIGHTEN
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, LIGHTEN_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, LIGHTEN_OP_RGBA);
	gCompGroupProgramsRGBA[G_LIGHTEN_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_LIGHTEN_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_LIGHTEN_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_LIGHTEN_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// COLOR_DODGE
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, COLOR_DODGE_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, COLOR_DODGE_OP_RGBA);
	gCompGroupProgramsRGBA[G_COLOR_DODGE_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_COLOR_DODGE_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_COLOR_DODGE_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_COLOR_DODGE_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// COLOR_BURN
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, COLOR_BURN_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, COLOR_BURN_OP_RGBA);
	gCompGroupProgramsRGBA[G_COLOR_BURN_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_COLOR_BURN_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_COLOR_BURN_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_COLOR_BURN_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// HARD_LIGHT
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, HARD_LIGHT_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, HARD_LIGHT_OP_RGBA);
	gCompGroupProgramsRGBA[G_HARD_LIGHT_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_HARD_LIGHT_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_HARD_LIGHT_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_HARD_LIGHT_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// SOFT_LIGHT
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, SOFT_LIGHT_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, SOFT_LIGHT_OP_RGBA);
	gCompGroupProgramsRGBA[G_SOFT_LIGHT_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_SOFT_LIGHT_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_SOFT_LIGHT_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_SOFT_LIGHT_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// DIFFERENCE
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, DIFFERENCE_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, DIFFERENCE_OP_RGBA);
	gCompGroupProgramsRGBA[G_DIFFERENCE_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_DIFFERENCE_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	gCompGroupProgramsRGB[G_DIFFERENCE_OP][TEXTURE_POW2] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_POW2];
	gCompGroupProgramsRGB[G_DIFFERENCE_OP][TEXTURE_RECT] = gCompGroupProgramsRGB[G_SRC_OVER_OP][TEXTURE_RECT];
	uniqueID++;
	// EXCLUSION
	gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2] = GenerateGroupProgram(TEXTURE_POW2, READ_TEX1_POW2, EXCLUSION_OP_RGBA);
	gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT] = GenerateGroupProgram(TEXTURE_RECT, READ_TEX1_RECT, EXCLUSION_OP_RGBA);
	gCompGroupProgramsRGBA[G_EXCLUSION_OP][TEXTURE_POW2] = gUniqueGroupProgramsID[uniqueID][TEXTURE_POW2];
	gCompGroupProgramsRGBA[G_EXCLUSION_OP][TEXTURE_RECT] = gUniqueGroupProgramsID[uniqueID][TEXTURE_RECT];
	uniqueID++;


	// identity op, it's valid for CLEAR, SRC, SRC_OVER, DST_OVER, SRC_IN, DST_IN, SRC_OUT, SRC_ATOP, XOR
	// PLUS, SCREEN (just for RGB), EXCLUSION (just for RGB)
	uniqueID = 0;
	for (i = COLOR_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, NULL, IDENTITY_OP_RGBA);

	// DST_ATOP
	uniqueID = DST_ATOP_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, DST_ATOP_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, DST_ATOP_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, DST_ATOP_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, DST_ATOP_OP_RGBA);

	// MULTIPLY
	uniqueID = MULTIPLY_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, MULTIPLY_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, MULTIPLY_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, MULTIPLY_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, MULTIPLY_OP_RGBA);

	// SCREEN
	uniqueID = SCREEN_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, SCREEN_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, SCREEN_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, SCREEN_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, SCREEN_OP_RGBA);

	// OVERLAY
	uniqueID = OVERLAY_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, OVERLAY_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, OVERLAY_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, OVERLAY_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, OVERLAY_OP_RGBA);

	// DARKEN
	uniqueID = DARKEN_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, DARKEN_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, DARKEN_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, DARKEN_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, DARKEN_OP_RGBA);

	// LIGHTEN
	uniqueID = LIGHTEN_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, LIGHTEN_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, LIGHTEN_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, LIGHTEN_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, LIGHTEN_OP_RGBA);

	// COLOR_DODGE
	uniqueID = COLOR_DODGE_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, COLOR_DODGE_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, COLOR_DODGE_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, COLOR_DODGE_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, COLOR_DODGE_OP_RGBA);

	// COLOR_BURN
	uniqueID = COLOR_BURN_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, COLOR_BURN_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, COLOR_BURN_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, COLOR_BURN_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, COLOR_BURN_OP_RGBA);

	// HARD_LIGHT
	uniqueID = HARD_LIGHT_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, HARD_LIGHT_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, HARD_LIGHT_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, HARD_LIGHT_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, HARD_LIGHT_OP_RGBA);

	// SOFT_LIGHT
	uniqueID = SOFT_LIGHT_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, SOFT_LIGHT_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, SOFT_LIGHT_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, SOFT_LIGHT_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, SOFT_LIGHT_OP_RGBA);

	// DIFFERENCE
	uniqueID = DIFFERENCE_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, DIFFERENCE_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, DIFFERENCE_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, DIFFERENCE_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, DIFFERENCE_OP_RGBA);

	// EXCLUSION
	uniqueID = EXCLUSION_OFFSET;
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_POW2, EXCLUSION_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_POW2, EXCLUSION_OP_RGBA);
	gUniqueProgramsID[uniqueID++] = GenerateProgram(COLOR_PAINT, READ_TEX0_RECT, EXCLUSION_OP_RGBA);
	for (i = LINGRAD_PAINT; i <= PATTERN_PAINT; i++)
		gUniqueProgramsID[uniqueID++] = GenerateProgram(i, READ_TEX1_RECT, EXCLUSION_OP_RGBA);


	for (i = COLOR_PAINT; i <= PATTERN_PAINT; i++) {

		// insert G_CLEAR_OP programs id
		gCompProgramsRGB[G_CLEAR_OP][i][TEXTURE_POW2] = gUniqueProgramsID[0];
		gCompProgramsRGB[G_CLEAR_OP][i][TEXTURE_RECT] = gUniqueProgramsID[0];
		gCompProgramsRGBA[G_CLEAR_OP][i][TEXTURE_POW2] = gUniqueProgramsID[0];
		gCompProgramsRGBA[G_CLEAR_OP][i][TEXTURE_RECT] = gUniqueProgramsID[0];

		// insert G_SRC_OP programs id
		gCompProgramsRGB[G_SRC_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGB[G_SRC_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_SRC_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_SRC_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_SRC_OVER_OP programs id
		gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_SRC_OVER_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_SRC_OVER_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_DST_OVER_OP programs id
		gCompProgramsRGB[G_DST_OVER_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_DST_OVER_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_DST_OVER_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_DST_OVER_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_SRC_IN_OP programs id
		gCompProgramsRGB[G_SRC_IN_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_SRC_IN_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_SRC_IN_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_SRC_IN_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_DST_IN_OP programs id
		gCompProgramsRGB[G_DST_IN_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGB[G_DST_IN_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_DST_IN_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_DST_IN_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_SRC_OUT_OP programs id
		gCompProgramsRGB[G_SRC_OUT_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_SRC_OUT_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_SRC_OUT_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_SRC_OUT_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_DST_OUT_OP programs id
		gCompProgramsRGB[G_DST_OUT_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGB[G_DST_OUT_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_DST_OUT_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_DST_OUT_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_SRC_ATOP_OP programs id
		gCompProgramsRGB[G_SRC_ATOP_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_SRC_ATOP_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_SRC_ATOP_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_SRC_ATOP_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_DST_ATOP_OP programs id
		gCompProgramsRGB[G_DST_ATOP_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_DST_ATOP_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_DST_ATOP_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + DST_ATOP_OFFSET];
		gCompProgramsRGBA[G_DST_ATOP_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + DST_ATOP_OFFSET + PAINT_TYPES_COUNT];
	
		// insert G_XOR_OP programs id
		gCompProgramsRGB[G_XOR_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_XOR_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_XOR_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_XOR_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_PLUS_OP programs id
		gCompProgramsRGB[G_PLUS_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGB[G_PLUS_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_PLUS_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_PLUS_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];

		// insert G_MULTIPLY_OP programs id
		gCompProgramsRGB[G_MULTIPLY_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_MULTIPLY_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_MULTIPLY_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + MULTIPLY_OFFSET];
		gCompProgramsRGBA[G_MULTIPLY_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + MULTIPLY_OFFSET + PAINT_TYPES_COUNT];

		// insert G_SCREEN_OP programs id
		gCompProgramsRGB[G_SCREEN_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGB[G_SCREEN_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_SCREEN_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + SCREEN_OFFSET];
		gCompProgramsRGBA[G_SCREEN_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + SCREEN_OFFSET + PAINT_TYPES_COUNT];

		// insert G_OVERLAY_OP programs id
		gCompProgramsRGB[G_OVERLAY_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_OVERLAY_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_OVERLAY_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + OVERLAY_OFFSET];
		gCompProgramsRGBA[G_OVERLAY_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + OVERLAY_OFFSET + PAINT_TYPES_COUNT];

		// insert G_DARKEN_OP programs id
		gCompProgramsRGB[G_DARKEN_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_DARKEN_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_DARKEN_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + DARKEN_OFFSET];
		gCompProgramsRGBA[G_DARKEN_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + DARKEN_OFFSET + PAINT_TYPES_COUNT];

		// insert G_LIGHTEN_OP programs id
		gCompProgramsRGB[G_LIGHTEN_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_LIGHTEN_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_LIGHTEN_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + LIGHTEN_OFFSET];
		gCompProgramsRGBA[G_LIGHTEN_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + LIGHTEN_OFFSET + PAINT_TYPES_COUNT];

		// insert G_COLOR_DODGE_OP programs id
		gCompProgramsRGB[G_COLOR_DODGE_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_COLOR_DODGE_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_COLOR_DODGE_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + COLOR_DODGE_OFFSET];
		gCompProgramsRGBA[G_COLOR_DODGE_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + COLOR_DODGE_OFFSET + PAINT_TYPES_COUNT];

		// insert G_COLOR_BURN_OP programs id
		gCompProgramsRGB[G_COLOR_BURN_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_COLOR_BURN_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_COLOR_BURN_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + COLOR_BURN_OFFSET];
		gCompProgramsRGBA[G_COLOR_BURN_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + COLOR_BURN_OFFSET + PAINT_TYPES_COUNT];

		// insert G_HARD_LIGHT_OP programs id
		gCompProgramsRGB[G_HARD_LIGHT_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_HARD_LIGHT_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_HARD_LIGHT_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + HARD_LIGHT_OFFSET];
		gCompProgramsRGBA[G_HARD_LIGHT_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + HARD_LIGHT_OFFSET + PAINT_TYPES_COUNT];

		// insert G_SOFT_LIGHT_OP programs id
		gCompProgramsRGB[G_SOFT_LIGHT_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_SOFT_LIGHT_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_SOFT_LIGHT_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + SOFT_LIGHT_OFFSET];
		gCompProgramsRGBA[G_SOFT_LIGHT_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + SOFT_LIGHT_OFFSET + PAINT_TYPES_COUNT];

		// insert G_DIFFERENCE_OP programs id
		gCompProgramsRGB[G_DIFFERENCE_OP][i][TEXTURE_POW2] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_POW2];
		gCompProgramsRGB[G_DIFFERENCE_OP][i][TEXTURE_RECT] = gCompProgramsRGB[G_SRC_OVER_OP][i][TEXTURE_RECT];
		gCompProgramsRGBA[G_DIFFERENCE_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + DIFFERENCE_OFFSET];
		gCompProgramsRGBA[G_DIFFERENCE_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + DIFFERENCE_OFFSET + PAINT_TYPES_COUNT];

		// insert G_EXCLUSION_OP programs id
		gCompProgramsRGB[G_EXCLUSION_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i];
		gCompProgramsRGB[G_EXCLUSION_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i];
		gCompProgramsRGBA[G_EXCLUSION_OP][i][TEXTURE_POW2] = gUniqueProgramsID[i + EXCLUSION_OFFSET];
		gCompProgramsRGBA[G_EXCLUSION_OP][i][TEXTURE_RECT] = gUniqueProgramsID[i + EXCLUSION_OFFSET + PAINT_TYPES_COUNT];
	}

	#undef COLOR_PAINT
	#undef LINGRAD_PAINT
	#undef RADGRAD_PAINT
	#undef CONGRAD_PAINT
	#undef PATTERN_PAINT
	#undef PAINT_TYPES_COUNT

	#undef DST_ATOP_OFFSET
	#undef MULTIPLY_OFFSET
	#undef SCREEN_OFFSET
	#undef OVERLAY_OFFSET
	#undef DARKEN_OFFSET
	#undef LIGHTEN_OFFSET
	#undef COLOR_DODGE_OFFSET
	#undef COLOR_BURN_OFFSET
	#undef HARD_LIGHT_OFFSET
	#undef SOFT_LIGHT_OFFSET
	#undef DIFFERENCE_OFFSET
	#undef EXCLUSION_OFFSET

	#undef TEXTURE_POW2
	#undef TEXTURE_RECT
}

void GOpenGLBoard::DestroyShadersTable() {

	#define TEXTURE_POW2 0
	#define TEXTURE_RECT 1

	GUInt32 i;

	for (i = 0; i < 5 * (1 + 12 * 2); ++i) {
		if (gUniqueProgramsID[i] > 0)
			glDeleteProgramsARB(1, &gUniqueProgramsID[i]);
	}

	for (i = 0; i < (1 + 12); ++i) {
		if (gUniqueGroupProgramsID[i][TEXTURE_POW2] > 0)
			glDeleteProgramsARB(1, &gUniqueGroupProgramsID[i][TEXTURE_POW2]);
		if (gUniqueGroupProgramsID[i][TEXTURE_RECT] > 0)
			glDeleteProgramsARB(1, &gUniqueGroupProgramsID[i][TEXTURE_RECT]);
	}

	#undef TEXTURE_POW2
	#undef TEXTURE_RECT
}

GBool GOpenGLBoard::CompOpPassesCount(const GCompositingOperation CompOp, GUInt32& StylePassesCount,
									  GUInt32& FrameBufferPassesCount) {

	GBool res;

	switch (CompOp) {

		case G_CLEAR_OP:
			res = G_FALSE;
			StylePassesCount = 1;
			FrameBufferPassesCount = 0;
			break;

		case G_SRC_OP:
			res = G_FALSE;
			if (!gFragmentProgramsInUse) {
				if (gAlphaBufferPresent) {
					// it writes Sca, then Sa
					StylePassesCount = 2;
					FrameBufferPassesCount = 0;
				}
				else {
					// it writes Sca
					StylePassesCount = 1;
					FrameBufferPassesCount = 0;
				}
			}
			else {
				// pixel shader writes (Sca, Sa)
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_DST_OP:
			res = G_FALSE;
			StylePassesCount = 0;
			FrameBufferPassesCount = 0;
			break;

		case G_SRC_OVER_OP:
			res = G_FALSE;
			if (!gFragmentProgramsInUse) {
				if (gAlphaBufferPresent) {
					// it writes Sca + (1 - Sa)Dca, then Sa + (1 - Sa)Da
					StylePassesCount = 2;
					FrameBufferPassesCount = 0;
				}
				else {
					// it writes Sca + (1 - Sa)Dca
					StylePassesCount = 1;
					FrameBufferPassesCount = 0;
				}
			}
			else {
				// pixel shader writes (Sca, Sa)
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_DST_OVER_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_DST_OVER_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			if (!gFragmentProgramsInUse) {
				res = G_TRUE;
				StylePassesCount = 2;
				FrameBufferPassesCount = 1;
			}
			else {
				res = G_FALSE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_SRC_IN_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_SRC_IN_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			if (!gFragmentProgramsInUse) {
				res = G_TRUE;
				StylePassesCount = 2;
				FrameBufferPassesCount = 1;
			}
			else {
				res = G_FALSE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_DST_IN_OP:
			res = G_FALSE;
			StylePassesCount = 1;
			FrameBufferPassesCount = 0;
			break;

		case G_SRC_OUT_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_SRC_OUT_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			if (!gFragmentProgramsInUse) {
				res = G_TRUE;
				StylePassesCount = 2;
				FrameBufferPassesCount = 1;
			}
			else {
				res = G_FALSE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_DST_OUT_OP:
			res = G_FALSE;
			StylePassesCount = 1;
			FrameBufferPassesCount = 0;
			break;

		case G_SRC_ATOP_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_SRC_ATOP_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			if (!gFragmentProgramsInUse) {
				res = G_TRUE;
				StylePassesCount = 2;
				FrameBufferPassesCount = 2;
			}
			else {
				res = G_FALSE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_DST_ATOP_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_DST_ATOP_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			res = G_TRUE;
			if (!gFragmentProgramsInUse) {
				StylePassesCount = 2;
				FrameBufferPassesCount = 1;
			}
			else {
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_XOR_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_XOR_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			if (!gFragmentProgramsInUse) {
				res = G_TRUE;
				StylePassesCount = 2;
				FrameBufferPassesCount = 1;
			}
			else {
				res = G_FALSE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_PLUS_OP:
			res = G_FALSE;
			if (!gFragmentProgramsInUse) {
				if (gAlphaBufferPresent) {
					// it writes Sca, then Sa
					StylePassesCount = 2;
					FrameBufferPassesCount = 0;
				}
				else {
					// it writes Sca
					StylePassesCount = 1;
					FrameBufferPassesCount = 0;
				}
			}
			else {
				// pixel shader writes (Sca, Sa)
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_MULTIPLY_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_MULTIPLY_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_MULTIPLY_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_SCREEN_OP:
			if (!gFragmentProgramsInUse) {
				res = G_TRUE;
				if (gAlphaBufferPresent) {
					StylePassesCount = 2;
					FrameBufferPassesCount = 2;
				}
				else {
					StylePassesCount = 1;
					FrameBufferPassesCount = 1;
				}
			}
			else {
				if (gAlphaBufferPresent) {
					res = G_TRUE;
					StylePassesCount = 1;
					FrameBufferPassesCount = 0;
				}
				else {
					res = G_FALSE;
					StylePassesCount = 1;
					FrameBufferPassesCount = 0;
				}
			}
			break;

		case G_OVERLAY_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_OVERLAY_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_OVERLAY_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_DARKEN_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_DARKEN_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_DARKEN_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_LIGHTEN_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_LIGHTEN_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_LIGHTEN_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_COLOR_DODGE_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_COLOR_DODGE_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_COLOR_DODGE_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_COLOR_BURN_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_COLOR_BURN_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_COLOR_BURN_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_HARD_LIGHT_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_HARD_LIGHT_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_HARD_LIGHT_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_SOFT_LIGHT_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_SOFT_LIGHT_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_SOFT_LIGHT_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_DIFFERENCE_OP:
			if (!gAlphaBufferPresent) {
				G_DEBUG("CompOpPassesCount: G_DIFFERENCE_OP without alpha channel, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else
			if (!gFragmentProgramsInUse) {
				G_DEBUG("CompOpPassesCount: G_DIFFERENCE_OP without fragment programs support, a G_SRC_OVER_OP fallback operation will be used");
				return CompOpPassesCount(G_SRC_OVER_OP, StylePassesCount, FrameBufferPassesCount);
			}
			else {
				res = G_TRUE;
				StylePassesCount = 1;
				FrameBufferPassesCount = 0;
			}
			break;

		case G_EXCLUSION_OP:
			if (!gFragmentProgramsInUse) {
				res = G_TRUE;
				if (gAlphaBufferPresent) {
					StylePassesCount = 2;
					FrameBufferPassesCount = 2;
				}
				else {
					StylePassesCount = 1;
					FrameBufferPassesCount = 1;
				}
			}
			else {
				if (gAlphaBufferPresent) {
					res = G_TRUE;
					StylePassesCount = 1;
					FrameBufferPassesCount = 0;
				}
				else {
					res = G_FALSE;
					StylePassesCount = 1;
					FrameBufferPassesCount = 0;
				}
			}
			break;

		default:
			res = G_FALSE;
			StylePassesCount = 0;
			FrameBufferPassesCount = 0;
			break;
	}
	return res;
}

GBool GOpenGLBoard::SetGLStyleCompOp(const GCompositingOperation CompOp, const GUInt32 PassIndex) {

	switch (CompOp) {

		case G_CLEAR_OP:
			G_ASSERT(PassIndex == 0);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDisable(GL_BLEND);
			return G_FALSE;
			break;

		case G_SRC_OP:
			if (!gFragmentProgramsInUse) {
				if (gAlphaBufferPresent) {
					G_ASSERT(PassIndex == 0 || PassIndex == 1);
					if (PassIndex == 0) {
						// it writes Sca
						glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
						glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
						glEnable(GL_BLEND);
					}
					else {
						// it writes Sa
						glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
						glDisable(GL_BLEND);
					}
				}
				else {
					// it writes Sca
					G_ASSERT(PassIndex == 0);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glEnable(GL_BLEND);
				}
			}
			else {
				// pixel shader writes (Sca, Sa)
				G_ASSERT(PassIndex == 0);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDisable(GL_BLEND);
			}
			return G_FALSE;
			break;

		case G_DST_OP:
			// do nothing
			return G_FALSE;
			break;

		case G_SRC_OVER_OP:
			if (!gFragmentProgramsInUse) {
			
				if (gAlphaBufferPresent) {
					G_ASSERT(PassIndex == 0 || PassIndex == 1);
					// it writes Sca + (1 - Sa)Dca
					if (PassIndex == 0) {
						glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						glEnable(GL_BLEND);
					}
					// it writes Sa + (1 - Sa)Da
					else {
						glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
						glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
						glEnable(GL_BLEND);
					}
				}
				else {
					// it writes Sca + (1 - Sa)Dca
					G_ASSERT(PassIndex == 0);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glEnable(GL_BLEND);
				}
			}
			else {
				// it writes Sca + (1 - Sa)Dca, and Sa + (1 - Sa)Da at the same time
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
			}
			return G_FALSE;
			break;

		case G_DST_OVER_OP:
			if (!gAlphaBufferPresent) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			
			if (!gFragmentProgramsInUse) {
				G_ASSERT(PassIndex <= 1);

				if (PassIndex == 0) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				}
				else
				if (PassIndex == 1) {
					glDisable(GL_BLEND);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				}
				return G_TRUE;
			}
			else {
				G_ASSERT(PassIndex == 0);
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glEnable(GL_BLEND);
				return G_FALSE;
			}
			break;

		case G_SRC_IN_OP:
			if (!gAlphaBufferPresent) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			
			if (!gFragmentProgramsInUse) {
				G_ASSERT(PassIndex <= 1);
				if (PassIndex == 0) {
					// it writes Sca
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				}
				else
				// it writes Sa
				if (PassIndex == 1) {
					glDisable(GL_BLEND);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				}
				return G_TRUE;
			}
			else {
				// pixel shader writes (Sca, Sa)
				G_ASSERT(PassIndex == 0);
				glBlendFunc(GL_DST_ALPHA, GL_ZERO);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glEnable(GL_BLEND);
				return G_FALSE;
			}
			break;

		case G_DST_IN_OP:
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
			glEnable(GL_BLEND);
			return G_FALSE;
			break;

		case G_SRC_OUT_OP:
			if (!gAlphaBufferPresent) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			
			if (!gFragmentProgramsInUse) {
				G_ASSERT(PassIndex <= 1);
				if (PassIndex == 0) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				}
				else
				if (PassIndex == 1) {
					glDisable(GL_BLEND);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				}
				return G_TRUE;
			}
			else {
				G_ASSERT(PassIndex == 0);
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glEnable(GL_BLEND);
				return G_FALSE;
			}
			break;

		case G_DST_OUT_OP:
			G_ASSERT(PassIndex == 0);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			return G_FALSE;
			break;

		case G_SRC_ATOP_OP:
			if (!gAlphaBufferPresent) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			if (!gFragmentProgramsInUse) {
				G_ASSERT(PassIndex <= 1);
				if (PassIndex == 0) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				}
				else
				if (PassIndex == 1) {
					glDisable(GL_BLEND);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				}
				return G_TRUE;
			}
			else {
				G_ASSERT(PassIndex == 0);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
				return G_FALSE;
			}
			break;

		case G_DST_ATOP_OP:
			if (!gAlphaBufferPresent) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			
			if (!gFragmentProgramsInUse) {
				G_ASSERT(PassIndex <= 1);

				if (PassIndex == 0) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				}
				else
				if (PassIndex == 1) {
					glDisable(GL_BLEND);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				}
			}
			else {
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDisable(GL_BLEND);
			}
			return G_TRUE;
			break;

		case G_XOR_OP:
			if (!gAlphaBufferPresent) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}

			if (!gFragmentProgramsInUse) {
				G_ASSERT(PassIndex <= 1);

				if (PassIndex == 0) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				}
				else
				if (PassIndex == 1) {
					glDisable(GL_BLEND);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				}
				return G_TRUE;
			}
			else {
				G_ASSERT(PassIndex == 0);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				return G_FALSE;
			}
			break;

		case G_PLUS_OP:
			if (!gFragmentProgramsInUse) {
				if (gAlphaBufferPresent) {
					G_ASSERT(PassIndex == 0 || PassIndex == 1);
					if (PassIndex == 0) {
						// it writes Sca
						glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE);
						glEnable(GL_BLEND);
					}
					else {
						// it writes Sa
						glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
						glBlendFunc(GL_ONE, GL_ONE);
						glEnable(GL_BLEND);
					}
				}
				else {
					// it writes Sca
					G_ASSERT(PassIndex == 0);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					glEnable(GL_BLEND);
				}
			}
			else {
				G_ASSERT(PassIndex == 0);
				// pixel shader writes (Sca, Sa)
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glBlendFunc(GL_ONE, GL_ONE);
				glEnable(GL_BLEND);
			}
			return G_FALSE;
			break;

		case G_MULTIPLY_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_SCREEN_OP:
			if (!gFragmentProgramsInUse) {

				if (PassIndex == 0) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				}
				else
				if (PassIndex == 1) {
					G_ASSERT(gAlphaBufferPresent == G_TRUE);
					glDisable(GL_BLEND);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				}
				return G_TRUE;
			}
			else {
				G_ASSERT(PassIndex == 0);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDisable(GL_BLEND);
				if (gAlphaBufferPresent)
					return G_TRUE;
				else
					return G_FALSE;
			}
			break;

		case G_OVERLAY_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_DARKEN_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_LIGHTEN_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_COLOR_DODGE_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_COLOR_BURN_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_HARD_LIGHT_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_SOFT_LIGHT_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_DIFFERENCE_OP:
			if (!gAlphaBufferPresent || !gFragmentProgramsInUse) {
				return SetGLStyleCompOp(G_SRC_OVER_OP, PassIndex);
			}
			G_ASSERT(PassIndex == 0);
			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			return G_TRUE;
			break;

		case G_EXCLUSION_OP:
			if (!gFragmentProgramsInUse) {

				if (PassIndex == 0) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
				}
				else
				if (PassIndex == 1) {
					G_ASSERT(gAlphaBufferPresent == G_TRUE);
					glDisable(GL_BLEND);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
				}
				return G_TRUE;
			}
			else {
				G_ASSERT(PassIndex == 0);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDisable(GL_BLEND);
				if (gAlphaBufferPresent)
					return G_TRUE;
				else
					return G_FALSE;
			}
			break;
		
		default:
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDisable(GL_BLEND);
			return G_FALSE;
			break;
	}
}

#undef IDENTITY_OP_RGBA
#undef READ_TEX0
#undef READ_TEX1
#undef DST_ATOP_OP_RGBA
#undef MULTIPLY_OP_RGBA
#undef SCREEN_OP_RGBA
#undef OVERLAY_OP_RGBA
#undef DARKEN_OP_RGBA
#undef LIGHTEN_OP_RGBA
#undef COLOR_DODGE_OP_RGBA
#undef COLOR_BURN_OP_RGBA
#undef HARD_LIGHT_OP_RGBA
#undef SOFT_LIGHT_OP_RGBA
#undef DIFFERENCE_OP_RGBA
#undef EXCLUSION_OP_RGBA
#undef END_PROGRAM_RGBA

};	// end namespace Amanith
