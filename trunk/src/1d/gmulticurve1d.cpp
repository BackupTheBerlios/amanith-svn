/****************************************************************************
** $file: amanith/1d/src/gmulticurve1d.cpp   0.1.1.0   edited Sep 24 08:00
**
** 1D Base multicurve segment implementation.
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
	\file gmulticurve1d.cpp
	\brief Implementation file for GMultiCurve1D class.
*/

#include "amanith/1d/gmulticurve1d.h"

namespace Amanith {


// *********************************************************************
//                           GMultiCurve1D
// *********************************************************************

// constructor
GMultiCurve1D::GMultiCurve1D() : GCurve1D() {
}

// constructor
GMultiCurve1D::GMultiCurve1D(const GElement* Owner) : GCurve1D(Owner) {
}

// destructor
GMultiCurve1D::~GMultiCurve1D() {
}

// cloning function
GError GMultiCurve1D::BaseClone(const GElement& Source) {
	return GCurve1D::BaseClone(Source);
}

// get parameter corresponding to specified point index
GError GMultiCurve1D::PointParameter(const GUInt32 Index, GReal& Parameter) const {

	GUInt32 i = PointsCount();

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (i == 0)
		return G_INVALID_OPERATION;

	if (Index >= i)
		return G_OUT_OF_RANGE;

	return DoGetPointParameter(Index, Parameter);
}

// set parameter corresponding to specified point index
GError GMultiCurve1D::SetPointParameter(const GUInt32 Index, const GReal NewParamValue,
										GUInt32& NewIndex, GBool& AlreadyExists) {

	GUInt32 i = PointsCount();
	GError err;

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (i == 0)
		return G_INVALID_OPERATION;

	if (Index >= i)
		return G_OUT_OF_RANGE;

	err = DoSetPointParameter(Index, NewParamValue, NewIndex, AlreadyExists);
	if (err == G_NO_ERROR) {
		// we have to update domain interval in the case that point has been front/back "appended"
		if (NewParamValue < DomainStart() - G_EPSILON)
			err = GCurve1D::SetDomain(NewParamValue, DomainEnd());
		else
		if (NewParamValue > DomainEnd() + G_EPSILON)
			err = GCurve1D::SetDomain(DomainStart(), NewParamValue);
	}
	return err;
}


// add point on curve, at the specified parameter
GError GMultiCurve1D::AddPoint(const GReal Parameter, GUInt32& Index, GBool& AlreadyExists) {

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (PointsCount() < 2)
		return G_INVALID_OPERATION;

	// check out of range
	if (Parameter < DomainStart() || Parameter > DomainEnd())
		return G_OUT_OF_RANGE;
	// add point on curve
	return DoAddPoint(Parameter, NULL, Index, AlreadyExists);
}

GError GMultiCurve1D::AddPoint(const GReal Parameter, const GReal Point, GUInt32& Index, GBool& AlreadyExists) {

	GError err = DoAddPoint(Parameter, &Point, Index, AlreadyExists);

	if (err == G_NO_ERROR) {
		G_ASSERT(PointsCount() > 0);

		if (PointsCount() == 1)
			GCurve1D::SetDomain(Parameter, Parameter);
		else {
			// we have to update parameter interval in the case that point has been front/back "appended"
			if (Parameter < DomainStart() - G_EPSILON)
				GCurve1D::SetDomain(Parameter, DomainEnd());
			else
			if (Parameter > DomainEnd() + G_EPSILON)
				GCurve1D::SetDomain(DomainStart(), Parameter);
		}
	}
	return err;
}

GError GMultiCurve1D::RemovePoint(const GUInt32 Index) {

	GUInt32 i = PointsCount();
	GError err;
	GReal u;

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (i == 0)
		return G_INVALID_OPERATION;

	if (Index >= i)
		return G_OUT_OF_RANGE;

	err = DoRemovePoint(Index);
	// if we have remove first or last point, parameter range must be updated
	if (err == G_NO_ERROR) {

		GUInt32 j = PointsCount();
		if (j == 0)
			GCurve1D::SetDomain(G_MIN_REAL, G_MIN_REAL);
		else
		if (j == 1) {
			err = DoGetPointParameter(0, u);
			if (err == G_NO_ERROR)
				GCurve1D::SetDomain(u, u);
		}
		else {
			if (Index == 0) {
				err = DoGetPointParameter(0, u);
				if (err == G_NO_ERROR)
					GCurve1D::SetDomain(u, DomainEnd());
			}
			else
			if (Index == i - 1) {
				err = DoGetPointParameter(j - 1, u);
				if (err == G_NO_ERROR)
					GCurve1D::SetDomain(DomainStart(), u);
			}
		}
	}
	return err;
}

};	// end namespace Amanith
