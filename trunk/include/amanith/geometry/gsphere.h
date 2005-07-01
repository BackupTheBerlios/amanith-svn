/****************************************************************************
** $file: amanith/geometry/gsphere.h   0.1.0.0   edited Jun 30 08:00
**
** Generic sphere (disk) definition.
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

#ifndef GSPHERE_H
#define GSPHERE_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gvect.h"

/*!
	\file gsphere.h
	\brief Sphere class header file.
*/
namespace Amanith {
	/*!
		\class GGenericSphere
		\brief Describes a sphere in N-dimensional space by its center point and its radius.
	
		This kind of geometric primitive can be used as a bounding volume in computer graphics.
		\param DATA_TYPE the internal type used for the point and radius
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	class GGenericSphere {

	private:
		//! The center of the sphere.
		GPoint<DATA_TYPE, SIZE> gCenter;
		//!* The radius of the sphere.
		DATA_TYPE gRadius;

	public:
		/*!
			Default constructor, constructs a sphere centered at the origin with a radius of 0.
		*/
		GGenericSphere() : gRadius(0) {
		}

		/*!
			Set constructor, constructs a sphere centered at the origin with the given radius.
		*/
		GGenericSphere(const DATA_TYPE& Radius): gRadius(Radius) {
		}

		/*!
			Constructor, constructs a sphere with the given center and radius.

			\param Center the point at which to center the sphere.
			\param Radius the radius of the sphere.
		*/
		GGenericSphere(const GPoint<DATA_TYPE, SIZE>& Center, const DATA_TYPE& Radius)
		: gCenter(Center), gRadius(Radius) {
		}

		/*/
			Copy constructor, constructs a duplicate of the given sphere.

			\param SrcSphere the sphere to make a copy of
		*/
		GGenericSphere(const GGenericSphere<DATA_TYPE, SIZE>& SrcSphere)
		: gCenter(SrcSphere.gCenter), gRadius(SrcSphere.gRadius) {
		}

		//! Gets the center of the sphere.
		const GPoint<DATA_TYPE, SIZE>& Center() const {
			return gCenter;
		}

		//! Gets the radius of the sphere.
		const DATA_TYPE& Radius() const {
			return gRadius;
		}

		/*!
			Gets the volume of the sphere.
			
			This function works for every N-Dimensional sphere. In the case of a 2D sphere (a circle) this
			volume is equal to area.
		*/
		DATA_TYPE Volume() const {
			
			GUInt32 n;
			GDouble k, res;

			n = SIZE / 2;
			if (SIZE & 1) {
				k = (((GDouble)GMath::Factorial(n)) * GMath::Pow(2.0, (GDouble)SIZE)) / ((GDouble)GMath::Factorial(SIZE));
				res = k * (GMath::Pow(G_PI, (GDouble)n) * GMath::Pow((GDouble)gRadius, (GDouble)SIZE));
			}
			else {
				k = GMath::Pow(G_PI, (GDouble)n) / (GDouble)GMath::Factorial(n);
				res = k * GMath::Pow((GDouble)gRadius, (GDouble)SIZE);
			}
			return (DATA_TYPE)res;
		}

		/*!
			Tests a point against the sphere, using an epsilon tolerance. This test returns:

			- G_INSIDE if all TestPoint components are inside the sphere
			- G_OUTSIDE if all TestPoint components are outside the sphere
			- G_ONSURFACE if all TestPoint components are on surface sphere

			\param TestPoint the point to test
			\param Epsilon the tolerance used during comparisons
		*/
		GSurfaceSide PointSign(const GPoint<DATA_TYPE, SIZE>& TestPoint, const DATA_TYPE Epsilon = 0) {

			GVect<DATA_TYPE, SIZE> aux = TestPoint - gCenter;
			DATA_TYPE l = aux.Length();

			if (l > (gRadius + Epsilon))
				return G_OUTSIDE;
			else
			if (l < (gRadius - Epsilon))
				return G_INSIDE;
			else
				return G_ONSURFACE;
		}

		//! Check if a point is inside the box. It uses PointSign() function.
		inline GBool IsPointInside(const GPoint<DATA_TYPE, SIZE>& TestPoint, DATA_TYPE Epsilon = 0) {
			return (PointSign(TestPoint, Epsilon) == G_INSIDE);
		}

		//! Check if a point is outside the box. It uses PointSign() function.
		inline GBool IsPointOutside(const GPoint<DATA_TYPE, SIZE>& TestPoint, DATA_TYPE Epsilon = 0) {
			return (PointSign(TestPoint, Epsilon) == G_OUTSIDE);
		}

		//! Check if a point is touching the box (it's on its surface). It uses PointSign() function.
		inline GBool IsPointOnSurface(const GPoint<DATA_TYPE, SIZE>& TestPoint, DATA_TYPE Epsilon = 0) {
			return (PointSign(TestPoint, Epsilon) == G_ONSURFACE);
		}

		/*!
			Sets the center point of the sphere.

			\param NewCenter the new point at which to center the sphere
		*/
		void SetCenter(const GPoint<DATA_TYPE, SIZE>& NewCenter) {
			gCenter = NewCenter;
		}

		/*!
			Sets the radius of the sphere.

			\param NewRadius the new radius of the sphere
		*/
		void SetRadius(const DATA_TYPE& NewRadius) {
			gRadius = NewRadius;
		}

		/*!
			Compare two spheres to see if they are EXACTLY the same. 

			\param Sph the sphere to compare to
			\return G_TRUE if they are equal, G_FALSE otherwise.
		*/
		inline GBool operator ==(const GGenericSphere<DATA_TYPE, SIZE>& Sph) const {
			return ((gCenter == Sph.gCenter) && (gRadius == Sph.gRadius));
		}

		/*!
			Compare two spheres to see if they are not EXACTLY the same. 

			\param Sph the sphere to compare
			\return G_TRUE if they are not equal, G_FALSE otherwise.
		*/
		inline GBool operator !=(const GGenericSphere<DATA_TYPE, SIZE>& Sph) const {
			return (!this->operator==(Sph));
		}
	};

	//! Common 2D sphere class, it uses GReal data type.
	typedef GGenericSphere<GReal, 2> GSphere2;
	//! Common 3D sphere class, it uses GReal data type.
	typedef GGenericSphere<GReal, 3> GSphere3;

	/*!
		Compare two spheres to see if they are the same within the given tolerance.

		\param Sph1 the first sphere to compare
		\param Sph2 the second sphere to compare
		\param Epsilon the tolerance value to use
		\pre Epsilon must be >= 0
		\return G_TRUE if they are equal within a tolerance, G_FALSE otherwise.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool IsEqual(const GGenericSphere<DATA_TYPE, SIZE>& Sph1, const GGenericSphere<DATA_TYPE, SIZE>& Sph2,
						 const DATA_TYPE& Epsilon) {
		G_ASSERT(Epsilon >= 0);
		return ((IsEqual(Sph1.Center(), Sph2.Center(), Epsilon)) &&	(GMath::IsEqual(Sph1.Radius(), Sph2.Radius(), Epsilon)));
	}

};	// end namespace Amanith

#endif
