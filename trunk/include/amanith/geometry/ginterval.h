/****************************************************************************
** $file: amanith/geometry/ginterval.h   0.3.0.0   edited Jan, 30 2006
**
** 1D Generic interval definition and implementation.
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

#ifndef GINTERVAL_H
#define GINTERVAL_H

/*!
	\file ginterval.h
	\brief Header file for GInterval class.
*/

namespace Amanith {

	// *********************************************************************
	//                              GInterval
	// *********************************************************************

	/*!
		\class GInterval
		\brief GInterval represents a mono dimensional interval.

		Intervals can be intersected, joined and some useful queries are provided by this class.
		\param DATA_TYPE the type used for interval ends.
	*/
	template <typename DATA_TYPE>
	class GInterval {

	private:
		//! Lower bound
		DATA_TYPE gStart;
		//! Upper bound
		DATA_TYPE gEnd;

		//! Check and (eventually) sort interval ends, so gStart is always before gEnd
		inline GBool SortInterval() {
			if (gStart > gEnd) {
				DATA_TYPE tmp = gStart;
				gStart = gEnd;
				gEnd = tmp;
				return G_TRUE;
			}
			else
				return G_FALSE;
		}
	public:
		//! Default constructor, interval ends are set to 0.
		GInterval() : gStart(0), gEnd(0) {
		}
		/*!
			Set constructor, interval ends are given as parameter.

			\note After this constructor is always ensured that interval ends are ordered in the correct way.
		*/
		GInterval(const DATA_TYPE StartValue, const DATA_TYPE EndValue)	: gStart(StartValue), gEnd(EndValue) {
			SortInterval();
		}
		//! Copy constructor.
		inline GInterval(const GInterval<DATA_TYPE>& Interval) {
			gStart = Interval.gStart;
			gEnd = Interval.gEnd;
		}
		//! Destructor, it does nothing.
		~GInterval() {
		}
		//! Get interval lower bound
		inline const DATA_TYPE Start() const {
			return gStart;
		}
		//! Get interval upper bound
		inline const DATA_TYPE End() const {
			return gEnd;
		}
		//! Set interval lower bound; after this call is always ensured that interval ends are ordered in the correct way.
		inline void SetStart(const DATA_TYPE& StartValue) {
			gStart = StartValue;
			SortInterval();
		}
		//! Set interval upper bound; after this call is always ensured that interval ends are ordered in the correct way.
		inline void SetEnd(const DATA_TYPE& EndValue) {
			gEnd = EndValue;
			SortInterval();
		}

		//! Set both interval bounds; after this call is always ensured that interval ends are ordered in the correct way.
		inline void Set(const DATA_TYPE& StartValue, const DATA_TYPE& EndValue) {
			gStart = StartValue;
			gEnd = EndValue;
			SortInterval();
		}

		//! Test if passed value is inside (including ends) the interval.
		inline GBool IsInInterval(const DATA_TYPE& Value) const {
			if ((Value >= gStart) && (Value <= gEnd))
				return G_TRUE;
			else
				return G_FALSE;
		}
		//! Test if specified interval is completely included in this interval (including ends)
		inline GBool IsIncluded(const GInterval<DATA_TYPE>& Interval) const {
			if ((Interval.Start() >= gStart) && (Interval.End() <= gEnd))
				return G_TRUE;
			else
				return G_FALSE;
		}
		//! Get interval length
		inline const DATA_TYPE Length() const {
			return (gEnd - gStart);
		}
		//! Check if interval is empty. It's empty if its upper bound is less or equal to its lower bound.
		inline const GBool IsEmpty() const {
			if (gEnd <= gStart)
				return G_TRUE;
			else
				return G_FALSE;
		}
		//! Copy operator
		inline GInterval<DATA_TYPE>& operator =(const GInterval<DATA_TYPE>& Source) {
			gStart = Source.gStart;
			gEnd = Source.gEnd;
			return *this;
		}
		//! Intersection operator
		GInterval<DATA_TYPE>& operator &=(const GInterval<DATA_TYPE>& Source) {
			
			if (Source.IsEmpty() || IsEmpty()) {
				gStart = 0;
				gEnd = 0;
			}
			else {
				DATA_TYPE newMin = GMath::Max(gStart, Source.gStart);
				DATA_TYPE newMax = GMath::Min(gEnd, Source.gEnd);

				if (newMin > newMax) {
					gStart = 0;
					gEnd = 0;
				}
				else {
					gStart = newMin;
					gEnd = newMax;
				}
			}
			return *this;
		}
		//! Union operator
		inline GInterval<DATA_TYPE>& operator |=(const GInterval<DATA_TYPE>& Source) {

			if (!Source.IsEmpty()) {
				if (Source.gStart < gStart)
					gStart = Source.gStart;
				if (Source.gEnd > gEnd)
					gEnd = Source.gEnd;
			}
			return *this;
		}

		/*!
			Calculates the number of cycles 'Value' is long; each cycle has a length of Length() unit. Example:
\code
	GInterval<GInt32> i(-2, 5);
	GInt32 c1 = i.CyclesCount(7);   // now c1 is equal to 1
	GInt32 c2 = i.CyclesCount(-18); // now c2 is equal to 3
	GInt32 c3 = i.CyclesCount(0);   // now c3 is equal to 0
\endcode
		*/
		inline GInt32 CyclesCount(const DATA_TYPE& Value) const {

			DATA_TYPE l = Length();
			if (l <= 0)
				return 1;
			if (Value < gStart)
				return (GInt32)GMath::Trunc((GMath::Abs(Value - gStart) / l)) + 1;
			else 
			if (Value > gEnd)
				return (GInt32)GMath::Trunc((GMath::Abs(Value - gEnd) / l)) + 1;
			else
				return 0;
		}

		/*!
			Cycles the passed value, so that the result is always inside interval. Example:
\code
	GInterval<GInt32> i(-2, 5);
	GInt32 c1 = i.CyclesCount(7);   // now c1 is equal to 0
	GInt32 c2 = i.CyclesCount(-10); // now c2 is equal to 4
	GInt32 c3 = i.CyclesCount(3);   // now c3 is equal to 3
\endcode
		*/
		inline DATA_TYPE CycleValue(const DATA_TYPE& Value) const {

			DATA_TYPE l = Length();

			if (l <= 0)
				return Value;

			DATA_TYPE delta = (Value - gStart);
			DATA_TYPE retValue = GMath::Trunc(delta / l);
			retValue = delta - (retValue * l);
			if (Value < gStart)
				return (gEnd + retValue);
			else
				return (gStart + retValue);
		}

		//! Binary intersection operator
		inline GInterval<DATA_TYPE> operator &(const GInterval<DATA_TYPE>& Interval) {
			
			GInterval<DATA_TYPE> ret_val(*this);
			ret_val &= Interval;
			return ret_val;
		}

		//! Binary union operator
		inline GInterval<DATA_TYPE> operator |(const GInterval<DATA_TYPE>& Interval) {

			GInterval<DATA_TYPE> ret_val(*this);
			ret_val |= Interval;
			return ret_val;
		}
	};

};	// end namespace Amanith

#endif
