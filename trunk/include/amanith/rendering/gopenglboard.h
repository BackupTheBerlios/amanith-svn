/****************************************************************************
** $file: amanith/rendering/gopenglboard.h   0.1.1.0   edited Sep 24 08:00
**
** OpenGL based draw board.
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

#ifndef GOPENGLBOARD_H
#define GOPENGLBOARD_H

#include "amanith/rendering/gdrawboard.h"
#include "amanith/gopenglext.h"

/*!
	\file gopenglboard.h
	\brief OpenGL based draw board header file.
*/
namespace Amanith {

	// forward declaration
	class GOpenGLBoard;

	// *********************************************************************
	//                          GOpenGLGradientDesc
	// *********************************************************************
	class G_EXPORT GOpenGLGradientDesc : public GGradientDesc {

		friend class GOpenGLBoard;

	private:
		// OpenGL texture handle.
		GLuint gGradientTexture;
		// G_TRUE if specified color keys contain alpha values (so GL_BLEND must be enabled).
		GBool gAlphaKeys;

	protected:
		static void SetGLGradientQuality(const GRenderingQuality Quality);
		void UpdateOpenGLTexture(const GRenderingQuality Quality, const GUInt32 MaxTextureSize);

	public:
		// default constructor
		GOpenGLGradientDesc();
		// destructor
		~GOpenGLGradientDesc();
		// set color keys
		void SetColorKeys(const GDynArray<GKeyValue>& ColorKeys);
		// get OpenGL texture handle (valid only for linear gradients)
		inline const GLuint GradientTexture() const {
			return gGradientTexture;
		}
	};

	// *********************************************************************
	//                          GOpenGLPatternDesc
	// *********************************************************************
	class G_EXPORT GOpenGLPatternDesc : public GPatternDesc {

		friend class GOpenGLBoard;

	private:
		GLuint gPatternTexture;
		GUInt32 gMaxTextureSize;

	protected:
		static void SetGLImageQuality(const GImageQuality Quality);

	public:
		// default constructor
		GOpenGLPatternDesc();
		// destructor
		~GOpenGLPatternDesc();
		// set image
		void SetImage(const GPixelMap *Image, const GImageQuality Quality);
		// get OpenGL texture handle
		inline const GLuint PatternTexture() const {
			return gPatternTexture;
		}
	};

	struct GLGrabbedRect {
		
		GInt32 X, Y;
		GUInt32 Width, Height;
		GUInt32 TexWidth, TexHeight;
		GLenum Target; // GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_EXT
		GLenum Format;
		GLuint TexName;
		GBool IsEmpty;
		//GBool IsOpaque;
	};

	// *********************************************************************
	//                             GOpenGLBoard
	// *********************************************************************
	class G_EXPORT GOpenGLBoard : public GDrawBoard {
	private:
		GOpenglExt *gExtManager;
		GDynArray<GOpenGLGradientDesc *> gGradients;
		GDynArray<GOpenGLPatternDesc *> gPatterns;
		GBool gClipByStencil;
		GBool gShaderSupport;
		GBool gGroupOpacitySupport;

		GLint gTopStencilValue;
		GLint gMaxTopStencilValue;
		GLuint gStencilMask;
		GLuint gStencilDualMask;


		GBool gFirstClipMaskReplace;
		GList<GAABox2> gClipMasksBoxes;
		GReal gDeviation;
		GReal gFlateness; // defined as sqrt(gDeviation)
		GLGrabbedRect gGLGroupRect;

	private:
		// calculate (squared) deviation given a (squared) pixel deviation
		GReal CalcDeviation(const GReal PixelDeviation);
		// calculate (squared) pixel deviation given a (squared) deviation
		GReal CalcPixelDeviation(const GReal Deviation);

		void PushGLWindowMode();
		void PopGLWindowMode();

		void GrabFrameBuffer(const GPoint<GInt32, 2>& LowLeft, const GUInt32 Width, const GUInt32 Height,
							 GLGrabbedRect& Shot);
		void ReplaceFrameBuffer(const GLGrabbedRect& Shot);
		// stencil management
		void StencilPush();
		void StencilPop();
		void StencilReplace();
		void StencilEnableTop();
		//void StencilEnableTopAndPush();

		// double pass technique for group opacity
		void GroupFirstPass();

	protected:
		void DeleteGradients();
		void DeletePatterns();

		// OpenGL low level drawings
		static void SetGLColor(const GVectBase<GReal, 4>& Color);
		static void SetGLColor(const GVectBase<GReal, 3>& Color);
		static void SetGLModelViewMatrix(const GMatrix33& Matrix);
		static void SetGLTextureMatrix(const GMatrix33& Matrix);
		
		GBool SetGLClipEnabled(const GTargetMode Mode, const GClipOperation Operation);

		void DrawGLCircleSlice(const GPoint2& Center, const GReal Radius, const GPoint2& Start,
							 const GPoint2& End, const GReal SpanAngle, const GBool CCW);

		void DrawGLJoinLine(const GJoinStyle JoinStyle, const GReal MiterLimit,
							const GPoint2& Previous, const GPoint2& P0, const GPoint2& P1,
							const GReal Thickness);
		void DrawGLJoinLineCap(const GJoinStyle JoinStyle, const GReal MiterLimit,
								const GPoint2& Previous, const GPoint2& P0, const GPoint2& P1,
								const GReal Thickness, const GCapStyle EndCapStyle);

		void DrawGLJoin(const GPoint2& JoinCenter, const GVector2& InDirection, const GReal InDistance,
						const GVector2& OutDirection, const GReal OutDistance, const GJoinStyle JoinStyle,
						const GReal MiterLimit, const GCapStyle StartCapStyle, const GCapStyle EndCapStyle,
						const GReal Thickness);

		void DrawGLCapsLine(const GBool DoStartCap, const GCapStyle StartCapStyle,
							const GBool DoEndCap, const GCapStyle EndCapStyle,
							const GPoint2& P0, const GPoint2& P1, const GReal Thickness);

		// draw solid (non-dashed) stroke
		void DrawSolidStroke(const GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed,
							 const GReal Thickness);
		// draw dashed stroke
		void DrawDashedStroke(const GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed,
							  const GReal Thickness);

		// intersect a circle with two rays, returning two intersection points and spanned angle
		// in CCW direction to go from P0 to P1; precondition is that rays origins are inside the circle
		void IntersectRaysDisk(const GRay2& Ray0, const GRay2& Ray1, const GPoint2& Center, const GReal Radius,
							   GPoint2& P0, GPoint2& P1, GReal& SpannedAngle) const;
		// return the sign of the (signed) distance between a given circle and an axes-aligne box
		static GInt32 SignBoxDisk(const GAABox2& Box, const GPoint2& Center, const GReal Radius);
		// draw a radial-shaded disk sector
		void DrawGLShadedSector(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
								const GReal Time0, const GReal Time1,
								const GPoint2& P0, const GPoint2& P1, const GBool WholeDisk,
								const GDynArray<GKeyValue>& ColorKeys, const GColorRampInterpolation Interpolation,
								const GColorRampSpreadMode SpreadMode,
								const GReal MultAlpha) const;
		// draw a radial-shaded disk sector, specifying an axes-aligned bounding box that must entirely filled
		void DrawShadedSector(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
							  const GAABox2& BoundingBox,
							  const GDynArray<GKeyValue>& ColorKeys, const GColorRampInterpolation Interpolation,
							  const GColorRampSpreadMode SpreadMode,
							  const GReal MultAlpha, const GMatrix33& GradientMatrix) const;

		void UpdateClipMasksState();
		void ClipReplaceOverflowFix();

		//GBool GeometricRadialGradient(const GDrawStyle& Style, const GBool TestFill);
		void PushDepthMask();
		void DrawAndPopDepthMask(const GAABox2& Box, const GDrawStyle& Style, const GBool DrawFill);

		//
		void UpdateStyle(GDrawStyle& Style);
		GBool UseStyle(const GPaintType PaintType, const GVector4& Color,
					  const GOpenGLGradientDesc *Gradient, const GOpenGLPatternDesc *Pattern,
					  const GMatrix33& ModelView, const GBool UseFill);
		GBool UseStrokeStyle(const GDrawStyle& Style);
		GBool UseFillStyle(const GDrawStyle& Style);
		void UpdateDeviation(const GRenderingQuality Quality);

		// implemented from GDrawBoard
		void DoSetRenderingQuality(const GRenderingQuality Quality);
		void DoSetImageQuality(const GImageQuality Quality);
		void DoSetTargetMode(const GTargetMode Mode);
		void DoSetClipOperation(const GClipOperation Operation);
		void DoSetClipEnabled(const GBool Enabled);
		void DoPopClipMask();
		void DoSetGroupOpacity(const GReal Opacity);
		void DoGroupBegin(const GAABox2& LogicBox);
		void DoGroupEnd();
		void DoFlush();
		void DoFinish();
		void DoClear(const GReal Red, const GReal Green, const GReal Blue, const GBool ClearClipMasks);
		void DoSetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
						   const GUInt32 Width, const GUInt32 Height);
		void DoSetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top);


		// draw primitives

		// here we are sure that corners are opposite and ordered
		void DoDrawRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner);
		void DoDrawLine(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1);
		void DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2);
		void DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3);
		void DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength,
							  const GReal YSemiAxisLength, const GReal OffsetRotation,
							  const GReal StartAngle, const GReal EndAngle, const GBool CCW);
		void DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
							  const GReal OffsetRotation, const GBool LargeArc, const GBool CCW);
		void DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed);

	public:
		void DumpBuffers(const GChar8 *fNameZ, const GChar8 *fNameS);
		GOpenGLBoard(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY, const GUInt32 Width, const GUInt32 Height);
		~GOpenGLBoard();

		// read only parameters
		GUInt32 MaxDashCount() const;
		GUInt32 MaxKernelSize() const;
		GUInt32 MaxSeparableKernelSize() const;
		GUInt32 MaxColorKeys() const;
		GUInt32 MaxImageWidth() const;
		GUInt32 MaxImageHeight() const;
		GUInt32 MaxImageBytes() const;

		// paint resources
		GGradientDesc *CreateLinearGradient(const GPoint2& StartPoint, const GPoint2& EndPoint,
											const GDynArray<GKeyValue>& ColorKeys,
											const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
											const GColorRampSpreadMode SpreadMode = G_PAD_COLOR_RAMP_SPREAD,
											const GMatrix33& Matrix = G_MATRIX_IDENTITY33);

		GGradientDesc *CreateRadialGradient(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
											const GDynArray<GKeyValue>& ColorKeys,
											const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
											const GColorRampSpreadMode SpreadMode = G_PAD_COLOR_RAMP_SPREAD,
											const GMatrix33& Matrix = G_MATRIX_IDENTITY33);

		GPatternDesc *CreatePattern(const GPixelMap *Image, const GTilingMode TilingMode = G_REPEAT_TILE,
									const GAABox2 *LogicalWindow = NULL,
									const GMatrix33& Matrix = G_MATRIX_IDENTITY33);

	};

};	// end namespace Amanith

#endif
