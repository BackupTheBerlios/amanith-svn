/****************************************************************************
** $file: amanith/geometry/gquat.h   0.3.0.0   edited Jan, 30 2006
**
** Quaternion definition and relative operators.
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

#ifndef GQUAT_H
#define GQUAT_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gvect.h"

/*!
	\file gquat.h
	\brief Quaternion class header file.
 */
namespace Amanith {

	/*!
		\class GQuat
		\brief This template class encapsulate quaternion behaviors.

		For exhaustive informations about quaternions, please have a see at these references:
		- Advanced Animation and Rendering Techniques: pp363-365
		- Animating Rotation with Quaternion Curves, Ken Shoemake,	SIGGRAPH Proceedings Vol 19, Number 3, 1985
		- Quaternion Calculus for Animation,  Ken Shoemake SIGGRAPH course notes 1989
		- Game Developer Magazine: Feb 98, pg.34-42
		- On quaternions; or on a new system of imaginaries in algebra, Sir William Rowan Hamilton, Philosophical
		Magazine, xxv, pp. 10-13 (July 1844)
		- You also can find more on quaternions at http://www.gamasutra.com/features/19980703/quaternions_01.htm

		\param DATA_TYPE the internal type used for the quaternion components.
	*/
	template <typename DATA_TYPE>
	class GQuat {

	private:
		//! Quaternion components, order is X, Y, Z, W
		DATA_TYPE gData[4];

	protected:
		/*!
			Product of two quaternions (quaternion product).

			Multiplication of quaternions is much like multiplication of typical complex numbers.
			\result Quat1 * Quat2 (where Quat2 would be applied first to any transformed geometry)
		*/
		void Mult(GQuat<DATA_TYPE>& Result, const GQuat<DATA_TYPE>& Quat1, const GQuat<DATA_TYPE>& Quat2) {

			// temporary variable to avoid aliasing problems
			GQuat<DATA_TYPE> temporary(
				Quat1[G_W] * Quat2[G_X] + Quat1[G_X] * Quat2[G_W] + Quat1[G_Y] * Quat2[G_Z] - Quat1[G_Z] * Quat2[G_Y],
				Quat1[G_W] * Quat2[G_Y] + Quat1[G_Y] * Quat2[G_W] + Quat1[G_Z] * Quat2[G_X] - Quat1[G_X] * Quat2[G_Z],
				Quat1[G_W] * Quat2[G_Z] + Quat1[G_Z] * Quat2[G_W] + Quat1[G_X] * Quat2[G_Y] - Quat1[G_Y] * Quat2[G_X],
				Quat1[G_W] * Quat2[G_W] - Quat1[G_X] * Quat2[G_X] - Quat1[G_Y] * Quat2[G_Y] - Quat1[G_Z] * Quat2[G_Z]
			);
			// use a temporary, in case q1 or q2 is the same as self.
			Result[G_X] = temporary[G_X];
			Result[G_Y] = temporary[G_Y];
			Result[G_Z] = temporary[G_Z];
			Result[G_W] = temporary[G_W];
			// don't normalize, because it might not be rotation arithmetic we're doing
			// (only rotation quaternions have unit length)
		}

		/*!
			Quotient of two quaternions.

			\result = Quat1 * (1 / Quat2)  (where 1 / Quat2 is applied first to any transformed geometry)
		*/
		void Div(GQuat<DATA_TYPE>& Result, const GQuat<DATA_TYPE>& Quat1, const GQuat<DATA_TYPE> Quat2) {
			// multiply q1 by the multiplicative inverse of the quaternion
			return Mult(Result, Quat1, Quat2.Invert());
		}

	public:
		/*!
			Default constructor, initializes to quaternion multiplication identity [x, y, z, w] == [0, 0, 0, 1].
			\note the addition identity is [0, 0, 0, 0]
		*/
		GQuat() {
			gData[0] = 0;
			gData[1] = 0;
			gData[2] = 0;
			gData[3] = 1;
		}
		   
		//!	Set constructor
		GQuat(const DATA_TYPE X, const DATA_TYPE Y, const DATA_TYPE Z, const DATA_TYPE W) {
			 gData[0] = X;
			 gData[1] = Y;
			 gData[2] = Z;
			 gData[3] = W;
		}

		//! Copy constructor
		GQuat(const GQuat<DATA_TYPE>& Source) {
			gData[0] = Source.gData[0];
			gData[1] = Source.gData[1];
			gData[2] = Source.gData[2];
			gData[3] = Source.gData[3];
		}

		//! Directly set the quaternion's values
		void Set(const DATA_TYPE X, const DATA_TYPE Y, const DATA_TYPE Z, const DATA_TYPE W) {
			gData[0] = X;
			gData[1] = Y;
			gData[2] = Z;
			gData[3] = W;
		}

		//! Get the raw data elements of the quaternion.
		void Get(DATA_TYPE& X, DATA_TYPE& Y, DATA_TYPE& Z, DATA_TYPE& W) {
			X = gData[G_X];
			Y = gData[G_Y];
			Z = gData[G_Z];
			W = gData[G_W];
		}

		//! Bracket operator, raw data accessor.
		DATA_TYPE& operator[](const GUInt32 Index) {
			G_ASSERT(Index < 4);
			return gData[Index];
		}

		//! Bracket operator, raw data accessor (const version).
		const DATA_TYPE& operator[](const GUInt32 Index) const {
			G_ASSERT(Index < 4);
			return gData[Index];
		}

		//! Get a pointer to the quaternion internal data.
		const DATA_TYPE* Data() const {
			return gData;
		}

		//! Swap function: swap two data elements.
		inline void Swap(const GUInt32 Index1, const GUInt32 Index2) {
			G_ASSERT(Index1 < 4);
			G_ASSERT(Index2 < 4);
			DATA_TYPE tmpData = gData[Index1];
			gData[Index1] = gData[Index2];
			gData[Index2] = tmpData;
		}

		/*!
			Compare two quaternions to see if they are EXACTLY the same. In other words, this comparison is done with
			zero tolerance.

			\param Quat the quaternion to compare to
			\return G_TRUE if they are equal, G_FALSE otherwise.
		*/
		inline GBool operator ==(const GQuat<DATA_TYPE>& Quat) const {
			return GBool(gData[0] == Quat.gData[0] && gData[1] == Quat.gData[1] &&
						 gData[2] == Quat.gData[2] && gData[3] == Quat.gData[3]);
		}

		/*!
			Compare two quaternions to see if they are not EXACTLY the same. In other words, this comparison is done with
			zero tolerance.

			\param Quat the quaternion to compare to
			\return G_TRUE if they are not equal, G_FALSE otherwise.
		*/
		inline GBool operator !=(const GQuat<DATA_TYPE>& Quat) const {
			return (!this->operator==(Quat));
		}

		/*!
			Quaternion scalar multiplication.
		*/
		GQuat<DATA_TYPE>& operator *=(const DATA_TYPE s) {

			gData[0] *= s;
			gData[1] *= s;
			gData[2] *= s;
			gData[3] *= s;
			return(*this);
		}

		/*!
			Quaternion scalar multiplication.
		*/
		GQuat<DATA_TYPE> operator *(const DATA_TYPE s) {

			GQuat<DATA_TYPE> temporary(*this);

			temporary *= s;
			return temporary;
		}

		/*!
			Quaternion negation, unary operator-.

			Return a quaternion that is the negative of this quaternion.
			The negative of a rotation quaternion is geometrically equivalent
			to the original. There exist 2 quaternions for every possible rotation.

			\return returns the negation of the given quaternion.
		*/
		GQuat<DATA_TYPE> operator -() {
			return GQuat<DATA_TYPE>(-gData[0], -gData[1], -gData[2], -gData[3]);
		}

		/*!
			Quaternion scalar division.
		*/
		GQuat<DATA_TYPE>& operator /=(const DATA_TYPE s) {

				gData[0] /= s;
				gData[1] /= s;
				gData[2] /= s;
				gData[3] /= s;
				return(*this);
			}

		/*!
			Quaternion scalar division.
		*/
		GQuat<DATA_TYPE> operator /(const DATA_TYPE s) {

			GQuat<DATA_TYPE> temporary(*this);

			temporary /= s;
			return temporary;
		}

		/*!
			Quaternion addition.
		*/
		GQuat<DATA_TYPE>& operator +=(const GQuat<DATA_TYPE>& Quat) {

			gData[0] += Quat.gData[0];
			gData[1] += Quat.gData[1];
			gData[2] += Quat.gData[2];
			gData[3] += Quat.gData[3];
			return(*this);
		}

		/*!
			Quaternion addition.
		*/
		GQuat<DATA_TYPE> operator +(const GQuat<DATA_TYPE>& Quat) {

			GQuat<DATA_TYPE> temporary(*this);

			temporary += Quat;
			return temporary;
		}

		/*!
			Quaternion subtraction.
		*/
		GQuat<DATA_TYPE>& operator -=(const GQuat<DATA_TYPE>& Quat) {

			gData[0] -= Quat.gData[0];
			gData[1] -= Quat.gData[1];
			gData[2] -= Quat.gData[2];
			gData[3] -= Quat.gData[3];
			return(*this);
		}

		/*!
			Quaternion subtraction.
		*/
		GQuat<DATA_TYPE> operator -(const GQuat<DATA_TYPE>& Quat) {

			GQuat<DATA_TYPE> temporary(*this);

			temporary -= Quat;
			return temporary;
		}

		/*!
			Product of two quaternions (quaternion product).

			Does quaternion multiplication.
			\return q1 * q2 (where q2 would be applied first to any transformed geometry)
			\todo meta programming on quaternion operator*()
		*/
		GQuat<DATA_TYPE> operator*(const GQuat<DATA_TYPE>& Quat) {
			// don't normalize, because it might not be rotation arithmetic we're doing
			// (only rotation quaternions have unit length)
			return GQuat<DATA_TYPE>(
				gData[G_W] * Quat[G_X] + gData[G_X] * Quat[G_W] + gData[G_Y] * Quat[G_Z] - gData[G_Z] * Quat[G_Y],
				gData[G_W] * Quat[G_Y] + gData[G_Y] * Quat[G_W] + gData[G_Z] * Quat[G_X] - gData[G_X] * Quat[G_Z],
				gData[G_W] * Quat[G_Z] + gData[G_Z] * Quat[G_W] + gData[G_X] * Quat[G_Y] - gData[G_Y] * Quat[G_X],
				gData[G_W] * Quat[G_W] - gData[G_X] * Quat[G_X] - gData[G_Y] * Quat[G_Y] - gData[G_Z] * Quat[G_Z]
			);
		}

		/*!
			Quaternion post multiplication
			\return this * q2 (where q2 is applied first to any transformed geometry)
		*/
		GQuat<DATA_TYPE>& operator *=(const GQuat<DATA_TYPE>& Quat) {
			
			Mult(*this, *this, Quat);
			return(*this);
		}

		/*!
			Quotient of two quaternions
			\result this * (1 / Quat) (where 1 / Quat is applied first to any transformed geometry)
		*/
		GQuat<DATA_TYPE> operator /(const GQuat<DATA_TYPE>& Quat) {
			return ((*this) * Quat.Invert());
		}

		/*!
			Quotient of two quaternions.
		*/
		GQuat<DATA_TYPE>& operator /=(const GQuat<DATA_TYPE>& Quat) {
			Div(*this, *this, Quat);
			return (*this);
		}

		/*!
			Quaternion "norm" (also known as vector length squared).

			Using this can be faster than using length for some operations.
		*/
		DATA_TYPE LengthSquared() const {
			return DATA_TYPE((gData[0] * gData[0]) + (gData[1] * gData[1]) + (gData[2] * gData[2]) + (gData[3] * gData[3]));
		}

		/*!
			Quaternion "absolute" (also known as vector length or magnitude).
		*/
		DATA_TYPE Length() const {
			return GMath::Sqrt(this->LengthSquared());
		}

		/*!
			Normalized this quaternion.
			
			\pre magnitude should be > 0, otherwise no calculation is done.
			\result the quaternion length before normalization.
		*/
		DATA_TYPE Normalize() {

			DATA_TYPE l = this->Length();

			// return if no magnitude (already as normalized as possible)
			if (l != 0) {
				gData[G_X] /= l;
				gData[G_Y] /= l;
				gData[G_Z] /= l;
				gData[G_W] /= l;
			}
			return l;
		}

		/*!
			Determines if the given quaternion is normalized within the given tolerance. The
			quaternion is normalized if its lengthSquared is 1.

			\param Epsilon the epsilon tolerance
			\return G_TRUE if the quaternion is normalized under the specified epsilon, G_FALSE otherwise.
		*/
		GBool IsNormalized(const DATA_TYPE Epsilon = G_EPSILON) {
			return GMath::IsEqual(this->LengthSquared(), DATA_TYPE(1), Epsilon);
		}

		//!	Quaternion complex conjugate.
		GQuat<DATA_TYPE> Conj() const {

			return GQuat<DATA_TYPE>(-gData[G_X], -gData[G_Y], -gData[G_Z], gData[G_W]);
		}

		/*!
			Quaternion multiplicative inverse.
			\result (conjugate / length)
		*/
		GQuat<DATA_TYPE> Invert() const {

			GQuat<DATA_TYPE> q(Conj());

			DATA_TYPE l = this->LengthSquared();
			if (l == 0)
				return(*this);
			
			q[G_X] /= l;
			q[G_Y] /= l;
			q[G_Z] /= l;
			q[G_W] /= l;
			return q;
		}

		/*!
			Complex exponentiation.
		*/
		GQuat<DATA_TYPE> Exp() const {

			DATA_TYPE len1, len2;

			len1 = GMath::Sqrt(gData[G_X] * gData[G_X] + gData[G_Y] * gData[G_Y] + gData[G_Z] * gData[G_Z]);
			if (len1 > (DATA_TYPE)0)
				len2 = GMath::Sin(len1) / len1;
			else
				len2 = (DATA_TYPE)1;

			return GQuat<DATA_TYPE>(gData[G_X] * len2, gData[G_Y] * len2, gData[G_Z] * len2, GMath::Cos(len1));
		}

		/*!
			Complex logarithm.
		*/
		GQuat<DATA_TYPE> Log() const {
			
			DATA_TYPE length;

			length = GMath::Sqrt(gData[G_X] * gData[G_X] + gData[G_Y] * gData[G_Y] + gData[G_Z] * gData[G_Z]);
			// avoid divide by 0
			if (GMath::IsEqual(gData[G_W], (DATA_TYPE)0, (DATA_TYPE)G_EPSILON) == G_FALSE)
				length = GMath::Atan(length / gData[G_W]);
			else
				length = (DATA_TYPE)G_PI_OVER2;

			return GQuat<DATA_TYPE>(gData[G_X] * length, gData[G_Y] * length, gData[G_Z] * length, (DATA_TYPE)0);
		}
	};

	//! GReal quaternion multiplication identity
	const GQuat<GReal> G_QUAT_MULT_IDENTITYF(0, 0, 0, 1);
	//! GReal quaternion addition identity
	const GQuat<GReal> G_QUAT_ADD_IDENTITYF(0, 0, 0, 0);
	//! Common quaternion class, it uses GReal data type.
	typedef GQuat<GReal> GQuaternion;


	/*!
		Quaternion dot product between two quaternions.
    */
	template <typename DATA_TYPE>
	DATA_TYPE Dot(const GQuat<DATA_TYPE>& Quat1, const GQuat<DATA_TYPE>& Quat2) {
		return DATA_TYPE((Quat1[0] * Quat2[0]) + (Quat1[1] * Quat2[1]) + (Quat1[2] * Quat2[2]) + (Quat1[3] * Quat2[3]));
	}

   
	/*!
		Spherical linear interpolation between two rotation quaternions (modified to include also extra spin)
	
		\param LerpPercentage a value between 0 and 1 that interpolates between from and to.
		\param From quaternion, corresponding to LerpPercentage = 0
		\param To quaternion, corresponding to LerpPercentage = 1
		\param SpinAmt extra spin expressed in radiants
		\param AdjustSign if G_TRUE, then slerp will operate by adjusting the sign of the slerp to take shortest path
		\result Interpolated quaternion
    */
	template <typename DATA_TYPE>
	GQuat<DATA_TYPE> Slerp(const DATA_TYPE LerpPercentage, const GQuat<DATA_TYPE>& From, const GQuat<DATA_TYPE>& To,
						   const DATA_TYPE SpinAmt, const GBool AdjustSign) {

		// calculate cosine theta
		DATA_TYPE cosalpha = Dot(From, To);
		DATA_TYPE flip;
		// adjust signs (if necessary)
		if (AdjustSign && (cosalpha < (DATA_TYPE)0)) {
			cosalpha = -cosalpha;
			flip = -1;
		}
		else
			flip = 1;

		// Calculate coefficients
		DATA_TYPE k1, k2;
		if (((DATA_TYPE)1 - GMath::Abs(cosalpha)) > (DATA_TYPE)G_EPSILON) {
			// Standard case (slerp)
			DATA_TYPE alpha = GMath::Acos(cosalpha); // extract theta from dot product's cos theta
			DATA_TYPE sinalpha = (DATA_TYPE)1 / GMath::Sin(alpha);
			DATA_TYPE anglespin = alpha + SpinAmt * (DATA_TYPE)G_PI;
			k1 = GMath::Sin(alpha - LerpPercentage * anglespin) * sinalpha;
			k2 = GMath::Sin(LerpPercentage * anglespin) * sinalpha;
		}
		else {
			// very close, do linear interpolation (because it's faster)
			k1 = (DATA_TYPE)1 - LerpPercentage;
			k2 = LerpPercentage;
		}

		k2 *= flip;
		return GQuat<DATA_TYPE>(k1 * From[G_X] + k2 * To[G_X],
								k1 * From[G_Y] + k2 * To[G_Y],
								k1 * From[G_Z] + k2 * To[G_Z],
								k1 * From[G_W] + k2 * To[G_W]);
	}

	/*!
		Spherical spline quaternion interpolation, between two rotation quaternions (modified to include also extra spin)
	
		\param LerpPercentage a value between 0 and 1 that interpolates between from and to.
		\param Quat1 quaternion, corresponding to LerpPercentage = 0
		\param Quat2 quaternion, corresponding to LerpPercentage = 1
		\param Tg1 quaternion, Bezier point corresponding to Quat1
		\param Tg2 quaternion, Bezier point corresponding to Quat2
		\param SpinAmt extra spin expressed in radiants
		\param AdjustSign if G_TRUE, then slerp will operate by adjusting the sign of the slerp to take shortest path
		\result Interpolated quaternion
    */
	template <typename DATA_TYPE>
	GQuat<DATA_TYPE> Squad(const DATA_TYPE LerpPercentage, const GQuat<DATA_TYPE>& Quat1, const GQuat<DATA_TYPE>& Quat2,
						   const GQuat<DATA_TYPE>& Tg1, const GQuat<DATA_TYPE>& Tg2,
						   const DATA_TYPE SpinAmt, const GBool AdjustSign) {

		GQuat<DATA_TYPE> qAux1, qAux2;

		qAux1 = Slerp(LerpPercentage, Quat1, Quat2, SpinAmt, AdjustSign);
		qAux2 = Slerp(LerpPercentage, Tg1, Tg2, SpinAmt, AdjustSign);
		return Slerp((DATA_TYPE)2 * LerpPercentage * ((DATA_TYPE)1 - LerpPercentage), qAux1, qAux2, SpinAmt, AdjustSign);
	}

	/*!
		Compare two quaternions for equality with tolerance.
    */
	template <typename DATA_TYPE>
	GBool IsEqual(const GQuat<DATA_TYPE>& Quat1, const GQuat<DATA_TYPE>& Quat2, const DATA_TYPE Epsilon = 0) {
		return GBool(GMath::IsEqual(Quat1[0], Quat2[0], Epsilon) && GMath::IsEqual(Quat1[1], Quat2[1], Epsilon) &&
					 GMath::IsEqual(Quat1[2], Quat2[2], Epsilon) && GMath::IsEqual(Quat1[3], Quat2[3], Epsilon));
	}

	/*!
		Compare two quaternions for geometric equivalence (with tolerance).
		There exist 2 quaternions for every possible rotation: the original, and its negative.
		The negative of a rotation quaternion is geometrically equivalent to the original.
    */
	template <typename DATA_TYPE>
	GBool IsEquivalent(const GQuat<DATA_TYPE>& Quat1, const GQuat<DATA_TYPE>& Quat2, const DATA_TYPE Epsilon = 0) {
		return GBool(IsEqual(Quat1, Quat2, Epsilon) || IsEqual(Quat1, -Quat2, Epsilon));
	}
};	// end namespace Amanith

#endif
