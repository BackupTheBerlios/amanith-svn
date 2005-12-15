/****************************************************************************
** $file: amanith/geometry/gxformconv.h   0.2.0.0   edited Dec, 12 2005
**
** Conversion between transformation representations (quaternions, matrix and vectors)
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

#ifndef GXFORMCONV_H
#define GXFORMCONV_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gmatrix.h"
#include "amanith/geometry/gvect.h"
#include "amanith/geometry/gquat.h"
#include "amanith/gerror.h"

/*!
	\file gxformconv.h
	\brief Transformations conversion header file.
*/
namespace Amanith {
	   
	//! Build a translation matrix, setting last column with specified translation
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, GUInt32 TRANS_SIZE>
	void TranslationToMatrix(GMatrix<DATA_TYPE, ROWS, COLS>& Result,
							const GVectBase<DATA_TYPE, TRANS_SIZE>& Translation) {

		G_ASSERT((ROWS >= TRANS_SIZE) && (COLS >= TRANS_SIZE));
		Identity(Result);
		for (GUInt32 i = 0; i < TRANS_SIZE; ++i)
			Result[i][COLS - 1] = Translation[i];
	}
	
	/*!
		Construct matrix from Euler angles. For examples:
\code
	GMatrix33 m;

	// this rotation is a 0.5 radians around X axis, then 1.1 radians around Y axis and 0.2 radians around Z axis.
	RotationToMatrix(m, 0.5, 1.1, 0.2, G_ZYX, G_TRUE);

	// this rotation is a 0.5 radians around Z axis, then 1.1 radians around Y axis and 0.2 radians around X axis.
	RotationToMatrix(m, 0.5, 1, 0.2, G_ZYX, G_FALSE);
\endcode

		\param Result the result matrix
		\param RadAngle1 first rotation angle
		\param RadAngle2 second rotation angle
		\param RadAngle3 third rotation angle
		\param ApplicationOrder rotations application order.
		\param RightOrder specify if specified ApplicationOrder must be "seen" from right to left (G_TRUE value) or
		from left to right (G_FALSE value).
		\note Adapted from Graphics Gems IV "Euler Angle Conversion".
	*/
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void RotationToMatrix(GMatrix<DATA_TYPE, ROWS, COLS>& Result,
						  const GReal RadAngle1, const GReal RadAngle2, const GReal RadAngle3,
						  const GEulerOrder ApplicationOrder = G_ZYX,
						  const GBool RightOrder = G_TRUE) {

		GReal ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
		GReal ang1 = RadAngle1, ang2 = RadAngle2, ang3 = RadAngle3;
		GInt32 i, j, k, h, n, s, order;
		
		G_ASSERT((ROWS >= 3) && (COLS >= 3));
		Identity(Result);

		#define EulSafe "\000\001\002\000"
		#define EulNext	"\001\002\000\001"
		#define EulRepNo     0
		#define EulRepYes    1
		#define EulParEven   0
		#define EulParOdd    1

		// EulOrd creates an order value between 0 and 23 from 4-tuple choices
		#define EulOrd(i, p, r) (((((((i) << 1) + (p)) << 1) + (r)) << 1))

		// EulGetOrd unpacks all useful information about order simultaneously
		#define EulGetOrd(ord, i, j, k, h, n, s) { \
			unsigned o = ord; \
			o >>= 1; \
			s = o & 1; \
			o >>= 1; \
			n = o & 1; \
			o >>= 1; \
			i = EulSafe[o & 3]; \
			j = EulNext[i + n]; \
			k = EulNext[i + 1 - n]; \
			h = s ? k : i; \
		}

		if (RightOrder) {
			switch (ApplicationOrder) {
				case G_XYX: order = EulOrd(G_X, EulParEven, EulRepYes);
					break;
				case G_XYZ: order = EulOrd(G_Z, EulParOdd, EulRepNo);
					break;
				case G_XZX: order = EulOrd(G_X, EulParOdd, EulRepYes);
					break;
				case G_XZY: order = EulOrd(G_Y, EulParEven, EulRepNo);
					break;
				case G_YXY: order = EulOrd(G_Y, EulParOdd, EulRepYes);
					break;
				case G_YXZ: order = EulOrd(G_Z, EulParEven, EulRepNo);
					break;
				case G_YZX: order = EulOrd(G_X, EulParOdd, EulRepNo);
					break;
				case G_YZY: order = EulOrd(G_Y, EulParEven, EulRepYes);
					break;
				case G_ZXY: order = EulOrd(G_Y, EulParOdd, EulRepNo);
					break;
				case G_ZXZ: order = EulOrd(G_Z, EulParEven, EulRepYes);
					break;
				case G_ZYX: order = EulOrd(G_X, EulParEven, EulRepNo);
					break;
				case G_ZYZ: order = EulOrd(G_Z, EulParOdd, EulRepYes);
					break;
			}
		}
		else {
			switch (ApplicationOrder) {
				case G_XYX: order = EulOrd(G_X, EulParEven, EulRepYes);
					break;
				case G_XYZ: order = EulOrd(G_X, EulParEven, EulRepNo);
					break;
				case G_XZX: order = EulOrd(G_X, EulParOdd, EulRepYes);
					break;
				case G_XZY: order = EulOrd(G_X, EulParOdd, EulRepNo);
					break;
				case G_YXY: order = EulOrd(G_Y, EulParOdd, EulRepYes);
					break;
				case G_YXZ: order = EulOrd(G_Y, EulParOdd, EulRepNo);
					break;
				case G_YZX: order = EulOrd(G_Y, EulParEven, EulRepNo);
					break;
				case G_YZY: order = EulOrd(G_Y, EulParEven, EulRepYes);
					break;
				case G_ZXY: order = EulOrd(G_Z, EulParEven, EulRepNo);
					break;
				case G_ZXZ: order = EulOrd(G_Z, EulParEven, EulRepYes);
					break;
				case G_ZYX: order = EulOrd(G_Z, EulParOdd, EulRepNo);
					break;
				case G_ZYZ: order = EulOrd(G_Z, EulParOdd, EulRepYes);
					break;
			}
		}

		EulGetOrd(order, i, j, k, h, n, s);

		if (RightOrder) {
			GReal t = ang1;
			ang1 = ang3;
			ang3 = t;
		}
		if (n == EulParOdd) {
			ang1 = -ang1;
			ang2 = -ang2;
			ang3 = -ang3;
		}
		ti = ang1;
		tj = ang2;
		th = ang3;
		ci = GMath::Cos(ti);
		cj = GMath::Cos(tj);
		ch = GMath::Cos(th);
		si = GMath::Sin(ti);
		sj = GMath::Sin(tj);
		sh = GMath::Sin(th);
		cc = ci * ch;
		cs = ci * sh;
		sc = si * ch;
		ss = si * sh;
		if (s == EulRepYes) {
			Result[i][i] = cj;
			Result[i][j] = sj * si;
			Result[i][k] = sj * ci;
			Result[j][i] = sj * sh;
			Result[j][j] = -cj * ss + cc;
			Result[j][k] = -cj * cs - sc;
			Result[k][i] = -sj * ch;
			Result[k][j] =  cj * sc + cs;
			Result[k][k] =  cj * cc - ss;
		}
		else {
			Result[i][i] = cj * ch;
			Result[i][j] = sj * sc - cs;
			Result[i][k] = sj * cc + ss;
			Result[j][i] = cj * sh;
			Result[j][j] = sj * ss + cc;
			Result[j][k] = sj * cs - sc;
			Result[k][i] = -sj;
			Result[k][j] = cj * si;
			Result[k][k] = cj * ci;
		}
		#undef EulOrd
		#undef EulGetOrd
		#undef EulSafe
		#undef EulNext
		#undef EulRepNo
		#undef EulRepYes
		#undef EulParEven
		#undef EulParOdd
	}

	//! Build a 2D rotation matrix, specifying the ccw rotation amount (in radians).
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void RotationToMatrix(GMatrix<DATA_TYPE, ROWS, COLS>& Result, const GReal RadAngle) {

		G_ASSERT((ROWS >= 2) && (COLS >= 2));
		GReal c, s;

		Identity(Result);
		c = GMath::Cos(RadAngle);
		s = GMath::Sin(RadAngle);

		Result[0][0] = c;
		Result[0][1] = -s;
		Result[1][0] = s;
		Result[1][1] = c;
	}

	/*!
		Build a scale matrix, specifying scaling factors.

		\note Each scale factor is inserted respectively into a diagonal matrix position.
	*/
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, GUInt32 SIZE>
	void ScaleToMatrix(GMatrix<DATA_TYPE, ROWS, COLS>& Result,	const GVectBase<DATA_TYPE, SIZE>& ScaleFactors) {

		GUInt32 i, j = GMath::Min(ROWS, COLS, SIZE);

		Identity(Result);
		for (i = 0; i < j; ++i)
			Result[i][i] = ScaleFactors[i];
	}

	//! Build a uniform scale matrix.
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void ScaleToMatrix(GMatrix<DATA_TYPE, ROWS, COLS>& Result, const DATA_TYPE ScaleFactor) {

		GUInt32 i, j = GMath::Min(ROWS, COLS);

		Identity(Result);
		for (i = 0; i < j; ++i)
			Result[i][i] = ScaleFactor;
	}

	/*!
		Convert a matrix to a quaternion representation.
		Sets the rotation quaternion using the given matrix; this algorithm avoids
		near-zero divides by looking for a large component
		(3x3, 3x4, 4x3, or 4x4 are all valid sizes. In general n x m are valid if n and m are >= 3).
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void MatrixToRotation(GQuat<DATA_TYPE>& Result, const GMatrix<DATA_TYPE, ROWS, COLS>& M) {

		G_ASSERT (ROWS >= 3 && COLS >= 3);
		// this algorithm avoids near-zero divides by looking for a large component
		// first w, then x, y, or z.  When the trace is greater than zero,
		// |w| is greater than 1/2, which is as small as a largest component can be.
		// Otherwise, the largest diagonal entry corresponds to the largest of |x|,
		// |y|, or |z|, one of which must be larger than |w|, and at least 1/2

		DATA_TYPE tr(M(0, 0) + M(1, 1) + M(2, 2)), s = 0;

		// if diagonal is positive
		if (tr > 0) {
			s = GMath::Sqrt(tr + 1);
			Result[G_W] = s / (DATA_TYPE)2;
			s = (DATA_TYPE)((GReal)0.5 / s);
			Result[G_X] = (M(2, 1) - M(1, 2)) * s;
			Result[G_Y] = (M(0, 2) - M(2, 0)) * s;
			Result[G_Z] = (M(1, 0) - M(0, 1)) * s;
		}
		// diagonal is negative
		else {
			static const GUInt32 nxt[3] = { 1, 2, 0 };
			GUInt32 i = G_X, j, k;

			if (M(1, 1) > M(0, 0))
				i = 1;
			if (M(2, 2) > M(i, i))
				i = 2;
			j = nxt[i];
			k = nxt[j];
			s = GMath::Sqrt((M(i, i) - (M(j, j) + M(k, k))) + 1);

			DATA_TYPE q[4];
			q[i] = s / (DATA_TYPE)2;

			if (s != 0)
	            s = (DATA_TYPE)((GReal)0.5 / s);

			q[3] = (M(k, j) - M(j, k)) * s;
			q[j] = (M(j, i) + M(i, j)) * s;
			q[k] = (M(k, i) + M(i, k)) * s;
			Result[G_X] = q[0];
			Result[G_Y] = q[1];
			Result[G_Z] = q[2];
			Result[G_W] = q[3];
		}
	}

	//! Given a matrix extract the 2D rotation amount corresponding to the upper-leftmost 2x2 submatrix.
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	GError MatrixToRotation(DATA_TYPE& RadAngle, const GMatrix<DATA_TYPE, ROWS, COLS>& M) {

		G_ASSERT((ROWS >= 2) && (COLS >= 2));

		GReal c = (GReal)M[0][0];
		GReal s = (GReal)(M[1][0]);
		GReal det = c * (GReal)(M[1][1]) - (GReal)(M[0][1]) * s;

		if (GMath::Abs(det - (GReal)1) <= 2 * G_EPSILON) {
			RadAngle = (DATA_TYPE)(GMath::Acos(c) * GMath::Sign(s));
			return G_NO_ERROR;
		}
		else
			return G_INVALID_PARAMETER;
	}

	/*!
		Convert matrix to Euler angles (in radians).

		\note Adapted from Graphics Gems IV "Euler Angle Conversion".
	*/
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	GError MatrixToRotation(DATA_TYPE& RadAngle1, DATA_TYPE& RadAngle2, DATA_TYPE& RadAngle3,
							const GMatrix<DATA_TYPE, ROWS, COLS>& M, const GEulerOrder ApplicationOrder = G_ZYX,
							const GBool RightOrder = G_TRUE, const GBool DoConsistenceCheck = G_TRUE) {

		G_ASSERT((ROWS >= 3) && (COLS >= 3));

		GReal ang1, ang2, ang3, det;
		GInt32 i, j, k, h, n, s, order;

		#define EulSafe "\000\001\002\000"
		#define EulNext	"\001\002\000\001"
		#define EulRepNo     0
		#define EulRepYes    1
		#define EulParEven   0
		#define EulParOdd    1

		// EulOrd creates an order value between 0 and 23 from 4-tuple choices
		#define EulOrd(i, p, r) (((((((i) << 1) + (p)) << 1) + (r)) << 1))

		// EulGetOrd unpacks all useful information about order simultaneously
		#define EulGetOrd(ord, i, j, k, h, n, s) { \
			unsigned o = ord; \
			o >>= 1; \
			s = o & 1; \
			o >>= 1; \
			n = o & 1; \
			o >>= 1; \
			i = EulSafe[o & 3]; \
			j = EulNext[i + n]; \
			k = EulNext[i + 1 - n]; \
			h = s ? k : i; \
		}

		// check determinant of matrix (it must be = 1)
		if (DoConsistenceCheck) {
			GReal a00 = (GReal)M[0][0];
			GReal a01 = (GReal)M[0][1];
			GReal a02 = (GReal)M[0][2];
			GReal a10 = (GReal)M[1][0];
			GReal a11 = (GReal)M[1][1];
			GReal a12 = (GReal)M[1][2];
			GReal a20 = (GReal)M[2][0];
			GReal a21 = (GReal)M[2][1];
			GReal a22 = (GReal)M[2][2];

			GReal det = (a00 * a11 * a22) + (a01 * a12 * a20) + (a02 * a10 * a21);
			det += -(a00 * a12 * a21) - (a01 * a10 * a22) - (a02 * a11 * a20);
			if (GMath::Abs(det - (GReal)1) > 2 * G_EPSILON)
				return G_INVALID_PARAMETER;
		}

		if (RightOrder) {
			switch (ApplicationOrder) {
				case G_XYX: order = EulOrd(G_X, EulParEven, EulRepYes);
					break;
				case G_XYZ: order = EulOrd(G_Z, EulParOdd, EulRepNo);
					break;
				case G_XZX: order = EulOrd(G_X, EulParOdd, EulRepYes);
					break;
				case G_XZY: order = EulOrd(G_Y, EulParEven, EulRepNo);
					break;
				case G_YXY: order = EulOrd(G_Y, EulParOdd, EulRepYes);
					break;
				case G_YXZ: order = EulOrd(G_Z, EulParEven, EulRepNo);
					break;
				case G_YZX: order = EulOrd(G_X, EulParOdd, EulRepNo);
					break;
				case G_YZY: order = EulOrd(G_Y, EulParEven, EulRepYes);
					break;
				case G_ZXY: order = EulOrd(G_Y, EulParOdd, EulRepNo);
					break;
				case G_ZXZ: order = EulOrd(G_Z, EulParEven, EulRepYes);
					break;
				case G_ZYX: order = EulOrd(G_X, EulParEven, EulRepNo);
					break;
				case G_ZYZ: order = EulOrd(G_Z, EulParOdd, EulRepYes);
					break;
			}
		}
		else {
			switch (ApplicationOrder) {
				case G_XYX: order = EulOrd(G_X, EulParEven, EulRepYes);
					break;
				case G_XYZ: order = EulOrd(G_X, EulParEven, EulRepNo);
					break;
				case G_XZX: order = EulOrd(G_X, EulParOdd, EulRepYes);
					break;
				case G_XZY: order = EulOrd(G_X, EulParOdd, EulRepNo);
					break;
				case G_YXY: order = EulOrd(G_Y, EulParOdd, EulRepYes);
					break;
				case G_YXZ: order = EulOrd(G_Y, EulParOdd, EulRepNo);
					break;
				case G_YZX: order = EulOrd(G_Y, EulParEven, EulRepNo);
					break;
				case G_YZY: order = EulOrd(G_Y, EulParEven, EulRepYes);
					break;
				case G_ZXY: order = EulOrd(G_Z, EulParEven, EulRepNo);
					break;
				case G_ZXZ: order = EulOrd(G_Z, EulParEven, EulRepYes);
					break;
				case G_ZYX: order = EulOrd(G_Z, EulParOdd, EulRepNo);
					break;
				case G_ZYZ: order = EulOrd(G_Z, EulParOdd, EulRepYes);
					break;
			}
		}

		EulGetOrd(order, i, j, k, h, n, s);

		if (s == EulRepYes) {
			GReal sy = GMath::Sqrt((GReal)M[i][j] * M[i][j] + (GReal)M[i][k] * M[i][k]);
			if (sy > 16 * G_EPSILON) {
				ang1 = GMath::Atan2((GReal)M[i][j], (GReal)M[i][k]);
				ang2 = GMath::Atan2(sy, (GReal)M[i][i]);
				ang3 = GMath::Atan2((GReal)M[j][i], (GReal)-M[k][i]);
			}
			else {
				ang1 = GMath::Atan2((GReal)-M[j][k], (GReal)M[j][j]);
				ang2 = GMath::Atan2(sy, (GReal)M[i][i]);
				ang3 = 0;
			}
		}
		else {
			GReal cy = GMath::Sqrt((GReal)M[i][i] * M[i][i] + (GReal)M[j][i] * M[j][i]);
			if (cy > 16 * G_EPSILON) {
				ang1 = GMath::Atan2((GReal)M[k][j], (GReal)M[k][k]);
				ang2 = GMath::Atan2((GReal)-M[k][i], cy);
				ang3 = GMath::Atan2((GReal)M[j][i], (GReal)M[i][i]);
			}
			else {
				ang1 = GMath::Atan2((GReal)-M[j][k], (GReal)M[j][j]);
				ang2 = GMath::Atan2((GReal)-M[k][i], cy);
				ang3 = 0;
			}
		}
		if (n == EulParOdd) {
			ang1 = -ang1;
			ang2 = -ang2;
			ang3 = -ang3;
		}
		if (RightOrder) {
			GReal t = ang1;
			ang1 = ang3;
			ang3 = t;
		}
		RadAngle1 = (DATA_TYPE)ang1;
		RadAngle2 = (DATA_TYPE)ang2;
		RadAngle3 = (DATA_TYPE)ang3;
		return G_NO_ERROR;
		#undef EulOrd
		#undef EulGetOrd
		#undef EulSafe
		#undef EulNext
		#undef EulRepNo
		#undef EulRepYes
		#undef EulParEven
		#undef EulParOdd
	}

	/*!
		Build a 2D shear matrix, specifying shear factors. Result matrix must have at least 2 rows and 2
		columns. Shear factors are inserted respectively into [0][1] position ShearX, and into [1][0] position
		ShearY, see the picture below:\n

		\f[
		Result = \left[ \begin{array}{ccc}
		1 & \mbox{ShearX} & \mbox{...} \\
		\mbox{ShearY} & 1 & \mbox{...} \\
		\mbox{...} & \mbox{...} & \mbox{...} \end{array} \right]
		\f]
		\note Result matrix is first set as an identity, then shear factors are inserted.
	*/
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void Shear2DToMatrix(GMatrix<DATA_TYPE, ROWS, COLS>& Result, const DATA_TYPE ShearX, const DATA_TYPE ShearY) {

		G_ASSERT(ROWS >= 2 && COLS >= 2);

		Identity(Result);
		Result[0][1] = ShearX;
		Result[1][0] = ShearY;
	}

	/*!
		Build a 3D shear matrix, specifying shear factors and their respective axes. Result matrix must have at
		least 3 rows and 3 columns. Shear factors are inserted according to specified application axes, see the
		picture belove for the possible 3 cases:\n

		\f[
		X, Y = \left[ \begin{array}{cccc}
		1 & 0 & \mbox{Shear0} & \mbox{...} \\
		0 & 1 & \mbox{Shear1} & \mbox{...} \\
		0 & 0 & 1 & \mbox{...} \end{array} \right]
		\f]

		\f[
		X, Z =  \left[ \begin{array}{cccc}
		1 & \mbox{Shear0} & 0 & \mbox{...} \\
		0 & 1 & 0 & \mbox{...} \\
		0 & \mbox{Shear1} & 1 & \mbox{...} \end{array} \right]
		\f]

		\f[
		Y, Z = \left[ \begin{array}{cccc}
		1 & 0 & 0 & \mbox{...} \\
		\mbox{Shear0} & 1 & 0 & \mbox{...} \\
		\mbox{Shear1} & 0 & 1 & \mbox{...} \end{array} \right]
		\f]
		\note Result matrix is first set as an identity, then shear factors are inserted.
	*/
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void Shear3DToMatrix(GMatrix<DATA_TYPE, ROWS, COLS>& Result, const GVectorIndex Axis0, const DATA_TYPE Shear0,
						 const GVectorIndex Axis1, const DATA_TYPE Shear1) {

		G_ASSERT(ROWS >= 3 && COLS >= 3);
		G_ASSERT(Axis0 != Axis1);

		GVectorIndex i0, i1;
		DATA_TYPE s0, s1;
		Identity(Result);

		if (Axis0 > Axis1) {
			i0 = Axis1;
			i1 = Axis0;
			s0 = Shear1;
			s1 = Shear0;
		}
		else {
			i0 = Axis0;
			i1 = Axis1;
			s0 = Shear0;
			s1 = Shear1;
		}

		if (Axis0 == G_X && Axis1 == G_Y) {
			Result[G_X][G_Z] = s0;
			Result[G_Y][G_Z] = s1;
		}
		else
		if (Axis0 == G_X && Axis1 == G_Z) {
			Result[G_X][G_Y] = s0;
			Result[G_Z][G_Y] = s1;
		}
		if (Axis0 == G_Y && Axis1 == G_Z) {
			Result[G_Y][G_X] = s0;
			Result[G_Z][G_X] = s1;
		}
	}

};	// end namespace Amanith

#endif
