/****************************************************************************
** $file: amanith/gopenglext.h   0.1.1.0   edited Sep 24 08:00
**
** Opengl extension manager definition.
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

#ifndef GOPENGLEXT_H
#define GOPENGLEXT_H

#include "GL/glew.h"
// this inclusion to make glew linking working with some platforms (ex: Mac)
#include "amanith/gelement.h"
#ifdef G_OS_WIN
	#include <GL/wglew.h>
#endif

/*!
	\file gopenglext.h
	\brief GOpenglExt header file.
*/

namespace Amanith {

	// *********************************************************************
	//                            GOpenglExt
	// *********************************************************************

	class G_EXPORT GOpenglExt {

	private:
		//! List of supported OpenGL functions over OpenGL 1.1 version
		GStringList gSupportedFunctions;

		//! Builds the list
		void InitSupportedFunctions();

	protected:
		void GlewInfoFunc(const GString& Name, GBool Undefined);

	public:
		//! Constructor
		GOpenglExt();
		//! Check if an OpenGL function is supported
		GBool IsFunctionSupported(const GString& FunctionName) const;
		//! Check if an OpenGL version is supported. Wildcard characters are supported in this version. 
		GBool IsGLVersionSupported(const GString& Version) const;
		//! Check if an OpenGL version is supported
		GBool IsGLVersionSupported(const GInt32 MajVers, const GInt32 MinVers) const;
		//! Check if an OpenGL extension is supported
		GBool IsGLExtensionSupported(const GString& ExtensionName) const;
		//! Check if underlying OpenGL device supports multi-texturing
		GBool IsMultitextureSupported() const;
		//! Check if underlying OpenGL device supports cubemaps
		GBool IsCubemapSupported() const;
		//! Check if underlying OpenGL device supports ARB shaders
		GBool IsArbShadersSupported() const;
		//! Check if underlying OpenGL device supports ARB programs
		GBool IsArbProgramsSupported() const;
		//! Check if underlying OpenGL device supports occlusion queries
		GBool IsOcclusionQuerySupported() const;
		//! Return the number of texture units of the underlying OpenGL device
		GInt32 TextureUnitsCount() const;
		//! Return maximum acceptable filter kernel width (0 if no convolution support).
		GUInt32 MaxConvolutionWidth() const;
		//! Return maximum acceptable filter kernel height (0 if no convolution support).
		GUInt32 MaxConvolutionHeight() const;
		//! Return the maximum width or height of any texture image (without borders). 
		GUInt32 MaxTextureSize() const;
		//! Get number of bits per red component in color buffers.
		GUInt32 RedBits() const;
		//! Get number of bits per green component in color buffers.
		GUInt32 GreenBits() const;
		//! Get number of bits per blue component in color buffers.
		GUInt32 BlueBits() const;
		//! Get number of depth-buffer bitplanes.
		GUInt32 DepthBits() const;
		//! Get number of stencil bitplanes.
		GUInt32 StencilBits() const;
		//! Get the list of supported OpenGL functions over OpenGL 1.1 version
		const GStringList& SupportedFunctions() const;
		//! Print out the information log for a shader object or a program object
		void PrintInfoLog(GLhandleARB obj);
		//! Print OpenGL error message, if one.
		GBool PrintOglError(const GString& FileName, const GInt32 Line);
	};
};

#endif
