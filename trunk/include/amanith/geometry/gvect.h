/****************************************************************************
** $file: amanith/geometry/gsphere.h   0.1.1.0   edited Sep 24 08:00
**
** Generic static vector definition and operators.
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

#ifndef GVECT_H
#define GVECT_H

#include "amanith/gglobal.h"
#include "amanith/gmath.h"
#include "amanith/support/gmetaprogramming.h"

/*!
	\file gvect.h
	\brief Vector class header file.
*/
namespace Amanith {

	/*!
		\class GVectBase
		\brief Base type for vector-like objects including Points and Vectors.
		
		It is templated on the component datatype as well as the number of components that make it up.

		\param DATA_TYPE the datatype to use for the components
		\param SIZE the number of components this GVectBase has; it must be a positive number.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	class GVectBase {

	protected:
		// The array of components.
		DATA_TYPE gData[SIZE];

	public:
		/*!
			Default constructor.
		
			Does nothing, leaves data alone.
			This is for performance because this constructor is called by derived class constructors, even when they
			just want to set the data directly.
		*/
		GVectBase() {
		}
		/*!
			Copy constructor, makes an exact copy of the given GVectBase object.
		*/
		GVectBase(const GVectBase<DATA_TYPE, SIZE>& SrcVec) {
			GMetaProgramming::AssignVecUnrolled<SIZE - 1, GVectBase<DATA_TYPE,SIZE> >::func(*this, SrcVec);
		}

		//! Creates a new VecBase initialized to the given values.
		GVectBase(const DATA_TYPE& Val0, const DATA_TYPE& Val1) {

			G_ASSERT(SIZE == 2);
			this->gData[0] = Val0;
			this->gData[1] = Val1;
		}
		//! Creates a new VecBase initialized to the given values.
		GVectBase(const DATA_TYPE& Val0, const DATA_TYPE& Val1, const DATA_TYPE& Val2) {

			G_ASSERT(SIZE == 3);
			this->gData[0] = Val0;
			this->gData[1] = Val1;
			this->gData[2] = Val2;
		}
		//! Creates a new VecBase initialized to the given values.
		GVectBase(const DATA_TYPE& Val0, const DATA_TYPE& Val1, const DATA_TYPE& Val2, const DATA_TYPE& Val3) {

			G_ASSERT(SIZE == 4);
			this->gData[0] = Val0;
			this->gData[1] = Val1;
			this->gData[2] = Val2;
			this->gData[3] = Val3;
		}
		/*!
			Set all components.

			\note DataPtr must be of a length of at least SIZE
		*/
		inline void Set(const DATA_TYPE* DataPtr){
			GMetaProgramming::AssignArrayUnrolled<SIZE-1, DATA_TYPE>::func(&(gData[0]), DataPtr);
		}
		//! Set first element to the given value
		inline void Set(const DATA_TYPE& Val0) {
			G_ASSERT(SIZE >= 1);
			gData[0] = Val0;
		}
		//! Set first 2 elements to the given values. Vector must have a SIZE of at least 2.
		inline void Set(const DATA_TYPE& Val0, const DATA_TYPE& Val1) {
			G_ASSERT(SIZE >= 2);
			gData[0] = Val0;
			gData[1] = Val1;
		}
		//! Set first 3 elements to the given values. Vector must have a SIZE of at least 3.
		inline void Set(const DATA_TYPE& Val0, const DATA_TYPE& Val1, const DATA_TYPE& Val2) {
			G_ASSERT(SIZE >= 3);
			gData[0] = Val0;
			gData[1] = Val1;
			gData[2] = Val2;
		}
		//! Set first 4 elements to the given values. Vector must have a SIZE of at least 4.
		inline void Set(const DATA_TYPE& Val0, const DATA_TYPE& Val1, const DATA_TYPE& Val2, const DATA_TYPE& Val3) {
			G_ASSERT(SIZE >= 4);
			gData[0] = Val0;
			gData[1] = Val1;
			gData[2] = Val2;
			gData[3] = Val3;
		}
		//! Get a pointer to the internal data array.
		const DATA_TYPE* Data() const {
			return gData;
		}
		//! Return the value at given location.
		inline DATA_TYPE& operator [](const GUInt32 Index) {
			G_ASSERT(Index < SIZE);
			return gData[Index];
		}
		//! Return the value at given location (const version).
		inline const DATA_TYPE& operator [](const GUInt32 Index) const {
			G_ASSERT(Index < SIZE);
			return gData[Index];
		}
		//! Swap two data element
		inline void Swap(const GUInt32 Index1, const GUInt32 Index2) {
			G_ASSERT(Index1 < SIZE);
			G_ASSERT(Index2 < SIZE);
			DATA_TYPE tmpData = gData[Index1];
			gData[Index1] = gData[Index2];
			gData[Index2] = tmpData;
		}
		//! Fills all components to the given value.
		inline void Fill(const DATA_TYPE Value) {
			GMetaProgramming::FillArrayUnrolled<SIZE - 1, DATA_TYPE >::func(gData, Value);
		}

		/*!
			Compares vectors if they are exactly the same.

			\param Vect the vector to compare to
			\return G_TRUE if vectors are identical, G_FALSE if they differ.
		*/
		inline GBool operator ==(const GVectBase<DATA_TYPE, SIZE>& Vect) const {
			return GMetaProgramming::EqualVecUnrolled<SIZE-1, GVectBase<DATA_TYPE, SIZE> >::func(*this, Vect);
		}

		/*!
			Compares vectors if they are NOT exactly the same with zero tolerance.

			\param Vect the vector to compare to
			\return G_TRUE if vectors are different, G_FALSE otherwise.
		*/
		inline GBool operator !=(const GVectBase<DATA_TYPE, SIZE>& Vect) const {
			return (!this->operator==(Vect));
		}

	};	// end class GVectBase


	/*!
		\class GVect
		\brief A representation of a vector with SIZE components using DATA_TYPE as the data type for each component.
	
		\param DATA_TYPE the datatype to use for the components
		\param SIZE the number of components this VecBase has; it must be a positive number.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	class GVect : public GVectBase<DATA_TYPE, SIZE> {
	public:
		//! The superclass type.
		typedef GVectBase<DATA_TYPE, SIZE> BaseType;
		
	public:
		//!	Default constructor. All components are initialized to zero.
		GVect() {
			GMetaProgramming::FillArrayUnrolled<SIZE - 1, DATA_TYPE >::func(this->gData, (DATA_TYPE)0);
		}

		/*!

			Copy constructor, make an exact copy of the given GVect object.
			
			\pre vector should be the same size and type as the one copied
			\param SrcVect the GVect object to copy
		*/
		GVect(const GVect<DATA_TYPE, SIZE>& SrcVect) : BaseType(static_cast<BaseType>(SrcVect)) {
		}
   
		//!	Copy constructor, make an exact copy of the given SrcBaseVect object.
		GVect(const GVectBase<DATA_TYPE, SIZE>& SrcBaseVect) : BaseType(SrcBaseVect) {
		}

		//! Creates a new vector initialized to the given values.
		GVect(const DATA_TYPE& Val0, const DATA_TYPE& Val1) : BaseType(Val0, Val1) {
		}

		//! Creates a new vector initialized to the given values.
		GVect(const DATA_TYPE& Val0, const DATA_TYPE& Val1, const DATA_TYPE& Val2) : BaseType(Val0, Val1, Val2) {
		}

		//! Creates a new vector initialized to the given values.
		GVect(const DATA_TYPE& Val0, const DATA_TYPE& Val1, const DATA_TYPE& Val2, const DATA_TYPE& Val3)
		: BaseType(Val0, Val1, Val2, Val3) {
		}

		/*!
			Computes the square of the squared length of the vector. This can be used in
			many calculations instead of length to increase speed by saving you an expensive sqrt call.
		*/
		inline DATA_TYPE LengthSquared() const {
			return GMetaProgramming::LenSqrVecUnrolled<SIZE - 1, GVect<DATA_TYPE, SIZE>, DATA_TYPE >::func(*this);
		}
		//! Computes the length of the vector.
		DATA_TYPE Length() const {

			DATA_TYPE ret_val = LengthSquared();

			if (ret_val == DATA_TYPE(0))
				return DATA_TYPE(0);
			else
				return GMath::Sqrt(ret_val);
		}

		/*!
			Normalizes the vector in place causing it to be of unit length. If the vector is already of
			length 1.0, nothing is done. For convenience, the original length of the vector is returned.

			\return the length of vector before it was normalized.
		*/
		DATA_TYPE Normalize() {

			DATA_TYPE len = Length();

			if (len != DATA_TYPE(0)) {
				for (GUInt32 i = 0; i < SIZE; ++i)
					this->gData[i] /= len;
			}
			return len;
		}

		/*!
			Determines if the vector is normalized within the given tolerance. The vector is normalized if its
			LengthSquared is 1.
			
			\param Epsilon the epsilon tolerance, must be >= 0.
			\return G_TRUE if the vector is normalized, G_FALSE otherwise.
		*/
		inline GBool IsNormalized(const DATA_TYPE Epsilon = DATA_TYPE(2.5 * G_EPSILON)) const {
			return GMath::IsEqual(LengthSquared(), (DATA_TYPE)1, Epsilon);
		}

	};	// end class GVect


	/*!
		\class GPoint
		\brief A geometric point entity.

		Use points when you need to represent a position. Don't use points to represent a vector. One difference
		you should note is that certain matrix operations are different between GPoint and GVect such as
		transform and operator*. A GVect transform by matrix is simply a rotation, while a GPoint transformed by a
		matrix is a full matrix transform (rotation, skew, translation, scale).
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	class GPoint : public GVectBase<DATA_TYPE, SIZE> {
	public:
		//! Placeholder for the base type
		typedef GVectBase<DATA_TYPE, SIZE> BaseType;

	public:
		//!	Default constructor. All components are initialized to zero.
		GPoint() {
			GMetaProgramming::FillArrayUnrolled<SIZE - 1, DATA_TYPE >::func(this->gData, (DATA_TYPE)0);
		}

		/*!
			Copy constructor, construct a point with copy of SrcPoint
		*/
		GPoint(const GPoint<DATA_TYPE, SIZE>& SrcPoint) : BaseType(static_cast<BaseType>(SrcPoint)) {
		}

		/*!
			Copy constructor, construct a point with copy of SrcBaseVect
		*/
		GPoint(const GVectBase<DATA_TYPE, SIZE>& SrcBaseVect) : BaseType(SrcBaseVect) {
		}

		//! Construct a 2D point with 2 given values
		GPoint(const DATA_TYPE& Val0, const DATA_TYPE& Val1) : BaseType(Val0, Val1) {
		}

		//! Construct a 3D point with 3 given values
		GPoint(const DATA_TYPE& Val0, const DATA_TYPE& Val1, const DATA_TYPE& Val2)	: BaseType(Val0, Val1, Val2) {
		}

		//! Construct a 4D point with 4 given values
		GPoint(const DATA_TYPE& Val0, const DATA_TYPE& Val1, const DATA_TYPE& Val2, const DATA_TYPE& Val3)
		: BaseType(Val0, Val1, Val2, Val3) {
		}

	};	// end class GPoint


	//! Common 2D vector class, it uses GReal data type.
	typedef GVect<GReal, 2> GVector2;
	//! Common 3D vector class, it uses GReal data type.
	typedef GVect<GReal, 3> GVector3;
	//! Common 4D vector class, it uses GReal data type.
	typedef GVect<GReal, 4> GVector4;
	//! Common 2D point class, it uses GReal data type.
	typedef GPoint<GReal, 2> GPoint2;
	//! Common 3D point class, it uses GReal data type.
	typedef GPoint<GReal, 3> GPoint3;
	//! Common 4D point class, it uses GReal data type.
	typedef GPoint<GReal, 4> GPoint4;

	//! Static 2D point located at the origin
	static const GPoint2 G_NULL_POINT2 = GPoint2();
	//! Static 3D point located at the origin
	static const GPoint3 G_NULL_POINT3 = GPoint3();
	//! Static 4D point located at the origin
	static const GPoint4 G_NULL_POINT4 = GPoint4();


	//! Unary - operator
	template<typename DATA_TYPE, GUInt32 SIZE>
	GVect<DATA_TYPE, SIZE> operator -(const GVectBase<DATA_TYPE, SIZE>& v1) {

		GVect<DATA_TYPE, SIZE> ret_val;
		for (GUInt32 i = 0; i < SIZE; ++i)
			ret_val[i] = -v1[i];
		return ret_val;
	}

	/*!
		Adds v2 to v1 and stores the result in V1. This is equivalent to the expression v1 = v1 + v2.

		\param v1 the first vector
		\param v2 the second vector
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	GVectBase<DATA_TYPE, SIZE>& operator +=(GVectBase<DATA_TYPE, SIZE>& v1,	const GVectBase<DATA_TYPE, SIZE>& v2) {
		for (GUInt32 i = 0; i < SIZE; ++i)
			v1[i] += v2[i];
		return v1;
	}

	/*!
		Adds v2 to v1 and returns the result. Thus result = v1 + v2.
	
		\param v1 the first vector
		\param v2 the second vector
		\return the result of adding v2 to v1
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	GVectBase<DATA_TYPE, SIZE> operator +(const GVectBase<DATA_TYPE, SIZE>& v1, const GVectBase<DATA_TYPE, SIZE>& v2) {
		GVectBase<DATA_TYPE, SIZE> ret_val(v1);
		ret_val += v2;
		return ret_val;
	}

	/*!
		Subtracts v2 from v1 and stores the result in v1. This is equivalent to the expression v1 = v1 - v2.

		\param v1 the first vector
		\param v2 the second vector
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	GVectBase<DATA_TYPE, SIZE>& operator -=(GVectBase<DATA_TYPE, SIZE>& v1, const GVectBase<DATA_TYPE, SIZE>& v2) {
		for (GUInt32 i = 0; i < SIZE; ++i)
			v1[i] -= v2[i];
		return v1;
	}

	/*!
		Subtracts v2 from v1 and returns the result. Thus result = v1 - v2.

		\param v1 the first vector
		\param v2 the second vector
		\return the result of subtracting v2 from v1
	*/
	template <typename DATA_TYPE, GUInt32 SIZE>
	GVect<DATA_TYPE, SIZE> operator -(const GVectBase<DATA_TYPE, SIZE>& v1, const GVectBase<DATA_TYPE, SIZE>& v2) {

		GVect<DATA_TYPE, SIZE> ret_val(v1);
		ret_val -= v2;
		return ret_val;
	}

	/*!
		Multiplies v1 by a scalar value and stores the result in v1. This is equivalent to the expression v1 = v1 * scalar.

		\param v1 the vector to scale
		\param scalar the amount by which to scale v1
	*/
	template<typename DATA_TYPE, GUInt32 SIZE, typename SCALAR_TYPE>
	GVectBase<DATA_TYPE, SIZE>& operator *=(GVectBase<DATA_TYPE, SIZE>& v1, const SCALAR_TYPE& scalar) {
		for (GUInt32 i = 0; i < SIZE; ++i)
			v1[i] *= (DATA_TYPE)scalar;
		return v1;
	}

	/*!
		Multiplies v1 by a scalar value and returns the result. Thus result = v1 * scalar.

		\param v1 the vector to scale
		\param scalar the amount by which to scale v1
		\return the result of multiplying v1 by scalar
	*/
	template<typename DATA_TYPE, GUInt32 SIZE, typename SCALAR_TYPE>
	GVectBase<DATA_TYPE, SIZE> operator *(const GVectBase<DATA_TYPE, SIZE>& v1, const SCALAR_TYPE& scalar) {
		GVectBase<DATA_TYPE, SIZE> ret_val(v1);
		ret_val *= scalar;
		return ret_val;
	}

	/*!
		Multiplies v1 by a scalar value and returns the result. Thus result = scalar * v1.
		This is equivalent to result = v1 * scalar.
		
		\param scalar the amount by which to scale v1
		\param v1 the vector to scale
		\return the result of multiplying v1 by scalar
	*/
	template<typename DATA_TYPE, GUInt32 SIZE, typename SCALAR_TYPE>
	GVectBase<DATA_TYPE, SIZE> operator *(const SCALAR_TYPE& scalar, const GVectBase<DATA_TYPE, SIZE>& v1) {
		GVectBase<DATA_TYPE, SIZE> ret_val(v1);
		ret_val *= scalar;
		return ret_val;
	}

	/*!
		Divides v1 by a scalar value and stores the result in v1. This is equivalent to the expression v1 = v1 / scalar.

		\param v1 the vector to scale
		\param scalar the amount by which to scale v1
		1return v1 after it has been divided by scalar
	*/
	template<typename DATA_TYPE, GUInt32 SIZE, typename SCALAR_TYPE>
	GVectBase<DATA_TYPE, SIZE>& operator /=(GVectBase<DATA_TYPE, SIZE>& v1, const SCALAR_TYPE& scalar) {

		for (GUInt32 i = 0; i < SIZE; ++i)
			v1[i] /= (SCALAR_TYPE)scalar;
		return v1;
	}

	/*!
		Divides v1 by a scalar value and returns the result. Thus result = v1 / scalar.

		\param v1 the vector to scale
		\param scalar the amount by which to scale v1
		\return the result of dividing v1 by scalar
	*/
	template<typename DATA_TYPE, GUInt32 SIZE, typename SCALAR_TYPE>
	GVectBase<DATA_TYPE, SIZE> operator /(const GVectBase<DATA_TYPE, SIZE>& v1,	const SCALAR_TYPE& scalar) {
		GVectBase<DATA_TYPE, SIZE> ret_val(v1);
		ret_val /= scalar;
		return ret_val;
	}

	/*!
		Computes dot product of v1 and v2 and returns the result.

		\param v1 the first vector
		\param v2 the second vector
		\return the dot product of v1 and v2
		\note Dot product is commutative
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	DATA_TYPE Dot(const GVect<DATA_TYPE, SIZE>& v1, const GVect<DATA_TYPE, SIZE>& v2) {
		return GMetaProgramming::DotVecUnrolled<SIZE-1, GVect<DATA_TYPE, SIZE>, DATA_TYPE >::func(v1, v2);
	}

	/*!
		Computes the cross product between v1 and v2 and returns the result.
		Note that this only applies to 3D vectors.

		\param v1 the first vector
		\param v2 the second vector
		\result the cross product of v1 x v2
		\note Cross product is not commutative
	*/
	template<typename DATA_TYPE>
	GVect<DATA_TYPE, 3> Cross(const GVect<DATA_TYPE, 3>& v1, const GVect<DATA_TYPE, 3>& v2) {
		return GVect<DATA_TYPE, 3>((v1[G_Y] * v2[G_Z]) - (v1[G_Z] * v2[G_Y]),
								   (v1[G_Z] * v2[G_X]) - (v1[G_X] * v2[G_Z]),
								   (v1[G_X] * v2[G_Y]) - (v1[G_Y] * v2[G_X]));
	}

	/*!
		Computes the third (Z) coordinate of the 3D vector obtained with a cross product of two 2D
		vectors (they are considered 3D vectors with a Z = 0)

		\param v1 the first vector.
		\param v2 the second vector.
	*/
	template<typename DATA_TYPE>
	DATA_TYPE Cross(const GVect<DATA_TYPE, 2>& v1, const GVect<DATA_TYPE, 2>& v2) {
		return ((v1[G_X] * v2[G_Y]) - (v2[G_X] * v1[G_Y]));
	}

	/*!
		Reflect a vector about a normal.

		\param Vect the original vector that we want to reflect.
		\param Normal the normal vector.
		\result the reflected vector.
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	GVectBase<DATA_TYPE, SIZE> Reflect(const GVectBase<DATA_TYPE, SIZE>& Vect, const GVect<DATA_TYPE, SIZE>& Normal) {
		return (Vect - DATA_TYPE(2) * Dot((GVect<DATA_TYPE, SIZE>&)Vect, Normal) * Normal);
	}

	/*!
		Computes the squared length of the vector.
	*/
	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE LengthSquared(const GVect<DATA_TYPE, SIZE>& Vect) {
		return Vect.LengthSquared();
	}


	/*!
		Computes the length of the vector. 
	*/
	template <typename DATA_TYPE, GUInt32 SIZE>
	inline DATA_TYPE Length(const GVect<DATA_TYPE, SIZE>& Vect) {
		return Vect.Length();
	}

	/*!
		Compares Vect1 and Vect1 to see if they are the same within the given epsilon tolerance.

		\param Vect1 the first vector
		\param Vect2 the second vector
		\param Epsilon the epsilon tolerance value, it must be >= 0
		\return G_TRUE if v1 equals v2 within the tolerance; G_FALSE if they differ
	*/
	template<typename DATA_TYPE, GUInt32 SIZE>
	inline GBool IsEqual(const GVectBase<DATA_TYPE, SIZE>& Vect1,
						 const GVectBase<DATA_TYPE, SIZE>& Vect2, const DATA_TYPE& Epsilon = G_EPSILON) {
		G_ASSERT(Epsilon >= 0);
		for (GUInt32 i = 0; i < SIZE; ++i) {
			if (GMath::Abs(Vect1[i] - Vect2[i]) > Epsilon)
				return G_FALSE;
		}
		return G_TRUE;
	}

	/*!
		Finds if a contour specified by a sequence of points has been given in a clockwise order.

		Algorithm: find the lowest, rightmost point of the polygon, and then take the cross product of the edges
		before and after of it. The reason that the lowest, rightmost point works is that the
		internal angle at this vertex is necessarily convex, strictly less than pi (even if there are
		several equally-lowest points)

		\param Contour the sequence of points defining the contour.
		\result G_TRUE if points have been given in a clockwise order, G_FALSE otherwise.
	*/
	template<typename DATA_TYPE>
	GBool IsClockWise(const GDynArray< GPoint<DATA_TYPE, 2> >& Contour) {

		GInt32 s = (GInt32)Contour.size();
		if (Contour.size() <= 1)
			return G_FALSE;

		GInt32 rmin = 0;
		DATA_TYPE xmin = Contour[0][G_X];
		DATA_TYPE ymin = Contour[0][G_Y];
		DATA_TYPE crossProd;
		// first find rightmost lowest vertex of the polygon
		for (GInt32 i = 1; i < s; ++i) {
			if (Contour[i][G_Y] > ymin)
				continue;
			if (Contour[i][G_Y] == ymin) {
				if (Contour[i][G_X] < xmin)
					continue;
			}
			// a new rightmost lowest vertex
			rmin = i;
			xmin = Contour[i][G_X];
			ymin = Contour[i][G_Y];
		}
		// test orientation at this rmin vertex
		// clockwise <=> the edge leaving is not left of the entering edge
		if (rmin == 0)
			crossProd = Cross(Contour[0] - Contour[s - 1], Contour[1] - Contour[s - 1]);
		else
		if (rmin == s - 1)
			crossProd = Cross(Contour[rmin] - Contour[rmin - 1], Contour[0] - Contour[rmin - 1]);
		else
			crossProd = Cross(Contour[rmin] - Contour[rmin - 1], Contour[rmin + 1] - Contour[rmin - 1]);
		return (crossProd < 0);
	}

};	// end namespace Amanith

#endif
