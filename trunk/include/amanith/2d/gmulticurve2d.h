/****************************************************************************
** $file: amanith/2d/g2dmulticurve.h   0.1.0.0   edited Jun 30 08:00
**
** 2D Base multicurve segment definition.
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

#ifndef GMULTICURVE2D_H
#define GMULTICURVE2D_H

/*!
	\file gcurve2d.h
	\brief Header file for 2D multicurve class.
*/

#include "amanith/2d/gcurve2d.h"

namespace Amanith {


	// *********************************************************************
	//                             GMultiCurve2D
	// *********************************************************************
	static const GClassID G_MULTICURVE2D_CLASSID = GClassID("GMultiCurve2D", 0x59BA6FA6, 0x62F943B2, 0xA5548655, 0xA4D349D1);

	class G_EXPORT GMultiCurve2D : public GCurve2D {

	private:

	protected:
		// add a point; if vertex pointer is NULL point must be inserted ON curve
		virtual GError DoAddPoint(const GReal Parameter, const GPoint2 *NewPoint, GUInt32& Index,
								  GBool& AlreadyExists) = 0;
		// remove a point(key); index is ensured to be valid and we are sure that after removing we'll have
		// (at least) a minimal (2-keys made) multi-curve
		virtual GError DoRemovePoint(const GUInt32 Index) = 0;
		// cloning function
		GError BaseClone(const GElement& Source);
		// get parameter corresponding to specified point index; index is ensured to be valid
		virtual GError DoGetPointParameter(const GUInt32 Index, GReal& Parameter) const = 0;
		// set parameter corresponding to specified point index; index is ensured to be valid
		virtual GError DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
										   GUInt32& NewIndex, GBool& AlreadyExists) = 0;

	public:
		// constructor
		GMultiCurve2D();
		// constructor
		GMultiCurve2D(const GElement* Owner);
		// destructor
		virtual ~GMultiCurve2D();
		// get parameter corresponding to specified point index
		GError PointParameter(const GUInt32 Index, GReal& Parameter) const;
		// set parameter corresponding to specified point index
		GError SetPointParameter(const GUInt32 Index, const GReal NewParamValue,
								 GUInt32& NewIndex, GBool& AlreadyExists);
		// add point on curve, at the specified parameter; in case of successful operation, Index will contain
		// the inserted point index into internal array
		GError AddPoint(const GReal Parameter, GUInt32& Index, GBool& AlreadyExists);
		// add point to curve, at the specified parameter; in case of successful operation, Index will contain
		// the inserted point index into internal array
		GError AddPoint(const GReal Parameter, const GPoint2& Point, GUInt32& Index, GBool& AlreadyExists);
		// remove a point from curve
		GError RemovePoint(const GUInt32 Index);
		// get class ID
		inline const GClassID& ClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
		// get class ID of base class
		inline const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};


	// *********************************************************************
	//                           GMultiCurve2DProxy
	// *********************************************************************
	class G_EXPORT GMultiCurve2DProxy : public GElementProxy {
	public:
		// get class ID
		const GClassID& ClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
		// get derived class id
		const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};

	static const GMultiCurve2DProxy G_MULTICURVE2D_PROXY;

};	// end namespace Amanith

#endif
