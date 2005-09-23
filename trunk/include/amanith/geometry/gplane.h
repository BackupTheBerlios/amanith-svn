/****************************************************************************
** $file: amanith/geometry/gplane.h   0.1.1.0   edited Sep 24 08:00
**
** 3D Plane definition.
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

#ifndef GPLANE_H
#define GPLANE_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gvect.h"

/*!
	\file gplane.h
	\brief Geometrical 3D plane class, header file.
*/
namespace Amanith {
	/*!
		\class GGenericPlane
		\brief Defines a 3D geometrical plane.

		All points on the plane satisfy the equation Dot(Point, Plane.Normal) = Plane.Offset\n
		Normal is assumed to be normalized.

		\param DATA_TYPE the internal type used for the normal and offset.
		\note some plane implementation store D instead of offset. Thus those implementation have opposite sign from
		what we have (pg. 309 Computer Graphics 2nd Edition Hearn Baker)
	
<pre>
N dot P = -D
  |
  |-Ofs-|
__|_____|-->Normal
  |     |
</pre>
	*/
	template<typename DATA_TYPE>
	class GGenericPlane	{

	private:
		/*!
			The normal for this vector. For any point on the plane, Dot(pt, gNorm) = gOffset.
		*/
		GVect<DATA_TYPE, 3> gNorm;
		/*!
			This plane's offset from the origin such that for any point pt, Dot(pt, gNorm) = gOffset.\n
			Note that gOffset = -D (negative dist from the origin).
		*/
		DATA_TYPE gOffset;

	public:
		/*!
			Default constructor.

			Creates an uninitialized plane. In other words, the normal is (0, 0, 0) and the offset is 0.
		*/
		GGenericPlane()	: gOffset(0) {
		}

		/*!
			Set constructor.
			
			Creates a plane that the given points lie on.

			\param Point1 a point on the plane
			\param Point2 a point on the plane
			\param Point3 a point on the plane
		*/
		GGenericPlane(const GPoint<DATA_TYPE, 3>& Point1, const GPoint<DATA_TYPE, 3>& Point2,
					  const GPoint<DATA_TYPE, 3>& Point3) {

			GVect<DATA_TYPE, 3> vec12(Point2 - Point1);
			GVect<DATA_TYPE, 3> vec13(Point3 - Point1);

			Cross(gNorm, vec12, vec13);
			gNorm.Normalize();
			// Graphics Gems I: Page 390
			gOffset = Dot(static_cast< GVect<DATA_TYPE, 3> >(Point1), gNorm);
		}

		/*!
			Set constructor.

			Creates a plane with the given Normal on which Point resides.

			\param Normal the normal of the plane, it can be non-normalized also.
			\param Point a point that lies on the plane.
			\note set normal will be normalized.
		*/
		GGenericPlane(const GVect<DATA_TYPE, 3>& Normal, const GPoint<DATA_TYPE, 3>& Point) : gNorm(Normal) {

			// ensure unit normal
			gNorm.Normalize();
			gOffset = Dot(static_cast< GVect<DATA_TYPE, 3> >(Point), gNorm);
		}

		/*!
			Set constructor.

			Creates a plane with the given normal and offset.

			\param Normal the normal of the plane, it can be non-normalized also.
			\param Offset the plane offset constant
		*/
		GGenericPlane(const GVect<DATA_TYPE, 3>& Normal, const DATA_TYPE& Offset) : gNorm(Normal), gOffset(Offset) {

			// ensure unit normal
			gNorm.Normalize();
		}

		//! Copy constructor
		GGenericPlane(const GGenericPlane<DATA_TYPE>& SourcePlane) : gNorm(SourcePlane.gNorm), gOffset(SourcePlane.gOffset) {
		}

		//! Gets plane normal. It's a unit vector.
		inline const GVect<DATA_TYPE, 3>& Normal() const {
			return gNorm;
		}

		/*!
			Sets the normal for this plane to the given vector.

			\param NewNormal the new plane normal, it can be non-normalized also.
		*/
		inline void SetNormal(const GVect<DATA_TYPE, 3>& NewNormal) {

			gNorm = NewNormal;
			// ensure unit normal
			gNorm.Normalize();
		}

		//! Gets the offset of this plane from the origin such that the offset is the negative distance from the origin.
		inline const DATA_TYPE& Offset() const {
			return gOffset;
		}

		//! Sets the offset of this plane from the origin.
		inline void SetOffset(const DATA_TYPE& NewOffset) {
			gOffset = NewOffset;
		}

		/*!
			Compare two planes to see if they are EXACTLY the same. In other words, this
			comparison is done with zero tolerance.

			\param Plane the plane to compare to
			\return G_TRUE if they are equal, G_FALSE otherwise.
		*/
		inline GBool operator ==(const GGenericPlane<DATA_TYPE>& Plane) const {
			return ((this->gNorm == Plane.gNorm) && (this->gOffset == Plane.gOffset));
		}

		/*!
			Compare two planes to see if they are not EXACTLY the same. In other words,
			this comparison is done with zero tolerance.

			\param Plane the plane to compare to
			\return G_TRUE if they are not equal, G_FALSE otherwise.
		*/
		inline GBool operator !=(const GGenericPlane<DATA_TYPE>& Plane) const {
			return (!this->operator==(*this, Plane));
		}

		/*!
			Determines which side of the plane the given point lies with the given epsilon tolerance.

			\param TestPoint the point to test.
			\param Epsilon the epsilon tolerance to use while testing
			\return the GSurfaceSide enum describing on which side of the plane the point lies.
		*/
		GSurfaceSide WhichSide(const GPoint<DATA_TYPE, 3>& TestPoint, const DATA_TYPE Epsilon = 0) {

			DATA_TYPE sgnDist = (Dot(gNorm, static_cast< GVect<DATA_TYPE, 3> >(TestPoint)) - gOffset);

			if (sgnDist < Epsilon)
				return G_INSIDE;
			else
			if (sgnDist > Epsilon)
				return G_OUTSIDE;
			else
				return G_ONSURFACE;
		}

		//!	Mirror the point by the plane
		GPoint<DATA_TYPE, 3> Reflect(const GPoint<DATA_TYPE, 3>& TestPoint) {

			GPoint<DATA_TYPE, 3> point_on_plane = FindNearestPoint(*this, TestPoint);
			GVect<DATA_TYPE, 3> dir = point_on_plane - TestPoint;
			return (TestPoint + (dir * DATA_TYPE(2)));
		}

	};	// end class GGenericPlane


	//! Common 3D plane class, it uses GReal data type.
	typedef GGenericPlane<GReal> GPlane;


	/*!
		Finds the point on the plane that is nearest to the given point.
		
		\param Plane the plane to compare the point to.
		\param TestPoint the point to test.
		\result the point on Plane closest to TestPoint.
	*/
	template<typename DATA_TYPE>
	GPoint<DATA_TYPE, 3> FindNearestPoint(const GGenericPlane<DATA_TYPE>& Plane, const GPoint<DATA_TYPE, 3>& TestPoint) {

		DATA_TYPE distToPlane;
		distToPlane = Dot(Plane.Normal(), static_cast< GVect<DATA_TYPE, 3> >(TestPoint)) - Plane.Offset();
		//distToPlane = plane.Offset() + Dot(plane.Normal(), static_cast< GVect<DATA_TYPE, 3> >(pt));
		return (TestPoint - (Plane.Normal() * distToPlane));
	}

	template<typename DATA_TYPE>
	inline GPoint<DATA_TYPE, 3> FindNearestPoint(const GPoint<DATA_TYPE, 3>& TestPoint,
												 const GGenericPlane<DATA_TYPE>& Plane) {
		return FindNearestPoint(Plane, TestPoint);
	}

	/*!
		Compare two planes to see if they are the same within the given tolerance.

		\param Plane1 the first plane to compare
		\param Plane2 the second plane to compare
		\param Epsilon the tolerance value to use

		\pre Epsilon must be>= 0
		\return G_TRUE if they are equal within a tolerance, G_FALSE otherwise.
	*/
	template<typename DATA_TYPE>
	inline GBool IsEqual(const GGenericPlane<DATA_TYPE>& Plane1, const GGenericPlane<DATA_TYPE>& Plane2,
						const DATA_TYPE& Epsilon) {
		G_ASSERT(Epsilon >= 0);
		return ((IsEqual(Plane1.gNorm, Plane2.gNorm, Epsilon)) && (GMath::IsEqual(Plane1.gOffset, Plane2.gOffset, Epsilon)));
	}

};	// end namespace Amanith

#endif
