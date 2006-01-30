/****************************************************************************
** $file: amanith/geometry/glineseg.h   0.3.0.0   edited Jan, 30 2006
**
** Generic line segment definition.
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

#ifndef GLINESEG_H
#define GLINESEG_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gray.h"
#include "amanith/geometry/gvect.h"

/*!
	\file glineseg.h
	\brief Header file GLineSeg class.
*/
namespace Amanith {
	/*!
		\class GLineSeg
		\brief Describes a compact (finite) line segment.

		This is represented by a point origin O and a vector spanning the length of the line segment originating at O.
		Thus any point on the line segment can be described as:\n\n

		P(t) = O + t * D\n\n

		where 0 <= t <= 1

    	\param DATA_TYPE the internal type used for the point and vector.
		\param SIZE the dimension of space (ex: 2 for a 2D line segment, 3 for a 3D line segment).
		It must be a positive number.
	*/
	template <typename DATA_TYPE, GUInt32 SIZE>
	class GLineSeg : public GGenericRay<DATA_TYPE, SIZE> {
	public:
		//! Default constructor, constructs a line segment at the origin with a zero vector.
		GLineSeg() {
		}

		/*!
			Set constructor, constructs a ray with the given origin and vector.

			\param Origin the point at which the ray starts
			\param Direction the vector describing the length vector of line segment
		*/
		GLineSeg(const GPoint<DATA_TYPE, SIZE>& Origin, const GVect<DATA_TYPE, SIZE>& Direction)
		: GGenericRay<DATA_TYPE, SIZE>(Origin, Direction) {
		}

		/*!
			Copy constructor, constructs an exact duplicate of the given line segment.

			\param SourceLineSeg the line segment to copy
		*/
		GLineSeg(const GLineSeg<DATA_TYPE, SIZE>& SourceLineSeg) : GGenericRay<DATA_TYPE, SIZE>(SourceLineSeg) {
		}

		/*!
			Set constructor, constructs a line segment with the given begin and end points.
		
			\param Origin the point at the beginning of the line segment
			\param Destination the point at the end of the line segment
		*/
		GLineSeg(const GPoint<DATA_TYPE, SIZE>& Origin, const GPoint<DATA_TYPE, SIZE>& Destination)
		: GGenericRay<DATA_TYPE, SIZE>(Origin, Destination - Origin) {
		}

		//! Gets the length of this line segment.
		DATA_TYPE Length() const {
			return this->gDir.Length();
		}

		//! Gets the squared length of this line segment.
		DATA_TYPE LengthSquared() const {
			return(this->gDirection.LengthSquared());
		}

		//! Get start point (the result is the same of Origin() function).
		inline GPoint<DATA_TYPE, SIZE> StartPoint() const {
			return this->gOrigin;
		}
		//! Set start point 8the result is the same of SetOrigin() function).
		inline void SetStartPoint(const GPoint<DATA_TYPE, SIZE>& NewValue) {
			this->gOrigin = NewValue;
		}
		//! Get end point.
		inline GPoint<DATA_TYPE, SIZE> EndPoint() const {
			return (this->gOrigin + this->gDirection);
		}
		// set end point
		inline void SetEndPoint(const GPoint<DATA_TYPE, SIZE>& NewValue) {
			this->gDirection = NewValue - StartPoint();
		}
	};

	//! Common 2D line segment class, it uses GReal data type.
	typedef GLineSeg<GReal, 2> GLineSegment2;
	//! Common 3D line segment class, it uses GReal data type.
	typedef GLineSeg<GReal, 3> GLineSegment3;

};	// end namespace Amanith

#endif
