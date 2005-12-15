/****************************************************************************
** $file: amanith/gfilter.h   0.2.0.0   edited Dec, 12 2005
**
** 1D filter definition.
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

#ifndef GFILTER_H
#define GFILTER_H

#include "amanith/gglobal.h"
#include "amanith/gmath.h"

/*!
	\file gfilter.h
	\brief Scalar filters definitions.
*/
namespace Amanith {

	//! Data structure for parameterized Mitchell filter. (float version)
	typedef struct {
		GFloat p0, p2, p3;
		GFloat q0, q1, q2, q3;
	} GMitchellDataFloat;

	//! Data structure for parameterized Mitchell filter. (double version)
	typedef struct {
		GDouble p0, p2, p3;
		GDouble q0, q1, q2, q3;
	} GMitchellDataDouble;

	//! Data structure for parameterized Kaiser window. (float version).
	typedef struct {
		// w * (N - 1) / 2 in Oppenheim & Schafer notation; typically 4 < a < 9
		// param a trades off main lobe width (sharpness) for side lobe amplitude (ringing)
		GFloat a;
		GFloat i0a;
	} GKaiserDataFloat;

	//! Data structure for parameterized Kaiser window. (double version).
	typedef struct {
		// w * (N - 1) / 2 in Oppenheim & Schafer notation; typically 4 < a < 9
		// param a trades off main lobe width (sharpness) for side lobe amplitude (ringing)
		GDouble a;
		GDouble i0a;
	} GKaiserDataDouble;

	// *************************************************************************************
	//         Unit-area filters for unit-spaced samples, all filters centered on 0
	// *************************************************************************************

	//! Box, pulse, Fourier window (1st order (constant) b-spline). Double version.
	inline GDouble BoxFilter(const GDouble x) {
		if (x < -0.5)
			return 0.0;
		if (x < 0.5)
			return 1.0;
		return 0.0;
	}
	//! Box, pulse, Fourier window (1st order (constant) b-spline). Float version.
	inline GFloat BoxFilter(const GFloat x) {
		if (x < -0.5f)
			return 0.0f;
		if (x < 0.5f)
			return 1.0f;
		return 0.0f;
	}

	//! Triangle, Bartlett window (2nd order (linear) b-spline). Double version.
	inline GDouble TriangleFilter(const GDouble x) {
		if (x < -1.0)
			return 0.0;
		if (x < 0.0)
			return (1.0 + x);
		if (x < 1.0)
			return (1.0 - x);
		return 0.0;
	}
	//! Triangle, Bartlett window (2nd order (linear) b-spline). Float version.
	inline GFloat TriangleFilter(const GFloat x) {
		if (x < -1.0f)
			return 0.0f;
		if (x < 0.0f)
			return (1.0f + x);
		if (x < 1.0f)
			return (1.0f - x);
		return 0.0f;
	}

	//! 3rd order (quadratic) b-spline. Double version.
	inline GDouble QuadraticFilter(const GDouble x) {

		GDouble t;

		if (x < -1.5)
			return 0.0;
		if (x <- 0.5) {
			t = x + 1.5;
			return (0.5 * t * t);
		}
		if (x < 0.5)
			return (0.75 - x * x);
		if (x < 1.5) {
			t = x - 1.5;
			return (0.5 * t * t);
		}
		return 0.0;
	}
	//! 3rd order (quadratic) b-spline. Float version.
	inline GFloat QuadraticFilter(const GFloat x) {

		GFloat t;

		if (x < -1.5f)
			return 0.0f;
		if (x <- 0.5f) {
			t = x + 1.5f;
			return (0.5f * t * t);
		}
		if (x < 0.5f)
			return (0.75f - x * x);
		if (x < 1.5f) {
			t = x - 1.5f;
			return (0.5f * t * t);
		}
		return 0.0f;
	}

	//! 4th order (cubic) b-spline. Double version.
	inline GDouble CubicFilter(const GDouble x) {

		GDouble t;

		if (x < -2.0)
			return 0.0;
		if (x < -1.0) {
			t = 2.0 + x;
			return (t * t * t / 6.0);
		}
		if (x < 0.0)
			return (4.0 + x * x * (-6.0 + x * -3.0)) / 6.0;
		if (x < 1.0)
			return (4.0 + x * x * (-6.0 + x * 3.0)) / 6.0;
		if (x < 2.0) {
			t = 2.0 - x;
			return (t * t * t / 6.0);
		}
		return 0.0;
	}
	//! 4th order (cubic) b-spline. Float version.
	inline GFloat CubicFilter(const GFloat x) {

		GFloat t;

		if (x < -2.0f)
			return 0.0f;
		if (x < -1.0f) {
			t = 2.0f + x;
			return (t * t * t / 6.0f);
		}
		if (x < 0.0f)
			return (4.0f + x * x * (-6.0f + x * -3.0f)) / 6.0f;
		if (x < 1.0f)
			return (4.0f + x * x * (-6.0f + x * 3.0f)) / 6.0f;
		if (x < 2.0f) {
			t = 2.0f - x;
			return (t * t * t / 6.0f);
		}
		return 0.0f;
	}

	//! Catmull-Rom spline, Overhauser spline. Double version.
	inline GDouble CatmullRomFilter(const GDouble x) {
		if (x < -2.0)
			return 0.0;
		if (x < -1.0)
			return 0.5 * (4.0 + x * (8.0 + x * (5.0 + x)));
		if (x < 0.0)
			return 0.5 * (2.0 + x * x * (-5.0 + x * -3.0));
		if (x < 1.0)
			return 0.5 * (2.0 + x * x * (-5.0 + x * 3.0));
		if (x < 2.0)
			return 0.5 * (4.0 + x * (-8.0 + x * (5.0 - x)));
		return 0.0;
	}
	//! Catmull-Rom spline, Overhauser spline. Float version.
	inline GFloat CatmullRomFilter(const GFloat x) {
		if (x < -2.0f)
			return 0.0f;
		if (x < -1.0f)
			return 0.5f * (4.0f + x * (8.0f + x * (5.0f + x)));
		if (x < 0.0f)
			return 0.5f * (2.0f + x * x * (-5.0f + x * -3.0f));
		if (x < 1.0f)
			return 0.5f * (2.0f + x * x * (-5.0f + x * 3.0f));
		if (x < 2.0f)
			return 0.5f * (4.0f + x * (-8.0f + x * (5.0f - x)));
		return 0.0f;
	}

	//! Gaussian (infinite) filter. Double version.
	inline GDouble GaussianFilter(const GDouble x) {
		return GMath::Exp(-2.0 * x * x) * GMath::Sqrt(2.0 / G_PI);
	}
	//! Gaussian (infinite) filter. Float version.
	inline GFloat GaussianFilter(const GFloat x) {
		return GMath::Exp(-2.0f * x * x) * GMath::Sqrt(2.0f / (GFloat)G_PI);
	}
	//! Sinc, perfect low-pass filter (infinite). Double version.
	inline GDouble SincFilter(const GDouble x) {
		return x == 0.0 ? 1.0 : GMath::Sin(G_PI * x) / (G_PI * x);
	}
	//! Sinc, perfect low-pass filter (infinite). Float version.
	inline GFloat SincFilter(const GFloat x) {
		return x == 0.0f ? 1.0f : GMath::Sin((GFloat)G_PI * x) / ((GFloat)G_PI * x);
	}
	//! Bessel (for circularly symmetric 2D filter, infinite). Double version.
	inline GDouble BesselFilter(const GDouble x) {
		// See Pratt "Digital Image Processing" p. 97 for Bessel functions
		// zeros are at approx x=1.2197, 2.2331, 3.2383, 4.2411, 5.2428, 6.2439,
		// 7.2448, 8.2454
		return x == 0.0 ? G_PI / 4.0 : GMath::BesselJ1(G_PI * x) /(2.0 * x);
	}
	//! Bessel (for circularly symmetric 2D filter, infinite). Float version.
	inline GFloat BesselFilter(const GFloat x) {
		// See Pratt "Digital Image Processing" p. 97 for Bessel functions
		// zeros are at approx x=1.2197, 2.2331, 3.2383, 4.2411, 5.2428, 6.2439,
		// 7.2448, 8.2454
		return x == 0.0f ? (GFloat)G_PI / 4.0f : GMath::BesselJ1((GFloat)G_PI * x) /(2.0f * x);
	}
	//! Bell filter (3 box convolutions). Double version.
	inline GDouble BellFilter(const GDouble x) {
		// box (*) box (*) box
		GDouble xx = x;
		if (xx < 0)
			xx = -xx;
		if (xx < 0.5)
			return (0.75 - (xx * xx));
		if (xx < 1.5) {
			GDouble t = xx - 1.5;
			return (0.5 * (t * t));
		}
		return 0.0;
	}
	//! Bell filter (3 box convolutions). Float version.
	inline GFloat BellFilter(const GFloat x) {
		// box (*) box (*) box
		GFloat xx = x;
		if (xx < 0)
			xx = -xx;
		if (xx < 0.5f)
			return (0.75f - (xx * xx));
		if (xx < 1.5f) {
			GFloat t = xx - 1.5f;
			return (0.5f * (t * t));
		}
		return 0.0f;
	}

	// *************************************************************************************
	//                                   Parameterized filters
	// *************************************************************************************

	//! Mitchell & Netravali's two-param cubic. Double version.
	inline GDouble MitchellFilter(const GDouble x, const GMitchellDataDouble *d) {
			
		// see Mitchell & Netravali, "Reconstruction Filters in Computer Graphics", SIGGRAPH 88
		if (x < -2.0)
			return 0.0;
		if (x < -1.0)
			return d->q0 - x * (d->q1 - x * (d->q2 - x * d->q3));
		if (x < 0.0)
			return d->p0 + x * x * (d->p2 - x * d->p3);
		if (x < 1.0)
			return d->p0 + x * x * (d->p2 + x * d->p3);
		if (x < 2.0)
			return d->q0 + x * (d->q1 + x * (d->q2 + x * d->q3));
		return 0.0;
	}
	//! Mitchell & Netravali's two-param cubic. Float version.
	inline GFloat MitchellFilter(const GFloat x, const GMitchellDataFloat *d) {
		G_ASSERT(d != NULL);

		// see Mitchell & Netravali, "Reconstruction Filters in Computer Graphics", SIGGRAPH 88
		if (x < -2.0f)
			return 0.0f;
		if (x < -1.0f)
			return d->q0 - x * (d->q1 - x * (d->q2 - x * d->q3));
		if (x < 0.0f)
			return d->p0 + x * x * (d->p2 - x * d->p3);
		if (x < 1.0f)
			return d->p0 + x * x * (d->p2 + x * d->p3);
		if (x < 2.0f)
			return d->q0 + x * (d->q1 + x * (d->q2 + x * d->q3));
		return 0.0f;
	}

	inline void MitchellFilterInit(const GDouble b, const GDouble c, GMitchellDataDouble *d) {
		if (d) {
			d->p0 = (  6.0 -  2.0 * b           ) / 6.0;
			d->p2 = (-18.0 + 12.0 * b +  6.0 * c) / 6.0;
			d->p3 = ( 12.0 -  9.0 * b -  6.0 * c) / 6.0;
			d->q0 = (	      8.0 * b + 24.0 * c) / 6.0;
			d->q1 = (	   - 12.0 * b - 48.0 * c) / 6.0;
			d->q2 = (	      6.0 * b + 30.0 * c) / 6.0;
			d->q3 = (      -        b -  6.0 * c) / 6.0;
		}
	}
	inline void MitchellFilterInit(const GFloat b, const GFloat c, GMitchellDataFloat *d) {
		if (d) {
			d->p0 = (  6.0f -  2.0f * b            ) / 6.0f;
			d->p2 = (-18.0f + 12.0f * b +  6.0f * c) / 6.0f;
			d->p3 = ( 12.0f -  9.0f * b -  6.0f * c) / 6.0f;
			d->q0 = (	       8.0f * b + 24.0f * c) / 6.0f;
			d->q1 = (	    - 12.0f * b - 48.0f * c) / 6.0f;
			d->q2 = (	       6.0f * b + 30.0f * c) / 6.0f;
			d->q3 = (       -         b -  6.0f * c) / 6.0f;
		}
	}

	// *************************************************************************************
	//                                   Window functions
	// *************************************************************************************

	//! Hanning window. Double version.
	inline GDouble HanningFilter(const GDouble x) {
		return 0.5 + 0.5 * GMath::Cos(G_PI * x);
	}
	//! Hanning window. Float version.
	inline GFloat HanningFilter(const GFloat x) {
		return 0.5f + 0.5f * GMath::Cos((GFloat)G_PI * x);
	}

	//! Hamming window. Double version.
	inline GDouble HammingFilter(const GDouble x) {
		return 0.54 + 0.46 * GMath::Cos(G_PI * x);
	}
	//! Hamming window. Float version.
	inline GFloat HammingFilter(const GFloat x) {
		return 0.54f + 0.46f * GMath::Cos((GFloat)G_PI * x);
	}

	//! Blackman window. Double version.
	inline GDouble BlackmanFilter(const GDouble x) {
		return 0.42 + 0.50 * GMath::Cos(G_PI * x) + 0.08 * GMath::Cos(2.0 * G_PI * x);
	}
	//! Blackman window. Float version.
	inline GFloat BlackmanFilter(const GFloat x) {
		return 0.42f + 0.50f * GMath::Cos((GFloat)G_PI * x) + 0.08f * GMath::Cos(2.0f * (GFloat)G_PI * x);
	}


	// *************************************************************************************
	//                                 Parameterized windows
	// *************************************************************************************

	//! Parameterized Kaiser window. Double version.
	inline GDouble KaiserFilter(const GDouble x, const GKaiserDataDouble *d) {
		G_ASSERT(d != NULL);
		// from Oppenheim & Schafer, Hamming
		return GMath::BesselI0(d->a * GMath::Sqrt(1.0 - x * x)) * d->i0a;
	}
	//! Parameterized Kaiser window. Float version.
	inline GFloat KaiserFilter(const GFloat x, const GKaiserDataFloat *d) {
		G_ASSERT(d != NULL);
		// from Oppenheim & Schafer, Hamming
		return GMath::BesselI0(d->a * GMath::Sqrt(1.0f - x * x)) * d->i0a;
	}

	inline GError KaiserFilterInit(const GDouble a, GKaiserDataDouble *d) {
		if (d) {
			d->a = a;
			d->i0a = 1.0 / GMath::BesselI0(a);
			return G_NO_ERROR;
		}
		else
			return G_INVALID_PARAMETER;
	}
	inline GError KaiserFilterInit(const GFloat a, GKaiserDataFloat *d) {
		if (d) {
			d->a = a;
			d->i0a = 1.0f / GMath::BesselI0(a);
			return G_NO_ERROR;
		}
		else
			return G_INVALID_PARAMETER;
	}


	// *************************************************************************************
	//                           Filters for non-unit spaced samples
	// *************************************************************************************

	//! Normal distribution (infinite). Double version.
	inline GDouble NormalFilter(const GDouble x) {
		// normal distribution: has unit area, but it's not for unit spaced samples
		// Normal(x) = Gaussian(x/2)/2
		return GMath::Exp(-x * x / 2.0) / GMath::Sqrt(2.0 * G_PI);
	}
	//! Normal distribution (infinite). Float version.
	inline GFloat NormalFilter(const GFloat x) {
		// normal distribution: has unit area, but it's not for unit spaced samples
		// Normal(x) = Gaussian(x/2)/2
		return GMath::Exp(-x * x / 2.0f) / GMath::Sqrt(2.0f * (GFloat)G_PI);
	}

};	// end namespace Amanith

#endif
