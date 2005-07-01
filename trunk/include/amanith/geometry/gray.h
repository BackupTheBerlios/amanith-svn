/****************************************************************************
** $file: amanith/geometry/gdistance.h   0.1.0.0   edited Jun 30 08:00
**
** Generic ray definition.
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

#ifndef GRAY_H
#define GRAY_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gvect.h"

/*!
	\file gray.h
	\brief GRay class header file.
 */
namespace Amanith {

	/*!
		\class GGenericRay
		\brief GGenericRay class describes an N-dimensional ray.
		
		This is represented by a point origin O and a vector direction D. Any point on the ray can be
		described as:\n\n

		P(t) = O + t * D\n\n

		where t > 0 for points along the ray direction D\n
		where t < 0 for points along the opposite ray direction D
		
		\param DATA_TYPE the internal type used for the point and vector.
		\param SIZE the dimension of space (ex: 2 for a 2D ray, 3 for a 3D ray). It must be a positive number.
		\note Direction vector could be not normalized. Use Normalize() function to normalize it.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	class GGenericRay {

	protected:
		//! The origin of the ray.
		GPoint<DATA_TYPE, SIZE> gOrigin;

		//! The vector along which the ray lies.
		GVect<DATA_TYPE, SIZE> gDirection;

	public:
		//! Default constructor, constructs a ray at the origin with a zero vector.
		GGenericRay() {
		}

		/*!
			Set constructor, constructs a ray with the given origin and vector.

			\param Origin the point at which the ray starts
			\param Direction the vector describing the direction and length of the ray starting at origin
		*/
		GGenericRay(const GPoint<DATA_TYPE, SIZE>& Origin, const GVect<DATA_TYPE, SIZE>& Direction)
		: gOrigin(Origin), gDirection(Direction) {
		}

		/*!
			Copy constructor, constructs an exact duplicate of the given ray.
		
			\param SourceRay the ray to copy
		*/
		GGenericRay(const GGenericRay& SourceRay) : gOrigin(SourceRay.gOrigin), gDirection(SourceRay.gDirection) {
		}

		//! Gets the origin of the ray.
		const GPoint<DATA_TYPE, SIZE>& Origin() const {
			return gOrigin;
		}

		//! Sets the origin point for this ray.
		void SetOrigin(const GPoint<DATA_TYPE, SIZE>& NewOrigin) {
			gOrigin = NewOrigin;
		}

		//! Gets the vector describing the direction and length of the ray.
		const GVect<DATA_TYPE, SIZE>& Direction() const {
			return gDirection;
		}

		//! Sets the vector describing the direction and length of the ray. It can be non-normalized also.
		void SetDirection(const GVect<DATA_TYPE, SIZE>& NewDirection) {
			gDirection = NewDirection;
		}

		/*!
			Normalize ray direction

			\return the length of the ray direction before normalization.
		*/
		inline DATA_TYPE Normalize() {
			return gDirection.Normalize();
		}

		//! Return G_TRUE is ray's direction is normalized (unit length)
		inline GBool IsNormalized() const {
			return gDirection.IsNormalized();
		}

		/*!
			Compare two rays to see if they are EXACTLY the same.

			\param Ray the first Ray to compare to
			\return G_TRUE if they are equal, G_FALSE otherwise.
		*/
		inline GBool operator ==(const GGenericRay<DATA_TYPE, SIZE>& Ray) const {
			return ((gOrigin == Ray.Origin()) && (gDirection() == Ray.Direction()));
		}

		/*!
			Compare two line segments to see if they are not EXACTLY the same.

			\param Ray the Ray to compare to
			\return G_TRUE if they are not equal, G_FALSE otherwise
		*/
		inline GBool operator !=(const GGenericRay<DATA_TYPE, SIZE>& Ray) const {
			return (!this->operator==(Ray));
		}
	};


	//! Common 2D ray class, it uses GReal data type.
	typedef GGenericRay<GReal, 2> GRay2;
	//! Common 3D ray class, it uses GReal data type.
	typedef GGenericRay<GReal, 3> GRay3;

	/*!
		Compare two rays to see if the are the same within the given tolerance.

		\param Ray1 the first Ray to compare
		\param Ray2 the second Ray to compare
		\param Epsilon the tolerance value to use
		\pre Epsilon must be >= 0
		\return G_TRUE if they are equal within the tolerance, G_FALSE otherwise.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool IsEqual(const GGenericRay<DATA_TYPE, SIZE>& Ray1, const GGenericRay<DATA_TYPE, SIZE>& Ray2,
						 const DATA_TYPE& Epsilon) {
		G_ASSERT(Epsilon >= 0);
		return ((IsEqual(Ray1.Origin(), Ray2.Origin(), Epsilon)) && (IsEqual(Ray1.Direction(), Ray2.Direction(), Epsilon)));
	}


	/*!
		Finds the closest point along the infinitive line spanned by ray direction, to a given point.

		\param Ray the ray whose direction describes the infinitive line
		\param TestPoint the point which to test against ray

		\return the point along the ray direction closest to TestPoint
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	GPoint<DATA_TYPE, SIZE> FindNearestPoint(const GGenericRay<DATA_TYPE, SIZE>& Ray,
											 const GPoint<DATA_TYPE, SIZE>& TestPoint) {
		return (Ray.Origin() + Ray.Direction() * Dot(TestPoint - Ray.Origin(), Ray.Direction()));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	GPoint<DATA_TYPE, SIZE> FindNearestPoint(const GPoint<DATA_TYPE, SIZE>& TestPoint,
											 const GGenericRay<DATA_TYPE, SIZE>& Ray) {
		return FindNearestPoint(Ray, TestPoint);
	}

};	// end namespace Amanith

#endif
