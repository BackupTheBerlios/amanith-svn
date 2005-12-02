/****************************************************************************
** $file: amanith/src/rendering/gopenglconicalgrad.cpp   0.1.1.0   edited Sep 24 08:00
**
** OpenGL based draw board conical gradient functions implementation.
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
	\file gopenglconicalgrad.cpp
	\brief OpenGL based draw board conical gradient functions implementation file.
*/

namespace Amanith {

GPoint2 GOpenGLBoard::DrawGLConicalSlice(const GPoint2& P0, const GPoint2& Center, const GReal Radius,
										 const GVector4 Col0, const GVector4 Col1,
										 const GVector4 Tan0, const GVector4 Tan1,
										 const GReal SpanAngle, const GReal Flatness,
										 const GColorRampInterpolation Interpolation) const {

	if (SpanAngle <= 0)
		return P0;

	GInt32 n = 3, k;
	GReal n1 = SpanAngle / (2 * GMath::Acos(1 - Flatness / Radius));
	if (n1 > 3 && n1 >= (SpanAngle * G_ONE_OVER_PI)) {
		if (n1 > n) \
			n = (GUInt32)GMath::Ceil(n1);
	}
	GReal cosDelta = GMath::Cos(SpanAngle / n);
	GReal sinDelta = GMath::Sin(SpanAngle / n);
	GReal t, dt, t2, t3, h1, h2, h3, h4;
	GPoint2 p = P0, q, mp = P0 + Center, mq;
	GVector4 oldCol, col, col01;

	switch (Interpolation) {

		case G_CONSTANT_COLOR_INTERPOLATION:
			for (k = 0; k < n; k++) {
				q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
				mq = q + Center;
				// triangle mp, mq, Center
				#ifdef DOUBLE_REAL_TYPE
					glColor4dv(Col0.Data());
					glVertex2dv(mp.Data());
					glVertex2dv(mq.Data());
					glVertex2dv(Center.Data());
				#else
					glColor4fv(Col0.Data());
					glVertex2fv(mp.Data());
					glVertex2fv(mq.Data());
					glVertex2fv(Center.Data());
				#endif
				p = q;
				mp = mq;
			}
			break;

		case G_LINEAR_COLOR_INTERPOLATION:

			dt = (GReal)1 / (GReal)n;
			t = dt;
			oldCol = Col0;
			col01 = (Col1 - Col0);

			for (k = 0; k < n; k++) {
				q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
				col = Col0 + t * col01;
				mq = q + Center;
				// triangle mp, mq, Center (oldCol, col)
				#ifdef DOUBLE_REAL_TYPE
					glColor4dv(oldCol.Data());
					glVertex2dv(mp.Data());
					glColor4dv(col.Data());
					glVertex2dv(mq.Data());
					glVertex2dv(Center.Data());
				#else
					glColor4fv(oldCol.Data());
					glVertex2fv(mp.Data());
					glColor4fv(col.Data());
					glVertex2fv(mq.Data());
					glVertex2fv(Center.Data());
				#endif
				t += dt;
				p = q;
				mp = mq;
				oldCol = col;
			}
			break;

		case G_HERMITE_COLOR_INTERPOLATION:

			dt = (GReal)1 / (GReal)n;
			t = dt;
			oldCol = Col0;

			for (k = 0; k < n; k++) {
				t2 = t * t;
				t3 = t2 * t;
				// Hermite basis functions
				h1 =  2 * t3 - 3 * t2 + 1;
				h2 = -2 * t3 + 3 * t2;
				h3 = t3 - 2 * t2 + t;
				h4 = t3 -  t2;
				col = (h1 * Col0) + (h2 * Col1) + (h3 * Tan0) + (h4 * Tan1);
				q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
				mq = q + Center;
				// triangle mp, mq, Center (oldCol, col)
				#ifdef DOUBLE_REAL_TYPE
					glColor4dv(oldCol.Data());
					glVertex2dv(mp.Data());
					glColor4dv(col.Data());
					glVertex2dv(mq.Data());
					glVertex2dv(Center.Data());
				#else
					glColor4fv(oldCol.Data());
					glVertex2fv(mp.Data());
					glColor4fv(col.Data());
					glVertex2fv(mq.Data());
					glVertex2fv(Center.Data());
				#endif
				t += dt;
				p = q;
				mp = mq;
				oldCol = col;
			}
			break;
	}
	return p;
}

void GOpenGLBoard::DrawGLConicalSector(const GPoint2& Center, const GVector2& DirCenterTarget, const GReal Radius,
									   const GPoint2& P0, const GPoint2& P1, const GBool WholeDisk,
									   const GDynArray<GKeyValue>& ColorKeys,
									   const GDynArray<GVector4>& InTangents, const GDynArray<GVector4>& OutTangents,
									   const GColorRampInterpolation Interpolation, const GReal MultAlpha) const {

	G_ASSERT(Radius > G_EPSILON);
	GReal spanAngle = 0;

	// now calculate the number of segments to produce (number of times we have to subdivide angle) that
	// permit to have a squared chordal distance less than gDeviation
	GReal dev = GMath::Clamp(gFlateness, G_EPSILON, Radius - (G_EPSILON * Radius));

	if (RenderingQuality() == G_HIGH_RENDERING_QUALITY)
		dev *= 0.5;
	else
	if (RenderingQuality() == G_LOW_RENDERING_QUALITY)
		dev *= 1.5;


	GReal deltaTime, deltaAngle, n1, cosDelta, sinDelta, ang0, ang1, angP0, angP1;
	GReal t, dt, t2, t3, h1, h2, h3, h4;
	GKeyValue k0, k1;
	GInt32 i, j, k, n;
	GPoint2 p, q, mp, mq;
	GVector4 col0, col1, col, col01, oldCol, tan0, tan1;
	j = (GInt32)ColorKeys.size();
	G_ASSERT(j >= 2);
	deltaTime = ColorKeys.back().TimePosition() - ColorKeys.front().TimePosition();
	p = Radius * DirCenterTarget;
	mp = p + Center;

	glBegin(GL_TRIANGLES);

	if (WholeDisk) {
		spanAngle = (GReal)G_2PI;
		for (k = 0; k < j - 1; ++k) {

			k0 = ColorKeys[k];
			k1 = ColorKeys[k + 1];
			ang0 = (spanAngle / deltaTime) * k0.TimePosition();
			ang1 = (spanAngle / deltaTime) * k1.TimePosition();

			col0 = k0.Vect4Value();
			col1 = k1.Vect4Value();
			if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
				tan0 = OutTangents[k];
				tan1 = InTangents[k + 1];
				tan0[G_W] *= MultAlpha;
				tan1[G_W] *= MultAlpha;
			}
			col0[G_W] *= MultAlpha;
			col1[G_W] *= MultAlpha;
			col01 = col1 - col0;

			deltaAngle = ang1 - ang0;
			n = 3;
			n1 = deltaAngle / (2 * GMath::Acos(1 - dev / Radius));
			if (n1 > 3 && n1 >= (deltaAngle * G_ONE_OVER_PI)) {
				if (n1 > n)
					n = (GUInt32)GMath::Ceil(n1);
			}
			cosDelta = GMath::Cos(deltaAngle / n);
			sinDelta = GMath::Sin(deltaAngle / n);

			switch (Interpolation) {

				case G_CONSTANT_COLOR_INTERPOLATION:

					#ifdef DOUBLE_REAL_TYPE
						glColor4dv(col0.Data());
					#else
						glColor4fv(col0.Data());
					#endif

					if (k == j - 2)
						n--;

					for (i = 0; i < n; ++i) {
						q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
						//mp = p + Center;
						mq = q + Center;
						#ifdef DOUBLE_REAL_TYPE
							glVertex2dv(Center.Data());
							glVertex2dv(mp.Data());
							glVertex2dv(mq.Data());
						#else
							glVertex2fv(Center.Data());
							glVertex2fv(mp.Data());
							glVertex2fv(mq.Data());
						#endif
						p = q;
						mp = mq;
					}
					// just to avoid numerical imprecision
					if (k == j - 2) {
						//mp = p + Center;
						q = Radius * DirCenterTarget;
						mq = q + Center;
						#ifdef DOUBLE_REAL_TYPE
							glVertex2dv(Center.Data());
							glVertex2dv(mp.Data());
							glVertex2dv(mq.Data());
						#else
							glVertex2fv(Center.Data());
							glVertex2fv(mp.Data());
							glVertex2fv(mq.Data());
						#endif
					}
					break;

				case G_LINEAR_COLOR_INTERPOLATION:

					dt = (GReal)1 / (GReal)n;
					t = dt;
					oldCol = col0;

					if (k == j - 2)
						n--;

					for (i = 0; i < n; ++i) {
						q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
						//mp = p + Center;
						mq = q + Center;
						col = col0 + t * col01;

						#ifdef DOUBLE_REAL_TYPE
							glColor4dv(oldCol.Data());
							glVertex2dv(mp.Data());
							glColor4dv(col.Data());
							glVertex2dv(mq.Data());
							glVertex2dv(Center.Data());
						#else
							glColor4fv(oldCol.Data());
							glVertex2fv(mp.Data());
							glColor4fv(col.Data());
							glVertex2fv(mq.Data());
							glVertex2fv(Center.Data());
						#endif
						p = q;
						mp = mq;
						t += dt;
						oldCol = col;
					}
					// just to avoid numerical imprecision
					if (k == j - 2) {
						//mp = p + Center;
						q = Radius * DirCenterTarget;
						mq = q + Center;
						#ifdef DOUBLE_REAL_TYPE
							glColor4dv(oldCol.Data());
							glVertex2dv(mp.Data());
							glColor4dv(col1.Data());
							glVertex2dv(mq.Data());
							glVertex2dv(Center.Data());
						#else
							glColor4fv(oldCol.Data());
							glVertex2fv(mp.Data());
							glColor4fv(col1.Data());
							glVertex2fv(mq.Data());
							glVertex2fv(Center.Data());
						#endif
					}
					break;

				case G_HERMITE_COLOR_INTERPOLATION:

					dt = (GReal)1 / (GReal)n;
					t = dt;
					oldCol = col0;

					if (k == j - 2)
						n--;

					for (i = 0; i < n; ++i) {
						q.Set(p[G_X] * cosDelta - p[G_Y] * sinDelta, p[G_Y] * cosDelta + p[G_X] * sinDelta);
						//mp = p + Center;
						mq = q + Center;

						t2 = t * t;
						t3 = t2 * t;
						h1 =  2 * t3 - 3 * t2 + 1;
						h2 = -2 * t3 + 3 * t2;
						h3 = t3 - 2 * t2 + t;
						h4 = t3 -  t2;
						col = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
						#ifdef DOUBLE_REAL_TYPE
							glColor4dv(oldCol.Data());
							glVertex2dv(mp.Data());
							glColor4dv(col.Data());
							glVertex2dv(mq.Data());
							glVertex2dv(Center.Data());
						#else
							glColor4fv(oldCol.Data());
							glVertex2fv(mp.Data());
							glColor4fv(col.Data());
							glVertex2fv(mq.Data());
							glVertex2fv(Center.Data());
						#endif
						p = q;
						mp = mq;
						t += dt;
						oldCol = col;
					}
					// just to avoid numerical imprecision
					if (k == j - 2) {
						//mp = p + Center;
						q = Radius * DirCenterTarget;
						mq = q + Center;
						#ifdef DOUBLE_REAL_TYPE
							glColor4dv(oldCol.Data());
							glVertex2dv(mp.Data());
							glColor4dv(col1.Data());
							glVertex2dv(mq.Data());
							glVertex2dv(Center.Data());
						#else
							glColor4fv(oldCol.Data());
							glVertex2fv(mp.Data());
							glColor4fv(col1.Data());
							glVertex2fv(mq.Data());
							glVertex2fv(Center.Data());
						#endif
					}
					break;
			}
		}
	}
	else {
		GReal t0, t1, spanAngle0, spanAngle1 = 0;
		GInt32 i0, i1;
		// perpendicular direction in CCW respect to DirCenterTarget
		GVector2 perpDirCT(-DirCenterTarget[G_Y], DirCenterTarget[G_X]);
		GVector2 p0 = P0 - Center;
		GVector2 p1 = P1 - Center;
		p0.Set(Dot(p0, DirCenterTarget), Dot(p0, perpDirCT));
		p1.Set(Dot(p1, DirCenterTarget), Dot(p1, perpDirCT));

		// calculate angles relative to P0 and P1 (in the range [0; 2PI]
		angP0 = GMath::Atan2(p0[G_Y], p0[G_X]);
		angP1 = GMath::Atan2(p1[G_Y], p1[G_X]);
		if (angP0 < 0)
			angP0 += (GReal)G_2PI;
		if (angP1 < 0)
			angP1 += (GReal)G_2PI;

		// find first sector greater or equal to angP0
		i0 = 0;
		do {
			k1 = ColorKeys[i0];
			ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
			i0++;
		} while (i0 < j && ang1 <= angP0);
		i1 = i0;
		i0--;
		k0 = ColorKeys[i0 - 1];
		ang0 = ((GReal)G_2PI / deltaTime) * k0.TimePosition();
		spanAngle0 = ang1 - angP0;
		t0 = (angP0 - ang0) / (ang1 - ang0);

		if (i0 == j - 1 && angP0 > angP1) {
			ang1 = 0;
			i1 = 0;
		}

		// check if the key contains also max point
		if (ang1 >= angP1) {
			i1 = i0;
			spanAngle = ang1 - ang0;
			col0 = k0.Vect4Value();
			col1 = k1.Vect4Value();
			col0[G_W] *= MultAlpha;
			col1[G_W] *= MultAlpha;
			if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
				tan0 = OutTangents[i0 - 1];
				tan1 = InTangents[i0];
				tan0[G_W] *= MultAlpha;
				tan1[G_W] *= MultAlpha;
			}
			ang1 = GMath::Atan2(DirCenterTarget[G_Y], DirCenterTarget[G_X]);
			p.Set(Radius * GMath::Cos(ang0 + ang1), Radius * GMath::Sin(ang0 + ang1));
		}
		else {
			// find first sector greater or equal to angP1
			i1 = i1 % j;
			do {
				k1 = ColorKeys[i1];
				if (i1 == j - 1 && angP0 > angP1) {
					ang1 = 0;
					i1 = 0;
				}
				else
					ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
				i1++;
			} while (i1 < j && ang1 <= angP1);
			i1--;
			k0 = ColorKeys[i1 - 1];
			ang0 = ((GReal)G_2PI / deltaTime) * k0.TimePosition();
			spanAngle1 = angP1 - ang0;
			t1 = (angP1 - ang0) / (ang1 - ang0);
		}

		if (i0 == i1) {
			DrawGLConicalSlice(p, Center, Radius, col0, col1, tan0, tan1, spanAngle, dev, Interpolation);
		}
		else
		// this is the case where "wrap" do not occur
		if (i0 < i1) {

			// calculate first sub-sector
			k0 = ColorKeys[i0 - 1];
			ang0 = angP0;
			k1 = ColorKeys[i0];
			ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
			spanAngle = spanAngle0;
			if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
				tan0 = OutTangents[i0 - 1];
				tan1 = InTangents[i0];
				tan0[G_W] *= MultAlpha;
				tan1[G_W] *= MultAlpha;
			}
			col0 = k0.Vect4Value();
			col1 = k1.Vect4Value();
			col0[G_W] *= MultAlpha;
			col1[G_W] *= MultAlpha;
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
					col0 = col0 + t0 * (col1 - col0);
					break;
				case G_HERMITE_COLOR_INTERPOLATION:
					// calculate interpolated color
					t2 = t0 * t0;
					t3 = t2 * t0;
					h1 =  2 * t3 - 3 * t2 + 1;
					h2 = -2 * t3 + 3 * t2;
					h3 = t3 - 2 * t2 + t0;
					h4 = t3 -  t2;
					col0 = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
					// calculate interpolated tangent
					h1 = 6 * t2 - 6 * t0;
					h2 = -h1;
					h3 = 3 * t2 - 4 * t0 + 1;
					h4 = 3 * t2 - 2 * t0;
					tan0 = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
					break;
			}
			GVector2 vTmp = P0 - Center;
			vTmp.Normalize();
			p = vTmp * Radius;

			// draw all "in between" sectors
			for (i = i0; i < i1; i++) {
				p = DrawGLConicalSlice(p, Center, Radius, col0, col1, tan0, tan1, spanAngle, dev, Interpolation);
				// next sector
				k0 = k1;
				col0 = col1;
				if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
					tan0 = OutTangents[i];
					tan0[G_W] *= MultAlpha;
				}
				ang0 = ang1;
				if (i < j - 1) {
					k1 = ColorKeys[i + 1];
					ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
					col1 = k1.Vect4Value();
					col1[G_W] *= MultAlpha;
					if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
						tan1 = InTangents[i + 1];
						tan1[G_W] *= MultAlpha;
					}
					spanAngle = ang1 - ang0;
				}
			}
			// draw last sub-sector
			k1 = ColorKeys[i1];
			ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
			spanAngle = spanAngle1;
			col1 = k1.Vect4Value();
			col1[G_W] *= MultAlpha;
			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
					col1 = col0 + t1 * (col1 - col0);
					break;
				case G_HERMITE_COLOR_INTERPOLATION:
					tan0 = OutTangents[i1 - 1];
					tan1 = InTangents[i1];
					tan0[G_W] *= MultAlpha;
					tan1[G_W] *= MultAlpha;
					// calculate interpolated color
					t2 = t1 * t1;
					t3 = t2 * t1;
					h1 =  2 * t3 - 3 * t2 + 1;
					h2 = -2 * t3 + 3 * t2;
					h3 = t3 - 2 * t2 + t1;
					h4 = t3 -  t2;
					col1 = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
					// calculate interpolated tangent
					h1 = 6 * t2 - 6 * t1;
					h2 = -h1;
					h3 = 3 * t2 - 4 * t1 + 1;
					h4 = 3 * t2 - 2 * t1;
					tan1 = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
					break;
			}
			p = DrawGLConicalSlice(p, Center, Radius, col0, col1, tan0, tan1, spanAngle, dev, Interpolation);
		}
		else {

			// calculate first sub-sector
			k0 = ColorKeys[i0 - 1];
			ang0 = angP0;
			k1 = ColorKeys[i0];
			ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
			spanAngle = spanAngle0;
			if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
				tan0 = OutTangents[i0 - 1];
				tan1 = InTangents[i0];
				tan0[G_W] *= MultAlpha;
				tan1[G_W] *= MultAlpha;
			}
			col0 = k0.Vect4Value();
			col1 = k1.Vect4Value();
			col0[G_W] *= MultAlpha;
			col1[G_W] *= MultAlpha;

			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
					col0 = col0 + t0 * (col1 - col0);
					break;
				case G_HERMITE_COLOR_INTERPOLATION:
					// calculate interpolated color
					t2 = t0 * t0;
					t3 = t2 * t0;
					h1 =  2 * t3 - 3 * t2 + 1;
					h2 = -2 * t3 + 3 * t2;
					h3 = t3 - 2 * t2 + t0;
					h4 = t3 -  t2;
					col0 = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
					// calculate interpolated tangent
					h1 = 6 * t2 - 6 * t0;
					h2 = -h1;
					h3 = 3 * t2 - 4 * t0 + 1;
					h4 = 3 * t2 - 2 * t0;
					tan0 = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
					break;
			}
			GVector2 vTmp = P0 - Center;
			vTmp.Normalize();
			p = vTmp * Radius;

			// draw all "in between" sectors
			for (i = i0; i < j; i++) {
				p = DrawGLConicalSlice(p, Center, Radius, col0, col1, tan0, tan1, spanAngle, dev, Interpolation);
				// next sector
				k0 = k1;
				col0 = col1;
				if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
					tan0 = OutTangents[i];
					tan0[G_W] *= MultAlpha;
				}
				ang0 = ang1;
				if (i < j - 1) {
					k1 = ColorKeys[i + 1];
					ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
					col1 = k1.Vect4Value();
					col1[G_W] *= MultAlpha;
					if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
						tan1 = InTangents[i + 1];
						tan1[G_W] *= MultAlpha;
					}
					spanAngle = ang1 - ang0;
				}
			}

			k0 = ColorKeys[0];
			k1 = ColorKeys[1];
			ang0 = ((GReal)G_2PI / deltaTime) * k0.TimePosition();
			ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
			col0 = k0.Vect4Value();
			col1 = k1.Vect4Value();
			col0[G_W] *= MultAlpha;
			col1[G_W] *= MultAlpha;
			if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
				tan0 = OutTangents[0];
				tan1 = InTangents[1];
				tan0[G_W] *= MultAlpha;
				tan1[G_W] *= MultAlpha;
			}

			spanAngle = ang1 - ang0;
			// draw all "out between" sectors
			for (i = 0; i < i1 - 1; i++) {
				p = DrawGLConicalSlice(p, Center, Radius, col0, col1, tan0, tan1, spanAngle, dev, Interpolation);
				// next sector
				k0 = k1;
				col0 = col1;
				if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
					tan0 = OutTangents[i];
					tan0[G_W] *= MultAlpha;
				}
				ang0 = ang1;
				if (i < j - 1) {
					k1 = ColorKeys[i + 1];
					ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
					col1 = k1.Vect4Value();
					col1[G_W] *= MultAlpha;
					if (Interpolation == G_HERMITE_COLOR_INTERPOLATION) {
						tan1 = InTangents[i + 1];
						tan1[G_W] *= MultAlpha;
					}
					spanAngle = ang1 - ang0;
				}
			}

			// draw last sub-sector
			k1 = ColorKeys[i1];
			ang1 = ((GReal)G_2PI / deltaTime) * k1.TimePosition();
			spanAngle = spanAngle1;
			col1 = k1.Vect4Value();
			col1[G_W] *= MultAlpha;

			switch (Interpolation) {
				case G_CONSTANT_COLOR_INTERPOLATION:
					break;
				case G_LINEAR_COLOR_INTERPOLATION:
					col1 = col0 + t1 * (col1 - col0);
					break;
				case G_HERMITE_COLOR_INTERPOLATION:
					tan0 = OutTangents[i1 - 1];
					tan1 = InTangents[i1];
					tan0[G_W] *= MultAlpha;
					tan1[G_W] *= MultAlpha;
					// calculate interpolated color
					t2 = t1 * t1;
					t3 = t2 * t1;
					h1 =  2 * t3 - 3 * t2 + 1;
					h2 = -2 * t3 + 3 * t2;
					h3 = t3 - 2 * t2 + t1;
					h4 = t3 -  t2;
					col1 = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
					// calculate interpolated tangent
					h1 = 6 * t2 - 6 * t1;
					h2 = -h1;
					h3 = 3 * t2 - 4 * t1 + 1;
					h4 = 3 * t2 - 2 * t1;
					tan1 = (h1 * col0) + (h2 * col1) + (h3 * tan0) + (h4 * tan1);
					break;
			}
			p = DrawGLConicalSlice(p, Center, Radius, col0, col1, tan0, tan1, spanAngle, dev, Interpolation);
		}
	}

	glEnd();
}

void GOpenGLBoard::DrawConicalSector(const GPoint2& Center, const GPoint2& Target, const GAABox2& BoundingBox,
									const GDynArray<GKeyValue>& ColorKeys,
									const GDynArray<GVector4>& InTangents, const GDynArray<GVector4>& OutTangents,
									const GColorRampInterpolation Interpolation,
									const GReal MultAlpha, const GMatrix33& GradientMatrix) const {

	GPoint2 transfCenter = GradientMatrix * Center;
	GPoint2 transfTarget = GradientMatrix * Target;

	GVector2 dirCT = transfTarget - transfCenter;
	GReal dirCTlen = dirCT.Length();

	// calculate normalized direction
	if (dirCTlen <= G_EPSILON)
		dirCT.Set(1, 0);
	else
		dirCT /= dirCTlen;

	GPoint2 pMin, pMax;
	GPoint2 p0 = BoundingBox.Min();
	GPoint2 p2 = BoundingBox.Max();
	GPoint2 p1(p2[G_X], p0[G_Y]);
	GPoint2 p3(p0[G_X], p2[G_Y]);

	GReal dMax, d;
	GBool wholeDisk = G_FALSE;

	// calculate the maximum radius
	dMax = DistanceSquared(p0, transfCenter);
	d = DistanceSquared(p1, transfCenter);
	if (d > dMax)
		dMax = d;
	d = DistanceSquared(p2, transfCenter);
	if (d > dMax)
		dMax = d;
	d = DistanceSquared(p3, transfCenter);
	if (d > dMax)
		dMax = d;
	dMax = GMath::Sqrt(dMax);

	if (dMax <= G_EPSILON)
		return;


	// if the focus is inside the box, whole disk must be rendered (and in this case tMin is 0)
	if ((transfCenter[G_X] > p0[G_X] && transfCenter[G_X] < p2[G_X]) &&
		(transfCenter[G_Y] > p0[G_Y] && transfCenter[G_Y] < p2[G_Y]))
		wholeDisk = G_TRUE;
	else {
		// calculate axes transformation (transform box corners int the coordinate system given by Center-Target
		// direction
		GMatrix22 A;
		GUInt32 iMax = 0;
		A.Set(dirCT[G_X], dirCT[G_Y], -dirCT[G_Y], dirCT[G_X]);

		GReal angles[4];
		GPoint2 pts[4];

		if (DistanceSquared(p0, transfCenter) > G_EPSILON) {
			pts[iMax] = A * (p0 - transfCenter);
			angles[iMax] = GMath::Atan2(pts[iMax][G_Y], pts[iMax][G_X]);
			iMax++;
		}
		if (DistanceSquared(p1, transfCenter) > G_EPSILON) {
			pts[iMax] = A * (p1 - transfCenter);
			angles[iMax] = GMath::Atan2(pts[iMax][G_Y], pts[iMax][G_X]);
			iMax++;
		}
		if (DistanceSquared(p2, transfCenter) > G_EPSILON) {
			pts[iMax] = A * (p2 - transfCenter);
			angles[iMax] = GMath::Atan2(pts[iMax][G_Y], pts[iMax][G_X]);
			iMax++;
		}
		if (DistanceSquared(p3, transfCenter) > G_EPSILON) {
			pts[iMax] = A * (p3 - transfCenter);
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
		pMin = (A * pMin) + transfCenter;
		pMax = (A * pMax) + transfCenter;
	}

	// draw the shaded sector (we have to expand by 5% radius, to avoid low quality related issues; NB: the radius
	// DO NOT influences performances)
	DrawGLConicalSector(transfCenter, dirCT, dMax * (GReal)1.05, pMin, pMax, wholeDisk, ColorKeys,
						InTangents, OutTangents, Interpolation, MultAlpha);
}

};	// end namespace Amanith
