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
#include "amanith/2d/gtesselator2d.h"
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
		GDynArray<GVector4> gInTangents;
		GDynArray<GVector4> gOutTangents;

		void GenerateTexture1D(const GInt32 Size, GPixelMap& Bitmap);

	protected:
		static void SetGLGradientQuality(const GRenderingQuality Quality);
		void UpdateOpenGLTextureLinRad(const GRenderingQuality Quality, const GUInt32 MaxTextureSize);
		void UpdateOpenGLTextureCon(const GRenderingQuality Quality, const GUInt32 MaxTextureSize,
									const GInt32 Atan2LookupTableSize, const GFloat *gAtan2LookupTable);
		void UpdateHermiteTangents();

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

	// *********************************************************************
	//                          GOpenGLDrawStyle
	// *********************************************************************
	class G_EXPORT GOpenGLDrawStyle : public GDrawStyle {

		friend class GOpenGLBoard;

	private:
		GReal gMiterMulThickness; // defined as Miter * Thickness
		GReal gRoundJoinAuxCoef;  // defined as 1 / (2 * acos(1 - deviation / thickness))

	public:
		GOpenGLDrawStyle();
		void SetStrokeMiterLimit(const GReal MiterLimit);
		void SetStrokeWidth(const GReal Width);
		// get Miterlimit * Thickness
		inline GReal StrokeMiterLimitMulThickness() const {
			return gMiterMulThickness;
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
	};

	typedef std::vector<GPoint2>::const_iterator Point2ConstIt;

	// *********************************************************************
	//                             GOpenGLBoard
	// *********************************************************************
	class G_EXPORT GOpenGLBoard : public GDrawBoard {
	private:
		GOpenglExt *gExtManager;
		GDynArray<GOpenGLGradientDesc *> gGradients;
		GDynArray<GOpenGLPatternDesc *> gPatterns;
		GBool gClipByStencil;
		GBool gFragmentProgramsSupport;
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

		// radial gradient fragment program
		GLuint gRadGradGLProgram;
		// conical gradient fragment program
		GLuint gConGradGLProgram;
		// atan2 lookup table
		GInt32 gAtan2LookupTableSize;
		GFloat *gAtan2LookupTable;
		// tesselator used to triangulate fill region
		GTesselator2D gTesselator;
		// SVG-like path data structures
		GPoint2 gSVGPathCursor;
		GPoint2 gLastCurveCP;
		GPoint2 gFirstPathPoint;
		GInt32 gOldPointsSize;
		GBool gInsideSVGPaths;
		GBool gInsideSVGPath;
		GDynArray<GPoint2> gSVGPathPoints;
		GDynArray<GInt32> gSVGPathPointsPerContour;
		GDynArray<GBool> gSVGPathClosedStrokes;

	private:
		// calculate (squared) deviation given a (squared) pixel deviation
		GReal CalcDeviation(const GReal PixelDeviation);
		// calculate (squared) pixel deviation given a (squared) deviation
		GReal CalcPixelDeviation(const GReal Deviation);
		// generate atant2 lookup table
		void GenerateAtan2LookupTable();

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
							  const GReal MultAlpha, const GMatrix33& GradientMatrix) const;

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
								const GReal MultAlpha, const GMatrix33& GradientMatrix) const;

		void UpdateClipMasksState();
		void ClipReplaceOverflowFix();

		void PushDepthMask();
		void DrawAndPopDepthMask(const GAABox2& Box, const GDrawStyle& Style, const GBool DrawFill);

		void UpdateStyle(GOpenGLDrawStyle& Style);
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
		void DrawGLPolygon(const GOpenGLDrawStyle& Style, const GBool ClosedFill, const GBool ClosedStroke,
							const GJoinStyle FlattenJoinStyle, const GDynArray<GPoint2>& Points, const GBool Convex);

		void DrawGLPolygons(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
							const GDynArray<GBool>& ClosedStrokes, const GOpenGLDrawStyle& Style);

		// here we are sure that corners are opposite and ordered
		void DoDrawRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner);
		// here we are sure that corners are opposite and ordered and arc dimensions are > 0
		void DoDrawRoundRectangle(GDrawStyle& Style, const GPoint2& MinCorner, const GPoint2& MaxCorner,
								  const GReal ArcWidth, const GReal ArcHeight);
		void DoDrawLine(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1);
		void DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2);
		void DoDrawBezier(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3);
		void DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength,
							  const GReal YSemiAxisLength, const GReal OffsetRotation,
							  const GReal StartAngle, const GReal EndAngle, const GBool CCW);
		void DoDrawEllipseArc(GDrawStyle& Style, const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
							  const GReal OffsetRotation, const GBool LargeArc, const GBool CCW);
		// here we are sure that semi-axes lengths are greater than 0
		void DoDrawEllipse(GDrawStyle& Style, const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength);
		// here we are sure that Radius is greater than 0
		void DoDrawCircle(GDrawStyle& Style, const GPoint2& Center, const GReal Radius);
		// here we are sure that Points has at least 2 entries
		void DoDrawPolygon(GDrawStyle& Style, const GDynArray<GPoint2>& Points, const GBool Closed);
		// here we are sure that Curve has a number of points greater than 1
		void DoDrawPath(GDrawStyle& Style, const GCurve2D& Curve);
		// here we are sure that we have at least one curve
		void DoDrawPaths(GDrawStyle& Style, const GDynArray<GCurve2D *>& Curves);

		// create a draw style
		GDrawStyle *CreateDrawStyle() const;

	public:
		void DumpBuffers(const GChar8 *fNameZ, const GChar8 *fNameS);
		GOpenGLBoard(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY, const GUInt32 Width, const GUInt32 Height);
		virtual ~GOpenGLBoard();

		// read only parameters
		GUInt32 MaxDashCount() const;
		GUInt32 MaxKernelSize() const;
		GUInt32 MaxSeparableKernelSize() const;
		GUInt32 MaxColorKeys() const;
		GUInt32 MaxImageWidth() const;
		GUInt32 MaxImageHeight() const;
		GUInt32 MaxImageBytes() const;

		void DisableShaders(const GBool Disable);

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
		GGradientDesc *CreateConicalGradient(const GPoint2& Center, const GPoint2& Target,
											 const GDynArray<GKeyValue>& ColorKeys,
											 const GColorRampInterpolation Interpolation = G_HERMITE_COLOR_INTERPOLATION,
											 const GMatrix33& Matrix = G_MATRIX_IDENTITY33);
		GPatternDesc *CreatePattern(const GPixelMap *Image, const GImageQuality Quality,
									const GTilingMode TilingMode = G_REPEAT_TILE,
									const GAABox2 *LogicalWindow = NULL,
									const GMatrix33& Matrix = G_MATRIX_IDENTITY33);

		// SVG-like path commands
		void BeginPaths();
		// begin a sub-path
		void MoveTo(const GPoint2& P, const GBool Relative);
		// draw a line
		void LineTo(const GPoint2& P, const GBool Relative);
		// draw an horizontal line
		void HorizontalLineTo(const GReal X, const GBool Relative);
		// draw a vertical line
		void VerticalLineTo(const GReal Y, const GBool Relative);
		// draw a cubic Bezier curve
		void CurveTo(const GPoint2& P1, const GPoint2& P2, const GPoint2& P, const GBool Relative);
		// draw a quadratic Bezier curve
		void CurveTo(const GPoint2& P1, const GPoint2& P, const GBool Relative);
		// draw a cubic Bezier curve using smooth tangent
		void SmoothCurveTo(const GPoint2& P2, const GPoint2& P, const GBool Relative);
		// draw a quadratic Bezier curve using smooth tangent
		void SmoothCurveTo(const GPoint2& P, const GBool Relative);
		// draw an elliptical arc
		void EllipticalArcTo(const GReal Rx, const GReal Ry, const GReal XRot, const GBool LargeArc,
							 const GBool Sweep, const GPoint2& P, const GBool Relative);
		// close current sub-path
		void ClosePath();
		void EndPaths();

	};

};	// end namespace Amanith

#endif
