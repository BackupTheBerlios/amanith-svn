/****************************************************************************
** $file: amanith/2d/src/gmulticurve2d.cpp   0.3.0.0   edited Jan, 30 2006
**
** 2D Base multicurve segment implementation.
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

// get curve tangent (specifying global parameter)
void GMultiCurve2D::TangentLR(const GReal u, GVector2& LeftTangent, GVector2& RightTangent) const {

	DerivativeLR(G_FIRST_ORDER_DERIVATIVE, u, LeftTangent, RightTangent);
	LeftTangent.Normalize();
	RightTangent.Normalize();
}

// get curve normal (specifying global parameter)
void GMultiCurve2D::NormalLR(const GReal u, GVector2& LeftNormal, GVector2& RightNormal) const {

	TangentLR(u, LeftNormal, RightNormal);
	LeftNormal.Swap(G_X, G_Y);
	LeftNormal[G_Y] = -LeftNormal[G_Y];
	RightNormal.Swap(G_X, G_Y);
	RightNormal[G_Y] = -LeftNormal[G_Y];
}

// get curvature (specifying global parameter)
void GMultiCurve2D::CurvatureLR(const GReal u, GReal& LeftCurvature, GReal& RightCurvature) const {

	// we have to calculate curvature K(u), from differential calculus:
	// v = first derivate vector
	// t = second derivate vector
	// --> k(u) = | v(u) cross t(u) | / |v(u)|^3
	GVector2 vL, vR;
	GVector2 tL, tR;	
	
	DerivativeLR(G_FIRST_ORDER_DERIVATIVE, u, vL, vR);
	DerivativeLR(G_SECOND_ORDER_DERIVATIVE, u, tL, tR);

	GReal lL = vL.LengthSquared();
	GReal lR = vR.LengthSquared();

	if (lL > G_EPSILON)
		LeftCurvature =  ((vL[G_X] * tL[G_Y]) - (vL[G_X] * tL[G_Y])) * (GMath::Pow(lL, (GReal)-1.5));
	else
		// degenerated case, we can't calculate curvature
		LeftCurvature = 0;

	if (lR > G_EPSILON)
		RightCurvature =  ((vR[G_X] * tR[G_Y]) - (vR[G_X] * tR[G_Y])) * (GMath::Pow(lR, (GReal)-1.5));
	else
		// degenerated case, we can't calculate curvature
		RightCurvature = 0;
}

// get curve speed (specifying global parameter)
void GMultiCurve2D::SpeedLR(const GReal u, GReal& LeftSpeed, GReal& RightSpeed) const {

	GVector2 tL, tR;
	
	DerivativeLR(G_FIRST_ORDER_DERIVATIVE, u, tL, tR);
	LeftSpeed = tL.Length();
	RightSpeed = tR.Length();
}


// get parameter corresponding to specified point index
GError GMultiCurve2D::PointParameter(const GUInt32 Index, GReal& Parameter) const {

	GUInt32 i = PointsCount();

	// operation can be done only if curve is still valid (at least made of 2 keys)
	if (i == 0)
		return G_INVALID_OPERATION;

	if (Index >= i)
		return G_OUT_OF_RANGE;

	return DoGetPointParameter(Index, Parameter);
}

// set parameter corresponding to specified point index
GError GMultiCurve2D::SetPointParameter(const GUInt32 Index, const GReal NewParamValue,
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
	if (PointsCount() < 2)
		return G_INVALID_OPERATION;

	// check out of range
	if (Parameter < DomainStart() || Parameter > DomainEnd())
		return G_OUT_OF_RANGE;
	// add point on curve
	return DoAddPoint(Parameter, NULL, Index, AlreadyExists);
}

GError GMultiCurve2D::AddPoint(const GReal Parameter, const GPoint2& Point, GUInt32& Index, GBool& AlreadyExists) {

	GError err = DoAddPoint(Parameter, &Point, Index, AlreadyExists);

	if (err == G_NO_ERROR) {
		G_ASSERT(PointsCount() > 0);

		if (PointsCount() == 1)
			GCurve2D::SetDomain(Parameter, Parameter);
		else {
			// we have to update parameter interval in the case that point has been front/back "appended"
			if (Parameter < DomainStart() - G_EPSILON)
				GCurve2D::SetDomain(Parameter, DomainEnd());
			else
			if (Parameter > DomainEnd() + G_EPSILON)
				GCurve2D::SetDomain(DomainStart(), Parameter);
		}
	}
	return err;
}

GError GMultiCurve2D::RemovePoint(const GUInt32 Index) {

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
			GCurve2D::SetDomain(G_MIN_REAL, G_MIN_REAL);
		else
		if (j == 1) {
			err = DoGetPointParameter(0, u);
			if (err == G_NO_ERROR)
				GCurve2D::SetDomain(u, u);
		}
		else {
			if (Index == 0) {
				err = DoGetPointParameter(0, u);
				if (err == G_NO_ERROR)
					GCurve2D::SetDomain(u, DomainEnd());
			}
			else
			if (Index == i - 1) {
				err = DoGetPointParameter(j - 1, u);
				if (err == G_NO_ERROR)
					GCurve2D::SetDomain(DomainStart(), u);
			}
		}
	}
	return err;
}

};	// end namespace Amanith
