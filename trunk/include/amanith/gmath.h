/****************************************************************************
** $file: amanith/gmath.h   0.2.0.0   edited Dec, 12 2005
**
** Mathematics utilities and functions.
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

#ifndef GMATH_H
#define GMATH_H

#include "gglobal.h"
#include <cstdlib>
#include <cmath>
#include <ctime>


/*!
	\file gmath.h
	\brief Mathematics functions and utilities.
*/
namespace Amanith {

	/*!
		\namespace Amanith::GMath
		\brief Math Functions
	*/
	namespace GMath {
		//! 2 * PI constant
		#define G_2PI 6.28318530717958647692
		//! PI constant
		#define G_PI 3.14159265358979323846
		//! PI/2 constant
		#define G_PI_OVER2 1.57079632679489661923
		//! PI/4 constant
		#define G_PI_OVER4 0.78539816339744830962
		//! Sqrt(0.5) constant
		#define G_SQRTHALF 0.7071067811865475244
		//! Nepero number
		#define G_EXP1 2.71828182845904523536
		//! 1/PI constant
		#define G_ONE_OVER_PI 0.31830988618379067154
		//! 1/2PI constant
		#define G_ONE_OVER_2PI 0.159154943091895335768

		//! Absolute function (float specific version)
		inline GFloat Abs(const GFloat Value) {
			return GFloat(std::fabs(Value));
		}
		//! Absolute function (double specific version)
		inline GDouble Abs(const GDouble Value) {
			return GDouble(std::fabs(Value));
		}
		//! Absolute function (template for others types)
		template <typename T>
		inline T Abs(const T Value) {
			return T(Value >= ((T)0) ? Value : -Value);
		}

		/*!
			Ceil function can't be called for types other than float and double. This template prevents an incorrect use
			of ceiling function.
		*/
		template <typename T>
		inline T Ceil(T fValue);
		//! Calculates the ceiling of Value (float version, optimized for some platforms)
		inline GFloat Ceil(const GFloat Value) {
			#ifdef G_NO_CEILF
				return GFloat(std::ceil(Value));
			#else
				return GFloat(std::ceilf(Value));
			#endif
		}
		//! Calculates the ceiling of Value (double version)
		inline GDouble Ceil(const GDouble Value) {
			return GDouble(std::ceil(Value));
		}

		/*!
			Floor function can't be called for types other than float and double. This template prevents an incorrect use
			of floor function.
		*/
		template <typename T>
		inline T Floor(T Value);
		//! Calculates the floor of Value (float version, optimized for some platforms)
		inline GFloat Floor(const GFloat Value) {
			#ifdef G_NO_FLOORF
				return GFloat(std::floor(Value));
			#else
				return GFloat(std::floorf(Value));
			#endif
		}
		//! Calculates the floor of Value (double version)
		inline GDouble Floor(const GDouble fValue) {
			return GDouble(std::floor(fValue));
		}

		/*!
			Sign function template, it returns:\n
			- 0 if Value == 0
			- 1 if Value > 0
			- -1 if Value < 0

			\param Value an object who's type has operator> and operator< defined.
		*/
		template <typename T>
		inline GInt32 Sign(const T Value) {
			if (Value > T(0))
				return 1;
			else {
				if (Value < T(0))
					return -1;
				else
					return 0;
			}
		}

		/*!
			Clamps the given value down to zero if it is within epsilon of zero.
			\param Value the value to clamp
			\param Eps the epsilon tolerance or zero by default
			\return zero if the value is close to 0, the value otherwise
		*/
		template <typename T>
		inline T ZeroClamp(const T Value, const T Eps = T(0)) {
			return ((GMath::Abs(Value) <= Eps) ? T(0) : Value);
		}

		/*!
			Compute the factorial.

			\param Value an object who's type has operator++, operator=, operator<=, and operator*= defined.
			It should be a single valued scalar type such as an int, float, double etc....
			\return The factorial of Value.
		*/
		template<typename DATA_TYPE>
		inline DATA_TYPE Factorial(const DATA_TYPE Value) {
			DATA_TYPE lhs = (DATA_TYPE)1;
			for (DATA_TYPE x = (DATA_TYPE)2; x <= Value; ++x)
				lhs *= x;
			return lhs;
		}

		/*!
			Compute the binomial coefficient "n over i".

			\param N an object who's type has operator++, operator=, operator<=, and operator*= defined.
			It should be a single valued scalar type such as an int, float, double etc....
			\param I an object who's type has operator++, operator=, operator<=, and operator*= defined.
			It should be a single valued scalar type such as an int, float, double etc....
		*/
		template<typename DATA_TYPE>
		inline DATA_TYPE Binomial(DATA_TYPE N, DATA_TYPE I) {

			if (I > N)
				return (DATA_TYPE)0;
			// optimization that keep loop at minimum cycles. it exploits the fact that
			// "n over i" = "n over (n-i)"
			if (I > (N / (DATA_TYPE)2))
				I = N - I;
			if (I == 0)
				return (DATA_TYPE)1;
			DATA_TYPE num = N - I + (DATA_TYPE)1;
			DATA_TYPE den = Factorial(I);
			for (DATA_TYPE x = num + 1; x <= N; ++x)
				num *= x;
			return num / den;
		}

		//----------------------------------------------------------------------------
		/*!
			Acos function can't be called for types other than float and double. This template prevents an incorrect use
			of Acos function.
		*/
		template <typename T>
		inline T Acos(T Value);
		/*!
			Calculates the arccosine (float version).

			\param Value the value that we wanna calculate the arccosine of
			\return The arccosine of passed value.
			\note If Value is lesser than -1, this function returns G_PI constant.\n
			If Value is greater than 1, this function returns 0.
		*/
		inline GFloat Acos(const GFloat Value) {

			if (-1.0f < Value) {
				if (Value < 1.0f) {
					#ifdef G_NO_ACOSF
						return GFloat(std::acos(Value));
					#else
						return GFloat(std::acosf(Value));
					#endif
				}
				else
					return 0.0f;
			}
			else
				return (GFloat)G_PI;
		}
		/*!
			Calculates the arccosine (double version).

			\param Value the value that we wanna calculate the arccosine of
			\return The arccosine of passed value.
			\note If Value is lesser than -1, this function returns G_PI constant.\n
			If Value is greater than 1, this function returns 0.
		*/
		inline GDouble Acos(const GDouble Value) {

			if (-1.0 < Value) {
				if (Value < 1.0)
					return GDouble(std::acos(Value));
				else
					return 0.0;
			}
			else
				return (GDouble)G_PI;
		}
		/*!
			Asin function can't be called for types other than float and double. This template prevents an incorrect use
			of Asin function.
		*/
		template <typename T>
		inline T Asin(T Value);
		/*!
			Calculates the arcsine (float version).

			\param Value the value that we wanna calculate the arcsine of
			\return The arcsine of passed value.
			\note If Value is lesser than -1, this function returns G_PI_OVER2 constant.\n
			If Value is greater than 1, this function returns -G_PI_OVER2 constant.
		*/
		inline GFloat Asin(const GFloat Value) {

			if (-1.0f < Value) {
				if (Value < 1.0f) {
					#ifdef G_NO_ASINF
						return GFloat(std::asin(Value));
					#else
						return GFloat(std::asinf(Value));
					#endif
				}
				else
					return (GFloat)-G_PI_OVER2;
			}
			else
				return (GFloat)G_PI_OVER2;
		}
		/*!
			Calculates the arcsine (double version).

			\param Value the value that we wanna calculate the arcsine of
			\return The arcsine of passed value.
			\note If Value is lesser than -1, this function returns G_PI_OVER2 constant.\n
			If Value is greater than 1, this function returns -G_PI_OVER2 constant.
		*/
		inline GDouble Asin(const GDouble Value) {

			if (-1.0 < Value) {
				if (Value < 1.0)
					return GDouble(std::asin(Value));
				else
					return (GDouble)-G_PI_OVER2;
			}
			else
				return (GDouble)G_PI_OVER2;
		}

		/*!
			Atan function can't be called for types other than float and double. This template prevents an incorrect use
			of Atan function.
		*/
		template <typename T>
		inline T Atan(T Value);
		/*!
			Calculates the arctangent (double version).

			\param Value the value that we wanna calculate the arctangent of
			\return The arctangent of passed value, in the range [-PI/2, PI/2] radians.
			\note If Value is 0, this function returns 0.
		*/
		inline GDouble Atan(const GDouble Value) {
			return std::atan(Value);
		}
		/*!
			Calculates the arctangent (float version, optimized for some platforms).

			\param Value the value that we wanna calculate the arctangent of
			\return The arctangent of passed value, in the range [-PI/2, PI/2] radians.
			\note If Value is 0, this function returns 0.
		*/
		inline GFloat Atan(const GFloat Value) {
			#ifdef G_NO_TANF
				return GFloat(std::atan(Value));
			#else
				return GFloat(std::atanf(Value));
			#endif
		}

		/*!
			Atan2 function can't be called for types other than float and double. This template prevents an incorrect use
			of Atan2 function.
		*/
		template <typename T>
		inline T Atan2(const T Y, const T X);

		/*!
			Calculates the arctangent of Y/X (float version, optimized for some platforms).

			\return the arctangent of Y/X, in the range [_PI; PI] radians.
			\note If both Y and X are 0, this function returns 0.\n
			This function uses the signs of both parameters to determine the quadrant of the return value
		*/
		inline GFloat Atan2(const GFloat Y, const GFloat X) {
			#ifdef G_NO_ATAN2F
				return GFloat(std::atan2(Y, X));
			#else
				return GFloat(std::atan2f(Y, X));
			#endif
		}
		/*!
			Calculates the arctangent of Y/X (double version).

			\return the arctangent of Y/X, in the range [_PI; PI] radians.
			\note If both Y and X are 0, this function returns 0.\n
			This function uses the signs of both parameters to determine the quadrant of the return value
		*/
		inline GDouble Atan2(const GDouble Y, const GDouble X) {
			return GDouble(std::atan2(Y, X));
		}

		/*!
			Cos function can't be called for types other than float and double. This template prevents an incorrect use
			of Cos function.
		*/
		template <typename T>
		inline T Cos(const T Value);
		/*!
			Calculates the cosine of Value (float version, optimized for some platforms).

			\param Value angle in radians
		*/
		inline GFloat Cos(const GFloat Value) {
			#ifdef G_NO_COSF
				return GFloat(std::cos(Value));
			#else
				return GFloat(std::cosf(Value));
			#endif
		}
		/*!
			Calculates the cosine of Value (double version).

			\param Value angle in radians
		*/
		inline GDouble Cos(const GDouble Value) {
			return GDouble(std::cos(Value));
		}

		/*!
			Exp function can't be called for types other than float and double. This template prevents an incorrect use
			of Exp function.
		*/
		template <typename T>
		inline T Exp(const T Value);
		/*!
			Calculates the exponential of Value (float version, optimized for some platforms).
		*/
		inline GFloat Exp(const GFloat Value) {
			#ifdef G_NO_EXPF
				return GFloat(std::exp(Value));
			#else
				return GFloat(std::expf(Value));
			#endif
		}
		/*!
			Calculates the exponential of Value (double version).
		*/
		inline GDouble Exp(const GDouble Value) {
			return GDouble(std::exp(Value));
		}

		/*!
			Log function can't be called for types other than float and double. This template prevents an incorrect use
			of Log function.
		*/
		template <typename T>
		inline T Log(const T Value);
		/*!
			Calculates the natural logarithm of Value (double version).

			\note If Value is negative, this function returns an indefinite result.\n
			If Value is 0, this function returns infinite.
		*/
		inline GDouble Log(const GDouble Value) {
			return GDouble(std::log(Value));
		}
		/*!
			Calculates the natural logarithm of Value (float version, optimized for some platforms).

			\note If Value is negative, this function returns an indefinite result.\n
			If Value is 0, this function returns infinite.
		*/
		inline GFloat Log(const GFloat Value) {
			#ifdef G_NO_LOGF
				return GFloat(std::log(Value));
			#else
				return GFloat(std::logf(Value));
			#endif
		}

		/*!
			Log2 function can't be called for types other than float and double. This template prevents an incorrect use
			of Log2 function.
		*/
		template <typename T>
		inline T Log2(const T Value);
		/*!
			Calculates the logarithm to base 2 of Value (double version).

			\note If Value is negative, this function returns an indefinite result.\n
			If Value is 0, this function returns infinite.
		*/
		inline GDouble Log2(const GDouble Value) {
			return (Log(Value) / Log(2.0));
		}
		/*!
			Calculates the logarithm to base 2 of Value (float version, optimized for some platforms).

			\note If Value is negative, this function returns an indefinite result.\n
			If Value is 0, this function returns infinite.
		*/
		inline GFloat Log2(const GFloat Value) {
			return (Log(Value) / Log(2.0f));
		}

		/*!
			Log10 function can't be called for types other than float and double. This template prevents an incorrect use
			of Log10 function.
		*/
		template <typename T>
		inline T Log10(const T Value);
		/*!
			Calculates the logarithm to base 10 of Value (double version).

			\note If Value is negative, this function returns an indefinite result.\n
			If Value is 0, this function returns infinite.
		*/
		inline GDouble Log10(const GDouble Value) {
			return GDouble(std::log10(Value));
		}
		/*!
			Calculates the logarithm to base 10 of Value (float version, optimized for some platforms).

			\note If Value is negative, this function returns an indefinite result.\n
			If Value is 0, this function returns infinite.
		*/
		inline GFloat Log10(const GFloat Value) {
			#ifdef G_NO_LOGF
				return GFloat(std::log10(Value));
			#else
				return GFloat(std::log10f(Value));
			#endif
		}

		/*!
			Calculates the natural log* of Value.
		*/
		inline GUInt32 LogStar(const GUInt32 Value) {

			GUInt32 i;
			GDouble v;

  			for (i = 0, v = (GDouble)Value; v >= 1; ++i)
				v = Log(v);
			return (i - 1);
		}

		/*!
			Calculates the log* to base 2 of Value.
		*/
		inline GUInt32 Log2Star(const GUInt32 Value) {

			GUInt32 i;
			GDouble v;

  			for (i = 0, v = (GDouble)Value; v >= 1; ++i)
				v = Log2(v);
			return (i - 1);
		}

		/*!
			Calculates the log* to base 10 of Value.
		*/
		inline GUInt32 Log10Star(const GUInt32 Value) {

			GUInt32 i;
			GDouble v;

  			for (i = 0, v = (GDouble)Value; v >= 1; ++i)
				v = Log10(v);
			return (i - 1);
		}

		/*!
			Pow function can't be called for types other than float and double. This template prevents an incorrect use
			of Pow function.
		*/
		template <typename T>
		inline T Pow(const T Base, const T Exponent);
		/*!
			Calculates Base raised to the power of Exponent (double version).
		*/
		inline GDouble Pow(const GDouble Base, const GDouble Exponent) {
			return GDouble(std::pow(Base, Exponent));
		}
		/*!
			Calculates Base raised to the power of Exponent (float version, optimized for some platforms).
		*/
		inline GFloat Pow(const GFloat Base, const GFloat Exponent) {
			#ifdef G_NO_POWF
				return GFloat(std::pow(Base, Exponent));
			#else
				return GFloat(std::powf(Base, Exponent));
			#endif
		}

		/*!
			Sin function can't be called for types other than float and double. This template prevents an incorrect use
			of Sin function.
		*/
		template <typename T>
		inline T Sin(const T Value);
		/*!
			Calculates the sine of Value (double version).

			\param Value angle in radians
		*/
		inline GDouble Sin(const GDouble Value) {
			return GDouble(std::sin(Value));
		}
		/*!
			Calculates the sine of Value (float version, optimized for some platforms).

			\param Value angle in radians
		*/
		inline GFloat Sin(const GFloat Value) {
			#ifdef G_NO_SINF
				return GFloat(std::sin(Value));
			#else
				return GFloat(std::sinf(Value));
			#endif
		}

		/*!
			Tan function can't be called for types other than float and double. This template prevents an incorrect use
			of Tan function.
		*/
		template <typename T>
		inline T Tan(const T Value);
		/*!
			Calculates the tangent of Value (double version).

			\param Value angle in radians
		*/
		inline GDouble Tan(const GDouble Value) {
			return GDouble(std::tan(Value));
		}
		/*!
			Calculates the sine of Value (float version, optimized for some platforms).

			\param Value angle in radians
		*/
		inline GFloat Tan(const GFloat Value) {
			#ifdef G_NO_TANF
				return GFloat(std::tan(Value));
			#else
				return GFloat(std::tanf(Value));
			#endif
		}
		
		//! template function to calculate the square of Value.
		template <typename T>
		inline T Sqr(const T Value) {
			return T(Value * Value);
		}

		/*!
			Sqrt function can't be called for types other than float and double. This template prevents an incorrect use
			of Sqrt function.
		*/
		template <typename T>
		inline T Sqrt(const T Value);
		/*!
			Calculates the square root of Value (double version).

			\note Value parameter must be nonnegative.\n
			If Value is negative this function returns an indefinite result.
		*/
		inline GDouble Sqrt(const GDouble Value) {
			return GDouble(std::sqrt(Value));
		}
		/*!
			Calculates the square root of Value (float version, optimized for some platforms).

			\note Value parameter must be nonnegative.\n
			If Value is negative this function returns an indefinite result.
		*/
		inline GFloat Sqrt(const GFloat Value) {
			#ifdef G_NO_SQRTF
				return GFloat(std::sqrt(Value));
			#else
				return GFloat(std::sqrtf(Value));
			#endif
		}

		/*!
			Return Bessel function of the first kind, order 0 (double version)

			\note Value must be nonnegative for correct results
		*/
		inline GDouble BesselJ0(const GDouble Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return ::_j0(Value);
			#else
				return ::j0(Value);
			#endif
		}
		/*!
			Return Bessel function of the first kind, order 0 (float version, provided for convenience)

			\note Value must be nonnegative for correct results
		*/
		inline GFloat BesselJ0(const GFloat Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return (GFloat)::_j0((GDouble)Value);
			#else
				return (GFloat)::j0((GDouble)Value);
			#endif
		}
		/*!
			Return Bessel function of the first kind, order 1 (double version)

			\note Value must be nonnegative for correct results
		*/
		inline GDouble BesselJ1(const GDouble Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return ::_j1(Value);
			#else
				return ::j1(Value);
			#endif
		}
		/*!
			Return Bessel function of the first kind, order 1 (float version, provided for convenience)

			\note Value must be nonnegative for correct results
		*/
		inline GFloat BesselJ1(const GFloat Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return (GFloat)::_j1((GDouble)Value);
			#else
				return (GFloat)::j1((GDouble)Value);
			#endif
		}
		/*!
			Return Bessel function of the first kind, specifying order (double version)

			\note Value must be nonnegative for correct results
		*/
		inline GDouble BesselJN(const GInt32 Order, const GDouble Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return ::_jn(Order, Value);
			#else
				return ::jn(Order, Value);
			#endif
		}
		/*!
			Return Bessel function of the first kind, specifying order (float version, provided for convenience)

			\note Value must be nonnegative for correct results
		*/
		inline GFloat BesselJN(const GInt32 Order, const GFloat Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return (GFloat)::_jn(Order, (GDouble)Value);
			#else
				return (GFloat)::jn(Order, (GDouble)Value);
			#endif
		}

		/*!
			Return Bessel function of the second kind, order 0 (double version)

			\note Value must be nonnegative for correct results
		*/
		inline GDouble BesselY0(const GDouble Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return ::_y0(Value);
			#else
				return ::y0(Value);
			#endif
		}
		/*!
			Return Bessel function of the second kind, order 0 (float version, provided for convenience)

			\note Value must be nonnegative for correct results
		*/
		inline GFloat BesselY0(const GFloat Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return (GFloat)::_y0((GDouble)Value);
			#else
				return (GFloat)::y0((GDouble)Value);
			#endif
		}
		/*!
			Return Bessel function of the second kind, order 1 (double version)

			\note Value must be nonnegative for correct results
		*/
		inline GDouble BesselY1(const GDouble Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return ::_y1(Value);
			#else
				return ::y1(Value);
			#endif
		}
		/*!
			Return Bessel function of the second kind, order 1 (float version, provided for convenience)

			\note Value must be nonnegative for correct results
		*/
		inline GFloat BesselY1(const GFloat Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return (GFloat)::_y1((GDouble)Value);
			#else
				return (GFloat)::y1((GDouble)Value);
			#endif
		}
		/*!
			Return Bessel function of the second kind, specifying order (double version)

			\note Value must be nonnegative for correct results
		*/
		inline GDouble BesselYN(const GInt32 Order, const GDouble Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return ::_yn(Order, Value);
			#else
				return ::yn(Order, Value);
			#endif
		}
		/*!
			Return Bessel function of the second kind, specifying order (float version, provided for convenience)

			\note Value must be nonnegative for correct results
		*/
		inline GFloat BesselYN(const GInt32 Order, const GFloat Value) {
			#if defined(G_OS_WIN) && _MSC_VER >= 1400
				return (GFloat)::_yn(Order, (GDouble)Value);
			#else
				return (GFloat)::yn(Order, (GDouble)Value);
			#endif
		}


		/*!
			Return the modified Bessel function of the first kind, order 0 (double version)

			\note If Value is negative, then its absolute value will be used during calculation.\n
			This function is taken from Numerical Recipes in C.
		*/
		inline GDouble BesselI0(const GDouble Value) {

			// taken from Numerical Recipes in C
			GDouble ax = Abs(Value), y, ans;
			
			if (ax < 3.75) {
				// Polynomial fit
				y = ax / 3.75;
				y *= y;
				ans = 1.0 + y * (3.5156229 + y * (3.0899424 + y * (1.2067492 + y * (0.2659732 + 
								y * (0.360768e-1 + y * 0.45813e-2)))));
			} else {
				y = 3.75 / ax;
				ans = (Exp(ax) / Sqrt(ax)) * (0.39894228 + y * (0.1328592e-1 + 
						y * (0.225319e-2 + y * (-0.157565e-2 + y * (0.916281e-2
						+ y * (-0.2057706e-1 + y * (0.2635537e-1 +
						y * (-0.1647633e-1 + y * 0.392377e-2))))))));
			}
			return ans;
		}
		/*!
			Return the modified Bessel function of the first kind, order 0 (float version, provided for convenience)

			\note If Value is negative, then its absolute value will be used during calculation.\n
			This function is taken from Numerical Recipes in C.
		*/
		inline GFloat BesselI0(const GFloat Value) {

			// taken from Numerical Recipes in C
			GFloat ax = Abs(Value), y, ans;

			if (ax < 3.75f) {
				// Polynomial fit
				y = ax / 3.75f;
				y *= y;
				ans = 1.0f + y * (3.5156229f + y * (3.0899424f + y * (1.2067492f + y * (0.2659732f + 
					y * (0.360768e-1f + y * 0.45813e-2f)))));
			} else {
				y = 3.75f / ax;
				ans = (Exp(ax) / Sqrt(ax)) * (0.39894228f + y * (0.1328592e-1f + 
					y * (0.225319e-2f + y * (-0.157565e-2f + y * (0.916281e-2f
					+ y * (-0.2057706e-1f + y * (0.2635537e-1f +
					y * (-0.1647633e-1f + y * 0.392377e-2f))))))));
			}
			return ans;
		}

		/*!
			Return the modified Bessel function of the first kind, order 1 (double version)

			\note If Value is negative, then its absolute value will be used during calculation.\n
			This function is taken from Numerical Recipes in C.
		*/
		inline GDouble BesslI1(const GDouble Value) {
			
			// taken from Numerical Recipes in C
			GDouble ax = Abs(Value), y, ans;

			if (ax < 3.75) {
				// Polynomial fit
				y = ax / 3.75;
				y *= y;
				ans = ax * (0.5 + y * (0.87890594 + y * (0.51498869 + y * (0.15084934
						+ y * (0.2658733e-1 + y * (0.301532e-2 + y * 0.32411e-3))))));
			}
			else {
				y = 3.75 / ax;
				ans = 0.2282967e-1 + y * (-0.2895312e-1+y*(0.1787654e-1 - y * 0.420059e-2));
				ans = 0.39894228 + y * (-0.3988024e-1 + y * (-0.362018e-2
						+ y * (0.163801e-2 + y * (-0.1031555e-1 + y * ans))));
				ans *= (Exp(ax) / Sqrt(ax));
			}
			return ans;
		}
		/*!
			Return the modified Bessel function of the first kind, order 1 (float version, provided for convenience)

			\note If Value is negative, then its absolute value will be used during calculation.\n
			This function is taken from Numerical Recipes in C.
		*/
		inline GFloat BesslI1(const GFloat Value) {

			// taken from Numerical Recipes in C
			GFloat ax = Abs(Value), y, ans;

			if (ax < 3.75f) {
				// Polynomial fit
				y = ax / 3.75f;
				y *= y;
				ans = ax * (0.5f + y * (0.87890594f + y * (0.51498869f + y * (0.15084934f
					+ y * (0.2658733e-1f + y * (0.301532e-2f + y * 0.32411e-3f))))));
			}
			else {
				y = 3.75f / ax;
				ans = 0.2282967e-1f + y * (-0.2895312e-1f + y * (0.1787654e-1f - y * 0.420059e-2f));
				ans = 0.39894228f + y * (-0.3988024e-1f + y * (-0.362018e-2f
					+ y * (0.163801e-2f + y * (-0.1031555e-1f + y * ans))));
				ans *= (Exp(ax) / Sqrt(ax));
			}
			return ans;
		}

		/*
			Calculate hypotenuse of double scalars A and B (double version)

			\note This function is very stable by avoiding underflow/overflow. It uses (A * sqrt(1 + (B/A) * (B/A)))
			rather than	sqrt(A * A + B * B).
		*/
		inline GDouble Hypot(const GDouble A, const GDouble B) {
			if (A == 0)
				return Abs(B);
			else {
				GDouble c = B / A;
				return Abs(A) * Sqrt(1.0 + c * c);
			}
		}
		/*
			Calculate hypotenuse of double scalars A and B (float version)

			\note This function is very stable by avoiding underflow/overflow. It uses (A * sqrt(1 + (B/A) * (B/A)))
			rather than	sqrt(A * A + B * B).
		*/
		inline GFloat Hypot(const GFloat A, const GFloat B) {
			if (A == 0)
				return Abs(B);
			else {
				GFloat c = B / A;
				return Abs(A) * Sqrt(1.0f + c * c);
			}
		}

		template <typename T>
		static void ComplexDiv(const T xr, const T xi, const T yr, const T yi, T& cdivr, T& cdivi) {

			T r, d;

			if (Abs(yr) > Abs(yi)) {
				r = yi / yr;
				d = yr + r * yi;
				cdivr = (xr + r * xi) / d;
				cdivi = (xi - r * xr) / d;
			}
			else {
				r = yr / yi;
				d = yi + r * yr;
				cdivr = (r * xr + xi) / d;
				cdivi = (r * xi - xr) / d;
			}
		}

		/*!
			Finds greatest common divider of N1 and N2 using Euclid's algorithm.
		*/
		static GInt32 GCD(GInt32 N1, GInt32 N2) {
			
			GInt32 mod;

			if ((N1 == 0) || (N2 == 0))
				return 1;

			while (N2 != 0) {
				mod = N1 % N2;
				N1 = N2;
				N2 = mod;
			}
			return N1;
		}

		/*!
			Finds the minimal common multiple of N! and N2.
		*/
		inline GInt32 MCM(GInt32 N1, GInt32 N2) {
			return (N1 * N2) / GCD(N1, N2);
		}


		//----------------------------------------------------------------------------
		/*!
			Seeds the pseudo-random number generator with the given seed.

			\param Seed the seed for the pseudo-random number generator.
		*/
		inline void SeedRandom(const GUInt32 Seed) {
			std::srand(Seed);
		}

		/*!
			Seeds the pseudo-random number generator with the clock-time random seed.
		*/
		inline void SeedRandom() {
			time_t ltime;
			time(&ltime);
			std::srand((unsigned int)ltime);
		}

		/*!
			Get a random number between 0 and 1 (float version)
		*/
		inline GFloat UnitRandomf() {
			return ((GFloat)(std::rand()) / (GFloat)(RAND_MAX));
		}

		/*!
			Get a random number between 0 and 1 (double version)
		*/
		inline GDouble UnitRandom() {
			return ((GDouble)(std::rand()) / (GDouble)(RAND_MAX));
		}

		/*!
			Return a random number between X1 and X2 (float version)
		*/
		inline GFloat RangeRandom(const GFloat X1, const GFloat X2) {

			return (UnitRandomf() * (X2 - X1) + X1);
		}
		/*!
			Return a random number between X1 and X2 (double version)
		*/
		inline GDouble RangeRandom(const GDouble X1, const GDouble X2) {

			return (UnitRandom() * (X2 - X1) + X1);
		}

		/*!
			Return a random number between X1 and X2 (int version)
		*/
		inline GUInt32 RangeRandom(const GInt32 X1, const GInt32 X2) {
			return ((std::rand() % (X2 - X1 + 1)) + X1);
		}

		/*!
			Degree to radian conversion (float version).

			\param DegValue value to convert, expressed in degrees.
			\return Radian representation of DegValue parameter.
		*/
		inline GFloat Deg2Rad(const GFloat DegValue) {

			return GFloat(DegValue * (GFloat)(G_PI / 180.0));
		}
		/*!
			Degree to radian conversion (double version).

			\param DegValue value to convert, expressed in degrees.
			\return Radian representation of DegValue parameter.
		*/
		inline GDouble Deg2Rad(const GDouble DegValue) {

			return GDouble(DegValue * (GDouble)(G_PI / 180.0));
		}

		/*!
			Radians to degrees conversion (float version).

			\param RadValue value to convert, expressed in radians.
			\return Degree representation of RadValue parameter.
		*/
		inline GFloat Rad2Deg(const GFloat RadValue) {

			return GFloat(RadValue * (GFloat)(180.0 / G_PI));
		}
		/*!
			Radians to degrees conversion (double version).

			\param RadValue value to convert, expressed in radians.
			\return Degree representation of RadValue parameter.
		*/
		inline GDouble Rad2Deg(const GDouble RadValue) {

			return GDouble(RadValue * (GDouble)(180.0 / G_PI));
		}

		/*!
			Converts an angle from one measurement system into another system.

			\param InputValue the angle to convert.
			\param InputSystem the system units in which InputValue is expressed.
			\param OutputSystem the system units into which to convert InputValue.
			\return the converted angle.
		*/
		inline GFloat AngleConversion(const GFloat InputValue, const GAnglesMeasureUnit InputSystem, const GAnglesMeasureUnit OutputSystem) {

			GFloat tmpValue(InputValue);

			// transform into circle measure unit
			switch (InputSystem) {

				case G_DEGREE_UNIT:
					tmpValue *= 0.0027777777777778f;
					break;

				case G_GRAD_UNIT:
					tmpValue *= 0.0025f;
					break;

				case G_MIL_UNIT:
					tmpValue *= 0.00015625f;
					break;

				case G_MINUTE_UNIT:
					tmpValue *= 4.6296296296296e-005f;
					break;

				case G_OCTANT_UNIT:
					tmpValue *= 0.125f;
					break;

				case G_QUADRANT_UNIT:
				case G_RIGHTANGLE_UNIT:
					tmpValue *= 0.25f;
					break;

				case G_RADIAN_UNIT:
					tmpValue *= 0.1591549430919f;
					break;

				case G_SECOND_UNIT:
					tmpValue *= 7.716049382716e-007f;
					break;

				case G_SEXTANT_UNIT:
					tmpValue *= 0.16666666666667f;
					break;

				case G_SIGN_UNIT:
					tmpValue *= 0.083333333333333f;
					break;

				case G_CIRCLE_UNIT:
				case G_TURN_UNIT:
				case G_REVOLUTION_UNIT:
					break;
			}

			// transform into target measure unit
			switch (OutputSystem) {

				case G_DEGREE_UNIT:
					return (tmpValue * 360.0f);
					break;

				case G_GRAD_UNIT:
					return (tmpValue * 400.0f);
					break;

				case G_MIL_UNIT:
					return (tmpValue * 6400.0f);
					break;

				case G_MINUTE_UNIT:
					return (tmpValue * 21600.0f);
					break;

				case G_OCTANT_UNIT:
					return (tmpValue * 8.0f);
					break;

				case G_QUADRANT_UNIT:
				case G_RIGHTANGLE_UNIT:
					return (tmpValue * 4.0f);
					break;

				case G_RADIAN_UNIT:
					return (tmpValue * 6.28318530717958f);
					break;

				case G_SECOND_UNIT:
					return (tmpValue * 1296000.0f);
					break;

				case G_SEXTANT_UNIT:
					return (tmpValue * 6.0f);
					break;

				case G_SIGN_UNIT:
					return (tmpValue * 12.0f);
					break;

				case G_CIRCLE_UNIT:
				case G_TURN_UNIT:
				case G_REVOLUTION_UNIT:
					return tmpValue;
					break;

				default:
					return tmpValue;
			}
			return tmpValue;
		}

		/*!
			Converts an angle from one measurement system into another system.

			\param InputValue the angle to convert.
			\param InputSystem the system units in which InputValue is expressed.
			\param OutputSystem the system units into which to convert InputValue.
			\return the converted angle.
		*/
		inline GDouble AngleConversion(const GDouble InputValue, const GAnglesMeasureUnit InputSystem, const GAnglesMeasureUnit OutputSystem) {

			GDouble tmpValue(InputValue);

			// transform into circle measure unit
			switch (InputSystem) {

				case G_DEGREE_UNIT:
					tmpValue *= 0.0027777777777778;
					break;

				case G_GRAD_UNIT:
					tmpValue *= 0.0025;
					break;

				case G_MIL_UNIT:
					tmpValue *= 0.00015625;
					break;

				case G_MINUTE_UNIT:
					tmpValue *= 4.6296296296296e-005;
					break;

				case G_OCTANT_UNIT:
					tmpValue *= 0.125;
					break;

				case G_QUADRANT_UNIT:
				case G_RIGHTANGLE_UNIT:
					tmpValue *= 0.25;
					break;

				case G_RADIAN_UNIT:
					tmpValue *= 0.1591549430919;
					break;

				case G_SECOND_UNIT:
					tmpValue *= 7.716049382716e-007;
					break;

				case G_SEXTANT_UNIT:
					tmpValue *= 0.16666666666667;
					break;

				case G_SIGN_UNIT:
					tmpValue *= 0.083333333333333;
					break;

				case G_CIRCLE_UNIT:
				case G_TURN_UNIT:
				case G_REVOLUTION_UNIT:
					break;
			}

			// transform into target measure unit
			switch (OutputSystem) {

				case G_DEGREE_UNIT:
					return (tmpValue * 360.0);
					break;

				case G_GRAD_UNIT:
					return (tmpValue * 400.0);
					break;

				case G_MIL_UNIT:
					return (tmpValue * 6400.0);
					break;

				case G_MINUTE_UNIT:
					return (tmpValue * 21600.0);
					break;

				case G_OCTANT_UNIT:
					return (tmpValue * 8.0);
					break;

				case G_QUADRANT_UNIT:
				case G_RIGHTANGLE_UNIT:
					return (tmpValue * 4.0);
					break;

				case G_RADIAN_UNIT:
					return (tmpValue * 6.28318530717958);
					break;

				case G_SECOND_UNIT:
					return (tmpValue * 1296000.0);
					break;

				case G_SEXTANT_UNIT:
					return (tmpValue * 6.0);
					break;

				case G_SIGN_UNIT:
					return (tmpValue * 12.0);
					break;

				case G_CIRCLE_UNIT:
				case G_TURN_UNIT:
				case G_REVOLUTION_UNIT:
					return tmpValue;
					break;

				default:
					return tmpValue;
			}
			return tmpValue;
		}

		/*!
			Test for equality within some Tolerance.

			\param A first object operand to be tested for equality
			\param B second object operand to be tested for equality
			\param Tolerance the tolerance used in the comparison
			\return G_TRUE if the operands are equal under the specified tolerance, G_FALSE otherwise
			\pre tolerance must be >= 0
		*/
		template<typename T>
		inline GBool IsEqual(const T A, const T B, const T Tolerance = T(0)) {
			G_ASSERT(Tolerance >= T(0));
			return GBool(GMath::Abs(A - B) <= Tolerance);
		}

		/*!
			Cut off the digits after the decimal place.
		*/
		template <typename T>
		inline T Trunc(const T Value) {
			return T((Value < ((T)0)) ? Ceil(Value) : Floor(Value));
		}

		/*!
			Round to nearest integer.
		*/
		template <typename T>
		inline T Round(const T Value) {
			return T(GMath::Floor(Value + (T)0.5));
		}

		/*!
			Returns the largest integer <= (A / B), (this works correctly for negatives A and negatives B)
		*/
		inline GInt32 FloorDiv(const GInt32 A, const GInt32 B) {

			if (B < 0) {
				if (A < 0)
					return (-A / -B);
				else
					return ((1 - A) / (-B) - 1);
			}
			else {
				if (A >= 0)
					return (A / B);
				else
					return ((1 + A) / B - 1);
			}
		}


		/*!
			Returns the minimum of 2 values
		*/
		template <typename T>
		inline T Min(const T X, const T Y) {

			return (X > Y) ? Y : X;
		}

		/*!
			Returns the minimum of 3 values
		*/
		template <typename T>
		inline T Min(const T X, const T Y, const T Z) {

			return Min(Min(X, Y), Z);
		}
		/*!
			Returns the minimum of 4 values
		*/
		template <typename T>
		inline T Min(const T X, const T Y, const T Z, const T W) {

			return GMath::Min(Min(X, Y), Min(Z, W));
		}
		/*!
			Returns the maximum of 2 values
		*/
		template <typename T>
		inline T Max(const T X, const T Y) {

			return (X > Y) ? X : Y;
		}
		/*!
			Returns the maximum of 3 values
		*/
		template <typename T>
		inline T Max(const T X, const T Y, const T Z) {

			return Max(Max(X, Y), Z);
		}
		/*!
			Returns the maximum of 4 values
		*/
		template <typename T>
		inline T Max(const T X, const T Y, const T Z, const T W) {

			return GMath::Max(Max(X, Y), Max(Z, W));
		}
		/*!
			Clamp Number to a range between Lo and Hi.

			\param Number the number to clamp
			\param Lo the lower bound of the clamping range
			\param Hi the upper bound of the clamping range
			\return The clamped number.
		*/
		template <typename T>
		inline T Clamp(const T Number, const T Lo, const T Hi) {

			if (Number > Hi)
				return Hi;
			else
			if (Number < Lo)
				return Lo;
			return Number;
		}


		/*!
			Linear interpolation between objects A and B.

			\pre LerpPercentage must be in the range [0; 1]
			\note The implementation is very stable numerically
		*/
		template <typename T, typename U>
		inline T Lerp(const U& LerpPercentage, const T& A, const T& B) {

			G_ASSERT(LerpPercentage >= U(0) && LerpPercentage <= U(1));

			if (LerpPercentage <= U(0.5))
				return (A + (B - A) * LerpPercentage);
			else
				return (B + (A - B) * (U(1) - LerpPercentage));
		}

		/*!
			Barycentric convex interpolation between objects A and B. Weights must be positive.

			\param WeightA the weight assigned to object A
			\param A the first object operand
			\param WeightB the weight assigned to object B
			\param B the second object operand
			\result The barycentric sum, (WeightA * A + WeightB * B) / (WeightA + WeightB)
			\note The implementation is very stable numerically.
		*/
		template <typename T, typename U>
		inline T BarycentricConvexSum(const U& WeightA, const T& A, const U& WeightB, const T& B) {

			G_ASSERT(WeightA >= U(0));
			G_ASSERT(WeightB >= U(0));

			if (WeightA < WeightB)
				return A + (B - A) * (WeightA / (WeightA + WeightB));
			else
				return B + (A - B) * (WeightB / (WeightA + WeightB));
		}

		/*!
			Quadratic formula used to compute the 2 roots of the given 2nd degree polynomial in the form of Ax^2 + Bx + C.
			Here's a numerical stable implementation (modified from the one proposed in	Geometric Tools for
			Computer Graphics), avoiding underflow/overflow.

			\param R1 set to the first root
			\param R2 set to the second root
			\param A the coefficient to x^2
			\param B the coefficient to x^1
			\param C the coefficient to x^0
			\param Epsilon the precision used in tests for 0 made on equation coefficients
			\return Number of roots.
		*/
		template <typename T>
		GInt32 QuadraticFormula(T& R1, T& R2,
								const T& A, const T& B, const T& C, const T Epsilon = G_EPSILON) {

			if (Abs(A) <= Epsilon) {
				if (Abs(B) <= Epsilon)
					return 0;
				R1 = R2 = (-C / B);
				return 1;
			}
			else {
				const T det = B * B - (T)4 * A * C;
				if (Abs(det) <= Epsilon) {
					R1 = R2 = -B / (2 * A);
					return 1;
				}
				if (det > 0) {
					if (Abs(B) <= Epsilon) {
						R2 = Sqrt(-C / A);
						R1 = -R2;
						return 2;
					}
					// here we avoid numerical instability
					const T stableA = B / (2 * A);
					const T stableB = C / (A * stableA * stableA);
					const T stableC = (T)-1 - Sqrt(1 - stableB);
					R2 = stableA * stableC;
					R1 = (stableA * stableB) / stableC;
					return 2;
				}
				else
					return 0;
			}
		}
	}; // end namespace Math

}; // end namespace Amanith

#endif
