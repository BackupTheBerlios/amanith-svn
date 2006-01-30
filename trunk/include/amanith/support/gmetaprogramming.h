/****************************************************************************
** $file: amanith/metaprogramming.h   0.3.0.0   edited Jan, 30 2006
**
** Metaprogramming utilities.
**
**
** Copyright (C) 2004-2006 Mazatech Inc. All rights reserved.
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

#ifndef GMETAPROGRAMMING_H
#define GMETAPROGRAMMING_H

/*!
	\file gmetaprogramming.h
	\brief Meta programming functions and utilities.
*/


namespace Amanith {

   /* A lightweight identifier you can pass to overloaded functions
    *  to typify them.
    *
    *  Type2Type lets you transport the type information about T to functions
    */
   template <typename T>
   struct Type2Type {
      typedef T OriginalType;
   };

}  // end namespace Amanith


namespace Amanith {

	/*!
		\namespace Amanith::GMetaProgramming
		\brief Meta programming functions and utilities
	*/
	namespace GMetaProgramming {

		// template programs for vector assignment unrolled
		template<GInt32 ELT, typename T>
		struct AssignVecUnrolled {
			static void func(T& lVec, const T& rVec) {
				AssignVecUnrolled<ELT-1,T>::func(lVec, rVec);
				lVec[ELT] = rVec[ELT];
			}
		};

		template<typename T>
		struct AssignVecUnrolled<0, T> {
			static void func(T& lVec, const T& rVec) {
				lVec[0] = rVec[0];
			}
		};

		// template programs for array assignment unrolled
		template<GInt32 ELT, typename T>
		struct AssignArrayUnrolled {
			static void func(T* lVec, const T* rVec) {
				AssignArrayUnrolled<ELT-1, T>::func(lVec, rVec);
				lVec[ELT] = rVec[ELT];
			}
		};

		template<typename T>
		struct AssignArrayUnrolled<0, T> {
			static void func(T* lVec, const T* rVec) {
				lVec[0] = rVec[0];
			}
		};

		/** meta class to unroll filling operations. */
		template<GInt32 ELT, typename T>
		struct FillArrayUnrolled {
			static void func(T* lVec, const T value) {
				FillArrayUnrolled<ELT-1, T>::func(lVec, value);
				lVec[ELT] = value;
			}
		};
		/** base case for filling operations unrolling. */
		template<typename T>
		struct FillArrayUnrolled<0, T> {
			static void func(T* lVec, const T value) {
				lVec[0] = value;
			}
		};


		/** meta class to unroll dot products. */
		template<GInt32 ELT, typename T, typename RESULT_TYPE>
		struct DotVecUnrolled {
			static RESULT_TYPE func(const T& v1, const T& v2)	{
				return (v1[ELT] * v2[ELT]) + DotVecUnrolled<ELT-1, T, RESULT_TYPE>::func(v1, v2);
			}
		};

		/** base case for dot product unrolling. */
		template<typename T, typename RESULT_TYPE>
		struct DotVecUnrolled<0, T, RESULT_TYPE> {
			static RESULT_TYPE func(const T& v1, const T& v2)	{
				return (v1[0] * v2[0]);
			}
		};

		/** meta class to unroll length squared operation. */
		template<GInt32 ELT, typename T, typename RESULT_TYPE>
		struct LenSqrVecUnrolled {
			static RESULT_TYPE func(const T& v) {
				return (v[ELT] * v[ELT]) + LenSqrVecUnrolled<ELT-1, T, RESULT_TYPE>::func(v);
			}
		};

		/** base case for dot product unrolling. */
		template<typename T, typename RESULT_TYPE>
		struct LenSqrVecUnrolled<0, T, RESULT_TYPE> {
			static RESULT_TYPE func(const T& v) {
				return (v[0] * v[0]);
			}
		};

		/** meta class to test vector equality. */
		template<GInt32 ELT, typename VT>
		struct EqualVecUnrolled	{
			static GBool func(const VT& v1, const VT& v2) {
				return (v1[ELT] == v2[ELT]) && EqualVecUnrolled<ELT-1, VT>::func(v1, v2);
			}
		};

		/** base case for dot product unrolling. */
		template<typename VT>
		struct EqualVecUnrolled<0, VT> {
			static GBool func(const VT& v1, const VT& v2) {
				return (v1[0] == v2[0]);
			}
		};

	}  // end namespace GMetaProgramming

}  // end namespace Amanith

#endif
