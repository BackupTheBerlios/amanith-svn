/****************************************************************************
** $file: amanith/geometry/gaabox.h   0.3.0.0   edited Jan, 30 2006
**
** Axes aligned bounding box definition.
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

#ifndef GAABOX_H
#define GAABOX_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gvect.h"
#include "amanith/gmath.h"

/*!
	\file gaabox.h
	\brief Header file for axially aligned box classes.
*/
namespace Amanith {

	/*!
		\class GGenericAABox
		\brief An axially aligned box in N-dimensional space.
		
		This is usually used for graphics applications. This type of box is defined by its minimum and maximum points.
    
		\param DATA_TYPE the internal type used for the points
		\param SIZE dimension of points (ex: 2 for a 2D box, 3 for a 3D box). It must be a positive number.
    */
	template<typename DATA_TYPE, GUInt32 SIZE>
	class GGenericAABox {

	private:
		//! The minimum point of the box.
		GPoint<DATA_TYPE, SIZE> gMin;
		//! The maximum point on the box.
		GPoint<DATA_TYPE, SIZE> gMax;
		/*!
			Make sure that order between minimum and maximum points is respected. Foe every components that does not
			respect order, a swap is done.
		*/
		inline void FixCornersOrder() {

			for (GUInt32 i = 0; i < SIZE; ++i) {
				if (gMin[i] > gMax[i]) {
					DATA_TYPE swap = gMin[i];
					gMin[i] = gMax[i];
					gMax[i] = swap;
				}
			}
		}

	public:
		//! Constructor, creates a new empty box. Min and Max points are set to zero.
		GGenericAABox()	{
		}

		/*!
			Creates a new box with the given min and max points.

			\param MinPoint the minimum point on the box
			\param MaxPoint the maximum point on the box
			\note For every elements of MinPoint that is not less than the MaxPoint's respective, a swap is done. Example:
\code
	GGenericAABox<GReal, 2> box(GPoint2(0, 0), GPoint2(3, -1));
	// not box has a minimum point of coordinates (0, -1) and a maximum point of coordinates (3, 0)
\endcode
		*/
		GGenericAABox(const GPoint<DATA_TYPE, SIZE>& MinPoint, const GPoint<DATA_TYPE, SIZE>& MaxPoint)
		: gMin(MinPoint), gMax(MaxPoint) {

			FixCornersOrder();
		}

		/*!
			Copy constructor, constructs a duplicate of the given box.

			\param SourceBox the box the make a copy of
		*/
		GGenericAABox(const GGenericAABox<DATA_TYPE, SIZE>& SourceBox) : gMin(SourceBox.gMin), gMax(SourceBox.gMax) {
		}

		/*!
			Constructs a new box from an array of points. Min and Max points are set according to the minimum and
			maximum points found into specified points array.

			\param Points the array of points
		*/
		GGenericAABox(const GDynArray< GPoint<DATA_TYPE, SIZE> >& Points) {

			SetMinMax(Points);
		}

		//!	Gets the minimum point of the box.
		inline const GVectBase<DATA_TYPE, SIZE>& Min() const {
			return gMin;
		}

		//! Gets the maximum point of the box.
		inline const GVectBase<DATA_TYPE, SIZE>& Max() const {
			return gMax;
		}

		//! Get the center of the box. It's calculated as (Min + Max) / 2
		const GPoint<DATA_TYPE, SIZE> Center() const {

			return GPoint<DATA_TYPE, SIZE>((gMin + gMax) / (DATA_TYPE)2);
		}
		/*!
			Gets the volume of the box. It's calculated as the product of every dimension edge length.

			\note For a 2D box, this value is equal to the area.
		*/
		DATA_TYPE Volume() const {

			G_ASSERT(SIZE > 0);
			DATA_TYPE res = gMax[0] - gMin[0];

			for (GUInt32 i = 1; i < SIZE; ++i)
				res *= (gMax[i] - gMin[i]);
			return res;
		}

		/*!
			Gets the Index-th dimension length.

			\param Index the index of dimension we wanna get
		*/
		inline const DATA_TYPE Dimension(const GUInt32 Index) const {

			G_ASSERT(Index < SIZE);
			return (gMax[Index] - gMin[Index]);
		}

		/*!
			Gets the Index-th half-dimension length.

			\param Index the index of half-dimension we wanna get
		*/
		inline const DATA_TYPE HalfDimension(const GUInt32 Index) const {

			G_ASSERT(Index < (GInt32)SIZE);
			return ((gMax[Index] - gMin[Index]) / (DATA_TYPE)2);
		}

		/*!
			Tests a point against box, using an epsilon tolerance. This test returns:

			- G_INSIDE if all TestPoint components are inside the box
			- G_OUTSIDE if all TestPoint components are outside the box
			- G_ONSURFACE if all TestPoint components are on box surface

			A single component (say its index is k) is inside surface if TestPoint[k] >= Min[k] - Epsilon and
			TestPoint[k] <= Max[k] + Epsilon.

			\param TestPoint the point to test
			\param Epsilon the tolerance used during comparisons
		*/
		GSurfaceSide PointSign(const GPoint<DATA_TYPE, SIZE>& TestPoint, const DATA_TYPE Epsilon = 0) const {

			for (GUInt32 i = 0; i < SIZE; ++i) {
				if ((TestPoint[i] > gMax[i] + Epsilon) || (TestPoint[i] < gMin[i] - Epsilon))
					return G_OUTSIDE;
			}
			for (GUInt32 i = 0; i < SIZE; ++i) {
				if ((TestPoint[i] <= gMin[i] + Epsilon) || (TestPoint[i] >= gMax[i] - Epsilon))
					return G_ONSURFACE;
			}
			return G_INSIDE;
		}

		//! Check if a point is inside the box. It uses PointSign() function.
		inline GBool IsPointInside(const GPoint<DATA_TYPE, SIZE>& TestPoint, const DATA_TYPE Epsilon = 0) const {

			return (PointSign(TestPoint, Epsilon) == G_INSIDE);			
		}

		//! Check if a point is outside the box. It uses PointSign() function.
		inline GBool IsPointOutside(const GPoint<DATA_TYPE, SIZE>& TestPoint, const DATA_TYPE Epsilon = 0) const {

			return (PointSign(TestPoint, Epsilon) == G_OUTSIDE);
		}

		//! Check if a point is touching the box (it's on its surface). It uses PointSign() function.
		inline GBool IsPointOnSurface(const GPoint<DATA_TYPE, SIZE>& TestPoint, const DATA_TYPE Epsilon = 0) const {

			return (PointSign(TestPoint, Epsilon) == G_ONSURFACE);
		}

		/*!
			Sets the minimum point of the box.

			\param NewMin the new minimum point.
			\note For every elements of NewMin that is not less than the Max's respective, a swap is done.
		*/
		void SetMin(const GPoint<DATA_TYPE, SIZE>& NewMin) {

			gMin = NewMin;
			FixCornersOrder();
		}

		/*!
			Sets the maximum point of the box.

			\param NewMax the new maximum point.
			\note For every elements of NewMax that is not greater than the Min's respective, a swap is done.
		*/
		void SetMax(const GPoint<DATA_TYPE, SIZE>& NewMax) {

			gMax = NewMax;
			FixCornersOrder();
		}

		/*!
			Sets a new minimum and maximum points.

			\param NewMin the new minimum point
			\param NewMax the new maximum point
			\note For every elements of NewMax that is not greater than the NewMin's respective, a swap is done.
		*/
		void SetMinMax(const GPoint<DATA_TYPE, SIZE>& NewMin, const GPoint<DATA_TYPE, SIZE>& NewMax) {

			gMin = NewMin;
			gMax = NewMax;
			FixCornersOrder();
		}

		/*!
			Sets the minimum and the maximum points of the box from an array of points. Min and Max points are set
			according to the minimum and maximum points found into specified points array.

			\param Points the array of points
		*/
		void SetMinMax(const GDynArray< GPoint<DATA_TYPE, SIZE> >& Points) {

			GUInt32 j = (GUInt32)Points.size();

			if (j < 2)
				return;
			gMin = Points[0];
			gMax = Points[1];
			FixCornersOrder();
			for (GUInt32 i = 2; i < j; ++i)
				ExtendToInclude(Points[i]);
		}

		/*!
			Update minimum and maximum points, so that specified NewPoint will reside on the updated box.
		*/
		inline void ExtendToInclude(const GPoint<DATA_TYPE, SIZE>& NewPoint) {

			for (GUInt32 w = 0; w < SIZE; w++) {
				if (gMin[w] > NewPoint[w])
					gMin[w] = NewPoint[w];
				if (gMax[w] < NewPoint[w])
					gMax[w] = NewPoint[w];
			}
		}

		/*!
			Compare two boxes to see if they are EXACTLY the same. In other words, this comparison is done with zero tolerance.

			\param Box the box to compare to
			\return G_TRUE if they are equal, G_FALSE otherwise.
		*/
		inline GBool operator ==(const GGenericAABox<DATA_TYPE, SIZE>& Box) const {

			return ((gMin == Box.Min()) && (gMax == Box.Max()));
		}

		/*!
			Compare two boxes to see if they are not EXACTLY the same. In other words, this comparison is done with
			zero tolerance.

			\param Box the to compare to.
			\return G_TRUE if they are not equal, G_FALSE otherwise.
		*/
		inline GBool operator !=(const GGenericAABox<DATA_TYPE, SIZE>& Box) const {

			return (!this->operator==(Box));
		}
	};

	//! Common 2D axes aligned box class, it uses GReal data type.
	typedef GGenericAABox<GReal, 2> GAABox2;
	//! Common 3D axes aligned box class, it uses GReal data type.
	typedef GGenericAABox<GReal, 3> GAABox3;

	/*!
		Compare two boxes to see if they are the same within the given tolerance.

		\param Box1 the first box to compare
		\param Box2 the second box to compare
		\param Epsilon the tolerance value to use

		\pre Epsilon must be >= 0
		\return G_TRUE if their points are within the given tolerance of each other, G_FALSE otherwise.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool IsEqual(const GGenericAABox<DATA_TYPE, SIZE>& Box1, const GGenericAABox<DATA_TYPE, SIZE>& Box2,
						 const DATA_TYPE& Epsilon) {
		G_ASSERT(Epsilon >= 0);
		return ((IsEqual(Box1.Min(), Box2.Min(), Epsilon)) && (IsEqual(Box1.Max(), Box2.Max(), Epsilon)));
	}

};	// end namespace Amanith

#endif
