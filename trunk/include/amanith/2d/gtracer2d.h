/****************************************************************************
** $file: amanith/2d/gtracer2d.h   0.3.0.0   edited Jan, 30 2006
**
** 2D Bitmap tracer (vectorizer) definition.
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

#ifndef GTRACER2D_H
#define GTRACER2D_H

#include "amanith/2d/gpath2d.h"
#include "amanith/2d/gpixelmap.h"

/*!
	\file gtracer2d.h
	\brief 2D bitmap vectorizer header file.
*/
namespace Amanith {

	// *********************************************************************
	//                          GTracedContour
	// *********************************************************************
	class GTracer2D;

	struct ThreePoints {
		GPoint2 Points[3];
	};

	/*!
		\class GTracedContour
		\brief This class represents a closed contour built by tracer.

		Internally a contour is compactly described by its "main" points and a sequence of flags, associated with main
		points, that clarify each point "degree". A such contour can be flattened using the well know maximum
		deviation schema.\n.
	*/
	class G_EXPORT GTracedContour {

		friend class GTracer2D;

	private:
		//! Main points array
		GDynArray<GInt32> gPointFlags;
		//! Points flags.
		GDynArray<ThreePoints> gPoints;

	public:
		//! Set constructor.
		GTracedContour(const GDynArray<GInt32>& PointFlags, const GDynArray<ThreePoints>& Points)
		: gPointFlags(PointFlags), gPoints(Points) {
		}
		//! Destructor
		~GTracedContour() {
		}
		/*!
			Draw (flatten) the contour, specifying a maximum deviation (squared chordal distance)

			\param Points the output array, where generated points will be appended.
			\param Variation maximum variation permitted by this flattening operation.
		*/
		void DrawContour(GDynArray<GPoint2>& Points, const GReal Variation) const;
		/*!
			Convert the contour into a GPath2D representation.

			\param Path the output paths.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError ConvertToPath(GPath2D& Path) const;
	};

	//! Turn policy used during contours extraction.
	enum GTurnPolicy {
		//! Prefers to connect black components
		G_CONNECT_BLACK,
		//! Prefers to connect white components
		G_CONNECT_WHITE,
		//! Prefers to connect the color which occurs most frequently.
		G_CONNECT_DOMINANT,
		/*!
			Prefers to connect the color (black or white) which occurs least frequently within a given neighborhood
			of the current position.
		*/
		G_CONNECT_NOT_DOMINANT
	};

	// *********************************************************************
	//                            GTracer2D
	// *********************************************************************

	/*!
		\class GTracer2D
		\brief This class implements a bitmap vectorizer.

		The implemented algorithm follows the Peter Selinger Potrace schema. For more informations about algorithm
		can be found here: http://potrace.sourceforge.net/potrace.pdf \n
		We have introduced some little improvements, for example the routine that pick the first pixel where to begin
		path vectorization, some useful thresholds (mask radius to resolve cross roads for G_CONNECT_DOMINANT and
		G_CONNECT_NOT_DOMINANT turn policies, minimum area of path to vectorize, and so on).
	*/
	class G_EXPORT GTracer2D {

	private:

		struct PixelPath {
			GDynArray< GPoint<GInt32, 2> > Points;
			GInt32 Length;
			GInt32 Area;
			GInt32 Direction;
		};

		struct PrivateCurve {
			GDynArray<GInt32> Tags;
			GDynArray<ThreePoints> CurvePoints;
			GDynArray<GPoint2> Vertexes;
		};

		static GError BuildPath(const GPixelMap& Image, const GPoint<GInt32, 2>& StartPoint,
								const GUChar8 WhiteColor, const GInt32 Direction, const GTurnPolicy TurnPolicy,
								const GInt32 MaxRadius,	PixelPath& Path);

		static GError ShiftPath(PixelPath& Path);
		static GInt32 SecondSeg(const PixelPath& Path, const GInt32 StartPoint,
								const GInt32 Direction);
		static GError XorUpdate(const PixelPath& Path, GPixelMap& DestImage, const GUChar8 WhiteColor);
		static GError StatisticalSums(const PixelPath& Path, GDynArray< GPoint<GReal, 5> >& SumsTable,
									  GInt32& X0, GInt32& Y0);
		static GError FurthestLinSubPaths(const PixelPath& Path, GDynArray<GInt32>& SubPathsTable);
		static GError BestPolygon(const PixelPath& Path, const GDynArray<GInt32>& lon,
								  const GDynArray< GPoint<GReal, 5> >& SumsTable,
								  GDynArray<GInt32>& Polygon);
		static GReal Penalty(const PixelPath& Path, const GDynArray< GPoint<GReal, 5> >& SumsTable,
							GInt32 i, GInt32 j);
		static GError AdjustVertices(const PixelPath& Path, const GDynArray< GPoint<GReal, 5> >& SumsTable,
									 const GDynArray<GInt32>& Polygon,
									 const GInt32 X0, const GInt32 Y0, PrivateCurve& Curve);
		static void BestFitSlope(const PixelPath& Path, const GDynArray< GPoint<GReal, 5> >& SumsTable,
								GInt32 i, GInt32 j, GPoint2 *ctr, GVector2 *dir);
		static GError SmoothCurve(PrivateCurve *curve, GInt32 sign, GReal alphamax, const GReal Scale,
								const GReal ImageHeight);
		static GError VectorizePath(PixelPath& Path, PrivateCurve& Curve,
									const GInt32 Sign, const GReal AlphaMax, const GReal Scale,
									const GReal ImageHeight);

	protected:
		static GBool FindBlackPixel(const GPixelMap& Image, const GUChar8 WhiteColor, const GInt32 StartY,
									 GPoint<GInt32, 2>& PixelCoords);
		static GBool BlackDominance(const GPixelMap& Image, const GPoint<GInt32, 2>& Center,
									const GUChar8 WhiteColor, const GInt32 MaxRadius);

	public:
		/*!
			Convert a bitmap into vector format.

			\param Image the image that will be converted into vector format. It must be an 8-bpp image (gray scale
			or paletted), else an G_INVALID_FORMAT error code will be returned.
			\param Paths the output array, where all traced closed paths will be returned.
			\param WhiteColor the color (index) used as white color. Every pixel with a value not equal to this value
			is considered as a 'black' pixel.
			\param TurnPolicy policy to take in case of cross roads pixels.
			\param MaxRadius radius of pixels mask that is used to resolve black/white dominance in case of
			G_CONNECT_NOT_DOMINANT or G_CONNECT_DOMINANT turn policies.
			\param MinArea the minimum area of closed contours. Every closed contours found into image will be vectorized
			if and only if its area is grater that the specified value. This parameter can be useful to avoid vectorization
			of isolated pixels or isolated small pixel areas. This area is expressed in pixels.
			\param Alpha corner detection threshold, smaller values lead to more corners and larger values lead
			to more rounded shapes. The default values is a good compromise.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		static GError Trace(const GPixelMap& Image, GDynArray<GTracedContour>& Paths,
							const GUChar8 WhiteColor = 255,
							const GTurnPolicy TurnPolicy = G_CONNECT_NOT_DOMINANT,
							const GInt32 MaxRadius = 6,
							const GInt32 MinArea = 10, const GReal Alpha = 0.55);
	};

};	// end namespace Amanith

#endif
