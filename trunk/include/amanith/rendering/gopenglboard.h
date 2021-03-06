/****************************************************************************
** $file: amanith/rendering/gopenglboard.h   0.3.0.0   edited Jan, 30 2006
**
** OpenGL based draw board.
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

#ifndef GOPENGLBOARD_H
#define GOPENGLBOARD_H

#include "amanith/rendering/gdrawboard.h"
#include "amanith/2d/gtesselator2d.h"
#include "amanith/gopenglext.h"
#include <utility>

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
	/*!
		\class GOpenGLGradientDesc
		\brief This is an OpenGL-based implementation gradient descriptor class.

		In this implementation, the basic GGradientDesc class has been extended to include reference to some
		OpenGL textures used to draw the gradient. In addition all color keys components are clamped in the range [0; 1].
	*/
	class G_EXPORT GOpenGLGradientDesc : public GGradientDesc {

		friend class GOpenGLBoard;

	private:
		//! OpenGL texture handle.
		GLuint gGradientTexture;
		//! G_TRUE if specified color keys contain alpha values (so GL_BLEND must be enabled).
		GBool gAlphaKeys;
		GDynArray<GVector4> gInTangents;
		GDynArray<GVector4> gOutTangents;

		void GenerateTexture1D(const GInt32 Size, GPixelMap& Bitmap, const GBool ReflectKeys);

	protected:
		static void SetGLGradientQuality(const GRenderingQuality Quality);
		void UpdateOpenGLTextureLinRad(const GRenderingQuality Quality, const GUInt32 MaxTextureSize,
									   const GBool MirroredRepeatSupported);
		void UpdateOpenGLTextureCon(const GRenderingQuality Quality, const GUInt32 MaxTextureSize,
									const GInt32 Atan2LookupTableSize, const GFloat *gAtan2LookupTable);
		void UpdateHermiteTangents();

	public:
		//! Default constructor.
		GOpenGLGradientDesc();
		//! Destructor, it deletes created OpenGL textures.
		~GOpenGLGradientDesc();
		//! Set color keys; this overridden implementation claps all color components in the range [0; 1].
		void SetColorKeys(const GDynArray<GKeyValue>& ColorKeys);
		//! Get OpenGL texture handle (valid only for linear gradients and for shader versions of radial and conical gradients).
		inline const GLuint GradientTexture() const {
			return gGradientTexture;
		}
	};

	// *********************************************************************
	//                          GOpenGLPatternDesc
	// *********************************************************************
	/*!
		\class GOpenGLPatternDesc
		\brief This is an OpenGL-based implementation pattern descriptor class.

		In this implementation, the basic GPatternDesc class has been extended to include reference to some
		OpenGL textures used to draw the pattern.
	*/
	class G_EXPORT GOpenGLPatternDesc : public GPatternDesc {

		friend class GOpenGLBoard;

	private:
		//! OpenGL texture handle.
		GLuint gPatternTexture;
		//! OpenGL mirrored texture handle (useful for those hardware where "mirrored repeat" extension is not supported
		GLuint gPatternMirroredTexture;
		//! Maximum size of permitted OpenGL texture. Written only at creation time by GOpenGLBoard.
		GUInt32 gMaxTextureSize;
		//! G_TRUE if "mirrored repeat" is supported, else G_FALSE. Written only at creation time by GOpenGLBoard.
		GUInt32 gMirroredRepeatSupport;

	protected:
		static void SetGLImageQuality(const GImageQuality Quality);

	public:
		//! Default constructor.
		GOpenGLPatternDesc();
		//! Destructor, it deletes generated OpenGL texture.
		~GOpenGLPatternDesc();
		/*!
			Set pattern image; in this implementation an OpenGL texture is created, and according to Quality
			parameter mipmaps could be created too.

			\note <b>if the Image has non-power-of-two dimensions, then it will be rescaled to the closest
			power-of-two dimensions. This is needed because tile; rectangular textures, in OpenGL,
			do not support repeat and reflect tile modes</b>.
		*/
		void SetImage(const GPixelMap *Image, const GImageQuality Quality);
		//! Get OpenGL texture handle.
		inline const GLuint PatternTexture() const {
			return gPatternTexture;
		}
	};

	// *********************************************************************
	//                          GOpenGLDrawStyle
	// *********************************************************************
	/*!
		\class GOpenGLDrawStyle
		\brief This is an OpenGL-based implementation drawstyle descriptor class.

		In this implementation, the basic GDrawStyle class has been extended to include some precalculated
		constants, very useful to speed up stroke and join computation.
	*/
	class G_EXPORT GOpenGLDrawStyle : public GDrawStyle {

		friend class GOpenGLBoard;

	private:
		//! Defined as Miter * Thickness
		GReal gMiterMulThickness;
		// ! Defined as 1 / (2 * acos(1 - Deviation / Thickness))
		GReal gRoundJoinAuxCoef;

	public:
		//! Constructor.
		GOpenGLDrawStyle();
		//! Set stroke color. This implementation clamps each entry in the range [0; 1].
		void SetStrokeColor(const GVectBase<GReal, 4>& Color);
		//! Set fill color. This implementation clamps each entry in the range [0; 1].
		void SetFillColor(const GVectBase<GReal, 4>& Color);
		//! Set srtoke miter limit; in this implementation, additional internal constants are calculated.
		void SetStrokeMiterLimit(const GReal MiterLimit);
		//! Set srtoke pen width; in this implementation, additional internal constants are calculated.
		void SetStrokeWidth(const GReal Width);
		//! Get Miterlimit * Thickness.
		inline GReal StrokeMiterLimitMulThickness() const {
			return gMiterMulThickness;
		}
	};

	// *********************************************************************
	//                           GOpenGLCacheSlot
	// *********************************************************************
	struct GOpenGLCacheSlot {
		// display list for fill
		GLuint FillDisplayList;
		// display list for stroke
		GLuint StrokeDisplayList;
		// bounding box of the primitive, ALWAYS including stroke
		GAABox2 Box;
		// constructor
		GOpenGLCacheSlot() {
			FillDisplayList = 0;
			StrokeDisplayList = 0;
		}
	};

	// *********************************************************************
	//                           GOpenGLCacheBank
	// *********************************************************************
	/*!
		\class GOpenGLCacheBank
		\brief This is the OpenGL implementation of a cache bank class, intended as a set of contiguous cache slots (each slot
		is in unique correspondence to a single drawn primitive on the current target buffer).
		
		A cache bank, after being filled, can be reused to draw cached shapes. The bank can be invalidated using
		the function Invalidate(). This implementation uses OpenGL display lists, and stroke and fill parts are
		always cached independently of StrokeEnabled() and FillEnabled() style flags.
	*/
	class G_EXPORT GOpenGLCacheBank : public GCacheBank {

		friend class GOpenGLBoard;

	private:
		//! Cache slots.
		GDynArray< GOpenGLCacheSlot > gSlots;

	public:
		//! Constructor, it build an empty cache bank.
		GOpenGLCacheBank();
		//! Destructor, it invalidates cached shapes and frees memory.
		~GOpenGLCacheBank();
		//! Get the number of slots (cached shapes).
		GInt32 SlotsCount() const;
		//! Invalidate the cache, freeing associated (video) memory.
		void Invalidate();
	};

	// internal structure used to grab a portion of the framebuffer
	struct GLGrabbedRect {
		
		GUInt32 Width, Height;
		GUInt32 TexWidth, TexHeight;
		GLenum Target; // GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_EXT
		GLenum Format;
		GLuint TexName;
		GBool IsEmpty;
		GAABox2 gExpandedLogicBox;
		GAABox2 gNotExpandedLogicBox;

		// constructor
		GLGrabbedRect() {
			Width = Height = 0;
			TexWidth = TexHeight = 0;
			Target = GL_TEXTURE_2D;
			Format = GL_RGBA;
			TexName = 0;
			IsEmpty = G_TRUE;
		}
		inline GBool Valid() const {
			return(!IsEmpty);
		}
		inline void SetValid(const GBool Valid) {
			IsEmpty = (!Valid);
		}
	};

	typedef std::vector<GPoint2>::const_iterator Point2ConstIt;

	#define SELECT_AND_DISABLE_TUNIT(TexUnit) \
		SelectTextureUnit(TexUnit); \
		glDisable(GL_TEXTURE_1D); \
		glDisable(GL_TEXTURE_2D); \
		glDisable(GL_TEXTURE_RECTANGLE_EXT); \
		glDisable(GL_TEXTURE_GEN_S); \
		glDisable(GL_TEXTURE_GEN_T);

	// *********************************************************************
	//                             GOpenGLBoard
	// *********************************************************************
	/*!
		\class GOpenGLBoard
		\brief This is an OpenGL-based implementation drawboard.

		This class implements all GDrawBoard features upon OpenGL 1.1. For radial and conical gradients there
		are also fragment program implementations, so the faster pipeline is always used.\n
		Clip paths are supported only if stencil buffer is present (and in this case it must have 4 or more bits).
		For an N bits stencil buffer, the maximum number of clip paths that can be composed at the same time is
		2^(N - 1) - 3.
		Group opacity is also supported only if stencil buffer is present.\n
		Z-buffer is used to do correct transparencies, in this case depth bits are not important, because 16 or 24
		or 32 bits are always enough. All 24 compositing operations are supported on those boards equipped with fragment
		programs, else just only 15 of them will be available.
		Some compositing operations need framebuffer grab and also multi-pass techniques. But independently of it,
		the geometric pass is always just one.
		The compositing done on groups is implemented according to SVG 1.2 rendering model following the
		behavior of clip-to-self=false, enable-background=new, knock-out=true.
		In this implementation the whole caching system is based on OpenGL display lists.
	*/
	class G_EXPORT GOpenGLBoard : public GDrawBoard {
	private:
		//! OpenGL extension manager.
		GOpenglExt *gExtManager;
		//! List of all created gradients.
		GDynArray<GOpenGLGradientDesc *> gGradients;
		//! List of all created patterns.
		GDynArray<GOpenGLPatternDesc *> gPatterns;
		//! List of all created cache banks.
		GDynArray<GOpenGLCacheBank *> gCacheBanks;
		//! G_TRUE if clip paths are supported by the graphics device, else G_FALSE.
		GBool gClipMasksSupport;
		//! G_TRUE if fragment programs are supported by the graphics device, else G_FALSE.
		GBool gFragmentProgramsSupport;
		//! G_TRUE if fragment programs are supported and they are currently in use, else G_FALSE.
		GBool gFragmentProgramsInUse;
		//! G_TRUE if screen has an alpha channel, else G_FALSE.
		GBool gAlphaBufferPresent;
		//! G_TRUE if rectangular textures are supported by the graphics device (used for group opacity), else G_FALSE.
		GBool gRectTexturesSupport;
		//! G_TRUE if rectangular textures are supported and they are currently in use, else G_FALSE.
		GBool gRectTexturesInUse;
		//! G_TRUE if multitexture is supported, and the number of texture units is at least 2.
		GBool gMultiTextureSupport;
		//! G_TRUE if group opacity is supported by the graphics device, else G_FALSE.
		GBool gGroupOpacitySupport;
		//! G_TRUE if OpenGL context has been opened with multi samples (FSAA), else G_FALSE.
		GBool gMultiSamplePresent;
		//! G_TRUE if mirrored repeat for texture's UV is supported by the graphics device, else G_FALSE.
		GBool gMirroredRepeatSupport;
		//! Current maximum value on top of the stencil stack.
		GLint gTopStencilValue;
		//! Maximum permitted value on top of the stencil stack.
		GLint gMaxTopStencilValue;
		//! Bitwise stencil mask to pass to stencil functions.
		GLuint gStencilMask;
		//! Bitwise dual stencil mask to pass to stencil functions.
		GLuint gStencilDualMask;
		//! Current cache bank.
		GOpenGLCacheBank *gCacheBank;

		GBool gFirstClipMaskReplace;
		// List of all clip paths (boxes) present on the stack.
		GList<GAABox2> gClipMasksBoxes;
		//! Box of the group, calculated (expanded every drawing operation) just for clip-mode.
		GAABox2 gGroupBox;
		//! Is the current drawing operation the first inside the GroupBegin() / GroupEnd() block?
		GBool gIsFirstGroupDrawing;
		//! Current squared deviation, used by all adaptive geometric primitives.
		GReal gDeviation;
		//! Defined as Sqrt(gDeviation).
		GReal gFlateness;
		//! Last grabbed portion of the frame buffer, used to do group opacity.
		GLGrabbedRect gGLGroupRect;
		//! Last grabbed portion of the frame buffer, used to do compositing operation.
		GLGrabbedRect gCompositingBuffer;
		//! Fragment programs table to do compositing, when screen has alpha channel.
		GLuint gCompProgramsRGBA[24][5][2];
		//! Fragment programs table to do compositing, when screen hasn't alpha channel.
		GLuint gCompProgramsRGB[24][5][2];
		//! Fragment programs table to do compositing for groups, when screen has alpha channel.
		GLuint gCompGroupProgramsRGBA[24][2];
		//! Fragment programs table to do compositing for groups, when screen hasn't alpha channel.
		GLuint gCompGroupProgramsRGB[24][2];
		/*
		For single drawing operations, there are 5 possible paint type(color, lingrad, radgrad, congrad,
		pattern). For identity operations (clear, src, dst, srcover, dstover, srcin, dstin, srcout,
		dstout, srcatop, xor, plus, screenRGB, exclusionRGB) there is a single program for each paint type (so 5 * 1).
		For others operations (dstatop, multiply, screenRGBA, overlay, darken, lighten, colordodge, colorburn,
		hardlight, softlight, difference, exclusionRGBA) there is one program for each paint type for each
		texture target (2D or RECT), so 5 * 12 * 2.
		*/
		//! List of unique OpenGL programs id (generated by glGenProgramsARB).
		GLuint gUniqueProgramsID[5 * (1 + 12 * 2)];
		//! List of unique OpenGL groups programs id (generated by glGenProgramsARB).
		GLuint gUniqueGroupProgramsID[(1 + 12)][2];
		//! OpenGL atan2 lookup table.
		GFloat *gAtan2LookupTable;
		//! OpenGL atan2 lookup table size.
		GInt32 gAtan2LookupTableSize;
		//! Tesselator used to triangulate fill regions.
		GTesselator2D gTesselator;
		//! Current SVG-like path geometric cursor.
		GPoint2 gSVGPathCursor;
		//! Last control point, used by SVG-like smoothed primitives.
		GPoint2 gLastCurveCP;
		//! The first point of the current SVG-like subpath.
		GPoint2 gFirstPathPoint;
		GInt32 gOldPointsSize;
		//! G_TRUE if drawboard is inside an SVG-like BeginPaths() / EndPaths() block.
		GBool gInsideSVGPaths;
		GBool gInsideSVGPath;
		//! The list of all points generated by SVG-like path commands.
		GDynArray<GPoint2> gSVGPathPoints;
		GDynArray<GInt32> gSVGPathPointsPerContour;
		GDynArray<GBool> gSVGPathClosedStrokes;
		//! HTML character mask (for valid color characters).
		GChar8 gHTMLMask[32];

		//! Calculate (squared) deviation given a (squared) pixel deviation.
		GReal CalcDeviation(const GReal PixelDeviation);
		//! Calculate (squared) pixel deviation given a (squared) deviation.
		GReal CalcPixelDeviation(const GReal Deviation);
		//! Generate openGL atant2 lookup table.
		void GenerateAtan2LookupTable();
		
		static GLuint GLGenerateProgram(const GChar8 *ProgramString);
		GLuint GenerateProgram(const GUInt32 PaintType, const GChar8 *TexString, const GChar8 *OpString);
		GLuint GenerateGroupProgram(const GUInt32 TextureTarget, const GChar8 *TexString, const GChar8 *OpString);
		//! Generate fragment shaders table
		void GenerateShadersTable();
		//! Destroy shaders.
		void DestroyShadersTable();

		void DrawGrabbedRect(const GLGrabbedRect& GrabbedRect, const GBool TexUnit0, const GBool SubPixel0,
							 const GBool TexUnit1, const GBool SubPixel1);

		//! Grab a portion of the frame buffer.
		void GrabFrameBuffer(const GAABox2& LogicBox, GLGrabbedRect& Shot);

		void ReplaceFrameBuffer(const GLGrabbedRect& GrabbedRect, const GCompositingOperation CompOp,
								const GUInt32 PassIndex);
		// stencil management
		void StencilPush();
		void StencilPop();
		void StencilReplace();
		void StencilEnableTop();
		void DepthNoStencilWrite();
		void StencilNoDepthWrite();
		void StencilWhereDepthEqual();

		// initialize HTML valid color characters table
		void BuildHTMLMask();
		GBool IsValidHTMLColorChar(const GUChar8 Char);

	protected:
		//! Delete all user-generated gradients.
		void DeleteGradients();
		//! Delete all user-generated patterns.
		void DeletePatterns();
		//! Delete all user-generated cache banks.
		void DeleteCacheBanks();

		// OpenGL low level drawings
		static void SetGLColor(const GVectBase<GReal, 4>& Color);
		static void SetGLColor(const GVectBase<GReal, 3>& Color);
		static void SetGLModelViewMatrix(const GMatrix33& Matrix);
		static void SetGLTextureMatrix(const GMatrix33& Matrix);
		static void DrawGLBox(const GAABox2& Box);
		static void DrawGLBox(const GGenericAABox<GInt32, 2>& Box);
		GMatrix44 GLProjectionMatrix(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top,
									 const GReal DepthOnScreen);
		GMatrix44 GLWindowModeMatrix(const GReal DepthOnScreen);
		static void UpdateBox(const GAABox2& Source, const GMatrix33& Matrix, GAABox2& Result);

		//! Retrieve the number of passes to do the specified compositing operation, and returns G_TRUE if framebuffer must be grabbed.
		GBool CompOpPassesCount(const GCompositingOperation CompOp, GUInt32& PassesCount,
								GUInt32& FrameBufferPassesCount);
		//!	Set compositing operation OpenGL states for the specified pass, and returns G_TRUE if framebuffer must be grabbed.
		GBool SetGLStyleCompOp(const GCompositingOperation CompOp, const GUInt32 PassIndex);
		//! Return G_TRUE if a depth mask is required to draw Fill/Stroke according to specified Style.
		GBool NeedDepthMask(const GOpenGLDrawStyle& Style, const GBool Fill) const;

		//! Update the texture used to grab a portion of the framebuffer.
		void UpdateGrabBuffer(const GUInt32 Width, const GUInt32 Height, GLGrabbedRect& GrabRect);

		GBool SetGLClipEnabled(const GTargetMode Mode, const GClipOperation Operation);

		void DrawGLCircleSlice(const GPoint2& Center, const GReal RoundAuxCoef, const GPoint2& Start,
							 const GPoint2& End, const GReal SpanAngle, const GBool CCW);

		void DrawGLJoinLine(const GJoinStyle JoinStyle, const GReal MiterLimitMulThickness,
							const GPoint2& Previous, const GPoint2& P0, const GPoint2& P1,
							const GReal Thickness, const GReal RoundAuxCoef);

		void DrawGLJoinLineCap(const GJoinStyle JoinStyle, const GReal MiterLimitMulThickness,
								const GPoint2& Previous, const GPoint2& P0, const GPoint2& P1,
								const GReal Thickness, const GCapStyle EndCapStyle, const GReal RoundAuxCoef);

		void DrawGLJoin(const GPoint2& JoinCenter, const GVector2& InDirection, const GReal InDistance,
						const GVector2& OutDirection, const GReal OutDistance, const GJoinStyle JoinStyle,
						const GReal MiterLimitMulThickness, const GCapStyle StartCapStyle, const GCapStyle EndCapStyle,
						const GReal Thickness, const GReal RoundAuxCoef);

		void DrawGLCapsLine(const GBool DoStartCap, const GCapStyle StartCapStyle,
							const GBool DoEndCap, const GCapStyle EndCapStyle,
							const GPoint2& P0, const GPoint2& P1, const GReal Thickness, const GReal RoundAuxCoef);

		// draw solid (non-dashed) stroke
		void DrawSolidStroke(const GCapStyle StartCapStyle, const GCapStyle EndCapStyle,
							 const GJoinStyle JoinStyle, const GReal MiterLimitMulThickness,
							 Point2ConstIt PointsBegin, Point2ConstIt PointsEnd,
							 const GBool Closed, const GReal Thickness, const GReal RoundAuxCoeff);
		// draw dashed stroke
		void DrawDashedStroke(const GOpenGLDrawStyle& Style,
							  Point2ConstIt PointsBegin, Point2ConstIt PointsEnd,
							  const GBool Closed, const GReal Thickness, const GReal RoundAuxCoeff);

		// intersect a circle with two rays, returning two intersection points and spanned angle
		// in CCW direction to go from P0 to P1; precondition is that rays origins are inside the circle
		void IntersectRaysDisk(const GRay2& Ray0, const GRay2& Ray1, const GPoint2& Center, const GReal Radius,
							   GPoint2& P0, GPoint2& P1, GReal& SpannedAngle) const;
		// return the sign of the (signed) distance between a given circle and an axes-aligne box
		static GInt32 SignBoxDisk(const GAABox2& Box, const GPoint2& Center, const GReal Radius);
		// draw a radial-shaded disk sector
		void DrawGLRadialSector(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
								const GReal Time0, const GReal Time1,
								const GPoint2& P0, const GPoint2& P1, const GBool WholeDisk,
								const GDynArray<GKeyValue>& ColorKeys, const GColorRampInterpolation Interpolation,
								const GColorRampSpreadMode SpreadMode,
								const GReal MultAlpha) const;
		// draw a radial-shaded disk sector, specifying an axes-aligned bounding box that must entirely filled
		void DrawRadialSector(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
							  const GAABox2& BoundingBox,
							  const GDynArray<GKeyValue>& ColorKeys, const GColorRampInterpolation Interpolation,
							  const GColorRampSpreadMode SpreadMode,
							  const GReal MultAlpha, const GMatrix33& GradientMatrix,
							  const GMatrix33& InverseGradientMatrix) const;

		GPoint2 DrawGLConicalSlice(const GPoint2& P0, const GPoint2& Center, const GReal Radius,
								   const GVector4 Col0, const GVector4 Col1,
								   const GVector4 Tan0, const GVector4 Tan1,
								   const GReal SpanAngle, const GReal Flatness, const GColorRampInterpolation Interpolation) const;

		// draw a conical-shaded disk sector
		void DrawGLConicalSector(const GPoint2& Center, const GVector2& DirCenterTarget, const GReal Radius,
								const GPoint2& P0, const GPoint2& P1, const GBool WholeDisk,
								const GDynArray<GKeyValue>& ColorKeys,
								const GDynArray<GVector4>& InTangents, const GDynArray<GVector4>& OutTangents,
								const GColorRampInterpolation Interpolation, const GReal MultAlpha) const;
		// draw a conical-shaded disk sector, specifying an axes-aligned bounding box that must entirely filled
		void DrawConicalSector(const GPoint2& Center, const GPoint2& Target, const GAABox2& BoundingBox,
								const GDynArray<GKeyValue>& ColorKeys,
								const GDynArray<GVector4>& InTangents, const GDynArray<GVector4>& OutTangents,
								const GColorRampInterpolation Interpolation,
								const GReal MultAlpha, const GMatrix33& GradientMatrix,
								const GMatrix33& InverseGradientMatrix) const;

		void UpdateClipMasksState();
		void ClipReplaceOverflowFix();

		void PushDepthMask();
		void DrawAndPopDepthMask(const GAABox2& Box, const GDrawStyle& Style, const GBool DrawFill,
								 const GUInt32 StylePassesCount, const GUInt32 FrameBufferPassesCount,
								 const GBool ScreenGrabbed);
		void SelectTextureUnit(const GLint Unit);
		void SetTextureVertex(const GUInt32 TextureIndex, const GReal u, const GReal v);

		void UpdateStyle(GOpenGLDrawStyle& Style);
		void UseStyle(const GPaintType PaintType, const GCompositingOperation CompOp, const GUInt32 PassIndex,
					  const GVector4& Color,
					  const GOpenGLGradientDesc *Gradient, const GOpenGLPatternDesc *Pattern,
					  const GMatrix33& ModelView, const GMatrix33& InverseModelView);

		void UseGroupStyle(const GUInt32 PassIndex, const GLGrabbedRect& GroupColorRect,
						   const GLGrabbedRect& BackgroundRect);

		void UseStrokeStyle(const GDrawStyle& Style, const GUInt32 PassIndex);
		void UseFillStyle(const GDrawStyle& Style, const GUInt32 PassIndex);
		void UpdateDeviation(const GRenderingQuality Quality);

		void GLDisableShaders();
		/*
			Get if caching is enabled (G_TRUE value) or disabled (G_FALSE value).
		*/
		inline GBool CachingEnabled() const {
			if (TargetMode() != G_COLOR_MODE && TargetMode() != G_CLIP_MODE)
				return G_TRUE;
			return G_FALSE;
		}

		/*!
			Do the effective set of rendering quality settings.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoSetRenderingQuality(const GRenderingQuality Quality);
		/*!
			Do the effective set of target mode (target buffer).

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoSetTargetMode(const GTargetMode Mode);
		/*!
			Do the effective set clip operation to use for clip paths.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoSetClipOperation(const GClipOperation Operation);
		/*!
			Enable (G_TRUE value) or disable (G_FALSE value) clip paths.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoSetClipEnabled(const GBool Enabled);
		/*!
			Do the effective pop of the last clip path from stack.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoPopClipMask();
		/*!
			Do the effective set of group opacity.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoSetGroupOpacity(const GReal Opacity);
		/*!
			Do the effective set of group compositing operation.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoSetGroupCompOp(const GCompositingOperation CompOp);
		/*!
			Do the effective opening of a group block.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoGroupBegin(const GAABox2& LogicBox);
		/*!
			Do the effective closure of a group block.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoGroupEnd();
		/*!
			Do the effective non-blocking flush of all outstanding requests on the current context.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoFlush();
		/*!
			Do the effective blocking flush of all outstanding requests on the current context.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
		*/
		void DoFinish();
		/*!
			Do the effective drawboard cleaning.

			This methods permits to clear the entire drawboard, using s specified color. In the same 'atomic'
			operation the clipbuffer can be cleared too.
			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.

			\param Red the red component of the clear color.
			\param Green the green component of the clear color.
			\param Blue the blue component of the clear color.
			\param Alpha the alpha component of the clear color.
			\param ClearClipMasks if G_TRUE also clipbuffer will be cleared. In this case all existing masks will
			be dropped.
		*/
		void DoClear(const GReal Red, const GReal Green, const GReal Blue, const GReal Alpha, const GBool ClearClipMasks);
		/*!
			Do the effective set of physical viewport.

			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block.
			\param LowLeftCornerX lowleft corner abscissa of the viewport.
			\param LowLeftCornerY lowleft corner ordinate of the viewport.
			\param Width width of the viewport expressed in pixels, it's ensured to be grater than 0.
			\param Height height of the viewport expressed in pixels, it's ensured to be grater than 0.
		*/
		void DoSetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
						   const GUInt32 Width, const GUInt32 Height);
		/*!
			Do the effective set of the logical viewport.

			Logical viewport (that defines implicitly a projection matrix) is used to select a rectangular portion of
			infinitive canvas. This can be used to do all kind of panning and zooming operations.
			When this method is called, it's ensured that drawboard is not inside a GroupBegin() / GroupEnd() block, in
			addition Left is ensure to be different than Right and Bottom is ensured to be different from Top.
			\param Left the abscissa of the bottom-left corner of current logical window.
			\param Right the abscissa of the top-right corner of current logical window.
			\param Bottom the ordinate of the bottom-left corner of current logical window.
			\param Top the ordinate of the top-right corner of current logical window.
		*/
		void DoSetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top);

		// draw primitives
		GInt32 DrawGLPolygon(const GOpenGLDrawStyle& Style, const GBool ClosedFill, const GBool ClosedStroke,
							const GJoinStyle FlattenJoinStyle, const GDynArray<GPoint2>& Points, const GBool Convex);
		GInt32 DrawGLPolygons(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
							  const GDynArray<GBool>& ClosedStrokes, const GOpenGLDrawStyle& Style);

		/*!
			Do the effective drawing of a rectangle.

			When this method is called, it's ensured that corners are opposite and ordered, in addition stroke or
			fill are enabled.

			\param Style the drawstyle to use.
			\param MinCorner a corner of the rectangle.
			\param MaxCorner the opposite (to MinCorner) corner of the rectangle.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner);
		/*!
			Do the effective drawing of a round rectangle.

			When this method is called, it's ensured that corners are opposite and ordered, in addition stroke or
			fill are enabled.

			\param Style the drawstyle to use.
			\param MinCorner a corner of the rectangle.
			\param MaxCorner the opposite (to MinCorner) corner of the rectangle.
			\param ArcWidth the x-axis radius of the ellipse used to round off the corners of the rectangle.
			It's ensured to be positive.
			\param ArcHeight the y-axis radius of the ellipse used to round off the corners of the rectangle.
			It's ensured to be positive.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawRoundRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner,
									const GReal ArcWidth, const GReal ArcHeight);
		/*!
			Do the effective drawing of a line.

			When this method is called it's ensured that stroke is enabled.

			\param Style the drawstyle to use.
			\param P0 the line start point.
			\param P1 the line end point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawLine(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1);
		/*!
			Do the effective drawing of a quadratic Bezier curve, going form P0 to
			P2 and having P1 as central control point.

			When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param P0 the curve start point.
			\param P1 the curve control point.
			\param P2 the curve end point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2);
		/*!
			Do the effective drawing of a cubic Bezier curve, going form P0 to P4 and
			having P1 and P2 as control points.

			When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param P0 the curve start point.
			\param P1 the curve first control point (associated with P0).
			\param P2 the curve second control point (associated with P3).
			\param P3 the curve end point.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3);
		/*!
			Do the effective drawing of an ellipse arc.

			When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param Center the center of the ellipse.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It's ensured to be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It's ensured to be greater than 0.
			\param OffsetRotation an optional offset rotation (expressed in radians) relative to the x-axis.
			\param StartAngle an angle (in radians) that defines the start point of the arc.
			\param EndAngle an angle (in radians) that defines the end point of the arc.
			\param CCW if G_TRUE the ellipse arc has to go from start point to end point in counter clockwise
			direction, else G_FALSE (clockwise direction).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength,
								const GReal YSemiAxisLength, const GReal OffsetRotation,
								const GReal StartAngle, const GReal EndAngle, const GBool CCW);
		/*!
			Do the effective drawing of an ellipse arc.

			When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param P0 the start point of the arc.
			\param P1 the end point of the arc.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It's ensured to be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It's ensured to be greater than 0.
			\param OffsetRotation an optional offset rotation (expressed in radians) relative to the x-axis.
			\param LargeArc if G_TRUE the ellipse arc has to go from start point to end point spanning the
			longest chord, else G_FALSE.
			\param CCW if G_TRUE the ellipse arc has to go from start point to end point in counter clockwise
			direction, else G_FALSE (clockwise direction).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
								const GReal OffsetRotation, const GBool LargeArc, const GBool CCW);
		/*!
			Do the effective drawing of a whole ellipse.

			When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param Center the center of the ellipse.
			\param XSemiAxisLength the radius of the ellipse along x-axis. It's ensured to be greater than 0.
			\param YSemiAxisLength the radius of the ellipse along y-axis. It's ensured to be greater than 0.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawEllipse(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength);
		/*!
			Do the effective drawing of a circle.

			When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param Center the center of the circle.
			\param Radius the radius of the circle, it's ensured to be positive.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawCircle(GDrawStyle& Style, const GPoint2& Center, const GReal Radius);
		/*!
			Draw the effective drawing of a polygon.

			When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param Points the set of consecutive points that define the polygon, it's ensured that at least 2 points
			entries exist.
			\param Closed if G_TRUE the polygon is considered closed (an imaginary edge goes from last point to
			the first one).
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed);
		/*!
			Do the effective drawing of curve/path.

			When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param Curve the curve to draw, it consists of at least 2 points.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawPath(GDrawStyle& Style, const GCurve2D& Curve);
		/*!
			Do the effective drawing of a set of curves and paths.

			This method permits to draw complex shapes, made of more than 1 curve/path, using the currently set
			fill rule. When this method is called it's ensured that stroke or fill are enabled.

			\param Style the drawstyle to use.
			\param Curves an array of curves pointers. Each pointer must be valid.
			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 DoDrawPaths(GDrawStyle& Style, const GDynArray<GCurve2D *>& Curves);
		/*!
			Draw a single cache slot, using specified style.

			\param Style the drawstyle to use.
			\param CacheSlot the cache slot to draw.
		*/
		void DoDrawCacheSlot(const GDrawStyle& Style, const GOpenGLCacheSlot& CacheSlot);
		/*!
			Draw the current cache bank slots. Here it's ensured that current cache bank is non-NULL
			and that that FirstSlotIndex <= LastSlotIndex.

			\param Style the drawstyle to use.
			\param FirstSlotIndex first cache slot (index) to draw. It's ensured to be inside the valid range.
			\param LastSlotIndex last cache slot (index) to draw. It's ensured to be inside the valid range.
		*/
		void DoDrawCacheSlots(GDrawStyle& Style, const GInt32 FirstSlotIndex, const GInt32 LastSlotIndex);
		/*!
			Do the effective screenshot.

			This function permits to grab a rectangular portion of the framebuffer, and put it into
			a bitmap. Here it's ensured that P0 <= P1.

			\param Output the output image, where grabbed portion will be copied into.
			\param P0 a corner of the rectangle portion to grab.
			\param P1 the opposite (to P0) corner of the rectangle.
			\return G_NO_ERROR if the operation succeeds, else an error code.
		*/
		GError DoScreenShot(GPixelMap& Output, const GVectBase<GUInt32, 2>& P0, const GVectBase<GUInt32, 2>& P1) const;
		/*!
			Create a new OpenGL draw style.
		*/
		GDrawStyle *CreateDrawStyle() const;

	public:
		/*!
			Constructor, setting the physical viewport according to specified parameters.

			\param LowLeftCornerX lowleft corner abscissa of the initial physical viewport.
			\param LowLeftCornerY lowleft corner ordinate of the initial physical viewport.
			\param Width width of the viewport expressed in pixels.
			\param Height height of the viewport expressed in pixels.
		*/
		GOpenGLBoard(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY, const GUInt32 Width, const GUInt32 Height);
		//! Destructor, it deletes all user-generated gradients and patterns.
		virtual ~GOpenGLBoard();

		/*!
			Get the maximum supported number of dash entries.
		*/
		GUInt32 MaxDashCount() const;
		/*!
			Get the maximum supported number of color keys.
		*/
		GUInt32 MaxColorKeys() const;
		/*!
			Get the maximum supported pattern width (in pixels).
		*/
		GUInt32 MaxImageWidth() const;
		/*!
			Get the maximum supported pattern height (in pixels).
		*/
		GUInt32 MaxImageHeight() const;
		/*!
			Get the maximum supported pattern memory size (in bytes).
		*/
		GUInt32 MaxImageBytes() const;
		/*!
			Enable or disable fragment programs for radial and conical gradients.

			As default, OpenGL drawboard always chooses the best performance drawing pipeline, according to
			the hardware features supported by the underlying graphic device.
			In some situations it could be useful to disable fragment programs versions of some rendering
			pipelines (radial and conical gradients). Using this function the user can force the OpenGL drawboard
			to use always the geometric-based pipelines.
		*/
		void SetShadersEnabled(const GBool Enabled);
		/*!
			Enable or disable the use of rectangular textures to do group opacity.

			As default, OpenGL drawboard disables the use of rectangular texture. This is due to the fact that on
			some graphics board (especially ATI ones) there could be memory leaks inside the driver, making the
			application crash. Enabling rectangular texture could speed up a little the use of group opacity and
			it uses less memory.
		*/
		void SetRectTextureEnabled(const GBool Enabled);
		/*!
			Set a rendering quality, specifying deviation used to flatten curves.
			The more the Deviation parameter is, the worst is the rendering quality.

			\param Deviation the custom rendering quality level.
			\note Deviation is the maximum squared chordal distance threshold, and it must be positive.
		*/
		inline void SetCustomRenderingQuality(const GReal Deviation) {
			if (Deviation > 0) {
				gDeviation = Deviation;
				gFlateness = GMath::Sqrt(gDeviation);
			}
			else {
				G_DEBUG("SetCustomRenderingQuality: Deviation parameter is negative");
			}
		}
		/*!
			Create a linear gradient.

			\param StartPoint the point corresponding to the first color key.
			\param EndPoint the point corresponding to the last color key.
			\param ColorKeys color keys, they must be sorted by time (in ascending order).
			\param Interpolation the interpolation schema to adopt during pixel color evaluation.
			\param SpreadMode indicates what happens if the gradient starts or ends inside the bounds of the target
			region.
			\param Matrix an optional gradient matrix, used to transform the gradient into another coordinates system.
			\return a structure containing the specified gradient, NULL if the gradient can't be created.
			\note the user <b>MUST NOT</b> delete the returned pointer (the drawboard will care of it).
		*/
		GGradientDesc *CreateLinearGradient(const GPoint2& StartPoint, const GPoint2& EndPoint,
											const GDynArray<GKeyValue>& ColorKeys,
											const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
											const GColorRampSpreadMode SpreadMode = G_PAD_COLOR_RAMP_SPREAD,
											const GMatrix33& Matrix = G_MATRIX_IDENTITY33);
		/*!
			Create a radial gradient.

			\param Center the center of gradient, it will be in correspondence of the first color key.
			\param Focus the focus point.
			\param Radius the radius of the gradient. Points at radius distances from center will be colored using
			last color key.
			\param ColorKeys color keys, they must be sorted by time (in ascending order).
			\param Interpolation the interpolation schema to adopt during pixel color evaluation.
			\param SpreadMode indicates what happens if the gradient starts or ends inside the bounds of the target
			region.
			\param Matrix an optional gradient matrix, used to transform the gradient into another coordinates system.
			\return a structure containing the specified gradient, NULL if the gradient can't be created.
			\note the user <b>MUST NOT</b> delete the returned pointer (the drawboard will care of it).
		*/
		GGradientDesc *CreateRadialGradient(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
											const GDynArray<GKeyValue>& ColorKeys,
											const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
											const GColorRampSpreadMode SpreadMode = G_PAD_COLOR_RAMP_SPREAD,
											const GMatrix33& Matrix = G_MATRIX_IDENTITY33);
		/*!
			Create a conical gradient.

			\param Center the center of gradient, this is the cone vertex point.
			\param Target it specifies a 'target' point, that coupled with start point defines a direction
			associated to the first color key.
			\param ColorKeys color keys, they must be sorted by time (in ascending order).
			\param Interpolation the interpolation schema to adopt during pixel color evaluation.
			\param Matrix an optional gradient matrix, used to transform the gradient into another coordinates system.
			\return a structure containing the specified gradient, NULL if the gradient can't be created.
			\note the user <b>MUST NOT</b> delete the returned pointer (the drawboard will care of it).
		*/
		GGradientDesc *CreateConicalGradient(const GPoint2& Center, const GPoint2& Target,
											 const GDynArray<GKeyValue>& ColorKeys,
											 const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
											 const GMatrix33& Matrix = G_MATRIX_IDENTITY33);
		/*!
			Create a bitmap pattern.

			\param Image a source bitmap image, must be valid.
			\param Quality a quality parameter that must be used when the pattern will be drawn on screen.
			\param TilingMode defines possible methods for defining colors for source pixels that lie outside the bounds
			of the source image.
			\param LogicalWindow defines a reference rectangle somewhere on the	infinite canvas. The tiling
			extends a series of such rectangles to infinity in X and Y (positive and negative). If NULL is specified
			a default rectangle (going from the lowleft logical corner with a correct ratio) will be set.
			\param Matrix an optional pattern matrix, used to transform the pattern into another coordinates system.
			\return a structure containing the specified pattern, NULL if the pattern can't be created.
			\note the user <b>MUST NOT</b> delete the returned pointer (the drawboard will care of it).
		*/
		GPatternDesc *CreatePattern(const GPixelMap *Image, const GImageQuality Quality,
									const GTilingMode TilingMode = G_REPEAT_TILE,
									const GAABox2 *LogicalWindow = NULL,
									const GMatrix33& Matrix = G_MATRIX_IDENTITY33);
		/*!
			Create a cache bank.

			\return the created cache bank if operation succeeds, else a NULL pointer.
			\note the user <b>MUST NOT</b> delete the returned pointer (the drawboard will care of it).
		*/
		GCacheBank *CreateCacheBank();
		/*!
			Get current cache bank. NULL if a cache bank hasn't already been set.
		*/
		GCacheBank *CacheBank() const;
		/*!
			Set current cache bank, a NULL value is valid.
		*/
		void SetCacheBank(GCacheBank *Bank);
		/*!
			Convert a color from a string format to its numerical representation (where each component is in
			the range [0; 1]. Implementation supports color in these forms:\n\n

			- HTML syntax: \#F0A10063 (RGBA), \#A10063 (RGB), \#8F36 (RGBA), \#8F3 (RGB)
			- SVG color keywords taken from http://www.w3.org/TR/SVG11/types.html#ColorKeywords (ex: "antiquewhite")
		*/
		GVector4 ColorFromString(const GString& Color);
		/*!
			Start an SVG-like path block.
			Each opened block must be closed calling EndPaths() function.
		*/
		void BeginPaths();
		/*!
			Begin an SVG-like sub-path.

			This function is in unique correspondence with SVG path command 'M'/'m'.

			\param P the start point.
			\param Relative if G_TRUE start point coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void MoveTo(const GPoint2& P, const GBool Relative);
		/*!
			Draw a line from the current cursor position to the specified point.

			\param P the endpoint of the line.
			\param Relative if G_TRUE end point coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void LineTo(const GPoint2& P, const GBool Relative);
		/*!
			Draw an horizontal line from the current cursor position to the specified location.

			\param X the endpoint abscissa.
			\param Relative if G_TRUE end point coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void HorizontalLineTo(const GReal X, const GBool Relative);
		/*!
			Draw a vertical line from the current cursor position to the specified location.

			\param Y the endpoint ordinate.
			\param Relative if G_TRUE end point coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void VerticalLineTo(const GReal Y, const GBool Relative);
		/*!
			Draw a cubic Bezier curve from current cursor position to P, using P1 as the control point at the
			beginning of the curve and P2 as the control point at the end of the curve.

			\param P1 control point at the beginning of the curve.
			\param P2 control point at the end of the curve.
			\param P curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void CurveTo(const GPoint2& P1, const GPoint2& P2, const GPoint2& P, const GBool Relative);
		/*!
			Draw a quadratic Bezier curve from current cursor position to P, using P1 as control point.

			\param P1 control point.
			\param P curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void CurveTo(const GPoint2& P1, const GPoint2& P, const GBool Relative);
		/*!
			Draw a cubic Bezier curve using smooth tangent.

			Draws a cubic Bezier curve from	the current cursor position to P. The first control point is assumed to
			be the reflection of the second control point on the previous command relative to the current point.
			At the end of the command, the new cursor position becomes P.

			\param P2 the second control point.
			\param P the final curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void SmoothCurveTo(const GPoint2& P2, const GPoint2& P, const GBool Relative);
		/*!
			Draw a quadratic Bezier curve using smooth tangent.

			Draws a quadratic Bezier curve from	the current cursor position to P. The control point is assumed to
			be the reflection of the control point on the previous command relative to the current point.
			At the end of the command, the new cursor position becomes P.

			\param P the final curve end point.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void SmoothCurveTo(const GPoint2& P, const GBool Relative);
		/*!
			Draw an elliptical arc from the current cursor position to P. The size and orientation of
			the ellipse are defined by two radii (Rx, Ry) and an x-axis-rotation (XRot, expressed in radians), which
			indicates how the ellipse as a whole is rotated relative to the current coordinate system.
			The center of the ellipse is calculated automatically to satisfy the constraints imposed by the other
			parameters. LargeArc flag and Sweep flag contribute to the automatic calculations and help determine
			how the	arc is drawn.

			\param Rx x-axis radius.
			\param Ry y-axis radius.
			\param XRot an offset rotation (in radians) which indicates	how the ellipse as a whole is rotated
			relative to the current coordinate system.
			\param LargeArc if G_TRUE then one of the two larger arc sweeps will be chosen; otherwise, if G_FALSE
			one of the smaller arc sweeps will be chosen.
			\param Sweep if G_TRUE then the arc will be drawn in a "positive-angle" direction. A value of G_FALSE
			causes the arc to be drawn in a	"negative-angle" direction.
			\param P the final endpoint to reach.
			\param Relative if G_TRUE points coordinates are relative to the current cursor position, else they
			are absolute.
		*/
		void EllipticalArcTo(const GReal Rx, const GReal Ry, const GReal XRot, const GBool LargeArc,
							 const GBool Sweep, const GPoint2& P, const GBool Relative);
		/*!
			Close the current SVG-like sub-path.
		*/
		void ClosePath();
		/*!
			Close an SVG-like path block.

			\return if caching is enabled (G_CACHE_MODE, G_CLIP_AND_CACHE_MODE and G_COLOR_AND_CACHE_MODE target modes)
			and a valid cache bank is currently set, it returns the slot index (where the primitive has been inserted)
			in the active cache bank, else an error code.
		*/
		GInt32 EndPaths();

		#ifdef _DEBUG
			void DumpStencilBuffer(const GChar8 *FileName);
			void DumpZBuffer(const GChar8 *FileName);
		#endif
	};

};	// end namespace Amanith

#endif
