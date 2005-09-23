/****************************************************************************
** $file: amanith/geometry/gintersect.h   0.1.1.0   edited Sep 24 08:00
**
** Basic intersect query functions for every simple geometry
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

#ifndef GINTERSECT_H
#define GINTERSECT_H

#include "gdistance.h"
#include "glineseg.h"
#include "gplane.h"
#include "goobox.h"
#include "gaabox.h"
#include "gsphere.h"

/*!
	\file gintersect.h
	\brief Intersect queries for all geometric entities (boxes, sphere and so on).
*/

namespace Amanith {

	#define NO_SOLUTIONS		0
	#define MULTIPLE_SOLUTIONS	1
	#define SINGLE_SOLUTION		2
	#define INFINITE_SOLUTIONS	4
	#define COINCIDENT_SHAPES	8
	#define INCLUDED_SHAPE		16
	#define TANGENT_SHAPES		32

	// intersection info structure
	template<typename DATA_TYPE, GUInt32 SIZE>
	struct GGenericIntersectInfo {
		GInt32 Flags;
		GPoint<DATA_TYPE, SIZE> IntersectionPoint;
		GVect<DATA_TYPE, SIZE> IntersectionNormal;
	};

	// official types
	typedef GGenericIntersectInfo<GReal, 2> GIntersectInfo2;
	typedef GGenericIntersectInfo<GReal, 3> GIntersectInfo3;


	// ray - ray (2D) full intersection test (low level stuff)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 2>& Ray1, const GGenericRay<DATA_TYPE, 2>& Ray2,
					GVect<DATA_TYPE, 2>& DiffOrigins, DATA_TYPE LocalParameters[2],	GUInt32& IntersectionFlag) {

		G_ASSERT(LocalParameters != NULL);

		DATA_TYPE Det = Ray2.Direction()[G_X] * Ray1.Direction()[G_Y] - Ray2.Direction()[G_Y] * Ray1.Direction()[G_X];
		DiffOrigins = Ray2.Origin() - Ray1.Origin();

		if (GMath::Abs(Det) > G_EPSILON) {
			// lines intersect in a single point. Return both s and t values for
			// use by calling functions
			DATA_TYPE InvDet = (DATA_TYPE)1 / Det;
			IntersectionFlag = SINGLE_SOLUTION;
			LocalParameters[0] = (Ray2.Direction()[G_X] * DiffOrigins[G_Y] - Ray2.Direction()[G_Y] * DiffOrigins[G_X]) * InvDet;
			LocalParameters[1] = (Ray1.Direction()[G_X] * DiffOrigins[G_Y] - Ray1.Direction()[G_Y] * DiffOrigins[G_X]) * InvDet;
			return G_TRUE;
		}
		else {
			// lines are parallel
			Det = Ray1.Direction()[G_X] * DiffOrigins[G_Y] - Ray1.Direction()[G_Y] * DiffOrigins[G_X];
			if (GMath::Abs(Det) > G_EPSILON) {
				// lines are disjoint
				IntersectionFlag = NO_SOLUTIONS;
				return G_FALSE;
			}
			else {
				// lines are the same (collinear)
				IntersectionFlag = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
				return G_TRUE;
			}
		}
	}

	// ray - ray (3D) full intersection test (low level stuff)
	// Adapted from Gernot Hoffmann "Distance between Line Segments"
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 3>& Ray1, const GGenericRay<DATA_TYPE, 3>& Ray2,
					GVect<DATA_TYPE, 3>& DiffOrigins, DATA_TYPE LocalParameters[2],	GUInt32& IntersectionFlag) {

		G_ASSERT(LocalParameters != NULL);

		GVect<DATA_TYPE, 3> h(Cross(Ray1.Direction(), Ray2.Direction()));
		// rays are parallel
		if (h.LengthSquared() <= G_EPSILON * G_EPSILON) {
			IntersectionFlag = NO_SOLUTIONS;
			return G_FALSE;
		}
		// calculate unsigned distance
		DATA_TYPE dist = GMath::Abs(Dot(h, (const GVect<DATA_TYPE, 3>&)Ray1.Origin()) - Dot(h, (const GVect<DATA_TYPE, 3>&)Ray2.Origin()));
		// in this case rays are askew and not coplanar. So they are not intersecting
		if (dist > G_EPSILON) {
			IntersectionFlag = NO_SOLUTIONS;
			return G_FALSE;
		}

		DATA_TYPE a11, a12, a22, b1, b2, D0, D1, D2;

		DiffOrigins = Ray2.Origin() - Ray1.Origin();

		a11 = Ray1.Direction().LengthSquared();
		a12 = -Dot(Ray1.Direction(), Ray2.Direction());
		a22 = Ray2.Direction().LengthSquared();
		// Cramer determinants
		D0 = a11 * a22 - a12 * a12;
		if (GMath::Abs(D0) > G_EPSILON) {
			b1 = Dot(Ray1.Direction(), DiffOrigins);
			b2 = -Dot(Ray2.Direction(), DiffOrigins);
			D1 = b1 * a22 - b2 * a12;
			D2 = -b1 * a12 + b2 * a11;
			// incident rays
			IntersectionFlag = SINGLE_SOLUTION;
			LocalParameters[0] = D1 / D0;
			LocalParameters[1] = D2 / D0;
			return G_TRUE;
		}
		// collinear rays
		IntersectionFlag = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
		return G_TRUE;
	}

	// ray - ray (2D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 2>& ray1, const GGenericRay<DATA_TYPE, 2>& ray2,
					GUInt32& Flags,	DATA_TYPE LocalParameters[2]) {

		G_ASSERT(LocalParameters != NULL);
		GVect<DATA_TYPE, 2> diffOrg;

		return Intersect(ray1, ray2, diffOrg, LocalParameters, Flags);
	}

	// ray - ray (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 3>& ray1, const GGenericRay<DATA_TYPE, 3>& ray2,
					GUInt32& Flags,	DATA_TYPE LocalParameters[2]) {

		G_ASSERT(LocalParameters != NULL);
		GVect<DATA_TYPE, 3> diffOrg;

		return Intersect(ray1, ray2, diffOrg, LocalParameters, Flags);
	}

	// ray - segment (2D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 2>& ray, const GLineSeg<DATA_TYPE, 2>& seg,
					GUInt32& Flags,	DATA_TYPE LocalParameters[2]) {

		G_ASSERT(LocalParameters != NULL);

		GBool fullTest;
		GVect<DATA_TYPE, 2> diffOrg;
		DATA_TYPE localParams[2], dirAccordance, prj0, prj1;
		GUInt32 flag;
		
		fullTest = Intersect(ray, seg, diffOrg, localParams, flag);
		if (fullTest) {
			// ok, intersection has been possible
			if (flag & SINGLE_SOLUTION) {
				// check if intersection is "inside" segment
				if ((localParams[1] > 1 + G_EPSILON) || (localParams[0] < -G_EPSILON) || (localParams[1] < -G_EPSILON)) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}	
				Flags = SINGLE_SOLUTION;
				LocalParameters[0] = localParams[0];
				//LocalParameters[1] = localParams[1];
				return G_TRUE;
			}
			// ray and segment are parts of the same line
			else {
				// first check if directions are the same (dir1 dot dir2 > 0) or opposite (dir1 dot dir2 < 0)
				dirAccordance = Dot(ray.Direction(), seg.Direction());
				if (dirAccordance > 0) {
					prj0 = Dot(diffOrg, ray.Direction());
					prj1 = prj0 + dirAccordance;
				}
				else {
					prj1 = Dot(diffOrg, ray.Direction());
					prj0 = prj1 + dirAccordance;
				}
				// entities disjointed
				if ((prj0 < 0) && (prj1 < 0)) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}
				// full-intersection
				if (prj0 >= 0) {
					Flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES | INCLUDED_SHAPE;
					// in this case parameters are referred both to ray
					DATA_TYPE rayInvDirSqr = (DATA_TYPE)1 / ray.Direction().LengthSquared();
					LocalParameters[0] = prj0 * rayInvDirSqr;
					LocalParameters[1] = prj1 * rayInvDirSqr;
				}
				else
				// semi-intersection
				if (prj1 > 0) {
					Flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
					// in this case parameters are referred both to ray
					LocalParameters[0] = 0;
					LocalParameters[1] = prj1 / ray.Direction().LengthSquared();
				}
				else {
					// single point overlap
					Flags = SINGLE_SOLUTION | COINCIDENT_SHAPES;
					LocalParameters[0] = 0;
					//if (dirAccordance > 0)
					//	LocalParameters[1] = 1;
					//else
					//	LocalParameters[1] = 0;
				}
				return G_TRUE;
			}
		}
		else
			return G_FALSE;
	}

	// segment - ray (2D)
	// NB: ray MUST be normalized
	template<typename DATA_TYPE>
	inline GBool Intersect(const GLineSeg<DATA_TYPE, 2>& seg, const GGenericRay<DATA_TYPE, 2>& ray,
						   GUInt32& Flags, DATA_TYPE LocalParameters[2]) {
		return Intersect(ray, seg, Flags, LocalParameters);
	}

	// ray - segment (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 3>& ray, const GLineSeg<DATA_TYPE, 3>& seg,
					GUInt32& Flags,	DATA_TYPE LocalParameters[2]) {

		G_ASSERT(LocalParameters != NULL);

		GBool fullTest;
		GVect<DATA_TYPE, 3> tmpDiff;
		DATA_TYPE localParams[2], dirAccordance, prj0, prj1;
		GUInt32 flag;
		
		fullTest = Intersect(ray, seg, tmpDiff, localParams, flag);
		if (fullTest) {
			// ok, intersection has been possible
			if (flag & SINGLE_SOLUTION) {
				// check if intersection is "inside" segment
				if ((localParams[1] > 1 + G_EPSILON) || (localParams[0] < -G_EPSILON) || (localParams[1] < -G_EPSILON)) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}
				Flags = SINGLE_SOLUTION;
				LocalParameters[0] = localParams[0];
				//LocalParameters[1] = localParams[1];
				return G_TRUE;
			}
			// ray and segment are parts of the same line
			else {
				// first check if directions are the same (dir1 dot dir2 > 0) or opposite (dir1 dot dir2 < 0)
				dirAccordance = Dot(ray.Direction(), seg.Direction());
				if (dirAccordance > 0) {
					prj0 = Dot(tmpDiff, ray.Direction());
					prj1 = prj0 + dirAccordance;
				}
				else {
					prj1 = Dot(tmpDiff, ray.Direction());
					prj0 = prj1 + dirAccordance;
				}
				// entities disjointed
				if ((prj0 < 0) && (prj1 < 0)) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}
				// full-intersection
				if (prj0 >= 0) {
					Flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES | INCLUDED_SHAPE;
					// in this case parameters are referred both to ray
					DATA_TYPE rayInvDirSqr = (DATA_TYPE)1 / ray.Direction().LengthSquared();
					LocalParameters[0] = prj0 * rayInvDirSqr;
					LocalParameters[1] = prj1 * rayInvDirSqr;
				}
				else
				// semi-intersection
				if (prj1 > 0) {
					Flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
					// in this case parameters are referred both to ray
					LocalParameters[0] = 0;
					LocalParameters[1] = prj1 / ray.Direction().LengthSquared();
				}
				else {
					// single point overlap
					Flags = SINGLE_SOLUTION | COINCIDENT_SHAPES;
					LocalParameters[0] = 0;
					//if (dirAccordance > 0)
					//	LocalParameters[1] = 1;
					//else
					//	LocalParameters[1] = 0;
				}
				return G_TRUE;
			}
		}
		else
			return G_FALSE;
	}

	// segment - ray (3D)
	// NB: ray MUST be normalized
	template<typename DATA_TYPE>
	inline GBool Intersect(const GLineSeg<DATA_TYPE, 3>& seg, const GGenericRay<DATA_TYPE, 3>& ray,
						   GUInt32& Flags, DATA_TYPE LocalParameters[2]) {
		return Intersect(ray, seg, Flags, LocalParameters);
	}

	// segment - segment (2D)
	// NB: LocalParameters refers to the first segment
	template<typename DATA_TYPE>
	GBool Intersect(const GLineSeg<DATA_TYPE, 2>& seg1, const GLineSeg<DATA_TYPE, 2>& seg2,
					GUInt32& Flags, DATA_TYPE LocalParameters[2]) {

		G_ASSERT(LocalParameters != NULL);

		GBool fullTest;
		GVect<DATA_TYPE, 2> tmpDiff;
		DATA_TYPE localParams[2], dirAccordance, prj0, prj1, seg1DirSqr, seg1DirLen;
		GUInt32 flag;
		
		fullTest = Intersect(seg1, seg2, tmpDiff, localParams, flag);
		if (fullTest) {
			// ok, intersection has been possible
			if (flag & SINGLE_SOLUTION) {
				// check if intersection is "inside" segments
				if ((localParams[0] < -G_EPSILON) || (localParams[0] > 1 + G_EPSILON) ||
					(localParams[1] < -G_EPSILON) || (localParams[1] > 1 + G_EPSILON)) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}
				LocalParameters[0] = localParams[0];
				//LocalParameters[1] = localParams[1];
				Flags = SINGLE_SOLUTION;
				return G_TRUE;
			}
			// segments are parts of the same line
			else {
				// first check if directions are the same (dir1 dot dir2 > 0) or opposite (dir1 dot dir2 < 0)
				dirAccordance = Dot(seg1.Direction(), seg2.Direction());
				if (dirAccordance > 0) {
					prj0 = Dot(tmpDiff, seg1.Direction());
					prj1 = prj0 + dirAccordance;
				}
				else {
					prj1 = Dot(tmpDiff, seg1.Direction());
					prj0 = prj1 + dirAccordance;
				}
				seg1DirSqr = seg1.Direction().LengthSquared();
				// entities disjointed
				if (prj1 < 0 || prj0 > seg1DirSqr) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}
				// now we are sure that there's an intersection
				if (prj1 > 0) {
					if (prj0 < seg1DirSqr) {
						seg1DirLen = (DATA_TYPE)1 / seg1DirSqr;
						Flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
						// first intersection point
						if (prj0 < 0)
							LocalParameters[0] = 0;
						else
							LocalParameters[0] = prj0 * seg1DirLen;
						// second intersection point
						if (prj1 > seg1DirSqr) {
							LocalParameters[1] = 1;
							if (LocalParameters[0] == 0)
								Flags |= INCLUDED_SHAPE;
						}
						else
							LocalParameters[1] = prj1 * seg1DirLen;
					}
					else {
						Flags = SINGLE_SOLUTION | COINCIDENT_SHAPES;
						LocalParameters[0] = 1;
						//if (dirAccordance > 0)
						//	LocalParameters[1] = 0;
						//else
						//	LocalParameters[1] = 1;
					}
				}
				else {
					Flags = SINGLE_SOLUTION | COINCIDENT_SHAPES;
					LocalParameters[0] = 0;
					//if (dirAccordance > 0)
					//	LocalParameters[1] = 1;
					//else
					//	LocalParameters[1] = 0;
				}
				return G_TRUE;
			}
		}
		else {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
	}


	// segment - segment (3D)
	// NB: LocalParameters refers to the first segment
	template<typename DATA_TYPE>
	GBool Intersect(const GLineSeg<DATA_TYPE, 3>& seg1, const GLineSeg<DATA_TYPE, 3>& seg2,
					GUInt32& Flags, DATA_TYPE LocalParameters[2]) {

		G_ASSERT(LocalParameters != NULL);

		GBool fullTest;
		GVect<DATA_TYPE, 3> tmpDiff;
		DATA_TYPE localParams[2], dirAccordance, prj0, prj1, seg1DirSqr, seg1DirLen;
		GInt32 flag;
		
		fullTest = Intersect(seg1, seg2, tmpDiff, localParams, flag);
		if (fullTest) {
			// ok, intersection has been possible
			if (flag & SINGLE_SOLUTION) {
				// check if intersection is "inside" segments
				if ((localParams[0] < -G_EPSILON) || (localParams[0] > 1 + G_EPSILON) ||
					(localParams[1] < -G_EPSILON) || (localParams[1] > 1 + G_EPSILON)) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}
				Flags = SINGLE_SOLUTION;
				LocalParameters[0] = localParams[0];
				//LocalParameters[1] = localParams[1];
				return G_TRUE;
			}
			// segments are parts of the same line
			else {
				// first check if directions are the same (dir1 dot dir2 > 0) or opposite (dir1 dot dir2 < 0)
				dirAccordance = Dot(seg1.Direction(), seg2.Direction());
				if (dirAccordance > 0) {
					prj0 = Dot(tmpDiff, seg1.Direction());
					prj1 = prj0 + dirAccordance;
				}
				else {
					prj1 = Dot(tmpDiff, seg1.Direction());
					prj0 = prj1 + dirAccordance;
				}
				seg1DirSqr = seg1.Direction().LengthSquared();
				// entities disjointed
				if (prj1 < 0 || prj0 > seg1DirSqr) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}
				// now we are sure that there's an intersection
				if (prj1 > 0) {
					if (prj0 < seg1DirSqr) {
						seg1DirLen = (DATA_TYPE)1 / seg1DirSqr;
						Flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
						// first intersection point
						if (prj0 < 0)
							LocalParameters[0] = 0;
						else
							LocalParameters[0] = prj0 * seg1DirLen;
						// second intersection point
						if (prj1 > seg1DirSqr) {
							LocalParameters[1] = 1;
							if (LocalParameters[0] == 0)
								Flags |= INCLUDED_SHAPE;
						}
						else
							LocalParameters[1] = prj1 * seg1DirLen;
					}
					else {
						Flags = SINGLE_SOLUTION | COINCIDENT_SHAPES;
						LocalParameters[0] = 1;
						//if (dirAccordance > 0)
						//	LocalParameters[1] = 0;
						//else
						//	LocalParameters[1] = 1;
					}
				}
				else {
					Flags = SINGLE_SOLUTION | COINCIDENT_SHAPES;
					LocalParameters[0] = 0;
					//if (dirAccordance > 0)
					//	LocalParameters[1] = 1;
					//else
					//	LocalParameters[1] = 0;
				}
				return G_TRUE;
			}
		}
		else {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
	}

	// ray - plane (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 3>& ray, const GGenericPlane<DATA_TYPE>& Plane,
					GUInt32& Flags, DATA_TYPE& LocalParameter) {

		DATA_TYPE dirsProj = Dot(ray.Direction(), Plane.Normal());

		// ray is parallel (or it lies on the plane)
		if (GMath::Abs(dirsProj) <= G_EPSILON) {
			if (Distance(Plane, ray.Origin()) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES | INCLUDED_SHAPE;
				return G_TRUE;
			}
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}

		DATA_TYPE orgProj = -(Dot(Plane.Normal(), ray.Origin()) + Plane.Offset());
		DATA_TYPE t = orgProj / dirsProj;
		if (t < -G_EPSILON) {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
	
		Flags = SINGLE_SOLUTION;
		LocalParameter = t;
		return G_TRUE;
	}

	template<typename DATA_TYPE>
	inline GBool Intersect(const GGenericPlane<DATA_TYPE>& Plane, const GGenericRay<DATA_TYPE, 3>& ray,
						   GUInt32& Flags, DATA_TYPE& LocalParameter) {

	   return Intersect(ray, Plane, Flags, LocalParameter);
	}

	// segment - plane (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GLineSeg<DATA_TYPE, 3>& seg, const GGenericPlane<DATA_TYPE>& Plane,
					GUInt32& Flags, DATA_TYPE& LocalParameter) {

		GBool b;
		GUInt32 locFlags;
		DATA_TYPE solution;

		b = Intersect((const GGenericRay<DATA_TYPE, 3>&)seg, Plane, locFlags, solution);
		if (b) {
			if (locFlags & SINGLE_SOLUTION) {
				if (solution > 1 + G_EPSILON) {
					Flags = NO_SOLUTIONS;
					return G_FALSE;
				}
				Flags = SINGLE_SOLUTION;
				LocalParameter = solution;
				return G_TRUE;
			}
			// the only remain case is when segment lies on plane
			else {
				Flags = locFlags;
				return G_TRUE;
			}
		}
		else {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
	}

	template<typename DATA_TYPE>
	inline GBool Intersect(const GGenericPlane<DATA_TYPE>& Plane, const GLineSeg<DATA_TYPE, 3>& seg,
						   GUInt32& Flags, DATA_TYPE& LocalParameter) {
	   return Intersect(seg, Plane, Flags, LocalParameter);
	}

	// plane - plane
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericPlane<DATA_TYPE>& Plane1, const GGenericPlane<DATA_TYPE>& Plane2,
					GUInt32& Flags, GGenericRay<DATA_TYPE, 3>* IntersectionRay = NULL) {

		DATA_TYPE normalsDot = Dot(Plane1.Normal(), Plane2.Normal());
		DATA_TYPE distDiff;

		// planes are parallel (maybe distinct or coincident)
		if (GMath::Abs(normalsDot) >= 1 - G_EPSILON) {
			// same normals direction
			if (normalsDot >= 0)
				distDiff = Plane1.Offset() - Plane2.Offset();
			// opposite normals direction
			else
				distDiff = Plane1.Offset() + Plane2.Offset();

			// coplanar planes
			if (GMath::Abs(distDiff) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES | COINCIDENT_SHAPES;
				return G_TRUE;
			}
			// distinct parallel planes
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
		// if wanted, calculate ray of intersection
		if (IntersectionRay) {
			DATA_TYPE invDet = (DATA_TYPE)1 / ((DATA_TYPE)1 - GMath::Sqr(normalsDot));
			DATA_TYPE fC0 = (Plane1.Offset() - Plane2.Offset() * normalsDot) * invDet;
			DATA_TYPE fC1 = (Plane2.Offset() - Plane1.Offset() * normalsDot) * invDet;

			IntersectionRay->SetOrigin(fC0 * Plane1.Normal() + fC1 * Plane2.Normal());
			IntersectionRay->SetDirection(Cross(Plane1.Normal(), Plane2.Normal()));
			IntersectionRay->Normalize();
		}
		Flags = INFINITE_SOLUTIONS;
		return G_TRUE;
	}


	// plane - sphere (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericPlane<DATA_TYPE>& Plane, const GGenericSphere<DATA_TYPE, 3>& sph,
					GUInt32& Flags, GPoint<DATA_TYPE, 3>& Center, DATA_TYPE& Radius) {

		DATA_TYPE centerDist = SignedDistance(Plane, sph.Center());
		DATA_TYPE dist = GMath::Abs(centerDist) - sph.Radius();

		if (GMath::Abs(dist) <= G_EPSILON) {
			Flags = SINGLE_SOLUTION | TANGENT_SHAPES;
			if (centerDist < 0)
				Center = sph.Center() + sph.Radius() * Plane.Normal();
			else
				Center = sph.Center() - sph.Radius() * Plane.Normal();
			Radius = 0;
			return G_TRUE;
		}
		else
		if (dist < 0) {
			// plane intersect sphere, we must calculate circle of intersection
			Flags = INFINITE_SOLUTIONS;
			if (centerDist < 0)
				Center = sph.Center() + (-centerDist) * Plane.Normal();
			else
				Center = sph.Center() - centerDist * Plane.Normal();
			Radius = GMath::Sqrt(GMath::Sqr(sph.Radius()) - GMath::Sqr(centerDist));
			return G_TRUE;
		}
		else {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
	}

	template<typename DATA_TYPE>
	inline GBool Intersect(const GGenericSphere<DATA_TYPE, 3>& sph, const GGenericPlane<DATA_TYPE>& Plane,
						   GUInt32& Flags, GPoint<DATA_TYPE, 3>& Center, DATA_TYPE& Radius) {
	   return Intersect(Plane, sph, Flags, Center, Radius);
	}

	// plane - axis aligned box (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericPlane<DATA_TYPE>& Plane, const GGenericAABox<DATA_TYPE, 3>& aab) {

		DATA_TYPE signDist = SignedDistance(aab, FindNearestPoint(Plane, aab.Center()));
		if (signDist <= G_EPSILON)
			return G_TRUE;
		return G_FALSE;
	}

	template<typename DATA_TYPE>
	inline GBool Intersect(const GGenericAABox<DATA_TYPE, 3>& aab, const GGenericPlane<DATA_TYPE>& Plane) {
		return Intersect(Plane, aab);
	}

	// plane - object oriented box (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericPlane<DATA_TYPE>& Plane, const GGenericOOBox<DATA_TYPE, 3>& oob) {

		DATA_TYPE signDist = SignedDistance(oob, FindNearestPoint(Plane, oob.Center()));
		if (signDist <= G_EPSILON)
			return G_TRUE;
		return G_FALSE;
	}

	template<typename DATA_TYPE>
	inline GBool Intersect(const GGenericOOBox<DATA_TYPE, 3>& oob, const GGenericPlane<DATA_TYPE>& Plane) {
		return Intersect(Plane, oob);
	}

	// ray - sphere
	template<typename DATA_TYPE, GUInt32 SIZE>
	GBool Intersect(const GGenericRay<DATA_TYPE, SIZE>& ray, const GGenericSphere<DATA_TYPE, SIZE>& sph,
					GUInt32& Flags, DATA_TYPE LocalParameters[2]) {

		G_ASSERT(LocalParameters != NULL);

		DATA_TYPE locParams[2];
		GVect<DATA_TYPE, 2> diffOrg = ray.Origin() - sph.Center();
		DATA_TYPE a = ray.Direction().LengthSquared();
		DATA_TYPE b = Dot(diffOrg, ray.Direction());
		DATA_TYPE c = diffOrg.LengthSquared() - GMath::Sqr(sph.Radius());
		GInt32 rootsCount = GMath::QuadraticFormula(locParams[0], locParams[1], a, 2 * b, c);

		if (rootsCount == 0) {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
		else
		if (rootsCount == 1) {
			if (locParams[0] < -G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
			else {
				Flags = SINGLE_SOLUTION | TANGENT_SHAPES;
				LocalParameters[0] = locParams[0];
				return G_TRUE;
			}
		}
		// 2 solutions found, lets first sort them (ascending order)
		if (locParams[0] > locParams[1]) {
			DATA_TYPE swap = locParams[0];
			locParams[0] = locParams[1];
			locParams[1] = swap;
		}
		if (locParams[0] >= -G_EPSILON) {
			LocalParameters[0] = locParams[0];
			LocalParameters[1] = locParams[1];
			Flags = MULTIPLE_SOLUTIONS;
			return G_TRUE;
		}
		else
		if (locParams[1] >= -G_EPSILON) {
			LocalParameters[0] = locParams[1];
			Flags = SINGLE_SOLUTION;
			return G_TRUE;
		}
		else {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericSphere<DATA_TYPE, SIZE>& sph,
						   const GGenericRay<DATA_TYPE, SIZE>& ray,
						   GUInt32& Flags, DATA_TYPE LocalParameters[2]) {
		return Intersect(ray, sph, Flags, LocalParameters);
	}


	// segment - sphere
	template<typename DATA_TYPE, GUInt32 SIZE>
	GBool Intersect(const GLineSeg<DATA_TYPE, SIZE>& seg, const GGenericSphere<DATA_TYPE, SIZE>& sph,
					GUInt32& Flags, DATA_TYPE LocalParameters[2]) {

		G_ASSERT(LocalParameters != NULL);

		DATA_TYPE locParams[2];
		GVect<DATA_TYPE, 2> diffOrg = seg.Origin() - sph.Center();
		DATA_TYPE a = seg.Direction().LengthSquared();
		DATA_TYPE b = Dot(diffOrg, seg.Direction());
		DATA_TYPE c = diffOrg.LengthSquared() - GMath::Sqr(sph.Radius());
		GInt32 rootsCount = GMath::QuadraticFormula(locParams[0], locParams[1], a, 2 * b, c);

		if (rootsCount == 0) {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
		else
		if (rootsCount == 1) {
			// ray would be tangent, but segment do not touch sphere
			if (locParams[0] < -G_EPSILON || locParams[0] > 1 + G_EPSILON)  {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
			Flags = SINGLE_SOLUTION | TANGENT_SHAPES;
			LocalParameters[0] = locParams[0];
			return G_TRUE;
		}
		// 2 solutions found, lets first sort them (ascending order)
		if (locParams[0] > locParams[1]) {
			DATA_TYPE swap = locParams[0];
			locParams[0] = locParams[1];
			locParams[1] = swap;
		}

		if (locParams[0] > 1 + G_EPSILON) {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
		if (locParams[0] < -G_EPSILON) {
			if (locParams[1] < -G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
			else
			if (locParams[1] > 1 + G_EPSILON) {
				Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
				return G_FALSE;
			}
			else {
				LocalParameters[0] = locParams[1];
				Flags = SINGLE_SOLUTION;
				if (GMath::Abs(locParams[1] - 1) <= 2 * G_EPSILON)
					Flags |= INCLUDED_SHAPE;
				return G_TRUE;
			}
		}
		else {
			LocalParameters[0] = locParams[0];
			if (locParams[1] > 1 + G_EPSILON) {
				Flags = SINGLE_SOLUTION;
				if (GMath::Abs(locParams[0]) <= 2 * G_EPSILON)
					Flags |= INCLUDED_SHAPE;
			}
			else {
				Flags = MULTIPLE_SOLUTIONS;
				if (GMath::Abs(locParams[0]) <= 2 * G_EPSILON && GMath::Abs(locParams[1] - 1) <= 2 * G_EPSILON)
					Flags |= INCLUDED_SHAPE;
				LocalParameters[1] = locParams[1];
			}
			return G_TRUE;
		}
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GLineSeg<DATA_TYPE, SIZE>& seg,
						   GUInt32& Flags, DATA_TYPE LocalParameters[2]) {
		return Intersect(seg, sph, Flags, LocalParameters);
	}

	// ray - axis aligned box (2D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 2>& ray, const GGenericAABox<DATA_TYPE, 2>& aab,
					GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {

		G_ASSERT(LocalParameters != NULL);

		#define SORT_SOLUTIONS_CHOOSE_SIDE(side1, side2) \
			if (t1 > t2) { \
				tswap = t1; \
				t1 = t2; \
				t2 = tswap; \
				if (t1 >= 0) \
					tmpSide = side2; \
				else \
					tmpSide = side1; \
			} \
			else { \
				if (t1 >= 0) \
					tmpSide = side1; \
				else \
					tmpSide = side2; \
			}

		#define UPDATE_SOLUTIONS \
			if (t1 > tNear) { \
				tNear = t1; \
				if (t1 >= 0) \
					hittenSide = tmpSide; \
			} \
			if (t2 < tFar) { \
				tFar = t2; \
				if (t2 >= 0 && tNear < 0) \
					hittenSide = tmpSide; \
			}

		GVect<DATA_TYPE, 2> base0 = ray.Origin() - aab.Center();
		DATA_TYPE tNear = (DATA_TYPE)G_MIN_REAL;
		DATA_TYPE tFar = (DATA_TYPE)G_MAX_REAL;
		DATA_TYPE t1, t2, k, tswap;
		// assign values just to shut up compiler..
		GBoxSide hittenSide = G_RIGHTSIDE, tmpSide, tangentSide = G_RIGHTSIDE;

		// Kay and Kayjia slabs method is used to find intersection
		Flags = NO_SOLUTIONS;
		// X - plane
		if (GMath::Abs(ray.Direction()[G_X]) > G_EPSILON) {
			k = 1 / ray.Direction()[G_X];
			t1 = (-aab.HalfDimension(G_X) - base0[G_X]) * k;  // intersection with left plane
			t2 = (aab.HalfDimension(G_X) - base0[G_X]) * k;  // intersection with right plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_LEFTSIDE, G_RIGHTSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON)
				return G_FALSE;
		}
		else {
			// ray is parallel to the X planes, so if origin is not between the slabs then return false
			if ((ray.Origin()[G_X] > aab.Max()[G_X] + G_EPSILON) || (ray.Origin()[G_X] < aab.Min()[G_X] - G_EPSILON))
				return G_FALSE;
			if (GMath::Abs(ray.Origin()[G_X] - aab.Max()[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_RIGHTSIDE;
			}
			else
			if (GMath::Abs(ray.Origin()[G_X] - aab.Min()[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_LEFTSIDE;
			}
		}

		// Y - plane
		if (GMath::Abs(ray.Direction()[G_Y]) > G_EPSILON) {
			k = 1 / ray.Direction()[G_Y];
			t1 = (-aab.HalfDimension(G_Y) - base0[G_Y]) * k;  // intersection with bottom plane
			t2 = (aab.HalfDimension(G_Y) - base0[G_Y]) * k;  // intersection with top plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_BOTTOMSIDE, G_TOPSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}

			if (Flags & TANGENT_SHAPES) {
				FirstHittedSide = tangentSide;
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
			}
			else {
				FirstHittedSide = hittenSide;
				if (tNear < -G_EPSILON) {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = tFar;
				}
				else {
					Flags = MULTIPLE_SOLUTIONS;
					LocalParameters[0] = tNear;
					LocalParameters[1] = tFar;
				}
			}
			return G_TRUE;
		}
		else {
			// ray is parallel to the Y planes, so if origin is not between the slabs then return false
			if ((ray.Origin()[G_Y] > aab.Max()[G_Y] + G_EPSILON) || (ray.Origin()[G_Y] < aab.Min()[G_Y] - G_EPSILON))
				return G_FALSE;

			if (GMath::Abs(ray.Origin()[G_Y] - aab.Max()[G_Y]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				FirstHittedSide = G_TOPSIDE;
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
			}
			else
			if (GMath::Abs(ray.Origin()[G_Y] - aab.Min()[G_Y]) <= G_EPSILON) {
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				FirstHittedSide = G_BOTTOMSIDE;
			}
			// we are parallel inside
			else {
				FirstHittedSide = hittenSide;
				if (tNear < -G_EPSILON) {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = tFar;
				}
				else {
					Flags = MULTIPLE_SOLUTIONS;
					LocalParameters[0] = tNear;
					LocalParameters[1] = tFar;
				}
			}
			return G_TRUE;
		}
		#undef SORT_SOLUTIONS_CHOOSE_SIDE
		#undef UPDATE_SOLUTIONS
	}

	// ray - axis aligned box (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 3>& ray, const GGenericAABox<DATA_TYPE, 3>& aab,
					GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {

		G_ASSERT(LocalParameters != NULL);

		#define SORT_SOLUTIONS_CHOOSE_SIDE(side1, side2) \
			if (t1 > t2) { \
				tswap = t1; \
				t1 = t2; \
				t2 = tswap; \
				if (t1 >= 0) \
					tmpSide = side2; \
				else \
					tmpSide = side1; \
			} \
			else { \
				if (t1 >= 0) \
					tmpSide = side1; \
				else \
					tmpSide = side2; \
			}

		#define UPDATE_SOLUTIONS \
			if (t1 > tNear) { \
				tNear = t1; \
				if (t1 >= 0) \
					hittenSide = tmpSide; \
			} \
			if (t2 < tFar) { \
				tFar = t2; \
				if (t2 >= 0 && tNear < 0) \
					hittenSide = tmpSide; \
			}

		GVect<DATA_TYPE, 3> base0 = ray.Origin() - aab.Center();
		DATA_TYPE tNear = (DATA_TYPE)G_MIN_REAL;
		DATA_TYPE tFar = (DATA_TYPE)G_MAX_REAL;
		DATA_TYPE t1, t2, k, tswap;
		GBoxSide hittenSide, tmpSide, tangentSide;

		// Kay and Kayjia slabs method is used to find intersection
		Flags = NO_SOLUTIONS;

		// X - plane
		if (GMath::Abs(ray.Direction()[G_X]) > G_EPSILON) {
			k = 1 / ray.Direction()[G_X];
			t1 = (-aab.HalfDimension(G_X) - base0[G_X]) * k;  // intersection with left plane
			t2 = (aab.HalfDimension(G_X) - base0[G_X]) * k;  // intersection with right plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_LEFTSIDE, G_RIGHTSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON)
				return G_FALSE;
		}
		else {
			// ray is parallel to the X planes, so if origin is not between the slabs then return false
			if ((ray.Origin()[G_X] > aab.Max()[G_X] + G_EPSILON) || (ray.Origin()[G_X] < aab.Min()[G_X] - G_EPSILON))
				return G_FALSE;
			if (GMath::Abs(ray.Origin()[G_X] - aab.Max()[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_RIGHTSIDE;
			}
			else
			if (GMath::Abs(ray.Origin()[G_X] - aab.Min()[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_LEFTSIDE;
			}
		}

		// Y - plane
		if (GMath::Abs(ray.Direction()[G_Y]) > G_EPSILON) {
			k = 1 / ray.Direction()[G_Y];
			t1 = (-aab.HalfDimension(G_Y) - base0[G_Y]) * k;  // intersection with bottom plane
			t2 = (aab.HalfDimension(G_Y) - base0[G_Y]) * k;  // intersection with top plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_BOTTOMSIDE, G_TOPSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
		}
		else {
			// ray is parallel to the Y planes, so if origin is not between the slabs then return false
			if ((ray.Origin()[G_Y] > aab.Max()[G_Y] + G_EPSILON) || (ray.Origin()[G_Y] < aab.Min()[G_Y] - G_EPSILON))
				return G_FALSE;
			if (GMath::Abs(ray.Origin()[G_X] - aab.Max()[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_RIGHTSIDE;
			}
			else
			if (GMath::Abs(ray.Origin()[G_X] - aab.Min()[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_LEFTSIDE;
			}
		}
		// Z - plane
		if (GMath::Abs(ray.Direction()[G_Z]) > 2 * G_EPSILON) {
			k = 1 / ray.Direction()[G_Z];
			t1 = (-aab.HalfDimension(G_Z) - base0[G_Z]) * k;  // intersection with back plane
			t2 = (aab.HalfDimension(G_Z) - base0[G_Z]) * k;  // intersection with front plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_BACKSIDE, G_FRONTSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}

			if (Flags & TANGENT_SHAPES) {
				FirstHittedSide = tangentSide;
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
			}
			else {
				FirstHittedSide = hittenSide;
				if (tNear < -G_EPSILON) {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = tFar;
				}
				else {
					Flags = MULTIPLE_SOLUTIONS;
					LocalParameters[0] = tNear;
					LocalParameters[1] = tFar;
				}
			}
			return G_TRUE;
		}
		else {
			// ray is parallel to the Z planes, so if origin is not between the slabs then return false
			if ((ray.Origin()[G_Z] > aab.Max()[G_Z] + G_EPSILON) || (ray.Origin()[G_Z] < aab.Min()[G_Z] - G_EPSILON))
				return G_FALSE;

			if (GMath::Abs(ray.Origin()[G_Z] - aab.Max()[G_Z]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				FirstHittedSide = G_FRONTSIDE;
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
			}
			else
			if (GMath::Abs(ray.Origin()[G_Z] - aab.Min()[G_Z]) <= G_EPSILON) {
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				FirstHittedSide = G_BACKSIDE;
			}
			// we are parallel inside
			else {
				FirstHittedSide = hittenSide;
				if (tNear < -G_EPSILON) {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = tFar;
				}
				else {
					Flags = MULTIPLE_SOLUTIONS;
					LocalParameters[0] = tNear;
					LocalParameters[1] = tFar;
				}
			}
			return G_TRUE;
		}
		#undef SORT_SOLUTIONS_CHOOSE_SIDE
		#undef UPDATE_SOLUTIONS
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GGenericRay<DATA_TYPE, SIZE>& ray,
						   GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {
		return Intersect(ray, aab, Flags, LocalParameters, FirstHittedSide);
	}

	// segment - axis aligned box (2D & 3D)
	template<typename DATA_TYPE, GUInt32 SIZE>
	GBool Intersect(const GLineSeg<DATA_TYPE, SIZE>& seg, const GGenericAABox<DATA_TYPE, SIZE>& aab,
					GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {

		G_ASSERT(LocalParameters != NULL);

		GUInt32 locFlags;
		GBoxSide locBoxSize;
		DATA_TYPE locParams[2];
		GBool b;

		b = Intersect((const GGenericRay<DATA_TYPE, SIZE> &)seg, aab, locFlags, locParams, locBoxSize);
		if (b) {
			if (locFlags & SINGLE_SOLUTION) {
				if (locParams[0] > 1 + G_EPSILON) {
					Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
					return G_FALSE;
				}
				else {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = locParams[0];
					if (GMath::Abs(locParams[0] - 1) <= G_EPSILON)
						Flags |= INCLUDED_SHAPE;
					FirstHittedSide = locBoxSize;
					return G_TRUE;
				}
			}
			// the only remain cases are MULTIPLE_SOLUTIONS and INFINITE_SOLUTIONS; in these cases first
			// intersection is outside or at least on border
			else {
				// full cross intersection (or tangent)
				//    --------
				//  o-|------|->o
				//    --------
				if (locParams[1] <= 1 + G_EPSILON) {
					Flags = locFlags;
					LocalParameters[0] = locParams[0];
					LocalParameters[1] = locParams[1];
					if (GMath::Abs(locParams[0]) <= G_EPSILON && locParams[1] >= 1 - G_EPSILON)
						Flags |= INCLUDED_SHAPE;
					FirstHittedSide = locBoxSize;
					return G_TRUE;
				}
				else {
					//          --------
					//  o--->o  |      |
					//          --------
					if (locParams[0] > 1 + G_EPSILON) {
						Flags = NO_SOLUTIONS;
						return G_FALSE;
					}
					else {
						LocalParameters[0] = locParams[0];
						if (locFlags & MULTIPLE_SOLUTIONS)
							Flags = SINGLE_SOLUTION;
						else {
							if (GMath::Abs(LocalParameters[0] - 1) <= G_EPSILON)
								//   o----->o-------
								//          |      |
								//          --------
								Flags = SINGLE_SOLUTION;
							else {
								Flags = locFlags;
								//   o----------------->o           o----->o---
								//          |      |         or        |      |
								//          --------                   --------
								LocalParameters[1] = GMath::Min(locParams[1], (DATA_TYPE)1);
							}
						}
						FirstHittedSide = locBoxSize;
						return G_TRUE;
					}
				}
			}
		}
		else {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GLineSeg<DATA_TYPE, SIZE>& seg,
						   GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {
		return Intersect(seg, aab, Flags, LocalParameters, FirstHittedSide);
	}


	// ray - object oriented box (2D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 2>& ray, const GGenericOOBox<DATA_TYPE, 2>& oob,
					GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {

		G_ASSERT(LocalParameters != NULL);

		#define SORT_SOLUTIONS_CHOOSE_SIDE(side1, side2) \
			if (t1 > t2) { \
				tswap = t1; \
				t1 = t2; \
				t2 = tswap; \
				if (t1 >= 0) \
					tmpSide = side2; \
				else \
					tmpSide = side1; \
			} \
			else { \
				if (t1 >= 0) \
				tmpSide = side1; \
			else \
				tmpSide = side2; \
			}

		#define UPDATE_SOLUTIONS \
			if (t1 > tNear) { \
				tNear = t1; \
				if (t1 >= 0) \
					hittenSide = tmpSide; \
			} \
			if (t2 < tFar) { \
				tFar = t2; \
				if (t2 >= 0 && tNear < 0) \
					hittenSide = tmpSide; \
			}

		// first we have to transform ray into box coordinate system
		GVect<DATA_TYPE, 2> tmpDelta = ray.Origin() - oob.Center();
		GVect<DATA_TYPE, 2> base0(Dot(tmpDelta, oob.Axis(G_X)),
								  Dot(tmpDelta, oob.Axis(G_Y)));
		GVect<DATA_TYPE, 2> dir0(Dot(ray.Direction(), oob.Axis(G_X)),
								 Dot(ray.Direction(), oob.Axis(G_Y)));  // new (rotated) ray direction
		GPoint<DATA_TYPE, 2> org0 = oob.Center() + base0;  // new (rotated) ray origin
		GPoint<DATA_TYPE, 2> boxMax = oob.Center() + oob.HalfDimensions(); // box max extension point
		GPoint<DATA_TYPE, 2> boxMin = oob.Center() - oob.HalfDimensions(); // box min extension point
		DATA_TYPE tNear = (DATA_TYPE)G_MIN_REAL;
		DATA_TYPE tFar = (DATA_TYPE)G_MAX_REAL;
		DATA_TYPE t1, t2, k, tswap;
		GBoxSide hittenSide, tmpSide, tangentSide;

		// Kay and Kayjia slabs method is used to find intersection
		Flags = NO_SOLUTIONS;

		// X - plane
		if (GMath::Abs(dir0[G_X]) > G_EPSILON) {
			k = 1 / dir0[G_X];
			t1 = (-oob.HalfDimension(G_X) - base0[G_X]) * k;  // intersection with left plane
			t2 = (oob.HalfDimension(G_X) - base0[G_X]) * k;  // intersection with right plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_LEFTSIDE, G_RIGHTSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON)
				return G_FALSE;
		}
		else {
			// ray is parallel to the X planes, so if origin is not between the slabs then return false
			if ((org0[G_X] > boxMax[G_X] + G_EPSILON) || (org0[G_X] < boxMin[G_X] - G_EPSILON))
				return G_FALSE;
			if (GMath::Abs(org0[G_X] - boxMax[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_RIGHTSIDE;
			}
			else
			if (GMath::Abs(org0[G_X] - boxMin[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_LEFTSIDE;
			}
		}

		// Y - plane
		if (GMath::Abs(dir0[G_Y]) > G_EPSILON) {
			k = 1 / dir0[G_Y];
			t1 = (-oob.HalfDimension(G_Y) - base0[G_Y]) * k;  // intersection with bottom plane
			t2 = (oob.HalfDimension(G_Y) - base0[G_Y]) * k;  // intersection with top plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_BOTTOMSIDE, G_TOPSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}

			if (Flags & TANGENT_SHAPES) {
				FirstHittedSide = tangentSide;
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
			}
			else {
				FirstHittedSide = hittenSide;
				if (tNear < -G_EPSILON) {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = tFar;
				}
				else {
					Flags = MULTIPLE_SOLUTIONS;
					LocalParameters[0] = tNear;
					LocalParameters[1] = tFar;
				}
			}
			return G_TRUE;
		}
		else {
			// ray is parallel to the Y planes, so if origin is not between the slabs then return false
			if ((org0[G_Y] > boxMax[G_Y] + G_EPSILON) || (org0[G_Y] < boxMin[G_Y] - G_EPSILON))
				return G_FALSE;

			if (GMath::Abs(org0[G_Y] - boxMax[G_Y]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				FirstHittedSide = G_TOPSIDE;
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
			}
			else
			if (GMath::Abs(org0[G_Y] - boxMin[G_Y]) <= G_EPSILON) {
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				FirstHittedSide = G_BOTTOMSIDE;
			}
			// we are parallel inside
			else {
				FirstHittedSide = hittenSide;
				if (tNear < -G_EPSILON) {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = tFar;
				}
				else {
					Flags = MULTIPLE_SOLUTIONS;
					LocalParameters[0] = tNear;
					LocalParameters[1] = tFar;
				}
			}
			return G_TRUE;
		}
		#undef SORT_SOLUTIONS_CHOOSE_SIDE
		#undef UPDATE_SOLUTIONS
	}


	// ray - object oriented box (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericRay<DATA_TYPE, 3>& ray, const GGenericOOBox<DATA_TYPE, 3>& oob,
					GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {

		G_ASSERT(LocalParameters != NULL);

		#define SORT_SOLUTIONS_CHOOSE_SIDE(side1, side2) \
			if (t1 > t2) { \
				tswap = t1; \
				t1 = t2; \
				t2 = tswap; \
				if (t1 >= 0) \
					tmpSide = side2; \
				else \
					tmpSide = side1; \
			} \
			else { \
				if (t1 >= 0) \
					tmpSide = side1; \
				else \
					tmpSide = side2; \
			}

		#define UPDATE_SOLUTIONS \
			if (t1 > tNear) { \
				tNear = t1; \
				if (t1 >= 0) \
					hittenSide = tmpSide; \
			} \
			if (t2 < tFar) { \
				tFar = t2; \
				if (t2 >= 0 && tNear < 0) \
					hittenSide = tmpSide; \
			}

		// first we have to transform ray into box coordinate system
		GVect<DATA_TYPE, 3> tmpDelta = ray.Origin() - oob.Center();
		GVect<DATA_TYPE, 3> base0(Dot(tmpDelta, oob.Axis(G_X)),
								  Dot(tmpDelta, oob.Axis(G_Y)),
								  Dot(tmpDelta, oob.Axis(G_Z)));
		GVect<DATA_TYPE, 3> dir0(Dot(ray.Direction(), oob.Axis(G_X)),
								 Dot(ray.Direction(), oob.Axis(G_Y)),
								 Dot(ray.Direction(), oob.Axis(G_Z)));  // new (rotated) ray direction
		GPoint<DATA_TYPE, 3> org0 = oob.Center() + base0;  // new (rotated) ray origin
		GPoint<DATA_TYPE, 3> boxMax = oob.Center() + oob.HalfDimensions(); // box max extension point
		GPoint<DATA_TYPE, 3> boxMin = oob.Center() - oob.HalfDimensions(); // box min extension point
		DATA_TYPE tNear = (DATA_TYPE)G_MIN_REAL;
		DATA_TYPE tFar = (DATA_TYPE)G_MAX_REAL;
		DATA_TYPE t1, t2, k, tswap;
		GBoxSide hittenSide, tmpSide, tangentSide;

		// Kay and Kayjia slabs method is used to find intersection
		Flags = NO_SOLUTIONS;

		// X - plane
		if (GMath::Abs(dir0[G_X]) > G_EPSILON) {
			k = 1 / dir0[G_X];
			t1 = (-oob.HalfDimension(G_X) - base0[G_X]) * k;  // intersection with left plane
			t2 = (oob.HalfDimension(G_X) - base0[G_X]) * k;  // intersection with right plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_LEFTSIDE, G_RIGHTSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON)
				return G_FALSE;
		}
		else {
			// ray is parallel to the X planes, so if origin is not between the slabs then return false
			if ((org0[G_X] > boxMax[G_X] + G_EPSILON) || (org0[G_X] < boxMin[G_X] - G_EPSILON))
				return G_FALSE;
			if (GMath::Abs(org0[G_X] - boxMax[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_RIGHTSIDE;
			}
			else
			if (GMath::Abs(org0[G_X] - boxMin[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_LEFTSIDE;
			}
		}

		// Y - plane
		if (GMath::Abs(dir0[G_Y]) > G_EPSILON) {
			k = 1 / dir0[G_Y];
			t1 = (-oob.HalfDimension(G_Y) - base0[G_Y]) * k;  // intersection with bottom plane
			t2 = (oob.HalfDimension(G_Y) - base0[G_Y]) * k;  // intersection with top plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_BOTTOMSIDE, G_TOPSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
		}
		else {
			// ray is parallel to the Y planes, so if origin is not between the slabs then return false
			if ((org0[G_Y] > boxMax[G_Y] + G_EPSILON) || (org0[G_Y] < boxMin[G_Y] - G_EPSILON))
				return G_FALSE;
			if (GMath::Abs(org0[G_X] - boxMax[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_RIGHTSIDE;
			}
			else
			if (GMath::Abs(org0[G_X] - boxMin[G_X]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				tangentSide = G_LEFTSIDE;
			}
		}

		// Z - plane
		if (GMath::Abs(dir0[G_Z]) > G_EPSILON) {
			k = 1 / dir0[G_Z];
			t1 = (-oob.HalfDimension(G_Z) - base0[G_Z]) * k;  // intersection with back plane
			t2 = (oob.HalfDimension(G_Z) - base0[G_Z]) * k;  // intersection with front plane
			SORT_SOLUTIONS_CHOOSE_SIDE(G_BACKSIDE, G_FRONTSIDE)
			UPDATE_SOLUTIONS
			if (tNear > tFar || tFar < -G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
			if (Flags & TANGENT_SHAPES) {
				FirstHittedSide = tangentSide;
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
			}
			else {
				FirstHittedSide = hittenSide;
				if (tNear < -G_EPSILON) {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = tFar;
				}
				else {
					Flags = MULTIPLE_SOLUTIONS;
					LocalParameters[0] = tNear;
					LocalParameters[1] = tFar;
				}
			}
			return G_TRUE;
		}
		else {
			// ray is parallel to the Z planes, so if origin is not between the slabs then return false
			if ((org0[G_Z] > boxMax[G_Z] + G_EPSILON) || (org0[G_Z] < boxMin[G_Z] - G_EPSILON))
				return G_FALSE;

			if (GMath::Abs(org0[G_Z] - boxMax[G_Z]) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				FirstHittedSide = G_FRONTSIDE;
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
			}
			else
			if (GMath::Abs(org0[G_Z] - boxMin[G_Z]) <= G_EPSILON) {
				tNear = GMath::Max((DATA_TYPE)0, tNear);
				LocalParameters[0] = tNear;
				LocalParameters[1] = tFar;
				Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
				FirstHittedSide = G_BACKSIDE;
			}
			// we are parallel inside
			else {
				FirstHittedSide = hittenSide;
				if (tNear < -G_EPSILON) {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = tFar;
				}
				else {
					Flags = MULTIPLE_SOLUTIONS;
					LocalParameters[0] = tNear;
					LocalParameters[1] = tFar;
				}
			}
			return G_TRUE;
		}
		#undef SORT_SOLUTIONS_CHOOSE_SIDE
		#undef UPDATE_SOLUTIONS
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericRay<DATA_TYPE, SIZE>& ray,
						   GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {
		return Intersect(ray, oob, Flags, LocalParameters, FirstHittedSide);
	}

	// segment - object oriented box (2D & 3D)
	template<typename DATA_TYPE, GUInt32 SIZE>
	GBool Intersect(const GLineSeg<DATA_TYPE, SIZE>& seg, const GGenericOOBox<DATA_TYPE, SIZE>& oob,
					GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {

		G_ASSERT(LocalParameters != NULL);

		GUInt32 locFlags;
		GBoxSide locBoxSize;
		DATA_TYPE locParams[2];
		GBool b;

		b = Intersect((const GGenericRay<DATA_TYPE, SIZE> &)seg, oob, locFlags, locParams, locBoxSize);
		if (b) {
			if (locFlags & SINGLE_SOLUTION) {
				if (locParams[0] > 1 + G_EPSILON) {
					Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
					return G_FALSE;
				}
				else {
					Flags = SINGLE_SOLUTION;
					LocalParameters[0] = locParams[0];
					if (GMath::Abs(locParams[0] - 1) <= G_EPSILON)
						Flags |= INCLUDED_SHAPE;
					FirstHittedSide = locBoxSize;
					return G_TRUE;
				}
			}
			// the only remain cases are MULTIPLE_SOLUTIONS and INFINITE_SOLUTIONS; in these cases first
			// intersection is outside or at least on border
			else {
				// full cross intersection (or tangent)
				//    --------
				//  o-|------|->o
				//    --------
				if (locParams[1] <= 1 + G_EPSILON) {
					Flags = locFlags;
					LocalParameters[0] = locParams[0];
					LocalParameters[1] = locParams[1];
					if (GMath::Abs(locParams[0]) <= G_EPSILON && locParams[1] >= 1 - G_EPSILON)
						Flags |= INCLUDED_SHAPE;
					FirstHittedSide = locBoxSize;
					return G_TRUE;
				}
				else {
					//          --------
					//  o--->o  |      |
					//          --------
					if (locParams[0] > 1 + G_EPSILON) {
						Flags = NO_SOLUTIONS;
						return G_FALSE;
					}
					else {
						LocalParameters[0] = locParams[0];
						if (locFlags & MULTIPLE_SOLUTIONS)
							Flags = SINGLE_SOLUTION;
						else {
							if (GMath::Abs(LocalParameters[0] - 1) <= 2 * G_EPSILON)
								//   o----->o-------
								//          |      |
								//          --------
								Flags = SINGLE_SOLUTION;
							else {
								Flags = locFlags;
								//   o----------------->o           o----->o---
								//          |      |         or        |      |
								//          --------                   --------
								LocalParameters[1] = GMath::Min(locParams[1], (DATA_TYPE)1);
							}
						}
						FirstHittedSide = locBoxSize;
						return G_TRUE;
					}
				}
			}
		}
		else {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GLineSeg<DATA_TYPE, SIZE>& seg,
						   GUInt32& Flags, DATA_TYPE LocalParameters[2], GBoxSide& FirstHittedSide) {
		return Intersect(seg, oob, Flags, LocalParameters, FirstHittedSide);
	}

	// sphere - sphere (2D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericSphere<DATA_TYPE, 2>& sph1, const GGenericSphere<DATA_TYPE, 2>& sph2,
					GUInt32& Flags, GPoint<DATA_TYPE, 2> IntersectionPoints[2]) {

		DATA_TYPE dSqr, k, m, r0Sqr;
		GVect<DATA_TYPE, 2> p = sph2.Center() - sph1.Center();
		DATA_TYPE radiusSum = sph1.Radius() + sph2.Radius();

		dSqr = p.LengthSquared();
		// circles are separate
		if (dSqr > GMath::Sqr(radiusSum)) {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}

		DATA_TYPE radiusDif = sph1.Radius() - sph2.Radius();
		k = GMath::Sqr(radiusDif);
		// circle is contained within the other
		if (dSqr < k) {
			// same circles
			if (k <= G_EPSILON * G_EPSILON)
				Flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
			else
			// full inclusion
			Flags = INCLUDED_SHAPE | NO_SOLUTIONS;
			return G_TRUE;
		}

		r0Sqr = GMath::Sqr(sph1.Radius());
		k = r0Sqr - GMath::Sqr(sph2.Radius()) + dSqr;
		m = 4 * r0Sqr * dSqr - GMath::Sqr(k);
		// tangents circles (internal or external)
		if (m <= G_EPSILON * (2 * sph1.Radius() * GMath::Abs(radiusDif) + k)) {
			// intersection point
			if (IntersectionPoints) {
				p /= (2 * dSqr);
				IntersectionPoints[0] = sph1.Center() + k * p;
			}
			Flags = SINGLE_SOLUTION | TANGENT_SHAPES;
		}
		else {
			Flags = MULTIPLE_SOLUTIONS;
			if (IntersectionPoints) {
				m = GMath::Sqrt(m);
				p /= (2 * dSqr);
				// first intersection point
				IntersectionPoints[0].Set(sph1.Center()[G_X] + k * p[G_X] + m * p[G_Y],
										  sph1.Center()[G_Y] + k * p[G_Y] + m * p[G_X]);
				// second intersection point
				IntersectionPoints[1].Set(sph1.Center()[G_X] + k * p[G_X] - m * p[G_Y],
										  sph1.Center()[G_Y] + k * p[G_Y] - m * p[G_X]);
			}
		}
		return G_TRUE;
	}

	// sphere - sphere (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericSphere<DATA_TYPE, 3>& sph1, const GGenericSphere<DATA_TYPE, 3>& sph2,
					GUInt32& Flags,	GPoint<DATA_TYPE, 3>& Center, DATA_TYPE& Radius, GGenericPlane<DATA_TYPE>& Plane) {

		GVect<DATA_TYPE, 3> diffOrg(sph2.Center() - sph1.Center());
		DATA_TYPE normalLenSqr = diffOrg.LengthSquared();
		DATA_TYPE radiusSum = sph1.Radius() + sph2.Radius();

		// spheres are disjointed
		if (normalLenSqr > GMath::Sqr(radiusSum)) {
			Flags = NO_SOLUTIONS;
			return G_FALSE;
		}

		DATA_TYPE radiusDiff = sph1.Radius() - sph2.Radius();
		// same centers
		if (normalLenSqr <= G_EPSILON * G_EPSILON) {
			// sphere intersect only if radius are equal
			if (GMath::Abs(radiusDiff) <= G_EPSILON) {
				Flags = INFINITE_SOLUTIONS | COINCIDENT_SHAPES;
				return G_TRUE;
			}
			else {
				Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
				return G_FALSE;
			}
		}

		DATA_TYPE fR0Sqr = GMath::Sqr(sph1.Radius());
		DATA_TYPE fR1Sqr = GMath::Sqr(sph2.Radius());
		DATA_TYPE fInvNSqrLen = (DATA_TYPE)1 / normalLenSqr;
		DATA_TYPE fT = ((DATA_TYPE)1 + (fR0Sqr - fR1Sqr) * fInvNSqrLen) / 2;

		DATA_TYPE normalInvLen = 1 / GMath::Sqrt(normalLenSqr);
		DATA_TYPE tMin = -sph1.Radius() * normalInvLen;
		DATA_TYPE tMax = 1 + (sph2.Radius() * normalInvLen);
		// in this case a sphere is inside (not strictly) the other one; possible cases are strict (not tangent)
		// inclusion or tangent inclusion
		if (fT < tMin - G_EPSILON || fT > tMax + G_EPSILON) {
			Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
			return G_FALSE;
		}
		else
		if ((GMath::Abs(tMin - fT) <= G_EPSILON) || (GMath::Abs(tMax - fT) <= G_EPSILON)) {
			Center = fT * diffOrg + sph1.Center();
			Flags = SINGLE_SOLUTION | TANGENT_SHAPES | INCLUDED_SHAPE;
			return G_TRUE;
		}

		DATA_TYPE fRSqr = fR0Sqr - GMath::Sqr(fT) * normalLenSqr;
		// test for external tangent intersection
		if (fRSqr <= G_EPSILON * (sph1.Radius() + GMath::Abs(tMin * radiusDiff))) {
			Center = fT * diffOrg + sph1.Center();
			Flags = SINGLE_SOLUTION | TANGENT_SHAPES;
			return G_FALSE;
		}
		// center and radius of circle of intersection
		Center = sph1.Center() + fT * diffOrg;
		Radius = GMath::Sqrt(fRSqr);
		// normalize diffOrg
		diffOrg *= normalInvLen;
		// build plane of intersection
		Plane.SetNormal(diffOrg);
		Plane.SetOffset(-Dot(diffOrg, (const GVect<DATA_TYPE, 3>&)Center));
		Flags = INFINITE_SOLUTIONS;
		return G_TRUE;
	}


	// sphere - axis aligned box
	template<typename DATA_TYPE, GUInt32 SIZE>
	GBool Intersect(const GGenericSphere<DATA_TYPE, SIZE>& sph,	const GGenericAABox<DATA_TYPE, SIZE>& aab,
					GUInt32& Flags) {

		DATA_TYPE dmin, dmax, a, b, aSqr, bSqr, radiusSqr, tmpMin, tmpMax;
		GUInt32 internalTangents;
		GBool sphereIncluded;

		// Arvo's algorithm
		dmin = dmax = 0;

		sphereIncluded = G_TRUE;
		internalTangents = 0;
		radiusSqr = GMath::Sqr(sph.Radius());

		for (GUInt32 i = 0; i < SIZE; ++i) {
			a = GMath::Sqr(sph.Center()[i] - aab.Min()[i]);
			b = GMath::Sqr(sph.Center()[i] - aab.Max()[i]);
			// find min and max distance
			if (a <= b) {
				tmpMin = a;
				tmpMax = b;
			}
			else {
				tmpMin = b;
				tmpMax = a;
			}
			// update max distance
			dmax += tmpMax;
			if (sph.Center()[i] < aab.Min()[i])
				dmin += a;
			else
			if (sph.Center()[i] > aab.Max()[i])
				dmin += b;
			else {
				// full inclusion for sphere
				if (radiusSqr > tmpMin)
					sphereIncluded = G_FALSE;

				if (GMath::Abs(radiusSqr - tmpMin) <= 2 * G_EPSILON * sph.Radius())
					internalTangents++;

				if (GMath::Abs(radiusSqr - tmpMax) <= 2 * G_EPSILON * sph.Radius())
					internalTangents++;
			}
		}
		// if dmin > 0 at least one center coordinate is out of the box
		if (dmin > 0) {
			if (GMath::Abs(dmin - radiusSqr) <= G_EPSILON * sph.Radius()) {
				Flags = SINGLE_SOLUTION | TANGENT_SHAPES;
				return G_TRUE;
			}
			else
			if (dmin < radiusSqr) {
				if (SIZE == 2)
					Flags = MULTIPLE_SOLUTIONS;
				else
					Flags = INFINITE_SOLUTIONS;
				return G_TRUE;
			}
			else {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
		}

		// sphere includes the box
		if (GMath::Abs(radiusSqr - dmax) <= G_EPSILON * sph.Radius()) {
			Flags = MULTIPLE_SOLUTIONS | INCLUDED_SHAPE;
			return G_TRUE;
		}
		else
		if (radiusSqr > dmax) {
			Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
			return G_FALSE;
		}
		// box is included inside sphere or vice versa
		if (sphereIncluded) {
			if (internalTangents > 0) {
				Flags = TANGENT_SHAPES;
				if (internalTangents > 1)
					Flags |= MULTIPLE_SOLUTIONS;
				else
					Flags |= SINGLE_SOLUTION;
				return G_TRUE;
			}
			Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
			return G_FALSE;
		}
		// at this point sphere is not included into box, and also it doesn't include the box
		if (SIZE == 2)
			Flags = MULTIPLE_SOLUTIONS;
		else
			Flags = INFINITE_SOLUTIONS;
		if (internalTangents > 0)
			Flags |= TANGENT_SHAPES;
		return G_TRUE;
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericAABox<DATA_TYPE, SIZE>& aab, const GGenericSphere<DATA_TYPE, SIZE>& sph,
						   GUInt32& Flags) {
		return Intersect(sph, aab, Flags);
	}


	// sphere - object oriented box
	template<typename DATA_TYPE, GUInt32 SIZE>
	GBool Intersect(const GGenericSphere<DATA_TYPE, SIZE>& sph, const GGenericOOBox<DATA_TYPE, SIZE>& oob,
					GUInt32& Flags) {

		DATA_TYPE s, d, prj;
		DATA_TYPE dmin, dmax, a, b, radiusSqr, tmpMin, tmpMax;
		GUInt32 internalTangents;
		GBool sphereIncluded;

		// first we have to transform the sphere into oob space, so oob will become
		// an axis aligned box (respect to the sphere)
		GVect<DATA_TYPE, 3> deltaCenter = sph.Center() - oob.Center();

		// Arvo's algorithm
		dmin = dmax = 0;

		sphereIncluded = G_TRUE;
		internalTangents = 0;
		radiusSqr = GMath::Sqr(sph.Radius());

		d = 0;
		for (GInt32 i = 0; i < SIZE; ++i) {
			prj = Dot(deltaCenter, oob.Axis(i));

			a = GMath::Sqr(prj - (-oob.HalfDimension(i)));
			b = GMath::Sqr(prj - oob.HalfDimension(i));

			// find min and max distance
			if (a <= b) {
				tmpMin = a;
				tmpMax = b;
			}
			else {
				tmpMin = b;
				tmpMax = a;
			}
			// update max distance
			dmax += tmpMax;
			if (prj < -oob.HalfDimension(i))
				dmin += a;
			else
			if (prj > oob.HalfDimension(i))
				dmin += b;
			else {
				// full inclusion for sphere
				if (radiusSqr > tmpMin)
					sphereIncluded = G_FALSE;
				if (GMath::Abs(radiusSqr - tmpMin) <= G_EPSILON * sph.Radius())
					internalTangents++;
				if (GMath::Abs(radiusSqr - tmpMax) <= G_EPSILON * sph.Radius())
					internalTangents++;
			}
		}
		// if dmin > 0 at least one center coordinate is out of the box
		if (dmin > 0) {
			if (GMath::Abs(dmin - radiusSqr) <= G_EPSILON * sph.Radius()) {
				Flags = SINGLE_SOLUTION | TANGENT_SHAPES;
				return G_TRUE;
			}
			else
			if (dmin < radiusSqr) {
				if (SIZE == 2)
					Flags = MULTIPLE_SOLUTIONS;
				else
					Flags = INFINITE_SOLUTIONS;
				return G_TRUE;
			}
			else {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
		}

		// sphere includes the box
		if (GMath::Abs(radiusSqr - dmax) <= G_EPSILON * sph.Radius()) {
			Flags = MULTIPLE_SOLUTIONS | INCLUDED_SHAPE;
			return G_TRUE;
		}
		else
		if (radiusSqr > dmax) {
			Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
			return G_FALSE;
		}
		// box is included inside sphere or vice versa
		if (sphereIncluded) {
			if (internalTangents > 0) {
				Flags = TANGENT_SHAPES;
				if (internalTangents > 1)
					Flags |= MULTIPLE_SOLUTIONS;
				else
					Flags |= SINGLE_SOLUTION;
				return G_TRUE;
			}
			Flags = NO_SOLUTIONS | INCLUDED_SHAPE;
			return G_FALSE;
		}
		// at this point sphere is not included into box, and also it doesn't include the box
		if (SIZE == 2)
			Flags = MULTIPLE_SOLUTIONS;
		else
			Flags = INFINITE_SOLUTIONS;
		if (internalTangents > 0)
			Flags |= TANGENT_SHAPES;
		return G_TRUE;
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericSphere<DATA_TYPE, SIZE>& sph,
						   GUInt32& Flags) {
		return Intersect(sph, oob, Flags);
	}


	// axis aligned box - axis aligned box
	template<typename DATA_TYPE, GUInt32 SIZE>
	GBool Intersect(const GGenericAABox<DATA_TYPE, SIZE>& aab1,	const GGenericAABox<DATA_TYPE, SIZE>& aab2,
					GUInt32& Flags) {

		GVect<DATA_TYPE, SIZE> deltaCenter = aab1.Center() - aab2.Center();
		DATA_TYPE dist[SIZE], signedDist1[SIZE], signedDist2[SIZE], s;
		GUInt32 tangentsCount;

		// first check for separation
		for (GUInt32 i = 0; i < SIZE; ++i) {
			dist[i] = GMath::Abs(deltaCenter[i]) - aab1.HalfDimension(i) - aab2.HalfDimension(i);
			if (dist[i] > 2 * G_EPSILON) {
				Flags = NO_SOLUTIONS;
				return G_FALSE;
			}
		}
		// count external tangent faces
		tangentsCount = 0;
		for (GUInt32 i = 0; i < SIZE; ++i)
			if (GMath::Abs(dist[i]) <= G_EPSILON)
				tangentsCount++;
		// corner touch
		if (tangentsCount == SIZE) {
			Flags = SINGLE_SOLUTION;
			return G_TRUE;
		}
		else
		// tangent face
		if (tangentsCount > 0) {
			Flags = INFINITE_SOLUTIONS | TANGENT_SHAPES;
			return G_TRUE;
		}

		// now calculate signed distances between extremities
		s = 0;
		for (GUInt32 i = 0; i < SIZE; ++i) {
			signedDist1[i] = aab1.Min()[i] - aab2.Min()[i];
			signedDist2[i] = aab1.Max()[i] - aab2.Max()[i];
			s += GMath::Abs(signedDist1[i]) + GMath::Abs(signedDist2[i]);
		}
		// check for coincidence
		if (s <= G_EPSILON) {
			Flags = COINCIDENT_SHAPES;
			return G_TRUE;
		}

		// now we are sure that boxes are not included and they intersects
		Flags = NO_SOLUTIONS;
		for (GUInt32 i = 0; i < SIZE; ++i) {

			s = signedDist1[i] * signedDist2[i];
			if (GMath::Abs(s) <= G_EPSILON * G_EPSILON)
				Flags |= TANGENT_SHAPES;

			if (s > 0) {
				if (Flags & TANGENT_SHAPES)
					Flags |= INFINITE_SOLUTIONS;
				else
					Flags = MULTIPLE_SOLUTIONS;
				return G_TRUE;
			}
		}
		// boxes are included (box are included if every signed distances pair are of different sign)
		Flags |= INCLUDED_SHAPE;
		if (Flags & TANGENT_SHAPES)
			Flags |= INFINITE_SOLUTIONS;
		return G_TRUE;
	}


	// axis aligned box - object oriented box (2D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericAABox<DATA_TYPE, 2>& aab,	const GGenericOOBox<DATA_TYPE, 2>& oob) {

		// Separating Axis Theorem: we have only 4 potential separating axes
		DATA_TYPE s, ra, rb;
		GVect<DATA_TYPE, 2> deltaCenters(aab.Center() - oob.Center());

		DATA_TYPE axisXX = GMath::Abs(oob.Axis(G_X)[G_X]) + G_EPSILON;
		DATA_TYPE axisYX = GMath::Abs(oob.Axis(G_Y)[G_X]) + G_EPSILON;

		// aab x-axis (1, 0)
		s = GMath::Abs(deltaCenters[G_X]);
		ra = aab.HalfDimension(G_X);
		rb = axisXX * oob.HalfDimension(G_X) + axisYX * oob.HalfDimension(G_Y);
		if (s > ra + rb)
			return G_FALSE;


		DATA_TYPE axisXY = GMath::Abs(oob.Axis(G_X)[G_Y]) + G_EPSILON;
		DATA_TYPE axisYY = GMath::Abs(oob.Axis(G_Y)[G_Y]) + G_EPSILON;

		// aab y-axis (0, 1)
		s = GMath::Abs(deltaCenters[G_Y]);
		ra = aab.HalfDimension(G_Y);
		rb = axisXY * oob.HalfDimension(G_X) + axisYY * oob.HalfDimension(G_Y);
		if (s > ra + rb)
			return G_FALSE;

		// oob x-axis
		s = GMath::Abs(Dot(oob.Axis(G_X), deltaCenters));
		ra = oob.HalfDimension(G_X);
		rb = axisXX * aab.HalfDimension(G_X) + axisXY * aab.HalfDimension(G_Y);
		if (s > ra + rb)
			return G_FALSE;
		// oob y-axis
		s = GMath::Abs(Dot(oob.Axis(G_Y), deltaCenters));
		ra = oob.HalfDimension(G_Y);
		rb = axisYY * aab.HalfDimension(G_Y) + axisYX * aab.HalfDimension(G_X);
		if (s > ra + rb)
			return G_FALSE;
		// no separating axis found, return intersection
		return G_TRUE;
	}


	// object oriented box - object oriented box (2D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericOOBox<DATA_TYPE, 2>& oob1, const GGenericOOBox<DATA_TYPE, 2>& oob2) {

			// Separating Axis Theorem: we have only 4 potential separating axes
			DATA_TYPE s, ra, rb;
			GVect<DATA_TYPE, 2> deltaCenters(oob2.Center() - oob1.Center());

			DATA_TYPE axisXX = GMath::Abs(Dot(oob1.Axis(G_X), oob2.Axes(G_X))) + G_EPSILON;
			DATA_TYPE axisXY = GMath::Abs(Dot(oob1.Axis(G_X), oob2.Axes(G_Y))) + G_EPSILON;

			// oob1 x-axis
			s = GMath::Abs(Dot(deltaCenters, oob1.Axis(G_X)));
			ra = oob1.HalfDimension(G_X);
			rb = axisXX * oob2.HalfDimension(G_X) + axisXY * oob2.HalfDimension(G_Y);
			if (s > ra + rb)
				return G_FALSE;

			DATA_TYPE axisYX = GMath::Abs(Dot(oob1.Axis(G_Y), oob2.Axes(G_X))) + G_EPSILON;
			DATA_TYPE axisYY = GMath::Abs(Dot(oob1.Axis(G_Y), oob2.Axes(G_Y))) + G_EPSILON;

			// oob1 y-axis
			s = GMath::Abs(Dot(deltaCenters, oob1.Axis(G_Y)));
			ra = oob1.HalfDimension(G_Y);
			rb = axisYX * oob2.HalfDimension(G_X) +	axisYY * oob2.HalfDimension(G_Y);
			if (s > ra + rb)
				return G_FALSE;

			// oob2 x-axis
			s = GMath::Abs(Dot(deltaCenters, oob2.Axis(G_X)));
			ra = oob2.HalfDimension(G_X);
			rb = axisXX * oob1.HalfDimension(G_X) +	axisYX * oob1.HalfDimension(G_Y);
			if (s > ra + rb)
				return G_FALSE;

			// oob2 y-axis
			s = GMath::Abs(Dot(deltaCenters, oob2.Axis(G_Y)));
			ra = oob2.HalfDimension(G_Y);
			rb = axisXY * oob1.HalfDimension(G_X) +	axisYY * oob1.HalfDimension(G_Y);
			if (s > ra + rb)
				return G_FALSE;

			// no separating axis found, return intersection
			return G_TRUE;
		}

	// axis aligned box - object oriented box (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericAABox<DATA_TYPE, 3>& aab,	const GGenericOOBox<DATA_TYPE, 3>& oob) {

		// Separating Axis Theorem
		GVect<DATA_TYPE, 3> deltaCenter = aab.Center() - oob.Center();
		// aab position in oob1 space
		GVect<DATA_TYPE, 3> T(Dot(oob.Axis(G_X), deltaCenter),
							  Dot(oob.Axis(G_Y), deltaCenter),
							  Dot(oob.Axis(G_Z), deltaCenter));

		DATA_TYPE B[3][3], Bf[3][3];

		#define a(r) \
			oob.HalfDimension(r)

		#define b(r) \
			aab.HalfDimension(r)

		#define TESTCASE1(x) \
			(GMath::Abs(T[x]) > \
			(a(x) + b(0) * Bf[0][x] + b(1) * Bf[1][x] + b(2) * Bf[2][x]))

		#define TESTCASE2(x) \
			(GMath::Abs(T[0] * B[0][x] + T[1] * B[1][x] + T[2] * B[2][x]) > \
			(b(x) + a(0) * Bf[0][x] + a(1) * Bf[1][x] + a(2) * Bf[2][x]))

		#define TESTCASE3(i, j) \
			(GMath::Abs(T[(i+2)%3] * B[(i+1)%3][j] - T[(i+1)%3] * B[(i+2)%3][j]) > \
			(a((i+1)%3) * Bf[(i+2)%3][j] + a((i+2)%3) * Bf[(i+1)%3][j] + \
			b((j+1)%3) * Bf[i][(j+2)%3] + b((j+2)%3) * Bf[i][(j+1)%3]))


		B[G_X][G_X] = oob.Axis(G_X)[G_X];
		Bf[G_X][G_X] = GMath::Abs(B[G_X][G_X]) + G_EPSILON;

		B[G_X][G_Y] = oob.Axis(G_X)[G_Y];
		Bf[G_X][G_Y] = GMath::Abs(B[G_X][G_Y]) + G_EPSILON;

		B[G_X][G_Z] = oob.Axis(G_X)[G_Z];
		Bf[G_X][G_Z] = GMath::Abs(B[G_X][G_Z]) + G_EPSILON;

		B[G_Y][G_X] = oob.Axis(G_Y)[G_X];
		Bf[G_Y][G_X] = GMath::Abs(B[G_Y][G_X]) + G_EPSILON;

		B[G_Y][G_Y] = oob.Axis(G_Y)[G_Y];
		Bf[G_Y][G_Y] = GMath::Abs(B[G_Y][G_Y]) + G_EPSILON;

		B[G_Y][G_Z] = oob.Axis(G_Y)[G_Z];
		Bf[G_Y][G_Z] = GMath::Abs(B[G_Y][G_Z]) + G_EPSILON;

		B[G_Z][G_X] = oob.Axis(G_Z)[G_X];
		Bf[G_Z][G_X] = GMath::Abs(B[G_Z][G_X]) + G_EPSILON;

		B[G_Z][G_Y] = oob.Axis(G_Z)[G_Y];
		Bf[G_Z][G_Y] = GMath::Abs(B[G_Z][G_Y]) + G_EPSILON;

		B[G_Z][G_Z] = oob.Axis(G_Z)[G_Z];
		Bf[G_Z][G_Z] = GMath::Abs(B[G_Z][G_Z]) + G_EPSILON;

		// case 1
		if TESTCASE1(0) return G_FALSE;
		if TESTCASE1(1) return G_FALSE;
		if TESTCASE1(2) return G_FALSE;
		// case 2
		if TESTCASE2(0) return G_FALSE;
		if TESTCASE2(1) return G_FALSE;
		if TESTCASE2(2) return G_FALSE;
		// case 3
		if TESTCASE3(0, 0) return G_FALSE;
		if TESTCASE3(1, 0) return G_FALSE;
		if TESTCASE3(2, 0) return G_FALSE;
		if TESTCASE3(0, 1) return G_FALSE;
		if TESTCASE3(1, 1) return G_FALSE;
		if TESTCASE3(2, 1) return G_FALSE;
		if TESTCASE3(0, 2) return G_FALSE;
		if TESTCASE3(1, 2) return G_FALSE;
		if TESTCASE3(2, 2) return G_FALSE;
		// intersection found
		return G_TRUE;
		#undef TESTCASE1
		#undef TESTCASE2
		#undef TESTCASE3
		#undef B
		#undef T
		#undef b
		#undef a
	}

	// object oriented box - object oriented box (3D)
	template<typename DATA_TYPE>
	GBool Intersect(const GGenericOOBox<DATA_TYPE, 3>& oob1, const GGenericOOBox<DATA_TYPE, 3>& oob2) {
		// Here we use "Separating Axis Theorem"
		// first transform oob2 box into obb1 box space
		GVect<DATA_TYPE, 3> deltaCenter = oob2.Center() - oob1.Center();
		// oob2 position in oob1 space
		GVect<DATA_TYPE, 3> T(Dot(oob1.Axis(G_X), deltaCenter),
							  Dot(oob1.Axis(G_Y), deltaCenter),
							  Dot(oob1.Axis(G_Z), deltaCenter));
		// oob2 axes into oob1 coordinate system, and it's corresponding absolute values
		DATA_TYPE B[3][3], Bf[3][3];

		#define a(r) \
			oob1.HalfDimension(r)

		#define b(r) \
			oob2.HalfDimension(r)

		#define TESTCASE1(x) \
			(GMath::Abs(T[x]) > \
			(a(x) + b(0) * Bf[0][x] + b(1) * Bf[1][x] + b(2) * Bf[2][x]))

		#define TESTCASE2(x) \
			(GMath::Abs(T[0] * B[0][x] + T[1] * B[1][x] + T[2] * B[2][x]) > \
			(b(x) + a(0) * Bf[0][x] + a(1) * Bf[1][x] + a(2) * Bf[2][x]))

		#define TESTCASE3(i, j) \
			(GMath::Abs(T[(i+2)%3] * B[(i+1)%3][j] - T[(i+1)%3] * B[(i+2)%3][j]) > \
			(a((i+1)%3) * Bf[(i+2)%3][j] + a((i+2)%3) * Bf[(i+1)%3][j] + \
			b((j+1)%3) * Bf[i][(j+2)%3] + b((j+2)%3) * Bf[i][(j+1)%3]))

		// transform each oob2 axis into oob1 coordinate system
		B[G_X][G_X] = Dot(oob1.Axis(G_X), oob2.Axis(G_X));
		Bf[G_X][G_X] = GMath::Abs(B[G_X][G_X]) + G_EPSILON;

		B[G_X][G_Y] = Dot(oob1.Axis(G_X), oob2.Axis(G_Y));
		Bf[G_X][G_Y] = GMath::Abs(B[G_X][G_Y]) + G_EPSILON;

		B[G_X][G_Z] = Dot(oob1.Axis(G_X), oob2.Axis(G_Z));
		Bf[G_X][G_Z] = GMath::Abs(B[G_X][G_Z]) + G_EPSILON;

		B[G_Y][G_X] = Dot(oob1.Axis(G_Y), oob2.Axis(G_X));
		Bf[G_Y][G_X] = GMath::Abs(B[G_Y][G_X]) + G_EPSILON;

		B[G_Y][G_Y] = Dot(oob1.Axis(G_Y), oob2.Axis(G_Y));
		Bf[G_Y][G_Y] = GMath::Abs(B[G_Y][G_Y]) + G_EPSILON;

		B[G_Y][G_Z] = Dot(oob1.Axis(G_Y), oob2.Axis(G_Z));
		Bf[G_Y][G_Z] = GMath::Abs(B[G_Y][G_Z]) + G_EPSILON;

		B[G_Z][G_X] = Dot(oob1.Axis(G_Z), oob2.Axis(G_X));
		Bf[G_Z][G_X] = GMath::Abs(B[G_Z][G_X]) + G_EPSILON;

		B[G_Z][G_Y] = Dot(oob1.Axis(G_Z), oob2.Axis(G_Y));
		Bf[G_Z][G_Y] = GMath::Abs(B[G_Z][G_Y]) + G_EPSILON;

		B[G_Z][G_Z] = Dot(oob1.Axis(G_Z), oob2.Axis(G_Z));
		Bf[G_Z][G_Z] = GMath::Abs(B[G_Z][G_Z]) + G_EPSILON;

		// case 1
		if TESTCASE1(0) return G_FALSE;
		if TESTCASE1(1) return G_FALSE;
		if TESTCASE1(2) return G_FALSE;
		// case 2
		if TESTCASE2(0) return G_FALSE;
		if TESTCASE2(1) return G_FALSE;
		if TESTCASE2(2) return G_FALSE;
		// case 3
		if TESTCASE3(0, 0) return G_FALSE;
		if TESTCASE3(1, 0) return G_FALSE;
		if TESTCASE3(2, 0) return G_FALSE;
		if TESTCASE3(0, 1) return G_FALSE;
		if TESTCASE3(1, 1) return G_FALSE;
		if TESTCASE3(2, 1) return G_FALSE;
		if TESTCASE3(0, 2) return G_FALSE;
		if TESTCASE3(1, 2) return G_FALSE;
		if TESTCASE3(2, 2) return G_FALSE;
		// intersection found
		return G_TRUE;
		#undef TESTCASE1
		#undef TESTCASE2
		#undef TESTCASE3
		#undef a
		#undef b
	}

	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool Intersect(const GGenericOOBox<DATA_TYPE, SIZE>& oob, const GGenericAABox<DATA_TYPE, SIZE>& aab) {
		return Intersect(aab, oob);
	}

};	// end namespace Amanith

#endif
