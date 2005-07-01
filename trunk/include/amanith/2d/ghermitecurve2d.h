/****************************************************************************
** $file: amanith/2d/g2dhermitecurve.h   0.1.0.0   edited Jun 30 08:00
**
** 2D Hermite curve segment definition.
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

#ifndef GHERMITECURVE2D_H
#define GHERMITECURVE2D_H

/*!
	\file ghermitecurve2d.h
	\brief Header file for 2D Hermite multicurve class.
*/

#include "amanith/2d/gmulticurve2d.h"
#include "amanith/2d/gbeziercurve2d.h"

namespace Amanith {


	// *********************************************************************
	//                           GHermiteCurve2D
	// *********************************************************************
	static const GClassID G_HERMITECURVE2D_CLASSID = GClassID("GHermiteCurve2D", 0xF2D8D69F, 0x8413481D, 0xBBA1F099, 0x6F724027);

	struct GHermiteKey {
		GReal Parameter;
		GPoint2 Value;
		GVector2 InTangent;  // incoming tangent
		GVector2 OutTangent;  // outgoing tangent

		// constructor
		GHermiteKey() {
			Parameter = 0;
		}
		// constructor
		GHermiteKey(const GReal _Parameter, const GPoint2& _Value) : Parameter(_Parameter), Value(_Value) {
		}
		// constructor
		GHermiteKey(const GReal _Parameter, const GPoint2& _Value, const GVector2& _InTangent,
					const GVector2& _OutTangent) : Parameter(_Parameter), Value(_Value), InTangent(_InTangent),
					OutTangent(_OutTangent) {
		}
		GHermiteKey(const GHermiteKey& Source) {
			Parameter = Source.Parameter;
			Value = Source.Value;
			InTangent = Source.InTangent;
			OutTangent = Source.OutTangent;
		}
		GHermiteKey& operator =(const GHermiteKey& Source) {
			Parameter = Source.Parameter;
			Value = Source.Value;
			InTangent = Source.InTangent;
			OutTangent = Source.OutTangent;
			return *this;
		}
	};


	class G_EXPORT GHermiteCurve2D : public GMultiCurve2D {

	private:
		struct GHermiteCallBackData {
			const GHermiteCurve2D* Curve;
			GUInt32 KeyIndex;
			// constructor
			GHermiteCallBackData(const GHermiteCurve2D *_Curve, const GUInt32 _KeyIndex) {
				Curve = _Curve;
				KeyIndex = _KeyIndex;
			}
		};

		// control points
		GDynArray<GHermiteKey> gKeys;

	protected:
		// sort keys
		void SortKeys();
		// given a parameter value, it returns the index such as parameter in between keys [index, index+1)
		GBool ParamToKeyIndex(const GReal Param, GUInt32& KeyIndex) const;
		// get max variation (chordal distance) in the range [u0;u1]; here are necessary also
		// curve evaluations at the interval ends
		GReal Variation(const GReal u0, const GReal u1,	const GPoint2& p0, const GPoint2& p1) const;
		// cloning function
		GError BaseClone(const GElement& Source);
		// cut the curve, giving the 2 new set of control points that represents 2 poly-line curves (with the
		// same degree of the original one)
		GError DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const;
		// add a point; if vertex pointer is NULL point must be inserted ON curve
		GError DoAddPoint(const GReal Parameter, const GPoint2 *NewPoint, GUInt32& Index,
						  GBool& AlreadyExists);
		// remove a point(key); index is ensured to be valid and we are sure that after removing we'll have
		// (at least) a minimal (2-keys made) multicurve
		GError DoRemovePoint(const GUInt32 Index);
		// calculate tangents using Catmull-Rom schema
		void CalcCatmullRomTangents(const GUInt32 Index0, const GUInt32 Index1);
		// calculate length of i-th segment (key (i) to key (i+1)), between 2 parameters; it suppose that
		// Index is valid and also parameter range is inside specified segment range
		GReal SegmentLength(const GUInt32 Index, const GReal MinParam, const GReal MaxParam,
							const GReal MaxError) const;
		// calculate point on i-th segment (key (i) to key (i+1)); it suppose that
		// Index is valid and also parameter value is inside segment range
		GPoint2 SegmentEvaluate(const GUInt32 Index, const GReal Parameter) const;
		// calculate derivative on i-th segment (key (i) to key (i+1)); it suppose that
		// Index is valid and also parameter value is inside segment range
		GVector2 SegmentDerivative(const GUInt32 Index, const GDerivativeOrder Order, const GReal Parameter) const;

		GVector2 SegmentTangent(const GUInt32 Index, const GDerivativeOrder Order, const GReal Parameter) const;

		// calculate variation on i-th segment (key (i) to key (i+1)); it suppose that
		// Index is valid and also parameter range is inside specified segment range
		GReal SegmentVariation(const GUInt32 Index, const GReal MinParam, const GReal MaxParam) const;
		// calculate ray intersection of i-th segment (key (i) to key (i+1)); it suppose that
		// Index is valid
		GBool SegmentIntersectRay(const GUInt32 Index, const GRay2& NormalizedRay,
								  GDynArray<GVector2>& Intersections,
								  const GReal Precision, const GInt32 MaxIterations) const;
		// flats (tessellates) i-th segment (key (i) to key (i+1)) specifying a max
		// error/variation (chordal distance); it suppose that Index is valid
		GError SegmentFlatten(const GUInt32 Index, GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
							  const GBool IncludeLastPoint) const;
		// convert the Index-th segment (from key (i) to key (i-1)) into a cubic Bezier form; it
		// suppose that Index is valid
		void SegmentToBezierConversion(const GUInt32 Index, GBezierCurve2D& Result) const;
		// static speed evaluation callback (useful for length evaluation)
		static GReal SegmentSpeedEvaluationCallBack(const GReal u, void *Data);
		// get parameter corresponding to specified point index; index is ensured to be valid
		GError DoGetPointParameter(const GUInt32 Index, GReal& Parameter) const;
		// set parameter corresponding to specified point index; index is ensured to be valid
		GError DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
								   GUInt32& NewIndex, GBool& AlreadyExists);

	public:
		// constructor
		GHermiteCurve2D();
		// constructor
		GHermiteCurve2D(const GElement* Owner);
		// destructor
		~GHermiteCurve2D();
		// clear the curve (remove keys and set an empty knots interval)
		void Clear();
		// returns number of points (of the control polygon)
		inline GInt32 PointsCount() const {
			return (GInt32)gKeys.size();
		}
		// get Index-th control point
		GPoint2 Point(const GInt32 Index) const;
		// set Index-th control point
		GError SetPoint(const GInt32 Index, const GPoint2& NewPoint);
		// set control points
		GError SetPoints(const GDynArray<GPoint2>& NewPoints,
						 const GReal NewMinValue, const GReal NewMaxValue, const GBool Uniform = G_FALSE);
		// get a key
		GError Key(const GUInt32 Index, GHermiteKey& KeyValue) const;
		// get keys array
		const GDynArray<GHermiteKey>& Keys() const {
			return gKeys;
		}
		// set keys
		GError SetKeys(const GDynArray<GHermiteKey>& NewKeys);
		// set a key
		GError SetKey(const GUInt32 Index, const GPoint2& NewKeyValue, const GVector2& InTangent,
					  const GVector2& OutTangent);
		// add a key
		GError AddKey(const GHermiteKey& NewKey, GUInt32& InsertedIndex, GBool& AlreadyExists);

		// set global parameters corresponding to the start point and to the end point
		GError SetDomain(const GReal NewMinValue, const GReal NewMaxValue);
		// intersect the curve with a ray, and returns a list of intersections
		GBool IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
						   const GReal Precision = G_EPSILON, const GInt32 MaxIterations = 100) const;
		// returns the length of the curve between the 2 specified global parameter values
		GReal Length(const GReal u0, const GReal u1, const GReal MaxError = G_EPSILON) const;
		// return the curve value calculated at global parameter u
		GPoint2 Evaluate(const GReal u) const;
		// return the derivate Order-th calculated at global parameter u
		GVector2 Derivative(const GDerivativeOrder Order, const GReal u) const;
		// convert the Index-th segment (from key (i) to key (i-1)) into a cubic Bezier form
		GError SegmentToBezier(const GUInt32 Index, GBezierCurve2D& Result);
		// flats (tessellates) the curve specifying a max error/variation (chordal distance)
		GError Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
					   const GBool IncludeLastPoint = G_TRUE) const;
		// get class ID
		inline const GClassID& ClassID() const {
			return G_HERMITECURVE2D_CLASSID;
		}
		// get class ID of base class
		inline const GClassID& DerivedClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
	};


	// *********************************************************************
	//                        GHermiteCurve2DProxy
	// *********************************************************************
	class G_EXPORT GHermiteCurve2DProxy : public GElementProxy {
	public:
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GHermiteCurve2D(Owner);
		}
		// get class ID
		const GClassID& ClassID() const {
			return G_HERMITECURVE2D_CLASSID;
		}
		// get derived class id
		const GClassID& DerivedClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
	};

	static const GHermiteCurve2DProxy G_HERMITECURVE2D_PROXY;

};	// end namespace Amanith

#endif
