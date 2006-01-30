/****************************************************************************
** $file: amanith/geometry/goobox.h   0.3.0.0   edited Jan, 30 2006
**
** Generic object-oriented box definition.
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

#ifndef GOOBOX_H
#define GOOBOX_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gvect.h"

/*!
	\file goobox.h
	\brief Object oriented box class, header file.
*/

namespace Amanith {

	/*!
		\class GGenericOOBox
		\brief Object oriented box, in N-dimensional space.

		This class implements an arbitrarily oriented rectanguloid.\n
		Description is maintained by axes and half-dimensions along each axis.

		\param DATA_TYPE the internal type used for the axes and half-lengths
		\param SIZE dimension space (ex: 2 for a 2D box, 3 for a 3D box). It must be a positive number.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	class GGenericOOBox {

	private:
		//! The center point of the box
		GPoint<DATA_TYPE, SIZE> gCenter;
		//! The axes of the oriented box
		GVect<DATA_TYPE, SIZE> gAxis[SIZE];
		// Half lengths of the box, they are all positive
		GVect<DATA_TYPE, SIZE> gHalfDimensions;

	public:
		typedef DATA_TYPE DataType;

		//! Default constructor, set center at the origin and each axis to a null vector (all 0 components).
		inline GGenericOOBox() {
		}
		//! Copy constructor
		inline GGenericOOBox(GGenericOOBox<DATA_TYPE, SIZE>& SourceBox) {

			gCenter = SourceBox.gCenter;
			for (GUInt32 i = 0; i < SIZE; ++i) {
				gAxis[i] = SourceBox.gAxis[i];
				gHalfDimensions[i] = SourceBox.gHalfDimensions[i];
			}
		}

		//! Get the box center
		inline const GPoint<DATA_TYPE, SIZE>& Center() const {
			return gCenter;
		}
		//! Set a new center
		inline void SetCenter(const GPoint<DATA_TYPE, SIZE>& NewCenter) {
			gCenter = NewCenter;
		}

		//! Get the Index-th axis.
		inline const GVect<DATA_TYPE, SIZE>& Axis (const GUInt32 Index) const {
			G_ASSERT(Index < SIZE);
			return gAxis[Index];
		}

		//! Set a new value for the Index-th axis
		inline void SetAxis(const GUInt32 Index, const GVect<DATA_TYPE, SIZE>& NewAxis) {

			G_ASSERT(Index < SIZE);
			gAxis[Index] = NewAxis;
			gAxis[Index].Normalize();
		}

		//! Get box axes.
		inline const GVect<DATA_TYPE, SIZE>* Axes() const {
			return gAxis;
		}

		//! Set Index-th half-length to a new value.
		inline void SetHalfDimension(const GUInt32 Index, const DATA_TYPE NewValue) {
			G_ASSERT(Index < SIZE);
			gHalfDimensions[Index] = NewValue;
		}

		//! Get Index-th half-length.
		inline const DATA_TYPE& HalfDimension(const GUInt32 Index) const {
			G_ASSERT(Index < SIZE);
			return gHalfDimensions[Index];
		}

		//! Set Index-th length to a new value.
		inline void SetDimension(const GUInt32 Index, const DATA_TYPE NewValue) {
			G_ASSERT(Index < SIZE);
			gHalfDimensions[Index] = NewValue / (DATA_TYPE)2;
		}
		//! Get Index-th length.
		inline const DATA_TYPE Dimension(const GUInt32 Index) const {
			G_ASSERT(Index < SIZE);
			return (gHalfDimensions[Index] * (DATA_TYPE)2);
		}
		//! Get all half-lengths
		const GVect<DATA_TYPE, SIZE>& HalfDimensions() const {
			return gHalfDimensions;
		}

		/*!
			Gets the volume of the box. It's calculated as the product of lengths.

			\note For a 2D box, this value is equal to the area.
		*/
		DATA_TYPE Volume() const {

			G_ASSERT(SIZE > 0);
			DATA_TYPE res = (DATA_TYPE)2 * gHalfDimensions[0];

			for (GUInt32 i = 1; i < SIZE; ++i)
				res *= ((DATA_TYPE)2 * gHalfDimensions[i]);
			return res;
		}

		/*!
			Tests a point against box, using an epsilon tolerance. This test returns:

			- G_INSIDE if all TestPoint components are inside the box
			- G_OUTSIDE if all TestPoint components are outside the box
			- G_ONSURFACE if all TestPoint components are on box surface

			\param TestPoint the point to test
			\param Epsilon the tolerance used during comparisons
		*/
		GSurfaceSide PointSign(const GPoint<DATA_TYPE, SIZE>& TestPoint, DATA_TYPE Epsilon = 0) const {
			
			GVect<DATA_TYPE, SIZE> deltaCenter = TestPoint - gCenter;
			DATA_TYPE prj[SIZE];

			for (GUInt32 i = 0; i < SIZE; ++i) {
				prj[i] = Dot(deltaCenter, gAxis[i]);
				if ((prj[i] > gHalfDimensions[i] + Epsilon) || (prj[i] < -gHalfDimensions[i] - Epsilon))
					return G_OUTSIDE;
			}
			for (GUInt32 i = 0; i < SIZE; ++i) {
				if ((prj[i] <= -gHalfDimensions[i] + Epsilon) || (prj[i] >= gHalfDimensions[i] - Epsilon))
					return G_ONSURFACE;
			}
			return G_INSIDE;
		}

		//! Check if a point is inside the box. It uses PointSign() function.
		inline GBool IsPointInside(const GPoint<DATA_TYPE, 3>& TestPoint, DATA_TYPE Epsilon = 0) const {
			return (PointSign(TestPoint, Epsilon) == G_INSIDE);
		}

		//! Check if a point is outside the box. It uses PointSign() function.
		inline GBool IsPointOutside(const GPoint<DATA_TYPE, 3>& TestPoint, DATA_TYPE Epsilon = 0) const {
			return (PointSign(TestPoint, Epsilon) == G_OUTSIDE);
		}

		//! Check if a point is touching the box (it's on its surface). It uses PointSign() function.
		inline GBool IsPointOnSurface(const GPoint<DATA_TYPE, 3>& TestPoint, DATA_TYPE Epsilon = 0) const {
			return (PointSign(TestPoint, Epsilon) == G_ONSURFACE);
		}

		/*!
			Compare two boxes to see if they are EXACTLY the same. In other words, this comparison is done with zero tolerance.

			\param Box the box to compare to
			\return G_TRUE if they are equal, G_FALSE otherwise.
		*/
		inline GBool operator ==(const GGenericOOBox<DATA_TYPE, SIZE>& Box) const {

			GBool res = (this->gCenter == Box.Center());
			for (GUInt32 i = 0; i < SIZE; ++i) {
				res &= (this->Axis(i) == Box.Axis(i));
				res &= (this->HalfDimension(i) == Box.HalfDimension(i));
			}
			return res;
		}

		/*!
			Compare two boxes to see if they are not EXACTLY the same. In other words, this comparison is done with
			zero tolerance.

			\param Box the to compare to.
			\return G_TRUE if they are not equal, G_FALSE otherwise.
		*/
		inline GBool operator !=(const GGenericOOBox<DATA_TYPE, SIZE>& Box) const {

			return (!this->operator==(Box));
		}
	};

	
	//! Common 2D oriented box class, it uses GReal data type.
	typedef GGenericOOBox<GReal, 2> GOOBox2;
	//! Common 3D oriented box class, it uses GReal data type.
	typedef GGenericOOBox<GReal, 3> GOOBox3;

	/*!
		Compare two boxes to see if they are the same within the given tolerance.

		\param Box1 the first box to compare
		\param Box2 the second box to compare
		\param Epsilon the tolerance value to use

		\pre Epsilon must be >= 0
		\return G_TRUE if their points are within the given tolerance of each other, G_FALSE otherwise.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool IsEqual(const GGenericOOBox<DATA_TYPE, SIZE>& Box1, const GGenericOOBox<DATA_TYPE, SIZE>& Box2,
						 const DATA_TYPE& Epsilon) {

		G_ASSERT(Epsilon >= 0);
		GBool res = IsEqual(Box1.Center(), Box2.Center(), Epsilon);
		for (GUInt32 i = 0; i < SIZE; ++i) {
			res &= IsEqual(Box1.Axis(i), Box2.Axis(i), Epsilon);
			res &= IsEqual(Box1.HalfDimension(i), Box2.HalfDimension(i), Epsilon);
		}
		return res;
	}

};	// end namespace Amanith

#endif
