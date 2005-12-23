/****************************************************************************
** $file: amanith/src/gopenglext.cpp   0.2.0.0   edited Dec, 12 2005
**
** Opengl extension manager implementation.
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

#include "amanith/gopenglext.h"
#include "amanith/support/gutilities.h"

/*!
	\file gopenglext.cpp
	\brief Opengl extensions manager implementation.
*/

namespace Amanith {

/*!
	\class GOpenglExt
	\brief OpenGL extensions manager.

	GOpenglExt implements a simple OpenGL extension manager.\n
	It's based on the wonderful glew library (http://glew.sourceforge.net) for the low level stuff.
	Using the manager we can know some useful informations about the used OpenGL device, like number of texture units,
	multitexture support, shaders support and so on.
*/

//! Function used for OpenGL errors. It use qDebug() for output printing
#define PrintOpenGLError() PrintOglError(__FILE__, __LINE__)

/*!
	It initializes glew library, and builds the internal list of supported functions.
*/
GOpenglExt::GOpenglExt() {

	GLenum err = glewInit();
	if (err == GLEW_OK)
		InitSupportedFunctions();
	else
		G_DEBUG("GLEW has not been loaded successfully!");
}

GUInt32 GOpenglExt::PowerOfTwo(const GUInt32 Value) {

	if (Value >= (GUInt32)(1 << 31))
		return (GUInt32)(1 << 31);

	GUInt32 v = 1;
	while (v < Value)
		v <<= 1;

	return v;
}

const GStringList& GOpenglExt::SupportedFunctions() const {

	return gSupportedFunctions;
}

/*!
	Print a possible pending OpenGL error. Error message is delivered to the standard error output.\n
	This function uses the glGetError call, to see if there was a pending error.
	The two parameters are frequently used in conjunction with __FILE__ and __LINE__ macros. Example:
\code
if (Manager->IsArbShadersSupported()) {
	glCompileShaderARB(brickVS);
	// Check for OpenGL errors; in case of error, standard error output will be filled by
	// and extended error message built by PrintOglError function
	if (Manager->PrintOglError(__FILE__, __LINE__))
		return -1;
}
\endcode
	\param FileName a string containing the file name that will be appended into error message.
	\param Line a line number that specifies where the error has been found in source code.
	\return G_TRUE if an OpenGL error occurred, false otherwise.
*/
GBool GOpenglExt::PrintOglError(const GString& FileName, const GInt32 Line) {
	
	GLenum glErr;
	GBool retCode = G_FALSE;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		GString aux = "glError in file ";
		aux += FileName;
		aux += " @ line ";
		aux += StrUtils::ToString(Line);
		aux += " : ";
		aux += GString((GChar8*)gluErrorString(glErr));
		G_DEBUG("glError:\n" + aux);
		glErr = glGetError();
		retCode = G_TRUE;
	}
	return retCode;
}

//
//! Print out the information log for a shader object or a program object
//
void GOpenglExt::PrintInfoLog(GLhandleARB obj) {

	GInt32 infologLength = 0;
	GInt32 charsWritten  = 0;
	GLcharARB *infoLog;

	PrintOpenGLError();  // Check for OpenGL errors

	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);
	PrintOpenGLError();  // Check for OpenGL errors

	if (infologLength > 0) {
		infoLog = new GLcharARB[infologLength];
		if (infoLog == NULL) {
			G_DEBUG("ERROR: Could not allocate InfoLog buffer\n");
			exit(1);
		}
		glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
		G_DEBUG("InfoLog:\n" + GString((char*)infoLog));
		delete [] infoLog;
	}
	PrintOpenGLError();  // Check for OpenGL errors
}

/*!
	\param FunctionName a string containing the function name we are asking support for (ex: "glCopyTexSubImage3D")
	\return G_TRUE if function is supported, G_FALSE otherwise
	\note Comparison is case-insensitive.\n
	An equivalent (and faster) method, is to check the function pointer directly, for example:
\code
if (Manager->IsFunctionSupported("glCopyTexSubImage3D") {
	...
}
// or equivalently
if (glCopyTexSubImage3D) {
...
}
\endcode
*/
GBool GOpenglExt::IsFunctionSupported(const GString& FunctionName) const {

	GStringList::const_iterator it;

	it = std::lower_bound(gSupportedFunctions.begin(), gSupportedFunctions.end(), FunctionName);
	if (it == gSupportedFunctions.end() || !StrUtils::SameText(*it, FunctionName))
		return G_FALSE;
	return G_TRUE;
}

/*!
	\param Version a string containing an OpenGL version (ex. "1.4")
	\note .* and .x wildcard characters are supported. So IsGLVersionSupported("1.x") and IsGLVersionSupported("1.*")
	are valid calls. The first call returns G_TRUE if any 1.x OpenGL version is supported (1.1 or 1.2 or 1.3 or 1.4 or
	1.5). The second call returns G_TRUE if all 1.x OpenGL versions are supported (1.1 and 1.2 and 1.3 and 1.4 and 1.5).
*/
GBool GOpenglExt::IsGLVersionSupported(const GString& Version) const {

	// if minor version is not essential we put it to minimum
	if (StrUtils::Upper(StrUtils::Right(Version, 2)) == ".X") {
		GStringList l = StrUtils::Split(Version, ".", G_FALSE);
		GString maj = GString(l.front());
		GInt32 majI = StrUtils::ToInt(maj);
		return IsGLVersionSupported(majI, 0);
	}

	if (StrUtils::Upper(StrUtils::Right(Version, 2)) == ".*") {

		GStringList l = StrUtils::Split(Version, ".", G_FALSE);
		GString maj = GString(l.front());
		GInt32 majI = StrUtils::ToInt(maj);

		if (majI == 1)
			return IsGLVersionSupported(1, 5);
		else
		if (majI == 2)
			return IsGLVersionSupported(2, 0);
		else
			return G_FALSE;
	}

	if (Version == "1.1" && GLEW_VERSION_1_1)
		return G_TRUE;
	else
	if (Version == "1.2" && GLEW_VERSION_1_2)
		return G_TRUE;
	else
	if (Version == "1.3" && GLEW_VERSION_1_3)
		return G_TRUE;
	else
	if (Version == "1.4" && GLEW_VERSION_1_4)
		return G_TRUE;
	else
	if (Version == "1.5" && GLEW_VERSION_1_5)
		return G_TRUE;
	else
	if (Version == "2.0" && GLEW_VERSION_2_0)
		return G_TRUE;
	else
		return G_FALSE;
}

/*!
	\param MajVers major version number to test
	\param MinVers minor version number to test
	\return G_TRUE if OpenGL MajVers.MinVers is supported, G_FALSE otherwise
*/
GBool GOpenglExt::IsGLVersionSupported(const GInt32 MajVers, const GInt32 MinVers) const {

	if (MajVers > 2)
		return G_FALSE;
	else
	if (MajVers <= 0)
		return G_FALSE;
	else {
		if (MajVers == 1) {
			switch (MinVers) {
			case 0:
			case 1:
				if (GLEW_VERSION_1_1)
					return G_TRUE;
				return G_FALSE;
				break;
			case 2:
				if (GLEW_VERSION_1_2)
					return G_TRUE;
				return G_FALSE;
				break;
			case 3:
				if (GLEW_VERSION_1_3)
					return G_TRUE;
				return G_FALSE;
				break;
			case 4:
				if (GLEW_VERSION_1_4)
					return G_TRUE;
				return G_FALSE;
				break;
			case 5:
				if (GLEW_VERSION_1_5)
					return G_TRUE;
				return G_FALSE;
				break;
			default:
				return G_FALSE;
			}
		}
		else
		if (MajVers == 2) {
			switch (MinVers) {
			case 0:
				if (GLEW_VERSION_2_0)
					return G_TRUE;
				return G_FALSE;
				break;
			default:
				return G_FALSE;
			}
		}
		else
			return G_FALSE;
	}
}

/*!
	\param ExtensionName a string that specifies the extension name we are asking support for (ex: "GL_ARB_multitexture")
	\return G_TRUE if extension is supported, G_FALSE otherwise;
	\note Comparison is made in case-insensitive manner.
*/
GBool GOpenglExt::IsGLExtensionSupported(const GString& ExtensionName) const {

	if (glewGetExtension(StrUtils::ToAscii(ExtensionName)))
		return G_TRUE;
	return G_FALSE;
}

/*!
	\return G_TRUE if GL_ARB_multitexture extension is supported, G_FALSE otherwise.
*/
GBool GOpenglExt::IsMultitextureSupported() const {

	if (glewGetExtension("GL_ARB_multitexture") && glewGetExtension("GL_ARB_texture_env_combine"))
		return G_TRUE;
	return G_FALSE;
}

/*!
	\return G_TRUE if GL_ARB_texture_cube_map extension is supported, G_FALSE otherwise.
*/
GBool GOpenglExt::IsCubemapSupported() const {

	if (glewGetExtension("GL_ARB_texture_cube_map"))
		return G_TRUE;
	return G_FALSE;
}

/*!
	\return G_TRUE if GL_ARB_texture_cube_map extension is supported, G_FALSE otherwise.
*/
GBool GOpenglExt::IsRectTextureSupported() {

	if (glewGetExtension("GL_EXT_texture_rectangle") || glewGetExtension("GL_ARB_texture_rectangle") || glewGetExtension("GL_NV_texture_rectangle"))
		return G_TRUE;
	return G_FALSE;
}

/*!
	\return G_TRUE if GL_ARB_texture_cube_map or GL_IBM_texture_mirrored_repeat extension is supported, G_FALSE otherwise.
*/
GBool GOpenglExt::IsMirroredRepeatSupported() const {

	if (glewGetExtension("GL_ARB_texture_mirrored_repeat") || glewGetExtension("GL_IBM_texture_mirrored_repeat"))
		return G_TRUE;
	return G_FALSE;
}

/*!
	\return G_TRUE if both GL_ARB_vertex_program and GL_ARB_fragment_program extensions are supported, G_FALSE otherwise.
*/
GBool GOpenglExt::IsArbProgramsSupported() const {

	if (glewGetExtension("GL_ARB_vertex_program") && glewGetExtension("GL_ARB_fragment_program"))
		return G_TRUE;
	return G_FALSE;
}

/*!
	\return G_TRUE if GL_ARB_fragment_shader, GL_ARB_shader_objects, GL_ARB_shading_language_100 and
	GL_ARB_vertex_shader extensions are supported, G_FALSE otherwise.
*/
GBool GOpenglExt::IsArbShadersSupported() const {

	if (glewGetExtension("GL_ARB_shading_language_100") && glewGetExtension("GL_ARB_vertex_shader") &&
		glewGetExtension("GL_ARB_fragment_shader") && glewGetExtension("GL_ARB_shader_objects"))
		return G_TRUE;
	return G_FALSE;
}

/*!
	\return G_TRUE if GL_ARB_occlusion_query extension is supported, G_FALSE otherwise.
*/
GBool GOpenglExt::IsOcclusionQuerySupported() const {

	if (glewGetExtension("GL_ARB_occlusion_query"))
		return G_TRUE;
	return G_FALSE;
}

GInt32 GOpenglExt::TextureUnitsCount() const {

	GLint num = 1;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &num);
	if (num < 0)
		num = 0;
	return (GInt32)num;
}

GUInt32 GOpenglExt::MaxConvolutionWidth() const {

	GLint num = 0;
	glGetIntegerv(GL_MAX_CONVOLUTION_WIDTH, &num);
	if (num < 0)
		num = 0;
	return (GUInt32)num;
}

GUInt32 GOpenglExt::MaxConvolutionHeight() const {

	GLint num = 0;
	glGetIntegerv(GL_MAX_CONVOLUTION_HEIGHT, &num);
	if (num < 0)
		num = 0;
	return (GUInt32)num;
}

GUInt32 GOpenglExt::MaxTextureSize() const {

	GLint num = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &num);
	if (num < 0)
		num = 0;
	return (GUInt32)num;
}

GUInt32 GOpenglExt::RedBits() const {

	GLint num = 0;
	glGetIntegerv(GL_RED_BITS, &num);
	return (GUInt32)num;
}

GUInt32 GOpenglExt::GreenBits() const {

	GLint num = 0;
	glGetIntegerv(GL_GREEN_BITS, &num);
	return (GUInt32)num;
}

GUInt32 GOpenglExt::BlueBits() const {

	GLint num = 0;
	glGetIntegerv(GL_BLUE_BITS, &num);
	return (GUInt32)num;
}

GUInt32 GOpenglExt::AlphaBits() const {

	GLint num = 0;
	glGetIntegerv(GL_ALPHA_BITS, &num);
	return (GUInt32)num;
}

GUInt32 GOpenglExt::DepthBits() const {

	GLint num = 0;
	glGetIntegerv(GL_DEPTH_BITS, &num);
	return (GUInt32)num;
}

GUInt32 GOpenglExt::StencilBits() const {

	GLint num = 0;
	glGetIntegerv(GL_STENCIL_BITS, &num);
	return (GUInt32)num;
}

GUInt32 GOpenglExt::MultiSamples() const {

	GLint num = 0;
	glGetIntegerv(GL_SAMPLES_ARB, &num);
	return (GUInt32)num;
}

void GOpenglExt::GlewInfoFunc(const GString& Name, GBool Undefined) {

	if (!Undefined)
		gSupportedFunctions.push_back(Name);
}

void GOpenglExt::InitSupportedFunctions() {

	// GL_VERSION_1_2
	GlewInfoFunc("glCopyTexSubImage3D", glCopyTexSubImage3D == NULL);
	GlewInfoFunc("glDrawRangeElements", glDrawRangeElements == NULL);
	GlewInfoFunc("glTexImage3D", glTexImage3D == NULL);
	GlewInfoFunc("glTexSubImage3D", glTexSubImage3D == NULL);

	// GL_VERSION_1_3
	GlewInfoFunc("glActiveTexture", glActiveTexture == NULL);
	GlewInfoFunc("glClientActiveTexture", glClientActiveTexture == NULL);
	GlewInfoFunc("glCompressedTexImage1D", glCompressedTexImage1D == NULL);
	GlewInfoFunc("glCompressedTexImage2D", glCompressedTexImage2D == NULL);
	GlewInfoFunc("glCompressedTexImage3D", glCompressedTexImage3D == NULL);
	GlewInfoFunc("glCompressedTexSubImage1D", glCompressedTexSubImage1D == NULL);
	GlewInfoFunc("glCompressedTexSubImage2D", glCompressedTexSubImage2D == NULL);
	GlewInfoFunc("glCompressedTexSubImage3D", glCompressedTexSubImage3D == NULL);
	GlewInfoFunc("glGetCompressedTexImage", glGetCompressedTexImage == NULL);
	GlewInfoFunc("glLoadTransposeMatrixd", glLoadTransposeMatrixd == NULL);
	GlewInfoFunc("glLoadTransposeMatrixf", glLoadTransposeMatrixf == NULL);
	GlewInfoFunc("glMultTransposeMatrixd", glMultTransposeMatrixd == NULL);
	GlewInfoFunc("glMultTransposeMatrixf", glMultTransposeMatrixf == NULL);
	GlewInfoFunc("glMultiTexCoord1d", glMultiTexCoord1d == NULL);
	GlewInfoFunc("glMultiTexCoord1dv", glMultiTexCoord1dv == NULL);
	GlewInfoFunc("glMultiTexCoord1f", glMultiTexCoord1f == NULL);
	GlewInfoFunc("glMultiTexCoord1fv", glMultiTexCoord1fv == NULL);
	GlewInfoFunc("glMultiTexCoord1i", glMultiTexCoord1i == NULL);
	GlewInfoFunc("glMultiTexCoord1iv", glMultiTexCoord1iv == NULL);
	GlewInfoFunc("glMultiTexCoord1s", glMultiTexCoord1s == NULL);
	GlewInfoFunc("glMultiTexCoord1sv", glMultiTexCoord1sv == NULL);
	GlewInfoFunc("glMultiTexCoord2d", glMultiTexCoord2d == NULL);
	GlewInfoFunc("glMultiTexCoord2dv", glMultiTexCoord2dv == NULL);
	GlewInfoFunc("glMultiTexCoord2f", glMultiTexCoord2f == NULL);
	GlewInfoFunc("glMultiTexCoord2fv", glMultiTexCoord2fv == NULL);
	GlewInfoFunc("glMultiTexCoord2i", glMultiTexCoord2i == NULL);
	GlewInfoFunc("glMultiTexCoord2iv", glMultiTexCoord2iv == NULL);
	GlewInfoFunc("glMultiTexCoord2s", glMultiTexCoord2s == NULL);
	GlewInfoFunc("glMultiTexCoord2sv", glMultiTexCoord2sv == NULL);
	GlewInfoFunc("glMultiTexCoord3d", glMultiTexCoord3d == NULL);
	GlewInfoFunc("glMultiTexCoord3dv", glMultiTexCoord3dv == NULL);
	GlewInfoFunc("glMultiTexCoord3f", glMultiTexCoord3f == NULL);
	GlewInfoFunc("glMultiTexCoord3fv", glMultiTexCoord3fv == NULL);
	GlewInfoFunc("glMultiTexCoord3i", glMultiTexCoord3i == NULL);
	GlewInfoFunc("glMultiTexCoord3iv", glMultiTexCoord3iv == NULL);
	GlewInfoFunc("glMultiTexCoord3s", glMultiTexCoord3s == NULL);
	GlewInfoFunc("glMultiTexCoord3sv", glMultiTexCoord3sv == NULL);
	GlewInfoFunc("glMultiTexCoord4d", glMultiTexCoord4d == NULL);
	GlewInfoFunc("glMultiTexCoord4dv", glMultiTexCoord4dv == NULL);
	GlewInfoFunc("glMultiTexCoord4f", glMultiTexCoord4f == NULL);
	GlewInfoFunc("glMultiTexCoord4fv", glMultiTexCoord4fv == NULL);
	GlewInfoFunc("glMultiTexCoord4i", glMultiTexCoord4i == NULL);
	GlewInfoFunc("glMultiTexCoord4iv", glMultiTexCoord4iv == NULL);
	GlewInfoFunc("glMultiTexCoord4s", glMultiTexCoord4s == NULL);
	GlewInfoFunc("glMultiTexCoord4sv", glMultiTexCoord4sv == NULL);
	GlewInfoFunc("glSampleCoverage", glSampleCoverage == NULL);

	// GL_VERSION_1_4
	GlewInfoFunc("glBlendColor", glBlendColor == NULL);
	GlewInfoFunc("glBlendEquation", glBlendEquation == NULL);
	GlewInfoFunc("glBlendFuncSeparate", glBlendFuncSeparate == NULL);
	GlewInfoFunc("glFogCoordPointer", glFogCoordPointer == NULL);
	GlewInfoFunc("glFogCoordd", glFogCoordd == NULL);
	GlewInfoFunc("glFogCoorddv", glFogCoorddv == NULL);
	GlewInfoFunc("glFogCoordf", glFogCoordf == NULL);
	GlewInfoFunc("glFogCoordfv", glFogCoordfv == NULL);
	GlewInfoFunc("glMultiDrawArrays", glMultiDrawArrays == NULL);
	GlewInfoFunc("glMultiDrawElements", glMultiDrawElements == NULL);
	GlewInfoFunc("glPointParameterf", glPointParameterf == NULL);
	GlewInfoFunc("glPointParameterfv", glPointParameterfv == NULL);
	GlewInfoFunc("glSecondaryColor3b", glSecondaryColor3b == NULL);
	GlewInfoFunc("glSecondaryColor3bv", glSecondaryColor3bv == NULL);
	GlewInfoFunc("glSecondaryColor3d", glSecondaryColor3d == NULL);
	GlewInfoFunc("glSecondaryColor3dv", glSecondaryColor3dv == NULL);
	GlewInfoFunc("glSecondaryColor3f", glSecondaryColor3f == NULL);
	GlewInfoFunc("glSecondaryColor3fv", glSecondaryColor3fv == NULL);
	GlewInfoFunc("glSecondaryColor3i", glSecondaryColor3i == NULL);
	GlewInfoFunc("glSecondaryColor3iv", glSecondaryColor3iv == NULL);
	GlewInfoFunc("glSecondaryColor3s", glSecondaryColor3s == NULL);
	GlewInfoFunc("glSecondaryColor3sv", glSecondaryColor3sv == NULL);
	GlewInfoFunc("glSecondaryColor3ub", glSecondaryColor3ub == NULL);
	GlewInfoFunc("glSecondaryColor3ubv", glSecondaryColor3ubv == NULL);
	GlewInfoFunc("glSecondaryColor3ui", glSecondaryColor3ui == NULL);
	GlewInfoFunc("glSecondaryColor3uiv", glSecondaryColor3uiv == NULL);
	GlewInfoFunc("glSecondaryColor3us", glSecondaryColor3us == NULL);
	GlewInfoFunc("glSecondaryColor3usv", glSecondaryColor3usv == NULL);
	GlewInfoFunc("glSecondaryColorPointer", glSecondaryColorPointer == NULL);
	GlewInfoFunc("glWindowPos2d", glWindowPos2d == NULL);
	GlewInfoFunc("glWindowPos2dv", glWindowPos2dv == NULL);
	GlewInfoFunc("glWindowPos2f", glWindowPos2f == NULL);
	GlewInfoFunc("glWindowPos2fv", glWindowPos2fv == NULL);
	GlewInfoFunc("glWindowPos2i", glWindowPos2i == NULL);
	GlewInfoFunc("glWindowPos2iv", glWindowPos2iv == NULL);
	GlewInfoFunc("glWindowPos2s", glWindowPos2s == NULL);
	GlewInfoFunc("glWindowPos2sv", glWindowPos2sv == NULL);
	GlewInfoFunc("glWindowPos3d", glWindowPos3d == NULL);
	GlewInfoFunc("glWindowPos3dv", glWindowPos3dv == NULL);
	GlewInfoFunc("glWindowPos3f", glWindowPos3f == NULL);
	GlewInfoFunc("glWindowPos3fv", glWindowPos3fv == NULL);
	GlewInfoFunc("glWindowPos3i", glWindowPos3i == NULL);
	GlewInfoFunc("glWindowPos3iv", glWindowPos3iv == NULL);
	GlewInfoFunc("glWindowPos3s", glWindowPos3s == NULL);
	GlewInfoFunc("glWindowPos3sv", glWindowPos3sv == NULL);

	// GL_VERSION_1_5
	GlewInfoFunc("glBeginQuery", glBeginQuery == NULL);
	GlewInfoFunc("glBindBuffer", glBindBuffer == NULL);
	GlewInfoFunc("glBufferData", glBufferData == NULL);
	GlewInfoFunc("glBufferSubData", glBufferSubData == NULL);
	GlewInfoFunc("glDeleteBuffers", glDeleteBuffers == NULL);
	GlewInfoFunc("glDeleteQueries", glDeleteQueries == NULL);
	GlewInfoFunc("glEndQuery", glEndQuery == NULL);
	GlewInfoFunc("glGenBuffers", glGenBuffers == NULL);
	GlewInfoFunc("glGenQueries", glGenQueries == NULL);
	GlewInfoFunc("glGetBufferParameteriv", glGetBufferParameteriv == NULL);
	GlewInfoFunc("glGetBufferPointerv", glGetBufferPointerv == NULL);
	GlewInfoFunc("glGetBufferSubData", glGetBufferSubData == NULL);
	GlewInfoFunc("glGetQueryObjectiv", glGetQueryObjectiv == NULL);
	GlewInfoFunc("glGetQueryObjectuiv", glGetQueryObjectuiv == NULL);
	GlewInfoFunc("glGetQueryiv", glGetQueryiv == NULL);
	GlewInfoFunc("glIsBuffer", glIsBuffer == NULL);
	GlewInfoFunc("glIsQuery", glIsQuery == NULL);
	GlewInfoFunc("glMapBuffer", glMapBuffer == NULL);
	GlewInfoFunc("glUnmapBuffer", glUnmapBuffer == NULL);

	// GL_VERSION_2_0
	GlewInfoFunc("glAttachShader", glAttachShader == NULL);
	GlewInfoFunc("glBindAttribLocation", glBindAttribLocation == NULL);
	GlewInfoFunc("glBlendEquationSeparate", glBlendEquationSeparate == NULL);
	GlewInfoFunc("glCompileShader", glCompileShader == NULL);
	GlewInfoFunc("glCreateProgram", glCreateProgram == NULL);
	GlewInfoFunc("glCreateShader", glCreateShader == NULL);
	GlewInfoFunc("glDeleteProgram", glDeleteProgram == NULL);
	GlewInfoFunc("glDeleteShader", glDeleteShader == NULL);
	GlewInfoFunc("glDetachShader", glDetachShader == NULL);
	GlewInfoFunc("glDisableVertexAttribArray", glDisableVertexAttribArray == NULL);
	GlewInfoFunc("glDrawBuffers", glDrawBuffers == NULL);
	GlewInfoFunc("glEnableVertexAttribArray", glEnableVertexAttribArray == NULL);
	GlewInfoFunc("glGetActiveAttrib", glGetActiveAttrib == NULL);
	GlewInfoFunc("glGetActiveUniform", glGetActiveUniform == NULL);
	GlewInfoFunc("glGetAttachedShaders", glGetAttachedShaders == NULL);
	GlewInfoFunc("glGetAttribLocation", glGetAttribLocation == NULL);
	GlewInfoFunc("glGetProgramInfoLog", glGetProgramInfoLog == NULL);
	GlewInfoFunc("glGetProgramiv", glGetProgramiv == NULL);
	GlewInfoFunc("glGetShaderInfoLog", glGetShaderInfoLog == NULL);
	GlewInfoFunc("glGetShaderSource", glGetShaderSource == NULL);
	GlewInfoFunc("glGetShaderiv", glGetShaderiv == NULL);
	GlewInfoFunc("glGetUniformLocation", glGetUniformLocation == NULL);
	GlewInfoFunc("glGetUniformfv", glGetUniformfv == NULL);
	GlewInfoFunc("glGetUniformiv", glGetUniformiv == NULL);
	GlewInfoFunc("glGetVertexAttribPointerv", glGetVertexAttribPointerv == NULL);
	GlewInfoFunc("glGetVertexAttribdv", glGetVertexAttribdv == NULL);
	GlewInfoFunc("glGetVertexAttribfv", glGetVertexAttribfv == NULL);
	GlewInfoFunc("glGetVertexAttribiv", glGetVertexAttribiv == NULL);
	GlewInfoFunc("glIsProgram", glIsProgram == NULL);
	GlewInfoFunc("glIsShader", glIsShader == NULL);
	GlewInfoFunc("glLinkProgram", glLinkProgram == NULL);
	GlewInfoFunc("glShaderSource", glShaderSource == NULL);
	GlewInfoFunc("glStencilFuncSeparate", glStencilFuncSeparate == NULL);
	GlewInfoFunc("glStencilMaskSeparate", glStencilMaskSeparate == NULL);
	GlewInfoFunc("glStencilOpSeparate", glStencilOpSeparate == NULL);
	GlewInfoFunc("glUniform1f", glUniform1f == NULL);
	GlewInfoFunc("glUniform1fv", glUniform1fv == NULL);
	GlewInfoFunc("glUniform1i", glUniform1i == NULL);
	GlewInfoFunc("glUniform1iv", glUniform1iv == NULL);
	GlewInfoFunc("glUniform2f", glUniform2f == NULL);
	GlewInfoFunc("glUniform2fv", glUniform2fv == NULL);
	GlewInfoFunc("glUniform2i", glUniform2i == NULL);
	GlewInfoFunc("glUniform2iv", glUniform2iv == NULL);
	GlewInfoFunc("glUniform3f", glUniform3f == NULL);
	GlewInfoFunc("glUniform3fv", glUniform3fv == NULL);
	GlewInfoFunc("glUniform3i", glUniform3i == NULL);
	GlewInfoFunc("glUniform3iv", glUniform3iv == NULL);
	GlewInfoFunc("glUniform4f", glUniform4f == NULL);
	GlewInfoFunc("glUniform4fv", glUniform4fv == NULL);
	GlewInfoFunc("glUniform4i", glUniform4i == NULL);
	GlewInfoFunc("glUniform4iv", glUniform4iv == NULL);
	GlewInfoFunc("glUniformMatrix2fv", glUniformMatrix2fv == NULL);
	GlewInfoFunc("glUniformMatrix3fv", glUniformMatrix3fv == NULL);
	GlewInfoFunc("glUniformMatrix4fv", glUniformMatrix4fv == NULL);
	GlewInfoFunc("glUseProgram", glUseProgram == NULL);
	GlewInfoFunc("glValidateProgram", glValidateProgram == NULL);
	GlewInfoFunc("glVertexAttrib1d", glVertexAttrib1d == NULL);
	GlewInfoFunc("glVertexAttrib1dv", glVertexAttrib1dv == NULL);
	GlewInfoFunc("glVertexAttrib1f", glVertexAttrib1f == NULL);
	GlewInfoFunc("glVertexAttrib1fv", glVertexAttrib1fv == NULL);
	GlewInfoFunc("glVertexAttrib1s", glVertexAttrib1s == NULL);
	GlewInfoFunc("glVertexAttrib1sv", glVertexAttrib1sv == NULL);
	GlewInfoFunc("glVertexAttrib2d", glVertexAttrib2d == NULL);
	GlewInfoFunc("glVertexAttrib2dv", glVertexAttrib2dv == NULL);
	GlewInfoFunc("glVertexAttrib2f", glVertexAttrib2f == NULL);
	GlewInfoFunc("glVertexAttrib2fv", glVertexAttrib2fv == NULL);
	GlewInfoFunc("glVertexAttrib2s", glVertexAttrib2s == NULL);
	GlewInfoFunc("glVertexAttrib2sv", glVertexAttrib2sv == NULL);
	GlewInfoFunc("glVertexAttrib3d", glVertexAttrib3d == NULL);
	GlewInfoFunc("glVertexAttrib3dv", glVertexAttrib3dv == NULL);
	GlewInfoFunc("glVertexAttrib3f", glVertexAttrib3f == NULL);
	GlewInfoFunc("glVertexAttrib3fv", glVertexAttrib3fv == NULL);
	GlewInfoFunc("glVertexAttrib3s", glVertexAttrib3s == NULL);
	GlewInfoFunc("glVertexAttrib3sv", glVertexAttrib3sv == NULL);
	GlewInfoFunc("glVertexAttrib4Nbv", glVertexAttrib4Nbv == NULL);
	GlewInfoFunc("glVertexAttrib4Niv", glVertexAttrib4Niv == NULL);
	GlewInfoFunc("glVertexAttrib4Nsv", glVertexAttrib4Nsv == NULL);
	GlewInfoFunc("glVertexAttrib4Nub", glVertexAttrib4Nub == NULL);
	GlewInfoFunc("glVertexAttrib4Nubv", glVertexAttrib4Nubv == NULL);
	GlewInfoFunc("glVertexAttrib4Nuiv", glVertexAttrib4Nuiv == NULL);
	GlewInfoFunc("glVertexAttrib4Nusv", glVertexAttrib4Nusv == NULL);
	GlewInfoFunc("glVertexAttrib4bv", glVertexAttrib4bv == NULL);
	GlewInfoFunc("glVertexAttrib4d", glVertexAttrib4d == NULL);
	GlewInfoFunc("glVertexAttrib4dv", glVertexAttrib4dv == NULL);
	GlewInfoFunc("glVertexAttrib4f", glVertexAttrib4f == NULL);
	GlewInfoFunc("glVertexAttrib4fv", glVertexAttrib4fv == NULL);
	GlewInfoFunc("glVertexAttrib4iv", glVertexAttrib4iv == NULL);
	GlewInfoFunc("glVertexAttrib4s", glVertexAttrib4s == NULL);
	GlewInfoFunc("glVertexAttrib4sv", glVertexAttrib4sv == NULL);
	GlewInfoFunc("glVertexAttrib4ubv", glVertexAttrib4ubv == NULL);
	GlewInfoFunc("glVertexAttrib4uiv", glVertexAttrib4uiv == NULL);
	GlewInfoFunc("glVertexAttrib4usv", glVertexAttrib4usv == NULL);
	GlewInfoFunc("glVertexAttribPointer", glVertexAttribPointer == NULL);
	// lets sort strings by name
	gSupportedFunctions.sort();
}

}  // end namespace Amanith
