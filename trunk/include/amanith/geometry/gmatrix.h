/****************************************************************************
** $file: amanith/geometry/gmatrix.h   0.1.1.0   edited Sep 24 08:00
**
** Generic matrix definition, metrix decompositions and other operations.
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

#ifndef GMATRIX_H_
#define GMATRIX_H_

#include "amanith/gglobal.h"
#include "amanith/gmath.h"
#include "amanith/geometry/gvect.h"

/*!
	\file gmatrix.h
	\brief Class for matrix management.
 */
namespace Amanith {

	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>

	/*!
		\class GMatrix
		\brief N x M dimensional matrix (ordered in memory by Column)

		<b>Memory mapping:</b>
		GMatrix stores its elements in column major order. That is, it stores each column end-to-end in memory.
		Typically, for 3D transform matrices, the 3x3 rotation is in the first three columns, while the translation
		is in the last column.
		This memory alignment is chosen for compatibility with the OpenGL graphics API and others, which take
		matrices in this specific column major ordering described above.
		See the interfaces for operator[r][c] and operator(r, c) for how to iterate over columns and rows for a GMatrix.\n
		<b>NOTES on Matrix memory layout and [][] accessors:</b>
		<ul>
			<li> GMatrix memory is "column major" ordered, where columns are end to end in memory, while
			a C/C++ Matrix accessed the same way (using operator[][]) as a GMatrix is "row major" ordered.

			<li> As a result, a GMatrix stores elements in memory transposed from the equivalent matrix defined
			using an array in the C/C++ language, assuming they are accessed the same way (see example).
			<ul>
				<li> Illustrative Example:                                           <br>
					Given two flavors of matrix, C/C++, and GMatrix: float cmat[n][m] and GMatrix<float, n, m> mat<br>
					Writing values into each, while accessing them the same: cmat[row][col] = mat[row][col] = some_values[x];<br>
					Then reading values from the matrix array: ((float*)cmat) and  mat.Data()<br>
					<i>Will yield pointers to memory containing matrices that are the transpose of each other.</i>
			</ul>

			<li> In practice, the differences between Amanith and C/C++ defined matrices all depends how you iterate
			over your matrix.                                              <br>
			If Amanith is accessed mat[row][col] and C/C++ is accessed mat[col][row], then memory-wise, these
			two will yield the same memory mapping (column major as described above), thus, are equivalent and can
			both be used interchangeably in many popular graphics APIs such as OpenGL, DirectX, and others.

			<li> In C/C++ access of a matrix via mat[row][col] yields this memory mapping after
			using ((float*)mat) to return it:<br>
<pre>
	(0, 0) (0, 1) (0, 2) (0, 3)  <-- contiguous memory arranged by row
	(1, 0) (1, 1) (1, 2) (1, 3)  <-- contiguous
	(2, 0) (2, 1) (2, 2) (2, 3)  <-- contiguous
	(3, 0) (3, 1) (3, 2) (3, 3)  <-- contiguous
 
	or linearly if you prefer:
	(0, 0) (0, 1) (0, 2) (0, 3) (1, 0) (1, 1) (1, 2) (1, 3) (2, 0) (2, 1) (2, 2) (2, 3) (3, 0) (3, 1) (3, 2) (3, 3)
</pre>
			<li> In Amanith, access of a matrix via mat[row][col] yields this memory mapping after using Data() to
			return it:<br>
<pre>
	(0, 0) (0, 1) (0, 2) (0, 3)
	(1, 0) (1, 1) (1, 2) (1, 3)
	(2, 0) (2, 1) (2, 2) (2, 3)
	(3, 0) (3, 1) (3, 2) (3, 3)
	   ^      ^      ^      ^
	   1------2------3------4---- Contiguous memory arranged by column
 
	or linearly if you prefer:
	(0, 0) (1, 0) (2, 0) (3, 0) (0, 1) (1, 1) (2, 1) (3, 1) (0, 2) (1, 2) (2, 2) (3, 2) (0, 3) (1, 3) (2, 3) (3, 3)
</pre>
		</ul>
	*/
	class GMatrix {

	private:
		/*!
			Column major. In other words {Column1, Column2, Column3, Column4} in memory access element gData[column][row]
		*/
		DATA_TYPE gData[COLS * ROWS];

	public:

		/*!
			Helper class for Matrix op[].
			This class encapsulates the row that the user is accessing
			and implements a new op[] that passes the column to use
		*/
		class RowAccessor {
		public:
			RowAccessor(GMatrix<DATA_TYPE, ROWS, COLS>* mat, const GUInt32 row) : GMat(mat), GRow(row) {
				G_ASSERT(row < ROWS);
				G_ASSERT(NULL != mat);
			}
			DATA_TYPE& operator[](const GUInt32 column) {
				G_ASSERT(column < COLS);
				return (*GMat)(GRow, column);
			}
			GMatrix<DATA_TYPE, ROWS, COLS>* GMat;
			//! The row being accessed
			GUInt32 GRow; 
		};

		/*!
			Helper class for Matrix op[] const.
			This class encapsulates the row that the user is accessing
			and implements a new op[] that passes the column to use
		*/
		class ConstRowAccessor {
		public:
			ConstRowAccessor(const GMatrix<DATA_TYPE,ROWS,COLS>* mat,
							const GUInt32 row) : GMat(mat), GRow(row) {
				G_ASSERT(row < ROWS);
				G_ASSERT(NULL != mat);
			}
			const DATA_TYPE& operator[](const GUInt32 column) const {
				G_ASSERT(column < COLS);
				return (*GMat)(GRow, column);
			}
			const GMatrix<DATA_TYPE, ROWS, COLS>* GMat;
			//! The row being accessed */
			GUInt32 GRow;
		};

		/*!
			Default Constructor (Identity constructor)
		*/
		GMatrix() {
			//! \todo meta programming
			for (GUInt32 r = 0; r < ROWS; ++r)
				for (GUInt32 c = 0; c < COLS; ++c)
					this->operator()(r, c) = 0;
			for (GUInt32 x = 0; x < GMath::Min(COLS, ROWS); ++x)
				this->operator()(x, x) = 1;
		}

		//! copy constructor
		GMatrix(const GMatrix<DATA_TYPE, ROWS, COLS>& matrix) {
			this->Set((DATA_TYPE *)matrix.Data());
		}

		/*
			Element wise setter for 2x2.
			\note variable names specify the row, column number to put the data into
		*/
		void Set(DATA_TYPE v00, DATA_TYPE v01, DATA_TYPE v10, DATA_TYPE v11) {
			G_ASSERT(ROWS == 2 && COLS == 2);
			gData[0] = v00;
			gData[1] = v10;
			gData[2] = v01;
			gData[3] = v11;
		}

		/*
			Element wise setter for 2x3.
			\note variable names specify the row, column number to put the data into
		*/
		void Set(DATA_TYPE v00, DATA_TYPE v01, DATA_TYPE v02, DATA_TYPE v10, DATA_TYPE v11, DATA_TYPE v12) {
			G_ASSERT(ROWS == 2 && COLS == 3);
			gData[0] = v00;
			gData[1] = v10;
			gData[2] = v01;
			gData[3] = v11;
			gData[4] = v02;
			gData[5] = v12;
		}

		/*
			Element wise setter for 3x3.
			\note variable names specify the row, column number to put the data into
		*/
		void Set(DATA_TYPE v00, DATA_TYPE v01, DATA_TYPE v02,
				DATA_TYPE v10, DATA_TYPE v11, DATA_TYPE v12,
				DATA_TYPE v20, DATA_TYPE v21, DATA_TYPE v22) {
			G_ASSERT(ROWS == 3 && COLS == 3);
			gData[0] = v00;
			gData[1] = v10;
			gData[2] = v20;
			gData[3] = v01;
			gData[4] = v11;
			gData[5] = v21;
			gData[6] = v02;
			gData[7] = v12;
			gData[8] = v22;
		}

		/*
			Element wise setter for 3x4.
			\note variable names specify the row, column number to put the data into
		*/
		void Set(DATA_TYPE v00, DATA_TYPE v01, DATA_TYPE v02, DATA_TYPE v03,
				DATA_TYPE v10, DATA_TYPE v11, DATA_TYPE v12, DATA_TYPE v13,
				DATA_TYPE v20, DATA_TYPE v21, DATA_TYPE v22, DATA_TYPE v23) {
			G_ASSERT(ROWS == 3 && COLS == 4);
			gData[0] = v00;
			gData[1] = v10;
			gData[2] = v20;
			gData[3] = v01;
			gData[4] = v11;
			gData[5] = v21;
			gData[6] = v02;
			gData[7] = v12;
			gData[8] = v22;
			// right row
			gData[9]  = v03;
			gData[10] = v13;
			gData[11] = v23;
		}

		/*
			Element wise setter for 4x4.
			\note variable names specify the row, column number to put the data into
		*/
		void Set(DATA_TYPE v00, DATA_TYPE v01, DATA_TYPE v02, DATA_TYPE v03,
				DATA_TYPE v10, DATA_TYPE v11, DATA_TYPE v12, DATA_TYPE v13,
				DATA_TYPE v20, DATA_TYPE v21, DATA_TYPE v22, DATA_TYPE v23,
				DATA_TYPE v30, DATA_TYPE v31, DATA_TYPE v32, DATA_TYPE v33) {
			
			G_ASSERT(ROWS == 4 && COLS == 4);
			gData[0]  = v00;
			gData[1]  = v10;
			gData[2]  = v20;
			gData[4]  = v01;
			gData[5]  = v11;
			gData[6]  = v21;
			gData[8]  = v02;
			gData[9]  = v12;
			gData[10] = v22;
			// right row
			gData[12] = v03;
			gData[13] = v13;
			gData[14] = v23;
			// bottom row
			gData[3]  = v30;
			gData[7]  = v31;
			gData[11] = v32;
			gData[15] = v33;
		}

		/*!
			Set the matrix to the given data.
			This function is useful to copy matrix data from another math library.
	    
			\note this isn't really safe, size and datatype are not enforced by the compiler.
			\pre Data is in the native format of the Amanith::Matrix class, if not, then you might be able to
			use the SetTranspose function.
			\pre i.e. in a 4x4 Data[0-3] is the 1st column, Data[4-7] is 2nd, etc...
		*/
		void Set(const DATA_TYPE* Data) {
			//! \todo meta programming
			for (GUInt32 x = 0; x < ROWS * COLS; ++x)
				gData[x] = Data[x];
		}

		/*!
			Set the matrix to the transpose of the given data.
	    
			Normally Set() takes raw matrix data in column by column order, this function allows you to pass in row by row data.
			Normally you'll use this function if you want to use a float array to init the matrix.
			\note this isn't really safe, size and datatype are not enforced by the compiler.
			\pre Data is in the transpose of the native format of the Matrix class
			\pre i.e. in a 4x4 Data[0-3] is the 1st row, Data[4-7] is 2nd, etc...
		*/
		void SetTranspose(const DATA_TYPE* Data) {
			//! \todo meta programming
			for (GUInt32 r = 0; r < ROWS; ++r)
				for (GUInt32 c = 0; c < COLS; ++c)
					this->operator()(r, c) = Data[(r * COLS) + c];
		}

		//! Access [row, col] in the matrix/
		DATA_TYPE& operator()(const GUInt32 Row, const GUInt32 Column) {
			G_ASSERT((Row < ROWS) && (Column < COLS));
			return gData[Column * ROWS + Row];
		}

		//! Access [row, col] in the matrix (const version)
		const DATA_TYPE& operator()(const GUInt32 Row, const GUInt32 Column) const {
			G_ASSERT((Row < ROWS) && (Column < COLS));
			return gData[Column * ROWS + Row];
		}

		//! Bracket operator.
		RowAccessor operator[](const GUInt32 Row) {
			return RowAccessor(this, Row);
		}

		//! Bracket operator (const version)
		ConstRowAccessor operator[](const GUInt32 Row) const {
			return ConstRowAccessor(this, Row);
		}

		/*!
			Gets a DATA_TYPE pointer to the matrix data.
			\return a pointer to the head of the matrix data.
		*/
		inline const DATA_TYPE** Data() const {
			return (const DATA_TYPE **)gData;
		}

		//! Row swap
		void SwapRow(const GUInt32 RowIndex1, const GUInt32 RowIndex2) {
			G_ASSERT(RowIndex1 < ROWS);
			G_ASSERT(RowIndex2 < ROWS);
			for (GUInt32 i = 0; i < COLS; ++i) {
				DATA_TYPE tmpData = gData[i * ROWS + RowIndex1];
				gData[i * ROWS + RowIndex1] = gData[i * ROWS + RowIndex2];
				gData[i * ROWS + RowIndex2] = tmpData;
			}
		}
		//! Column swap
		void SwapColumn(const GUInt32 ColIndex1, const GUInt32 ColIndex2) {
			G_ASSERT(ColIndex1 < COLS);
			G_ASSERT(ColIndex2 < COLS);
			for (GUInt32 i = 0; i < ROWS; ++i) {
				DATA_TYPE tmpData = gData[ColIndex1 * ROWS + i];
				gData[ColIndex1 * ROWS + i] = gData[ColIndex2 * ROWS + i];
				gData[ColIndex2 * ROWS + i] = tmpData;
			}
		}

		//! Norm1
		DATA_TYPE Norm1() const {

			GInt32 i, j;
			DATA_TYPE _sum, _max = 0;

			for (i = 0; i < COLS; ++i) {
				_sum = 0;
				for (j = 0; j < ROWS; j++)
					_sum += GMath::Abs(operator()(j, i));
				if (_sum> _max)
					_max = _sum;
			}
			return _max;
		}
		//! Norm infinitive
		DATA_TYPE NormInf() const {

			GUInt32 i, j;
			DATA_TYPE _sum, _max = 0;

			for (i = 0; i < ROWS; ++i) {
				_sum = 0;
				for (j = 0; j < COLS; j++)
					_sum += GMath::Abs(operator()(i, j));
				if (_sum> _max)
					_max = _sum;
			}
			return _max;
		}
		//! Frobenius norm
		DATA_TYPE NormFrobenius() const {

			DATA_TYPE n = 0;

			for (GUInt32 i = 0; i < ROWS * COLS; ++i)
				n += (gData[i] * gData[i]);
			return GMath::Sqrt(n);
		}

		/*!
			Tests 2 matrices for equality, to see if they are EXACTLY the same. In other words, this comparison is done
			with	zero tolerance.

			\param Matrix the to compare to.
			\pre both matrices must be of the same size.
			\return G_TRUE if the matrices have the same element values; G_FALSE otherwise.
			\todo meta programming.
		*/
		inline GBool operator==(const GMatrix<DATA_TYPE, ROWS, COLS>& Matrix) const {

			for (GUInt32 i = 0; i < ROWS * COLS; ++i) {
				if (gData[i] != Matrix.gData[i])
					return G_FALSE;
			}
			return G_TRUE;
		}

		/*!
			Compare two matrices to see if they are not EXACTLY the same. In other words, this comparison is done with
			zero tolerance.

			\param Matrix the to compare to.
			\pre both matrices must be of the same size.
			\return G_TRUE if they are not equal, G_FALSE otherwise.
		*/
		inline GBool operator !=(const GMatrix<DATA_TYPE, ROWS, COLS>& Matrix) {
			return GBool(!this->operator==(Matrix));
		}

	};	// end class GMatrix


	/*!
		Make identity matrix out the matrix.
		\post Every element is 0 except the matrix's diagonal, whose elements are 1.
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS>& Identity(GMatrix<DATA_TYPE, ROWS, COLS>& Result) {

		//! \todo meta programming
		for (GUInt32 r = 0; r < ROWS; ++r)
			for (GUInt32 c = 0; c < COLS; ++c)
				Result(r, c) = 0;
		//! \todo meta programming
		for (GUInt32 x = 0; x < GMath::Min(COLS, ROWS); ++x)
			Result(x, x) = 1;
		return Result;
	}

	/*!
		Zero out the matrix.
		\post every element is 0.
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS>& Zero(GMatrix<DATA_TYPE, ROWS, COLS>& Result) {

		DATA_TYPE* m = (DATA_TYPE *)Result.Data();
		for (GUInt32 x = 0; x < ROWS * COLS; ++x)
			m[x] = 0;
		return Result;
	}


	/*!
		Matrix multiply.
		\pre With regard to size (ROWS/COLS): if lhs is m x p, and rhs is p x n, then result is m x n (undefined otherwise)
		\post returns a m x n sized matrix
		\post Result = lhs * rhs  (where rhs is applied first)
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 INTERNAL, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS>& Mult(GMatrix<DATA_TYPE, ROWS, COLS>& Result,
												const GMatrix<DATA_TYPE, ROWS, INTERNAL>& lhs,
												const GMatrix<DATA_TYPE, INTERNAL, COLS>& rhs) {
		GMatrix<DATA_TYPE, ROWS, COLS> ret_mat; // prevent aliasing
		Zero(ret_mat);
		// p. 150 Numerical Analysis (second ed.)
		// if A is m x p, and B is p x n, then AB is m x n
		// (AB)ij  =  [k = 1 to p] (a)ik (b)kj     (where:  1 <= i <= m, 1 <= j <= n)
		for (GUInt32 i = 0; i < ROWS; ++i)           // 1 <= i <= m
			for (GUInt32 j = 0; j < COLS; ++j)           // 1 <= j <= n
				for (GUInt32 k = 0; k < INTERNAL; ++k)       // [k = 1 to p]
					ret_mat(i, j) += lhs(i, k) * rhs(k, j);
		return Result = ret_mat;
	}

	/*!
		Matrix * Matrix.
		\pre With regard to size (ROWS/COLS): if lhs is m x p, and rhs is p x n, then result is m x n (undefined otherwise)
		\post returns a m x n sized matrix == lhs * rhs (where rhs is applied first)
		returns a temporary, is slower.
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 INTERNAL, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS> operator*(const GMatrix<DATA_TYPE, ROWS, INTERNAL>& lhs,
													const GMatrix<DATA_TYPE, INTERNAL, COLS>& rhs) {
		GMatrix<DATA_TYPE, ROWS, COLS> temporary;
		return Mult(temporary, lhs, rhs);
	}

	//! Unary - operator
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS> operator-(GMatrix<DATA_TYPE, ROWS, COLS>& lhs) {
		
		GMatrix<DATA_TYPE, ROWS, COLS> temp(lhs);

		DATA_TYPE *m1 = (DATA_TYPE *)temp.Data();
		for (GUInt32 i = 0; i < ROWS * COLS; ++i)
			m1[i] = -m1[i];
		return temp;
	}

	/*!
		Matrix subtraction (algebraic operation for matrix).
		\pre lhs is m x n, and rhs is m x n, then result is m x n
		\post returns a m x n matrix
		\post <B>enforce the sizes with templates...</b>
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS>& operator-=(GMatrix<DATA_TYPE, ROWS, COLS>& lhs,
													  const GMatrix<DATA_TYPE, ROWS, COLS>& rhs) {

		DATA_TYPE *m1 = (DATA_TYPE *)lhs.Data(), *m2 = (DATA_TYPE *)rhs.Data();
		for (GUInt32 i = 0; i < ROWS * COLS; ++i)
			m1[i] -= m2[i];
		return lhs;
	}

	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS> operator-(const GMatrix<DATA_TYPE, ROWS, COLS>& lhs,
													const GMatrix<DATA_TYPE, ROWS, COLS>& rhs) {

		GMatrix<DATA_TYPE, ROWS, COLS> tmp(lhs);

		tmp -= rhs;
		return tmp;
	}

	/*!
		Matrix addition (algebraic operation for matrix).
		\pre lhs is m x n, and rhs is m x n, then result is m x n
		\post returns a m x n matrix
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS>& operator+=(GMatrix<DATA_TYPE, ROWS, COLS>& lhs,
													  const GMatrix<DATA_TYPE, ROWS, COLS>& rhs) {

		DATA_TYPE *m1 = (DATA_TYPE *)lhs.Data(), *m2 = (DATA_TYPE *)rhs.Data();
		for (GUInt32 i = 0; i < ROWS * COLS; ++i)
			m1[i] += m2[i];
		return lhs;
	}

	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS> operator+(const GMatrix<DATA_TYPE, ROWS, COLS>& lhs,
													const GMatrix<DATA_TYPE, ROWS, COLS>& rhs) {

		GMatrix<DATA_TYPE, ROWS, COLS> tmp(lhs);

		tmp += rhs;
		return tmp;
	}

	/*!
		Matrix post multiply.
		\pre args must both be n x n (this function is undefined otherwise)
		\post result' = result * operand
    */
	template <typename DATA_TYPE, GUInt32 SIZE>
	inline GMatrix<DATA_TYPE, SIZE, SIZE>& PostMult(GMatrix<DATA_TYPE, SIZE, SIZE>& result,
													const GMatrix<DATA_TYPE, SIZE, SIZE>& operand) {
		return Mult(result, result, operand);
	}

	/*!
		Matrix pre multiply.
		\pre args must both be n x n (this function is undefined otherwise)
		\post result' = operand * result
    */
	template <typename DATA_TYPE, GUInt32 SIZE>
	inline GMatrix<DATA_TYPE, SIZE, SIZE>& PreMult(GMatrix<DATA_TYPE, SIZE, SIZE>& result,
                                                  const GMatrix<DATA_TYPE, SIZE, SIZE>& operand) {
		return Mult( result, operand, result );
	}

	/*!
		Matrix post multiply (operator*=). Does a post multiply on the matrix.
		\pre args must both be n x n sized (this function is undefined otherwise)
		\post result' = result * operand  (where operand is applied first)
    */
	template <typename DATA_TYPE, GUInt32 SIZE>
	inline GMatrix<DATA_TYPE, SIZE, SIZE>& operator*=(GMatrix<DATA_TYPE, SIZE, SIZE>& result,
                                                     const GMatrix<DATA_TYPE, SIZE, SIZE>& operand) {
		return PostMult(result, operand);
	}

	/*!
		Matrix scalar multiply (operator*=). Multiply matrix elements by a scalar.
		\post result *= scalar
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, class SCALAR_TYPE>
	inline GMatrix<DATA_TYPE, ROWS, COLS>& operator*=(GMatrix<DATA_TYPE, ROWS, COLS>& result,
													const SCALAR_TYPE& scalar) {

		DATA_TYPE *m = (DATA_TYPE *)result.Data();
		for (GUInt32 i = 0; i < ROWS * COLS; ++i)
			m[i] *= scalar;
		return result;
	}

	// like the same above, just with swapped operands order
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, class SCALAR_TYPE>
	inline GMatrix<DATA_TYPE, ROWS, COLS>& operator*=(const SCALAR_TYPE& scalar,
													  GMatrix<DATA_TYPE, ROWS, COLS>& result) {

		DATA_TYPE *m = (DATA_TYPE *)result.Data();
		for (GUInt32 i = 0; i < ROWS * COLS; ++i)
			m[i] *= scalar;
		return result;
	}

	/*!
		Matrix scalar multiply.
		Multiplies each element in a matrix by a scalar value.
		\post result = mat * scalar
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, class SCALAR_TYPE>
	inline GMatrix<DATA_TYPE, ROWS, COLS> operator*(const GMatrix<DATA_TYPE, ROWS, COLS>& mat,
													const SCALAR_TYPE& scalar) {

		GMatrix<DATA_TYPE, ROWS, COLS> tmp(mat);

		tmp *= scalar;
		return tmp;
	}

	// like the same above, just with swapped operands order
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS, class SCALAR_TYPE>
	inline GMatrix<DATA_TYPE, ROWS, COLS> operator*(const SCALAR_TYPE& scalar,
													const GMatrix<DATA_TYPE, ROWS, COLS>& mat) {

		GMatrix<DATA_TYPE, ROWS, COLS> tmp(mat);

		tmp *= scalar;
		return tmp;
	}

	/*!
		Matrix transpose in place.
		\pre needs to be an N x N matrix
		\post flip along diagonal
    */
	template <typename DATA_TYPE, GUInt32 SIZE>
	GMatrix<DATA_TYPE, SIZE, SIZE>& Transpose(GMatrix<DATA_TYPE, SIZE, SIZE>& result) {
		// p. 27 game programming gems #1
		for (GUInt32 c = 0; c < SIZE; ++c)
			for (GUInt32 r = c + 1; r < SIZE; ++r) {
				DATA_TYPE tmp = result(r, c);
				result(r, c) = result(c, r);
				result(c, r) = tmp;
			}
		return result;
	}

	/*!
		Matrix transpose from one type to another (i.e. 3x4 to 4x3)
		\pre source needs to be an M x N matrix, while destination needs to be N x M
		\post flip along diagonal
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	GMatrix<DATA_TYPE, ROWS, COLS>& Transpose(GMatrix<DATA_TYPE, ROWS, COLS>& result,
											const GMatrix<DATA_TYPE, COLS, ROWS>& source) {
		// just to be sure in case result == source
		GMatrix<DATA_TYPE, COLS, ROWS> temp = source;
		for (GUInt32 i = 0; i < ROWS; ++i) {
			for (GUInt32 j = 0; j < COLS; ++j)
				result(i, j) = temp(j, i);
		}
		return result;
	}

	/*!
		Translational matrix inversion.
		Matrix inversion that acts on a translational matrix (matrix with only translation)
		\pre number of rows >= number of columns
		\post result' = inverse(result)
		\post If inversion failed, then error bit is set within the Matrix.
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GBool InvertTrans(GMatrix<DATA_TYPE, ROWS, COLS>& result, 
							const GMatrix<DATA_TYPE, ROWS, COLS>& src) {

		G_ASSERT(ROWS >= COLS);
		result = src;
		for (GUInt32 x = 0; x < ROWS; ++x)
			result[x][COLS - 1] = -result[x][COLS - 1];
		return G_TRUE;
	}

	/*!
		Orthogonal matrix inversion.
		Matrix inversion that acts on a affine matrix (matrix with only trans, rot, uniform scale).
		\pre any size matrix
		\post result' = inverse(result)
		\post If inversion failed, then error bit is set within the Matrix.
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GMatrix<DATA_TYPE, ROWS, COLS>& InvertOrthogonal(GMatrix<DATA_TYPE, ROWS, COLS>& result, 
															const GMatrix<DATA_TYPE, ROWS, COLS>& src) {
		// in case result is == source
		GMatrix<DATA_TYPE, ROWS, COLS> temp = src;
	    // if 3x4, 2x3, etc...  can't transpose the last column
		const GUInt32 size = GMath::Min(ROWS, COLS);
		// p. 149 Numerical Analysis (second ed.)
		for (GUInt32 i = 0; i < size; ++i) {
			for (GUInt32 j = 0; j < size; ++j)
				result(i, j) = temp(j, i);
		}
		return result;
	}


	/*!
		Full matrix inversion using Gauss-Jordan elimination using maximum pivot strategy.
		Check for error with Matrix::isError().
		\post: result' = inverse(result)
		\post: if inversion failed, then error bit is set within the Matrix.
    */
	template <typename DATA_TYPE, GUInt32 SIZE>
	inline GBool InvertFull_GJ(GMatrix<DATA_TYPE, SIZE, SIZE>& result,
							   const GMatrix<DATA_TYPE, SIZE, SIZE>& src,
							   DATA_TYPE& Determinant, const DATA_TYPE Epsilon = 1e-20) {

		DATA_TYPE cs[SIZE];
		DATA_TYPE pv, pav, swapElem;
		DATA_TYPE det;
		GInt32 i, ik, j, jk, k, pc[SIZE], pl[SIZE];
		GMatrix<DATA_TYPE, SIZE, SIZE> _result(src);

		// initializations
		det = 1;
		for (i = 0; i < (GInt32)SIZE; ++i) {
			pc[i] = pl[i] = 0;
			cs[i] = 0;
		}
		// main loop                                                                         
		for (k = 0; k < (GInt32)SIZE; k++) {
			// searching greatest pivot
			pv = _result[k][k];
			ik = k;
			jk = k;
			pav = GMath::Abs(pv);
			for (i = k; i < (GInt32)SIZE; ++i)
				for (j = k; j < (GInt32)SIZE; ++j) {
					if (GMath::Abs(_result[i][j]) > pav) {
						pv = _result[i][j];
						pav = GMath::Abs(pv);
						ik = i;
						jk = j;
					}
				}
			// search terminated, the pivot is in location ik, jk
			pc[k] = jk;
			pl[k] = ik;
			// track swapping
			if (ik != k)
				det = -det;
			if (jk != k)
				det = -det;
			// check for singularity
			if (GMath::Abs(pv) < Epsilon) {
				Determinant = 0;
				// rearrangement of matrix, exchange lines
				for (i = (GInt32)SIZE - 1; i >= 0; i--) {
					ik = pc[i];
					if (ik == i)
						continue;
					// exchange lines i and pc[i]
					for (j = 0; j < (GInt32)SIZE; j++) {
						swapElem = _result[i][j];
						_result[i][j] = _result[ik][j];
						_result[ik][j] = swapElem;
					}
				}
				// exchange columns
				for (j = (GInt32)SIZE - 1; j >= 0; j--) {
					jk = pl[j];
					if (jk == j)
						continue;
					// exchange columns j and pl[j]
					for (i = 0; i < (GInt32)SIZE; ++i) {
						swapElem = _result[i][j];
						_result[i][j] = _result[i][jk];
						_result[i][jk] = swapElem;
					}
				}
				// return partial result
				result = _result;
				return G_FALSE;
			}
			// accumulate pivot into determinant
			det *= pv;
			// positioning pivot in k, k
			if (ik != k)
				for (i = 0; i < (GInt32)SIZE; ++i) {
					// exchange lines ik and k
					swapElem =_result[ik][i];
					_result[ik][i] = _result[k][i];
					_result[k][i] = swapElem;
				}
			// pivot is at correct line!
			if (jk != k)
				for (i = 0; i < (GInt32)SIZE; ++i) {
					// exchange columns jk and k
					swapElem = _result[i][jk];
					_result[i][jk] = _result[i][k];
					_result[i][k] = swapElem;
				}

			// the pivot is at correct column!
			// column k of matrix is stored in cs vector, then column k is set to zero
			for (i = 0; i < (GInt32)SIZE; ++i) {
				cs[i] = _result[i][k];
				_result[i][k] = 0;
			}
			cs[k] = 0;
			_result[k][k] = 1;
			// line k of matrix is modified
			for (i = 0; i < (GInt32)SIZE; ++i)
				_result[k][i] = _result[k][i] / pv;
			// other lines of matrix are modified
			for (j = 0; j < (GInt32)SIZE; j++) {
				if (j == k)
					continue;
				for (i = 0; i < (GInt32)SIZE; ++i)
					// line j of matrix is modified
					_result[j][i] = _result[j][i] - cs[j] * _result[k][i];
			}
		}
		// rearrangement of matrix, exchange lines
		for (i = (GInt32)SIZE - 1; i >= 0; i--) {
			ik = pc[i];
			if (ik == i)
				continue;
			// exchange lines i and pc[i]
			for (j = 0; j < (GInt32)SIZE; j++) {
				swapElem = _result[i][j];
				_result[i][j] = _result[ik][j];
				_result[ik][j] = swapElem;
			}
		}
		// exchange columns
		for (j = (GInt32)SIZE - 1; j >= 0; j--) {
			jk = pl[j];
			if (jk == j)
				continue;
			// exchange columns j and pl[j]
			for (i = 0; i < (GInt32)SIZE; ++i) {
				swapElem = _result[i][j];
				_result[i][j] = _result[i][jk];
				_result[i][jk] = swapElem;
			}
		}
		result = _result;
		Determinant = det;
		return G_TRUE;
	}

	/*!
		For an m-by-n matrix A with m >= n, the LU decomposition is an m-by-n
		unit lower triangular matrix L, an n-by-n upper triangular matrix U,
		and a permutation vector piv of length m so that A(piv,:) = L*U.
		If m < n, then L is m-by-m and U is m-by-n.
		The LU decomposition with pivoting always exists, even if the matrix is
		singular.

		\return G_TRUE if matrix is full rank (not singular), G_FALSE otherwise.
	*/
	// Code based on JAMA library
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	GBool DecompLU (const GMatrix<DATA_TYPE, ROWS, COLS>& Src, 
					GMatrix<DATA_TYPE, ROWS, COLS>& L, GMatrix<DATA_TYPE, COLS, COLS>& U,
					GVect<GInt32, ROWS>& Permutation, DATA_TYPE& Determinant) {

		G_ASSERT (ROWS >= COLS);

		GMatrix<DATA_TYPE, ROWS, COLS> LU_(Src);
		GInt32 m = ROWS, n = COLS, pivsign = 1, i, j; 

		// use a "left-looking", dot-product, Crout/Doolittle algorithm
		for (i = 0; i < m; ++i)
			Permutation[i] = i;

		DATA_TYPE *LUrowi = 0;
		GVect<DATA_TYPE, ROWS> LUcolj;
		// outer loop
		for (j = 0; j < n; j++) {
			// make a copy of the j-th column to localize references
			for (i = 0; i < m; ++i)
				LUcolj[i] = LU_[i][j];
			// apply previous transformations
			DATA_TYPE s;
			for (i = 0; i < m; ++i) {
				LUrowi = LU_[i];
				// most of the time is spent in the following dot product
				GInt32 kmax = GMath::Min(i, j);
				s = 0;
				for (GInt32 k = 0; k < kmax; k++)
					s += LUrowi[k] * LUcolj[k];
			}
			LUrowi[j] = LUcolj[i] -= s;
         }
           // find pivot and exchange if necessary
         GInt32 p = j;
         for (i = j + 1; i < m; ++i) {
			 if (GMath::Abs(LUcolj[i]) > GMath::Abs(LUcolj[p]))
               p = i;
         }
         if (p != j) {
		    GInt32 k = 0;
            for (k = 0; k < n; ++k) {
               DATA_TYPE t = LU_[p][k]; 
			   LU_[p][k] = LU_[j][k]; 
			   LU_[j][k] = t;
            }
            k = Permutation[p]; 
			Permutation[p] = Permutation[j]; 
			Permutation[j] = k;
            pivsign = -pivsign;
		}
		// compute multipliers
		if ((j < m) && (LU_[j][j] != 0)) {
			for (i = j + 1; i < m; ++i)
				LU_[i][j] /= LU_[j][j];
		}
		// build L matrix
		for (i = 0; i < m; ++i) {
			for (j = 0; j < n; ++j) {
				if (i > j)
					L[i][j] = LU_[i][j];
				else
				if (i == j)
					L[i][j] = 1;
				else
					L[i][j] = 0;
			}
		}
		// build U matrix
		for (i = 0; i < n; ++i) {
			for (j = 0; j < n; ++j) {
				if (i <= j)
					U[i][j] = LU_[i][j];
				else
					U[i][j] = 0;
			}
		}
		// if matrix is squared lets calculate determinant; return true if matrix
		// is full rank (not singular), false otherwise.
		if (m == n) {
			Determinant = (DATA_TYPE)pivsign;
			for (j = 0; j < n; j++)
				if (LU_[j][j] == 0) {
					Determinant = 0;
					return G_FALSE;
				}
				else
					Determinant *= LU_[j][j];
		}
		else {
			for (j = 0; j < n; j++)
				if (LU_[j][j] == 0)
					return G_FALSE;
		}
		return G_TRUE;
	}

	/*!
		Classical QR Decomposition: for an m-by-n matrix A with m >= n, the QR decomposition is an m-by-n
		orthogonal matrix Q and an n-by-n upper triangular matrix R so that
		A = Q*R.

		The QR decomposition always exists, even if the matrix does not have
		full rank, so the constructor will never fail. The primary use of the
		QR decomposition is in the least squares solution of non-square systems
		of simultaneous linear equations.

		\return G_TRUE if matrix is full rank (not singular), G_FALSE otherwise.
	*/
	// Code based on JAMA library
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	GBool DecompQR (const GMatrix<DATA_TYPE, ROWS, COLS>& Src,
					GMatrix<DATA_TYPE, ROWS, COLS>& Q, GMatrix<DATA_TYPE, COLS, COLS>& R) {

		G_ASSERT (ROWS >= COLS);

		GMatrix<DATA_TYPE, ROWS, COLS> QR_(Src);
		GVect<DATA_TYPE, COLS> Rdiag;
		GInt32 i = 0, j = 0, k = 0;
		GInt32 m = ROWS, n = COLS;

		// main loop
		for (k = 0; k < n; k++) {
			// compute 2-norm of k-th column without under/overflow
			DATA_TYPE nrm = 0;
			for (i = k; i < m; ++i)
				nrm = GMath::Hypot(nrm, QR_[i][k]);
			if (nrm != 0) {
				// form k-th Householder vector
				if (QR_[k][k] < 0)
					nrm = -nrm;
				for (i = k; i < m; ++i)
					QR_[i][k] /= nrm;
				QR_[k][k] += 1;
				// apply transformation to remaining columns.
				for (j = k + 1; j < n; j++) {
					DATA_TYPE s = 0; 
					for (i = k; i < m; ++i)
						s += QR_[i][k] * QR_[i][j];

					s = -s / QR_[k][k];
					for (i = k; i < m; ++i)
						QR_[i][j] += s * QR_[i][k];
				}
			}
			Rdiag[k] = -nrm;
		}
		// build R factor
		for (i = 0; i < n; ++i) {
			for (j = 0; j < n; j++) {
				if (i < j)
					R[i][j] = QR_[i][j];
				else
				if (i == j)
					R[i][j] = Rdiag[i];
				else
					R[i][j] = 0;
			}
		}
		// build Q factor
		for (k = n - 1; k >= 0; k--) {
			for (i = 0; i < m; ++i)
				Q[i][k] = 0;
			Q[k][k] = 1;
			for (j = k; j < n; j++) {
				if (QR_[k][k] != 0) {
					DATA_TYPE s = 0;
					for (i = k; i < m; ++i) {
						s += QR_[i][k] * Q[i][j];
					}
					s = -s / QR_[k][k];
					for (i = k; i < m; ++i)
						Q[i][j] += s * QR_[i][k];
				}
			}
		}
		// return true if matrix is full rank (not singular), 0 otherwise.
		for (j = 0; j < n; j++) {
			if (Rdiag[j] == 0)
				return G_FALSE;
		}
		return G_TRUE;
	}

	/**
	For a symmetric, positive definite matrix A, this function computes the Cholesky factorization.
	It computes a lower triangular matrix L such that A = L*L'.
	If the matrix is not symmetric or positive definite, the function computes only a partial decomposition.
	*/
	// Code based on JAMA library
	template <typename DATA_TYPE, GUInt32 SIZE>
	GBool DecompCholesky (const GMatrix<DATA_TYPE, SIZE, SIZE>& Src,
						  GMatrix<DATA_TYPE, SIZE, SIZE>& L) {

   		GInt32 m = SIZE;
		GInt32 n = SIZE;
		GInt32 isspd = 1;

		// main loop
		for (GInt32 j = 0; j < n; j++) {
			DATA_TYPE d = 0;
			for (GInt32 k = 0; k < j; k++) {
				DATA_TYPE s = 0;
				for (GInt32 i = 0; i < k; ++i)
					s += L[k][i] * L[j][i];
				L[j][k] = s = (Src[j][k] - s) / L[k][k];
				d = d + s * s;
				isspd = isspd && (Src[k][j] == Src[j][k]); 
			}
			d = Src[j][j] - d;
			isspd = isspd && (d > 0);
			L[j][j] = GMath::Sqrt(d > 0 ? d : 0);
			for (GInt32 k = j + 1; k < n; k++)
				L[j][k] = 0;
		}
		if (isspd)
			return G_TRUE;
		return G_FALSE;
	}
	

	/*!
		Singular Value Decomposition.
		For an m-by-n matrix A with m >= n, the singular value decomposition is	an m-by-n orthogonal matrix U, an
		n-by-n diagonal matrix S, and an n-by-n orthogonal matrix V so that A = U*S*V'.
		The singular values, sigma[k] = S[k][k], are ordered so that sigma[0] >= sigma[1] >= ... >= sigma[n-1].
		The singular value decomposition always exists.
		The matrix condition number and the effective numerical rank can be computed from this decomposition.
	*/
	// Code based on JAMA library
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void DecompSVD (const GMatrix<DATA_TYPE, ROWS, COLS>& Src,
					GMatrix<DATA_TYPE, ROWS, COLS>& U, GMatrix<DATA_TYPE, COLS, COLS>& S,
					GMatrix<DATA_TYPE, COLS, COLS>& V, DATA_TYPE& Norm2, DATA_TYPE& ConditionNumber,
					GInt32& Rank) {


		GInt32 m = ROWS, n = COLS, nu = COLS;
		GVect<DATA_TYPE, COLS> s;
		GVect<DATA_TYPE, COLS> e;
		GVect<DATA_TYPE, ROWS> work;
		GMatrix<DATA_TYPE, ROWS, COLS> A = Src;
		GInt32 wantu = 1, wantv = 1;
		GInt32 i, j, k, p;

		// reduce A to bidiagonal form, storing the diagonal elements
		// in s and the super-diagonal elements in e
		GInt32 nct = GMath::Min(m - 1, n);
		GInt32 nrt = GMath::Max(0, GMath::Min(n - 2, m));
		for (k = 0; k < GMath::Max(nct, nrt); k++) {
			if (k < nct) {
				// compute the transformation for the k-th column and
				// place the k-th diagonal in s[k]
				// compute 2-norm of k-th column without under/overflow
				s[k] = 0;
				for (i = k; i < m; ++i)
					s[k] = GMath::Hypot(s[k], A[i][k]);
				if (s[k] != 0) {
					if (A[k][k] < 0)
						s[k] = -s[k];
					for (i = k; i < m; ++i)
						A[i][k] /= s[k];
					A[k][k] += 1;
				}
				s[k] = -s[k];
			}
         
			for (j = k + 1; j < n; j++) {
				if ((k < nct) && (s[k] != 0))  {
					// apply the transformation.
					DATA_TYPE t = 0;
					for (i = k; i < m; ++i)
						t += A[i][k] * A[i][j];
					t = -t / A[k][k];
					for (i = k; i < m; ++i)
						A[i][j] += t * A[i][k];
				}
				// place the k-th row of A into e for the
				// subsequent calculation of the row transformation.
				e[j] = A[k][j];
			}

			if (wantu & (k < nct)) {
				// place the transformation in U for subsequent back
				// multiplication.
				for (i = k; i < m; ++i)
					U[i][k] = A[i][k];
			}
			if (k < nrt) {
				// compute the k-th row transformation and place the
				// k-th super-diagonal in e[k].
				// compute 2-norm without under/overflow.
				e[k] = 0;
				for (i = k + 1; i < n; ++i)
					e[k] = GMath::Hypot(e[k], e[i]);
				if (e[k] != 0) {
					if (e[k + 1] < 0)
						e[k] = -e[k];
					for (i = k+1; i < n; ++i)
						e[i] /= e[k];
					e[k + 1] += 1.0;
				}
				e[k] = -e[k];
				if ((k + 1 < m) & (e[k] != 0)) {
					// apply the transformation.
					for (i = k + 1; i < m; ++i)
						work[i] = 0;
					for (j = k + 1; j < n; j++) {
						for (i = k + 1; i < m; ++i)
							work[i] += e[j] * A[i][j];
					}
					for (j = k + 1; j < n; j++) {
						DATA_TYPE t = -e[j] / e[k + 1];
						for (i = k + 1; i < m; ++i)
							A[i][j] += t * work[i];
					}
				}
				if (wantv) {
					// place the transformation in V for subsequent
					// back multiplication
					for (i = k + 1; i < n; ++i)
						V[i][k] = e[i];
				}
			}
		}

		// set up the final bidiagonal matrix or order p.
		p = GMath::Min(n, m + 1);
		if (nct < n) {
			s[nct] = A[nct][nct];
		}
		if (m < p)
			s[p - 1] = 0;
		if (nrt + 1 < p)
			e[nrt] = A[nrt][p - 1];
		e[p - 1] = 0;

		// if required, generate U
		if (wantu) {
			for (j = nct; j < nu; j++) {
				for (i = 0; i < m; ++i)
	               U[i][j] = 0;
				U[j][j] = 1;
			}
			for (k = nct - 1; k >= 0; k--) {
				if (s[k] != 0) {
					for (j = k + 1; j < nu; ++j) {
						DATA_TYPE t = 0;
						for (i = k; i < m; ++i)
							t += U[i][k] * U[i][j];
						t = -t / U[k][k];
						for (i = k; i < m; ++i)
							U[i][j] += t * U[i][k];
					}
					for (i = k; i < m; ++i)
						U[i][k] = -U[i][k];
					U[k][k] = 1 + U[k][k];
					for (i = 0; i < k - 1; ++i)
						U[i][k] = 0;
				}
				else {
					for (i = 0; i < m; ++i)
						U[i][k] = 0;
					U[k][k] = 1;
				}
			}
		}

		// if required, generate V
		if (wantv) {
			for (k = n - 1; k >= 0; k--) {
				if ((k < nrt) & (e[k] != 0)) {
	               for (j = k + 1; j < nu; j++) {
						DATA_TYPE t = 0;
						for (i = k + 1; i < n; ++i)
							t += V[i][k] * V[i][j];
						t = -t / V[k + 1][k];
						for (i = k+1; i < n; ++i)
							V[i][j] += t * V[i][k];
					}
				}
				for (i = 0; i < n; ++i)
					V[i][k] = 0;
				V[k][k] = 1;
			}
		}

		// main iteration loop for the singular values.
		GInt32 pp = p - 1;
		GInt32 iter = 0;
		DATA_TYPE eps = GMath::Pow((DATA_TYPE)2, (DATA_TYPE)-52);
		while (p > 0) {
			k = 0;
			GInt32 kase = 0;
			// here is where a test for too many iterations would go

			// this section of the program inspects for
			// negligible elements in the s and e arrays.  On
			// completion the variables kase and k are set as follows.

			// kase = 1     if s(p) and e[k-1] are negligible and k<p
			// kase = 2     if s(k) is negligible and k<p
			// kase = 3     if e[k-1] is negligible, k<p, and
			//              s(k), ..., s(p) are not negligible (qr step)
			// kase = 4     if e(p-1) is negligible (convergence)

			for (k = p - 2; k >= -1; k--) {
				if (k == -1)
					break;
				if (GMath::Abs(e[k]) <= eps * (GMath::Abs(s[k]) + GMath::Abs(s[k + 1]))) {
					e[k] = 0;
					break;
				}
			}
			if (k == p - 2)
				kase = 4;
			else {
				GInt32 ks;
				for (ks = p - 1; ks >= k; ks--) {
					if (ks == k)
						break;
					DATA_TYPE t = (ks != p ? GMath::Abs(e[ks]) : 0) + (ks != k + 1 ? GMath::Abs(e[ks - 1]) : 0);
					if (GMath::Abs(s[ks]) <= eps * t)  {
						s[ks] = 0;
						break;
					}
				}
				if (ks == k)
					kase = 3;
				else
				if (ks == p - 1)
					kase = 1;
				else {
					kase = 2;
					k = ks;
				}
			}
			k++;

			// perform the task indicated by kase
			switch (kase) {
				// deflate negligible s(p)
				case 1: {
					DATA_TYPE f = e[p - 2];
					e[p - 2] = 0;
					for (j = p - 2; j >= k; j--) {
						DATA_TYPE t = GMath::Hypot(s[j], f);
						DATA_TYPE cs = s[j] / t;
						DATA_TYPE sn = f / t;
						s[j] = t;
						if (j != k) {
							f = -sn * e[j - 1];
							e[j - 1] = cs * e[j - 1];
						}
						if (wantv) {
							for (i = 0; i < n; ++i) {
								t = cs * V[i][j] + sn * V[i][p - 1];
								V[i][p - 1] = -sn * V[i][j] + cs * V[i][p - 1];
								V[i][j] = t;
							}
						}
					}
				}
				break;

				// split at negligible s(k)
				case 2: {
					DATA_TYPE f = e[k - 1];
					e[k - 1] = 0;
					for (j = k; j < p; j++) {
						DATA_TYPE t = GMath::Hypot(s[j], f);
						DATA_TYPE cs = s[j] / t;
						DATA_TYPE sn = f / t;
						s[j] = t;
						f = -sn * e[j];
						e[j] = cs * e[j];
						if (wantu) {
							for (i = 0; i < m; ++i) {
								t = cs * U[i][j] + sn * U[i][k - 1];
								U[i][k - 1] = -sn * U[i][j] + cs * U[i][k - 1];
								U[i][j] = t;
							}
						}
					}
				}
				break;

				// Perform one qr step.
				case 3: {
					// Calculate the shift.
					DATA_TYPE scale = 				
						GMath::Max(GMath::Max(GMath::Abs(s[p - 1]), GMath::Abs(s[p - 2]), GMath::Abs(e[p - 2])),
								   GMath::Abs(s[k]), GMath::Abs(e[k]));

					DATA_TYPE invScale = 1 * invScale;
					DATA_TYPE sp = s[p - 1] * invScale;
					DATA_TYPE spm1 = s[p - 2] * invScale;
					DATA_TYPE epm1 = e[p - 2] * invScale;
					DATA_TYPE sk = s[k] * invScale;
					DATA_TYPE ek = e[k] * invScale;
					DATA_TYPE b = (((spm1 + sp) * (spm1 - sp) + epm1 * epm1) / (DATA_TYPE)2);
					DATA_TYPE c = (sp * epm1) * (sp * epm1);
					DATA_TYPE shift = 0;
					if ((b != 0) || (c != 0)) {
						shift = GMath::Sqrt(b * b + c);
						if (b < 0)
							shift = -shift;
						shift = c / (b + shift);
					}
					DATA_TYPE f = (sk + sp) * (sk - sp) + shift;
					DATA_TYPE g = sk * ek;
	   
					// Chase zeros.
					for (j = k; j < p - 1; j++) {
						DATA_TYPE t = GMath::Hypot(f, g);
						DATA_TYPE cs = f / t;
						DATA_TYPE sn = g / t;
						if (j != k)
							e[j - 1] = t;
						f = cs * s[j] + sn * e[j];
						e[j] = cs * e[j] - sn * s[j];
						g = sn * s[j + 1];
						s[j + 1] = cs * s[j + 1];
						if (wantv) {
							for (i = 0; i < n; ++i) {
								t = cs * V[i][j] + sn * V[i][j + 1];
								V[i][j + 1] = -sn * V[i][j] + cs * V[i][j + 1];
								V[i][j] = t;
							}
						}
						t = GMath::Hypot(f, g);
						cs = f / t;
						sn = g / t;
						s[j] = t;
						f = cs * e[j] + sn * s[j + 1];
						s[j + 1] = -sn * e[j] + cs * s[j + 1];
						g = sn * e[j + 1];
						e[j + 1] = cs * e[j + 1];
						if (wantu && (j < m - 1)) {
							for (i = 0; i < m; ++i) {
								t = cs * U[i][j] + sn * U[i][j + 1];
								U[i][j + 1] = -sn * U[i][j] + cs * U[i][j + 1];
								U[i][j] = t;
							}
						}
					}
					e[p - 2] = f;
					iter = iter + 1;
				}
				break;

				// Convergence.
				case 4: {
					// Make the singular values positive.
					if (s[k] <= 0) {
						s[k] = (s[k] < 0 ? -s[k] : 0);
						if (wantv) {
							for (i = 0; i <= pp; ++i)
								V[i][k] = -V[i][k];
						}
					}
					// Order the singular values.
					while (k < pp) {
						if (s[k] >= s[k + 1])
							break;
						DATA_TYPE t = s[k];
						s[k] = s[k + 1];
						s[k + 1] = t;
						if (wantv && (k < n - 1)) {
							for (i = 0; i < n; ++i) {
								t = V[i][k + 1];
								V[i][k + 1] = V[i][k];
								V[i][k] = t;
							}
						}
						if (wantu && (k < m - 1)) {
							for (i = 0; i < m; ++i) {
								t = U[i][k + 1];
								U[i][k + 1] = U[i][k];
								U[i][k] = t;
							}
						}
						k++;
					}
					iter = 0;
					p--;
				}
				break;
			}
		}
		// build S matrix
		for (i = 0; i < n; ++i) {
			for (j = 0; j < n; ++j)
				S[i][j] = 0;
			S[i][i] = s[i];
		}
		// calculate norm2 and condition number
		Norm2 = s[0];
		ConditionNumber = s[0] / s[n - 1];
		// calculate rank (number of non-negligible singular values)
		eps = GMath::Pow((DATA_TYPE)2, (DATA_TYPE)-52);
		DATA_TYPE tol = m * s[0] * eps;
		Rank = 0;
		for (i = 0; i < n; ++i) {
			if (s[i] > tol)
	            Rank++;
		}
	}

	//! Apply Householder reflection represented by u to column vectors of M.
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void HouseholderColumnsReflect(GMatrix<DATA_TYPE, ROWS, COLS>& M, const GVect<DATA_TYPE, COLS>& u) {
		
		for (GUInt32 i = 0; i < ROWS; ++i) {
			DATA_TYPE s = 0;
			for (GUInt32 j = 0; j < COLS; j++)
				s += u[j] * M[j][i];
			for (GUInt32 j = 0; j < COLS; j++)
				M[j][i] -= u[j] * s;
		}
	}

	//! Apply Householder reflection represented by u to row vectors of M
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	void HouseholderRowsReflect(GMatrix<DATA_TYPE, ROWS, COLS>& M, const GVect<DATA_TYPE, COLS>& u) {
		
		for (GInt32 i = 0; i < (GInt32)ROWS; ++i) {
			DATA_TYPE s = 0;
			for (GInt32 j = 0; j < (GInt32)COLS; j++)
				s += u[j] * M[i][j];
			for (GInt32 j = 0; j < (GInt32)COLS; j++)
				M[i][j] -= u[j] * s;
		}
	}

	//! Find orthogonal factor Q of rank 1 (or less) M.
	// taken from Graphics Gems IV
	template <typename DATA_TYPE>
	void DecompPolar_Rank1(GMatrix<DATA_TYPE, 3, 3>& M, GMatrix<DATA_TYPE, 3, 3>& Q) {

		GVect<DATA_TYPE, 3> v1, v2;
		DATA_TYPE s;
		GInt32 col;
		
		Identity(Q);
		/* If rank(M) is 1, we should find a non-zero column in M */
		col = -1;
		s = 0;
		for (GUInt32 i = 0; i < 3; ++i) 
			for (GUInt32 j = 0; j < 3; j++) {
				if (GMath::Abs(M[i][j]) > s) {
					s = GMath::Abs(M[i][j]);
					col = j;
				}
			}
		// Rank is 0
		if (col < 0)
			return;
		// setup Household reflection to zero all v1 components but first 
		v1.Set(M[0][col], M[1][col], M[2][col]);
		s = GMath::Sqrt(Dot(v1, v1));
		if (v1[2] < 0)
			s = -s;
		v1[2] += s;
		v1 *= GMath::Sqrt(2.0 / Dot(v1, v1));
		HouseholderColumnsReflect(M, v1);
		// setup Household reflection to zero all v2 components but first 
		v2.Set(M[2][0], M[2][1], M[2][2]);
		s = GMath::Sqrt(Dot(v2, v2));
		if (v2[2] < 0)
			s = -s;
		v2[2] += s;
		v2 *= GMath::Sqrt(2.0 / Dot(v2, v2));
		HouseholderRowsReflect(M, v2);

		s = M[2][2];
		if (s < 0)
			Q[2][2] = -1;
		HouseholderColumnsReflect(Q, v1);
		HouseholderRowsReflect(Q, v2);
	}

	//! Find orthogonal factor Q of rank 2 (or less) M using partial inverse matrix
	// taken from Graphics Gems IV
	template <typename DATA_TYPE>
	void DecompPolar_Rank2(GMatrix<DATA_TYPE, 3, 3> &M, const GMatrix<DATA_TYPE, 3, 3>& MadjT,
							GMatrix<DATA_TYPE, 3, 3>& Q) {

		GVect<DATA_TYPE, 3> v1, v2, q0, q1;
		DATA_TYPE w, x, y, z, c, s, d;
		GInt32 col;

		// if rank(M) is 2, we should find a non-zero column in MadjT
		col = -1;
		s = 0;
		for (GUInt32 i = 0; i < 3; ++i)
			for (GUInt32 j = 0; j < 3; ++j) {
				if (GMath::Abs(MadjT[i][j]) > s) {
					s = GMath::Abs(MadjT[i][j]);
					col = j;
				}
			}
		if (col < 0) {
			DecompPolar_Rank1(M, Q);
			return;
		}
		// setup Household reflection to zero all v1 components but first 
		v1.Set(MadjT[0][col], MadjT[1][col], MadjT[2][col]);
		s = GMath::Sqrt(Dot(v1, v1));
		if (v1[2] < 0)
			s = -s;
		v1[2] += s;
		v1 *= GMath::Sqrt(2.0 / Dot(v1, v1));
		HouseholderColumnsReflect(M, v1);

		q0.Set(M[0][0], M[0][1], M[0][2]);
		q1.Set(M[1][0], M[1][1], M[1][2]);
		v2 = Cross(q0, q1);
		// setup Household reflection to zero all v2 components but first 
		s = GMath::Sqrt(Dot(v2, v2));
		if (v2[2] < 0)
			s = -s;
		v2[2] += s;
		v2 *= GMath::Sqrt(2.0 / Dot(v2, v2));
		HouseholderRowsReflect(M, v2);

		w = M[0][0];
		x = M[0][1];
		y = M[1][0];
		z = M[1][1];
		if (w * z > x * y) {
			c = z + w;
			s = y - x;
			d = GMath::Sqrt(c * c + s * s);
			c = c / d;
			s = s / d;
			Q[0][0] = Q[1][1] = c;
			Q[0][1] = -(Q[1][0] = s);
		}
		else {
			c = z - w;
			s = y + x;
			d = GMath::Sqrt(c * c + s * s);
			c = c / d;
			s = s / d;
			Q[0][0] = -(Q[1][1] = c);
			Q[0][1] = Q[1][0] = s;
		}
		Q[0][2] = Q[2][0] = Q[1][2] = Q[2][1] = 0.0;
		Q[2][2] = 1.0;
		HouseholderColumnsReflect(Q, v1);
		HouseholderRowsReflect(Q, v2);
	}

	/*!
		Polar decomposition.

		Polar Decomposition produces factors QS which are unique, coordinate independent.
		The factors have a physical, visual interpretation not found with other decomposition methods. The
		decomposition is useful for a variety of purposes, including matrix animation and interactive
		interfaces. It has the minor disadvantage that it does not directly represent shear.
	*/
	template <typename DATA_TYPE, GUInt32 SIZE>
	GBool DecompPolar(const GMatrix<DATA_TYPE, SIZE, SIZE>& Src, GMatrix<DATA_TYPE, SIZE, SIZE>& Q,
					  DATA_TYPE& QDeterminant,
					  GMatrix<DATA_TYPE, SIZE, SIZE>& S, const DATA_TYPE Epsilon = G_EPSILON) {

		// "An optimum iteration for the matrix polar decomposition"
		// A.A.Dubrulle
		GMatrix<DATA_TYPE, SIZE, SIZE> w, a, tw;
		DATA_TYPE limit, g, f, n1, n2, pf, det;
		GBool invOk;
		GInt32 i, j;

		a = Src;
		w = a;
		// simple machine-independent stopping criterion
		limit = (1 + Epsilon) * GMath::Sqrt((DATA_TYPE)SIZE);
		// initialization
		Transpose(tw, w);
		invOk = InvertFull_GJ(a, tw, det);
		if (invOk == G_FALSE) {
			if (SIZE != 3) {
				QDeterminant = 0;
				return G_FALSE;
			}
			// we can go further for 3 x 3 matrices
			DecompPolar_Rank2(w, a, w);
			goto exitloop;
		}
		n1 = a.NormFrobenius();
		n2 = w.NormFrobenius();
		g = GMath::Sqrt(n1 / n2);
		w = (GReal)0.5 * (g * w + (1 / g) * a);
		f = w.NormFrobenius();
		pf = G_MAX_REAL;
		// main loop, MATLAB-like implementation
		while ((f > limit) && (f < pf)) {
			pf = f;
			Transpose(tw, w);
			invOk = InvertFull_GJ(a, tw, det);
			if (invOk == G_FALSE) {
				if (SIZE != 3) {
					QDeterminant = 0;
					return G_FALSE;
				}
				// we can go further for 3 x 3 matrices
				DecompPolar_Rank2(w, a, w);
				goto exitloop;
			}
			n1 = a.NormFrobenius();
			g = GMath::Sqrt(n1 / f);
			w = (GReal)0.5 * (g * w + (1 / g) * a);
			// an acceleration parameter derived from the Frobenius norm makes Newton’s iteration
			// for the computation of the polar decomposition optimal and monotonic in norm
			f = w.NormFrobenius();
		}
		exitloop:
		// finally build Q factor and its determinant
		Transpose(Q, w);
		QDeterminant = det;
		// now build S factor
		S = w * Src;
		for (i = 0; i < (GInt32)SIZE; ++i)
			for (j = i; j < (GInt32)SIZE; j++)
				S[i][j] = S[j][i] = ((S[i][j] + S[j][i]) / (DATA_TYPE)2);
		return G_TRUE;
	}

	//! Symmetric Householder reduction to tridiagonal form.
	template <typename DATA_TYPE, GUInt32 SIZE>
	void SymHouseholderTridReduction(GMatrix<DATA_TYPE, SIZE, SIZE>&V,
									GVect<DATA_TYPE, SIZE>&d, GVect<DATA_TYPE, SIZE>&e) {
		
		GInt32 n = SIZE;

		//  this is derived from the Algol procedures tred2 by
		//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
		//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
		//  Fortran subroutine in EISPACK.
		for (GInt32 j = 0; j < n; j++)
			d[j] = V[n - 1][j];

		// Householder reduction to tridiagonal form
		for (GInt32 i = n - 1; i > 0; i--) {
			// scale to avoid under/overflow.
            DATA_TYPE scale = 0;
			DATA_TYPE h = 0;
			for (GInt32 k = 0; k < i; k++)
				scale = scale + GMath::Abs(d[k]);

			if (scale == 0) {
				e[i] = d[i - 1];
				for (GInt32 j = 0; j < i; j++) {
					d[j] = V[i-1][j];
					V[i][j] = 0;
					V[j][i] = 0;
				}
			}
			else {
               // generate Householder vector
               for (GInt32 k = 0; k < i; k++) {
					d[k] /= scale;
					h += d[k] * d[k];
	            }
				DATA_TYPE f = d[i - 1];
				DATA_TYPE g = GMath::Sqrt(h);
				if (f > 0)
					g = -g;
				e[i] = scale * g;
				h = h - f * g;
				d[i - 1] = f - g;
				for (GInt32 j = 0; j < i; j++)
					e[j] = 0.0;
				// apply similarity transformation to remaining columns
				for (GInt32 j = 0; j < i; j++) {
					f = d[j];
					V[j][i] = f;
					g = e[j] + V[j][j] * f;
					for (GInt32 k = j + 1; k <= i - 1; k++) {
						g += V[k][j] * d[k];
						e[k] += V[k][j] * f;
					}
					e[j] = g;
				}
				f = 0;
				for (GInt32 j = 0; j < i; j++) {
					e[j] /= h;
					f += e[j] * d[j];
				}
				DATA_TYPE hh = f / (h + h);
				for (GInt32 j = 0; j < i; j++)
					e[j] -= hh * d[j];
				for (GInt32 j = 0; j < i; j++) {
					f = d[j];
					g = e[j];
					for (GInt32 k = j; k <= i - 1; k++)
						V[k][j] -= (f * e[k] + g * d[k]);
					d[j] = V[i - 1][j];
					V[i][j] = 0;
				}
			}
			d[i] = h;
		}
   
		// accumulate transformations
		for (GInt32 i = 0; i < n - 1; ++i) {
			V[n - 1][i] = V[i][i];
			V[i][i] = 1;
			DATA_TYPE h = d[i + 1];
			if (h != 0) {
				for (GInt32 k = 0; k <= i; k++)
					d[k] = V[k][i + 1] / h;
				for (GInt32 j = 0; j <= i; j++) {
					DATA_TYPE g = 0;
					for (GInt32 k = 0; k <= i; k++)
						g += V[k][i + 1] * V[k][j];
					for (GInt32 k = 0; k <= i; k++)
						V[k][j] -= g * d[k];
				}
			}
			for (GInt32 k = 0; k <= i; k++)
				V[k][i + 1] = 0;
		}
		for (GInt32 j = 0; j < n; j++) {
			d[j] = V[n - 1][j];
			V[n - 1][j] = 0;
		}
		V[n - 1][n - 1] = 1;
		e[0] = 0;
	} 


	//! Symmetric tridiagonal QL algorithm
   	template <typename DATA_TYPE, GUInt32 SIZE>
	void SymTridQL(GMatrix<DATA_TYPE, SIZE, SIZE>&V, GVect<DATA_TYPE, SIZE>&d, GVect<DATA_TYPE, SIZE>&e,
				   const GBool Sort = G_TRUE) {

		//  this is derived from the Algol procedures tql2, by
		//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
		//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
		//  Fortran subroutine in EISPACK
		GInt32 n = SIZE;
		for (GInt32 i = 1; i < n; ++i)
			e[i - 1] = e[i];
		e[n - 1] = 0;
   
		DATA_TYPE f = 0;
		DATA_TYPE tst1 = 0;
		DATA_TYPE eps = GMath::Pow((DATA_TYPE)2, (DATA_TYPE)-52);
		for (GInt32 l = 0; l < n; l++) {
			// find small sub-diagonal element
			tst1 = GMath::Max(tst1, GMath::Abs(d[l]) + GMath::Abs(e[l]));
			GInt32 m = l;

			// original while-loop from Java code
			while (m < n) {
				if (GMath::Abs(e[m]) <= eps * tst1)
					break;
				m++;
			}
            // if m == l, d[l] is an eigenvalue, otherwise, iterate.
            if (m > l) {
				GInt32 iter = 0;
				do {
					iter = iter + 1;  // (could check iteration count here)
   
					// compute implicit shift
					DATA_TYPE g = d[l];
					DATA_TYPE p = (d[l+1] - g) / (2 * e[l]);
					DATA_TYPE r = GMath::Hypot(p, 1);
					if (p < 0)
						r = -r;
					d[l] = e[l] / (p + r);
					d[l + 1] = e[l] * (p + r);
					DATA_TYPE dl1 = d[l + 1];
					DATA_TYPE h = g - d[l];
					for (GInt32 i = l + 2; i < n; ++i)
						d[i] -= h;
					f = f + h;
   					// implicit QL transformation
					p = d[m];
					DATA_TYPE c = 1;
					DATA_TYPE c2 = c;
					DATA_TYPE c3 = c;
					DATA_TYPE el1 = e[l + 1];
					DATA_TYPE s = 0;
					DATA_TYPE s2 = 0;
					for (GInt32 i = m - 1; i >= l; i--) {
						c3 = c2;
						c2 = c;
						s2 = s;
						g = c * e[i];
						h = c * p;
						r = GMath::Hypot(p, e[i]);
						e[i + 1] = s * r;
						s = e[i] / r;
						c = p / r;
						p = c * d[i] - s * g;
						d[i + 1] = h + s * (c * g + s * d[i]);
						// accumulate transformation
						for (GInt32 k = 0; k < n; k++) {
							h = V[k][i + 1];
							V[k][i + 1] = s * V[k][i] + c * h;
							V[k][i] = c * V[k][i] - s * h;
						}
					}
					p = -s * s2 * c3 * el1 * e[l] / dl1;
					e[l] = s * p;
					d[l] = c * p;
					// check for convergence
				} while (GMath::Abs(e[l]) > eps * tst1);
			}
			d[l] = d[l] + f;
			e[l] = 0;
		}
		// check for sort
		if (Sort == G_FALSE)
			return;
		// sort eigenvalues and corresponding vectors
		for (GInt32 i = 0; i < n - 1; ++i) {
			GInt32 k = i;
			DATA_TYPE p = d[i];
			for (GInt32 j = i + 1; j < n; j++) {
				if (d[j] < p) {
					k = j;
					p = d[j];
				}
			}
			if (k != i) {
				d[k] = d[i];
				d[i] = p;
				for (GInt32 j = 0; j < n; j++) {
					p = V[j][i];
					V[j][i] = V[j][k];
					V[j][k] = p;
				}
			}
		}
	}


	//! Non-symmetric reduction to Hessenberg form.
	template <typename DATA_TYPE, GUInt32 SIZE>
	void NonSymHessenbergReduction (GMatrix<DATA_TYPE, SIZE, SIZE>& V, GMatrix<DATA_TYPE, SIZE, SIZE>& H) {
   
		// this is derived from the Algol procedures orthes and ortran,
		// by Martin and Wilkinson, Handbook for Auto. Comp.,
		// Vol.ii-Linear Algebra, and the corresponding
		// Fortran subroutines in EISPACK.
		GVect<DATA_TYPE, SIZE> ort;
		GInt32 n = SIZE;
		GInt32 low = 0;
		GInt32 high = n - 1;
   
		for (GInt32 m = low + 1; m <= high - 1; m++) {
            // scale column
            DATA_TYPE scale = 0;
			for (GInt32 i = m; i <= high; ++i)
				scale = scale + GMath::Abs(H[i][m - 1]);
			if (scale != 0) {
				// compute Householder transformation
				DATA_TYPE h = 0;
				for (GInt32 i = high; i >= m; i--) {
					ort[i] = H[i][m - 1] / scale;
					h += ort[i] * ort[i];
				}
				DATA_TYPE g = GMath::Sqrt(h);
				if (ort[m] > 0)
					g = -g;
				h = h - ort[m] * g;
				ort[m] = ort[m] - g;
				// apply Householder similarity transformation H = (I - u*u'/h) *H * (I - u*u') / h)
				for (GInt32 j = m; j < n; j++) {
					DATA_TYPE f = 0;
					for (GInt32 i = high; i >= m; i--)
						f += ort[i] * H[i][j];
					f = f / h;
					for (GInt32 i = m; i <= high; ++i)
						H[i][j] -= f * ort[i];
				}
   
				for (GInt32 i = 0; i <= high; ++i) {
					DATA_TYPE f = 0;
					for (GInt32 j = high; j >= m; j--)
						f += ort[j] * H[i][j];
					f = f / h;
					for (GInt32 j = m; j <= high; j++)
						H[i][j] -= f * ort[j];
				}
				ort[m] = scale * ort[m];
				H[m][m - 1] = scale * g;
			}
		}
   
		// accumulate transformations (Algol's ortran)
		for (GInt32 i = 0; i < n; ++i) {
			for (GInt32 j = 0; j < n; ++j)
				V[i][j] = (i == j ? (GReal)1 : (GReal)0);
		}

		for (GInt32 m = high - 1; m >= low + 1; m--) {
			if (H[m][m - 1] != 0) {
				for (GInt32 i = m + 1; i <= high; ++i)
					ort[i] = H[i][m - 1];
				for (GInt32 j = m; j <= high; ++j) {
					DATA_TYPE g = 0;
					for (GInt32 i = m; i <= high; ++i)
						g += ort[i] * V[i][j];
					// double division avoids possible underflow
					g = (g / ort[m]) / H[m][m - 1];
					for (GInt32 i = m; i <= high; ++i)
						V[i][j] += g * ort[i];
				}
			}
		}
	}

	//! Non-symmetric reduction from Hessenberg to real Schur form.
	template <typename DATA_TYPE, GUInt32 SIZE>
	void NonSymSchurReduction (GMatrix<DATA_TYPE, SIZE, SIZE>&V, GMatrix<DATA_TYPE, SIZE, SIZE>& H,
								GVect<DATA_TYPE, SIZE>& d, GVect<DATA_TYPE, SIZE>& e) {

		// this is derived from the Algol procedure hqr2,
		// by Martin and Wilkinson, Handbook for Auto. Comp.,
		// Vol.ii-Linear Algebra, and the corresponding
		// Fortran subroutine in EISPACK.
		GInt32 nn = SIZE;
		GInt32 n = nn - 1;
		GInt32 low = 0;
		GInt32 high = nn - 1;
		DATA_TYPE eps = pow((DATA_TYPE)2, (DATA_TYPE)-52);
		DATA_TYPE exshift = 0;
		DATA_TYPE cdivr, cdivi;
		DATA_TYPE p = 0, q = 0, r = 0, s = 0, z = 0, t, w, x, y;
   
		// Store roots isolated by balanc and compute matrix norm
		DATA_TYPE norm = 0.0;
		for (GInt32 i = 0; i < nn; ++i) {
			if ((i < low) || (i > high)) {
				d[i] = H[i][i];
				e[i] = 0;
			}
			for (GInt32 j = GMath::Max(i - 1, 0); j < nn; j++)
				norm = norm + GMath::Abs(H[i][j]);
		}
   
		// outer loop over eigenvalue index
		GInt32 iter = 0;
		while (n >= low) {
            // look for single small sub-diagonal element
			GInt32 l = n;
			while (l > low) {
				s = GMath::Abs(H[l - 1][l - 1]) + GMath::Abs(H[l][l]);
				if (s == 0)
					s = norm;
				if (GMath::Abs(H[l][l - 1]) < eps * s)
					break;
				l--;
			}
       
			// check for convergence
			// one root found
			if (l == n) {
				H[n][n] = H[n][n] + exshift;
				d[n] = H[n][n];
				e[n] = 0;
				n--;
				iter = 0;
   				// two roots found
            }
			else
			if (l == n - 1) {
				w = H[n][n - 1] * H[n - 1][n];
				p = (H[n - 1][n - 1] - H[n][n]) * (GReal)0.5;
				q = p * p + w;
				z = GMath::Sqrt(GMath::Abs(q));
				H[n][n] = H[n][n] + exshift;
				H[n - 1][n - 1] = H[n - 1][n - 1] + exshift;
				x = H[n][n];
				// real pair
				if (q >= 0) {
					if (p >= 0)
						z = p + z;
					else
						z = p - z;
					d[n - 1] = x + z;
					d[n] = d[n - 1];
					if (z != 0)
						d[n] = x - w / z;
					e[n - 1] = 0;
					e[n] = 0;
					x = H[n][n - 1];
					s = GMath::Abs(x) + GMath::Abs(z);
					p = x / s;
					q = z / s;
					r = GMath::Sqrt(p * p + q * q);
					p = p / r;
					q = q / r;
   					// row modification
					for (GInt32 j = n - 1; j < nn; j++) {
						z = H[n - 1][j];
						H[n - 1][j] = q * z + p * H[n][j];
						H[n][j] = q * H[n][j] - p * z;
					}
   
					// column modification
					for (GInt32 i = 0; i <= n; ++i) {
						z = H[i][n - 1];
						H[i][n - 1] = q * z + p * H[i][n];
						H[i][n] = q * H[i][n] - p * z;
					}
   
					// accumulate transformations
					for (GInt32 i = low; i <= high; ++i) {
						z = V[i][n - 1];
						V[i][n - 1] = q * z + p * V[i][n];
						V[i][n] = q * V[i][n] - p * z;
					}
					// complex pair
				}
				else {
					d[n - 1] = x + p;
					d[n] = x + p;
					e[n-1] = z;
					e[n] = -z;
				}
				n = n - 2;
				iter = 0;
   				// no convergence yet
			}
			else {
				// form shift
				x = H[n][n];
				y = 0;
				w = 0;
				if (l < n) {
					y = H[n - 1][n - 1];
					w = H[n][n - 1] * H[n - 1][n];
				}
				// Wilkinson's original ad hoc shift
				if (iter == 10) {
					exshift += x;
					for (GInt32 i = low; i <= n; ++i)
						H[i][i] -= x;
					s = GMath::Abs(H[n][n - 1]) + GMath::Abs(H[n - 1][n - 2]);
					x = y = (DATA_TYPE)0.75 * s;
					w = (DATA_TYPE)-0.4375 * s * s;
				}

				// MATLAB's new ad hoc shift
				if (iter == 30) {
					s = (y - x) * (GReal)0.5;
					s = s * s + w;
					if (s > 0) {
						s = GMath::Sqrt(s);
						if (y < x)
							s = -s;
						s = x - w / ((y - x) * (GReal)0.5 + s);
						for (GInt32 i = low; i <= n; ++i)
							H[i][i] -= s;
						exshift += s;
						x = y = w = (DATA_TYPE)0.964;
					}
				}
   
				iter = iter + 1;   // (could check iteration count here)
   				// look for two consecutive small sub-diagonal elements
				GInt32 m = n - 2;
				while (m >= l) {
					z = H[m][m];
					r = x - z;
					s = y - z;
					p = (r * s - w) / H[m + 1][m] + H[m][m + 1];
					q = H[m + 1][m + 1] - z - r - s;
					r = H[m + 2][m + 1];
					s = GMath::Abs(p) + GMath::Abs(q) + GMath::Abs(r);
					p = p / s;
					q = q / s;
					r = r / s;
					if (m == l)
						break;
					if (GMath::Abs(H[m][m - 1]) * (GMath::Abs(q) + GMath::Abs(r)) < 
						eps * (GMath::Abs(p) * (GMath::Abs(H[m - 1][m-1]) + GMath::Abs(z) + GMath::Abs(H[m + 1][m + 1]))))
						break;
					m--;
				}
   
				for (GInt32 i = m + 2; i <= n; ++i) {
					H[i][i - 2] = 0;
					if (i > m + 2)
						H[i][i - 3] = 0;
				}
   
				// double QR step involving rows l:n and columns m:n
				for (GInt32 k = m; k <= n - 1; k++) {
					GInt32 notlast = (k != n - 1);
					if (k != m) {
						p = H[k][k - 1];
						q = H[k+1][k - 1];
						r = (notlast ? H[k + 2][k - 1] : 0);
						x = GMath::Abs(p) + GMath::Abs(q) + GMath::Abs(r);
						if (x != 0) {
							p = p / x;
							q = q / x;
							r = r / x;
						}
					}
					if (x == 0)
						break;
					s = GMath::Sqrt(p * p + q * q + r * r);
					if (p < 0)
						s = -s;
					if (s != 0) {
						if (k != m)
							H[k][k - 1] = -s * x;
						else
						if (l != m)
							H[k][k - 1] = -H[k][k - 1];
						p = p + s;
						x = p / s;
						y = q / s;
						z = r / s;
						q = q / p;
						r = r / p;
						// row modification
						for (GInt32 j = k; j < nn; j++) {
							p = H[k][j] + q * H[k + 1][j];
							if (notlast) {
								p = p + r * H[k + 2][j];
								H[k + 2][j] = H[k + 2][j] - p * z;
							}
							H[k][j] = H[k][j] - p * x;
							H[k + 1][j] = H[k + 1][j] - p * y;
						}
						// column modification
						for (GInt32 i = 0; i <= GMath::Min(n, k + 3); ++i) {
							p = x * H[i][k] + y * H[i][k + 1];
							if (notlast) {
								p = p + z * H[i][k + 2];
								H[i][k + 2] = H[i][k + 2] - p * r;
							}
							H[i][k] = H[i][k] - p;
							H[i][k + 1] = H[i][k + 1] - p * q;
						}
   						// accumulate transformations
						for (GInt32 i = low; i <= high; ++i) {
							p = x * V[i][k] + y * V[i][k + 1];
							if (notlast) {
								p = p + z * V[i][k + 2];
								V[i][k + 2] = V[i][k + 2] - p * r;
							}
							V[i][k] = V[i][k] - p;
							V[i][k + 1] = V[i][k + 1] - p * q;
						}
					}  // (s != 0)
				}  // k loop
			}  // check convergence
		}  // while (n >= low)
      
		// abcksubstitute to find vectors of upper triangular form
		if (norm == 0)
			return;
   
		for (n = nn - 1; n >= 0; n--) {
			p = d[n];
			q = e[n];
   			// real vector
			if (q == 0) {
				GInt32 l = n;
				H[n][n] = 1;
				for (GInt32 i = n - 1; i >= 0; i--) {
					w = H[i][i] - p;
					r = 0;
					for (GInt32 j = l; j <= n; j++)
						r = r + H[i][j] * H[j][n];
					if (e[i] < 0) {
						z = w;
						s = r;
					}
					else {
						l = i;
						if (e[i] == 0) {
							if (w != 0)
								H[i][n] = -r / w;
							else
								H[i][n] = -r / (eps * norm);
							// solve real equations
						}
						else {
							x = H[i][i+1];
							y = H[i+1][i];
							q = (d[i] - p) * (d[i] - p) + e[i] * e[i];
							t = (x * s - z * r) / q;
							H[i][n] = t;
							if (GMath::Abs(x) > GMath::Abs(z))
								H[i + 1][n] = (-r - w * t) / x;
							else
								H[i + 1][n] = (-s - y * t) / z;
						}
						// overflow control
						t = GMath::Abs(H[i][n]);
						if ((eps * t) * t > 1) {
							for (GInt32 j = i; j <= n; j++)
								H[j][n] = H[j][n] / t;
						}
					}
				}
				// complex vector
			}
			else
			if (q < 0) {
				GInt32 l = n - 1;
				// last vector component imaginary so matrix is triangular
				if (GMath::Abs(H[n][n - 1]) > GMath::Abs(H[n - 1][n])) {
					H[n - 1][n - 1] = q / H[n][n - 1];
					H[n - 1][n] = -(H[n][n] - p) / H[n][n - 1];
				}
				else {
					GMath::ComplexDiv((DATA_TYPE)0, -H[n - 1][n], H[n - 1][n - 1] - p, q, cdivr, cdivi);
					H[n - 1][n - 1] = cdivr;
					H[n - 1][n] = cdivi;
				}
				H[n][n - 1] = 0;
				H[n][n] = 1;
				for (GInt32 i = n - 2; i >= 0; i--) {
					DATA_TYPE ra, sa, vr, vi;
					ra = 0;
					sa = 0;
					for (GInt32 j = l; j <= n; j++) {
						ra = ra + H[i][j] * H[j][n - 1];
						sa = sa + H[i][j] * H[j][n];
					}
					w = H[i][i] - p;
   					if (e[i] < 0) {
						z = w;
						r = ra;
						s = sa;
					}
					else {
						l = i;
						if (e[i] == 0) {
							GMath::ComplexDiv(-ra, -sa, w, q, cdivr, cdivi);
							H[i][n - 1] = cdivr;
							H[i][n] = cdivi;
						}
						else {
							// solve complex equations
							x = H[i][i + 1];
							y = H[i + 1][i];
							vr = (d[i] - p) * (d[i] - p) + e[i] * e[i] - q * q;
							vi = (d[i] - p) * 2 * q;
							if ((vr == 0) && (vi == 0)) {
								vr = eps * norm * (GMath::Abs(w) + GMath::Abs(q) +
								GMath::Abs(x) + GMath::Abs(y) + GMath::Abs(z));
							}
							GMath::ComplexDiv(x * r - z * ra + q * sa, x * s - z * sa - q * ra, vr, vi, cdivr, cdivi);
							H[i][n - 1] = cdivr;
							H[i][n] = cdivi;
							if (GMath::Abs(x) > (GMath::Abs(z) + GMath::Abs(q))) {
								H[i + 1][n - 1] = (-ra - w * H[i][n - 1] + q * H[i][n]) / x;
								H[i + 1][n] = (-sa - w * H[i][n] - q * H[i][n - 1]) / x;
							}
							else {
								GMath::ComplexDiv(-r - y * H[i][n - 1], -s - y * H[i][n], z, q, cdivr, cdivi);
								H[i + 1][n - 1] = cdivr;
								H[i + 1][n] = cdivi;
							}
						}
						// overflow control
						t = GMath::Max(GMath::Abs(H[i][n - 1]), GMath::Abs(H[i][n]));
						if ((eps * t) * t > 1) {
							for (GInt32 j = i; j <= n; j++) {
								H[j][n - 1] = H[j][n - 1] / t;
								H[j][n] = H[j][n] / t;
							}
						}
					}
				}
			}
		}
		// vectors of isolated roots
		for (GInt32 i = 0; i < nn; ++i) {
			if (i < low || i > high) {
				for (GInt32 j = i; j < nn; j++)
					V[i][j] = H[i][j];
			}
		}
		// back transformation to get eigenvectors of original matrix
		for (GInt32 j = nn - 1; j >= low; j--) {
			for (GInt32 i = low; i <= high; ++i) {
				z = 0;
				for (GInt32 k = low; k <= GMath::Min(j, high); k++)
					z = z + V[i][k] * H[k][j];
				V[i][j] = z;
			}
		}
	}

	/** Tests 2 matrices for equality within a tolerance
    *  \param lhs    The first matrix
    *  \param rhs    The second matrix
    *  \param eps    The tolerance value
    *  \pre Both matrices must be of the same size.
    *  \return true if the matrices' elements are within the tolerance value of each other; false otherwise
    */
	template <typename DATA_TYPE, GUInt32 ROWS, GUInt32 COLS>
	inline GBool IsEqual(const GMatrix<DATA_TYPE, ROWS, COLS>& lhs,
						const GMatrix<DATA_TYPE, ROWS, COLS>& rhs,
						const DATA_TYPE& eps = (DATA_TYPE)0) {
		G_ASSERT(eps >= (DATA_TYPE)0 );

		for (GUInt32 i = 0; i < ROWS*COLS; ++i) {
			if (!GMath::IsEqual(lhs.gData[i], rhs.gData[i], eps))
				return G_FALSE;
		}
		return G_TRUE;
	}

	//! Common 2x2 matrix class, it uses GReal data type.
	typedef GMatrix<GReal, 2, 2> GMatrix22;
	//! Common 2x3 matrix class, it uses GReal data type.
	typedef GMatrix<GReal, 2, 3> GMatrix23;
	//! Common 3x3 matrix class, it uses GReal data type.
	typedef GMatrix<GReal, 3, 3> GMatrix33;
	//! Common 3x4 matrix class, it uses GReal data type.
	typedef GMatrix<GReal, 3, 4> GMatrix34;
	//! Common 4x3 matrix class, it uses GReal data type.
	typedef GMatrix<GReal, 4, 3> GMatrix43;
	//! Common 4x4 matrix class, it uses GReal data type.
	typedef GMatrix<GReal, 4, 4> GMatrix44;

	//! Identity 2x2 matrix constant.
	static const GMatrix22 G_MATRIX_IDENTITY22 = GMatrix22();
	//! Identity 3x3 matrix constant.
	static const GMatrix33 G_MATRIX_IDENTITY33 = GMatrix33();
	//! Identity 4x4 matrix constant.
	static const GMatrix44 G_MATRIX_IDENTITY44 = GMatrix44();

}; // end namespace Amanith

#endif
