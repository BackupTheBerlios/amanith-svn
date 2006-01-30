/****************************************************************************
** $file: amanith/geometry/garea.h   0.3.0.0   edited Jan, 30 2006
**
** Basic area calculation and predicates.
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

#ifndef GAREA_H
#define GAREA_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gvect.h"

/*!
	\file garea.h
	\brief Area calculation header file.
*/
namespace Amanith {

	//! Compute the twice signed area of triangle defined by P1-P2-P3 points
	template<typename DATA_TYPE>
	inline DATA_TYPE TwiceSignedArea(const GVectBase<DATA_TYPE, 2>& P1, const GVectBase<DATA_TYPE, 2>& P2,
									 const GVectBase<DATA_TYPE, 2>& P3) {
		// the default implementation would be:
		// return (P2[G_X] - P1[G_X]) * (P3[G_Y] - P1[G_Y]) - (P2[G_Y] - P1[G_Y]) * (P3[G_X] - P1[G_X]);
		// Here we use a more numerically stable implementation, even if for some almost degenerate
		// situations, the results are not reliable.
		return P1[G_Y] * (P3[G_X] - P2[G_X]) + P2[G_Y] * (P1[G_X] - P3[G_X]) + P3[G_Y] * (P2[G_X] - P1[G_X]);
	}
	//! Compute the signed area of triangle defined by P1-P2-P3 points
	template<typename DATA_TYPE>
	inline DATA_TYPE SignedArea(const GVectBase<DATA_TYPE, 2>& P1, const GVectBase<DATA_TYPE, 2>& P2,
								const GVectBase<DATA_TYPE, 2>& P3) {
		return(TwiceSignedArea(P1, P2, P3) / (DATA_TYPE)2);
	}

	//! Returns G_TRUE if the point TestPoint is inside the circle defined by the points P1, P2, P3
	template<typename DATA_TYPE>
	inline GBool IsInCircle(const GVectBase<DATA_TYPE, 2>& P1, const GVectBase<DATA_TYPE, 2>& P2,
							const GVectBase<DATA_TYPE, 2>& P3, const GVectBase<DATA_TYPE, 2>& TestPoint) {

		DATA_TYPE r = (P1[G_X] * P1[G_X] + P1[G_Y] * P1[G_Y]) * TwiceSignedArea(P2, P3, TestPoint) -
					  (P2[G_X] * P2[G_X] + P2[G_Y] * P2[G_Y]) * TwiceSignedArea(P1, P3, TestPoint) +
					  (P3[G_X] * P3[G_X] + P3[G_Y] * P3[G_Y]) * TwiceSignedArea(P1, P2, TestPoint) -
					  (TestPoint[G_X] * TestPoint[G_X] + TestPoint[G_Y] * TestPoint[G_Y]) * TwiceSignedArea(P1, P2, P3);
		return (r > 0);
	}
	/*!
		Returns G_TRUE if the points P1, P2, P3 are in a counterclockwise order.

		\param P1 first point of test sequence
		\param P2 second point of test sequence
		\param P3 third point of test sequence
		\param Precision a tolerance value used in the test.
		\note Result is in accord with Amanith coordinate system (x going left to right, and y upward)
	*/
	template<typename DATA_TYPE>
	inline GBool CounterClockWise(const GVectBase<DATA_TYPE, 2>& P1, const GVectBase<DATA_TYPE, 2>& P2,
								  const GVectBase<DATA_TYPE, 2>& P3, const DATA_TYPE Precision = (DATA_TYPE)G_EPSILON) {

		return (TwiceSignedArea(P1, P2, P3) > GMath::Abs(Precision));
	}

	/*!
		Returns G_TRUE if the points P1, P2, P3 are in a counterclockwise order or they are aligned (under an epsilon precision).

		\param P1 first point of test sequence
		\param P2 second point of test sequence
		\param P3 third point of test sequence
		\param Precision a tolerance value used in the test.
		\note Result is in accord with Amanith coordinate system (x going left to right, and y upward)
	*/
	template<typename DATA_TYPE>
	inline GBool CounterClockWiseOrAligned(const GVectBase<DATA_TYPE, 2>& P1, const GVectBase<DATA_TYPE, 2>& P2,
										   const GVectBase<DATA_TYPE, 2>& P3, const DATA_TYPE Precision = (DATA_TYPE)G_EPSILON) {

		return (TwiceSignedArea(P1, P2, P3) >= -GMath::Abs(Precision));
	}

	/*!
		Returns G_TRUE if the points P1, P2, P3 are in a clockwise order.

		\param P1 first point of test sequence
		\param P2 second point of test sequence
		\param P3 third point of test sequence
		\param Precision a tolerance value used in the test.
		\note Result is in accord with Amanith coordinate system (x going left to right, and y upward)
	*/
	template<typename DATA_TYPE>
	inline GBool ClockWise(const GVectBase<DATA_TYPE, 2>& P1, const GVectBase<DATA_TYPE, 2>& P2,
						   const GVectBase<DATA_TYPE, 2>& P3, const DATA_TYPE Precision = (DATA_TYPE)G_EPSILON) {

		return (TwiceSignedArea(P1, P2, P3) < -GMath::Abs(Precision));
	}

	/*!
		Returns G_TRUE if the points P1, P2, P3 are in a clockwise order or they are aligned (under an epsilon precision).

		\param P1 first point of test sequence
		\param P2 second point of test sequence
		\param P3 third point of test sequence
		\param Precision a tolerance value used in the test.
		\note Result is in accord with Amanith coordinate system (x going left to right, and y upward)
	*/
	template<typename DATA_TYPE>
	inline GBool ClockWiseOrAligned(const GVectBase<DATA_TYPE, 2>& P1, const GVectBase<DATA_TYPE, 2>& P2,
									const GVectBase<DATA_TYPE, 2>& P3, const DATA_TYPE Precision = (DATA_TYPE)G_EPSILON) {

		return (TwiceSignedArea(P1, P2, P3) <= GMath::Abs(Precision));
	}

	/*!
		Returns G_TRUE if the points P1, P2, P3 are aligned (under an epsilon precision).

		\param P1 first point of test sequence.
		\param P2 second point of test sequence.
		\param P3 third point of test sequence.
		\param Precision a tolerance value used in the test.
		\note Result is in accord with Amanith coordinate system (x going left to right, and y upward).
	*/
	template<typename DATA_TYPE>
	inline GBool Aligned(const GVectBase<DATA_TYPE, 2>& P1, const GVectBase<DATA_TYPE, 2>& P2,
						 const GVectBase<DATA_TYPE, 2>& P3, const DATA_TYPE Precision = (DATA_TYPE)G_EPSILON) {
		return (GMath::Abs(TwiceSignedArea(P1, P2, P3)) <= GMath::Abs(Precision));
	}

	/*!
		Given 3 vectors with the same origin:

		-# target: Origin-->Destination
		-# v1: Origin-->Dest1
		-# v2: Origin-->Dest2

		it finds witch vector (v1 or v2) span the smallest angle in counter clock wise order to overlap target vector.
		\return 1 if v1 spans the smallest angle in counter clock wise order to overlap target vector, else 2.
	*/
	template<typename DATA_TYPE>
	GUInt32 CCWSmallerAngleSpan(const GPoint<DATA_TYPE, 2>& Origin, const GPoint<DATA_TYPE, 2>& Dest1,
								const GPoint<DATA_TYPE, 2>& Dest2, const GPoint<DATA_TYPE, 2>& Destination,
								const DATA_TYPE Precision = (DATA_TYPE)G_EPSILON) {

		DATA_TYPE area1 = TwiceSignedArea(Dest1, Origin, Destination);
		DATA_TYPE normArea1;
		DATA_TYPE area2 = TwiceSignedArea(Dest2, Origin, Destination);
		DATA_TYPE copyArea2 = area2;
		DATA_TYPE normArea2;
		DATA_TYPE concord;
		GBool swapped;

		DATA_TYPE area1Abs = GMath::Abs(area1);
		DATA_TYPE area2Abs = GMath::Abs(area2);

		// area normalization, we wanna area1 the less one
		if (area1Abs > area2Abs) {
			normArea1 = area2 / area1Abs;
			normArea2 = area1;
			swapped = G_TRUE;
		}
		else {
			normArea1 = area1 / area2Abs;
			normArea2 = area2;
			swapped = G_FALSE;
		}

		if ((normArea1 > Precision && normArea2 > 0) || (normArea1 < -Precision && normArea2 < 0)) {

			area1 = -area1;
			area2 = TwiceSignedArea(Dest2, Origin, Dest1);
			// area1Abs is still valid
			area2Abs = GMath::Abs(area2);
			// area normalization, we wanna area1 the less one
			if (area1Abs > area2Abs) {
				normArea1 = area2 / area1Abs;
				normArea2 = area1;
			}
			else {
				normArea1 = area1 / area2Abs;
				normArea2 = area2;
			}

			if ((normArea1 > Precision && normArea2 > 0) || (normArea1 < -Precision && normArea2 < 0)) {

				area1 = -area2;
				area2 = -copyArea2;
				if (area2 < 0)
					return 1;
				else
				if (area2 > 0)
					return 2;
				else {
					G_ASSERT(0 == 1);
					return 0xFFFF;
				}
			}
			else {
				if (area2 < 0)
					return 1;
				else
					if (area2 >= 0)
						return 2;
					else {
						G_ASSERT(0 == 1);
						return 0xFFFF;
					}
			}

		}
		else {
			if (!swapped) {
				if (area2 < -Precision) {
					if (GMath::Abs(area1) <= Precision) {
						concord = Dot(Destination - Origin, Dest1 - Origin);
						G_ASSERT(concord != 0);
						if (concord > 0)
							return 1;
						else
							return 2;
					}
					else
						return 2;
				}
				else
				if (area2 > Precision)
					return 1;
				// area2 == 0
				else {
					concord = Dot(Destination - Origin, Dest2 - Origin);
					if (concord > 0)
						return 2;
					else
						return 1;
				}
			}
			else {
				if (area1 < -Precision) {
					if (GMath::Abs(area2) <= Precision) {
						concord = Dot(Destination - Origin, Dest2 - Origin);
						G_ASSERT(concord != 0);
						if (concord > 0)
							return 2;
						else
							return 1;
					}
					else
						return 1;
				}
				else
				if (area1 > Precision)
					return 2;
				else {
					concord = Dot(Destination - Origin, Dest1 - Origin);
					if (concord > 0)
						return 1;
					else
						return 2;
				}
			}
		}
	}

};	// end namespace Amanith

#endif
