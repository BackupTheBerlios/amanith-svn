/****************************************************************************
** $file: amanith/2d/src/g2dmulticurve.cpp   0.1.0.0   edited Jun 30 08:00
**
** 2D Base multicurve segment implementation.
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

/*!
	\file gmulticurve2d.cpp
	\brief Implementation file for GMultiCurve2D class.
*/

#include "amanith/2d/gmulticurve2d.h"

namespace Amanith {


// *********************************************************************
//                           GMultiCurve2D
// *********************************************************************

// constructor
GMultiCurve2D::GMultiCurve2D() : GCurve2D() {
}

// constructor
GMultiCurve2D::GMultiCurve2D(const GElement* Owner) : GCurve2D(Owner) {
}

// destructor
GMultiCurve2D::~GMultiCurve2D() {
}

// cloning function
GError GMultiCurve2D::BaseClone(const GElement& Source) {
	return GCurve2D::BaseClone(Source);
}

// get parameter corresponding to specified point index
GError GMultiCurve2D::PointParameter(const GUInt32 Index, GReal& Parameter) const {

	GInt32 i = PointsCount();

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (i <= 0)
		return G_INVALID_OPERATION;

	if ((GInt32)Index >= i)
		return G_OUT_OF_RANGE;

	return DoGetPointParameter(Index, Parameter);
}

// set parameter corresponding to specified point index
GError GMultiCurve2D::SetPointParameter(const GUInt32 Index, const GReal NewParamValue,
										GUInt32& NewIndex, GBool& AlreadyExists) {

	GInt32 i = PointsCount();
	GError err;

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (i <= 0)
		return G_INVALID_OPERATION;

	if ((GInt32)Index >= i)
		return G_OUT_OF_RANGE;

	err = DoSetPointParameter(Index, NewParamValue, NewIndex, AlreadyExists);
	if (err == G_NO_ERROR) {
		// we have to update domain interval in the case that point has been front/back "appended"
		if (NewParamValue < DomainStart() - G_EPSILON)
			err = GCurve2D::SetDomain(NewParamValue, DomainEnd());
		else
		if (NewParamValue > DomainEnd() + G_EPSILON)
			err = GCurve2D::SetDomain(DomainStart(), NewParamValue);
	}
	return err;
}


// add point on curve, at the specified parameter
GError GMultiCurve2D::AddPoint(const GReal Parameter, GUInt32& Index, GBool& AlreadyExists) {

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (PointsCount() <= 0)
		return G_INVALID_OPERATION;

	if (GMath::Abs(Parameter - DomainStart()) <= G_EPSILON)
		return G_NO_ERROR;
	if (GMath::Abs(Parameter - DomainEnd()) <= G_EPSILON)
		return G_NO_ERROR;
	// check out of range
	if (Parameter < DomainStart() || Parameter > DomainEnd())
		return G_OUT_OF_RANGE;
	// add point on curve
	return DoAddPoint(Parameter, NULL, Index, AlreadyExists);
}

GError GMultiCurve2D::AddPoint(const GReal Parameter, const GPoint2& Point, GUInt32& Index, GBool& AlreadyExists) {

	GError err;

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (PointsCount() <= 0)
		return G_INVALID_OPERATION;

	err = DoAddPoint(Parameter, &Point, Index, AlreadyExists);
	if (err == G_NO_ERROR) {
		// we have to update parameter interval in the case that point has been front/back "appended"
		if (Parameter < DomainStart() - G_EPSILON)
			GCurve2D::SetDomain(Parameter, DomainEnd());
		else
		if (Parameter > DomainEnd() + G_EPSILON)
			GCurve2D::SetDomain(DomainStart(), Parameter);
	}
	return err;
}

GError GMultiCurve2D::RemovePoint(const GUInt32 Index) {

	GInt32 i = PointsCount();
	GError err;
	GReal u;

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (i <= 0)
		return G_INVALID_OPERATION;

	if ((GInt32)Index >= i)
		return G_OUT_OF_RANGE;

	// if we remove a point in a minimal multi-curve, we empty the entire curve
	if (i <= 2) {
		Clear();
		return G_NO_ERROR;
	}

	err = DoRemovePoint(Index);
	// if we have remove first or last point, parameter range must be updated
	if (err == G_NO_ERROR) {
		if (Index == 0) {
			err = PointParameter(0, u);
			if (err == G_NO_ERROR)
				GCurve2D::SetDomain(u, DomainEnd());
		}
		else
		if ((GInt32)Index == i - 1) {
			err = PointParameter(i - 2, u);
			if (err == G_NO_ERROR)
				GCurve2D::SetDomain(DomainStart(), u);
		}
	}
	return err;
}

};	// end namespace Amanith
