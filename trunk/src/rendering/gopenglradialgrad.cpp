/****************************************************************************
** $file: amanith/src/rendering/gopenglradialgrad.cpp   0.1.1.0   edited Sep 24 08:00
**
** OpenGL based draw board radial gradient functions implementation.
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

#include "amanith/rendering/gopenglboard.h"
#include "amanith/geometry/gxform.h"
#include "amanith/geometry/gxformconv.h"

/*!
	\file gopenglradialgrad.cpp
	\brief OpenGL based draw board radial gradient functions implementation file.
*/

namespace Amanith {


void GOpenGLBoard::IntersectRaysDisk(const GRay2& Ray0, const GRay2& Ray1, const GPoint2& Center, const GReal Radius,
									  GPoint2& P0, GPoint2& P1, GReal& SpannedAngle) const {

	GUInt32 intFlags;
	GReal intParams[2];
	GBool intFound;
	GSphere2 sph(Center, Radius);

	intFound = Intersect(Ray0, sph, intFlags, intParams);
	G_ASSERT(intFound == G_TRUE);
	G_ASSERT(intFlags & SINGLE_SOLUTION);
	P0 = Ray0.Origin() + intParams[0] * Ray0.Direction();

	intFound = Intersect(Ray1, sph, intFlags, intParams);
	G_ASSERT(intFound == G_TRUE);
	G_ASSERT(intFlags & SINGLE_SOLUTION);
	P1 = Ray1.Origin() + intParams[0] * Ray1.Direction();

	GVector2 d0(P0 - Center);
	GVector2 d1(P1 - Center);
	GReal cr = Cross(d0, d1);

	// smaller angle in CCW, going from P0 to P1
	if (cr > 0)
		SpannedAngle = GMath::Acos(Dot(d0, d1) / (d0.Length() * d1.Length()));
	else
		SpannedAngle = (GReal)G_2PI - GMath::Acos(Dot(d0, d1) / (d0.Length() * d1.Length()));

	G_ASSERT(SpannedAngle >= 0);
}

void GOpenGLBoard::DrawGLRadialSector(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
									  const GReal Time0, const GReal Time1,
									  const GPoint2& P0, const GPoint2& P1, const GBool WholeDisk,
									  const GDynArray<GKeyValue>& ColorKeys, const GColorRampInterpolation Interpolation,
									  const GColorRampSpreadMode SpreadMode,
									  const GReal MultAlpha) const {

	/*
		NB: color keys must be sorted by TimePosition() !!!
	*/
	if (Time0 >= Time1)
		return;

	G_ASSERT(Time0 >= 0 && Time1 >= 0);

	GPoint2 e0, e1;
	GVector2 normDirFC = Center - Focus;
	GVector2 dirFC = normDirFC;
	GReal distFC = normDirFC.Normalize();

	// handle the case where Center == Focus
	if (distFC <= G_EPSILON) {
		distFC = 0;
		normDirFC.Set(1, 0);
	}

	GRay2 rayP0(Focus, P0 - Focus);
	GRay2 rayP1(Focus, P1 - Focus);

	// calculate radius corresponding to Time1
	GReal maxRadius = (distFC + Radius) * Time1;
	GPoint2 maxCenter = Focus + Time1 * dirFC;
	GReal deltaAngle;

	if (!WholeDisk)
		IntersectRaysDisk(rayP0, rayP1, maxCenter, maxRadius, e0, e1, deltaAngle);
	else
		deltaAngle = (GReal)G_2PI;

	// now calculate the number of segments to produce (number of times we have to subdivide angle) that
	// permit to have a squared chordal distance less than gDeviation
	GReal dev = GMath::Clamp(gFlateness, G_EPSILON, maxRadius - (G_EPSILON * maxRadius));

	if (RenderingQuality() == G_NORMAL_RENDERING_QUALITY)
		dev *= 1.5;
	else
	if (RenderingQuality() == G_LOW_RENDERING_QUALITY)
		dev *= 2.5;

	GInt32 n = 1;
	GReal n1 = deltaAngle / (2 * GMath::Acos(1 - dev / maxRadius));
	if (n1 > 1 && n1 >= (deltaAngle * G_ONE_OVER_PI)) {
		if (n1 > n)
			n = (GUInt32)GMath::Ceil(n1);
	}
	if (n < 3)
		n = 3;


	// calculate a new array of color keys, taking care of spread mode and color interpolation
	GInt32 i, w, k, keyDir, j = (GInt32)ColorKeys.size();
	GTimeValue t, tOld, tSum;
	GDynArray<GKeyValue> tmpKeys;
	GReal ratio;

	G_ASSERT(j >= 2);

	// now we are sure that we have at least 2 keys
	switch (SpreadMode) {

		case G_PAD_COLOR_RAMP_SPREAD:

			// skip all keys <= Time0
			k = 0;
			while (k < j && ColorKeys[k].TimePosition() <= Time0)
				k++;

			// push a new key at Time0
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - 1]);
					tmpKeys.back().SetTimePosition(Time0);
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:

					if (k == j) {
						tmpKeys.push_back(ColorKeys[k - 1]);
						tmpKeys.back().SetTimePosition(Time0);
					}
					else {
						ratio = (Time0 - ColorKeys[k - 1].TimePosition()) / (ColorKeys[k].TimePosition() - ColorKeys[k - 1].TimePosition());
						tmpKeys.push_back(GKeyValue(Time0, GMath::Lerp(ratio, ColorKeys[k - 1].Vect4Value(), ColorKeys[k].Vect4Value())));
					}
					break;
			}

			// push all intermediate keys ( > Time0 and < Time1)
			while (k < j && ColorKeys[k].TimePosition() < Time1) {
				tmpKeys.push_back(ColorKeys[k]);
				k++;
			}

			// push a new key at Time1
			// interpolate color value between key k-1 and k
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - 1]);
					tmpKeys.back().SetTimePosition(Time1);
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					if (k == j) {
						tmpKeys.push_back(ColorKeys[k - 1]);
						tmpKeys.back().SetTimePosition(Time1);
					}
					else {
						ratio = (Time1 - ColorKeys[k - 1].TimePosition()) / (ColorKeys[k].TimePosition() - ColorKeys[k - 1].TimePosition());
						tmpKeys.push_back(GKeyValue(Time1, GMath::Lerp(ratio, ColorKeys[k - 1].Vect4Value(), ColorKeys[k].Vect4Value())));
					}
					break;
			}
			break;

		case G_REPEAT_COLOR_RAMP_SPREAD:

			k = 0;
			t = tOld = ColorKeys[0].TimePosition();
			tSum = 0;

			// skip all keys <= Time0
			//! \todo optimize skip, summing whole intervals
			while (tSum <= Time0) {
				tOld = t;
				k++;
				if (k >= j) {
					tOld = 0;
					k = 1;
				}
				t = ColorKeys[k].TimePosition();
				tSum += (t - tOld);
			}

			// push a new key at Time0
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - 1]);
					tmpKeys.back().SetTimePosition(Time0);
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					ratio = (Time0 - tSum + (t - tOld)) / (t - tOld);
					tmpKeys.push_back(GKeyValue(Time0, GMath::Lerp(ratio, ColorKeys[(k - 1) % j].Vect4Value(), ColorKeys[k % j].Vect4Value())));
					break;
			}

			// push all intermediate keys ( > Time0 and < Time1)
			while (tSum < Time1) {

				tOld = t;
				k++;
				if (k >= j) {
					tmpKeys.push_back(ColorKeys[(k - 1) % j]);
					tmpKeys.back().SetTimePosition(tSum);
					k = 0;
					t = tOld = ColorKeys[0].TimePosition();
				}
				else {
					tmpKeys.push_back(ColorKeys[(k - 1) % j]);
					tmpKeys.back().SetTimePosition(tSum);
					t = ColorKeys[k % j].TimePosition();
					tSum += (t - tOld);
				}
			}

			// push a new key at Time1
			// interpolate color value between key k-1 and k
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[(k - 1) % j]);
					tmpKeys.back().SetTimePosition(Time1);
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					ratio = (Time1 - tSum + (t - tOld)) / (t - tOld);
					tmpKeys.push_back(GKeyValue(Time1, GMath::Lerp(ratio, ColorKeys[(k - 1) % j].Vect4Value(), ColorKeys[k % j].Vect4Value())));
					break;
			}
			break;

		case G_REFLECT_COLOR_RAMP_SPREAD:

			keyDir = 1;

			// skip all keys <= Time0
			k = 0;
			t = tOld = ColorKeys[0].TimePosition();
			tSum = 0;

			// skip all keys <= Time0
			//! \todo optimize skip, summing whole intervals
			while (tSum <= Time0) {

				tOld = t;

				k += keyDir;
				if (k < 0) {
					keyDir = 1;
					k = 0;
				}
				else
				if (k >= j) {
					keyDir = -1;
					k = j - 1;
				}

				t = ColorKeys[k].TimePosition();
				tSum += GMath::Abs(t - tOld);
			}

			// push a new key at Time0
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - keyDir]);
					tmpKeys.back().SetTimePosition(Time0);
					break;
				
				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					ratio = (Time0 - tSum + GMath::Abs(t - tOld)) / GMath::Abs(t - tOld);
					tmpKeys.push_back(GKeyValue(Time0, GMath::Lerp(ratio, ColorKeys[(k - keyDir)].Vect4Value(), ColorKeys[k].Vect4Value())));
					break;
			}

			// push all intermediate keys ( > Time0 and < Time1)
			while (tSum < Time1) {

				tOld = t;
				
				k += keyDir;
				if (k < 0) {
					keyDir = 1;
					k = 1;
				}
				else
				if (k >= j) {
					keyDir = -1;
					k = j - 2;
				}

				tmpKeys.push_back(ColorKeys[(k - keyDir)]);
				tmpKeys.back().SetTimePosition(tSum);

				t = ColorKeys[k].TimePosition();
				tSum += GMath::Abs(t - tOld);
			}

			// push a new key at Time1
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					tmpKeys.push_back(ColorKeys[k - keyDir]);
					tmpKeys.back().SetTimePosition(Time1);
					break;

				case G_LINEAR_COLOR_INTERPOLATION:
				//! \todo Handle exact Hermitian interpolation
				case G_HERMITE_COLOR_INTERPOLATION:
					ratio = (Time1 - tSum + GMath::Abs(t - tOld)) / GMath::Abs(t - tOld);
					tmpKeys.push_back(GKeyValue(Time1, GMath::Lerp(ratio, ColorKeys[(k - keyDir)].Vect4Value(), ColorKeys[k].Vect4Value())));
					break;
			}

			break;
	}

	// modulate color keys with specified "global" alpha
	if (MultAlpha < 1) {
		GDynArray<GKeyValue>::iterator it = tmpKeys.begin();
		for (; it != tmpKeys.end(); ++it) {
			GVector4 v = it->Vect4Value();
			v[G_W] *= MultAlpha;
			it->SetValue(v);
		}
	}
	
	GDynArray< GVectBase<GReal, 2> > ptsCache(n + 1);
	GReal cosDelta;
	GReal sinDelta;
	GReal r;
	GPoint2 p, q, m, c;
	GVector4 col, oldCol;

	if (Time0 == 0) {
		i = 1;
		t = tmpKeys[i].TimePosition();
		oldCol = tmpKeys[0].Vect4Value();
		r = t * Radius;
		c = Focus + t * dirFC;

		if (!WholeDisk) {
			IntersectRaysDisk(rayP0, rayP1, c, r, e0, e1, deltaAngle);
			p = e0 - c;
			m = e0;
		}
		else {
			p = r * normDirFC;
			m = c + p;
		}

		cosDelta = GMath::Cos(deltaAngle / n);
		sinDelta = GMath::Sin(deltaAngle / n);

		ptsCache[0] = m;

		// first draw triangle fan
		glBegin(GL_TRIANGLE_FAN);

		#ifdef DOUBLE_REAL_TYPE
			glColor4dv(oldCol.Data());
			glVertex2dv(Focus.Data());
		#else
			glColor4fv(oldCol.Data());
			glVertex2fv(Focus.Data());
		#endif

		if (Interpolation != G_CONSTANT_COLOR_INTERPOLATION)
			oldCol = tmpKeys[1].Vect4Value();

		#ifdef DOUBLE_REAL_TYPE
			glColor4dv(oldCol.Data());
			glVertex2dv(m.Data());
		#else
			glColor4fv(oldCol.Data());
			glVertex2fv(m.Data());
		#endif

		for (k = 0; k < n - 1; ++k) {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
			ptsCache[k + 1] = m;
			#ifdef DOUBLE_REAL_TYPE
				glVertex2dv(m.Data());
			#else
				glVertex2fv(m.Data());
			#endif
			p = q;
		}

		// push last point (avoiding precision errors for the case 2PI)
		if (deltaAngle == G_2PI)
			m = c + r * normDirFC;
		else {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
		}
		ptsCache[k + 1] = m;
		#ifdef DOUBLE_REAL_TYPE
			glVertex2dv(m.Data());
		#else
			glVertex2fv(m.Data());
		#endif

		glEnd();

		if (Interpolation == G_CONSTANT_COLOR_INTERPOLATION)
			oldCol = tmpKeys[1].Vect4Value();
	}
	else {
		i = 0;
		t = tmpKeys[i].TimePosition();
		oldCol = tmpKeys[i].Vect4Value();
		r = t * Radius;
		c = Focus + t * dirFC;

		if (!WholeDisk) {
			IntersectRaysDisk(rayP0, rayP1, c, r, e0, e1, deltaAngle);
			p = e0 - c;
			m = e0;
		}
		else {
			p = r * normDirFC;
			m = c + p;
		}

		ptsCache[0] = m;
		cosDelta = GMath::Cos(deltaAngle / n);
		sinDelta = GMath::Sin(deltaAngle / n);

		for (k = 0; k < n - 1; ++k) {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
			ptsCache[k + 1] = m;
			p = q;
		}
		// push last point (avoiding precision errors for the case 2PI)
		if (deltaAngle == G_2PI)
			m = c + r * normDirFC;
		else {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
		}
		ptsCache[k + 1] = m;
	}

	i++;
	k = (GInt32)tmpKeys.size();

	for (; i < k; ++i) {

		t = tmpKeys[i].TimePosition();
		if (Interpolation == G_CONSTANT_COLOR_INTERPOLATION)
			col = oldCol;
		else
			col = tmpKeys[i].Vect4Value();

		r = t * Radius;
		c = Focus + t * dirFC;

		if (!WholeDisk) {
			IntersectRaysDisk(rayP0, rayP1, c, r, e0, e1, deltaAngle);
			p = e0 - c;
			m = e0;
		}
		else {
			p = r * normDirFC;
			m = c + p;
		}

		cosDelta = GMath::Cos(deltaAngle / n);
		sinDelta = GMath::Sin(deltaAngle / n);


		glBegin(GL_TRIANGLE_STRIP);

		// push first point
		#ifdef DOUBLE_REAL_TYPE
			glColor4dv(oldCol.Data());
			glVertex2dv(ptsCache[0].Data());
			glColor4dv(col.Data());
			glVertex2dv(m.Data());
		#else
			glColor4fv(oldCol.Data());
			glVertex2fv(ptsCache[0].Data());
			glColor4fv(col.Data());
			glVertex2fv(m.Data());
		#endif
		ptsCache[0] = m;

		for (w = 0; w < n - 1; ++w) {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
			#ifdef DOUBLE_REAL_TYPE
				glColor4dv(oldCol.Data());
				glVertex2dv(ptsCache[w + 1].Data());
				glColor4dv(col.Data());
				glVertex2dv(m.Data());
			#else
				glColor4fv(oldCol.Data());
				glVertex2fv(ptsCache[w + 1].Data());
				glColor4fv(col.Data());
				glVertex2fv(m.Data());
			#endif
			ptsCache[w + 1] = m;
			p = q;
		}
		// push last point (avoiding precision errors for the case 2PI)
		if (deltaAngle == G_2PI)
			m = c + r * normDirFC;
		else {
			q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
			m = q + c;
		}
		#ifdef DOUBLE_REAL_TYPE
			glColor4dv(oldCol.Data());
			glVertex2dv(ptsCache[w + 1].Data());
			glColor4dv(col.Data());
			glVertex2dv(m.Data());
		#else
			glColor4fv(oldCol.Data());
			glVertex2fv(ptsCache[w + 1].Data());
			glColor4fv(col.Data());
			glVertex2fv(m.Data());
		#endif

		ptsCache[w + 1] = m;

		if (Interpolation == G_CONSTANT_COLOR_INTERPOLATION)
			oldCol = tmpKeys[i].Vect4Value();
		else
			oldCol = col;

		glEnd();
	}
}

GInt32 GOpenGLBoard::SignBoxDisk(const GAABox2& Box, const GPoint2& Center, const GReal Radius) {

	GVector2 deltaCenter = Center - Box.Center();
	GReal distX = GMath::Abs(deltaCenter[G_X]) - Box.HalfDimension(G_X);
	GReal distY = GMath::Abs(deltaCenter[G_Y]) - Box.HalfDimension(G_Y);
	// if both distances are negative, then Distance = Max(distX, distY)
	// if one distance is negative, then Distance = the positive one
	// else Arvo's algorithm
	if (distX <= 0) {
		if (distY <= 0)
			return GMath::Sign(GMath::Max(distX, distY) - Radius);
		else
			return GMath::Sign(distY - Radius);
	}
	else {
		if (distY <= 0)
			return GMath::Sign(distX - Radius);
		else {
			// Arvo's algorithm
			//GReal d = GMath::Sqrt(distX * distX) + (distY * distY);
			return GMath::Sign((distX * distX) + (distY * distY) - Radius * Radius);
			//return GMath::Sign(d - Radius);
		}
	}
}

void GOpenGLBoard::DrawRadialSector(const GPoint2& Center, const GPoint2& Focus, const GReal Radius,
									const GAABox2& BoundingBox,
									const GDynArray<GKeyValue>& ColorKeys, const GColorRampInterpolation Interpolation,
									const GColorRampSpreadMode SpreadMode,
									const GReal MultAlpha, const GMatrix33& GradientMatrix) const {


	GPoint2 transfFocus = GradientMatrix * Focus;
	GPoint2 transfCenter = GradientMatrix * Center;
	GPoint2 tmpRadiusPoint(Center[G_X] + Radius, Center[G_Y]);
	GPoint2 transfRadiusPoint = GradientMatrix * tmpRadiusPoint;
	GReal transfRadius = Distance(transfCenter, transfRadiusPoint);


	GPoint2 realFocus(transfFocus);
	if (Distance(transfFocus, transfCenter) >= transfRadius)
		realFocus = transfCenter;

	GPoint2 p0 = BoundingBox.Min();
	GPoint2 p2 = BoundingBox.Max();
	GPoint2 p1(p2[G_X], p0[G_Y]);
	GPoint2 p3(p0[G_X], p2[G_Y]);

	GPoint2 pMax, pMin;
	GVector2 v;
	GReal dMax, tMax, tMin;
	GSphere2 sph(transfCenter, transfRadius);
	GBool intFound;
	GUInt32 intFlags;
	GReal intParams[2];
	GRay2 ray;
	GBool wholeDisk = G_FALSE;

	ray.SetOrigin(realFocus);
	tMax = 0;

	// calculate the maximum radial value (distance between point and focus divided by the length of the
	// line segment starting at focus, passing through the point, and ending on the circumference of the
	// gradient circle)	of box corners
	ray.SetDirection(p0 - realFocus);
	intFound = Intersect(ray, sph, intFlags, intParams);
	if (intFound) {

		G_ASSERT(intFlags & SINGLE_SOLUTION);
		v = (ray.Origin() + intParams[0] * ray.Direction()) - realFocus;
		dMax = ray.Direction().LengthSquared() / v.LengthSquared();
		if (dMax > tMax)
			tMax = dMax;
	}
	ray.SetDirection(p1 - realFocus);
	intFound = Intersect(ray, sph, intFlags, intParams);
	if (intFound) {
		G_ASSERT(intFlags & SINGLE_SOLUTION);
		v = (ray.Origin() + intParams[0] * ray.Direction()) - realFocus;
		dMax = ray.Direction().LengthSquared() / v.LengthSquared();
		if (dMax > tMax)
			tMax = dMax;
	}
	ray.SetDirection(p2 - realFocus);
	intFound = Intersect(ray, sph, intFlags, intParams);
	if (intFound) {
		G_ASSERT(intFlags & SINGLE_SOLUTION);
		v = (ray.Origin() + intParams[0] * ray.Direction()) - realFocus;
		dMax = ray.Direction().LengthSquared() / v.LengthSquared();
		if (dMax > tMax)
			tMax = dMax;
	}
	ray.SetDirection(p3 - realFocus);
	intFound = Intersect(ray, sph, intFlags, intParams);
	if (intFound) {
		G_ASSERT(intFlags & SINGLE_SOLUTION);
		v = (ray.Origin() + intParams[0] * ray.Direction()) - realFocus;
		dMax = ray.Direction().LengthSquared() / v.LengthSquared();
		if (dMax > tMax)
			tMax = dMax;
	}
	// the final tMax
	tMax = GMath::Sqrt(tMax);


	// if the focus is inside the box, whole disk must be rendered (and in this case tMin is 0)
	if ((realFocus[G_X] > BoundingBox.Min()[G_X] && realFocus[G_X] < BoundingBox.Max()[G_X]) &&
		(realFocus[G_Y] > BoundingBox.Min()[G_Y] && realFocus[G_Y] < BoundingBox.Max()[G_Y])) {
		wholeDisk = G_TRUE;
		tMin = 0;
	}
	else {
		GUInt32 curSteps = 0;
		GReal tTail = 0;
		GReal tHead = tMax;

		GVector2 dirFC = transfCenter - realFocus;
		GReal lenFC = dirFC.Length();

	#ifdef _DEBUG
		GInt32 signHead = SignBoxDisk(BoundingBox, realFocus + tHead * dirFC, tHead * transfRadius);
		GInt32 signTail = SignBoxDisk(BoundingBox, realFocus + tTail * dirFC, tTail * transfRadius);
		G_ASSERT((signHead * signTail) <= 0);
	#endif

		// now use a bisection iterative method to find a good bound for tMin
		do {
			GReal tPivot = (tTail + tHead) * (GReal)0.5;
			GInt32 signPivot = SignBoxDisk(BoundingBox, realFocus + tPivot * dirFC, tPivot * transfRadius);

			if (signPivot == 0) {
				tTail = tPivot;
				break;
			}
			else
			if (signPivot < 0)
				tHead = tPivot;
			else
				tTail = tPivot;
			curSteps++;
		} while(curSteps < 5);
		tMin = tTail;

		// it handles the case when focus and center are the same 
		if (lenFC <= G_EPSILON) {
			dirFC = BoundingBox.Center() - realFocus;
			lenFC = dirFC.Length();
		}

		// if we have identified a sector we must calculate "external" points
		dirFC /= lenFC;

		// calculate axes transformation (transform box corners int the coordinate system given by Focus-Center
		// direction
		GMatrix22 A;
		GUInt32 iMax = 0;
		A.Set(dirFC[G_X], dirFC[G_Y], -dirFC[G_Y], dirFC[G_X]);

		GReal angles[4];
		GPoint2 pts[4];

		if (DistanceSquared(p0, realFocus) > G_EPSILON) {
			pts[iMax] = A * (p0 - realFocus);
			angles[iMax] = GMath::Atan2(pts[iMax][G_Y], pts[iMax][G_X]);
			iMax++;
		}
		if (DistanceSquared(p1, realFocus) > G_EPSILON) {
			pts[iMax] = A * (p1 - realFocus);
			angles[iMax] = GMath::Atan2(pts[iMax][G_Y], pts[iMax][G_X]);
			iMax++;
		}
		if (DistanceSquared(p2, realFocus) > G_EPSILON) {
			pts[iMax] = A * (p2 - realFocus);
			angles[iMax] = GMath::Atan2(pts[iMax][G_Y], pts[iMax][G_X]);
			iMax++;
		}
		if (DistanceSquared(p3, realFocus) > G_EPSILON) {
			pts[iMax] = A * (p3 - realFocus);
			angles[iMax] = GMath::Atan2(pts[iMax][G_Y], pts[iMax][G_X]);
			iMax++;
		}
		// iMax can be 3 or 4
		G_ASSERT(iMax >= 3);

		// sort by angles (ascending order)
		for (GUInt32 i = 0; i < iMax - 1; ++i) {
			for (GUInt32 j = i + 1; j < iMax; ++j) {
				if (angles[j] < angles[i]) {
					GReal angle = angles[j];
					angles[j] = angles[i];
					angles[i] = angle;
					p0 = pts[j];
					pts[j] = pts[i];
					pts[i] = p0;
				}
			}
		}

		pMin = pts[0];
		pMax = pts[iMax - 1];
		// it handles the case when the box intersect x-axis at the focus side (behind the focus); in this case
		// we must choose other points couple
		if (angles[0] < 0 && angles[iMax - 1] > 0) {
			if (Cross(pMin, pMax) < 0) {
				if (iMax == 4) {
					if (angles[1] < 0 && angles[2] > 0) {
						pMax = pts[1];
						pMin = pts[2];
					}
					else {
						if (angles[1] > 0) {
							pMin = pts[1];
							pMax = pts[0];
						}
						else {
							pMin = pts[3];
							pMax = pts[2];
						}
					}
				}
				else {
					G_ASSERT(iMax == 3);
					if (angles[1] > 0) {
						pMin = pts[1];
						pMax = pts[0];
					}
					else {
						pMin = pts[2];
						pMax = pts[1];
					}
				}
			}
		}
		// anti-transform "external" point into the original coordinate system
		Transpose(A, A);
		pMin = (A * pMin) + realFocus;
		pMax = (A * pMax) + realFocus;
	}

	// draw the shaded sector
	DrawGLRadialSector(transfCenter, realFocus, transfRadius, tMin, tMax, pMin, pMax, wholeDisk, ColorKeys,
					   Interpolation, SpreadMode, MultAlpha);
}

};	// end namespace Amanith
