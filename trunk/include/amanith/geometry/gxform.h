/****************************************************************************
** $file: amanith/geometry/gxform.h   0.2.0.0   edited Dec, 12 2005
**
** Transformations / interaction between quaternions, matrix and vectors
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

#ifndef GXFORM_H
#define GXFORM_H

/*!
	\file gxform.h
	\brief Header file for transformations (vector, matrix and quaternions).

	Due to a <b>doxygen bug</b>, a lot of similar function (same name but different parameters) are present
	into this file, but they cannot be documented. Please refer to source file for the full list of functions
	available.
*/

#include "amanith/gglobal.h"
#include "amanith/geometry/gmatrix.h"
#include "amanith/geometry/gvect.h"
#include "amanith/geometry/gquat.h"
#include "amanith/geometry/glineseg.h"

namespace Amanith {
	   
	/*
		Transform a vector by a rotation quaternion.
		\pre give a vector, and a rotation quaternion (by definition, a rotation quaternion is normalized).
		\param result the vector to write the result into
		\param rot the quaternion
		\param vector the original vector to transform
		\post v' = q P(v) q*  (where result is v', rot is q, and vector is v. q* is conj(q), and P(v) is pure quaternion
		made from v)
		\see game programming gems #1 p199
		\see Shoemake Siggraph notes.
		\note for the implementation, inv and conj should both work for the "q*" in "Rv = q P(v) q*"
		but conj is actually faster so we usually choose that.
		note also note, that if the input quat wasn't normalized (and thus isn't a rotation quat),
		then this might not give the correct result, since conj and invert is only equiv when normalized...
	*/
	template <typename DATA_TYPE>
	inline GVect<DATA_TYPE, 3>& XForm(GVect<DATA_TYPE, 3>& result, const GQuat<DATA_TYPE>& rot,
									  const GVect<DATA_TYPE, 3>& vector) {
		// check preconditions...
		G_ASSERT(GMath::IsEqual(Length(rot), (DATA_TYPE)1.0, (DATA_TYPE)G_EPSILON) && "must pass a rotation quaternion to xform(result,quat,vec) - by definition, a rotation quaternion is normalized).  if you need non-rotation quaternion support, let us know.");

		// easiest to write and understand (slowest too)
		// return result_vec = makeVec(rot * makePure(vector) * makeConj(rot));

		// completely hand expanded
		// (faster by 28% in gcc 2.96 debug mode.)
		// (faster by 35% in gcc 2.96 opt3 mode (78% for doubles))
		GQuat<DATA_TYPE> rot_conj(-rot[G_X], -rot[G_Y], -rot[G_Z], rot[G_W]);
		GQuat<DATA_TYPE> pure(vector[0], vector[1], vector[2], (DATA_TYPE)0.0);
		GQuat<DATA_TYPE> temp(
			pure[G_W] * rot_conj[G_X] + pure[G_X] * rot_conj[G_W] + pure[G_Y] * rot_conj[G_Z] - pure[G_Z] * rot_conj[G_Y],
			pure[G_W] * rot_conj[G_Y] + pure[G_Y] * rot_conj[G_W] + pure[G_Z] * rot_conj[G_X] - pure[G_X] * rot_conj[G_Z],
			pure[G_W] * rot_conj[G_Z] + pure[G_Z] * rot_conj[G_W] + pure[G_X] * rot_conj[G_Y] - pure[G_Y] * rot_conj[G_X],
			pure[G_W] * rot_conj[G_W] - pure[G_X] * rot_conj[G_X] - pure[G_Y] * rot_conj[G_Y] - pure[G_Z] * rot_conj[G_Z]);

		result.Set(
			rot[G_W]*temp[G_X] + rot[G_X]*temp[G_W] + rot[G_Y]*temp[G_Z] - rot[G_Z]*temp[G_Y],
			rot[G_W]*temp[G_Y] + rot[G_Y]*temp[G_W] + rot[G_Z]*temp[G_X] - rot[G_X]*temp[G_Z],
			rot[G_W]*temp[G_Z] + rot[G_Z]*temp[G_W] + rot[G_X]*temp[G_Y] - rot[G_Y]*temp[G_X]);
		return result;
	}

	/*
		Transform a vector by a rotation quaternion.

		\pre give a vector, and a rotation quaternion (by definition, a rotation quaternion is normalized).
		\param rot the quaternion.
		\param vector the original vector to transform.
		\return the resulting vector transformed by the quaternion.
		\post v' = q P(v) q*  (where result is v', rot is q, and vector is v.  q* is conj(q), and P(v) is pure
		quaternion made from v).
	*/
	template <typename DATA_TYPE>
	inline GVect<DATA_TYPE, 3> operator *(const GQuat<DATA_TYPE>& rot, const GVect<DATA_TYPE, 3>& vector) {
		GVect<DATA_TYPE, 3> temporary;
		return XForm(temporary, rot, vector);
	}


	/*
		Transform a vector by a rotation quaternion.

		\pre give a vector, and a rotation quaternion (by definition, a rotation quaternion is normalized).
		\param rot the quaternion.
		\param vector the original vector to transform
		\post v' = q P(v) q*  (where result is v', rot is q, and vector is v. q* is conj(q), and P(v) is pure
		quaternion made from v).
	*/   
	template <typename DATA_TYPE>
	inline GVect<DATA_TYPE, 3> operator *=(GVect<DATA_TYPE, 3>& vector, const GQuat<DATA_TYPE>& rot) {
		GVect<DATA_TYPE, 3> temporary = vector;
		return XForm(vector, rot, temporary);
	}


	/*
		Transform  a vector by a matrix.

		Transforms a vector with a matrix, uses multiplication of [m x k] matrix by a [k x 1] matrix (the later also
		known as a vector).

		\param result the vector to write the result in.
		\param matrix the transform matrix.
		\param vector the original vector.
		\post This results in a rotational xform of the vector (assumes you know what you are doing -
		i.e. that you know that the last component of a vector by definition is 0.0, and changing
		this might make the xform different than what you may expect).
		\post returns a point same size as the matrix rows.
	*/
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GVect<DATA_TYPE, COLS>& XForm(GVect<DATA_TYPE, COLS>& result,
										const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
										const GVect<DATA_TYPE, COLS>& vector) {
		// do a standard [m x k] by [k x n] matrix multiplication (where n == 0).

		// reset vec to zero...
		result = GVect<DATA_TYPE, COLS>();
		for (GUInt32 iRow = 0; iRow < ROWS; ++iRow)
			for (GUInt32 iCol = 0; iCol < COLS; ++iCol)
				result[iRow] += matrix(iRow, iCol) * vector[iCol];
		return result;
	}


	/*
		Matrix * vector transformation.
		Mmultiplication of [m x k] matrix by a [k x 1] matrix (also known as a vector).
	 
		\param matrix the transform matrix
		\param vector the original vector
		\return the vector transformed by the matrix
		\post This results in a full matrix xform of the vector (assumes you know what you are doing -
		i.e. that you know that the last component of a vector by definition is 0.0, and changing
		this might make the xform different that what you may expect).
		\post returns a vector same size as the matrix rows.
	*/
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GVect<DATA_TYPE, COLS> operator *(const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
											const GVect<DATA_TYPE, COLS>& vector) {
		// do a standard [m x k] by [k x n] matrix multiplication (where n == 0).
		GVect<DATA_TYPE, COLS> temporary;
		return XForm(temporary, matrix, vector);
	}



	/* partially transform a partially specified vector by a matrix, assumes last elt of vector is 0 (the 0 makes it only partially transformed).
	 *  Transforms a vector with a matrix, uses multiplication of [m x k] matrix by a [k-1 x 1] matrix (also known as a Vector [with w == 0 for vectors by definition]).
	 *  \param result        the vector to write the result in
	 *  \param matrix        the transform matrix
	 *  \param vector        the original vector
	 *  \post the [k-1 x 1] vector you pass in is treated as a [vector, 0.0]
	 *  \post This ends up being a partial xform using only the rotation from the matrix (vector xformed result is untranslated).
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, GUInt32 VEC_SIZE>
	inline GVect<DATA_TYPE, VEC_SIZE>& XForm(GVect<DATA_TYPE, VEC_SIZE >& result,
											const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
											const GVect<DATA_TYPE, VEC_SIZE>& vector) {
		G_ASSERT(VEC_SIZE == COLS - 1);
		// do a standard [m x k] by [k x n] matrix multiplication (where n == 0).

		// copy the point to the correct size.
		GVect<DATA_TYPE, COLS> temp_vector, temp_result;
		for (GUInt32 x = 0; x < VEC_SIZE; ++x)
			temp_vector[x] = vector[x];
		// by definition of a vector, set the last unspecified elt to 0.0
		temp_vector[COLS - 1] = (DATA_TYPE)0;

		// transform it
		XForm<DATA_TYPE, ROWS, COLS>(temp_result, matrix, temp_vector);

		for (GUInt32 x = 0; x < VEC_SIZE; ++x)
			result[x] = temp_result[x];
		return result;
	}

	/* matrix * partial vector, assumes last elt of vector is 0 (partial transform).
	 *  \param matrix        the transform matrix
	 *  \param vector        the original vector
	 *  \return  the vector transformed by the matrix
	 *  multiplication of [m x k] matrix by a [k-1 x 1] matrix (also known as a Vector [with w == 0 for vectors by definition]).
	 *  \post the [k-1 x 1] vector you pass in is treated as a [vector, 0.0]
	 *  \post This ends up being a partial xform using only the rotation from the matrix (vector xformed result is untranslated).
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, GUInt32 COLS_MINUS_ONE>
	inline GVect<DATA_TYPE, COLS_MINUS_ONE> operator *(const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
													   const GVect<DATA_TYPE, COLS_MINUS_ONE>& vector) {
		GVect<DATA_TYPE, COLS_MINUS_ONE> temporary;
		return XForm(temporary, matrix, vector);
	}


	/* transform point by a matrix.
	 *  multiplication of [m x k] matrix by a [k x 1] matrix (also known as a Point).
	 *  \param result        the point to write the result in
	 *  \param matrix        the transform matrix
	 *  \param point         the original point
	 *  \post This results in a full matrix xform of the point.
	 *  \post returns a point same size as the matrix rows...  (p[r][1] = m[r][k] * p[k][1])
	 */

	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GPoint<DATA_TYPE, COLS>& XForm(GPoint<DATA_TYPE, COLS>& result,
										const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
										const GPoint<DATA_TYPE, COLS>& point) {
		// do a standard [m x k] by [k x n] matrix multiplication (n == 1).

		// reset point to zero
		result = GPoint<DATA_TYPE, COLS>();

		for (GUInt32 iRow = 0; iRow < ROWS; ++iRow)
			for (GUInt32 iCol = 0; iCol < COLS; ++iCol)
				result[iRow] += matrix(iRow, iCol) * point[iCol];
		return result;
	}

	/* matrix * point.
	 *  multiplication of [m x k] matrix by a [k x 1] matrix (also known as a Point...).
	 *  \param matrix        the transform matrix
	 *  \param point         the original point
	 *  \return  the point transformed by the matrix
	 *  \post This results in a full matrix xform of the point.
	 *  \post returns a point same size as the matrix rows...  (p[r][1] = m[r][k] * p[k][1])
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GPoint<DATA_TYPE, COLS> operator *(const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
											const GPoint<DATA_TYPE, COLS>& point) {
		GPoint<DATA_TYPE, COLS> temporary;
		return XForm(temporary, matrix, point);
	}


	/* transform a partially specified point by a matrix, assumes last element of point is 1.
	 *  Transforms a point with a matrix, uses multiplication of [m x k] matrix by a [k-1 x 1] matrix (also known as a Point [with w == 1 for points by definition]).
	 *  \param result        the point to write the result in
	 *  \param matrix        the transform matrix
	 *  \param point         the original point
	 *  \post the [k-1 x 1] point you pass in is treated as [point, 1.0]
	 *  \post This results in a full matrix xform of the point.
	 *  \todo we need a PointOps.h operator*=(scalar) function
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, GUInt32 PNT_SIZE>
	inline GPoint<DATA_TYPE, PNT_SIZE>& XForm(GPoint<DATA_TYPE, PNT_SIZE>& result,
											const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
											const GPoint<DATA_TYPE, PNT_SIZE>& point) {
		G_ASSERT(PNT_SIZE == COLS - 1);

		// copy the point to the correct size
		GPoint<DATA_TYPE, PNT_SIZE + 1> temp_point, temp_result;
		for (GUInt32 x = 0; x < PNT_SIZE; ++x)
			temp_point[x] = point[x];
		// by definition of a point, set the last unspecified element to 1.0
		temp_point[PNT_SIZE] = (DATA_TYPE)1;

		// transform it
		XForm<DATA_TYPE, ROWS, COLS>(temp_result, matrix, temp_point);

		for (GUInt32 x = 0; x < PNT_SIZE; ++x)
			result[x] = temp_result[x];
		return result;
	}

	/* matrix * partially specified point.
	 *  multiplication of [m x k] matrix by a [k-1 x 1] matrix (also known as a Point [with w == 1 for points by definition]).
	 *  \param matrix        the transform matrix
	 *  \param point         the original point
	 *  \return  the point transformed by the matrix
	 *  \post the [k-1 x 1] vector you pass in is treated as a [point, 1.0]
	 *  \post This results in a full matrix xform of the point.
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, GUInt32 COLS_MINUS_ONE>
	inline GPoint<DATA_TYPE, COLS_MINUS_ONE> operator *(const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
														const GPoint<DATA_TYPE, COLS_MINUS_ONE>& point)	{
		GPoint<DATA_TYPE, COLS_MINUS_ONE> temporary;
		return XForm(temporary, matrix, point);
	}

	/* point * a matrix
	 *  multiplication of [m x k] matrix by a [k x 1] matrix (also known as a Point [with w == 1 for points by definition]).
	 *  \param matrix        the transform matrix
	 *  \param point         the original point
	 *  \return  the point transformed by the matrix
	 *  \post This results in a full matrix xform of the point.
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GPoint<DATA_TYPE, COLS> operator *(const GPoint<DATA_TYPE, COLS>& point,
											const GMatrix<DATA_TYPE, ROWS, COLS>& matrix) {
		GPoint<DATA_TYPE, COLS> temporary;
		return XForm(temporary, matrix, point);
	}


	/* point *= a matrix
	 *  multiplication of [m x k] matrix by a [k x 1] matrix (also known as a Point [with w == 1 for points by definition]).
	 *  \param matrix        the transform matrix
	 *  \param point         the original point
	 *  \return  the point transformed by the matrix
	 *  \post This results in a full matrix xform of the point.
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GPoint<DATA_TYPE, COLS> operator *=(GPoint<DATA_TYPE, COLS>& point,
											const GMatrix<DATA_TYPE, ROWS, COLS>& matrix) {
		GPoint<DATA_TYPE, COLS> temporary = point;
		return XForm(point, matrix, temporary);
	}

	/* partial point *= a matrix
	 *  multiplication of [m x k] matrix by a [k-1 x 1] matrix (also known as a Point [with w == 1 for points by definition]).
	 *  \param matrix        the transform matrix
	 *  \param point         the original point
 	 *  \return  the point transformed by the matrix
	 *  \post the [k-1 x 1] vector you pass in is treated as a [point, 1.0]
	 *  \post This results in a full matrix xform of the point.
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, GUInt32 COLS_MINUS_ONE>
	inline GPoint<DATA_TYPE, COLS_MINUS_ONE>& operator *=(GPoint<DATA_TYPE, COLS_MINUS_ONE>& point,
														const GMatrix<DATA_TYPE, ROWS, COLS>& matrix) {
		GPoint<DATA_TYPE, COLS_MINUS_ONE> temporary = point;
		return XForm(point, matrix, temporary);
	}



	/* transform ray by a matrix.
	 *  multiplication of [m x k] matrix by two [k x 1] matrices (also known as a ray...).
	 *  \param result        the ray to write the result in
	 *  \param matrix        the transform matrix
	 *  \param ray           the original ray
	 *  \post This results in a full matrix xform of the ray.
	 *  \post returns a ray same size as the matrix rows...  (p[r][1] = m[r][k] * p[k][1])
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GGenericRay<DATA_TYPE, COLS>& XForm(GGenericRay<DATA_TYPE, COLS>& result,
											const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
											const GGenericRay<DATA_TYPE, COLS>& ray) {
		Amanith::GPoint<DATA_TYPE, COLS> pos;
		Amanith::GVect<DATA_TYPE, COLS> dir;
		result.SetOrigin(XForm(pos, matrix, ray.GetOrigin()));
		result.SetDir(XForm(dir, matrix, ray.GetDir()));
		return result;
	}

	/* ray * a matrix
	 *  multiplication of [m x k] matrix by a ray.
	 *  \param matrix        the transform matrix
	 *  \param ray           the original ray
	 *  \return  the ray transformed by the matrix
	 *  \post This results in a full matrix xform of the ray.
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GGenericRay<DATA_TYPE, COLS> operator *(const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
												  const GGenericRay<DATA_TYPE, COLS>& ray) {
		GGenericRay<DATA_TYPE, COLS> temporary;
		return XForm(temporary, matrix, ray);
	}

	/* ray *= a matrix
	 *  multiplication of [m x k] matrix by a ray.
	 *  \param matrix        the transform matrix
	 *  \param ray           the original ray
	 *  \return  the ray transformed by the matrix
	 *  \post This results in a full matrix xform of the ray.
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GGenericRay<DATA_TYPE, COLS>& operator *=(GGenericRay<DATA_TYPE, COLS>& ray,
													const GMatrix<DATA_TYPE, ROWS, COLS>& matrix) {
		GGenericRay<DATA_TYPE, COLS> temporary = ray;
		return XForm(ray, matrix, temporary);
	}

	/* transform seg by a matrix.
	 *  multiplication of [m x k] matrix by two [k x 1] matrices (also known as a seg...).
	 *  \param result        the seg to write the result in
	 *  \param matrix        the transform matrix
	 *  \param seg           the original seg
	 *  \post This results in a full matrix xform of the seg.
	 *  \post returns a seg same size as the matrix rows...  (p[r][1] = m[r][k] * p[k][1])
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GLineSeg<DATA_TYPE, COLS>& XForm(GLineSeg<DATA_TYPE, COLS>& result,
											const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
											const GLineSeg<DATA_TYPE, COLS>& seg) {
		Amanith::GPoint<DATA_TYPE, COLS> pos;
		Amanith::GVect<DATA_TYPE, COLS> dir;
		result.SetOrigin(XForm(pos, matrix, seg.GetOrigin()));
		result.SetDir(XForm(dir, matrix, seg.GetDir()));
		return result;
	}

	/* seg * a matrix
	 *  multiplication of [m x k] matrix by a seg.
	 *  \param matrix        the transform matrix
	 *  \param seg         the original ray
	 *  \return  the seg transformed by the matrix
	 *  \post This results in a full matrix xform of the seg.
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GLineSeg<DATA_TYPE, COLS> operator *(const GMatrix<DATA_TYPE, ROWS, COLS>& matrix,
												const GLineSeg<DATA_TYPE, COLS>& seg) {
		GLineSeg<DATA_TYPE, COLS> temporary;
		return XForm(temporary, matrix, seg);
	}


	/* seg *= a matrix
	 *  multiplication of [m x k] matrix by a seg.
	 *  \param matrix        the transform matrix
	 *  \param seg         the original point
	 *  \return  the point transformed by the matrix
	 *  \post This results in a full matrix xform of the point.
	 */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GLineSeg<DATA_TYPE, COLS>& operator *=(GLineSeg<DATA_TYPE, COLS>& seg,
												const GMatrix<DATA_TYPE, ROWS, COLS>& matrix) {
		GLineSeg<DATA_TYPE, COLS> temporary = seg;
		return XForm(seg, matrix, temporary);
	}

	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE QuadraticForm(const GMatrix<DATA_TYPE, SIZE, SIZE>& matrix,
									const GVect<DATA_TYPE, SIZE>& v) {
		return Dot(matrix * v, v);
	}

	/*!
		Quadratic form.

		\param matrix the characteristic matrix that describes quadratic form.
		\param p the point used to evaluate quadratic form value.
		\return the quadratic form evaluated at specified point.
	*/
	template <typename DATA_TYPE, GUInt32 SIZE, GUInt32 POINT_SIZE>
	inline DATA_TYPE QuadraticForm(const GMatrix<DATA_TYPE, SIZE, SIZE>& matrix,
									const GPoint<DATA_TYPE, POINT_SIZE>& p) {

		G_ASSERT(POINT_SIZE == SIZE - 1);
		GVect<DATA_TYPE, SIZE> v;

		// copy the point to the correct size
		for (GUInt32 i = 0; i < POINT_SIZE; ++i)
			v[i] = p[i];
		v[POINT_SIZE] = (DATA_TYPE)1;
		return QuadraticForm(matrix, v);
	}


};	// end namespace Amanith

#endif
