/****************************************************************************
** $file: amanith/geometry/gdistance.h   0.1.0.0   edited Jun 30 08:00
**
** Basic distance query functions for every simple geometry
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

#ifndef GDISTANCE_H
#define GDISTANCE_H

#include "amanith/gglobal.h"
#include "amanith/geometry/glineseg.h"
#include "amanith/geometry/gsphere.h"
#include "amanith/geometry/gaabox.h"
#include "amanith/geometry/goobox.h"
#include "amanith/geometry/gplane.h"
#include "amanith/geometry/garea.h"

/*!
	\file gdistance.h
	\brief Distance queries for all geometric entities (boxes, sphere and so on).
*/
namespace Amanith {

	// point - point
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GPoint<DATA_TYPE, SIZE>& pt1, const GPoint<DATA_TYPE, SIZE>& pt2) {
		return Length(pt2 - pt1);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GPoint<DATA_TYPE, SIZE>& pt1, const GPoint<DATA_TYPE, SIZE>& pt2) {
		return LengthSquared(pt2 - pt1);
	}


	// plane - point
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericPlane<DATA_TYPE>& plane, const GPoint<DATA_TYPE, 3>& pt) {
		return (Dot(plane.Normal(), static_cast< GVect<DATA_TYPE, 3> >(pt)) - plane.Offset());
	}

	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GPoint<DATA_TYPE, 3>& pt, const GGenericPlane<DATA_TYPE>& plane) {
		return (Dot(plane.Normal(), static_cast< GVect<DATA_TYPE, 3> >(pt)) - plane.Offset());
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericPlane<DATA_TYPE>& plane, const GPoint<DATA_TYPE, 3>& pt) {
		return (GMath::Abs(SignedDistance(plane, pt)));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GPoint<DATA_TYPE, 3>& pt, const GGenericPlane<DATA_TYPE>& plane) {
		return (GMath::Abs(SignedDistance(plane, pt)));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericPlane<DATA_TYPE>& plane, const GPoint<DATA_TYPE, 3>& pt) {
		return GMath::Sqr(SignedDistance(plane, pt));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GPoint<DATA_TYPE, 3>& pt, const GGenericPlane<DATA_TYPE>& plane) {
		return GMath::Sqr(SignedDistance(plane, pt));
	}

	// plane - ray
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericPlane<DATA_TYPE>& plane, const GGenericRay<DATA_TYPE, 3>& ray) {

		DATA_TYPE dirsProj = Dot(ray.Direction(), plane.Normal());

		// ray is parallel (or it lies on the plane)
		if (GMath::Abs(dirsProj) <= G_EPSILON)
			return Distance(plane, ray.Origin());
		else {
			DATA_TYPE orgProj = -(Dot(plane.Normal(), (const GVect<DATA_TYPE, 3>&)ray.Origin()) + plane.Offset());
			DATA_TYPE t = orgProj / dirsProj;
			if (t <= 0)
				return Distance(plane, ray.Origin());
			else
				return -Distance(plane, ray.Origin());
		}
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GGenericRay<DATA_TYPE, 3>& ray, const GGenericPlane<DATA_TYPE>& plane) {
		return SignedDistance(plane, ray);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericPlane<DATA_TYPE>& plane, const GGenericRay<DATA_TYPE, 3>& ray) {
		return GMath::Abs(SignedDistance(plane, ray));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericRay<DATA_TYPE, 3>& ray, const GGenericPlane<DATA_TYPE>& plane) {
		return Distance(plane, ray);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericPlane<DATA_TYPE>& plane, const GGenericRay<DATA_TYPE, 3>& ray) {
		return GMath::Sqr(SignedDistance(plane, ray));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericRay<DATA_TYPE, 3>& ray, const GGenericPlane<DATA_TYPE>& plane) {
		return DistanceSquared(plane, ray);
	}

	// plane - plane
	template<typename DATA_TYPE>
	DATA_TYPE Distance(const GGenericPlane<DATA_TYPE>& Plane1, const GGenericPlane<DATA_TYPE>& Plane2) {

		DATA_TYPE normalsDot = Dot(Plane1.Normal(), Plane2.Normal());
		DATA_TYPE dist;

		// planes are parallel (maybe distinct or coincident)
		if (GMath::Abs(normalsDot) >= 1 - G_EPSILON) {
			// same normals direction
			if (normalsDot >= 0)
				dist = Plane1.Offset() - Plane2.Offset();
			// opposite normals direction
			else
				dist = Plane1.Offset() + Plane2.Offset();
			return GMath::Abs(dist);
		}
		else
			return 0;
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericPlane<DATA_TYPE>& Plane1, const GGenericPlane<DATA_TYPE>& Plane2) {
		return GMath::Sqr(Distance(Plane1, Plane2));
	}

	// plane - sphere
	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GGenericPlane<DATA_TYPE>& Plane, const GGenericSphere<DATA_TYPE, 3>& sph) {
		return Distance(Plane, sph.Center()) - sph.Radius();
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GGenericSphere<DATA_TYPE, 3>& sph, const GGenericPlane<DATA_TYPE>& Plane) {
		return SignedDistance(Plane, sph);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericPlane<DATA_TYPE>& Plane, const GGenericSphere<DATA_TYPE, 3>& sph) {
		return GMath::Abs(SignedDistance(Plane, sph));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericSphere<DATA_TYPE, 3>& sph, const GGenericPlane<DATA_TYPE>& Plane) {
		return Distance(Plane, sph);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericPlane<DATA_TYPE>& Plane, const GGenericSphere<DATA_TYPE, 3>& sph) {
		return GMath::Sqr(SignedDistance(Plane, sph));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericSphere<DATA_TYPE, 3>& sph, const GGenericPlane<DATA_TYPE>& Plane) {
		return DistanceSquared(Plane, sph);
	}

	// plane - axis aligned box (3D)
	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GGenericPlane<DATA_TYPE>& Plane, const GGenericAABox<DATA_TYPE, 3>& aab) {

		#define UPDATE_CONCORDE_DISTANCE \
			if (Dot(centerToNear, tmpPoint - nearestPoint) > 0) { \
				tmpDist = Distance(tmpPoint, Plane); \
				if (tmpDist > sDist) \
					sDist = tmpDist; \
			}

		GPoint<DATA_TYPE, 3> nearestPoint = FindNearestPoint(Plane, aab.Center());
		DATA_TYPE absDirX = GMath::Abs(Plane.Normal()[G_X]);
		DATA_TYPE absDirY = GMath::Abs(Plane.Normal()[G_Y]);
		DATA_TYPE absDirZ = GMath::Abs(Plane.Normal()[G_Z]);
		DATA_TYPE sDist = SignedDistance(nearestPoint, aab);

		if ((sDist >= 0) || (absDirX <= G_EPSILON && absDirY <= G_EPSILON) ||
			(absDirX <= G_EPSILON && absDirZ <= G_EPSILON) || (absDirY <= G_EPSILON && absDirZ <= G_EPSILON))
				return sDist;
		else {
			DATA_TYPE tmpDist, sDist = -1;
			GPoint<DATA_TYPE, 3> tmpPoint;
			GVect<DATA_TYPE, 3> centerToNear = nearestPoint - aab.Center();
			// if nearest point passes through center, we can choose as concordance direction the
			// plane normal; NB: it's important to consider the relative distance
			// centerToNear respect to the box dimensions, to avoid numerical instabilities
			DATA_TYPE maxDim = GMath::Max(aab.Dimension(G_X), aab.Dimension(G_Y), aab.Dimension(G_Z)) / (DATA_TYPE)2;
			if (centerToNear.LengthSquared() / (maxDim * maxDim) <= G_EPSILON * G_EPSILON)
				centerToNear = Plane.Normal();
			// distance is now the quantity the ray has to move along centerToNear direction to exit
			// the box
			tmpPoint = aab.Min();
			UPDATE_CONCORDE_DISTANCE
			tmpPoint = aab.Max();
			UPDATE_CONCORDE_DISTANCE
			tmpPoint.Set(aab.Min()[G_X], aab.Min()[G_Y], aab.Max()[G_Z]);
			UPDATE_CONCORDE_DISTANCE
			tmpPoint.Set(aab.Min()[G_X], aab.Max()[G_Y], aab.Max()[G_Z]);
			UPDATE_CONCORDE_DISTANCE
			tmpPoint.Set(aab.Min()[G_X], aab.Max()[G_Y], aab.Min()[G_Z]);
			UPDATE_CONCORDE_DISTANCE
			tmpPoint.Set(aab.Max()[G_X], aab.Min()[G_Y], aab.Min()[G_Z]);
			UPDATE_CONCORDE_DISTANCE
			tmpPoint.Set(aab.Max()[G_X], aab.Min()[G_Y], aab.Max()[G_Z]);
			UPDATE_CONCORDE_DISTANCE
			tmpPoint.Set(aab.Max()[G_X], aab.Max()[G_Y], aab.Min()[G_Z]);
			UPDATE_CONCORDE_DISTANCE
			return -sDist;
		}
		#undef UPDATE_CONCORDE_DISTANCE
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GGenericAABox<DATA_TYPE, 3>& aab, const GGenericPlane<DATA_TYPE>& Plane) {
		return SignedDistance(Plane, aab);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericPlane<DATA_TYPE>& Plane, const GGenericAABox<DATA_TYPE, 3>& aab) {
		return GMath::Abs(SignedDistance(Plane, aab));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericAABox<DATA_TYPE, 3>& aab, const GGenericPlane<DATA_TYPE>& Plane) {
		return Distance(Plane, aab);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericPlane<DATA_TYPE>& Plane, const GGenericAABox<DATA_TYPE, 3>& aab) {
		return GMath::Sqr(SignedDistance(Plane, aab));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericAABox<DATA_TYPE, 3>& aab, const GGenericPlane<DATA_TYPE>& Plane) {
		return DistanceSquared(Plane, aab);
	}

	// plane - object oriented box (3D)
	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GGenericPlane<DATA_TYPE>& Plane, const GGenericOOBox<DATA_TYPE, 3>& oob) {

		GVect<DATA_TYPE, 3> tmpNormal;
		GVect<DATA_TYPE, 3> pointOnPlane = (Plane.Normal() * Plane.Offset()) - oob.Center();
		GVect<DATA_TYPE, 3> rotatedPointOnPlane;

		// transform ray into box frame
		for (GUInt32 i = 0; i < 3; i++) {
			tmpNormal[i] = Dot(Plane.Normal(), oob.Axis(i));
			rotatedPointOnPlane[i] = Dot(pointOnPlane, oob.Axis(i));
		}
		rotatedPointOnPlane += oob.Center();

		GGenericPlane<DATA_TYPE> tmpPlane(tmpNormal, rotatedPointOnPlane);
		// after transformation box is axis aligned, centered at axes origin
		GGenericAABox<DATA_TYPE, 3> tmpBox(oob.Center() - oob.HalfDimensions(), oob.Center() + oob.HalfDimensions());
		return SignedDistance(tmpPlane, tmpBox);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GGenericOOBox<DATA_TYPE, 3>& oob, const GGenericPlane<DATA_TYPE>& Plane) {
		return SignedDistance(Plane, oob);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericPlane<DATA_TYPE>& Plane, const GGenericOOBox<DATA_TYPE, 3>& oob) {
		return GMath::Abs(SignedDistance(Plane, oob));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE Distance(const GGenericOOBox<DATA_TYPE, 3>& oob, const GGenericPlane<DATA_TYPE>& Plane) {
		return Distance(Plane, oob);
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericPlane<DATA_TYPE>& Plane, const GGenericOOBox<DATA_TYPE, 3>& oob) {
		return GMath::Sqr(SignedDistance(Plane, oob));
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE DistanceSquared(const GGenericOOBox<DATA_TYPE, 3>& oob, const GGenericPlane<DATA_TYPE>& Plane) {
		return DistanceSquared(Plane, oob);
	}


	// signed distance ray - point (2D)
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericRay<DATA_TYPE, 2>& ray, const GPoint<DATA_TYPE, 2>& pt) {

		// in 2D space, a <0 distance means that point lies on the right side of ray, else it's on left side
		DATA_TYPE rayPrj = Dot(pt - ray.Origin(), ray.Direction());
		DATA_TYPE dist, area;

		if (rayPrj <= 0)
			dist = Distance(ray.Origin(), pt);
		else
			dist = Length(pt - (ray.Origin() + ray.Direction() * rayPrj));

		area = TwiceSignedArea(ray.Origin(), ray.Origin() + ray.Direction(), pt);
		if (area != 0)
			return (dist * GMath::Sign(TwiceSignedArea(ray.Origin(), ray.Origin() + ray.Direction(), pt)));
		else
			return dist;
	}

	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GPoint<DATA_TYPE, 2>& pt, const GGenericRay<DATA_TYPE, 2>& ray) {
		return SignedDistance(ray, pt);
	}

	// distance ray - point (2D & 3D)
	template<typename DATA_TYPE, GUInt32 SIZE>
	DATA_TYPE Distance(const GGenericRay<DATA_TYPE, SIZE>& ray, const GPoint<DATA_TYPE, SIZE>& pt) {

		DATA_TYPE rayPrj = Dot(pt - ray.Origin(), ray.Direction());

		if (rayPrj <= 0)
			return Distance(ray.Origin(), pt);
		else
			return Length(pt - (ray.Origin() + ray.Direction() * rayPrj));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericRay<DATA_TYPE, SIZE>& ray) {
		return Distance(ray, pt);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericRay<DATA_TYPE, SIZE>& ray, const GPoint<DATA_TYPE, SIZE>& pt) {

		DATA_TYPE rayPrj = Dot(pt - ray.Origin(), ray.Direction());

		if (rayPrj <= 0)
			return DistanceSquared(ray.Origin(), pt);
		else
			return LengthSquared(pt - (ray.Origin() + ray.Direction() * rayPrj));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericRay<DATA_TYPE, SIZE>& ray) {
		return DistanceSquared(ray, pt);
	}


	// ray - ray (2D)
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericRay<DATA_TYPE, 2>& ray1, const GGenericRay<DATA_TYPE, 2>& ray2) {

		DATA_TYPE det = ray2.Direction()[G_X] * ray1.Direction()[G_Y] - ray2.Direction()[G_Y] * ray1.Direction()[G_X];

		// rays are parallel
		if (GMath::Abs(det) <= G_EPSILON) {
			DATA_TYPE dist1 = Distance(ray1.Origin(), ray2);
			DATA_TYPE dist2 = Distance(ray2.Origin(), ray1);
			return GMath::Min(dist1, dist2);
		}
		else {
			// find parameters of intersection
			GVect<DATA_TYPE, 2> diffOrigins = ray2.Origin() - ray1.Origin();
			DATA_TYPE invDet = (DATA_TYPE)1 / det;
			DATA_TYPE param0 = (ray2.Direction()[G_X] * diffOrigins[G_Y] - ray2.Direction()[G_Y] * diffOrigins[G_X]) * invDet;
			DATA_TYPE param1 = (ray1.Direction()[G_X] * diffOrigins[G_Y] - ray1.Direction()[G_Y] * diffOrigins[G_X]) * invDet;
			// ray do not intersect
			if (param0 < 0 && param1 < 0) {
				DATA_TYPE dist1 = Distance(ray1.Origin(), ray2);
				DATA_TYPE dist2 = Distance(ray2.Origin(), ray1);
				return GMath::Min(dist1, dist2);
			}
			else
			// rays do intersect
			if (param0 > 0 && param1 > 0) {
				DATA_TYPE dist1 = Distance(ray1.Origin(), ray2);
				DATA_TYPE dist2 = Distance(ray2.Origin(), ray1);
				return -GMath::Min(dist1, dist2);
			}
			else {
				if (param0 <= 0)
					return Distance(ray1.Origin(), ray2);
				else
					return Distance(ray2.Origin(), ray1);
			}
		}
	}

	// ray - ray (3D)
	template<typename DATA_TYPE>
	inline DATA_TYPE SignedDistance(const GGenericRay<DATA_TYPE, 3>& ray1, const GGenericRay<DATA_TYPE, 3>& ray2) {

		GVect<DATA_TYPE, 3> h(Cross(ray1.Direction(), ray2.Direction()));

		// rays are parallel
		if (h.LengthSquared() <= G_EPSILON * G_EPSILON) {
			DATA_TYPE dist1 = Distance(ray1.Origin(), ray2);
			DATA_TYPE dist2 = Distance(ray2.Origin(), ray1);
			return GMath::Min(dist1, dist2);
		}
		else {
			DATA_TYPE dist = GMath::Abs(Dot(h, (const GVect<DATA_TYPE, 3>&)ray1.Origin()) - Dot(h, (const GVect<DATA_TYPE, 3>&)ray2.Origin()));
			// ray are askew and not coplanar
			if (dist > G_EPSILON)
				return dist;
			// ray are incident, lets calculate parameters
			DATA_TYPE a11 = ray1.Direction().LengthSquared();
			DATA_TYPE a12 = -Dot(ray1.Direction(), ray2.Direction());
			DATA_TYPE a22 = ray2.Direction().LengthSquared();
			// Cramer determinants
			DATA_TYPE D0 = a11 * a22 - a12 * a12;
			G_ASSERT(GMath::Abs(D0) > G_EPSILON);

			GVect<DATA_TYPE, 3> diffOrigins(ray2.Origin() - ray1.Origin());
			DATA_TYPE b1 = Dot(ray1.Direction(), diffOrigins);
			DATA_TYPE b2 = -Dot(ray2.Direction(), diffOrigins);
			DATA_TYPE D1 = b1 * a22 - b2 * a12;
			DATA_TYPE D2 = -b1 * a12 + b2 * a11;
			DATA_TYPE param0 = D1 / D0;
			DATA_TYPE param1 = D2 / D0;
			// ray do not intersect
			if (param0 < 0 && param1 < 0) {
				DATA_TYPE dist1 = Distance(ray1.Origin(), ray2);
				DATA_TYPE dist2 = Distance(ray2.Origin(), ray1);
				return GMath::Min(dist1, dist2);
			}
			else {
				// rays do intersect
				if (param0 > 0 && param1 > 0) {
					DATA_TYPE dist1 = Distance(ray1.Origin(), ray2);
					DATA_TYPE dist2 = Distance(ray2.Origin(), ray1);
					return -GMath::Min(dist1, dist2);
				}
				else {
					if (param0 <= 0)
						return Distance(ray1.Origin(), ray2);
					else
						return Distance(ray2.Origin(), ray1);
				}
			}

		}
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericRay<DATA_TYPE, SIZE>& ray1, const GGenericRay<DATA_TYPE, SIZE>& ray2) {
		return GMath::Abs(SignedDistance(ray1, ray2));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericRay<DATA_TYPE, SIZE>& ray1, const GGenericRay<DATA_TYPE, SIZE>& ray2) {
		return GMath::Sqr(SignedDistance(ray1, ray2));
	}

	// sphere - ray (2D & 3D)
	template<typename DATA_TYPE, GUInt32 SIZE>
	DATA_TYPE SignedDistance(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericRay<DATA_TYPE, SIZE>& ray) {

		DATA_TYPE prj = Dot(sph.Center() - ray.Origin(), ray.Direction());

		if (prj <= 0)
			return(Distance(ray.Origin(), sph.Center()) - sph.Radius());
		else {
			GVect<DATA_TYPE, SIZE> nearestPoint = ray.Origin() + ray.Direction() * prj;
			return (Length(nearestPoint - sph.Center()) - sph.Radius());
		}
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return SignedDistance(sph, ray);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericRay<DATA_TYPE, SIZE>& ray) {
		return GMath::Abs(SignedDistance(sph, ray));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return Distance(sph, ray);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericRay<DATA_TYPE, SIZE>& ray) {
		return GMath::Sqr(SignedDistance(sph, ray));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return DistanceSquared(sph, ray);
	}


	// ray - axis aligned box (2D)
	template <typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericRay<DATA_TYPE, 2>& ray, const GGenericAABox<DATA_TYPE, 2>& aab){

		DATA_TYPE prj = Dot(aab.Center() - ray.Origin(), ray.Direction());
		#define UPDATE_DISTANCE \
			if (Dot(centerToNear, tmpPoint - nearestPoint) > 0) { \
				tmpDist = Distance(tmpPoint, ray); \
				if (tmpDist > sDist) \
					sDist = tmpDist; \
			}

		if (prj <= 0)
			return SignedDistance(ray.Origin(), aab);
		else {
			GPoint<DATA_TYPE, 2> nearestPoint = ray.Origin() + ray.Direction() * prj;
			DATA_TYPE sDist = SignedDistance(nearestPoint, aab);
			if (sDist >= 0)
				return sDist;
			else {
				GVect<DATA_TYPE, 2> centerToNear = nearestPoint - aab.Center();
				// if nearest point passes through center, we can choose as concordance direction the
				// perpendicular to ray direction; NB: it's important to consider the relative distance
				// centerToNear respect to the box dimensions, to avoid numerical instabilities
				DATA_TYPE maxDim = GMath::Max(aab.Dimension(G_X), aab.Dimension(G_Y)) / (DATA_TYPE)2;
				if (centerToNear.LengthSquared() / (maxDim * maxDim) <= G_EPSILON * G_EPSILON)
					centerToNear.Set(ray.Direction()[G_Y], -ray.Direction()[G_X]);

				DATA_TYPE tmpDist, sDist = -1;
				GPoint<DATA_TYPE, 2> tmpPoint;
				// distance is now the quantity the ray has to move along centerToNear direction to exit
				// the box
				tmpPoint = aab.Min();
				UPDATE_DISTANCE
				tmpPoint = aab.Max();
				UPDATE_DISTANCE
				tmpPoint.Set(aab.Min()[G_X], aab.Max()[G_Y]);
				UPDATE_DISTANCE
				tmpPoint.Set(aab.Max()[G_X], aab.Min()[G_Y]);
				UPDATE_DISTANCE
				return -sDist;
			}
		}
		#undef UPDATE_DISTANCE
	}

	// ray - axis aligned box (3D)
	template <typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericRay<DATA_TYPE, 3>& ray, const GGenericAABox<DATA_TYPE, 3>& aab) {

		DATA_TYPE prj = Dot(aab.Center() - ray.Origin(), ray.Direction());
		#define UPDATE_CONCORDE_DISTANCE \
			if (Dot(centerToNear, tmpPoint - nearestPoint) > 0) { \
				tmpDist = Distance(tmpPoint, ray); \
				if (tmpDist > sDist) \
					sDist = tmpDist; \
			}

		#define UPDATE_DISTANCE \
			tmpDist = Distance(tmpPoint, ray); \
			if (tmpDist > sDist) \
				sDist = tmpDist;

		if (prj <= 0)
			return SignedDistance(ray.Origin(), aab);
		else {
			GPoint<DATA_TYPE, 3> nearestPoint = ray.Origin() + ray.Direction() * prj;
			DATA_TYPE sDist = SignedDistance(nearestPoint, aab);
			if ((sDist >= 0) || (GMath::Abs(ray.Direction()[G_X]) <= G_EPSILON) ||
				(GMath::Abs(ray.Direction()[G_Y]) <= G_EPSILON) || (GMath::Abs(ray.Direction()[G_Z]) <= G_EPSILON))
				return sDist;
			else {
				DATA_TYPE tmpDist, sDist = -1;
				GPoint<DATA_TYPE, 3> tmpPoint;
				GVect<DATA_TYPE, 3> centerToNear = nearestPoint - aab.Center();
				// point passes through center; NB: it's important to consider the relative distance
				// centerToNear respect to the box dimensions, to avoid numerical instabilities
				DATA_TYPE maxDim = GMath::Max(aab.Dimension(G_X), aab.Dimension(G_Y), aab.Dimension(G_Z)) / (DATA_TYPE)2;
				if (centerToNear.LengthSquared() / (maxDim * maxDim) <= G_EPSILON * G_EPSILON) {
					tmpPoint = aab.Min();
					UPDATE_DISTANCE
					tmpPoint = aab.Max();
					UPDATE_DISTANCE
					tmpPoint.Set(aab.Min()[G_X], aab.Min()[G_Y], aab.Max()[G_Z]);
					UPDATE_DISTANCE
					tmpPoint.Set(aab.Min()[G_X], aab.Max()[G_Y], aab.Max()[G_Z]);
					UPDATE_DISTANCE
					tmpPoint.Set(aab.Min()[G_X], aab.Max()[G_Y], aab.Min()[G_Z]);
					UPDATE_DISTANCE
					tmpPoint.Set(aab.Max()[G_X], aab.Min()[G_Y], aab.Min()[G_Z]);
					UPDATE_DISTANCE
					tmpPoint.Set(aab.Max()[G_X], aab.Min()[G_Y], aab.Max()[G_Z]);
					UPDATE_DISTANCE
					tmpPoint.Set(aab.Max()[G_X], aab.Max()[G_Y], aab.Min()[G_Z]);
					UPDATE_DISTANCE
				}
				else {
					// distance is now the quantity the ray has to move along centerToNear direction to exit
					// the box
					tmpPoint = aab.Min();
					UPDATE_CONCORDE_DISTANCE
					tmpPoint = aab.Max();
					UPDATE_CONCORDE_DISTANCE
					tmpPoint.Set(aab.Min()[G_X], aab.Min()[G_Y], aab.Max()[G_Z]);
					UPDATE_CONCORDE_DISTANCE
					tmpPoint.Set(aab.Min()[G_X], aab.Max()[G_Y], aab.Max()[G_Z]);
					UPDATE_CONCORDE_DISTANCE
					tmpPoint.Set(aab.Min()[G_X], aab.Max()[G_Y], aab.Min()[G_Z]);
					UPDATE_CONCORDE_DISTANCE
					tmpPoint.Set(aab.Max()[G_X], aab.Min()[G_Y], aab.Min()[G_Z]);
					UPDATE_CONCORDE_DISTANCE
					tmpPoint.Set(aab.Max()[G_X], aab.Min()[G_Y], aab.Max()[G_Z]);
					UPDATE_CONCORDE_DISTANCE
					tmpPoint.Set(aab.Max()[G_X], aab.Max()[G_Y], aab.Min()[G_Z]);
					UPDATE_CONCORDE_DISTANCE
				}
				return -sDist;
			}
		}
		#undef UPDATE_CONCORDE_DISTANCE
		#undef UPDATE_DISTANCE
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GGenericRay<DATA_TYPE, SIZE>& ray){
		return SignedDistance(ray, aab);
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericAABox<DATA_TYPE, SIZE>& aab){
		return GMath::Abs(SignedDistance(ray, aab));
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GGenericRay<DATA_TYPE, SIZE>& ray){
		return Distance(ray, aab);
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericAABox<DATA_TYPE, SIZE>& aab){
		return GMath::Sqr(SignedDistance(ray, aab));
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GGenericRay<DATA_TYPE, SIZE>& ray){
		return DistanceSquared(ray, aab);
	}


	// ray - object oriented box (2D & 3D)
	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericOOBox<DATA_TYPE, SIZE>& oob){

		GVect<DATA_TYPE, SIZE> tmpDirection, deltaOrg;
		GPoint<DATA_TYPE, SIZE> tmpOrigin;

		// transform ray into box frame
		deltaOrg = ray.Origin() - oob.Center();
		for (GUInt32 i = 0; i < SIZE; i++) {
			tmpDirection[i] = Dot(ray.Direction(), oob.Axis(i));
			tmpOrigin[i] = Dot(deltaOrg, oob.Axis(i));
		}
		GGenericRay<DATA_TYPE, SIZE> tmpRay(tmpOrigin, tmpDirection);
		// after transformation box is axis aligned, centered at axes origin
		GGenericAABox<DATA_TYPE, SIZE> tmpBox(-oob.HalfDimensions(), oob.HalfDimensions());
		return SignedDistance(tmpRay, tmpBox);
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericRay<DATA_TYPE, SIZE>& ray){
		return SignedDistance(ray, oob);
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericOOBox<DATA_TYPE, SIZE>& oob){
		return GMath::Abs(SignedDistance(ray, oob));
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericRay<DATA_TYPE, SIZE>& ray){
		return Distance(ray, oob);
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericOOBox<DATA_TYPE, SIZE>& oob){
		return GMath::Sqr(SignedDistance(ray, oob));
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericRay<DATA_TYPE, SIZE>& ray){
		return DistanceSquared(ray, oob);
	}


	// sphere - point
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GPoint<DATA_TYPE, SIZE>& pt) {
		return (Length(pt - sph.Center()) - sph.Radius());
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return SignedDistance(sph, pt);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GPoint<DATA_TYPE, SIZE>& pt) {
		return GMath::Abs(SignedDistance(sph, pt));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return Distance(sph, pt);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GPoint<DATA_TYPE, SIZE>& pt) {
		return (GMath::Sqr(SignedDistance(sph, pt)));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return DistanceSquared(sph, pt);
	}


	// distance object oriented box - point (2D)
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericOOBox<DATA_TYPE, 2>& oob, const GPoint<DATA_TYPE, 2>& pt) {

		GGenericAABox<DATA_TYPE, 2> tmpBox(-oob.HalfDimensions(), oob.HalfDimensions());
		// make the point be in oob coordinate system
		GVect<DATA_TYPE, 2> deltaCenters(pt - oob.Center());
		GPoint<DATA_TYPE, 2> tmpPt(Dot(deltaCenters, oob.Axis(G_X)), Dot(deltaCenters, oob.Axis(G_Y)));

		return SignedDistance(tmpBox, tmpPt);
	}

	// distance object oriented box - point (3D)
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericOOBox<DATA_TYPE, 3>& oob, const GPoint<DATA_TYPE, 3>& pt) {

		GGenericAABox<DATA_TYPE, 3> tmpBox(-oob.HalfDimensions(), oob.HalfDimensions());
		// make the point be in oob coordinate system
		GVect<DATA_TYPE, 3> deltaCenters(pt - oob.Center());
		GPoint<DATA_TYPE, 3> tmpPt(Dot(deltaCenters, oob.Axis(G_X)), Dot(deltaCenters, oob.Axis(G_Y)),
								   Dot(deltaCenters, oob.Axis(G_Z)));

		return SignedDistance(tmpBox, tmpPt);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericOOBox<DATA_TYPE, SIZE>& oob) {
		return SignedDistance(oob, pt);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GPoint<DATA_TYPE, SIZE>& pt) {
		return GMath::Abs(SignedDistance(oob, pt));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericOOBox<DATA_TYPE, SIZE>& oob) {
		return Distance(oob, pt);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GPoint<DATA_TYPE, SIZE>& pt) {
		return GMath::Sqr(SignedDistance(oob, pt));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericOOBox<DATA_TYPE, SIZE>& oob) {
		return DistanceSquared(oob, pt);
	}


	// sphere - sphere
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericSphere<DATA_TYPE, SIZE>& sph1, const GGenericSphere<DATA_TYPE, SIZE>& sph2) {
		return (Length(sph1.Center() - sph2.Center()) - (sph1.Radius() + sph2.Radius()));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericSphere<DATA_TYPE, SIZE>& sph1, const GGenericSphere<DATA_TYPE, SIZE>& sph2) {
		return GMath::Abs(SignedDistance(sph1, sph2));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericSphere<DATA_TYPE, SIZE>& sph1, const GGenericSphere<DATA_TYPE, SIZE>& sph2) {
		return GMath::Sqr(SignedDistance(sph1, sph2));
	}

	// axis aligned box - point (2D)
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GPoint<DATA_TYPE, 2>& pt, const GGenericAABox<DATA_TYPE, 2>& aab) {

		GVect<DATA_TYPE, 2> deltaCenter = pt - aab.Center();
		DATA_TYPE distX = GMath::Abs(deltaCenter[G_X]) - aab.HalfDimension(G_X);
		DATA_TYPE distY = GMath::Abs(deltaCenter[G_Y]) - aab.HalfDimension(G_Y);
		// if both distances are negative, then Distance = Max(distX, distY)
		// if one distance is negative, then Distance = the positive one
		// else Arvo's algorithm
		if (distX <= 0) {
			if (distY <= 0)
				return (GMath::Max(distX, distY));
			else
				return distY;
		}
		else {
			if (distY <= 0)
				return distX;
			else
				// Arvo's algorithm
				return GMath::Sqrt((distX * distX) + (distY * distY));
		}
	}

	// axis aligned box - point (3D)
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GPoint<DATA_TYPE, 3>& pt, const GGenericAABox<DATA_TYPE, 3>& aab) {

		GVect<DATA_TYPE, 3> deltaCenter = pt - aab.Center();
		DATA_TYPE distX = GMath::Abs(deltaCenter[G_X]) - aab.HalfDimension(G_X);
		DATA_TYPE distY = GMath::Abs(deltaCenter[G_Y]) - aab.HalfDimension(G_Y);
		DATA_TYPE distZ = GMath::Abs(deltaCenter[G_Z]) - aab.HalfDimension(G_Z);
		// if 2 distances are negative, then Distance = the positive one
		// if 3 distances are negative, then Distance = Max(distX, distY, distZ)
		// else Arvo's algorithm
		if (distX <= 0) {
			if (distY <= 0) {
				if (distZ <= 0)
					return (GMath::Max(distX, distY, distZ));
				else
					return distZ;
			}
			// distX < 0, distY > 0
			else {
				if (distZ <= 0)
					return distY;
				else
					return GMath::Sqrt((distY * distY) + (distZ * distZ));
			}
		}
		// distX > 0
		else {
			if (distY <= 0) {
				// distY < 0
				if (distZ <= 0)
					return distX;
				else
					return GMath::Sqrt((distX * distX) + (distZ * distZ));
			}
			// distY > 0
			else {
				if (distZ <= 0)
					return GMath::Sqrt((distX * distX) + (distY * distY));
				else
					return GMath::Sqrt((distX * distX) + (distY * distY) + (distZ * distZ));
			}
		}
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GPoint<DATA_TYPE, SIZE>& pt) {
		return SignedDistance(pt, aab);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericAABox<DATA_TYPE, SIZE>& aab) {
		return GMath::Abs(SignedDistance(pt, aab));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GPoint<DATA_TYPE, SIZE>& pt) {
		return Distance(pt, aab);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GPoint<DATA_TYPE, SIZE>& pt, const GGenericAABox<DATA_TYPE, SIZE>& aab) {
		return GMath::Sqr(SignedDistance(pt, aab));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GPoint<DATA_TYPE, SIZE>& pt) {
		return DistanceSquared(pt, aab);
	}

	// axis aligned box - axis aligned box (2D)
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericAABox<DATA_TYPE, 2>& aab1, const GGenericAABox<DATA_TYPE, 2>& aab2) {

		GVect<DATA_TYPE, 2> deltaCenter = aab1.Center() - aab2.Center();
		DATA_TYPE distX = GMath::Abs(deltaCenter[G_X]) - (aab1.HalfDimension(G_X) + aab2.HalfDimension(G_X));
		DATA_TYPE distY = GMath::Abs(deltaCenter[G_Y]) - (aab1.HalfDimension(G_Y) + aab2.HalfDimension(G_Y));
		// if both distances are negative, then Distance = Max(distX, distY)
		// if one distance is negative, then Distance = the positive one
		// else Arvo's algorithm
		if (distX <= 0) {
			if (distY <= 0)
				return (GMath::Max(distX, distY));
			else
				return distY;
		}
		else {
			if (distY <= 0)
				return distX;
			else
				// Arvo's algorithm
				return GMath::Sqrt((distX * distX) + (distY * distY));
		}
	}

	// axis aligned box - axis aligned box (3D)
	template<typename DATA_TYPE>
	DATA_TYPE SignedDistance(const GGenericAABox<DATA_TYPE, 3>& aab1, const GGenericAABox<DATA_TYPE, 3>& aab2) {

		GVect<DATA_TYPE, 3> deltaCenter = aab1.Center() - aab2.Center();
		DATA_TYPE distX = GMath::Abs(deltaCenter[G_X]) - (aab1.HalfDimension(G_X) + aab2.HalfDimension(G_X));
		DATA_TYPE distY = GMath::Abs(deltaCenter[G_Y]) - (aab1.HalfDimension(G_Y) + aab2.HalfDimension(G_Y));
		DATA_TYPE distZ = GMath::Abs(deltaCenter[G_Z]) - (aab1.HalfDimension(G_Z) + aab2.HalfDimension(G_Z));
		// if 2 distances are negative, then Distance = the positive one
		// if 3 distances are negative, then Distance = Max(distX, distY, distZ)
		// else Arvo's algorithm
		if (distX <= 0) {
			if (distY <= 0) {
				if (distZ <= 0)
					return (GMath::Max(distX, distY, distZ));
				else
					return distZ;
			}
			// distX < 0, distY > 0
			else {
				if (distZ <= 0)
					return distY;
				else
					return GMath::Sqrt((distY * distY) + (distZ * distZ));
			}
		}
		// distX > 0
		else {
			if (distY <= 0) {
				// distY < 0
				if (distZ <= 0)
					return distX;
				else
					return GMath::Sqrt((distX * distX) + (distZ * distZ));
			}
			// distY > 0
			else {
				if (distZ <= 0)
					return GMath::Sqrt((distX * distX) + (distY * distY));
				else
					return GMath::Sqrt((distX * distX) + (distY * distY) + (distZ * distZ));
			}
		}
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericAABox<DATA_TYPE, SIZE>& aab1, const GGenericAABox<DATA_TYPE, SIZE>& aab2) {
		return GMath::Abs(SignedDistance(aab1, aab2));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericAABox<DATA_TYPE, SIZE>& aab1, const GGenericAABox<DATA_TYPE, SIZE>& aab2) {
		return GMath::Sqr(SignedDistance(aab1, aab2));
	}


	// sphere - axis aligned box
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericAABox<DATA_TYPE, SIZE>& aab) {
		return (SignedDistance(aab, sph.Center()) - sph.Radius());
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return SignedDistance(sph, aab);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return GMath::Abs(SignedDistance(sph, aab));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericAABox<DATA_TYPE, SIZE>& aab) {
		return Distance(aab, sph);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return GMath::Sqr(SignedDistance(sph, aab));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericAABox<DATA_TYPE, SIZE>& aab) {
		return DistanceSquared(aab, sph);
	}

	// sphere - object oriented box
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericOOBox<DATA_TYPE, SIZE>& oob) {
		return (SignedDistance(oob, sph.Center()) - sph.Radius());
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE SignedDistance(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return SignedDistance(sph, oob);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return GMath::Abs(SignedDistance(sph, oob));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Distance(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericOOBox<DATA_TYPE, SIZE>& oob) {
		return Distance(oob, sph);
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericSphere<DATA_TYPE, SIZE>& sph) {
		return GMath::Sqr(SignedDistance(sph, oob));
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE DistanceSquared(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericOOBox<DATA_TYPE, SIZE>& aab) {
		return DistanceSquared(aab, sph);
	}

};	// end namespace Amanith

#endif
