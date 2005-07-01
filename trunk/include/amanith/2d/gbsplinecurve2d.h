/****************************************************************************
** $file: amanith/2d/g2dbsplinecurve.h   0.1.0.0   edited Jun 30 08:00
**
** 2D B-Spline curve segment definition.
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

#ifndef GBSPLINECURVE2D_H
#define GBSPLINECURVE2D_H

/*!
	\file gbsplinecurve2d.h
	\brief Header file for 2D BSpline curve class.
*/

#include "amanith/2d/gcurve2d.h"

namespace Amanith {


	// *********************************************************************
	//                           GBSplineCurve2D
	// *********************************************************************
	static const GClassID G_BSPLINECURVE2D_CLASSID = GClassID("GBSplineCurve2D", 0xA93232A6, 0xDA274032, 0xB4653354, 0xDE375419);

	struct GKnotMultiplicity {
		GReal Value;
		GInt32 Multiplicity;
	};

	class G_EXPORT GBSplineCurve2D : public GCurve2D {

	private:
		// curve degree
		GInt32 gDegree;
		// opened/closed bspline
		GBool gOpened;
		// uniform / non-uniform bspline
		GBool gUniform;
		// knots vector
		GDynArray<GReal> gKnots;
		// control points
		GDynArray<GPoint2> gPoints;
		// first order forward differences
		mutable GDynArray<GPoint2> gForwDiff1;
		// second order forward differences
		mutable GDynArray<GPoint2> gForwDiff2;
		// signs if (at least) a control point has been modified (so forward differences must be recalculated
		// when calling derivatives estimation)
		mutable GBool gModified;
		// support arrays for basis functions evaluation
		mutable GDynArray<GReal> gBasisFuncEval;

		// build the knot array
		static void BuildKnots(GDynArray<GReal>& OutKnots,
							   const GDynArray<GPoint2>& ControlPoints,
							   const GReal MinValue, const GReal MaxValue,
							   const GInt32 Degree, const GBool OpenedKnots, const GBool UniformKnots);
		// build a knot array (clamped and non-uniform) with a centripetal schema and knot averaging
		static void BuildCentripetalKnots(GDynArray<GReal>& OutKnots,
										  const GDynArray<GPoint2>& ControlPoints,
										  const GReal MinValue, const GReal MaxValue, 
										  const GInt32 Degree);
		// build a uniform knots array, open(clamped) or not
		static void BuildUniformKnots(GDynArray<GReal>& OutKnots,
									  const GReal MinValue, const GReal MaxValue,
									  const GInt32 Degree, const GInt32 NumControlPoints,
									  const GBool Clamped);
		// calculate first and second order forward differences
		void BuildForwDiff() const;
		// evaluate non-vanishing basis functions for a given span index, and global parameter
		GReal* BasisFunctions(const GInt32 SpanIndex, const GInt32 Degree, const GReal u) const;
		// evaluate non-vanishing basis functions derivatives (up to Degree) for a given span index
		// and global parameter
		GReal* BasisFuncDerivatives(const GInt32 Order, const GInt32 SpanIndex,
									const GInt32 Degree, const GReal u) const;

	protected:
		// get max variation (chordal distance) in the range [u0;u1]; here are necessary also
		// curve evaluations at the interval ends
		GReal Variation(const GReal u0, const GReal u1,
						const GPoint2& p0, const GPoint2& p1) const;
		// returns number of intersection between control polygon and y axis
		GInt32 CrossingCountX() const;
		// ray intersection based on Newton schema
		GBool IntersectXRay(GDynArray<GVector2>& Intersections,
								  const GReal Precision, const GInt32 MaxIterations) const;
		// set control points
		GError SetPoints(const GDynArray<GPoint2>& NewPoints, const GDynArray<GReal>& Knots,
						 const GInt32 Degree, const GBool Uniform);
		// build a chord-length parametrized knots array
		static void ChordLengthKnots(GDynArray<GReal>& OutKnots, const GDynArray<GPoint2>& Points,
									 const GReal MinKnotValue, const GReal MaxKnotValue);
		// knot averaging
		static GError KnotsAveraging(GDynArray<GReal>& OutKnots, const GDynArray<GReal>& ChordalKnots,
									const GInt32 Degree, const GInt32 Offset0, const GInt32 HowManyAveraged);
		// cloning function
		GError BaseClone(const GElement& Source);
		// cut the curve, giving the 2 new set of control points that represents 2 bspline curve (with the
		// same degree of the original one)
		GError DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const;
		// return true if bspline is clamped (first and last knots have a multiplicity greater or equal to
		// the degree)
		GBool IsClamped() const;
		// resolve banded system for curve fitting
		static GError SolveBandedSystem(const GDynArray<GReal>& BasisMatrix, const GInt32 MatrixSize,
										const GInt32 LeftSemiBandWidth, const GInt32 RightSemiBandWidth,
										GDynArray<GPoint2>& Rhs);

	public:
		// constructor
		GBSplineCurve2D();
		// constructor
		GBSplineCurve2D(const GElement* Owner);
		// destructor
		virtual ~GBSplineCurve2D();
		// clear the curve (remove control points and set an empty knots interval)
		void Clear();
		// returns number of points (of the control polygon)
		GInt32 PointsCount() const;
		// get curve degree
		GInt32 Degree() const;
		// get Index-th control point
		GPoint2 Point(const GInt32 Index) const;
		// get points array
		inline const GDynArray<GPoint2>& Points() const {
			return gPoints;
		}
		// get knots array
		inline const GDynArray<GReal>& Knots() const {
			return gKnots;
		}
		// calculate knot multiplicity
		GInt32 Multiplicity(const GReal u) const;
		// find knot span
		GInt32 FindSpan(const GReal u) const;
		// find at the same time knot span and knot multiplicity
		GInt32 FindSpanMult(const GReal u, GInt32& Multiplicity) const;
		// get knots multiplicities
		void Multiplicities(GDynArray<GKnotMultiplicity>& Values) const;
		// set Index-th control point
		GError SetPoint(const GInt32 Index, const GPoint2& NewPoint);
		// set control points
		GError SetPoints(const GDynArray<GPoint2>& NewPoints, const GInt32 Degree,
						 const GBool Opened = G_TRUE, const GBool Uniform = G_FALSE);
		// set control points specifying also knots range
		GError SetPoints(const GDynArray<GPoint2>& NewPoints, const GInt32 Degree,
						 const GReal NewMinValue, const GReal NewMaxValue,
						 const GBool Opened = G_TRUE, const GBool Uniform = G_FALSE);
		// set global parameters corresponding to the start point and to the end point
		GError SetDomain(const GReal NewMinValue, const GReal NewMaxValue);
		// get number of knots
		inline GInt32 KnotsCount() const {
			return (GInt32)gKnots.size();
		}
		// returns control polygon length
		GReal ControlPolygonLength(const GInt32 FromIndex, const GInt32 ToIndex) const;
		// increases by one the degree of the curve
		GError HigherDegree(const GInt32 HowManyTimes);
		// increases by one the degree of the curve and gives the result out
		GError HigherDegree(const GInt32 HowManyTimes, GBSplineCurve2D& OutputCurve) const;
		// decreases by one the degree of the curve
		GError LowerDegree();
		// decreases by one the degree of the curve and gives the result out
		GError LowerDegree(GBSplineCurve2D& OutputCurve) const;
		// returns number of intersection between control polygon and a normalized ray
		GInt32 CrossingCount(const GRay2& NormalizedRay) const;
		// intersect the curve with a ray, and returns a list of intersections
		GBool IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
						   const GReal Precision = G_EPSILON, const GInt32 MaxIterations = 100) const;
		// knot insertion
		GError InsertKnot(const GReal u);
		// flats (tessellates) the curve specifying a max error/variation (chordal distance)
		GError Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
						const GBool IncludeLastPoint = G_TRUE) const;
		// evaluate Index-th basic function of specified degree at global parameter value u
		GReal EvaluateBasisFunc(const GReal u, const GInt32 Index, const GInt32 Degree) const;
		// return the curve value calculated at global parameter u
		GPoint2 Evaluate(const GReal u) const;
		// return the derivate Order-th calculated at global parameter u
		GVector2 Derivative(const GDerivativeOrder Order, const GReal u) const;
		// curve (global) fitting
		GError GlobalFit(const GInt32 Degree, const GDynArray<GPoint2>& FitPoints,
						 const GReal MinKnotValue = 0, const GReal MaxKnotValue = 1);
		// curve (global) natural (second derivaives = 0) fitting
		GError GlobalNaturalFit(const GInt32 Degree, const GDynArray<GPoint2>& FitPoints,
								const GReal MinKnotValue = 0, const GReal MaxKnotValue = 1);
		// curve (global) fitting with first derivative specified at end points
		GError GlobalFit(const GInt32 Degree, const GDynArray<GPoint2>& FitPoints,
						 const GVector2& Derivative0, const GVector2& Derivative1,
						 const GReal MinKnotValue = 0, const GReal MaxKnotValue = 1);
		// curve (global) fitting with first derivative specified at each point
		GError GlobalFit(const GInt32 Degree, const GDynArray<GPoint2>& FitPoints, const GDynArray<GVector2>& Derivatives,
						 const GReal MinKnotValue = 0, const GReal MaxKnotValue = 1);
		// get class ID
		inline const GClassID& ClassID() const {
			return G_BSPLINECURVE2D_CLASSID;
		}
		// get class ID of base class
		inline const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};


	// *********************************************************************
	//                         GBSplineCurve2DProxy
	// *********************************************************************
	class G_EXPORT GBSplineCurve2DProxy : public GElementProxy {
	public:
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GBSplineCurve2D(Owner);
		}
		// get class ID
		const GClassID& ClassID() const {
			return G_BSPLINECURVE2D_CLASSID;
		}
		// get derived class id
		const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};

	static const GBSplineCurve2DProxy G_BSPLINECURVE2D_PROXY;

};	// end namespace Amanith

#endif
