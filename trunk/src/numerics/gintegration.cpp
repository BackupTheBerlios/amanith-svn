/****************************************************************************
** $file: amanith/src/numerics/gintegration.h   0.3.0.0   edited Jan, 30 2006
**
** Integration of scalar functions.
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

#include "amanith/numerics/gintegration.h"
#include "amanith/gmath.h"

/*!
	\file gintegration.cpp
	\brief Implementation of numerical integration class.
*/


namespace Amanith {

	//  Romberg formula
	//
	//                          R(j, k-1) - R(j-1, k-1)
	//  R(j, k) = R(j, k-1) +  -------------------------
	//                                   k
	//                                  4  - 1
	//
	// Richardson (extrapolation coefficients) table for k = 1..26
	// RichardsonInvK[k] = 1 / (4^k - 1)

	
	#ifdef DOUBLE_REAL_TYPE
		#define RICHARDSON_TABLE_SIZE 26
		static const GReal RichardsonInvK[RICHARDSON_TABLE_SIZE] = {
			3.333333333333333333e-01, 6.666666666666666667e-02, 1.587301587301587302e-02,
			3.921568627450980392e-03, 9.775171065493646139e-04, 2.442002442002442002e-04,
			6.103888176768601599e-05, 1.525902189669642176e-05, 3.814711817595739730e-06,
			9.536752259018191355e-07, 2.384186359449949133e-07, 5.960464832810451556e-08,
			1.490116141589226448e-08, 3.725290312339701922e-09, 9.313225754828402544e-10,
			2.328306437080797376e-10, 5.820766091685553902e-11, 1.455191522857861004e-11,
			3.637978807104947841e-12, 9.094947017737554185e-13, 2.273736754432837583e-13,
			5.684341886081124604e-14, 1.421085471520220567e-14, 3.552713678800513551e-15,
			8.881784197001260212e-16, 2.220446049250313574e-16
		};
	#else
		#define RICHARDSON_TABLE_SIZE 11
		static const GReal RichardsonInvK[RICHARDSON_TABLE_SIZE] = {
			3.3333334326744080e-01f, 6.6666670143604279e-02f, 1.5873016789555550e-02f,
			3.9215688593685627e-03f, 9.7751710563898087e-04f, 2.4420025874860585e-04f,
			6.1038881540298462e-05f, 1.5259021893143654e-05f, 3.8147118175402284e-06f,
			9.5367522590095177e-07f, 2.3841863594498136e-07f
		};
	#endif

	/*!
		\param Result the output calculated integral.
		\param u0 lower bound of integration domain
		\param u1 upper bound of integration domain
		\param Function the callback function used to evaluate integrand.
		\param UserData pointer passed to the callback function.
		\param MaxError the maximum relative precision we wanna reach for the integral calculus.
		\return G_TRUE if the specified precision has been reached, G_FALSE otherwise
	*/
	GBool GIntegration::Romberg(GReal& Result, const GReal u0, const GReal u1, GFunction Function,
								void *UserData,	const GReal MaxError) {

		GReal h;
		GReal dt[1 + RICHARDSON_TABLE_SIZE];		// dt[i] is the last element in column i.
		GReal integral, err, oldErr;
		GReal x, old_h, delta = 0, maxMachineError;
		GUInt32 j, k;

		maxMachineError = GMath::Min(MaxError,  2 * G_EPSILON);
		// first interval length
		h = u1 - u0;
		// first integral approximation
		integral = (GReal)0.5 * (Function(u0, UserData) + Function(u1, UserData));
		// initialize err and the first column, dt[0], to the numerical estimate
		// of the integral using the trapezoidal rule with a step size of h
		dt[0] = integral * h;
		// for each possible succeeding column, halve the step size, calculate the composite trapezoidal rule
		// using the new step size, and up date preceding columns using Richardson extrapolation.
		oldErr = G_MAX_REAL;
		for (k = 1; k < RICHARDSON_TABLE_SIZE + 1; k++) {
			old_h = h;
			// calculate T(f, h/2, a, b) using T(f, h, a, b)
			h *= (GReal)0.5;
			integral = 0.0;
			for (x = u0 + h; x < u1; x += old_h)
				integral += Function(x, UserData);
			integral = h * integral + (GReal)0.5 * dt[0];
			// calculate the Richardson Extrapolation to the limit
			for (j = 0; j < k; j++) {
				delta = integral - dt[j];
				dt[j] = integral;
				integral += RichardsonInvK[j] * delta;
			} 
			// if the magnitude of the change in the extrapolated estimate for the integral is less than the
			// preassigned tolerance, return the estimate
			err = GMath::Abs(delta) / (GMath::Abs(integral) + 1);
			if ((err <= maxMachineError) || ((oldErr < err) && (k > (RICHARDSON_TABLE_SIZE / 2 + 1)))) {
				if (k > 2) {
					Result = integral;
					if (err <= maxMachineError)
						return G_TRUE;
					else
						return G_FALSE;
				}
			}
			oldErr = err;
			// store the current estimate in the k-th column
			dt[k] = integral;
		}
		// the process didn't converge within the preassigned tolerance using the maximum number of columns designated
		// return the current estimate of integral
		Result = integral;
		return G_FALSE;
	}
}
