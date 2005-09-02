/****************************************************************************
** $file: amanith/1d/gbsplinecurve1d.h   0.1.0.0   edited Jun 30 08:00
**
** 1D B-Spline curve segment definition.
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

#ifndef GBSPLINECURVE1D_H
#define GBSPLINECURVE1D_H

/*!
	\file gbsplinecurve1d.h
	\brief Header file for 1D BSpline curve class.
*/

#include "amanith/1d/gcurve1d.h"

namespace Amanith {


	// *********************************************************************
	//                           GBSplineCurve1D
	// *********************************************************************

	//! GBSplineCurve1D static class descriptor.
	static const GClassID G_BSPLINECURVE1D_CLASSID = GClassID("GBSplineCurve1D", 0x397B8DCA, 0xB3F3450E, 0x9BF031FE, 0x06873DE8);

	/*!
		\struct GKnotMultiplicity
		\brief A simple pair-like structure that describes a spline knot multiplicity.
	*/
	struct GKnotMultiplicity {
		//! Knot value
		GReal Value;
		//! Knot multiplicity
		GInt32 Multiplicity;
	};

	/*!
		class GBSplineCurve1D
		\brief A generic degree 1D B-spline curve.

		B-spline curves are generalizations of Bezier curves.
		Given (n+1) control points P0, P1, ..., Pn and a knot vector U = { u0, u1, ..., um }, the B-spline curve of degree
		p defined by these control points and knot vector U is:
		\f[
			C(u) = \sum_{i=0}^{n} N_{i,p}(u)P_{i}
		\f]

		where Ni,p(u) are B-spline basis functions of degree p. The i-th B-spline basis function of degree p, written
		as Ni,p(u), is defined recursively as follows:
		\f[
			N_{i,0}(u) = \left\{ \begin{array}{ll}
			1 & \mbox{if $u_i \leq u \leq u_{i+1}$};\\
			0 & \mbox{otherwise}.\end{array} \right.
		\f]
		\f[
			N_{i,p}(u) = \frac{u - u_i}{u_{i+p} - u_i}N_{i,p-1}(u) + \frac{u_{i+p+1} - u}{u_{i+p+1} - u_{i+1}}N_{i+1,p-1}(u)
		\f]
		
		The form of a B-spline curve is very similar to that
		of a Bezier curve. Unlike a Bezier curve, a B-spline curve involves more information, namely: a set of (n+1) control
		points, a knot vector of (m+1) knots, and a degree p. Note that n, m and p must satisfy m = n + p + 1.\n More
		precisely, if we want to define a B-spline curve of degree p with n + 1 control points, we have to supply
		n + p + 2 knots u0, u1, ..., un+p+1. On the other hand, if a knot vector of m + 1 knots and n + 1 control points
		are given, the degree of the B-spline curve is p = m - n - 1. The point on the curve that corresponds to a
		knot ui, C(ui), is referred to as a knot point. Hence, the knot points divide a B-spline curve into curve
		segments, each of which is defined on a knot span. It can be shown  that these curve segments are all Bezier curve
		of degree p.\n
		If the knot vector does not have any particular structure, the generated curve will not touch the first and last legs
		of the control polyline. This type of B-spline curves is called unclamped B-spline curves. We may want to clamp the
		curve so that it is tangent to the first and the last legs at the first and last control points, respectively, as
		a Bezier curve does. To do so, the first knot and the last knot must be of multiplicity p+1. This will generate
		the so-called clamped B-spline curves.\n
		Amanith supports both flavors.
	*/
	class G_EXPORT GBSplineCurve1D : public GCurve1D {

	private:
		//! Curve degree
		GInt32 gDegree;
		//! Opened/closed flag. If G_TRUE this B-spline is clamped (curve touches its end points).
		GBool gOpened;
		//! Uniform / non-uniform flag. It refers to knots vector.
		GBool gUniform;
		//! Knots vector
		GDynArray<GReal> gKnots;
		//! Control points
		GDynArray<GReal> gPoints;
		//! Internal array used for first order forward differences.
		mutable GDynArray<GReal> gForwDiff1;
		//! Internal array used for second order forward differences.
		mutable GDynArray<GReal> gForwDiff2;
		/*!
			This flag signs if (at least) a control point (or a knot) has been modified (so forward differences must be
			recalculated when calling derivatives estimation).
		*/
		mutable GBool gModified;
		//! Internal support array, for basis functions evaluation.
		mutable GDynArray<GReal> gBasisFuncEval;

		/*!
			Build a knots vector.

			\param OutKnots the output generated knots vector
			\param ControlPoints set of control points that will build B-spline. This is necessary for creating
			a non-uniform knots vector using chords parametrization.
			\param MinValue lower bound of knots vector.
			\param MaxValue upper bound of knots vector.
			\param Degree degree of B-spline curve that will be constructed over the returned knots vector. This data is
			important, because together ControlPoints parameter will determine the correct size of output knots vector.
			\param OpenedKnots if G_TRUE, a "clamped" knots vector will be created (the relative B-spline will touch
			the endpoints).
			\param UniformKnots if G_TRUE generated knots will be uniform-spanned across specified domain. Else a
			centripetal (chord-length) schema will be used to generate knots vector.
		*/
		static void BuildKnots(GDynArray<GReal>& OutKnots,
							   const GDynArray<GReal>& ControlPoints,
							   const GReal MinValue, const GReal MaxValue,
							   const GInt32 Degree, const GBool OpenedKnots, const GBool UniformKnots);
		// build a knot array (clamped and non-uniform) with a centripetal schema and knot averaging
		static void BuildCentripetalKnots(GDynArray<GReal>& OutKnots,
										  const GDynArray<GReal>& ControlPoints,
										  const GReal MinValue, const GReal MaxValue, 
										  const GInt32 Degree);
		// build a uniform knots array, open(clamped) or not
		static void BuildUniformKnots(GDynArray<GReal>& OutKnots,
									  const GReal MinValue, const GReal MaxValue,
									  const GInt32 Degree, const GInt32 NumControlPoints,
									  const GBool Clamped);
		//! Calculate first and second order forward differences.
		void BuildForwDiff() const;
		//! Evaluate non-vanishing basis functions for a given span index, and domain parameter.
		GReal* BasisFunctions(const GInt32 SpanIndex, const GInt32 Degree, const GReal u) const;
		/*!
			Evaluate non-vanishing basis functions derivatives (up to Degree) for a given span index
			and domain parameter.
		*/
		GReal* BasisFuncDerivatives(const GInt32 Order, const GInt32 SpanIndex,
									const GInt32 Degree, const GReal u) const;

	protected:
		// set control points
		GError SetPoints(const GDynArray<GReal>& NewPoints, const GDynArray<GReal>& Knots,
						 const GInt32 Degree, const GBool Uniform);
		// build a chord-length parametrized knots array
		static void ChordLengthKnots(GDynArray<GReal>& OutKnots, const GDynArray<GReal>& Points,
									 const GReal MinKnotValue, const GReal MaxKnotValue);
		// knot averaging
		static GError KnotsAveraging(GDynArray<GReal>& OutKnots, const GDynArray<GReal>& ChordalKnots,
									const GInt32 Degree, const GInt32 Offset0, const GInt32 HowManyAveraged);
		//! Cloning function, copies (physically) a Source B-spline curve into this curve.
		GError BaseClone(const GElement& Source);
		/*!
			Curve subdivision.

			Cuts the curve at specified parameter, and return left and right B-spline arcs.

			\param u domain parameter specifying where to cut the curve.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\note The domain parameter is ensured to be completely inside the curve domain. Furthermore RightCurve and
			LeftCurve parameters, if specified, are ensured to be GBSplineCurve1D classes (so cast is type-safe).
		*/
		GError DoCut(const GReal u, GCurve1D *RightCurve, GCurve1D *LeftCurve) const;
		// return true if B-spline is clamped (first and last knots have a multiplicity greater or equal to
		// the degree)
		GBool IsClamped() const;
		//! Resolve banded system for B-spline fitting.
		static GError SolveBandedSystem(const GDynArray<GReal>& BasisMatrix, const GInt32 MatrixSize,
										const GInt32 LeftSemiBandWidth, const GInt32 RightSemiBandWidth,
										GDynArray<GReal>& Rhs);

	public:
		//! Default constructor, creates an empty B-spline.
		GBSplineCurve1D();
		//! Constructor with owner (kernel) parameter, creates an empty B-spline.
		GBSplineCurve1D(const GElement* Owner);
		//! Destructor
		virtual ~GBSplineCurve1D();
		//! Clear the curve (remove control points, free internal structures and set an empty domain).
		void Clear();
		//! Returns number of control points.
		GInt32 PointsCount() const;
		//! Get curve degree.
		GInt32 Degree() const;
		//! Get Index-th control point; Index must be valid, else a point with infinitive components is returned.
		GReal Point(const GInt32 Index) const;
		//! Get control points array.
		inline const GDynArray<GReal>& Points() const {
			return gPoints;
		}
		//! Get knots array.
		inline const GDynArray<GReal>& Knots() const {
			return gKnots;
		}
		//! Calculate a knot multiplicity.
		GInt32 Multiplicity(const GReal u) const;
		//! Find knot span.
		GInt32 FindSpan(const GReal u) const;
		/*!
			Find at the same time knot span and knot multiplicity.

			\note This method is faster than call disjointedly FindSpan() and Multiplicity() functions.
		*/
		GInt32 FindSpanMult(const GReal u, GInt32& Multiplicity) const;
		//! Get knots multiplicities.
		void Multiplicities(GDynArray<GKnotMultiplicity>& Values) const;
		//! Set Index-th control point; Index must be valid.
		GError SetPoint(const GInt32 Index, const GReal NewPoint);
		/*!
			Construct a B-spline curve, specifying control points and degree. Example:
\code
	GDynArray<GReal> pts;

	// the following code create a non-uniform clamped cubic B-spline
	pts.push_back(13);
	pts.push_back(5);
	pts.push_back(10);
	pts.push_back(8);
	bsplineCurve.SetPoints(pts, 3);
\endcode

			\param NewPoints control points array; array must contains at least 2 points, else an G_INVALID_PARAMETER
			error code will be returned.
			\param Degree desired B-spline degree, it must be positive (at least 1) else an G_INVALID_PARAMETER error
			code will be returned.
			\param Opened if G_TRUE, a "clamped" knots vector will be created (the B-spline will touch
			the endpoints).
			\param Uniform if G_TRUE generated knots will be uniform-spanned across current domain. Else a
			centripetal (chord-length) schema will be used to generate knots vector.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note the curve domain will not be changed.
		*/
		GError SetPoints(const GDynArray<GReal>& NewPoints, const GInt32 Degree,
						 const GBool Opened = G_TRUE, const GBool Uniform = G_FALSE);
		/*!
			Construct a B-spline curve, specifying control points, degree and domain. Example:

\code
	GDynArray<GReal> pts;

	// the following code create a uniform clamped cubic B-spline, with a [-0.2; 3.0] domain
	pts.push_back(13);
	pts.push_back(5);
	pts.push_back(10);
	pts.push_back(8);
	bsplineCurve.SetPoints(pts, 3, -0.2, 3.0, G_TRUE, G_TRUE);
\endcode

			\param NewPoints control points array; array must contains at least 2 points, else an G_INVALID_PARAMETER
			error code will be returned.
			\param Degree desired B-spline degree, it must be positive (at least 1) else an G_INVALID_PARAMETER error
			code will be returned.
			\param NewMinValue lower bound of the new domain.
			\param NewMaxValue upper bound of the new domain.
			\param Opened if G_TRUE, a "clamped" knots vector will be created (the B-spline will touch
			the endpoints).
			\param Uniform if G_TRUE generated knots will be uniform-spanned across current domain. Else a
			centripetal (chord-length) schema will be used to generate knots vector.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError SetPoints(const GDynArray<GReal>& NewPoints, const GInt32 Degree,
						 const GReal NewMinValue, const GReal NewMaxValue,
						 const GBool Opened = G_TRUE, const GBool Uniform = G_FALSE);
		/*!
			Set curve domain, and sign this B-spline curve as modified.

			The function rescale and shift all knots, in according to specified new domain.\n\n
			- Lets call: [a; b] the current domain. Let 'u' be a domain parameter value.\n
			- Lets call: [c; d] the new wanted domain. Let 's' be the new domain value corresponding to 'u'.\n
			- Lets set k = (d - c) / (b - a)\n\n

			We know that f(s) = (1 / k) * s - (c / k) + a. This function is called the re-parametrization function.\n
			u = f(s) is well posed, because:\n\n

			- f(c) = a
			- f(d) = b
			- f'(s) > 0 for all s included in [c; d] (it means that f(s) is strictly increasing)

			It results that C(f(s)) is a re-parametrization of C(u). C(s) is geometrically the same curve as C(u), but
			parametrically they are different:\n\n

			- internal point mapping: internal knots becomes \f[ s_{i} = f^{-1}(u_{i}) \f]

			- modifications of derivatives, note that only magnitudes change:
			   - \f[ C'(s) = C'(u) f'(s) = C'(u) \left(\frac{1}{k}\right) \f]
			   - \f[ C''(s) = C'(u) f''(s) + C''(u) \left( f'(s)\right)^2 = C''(u) \left(\frac{1}{k}\right)^2 \f]

			\param NewMinValue the lower bound of the new domain.
			\param NewMaxValue the upper bound of the new domain.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note if NewMinValue is greater than NewMaxValue they will be swapped, so any combination of values is valid.
		*/
		GError SetDomain(const GReal NewMinValue, const GReal NewMaxValue);
		//! Get number of knots, this function is provided for convenience.
		inline GInt32 KnotsCount() const {
			return (GInt32)gKnots.size();
		}
		/*!
			Increases by HowManyTimes the degree of the curve (without changing its shape).

			The steps are:\n\n

            -# extract the i-th Bezier segment from the curve.
			-# degree elevate the i-th Bezier segment.
			-# remove unnecessary knots separating the (i-1)th and i-th segments.

			\param HowManyTimes the degree elevation amount.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note The implemented algorithm is simpler and competitive with that given in "Prautzsch, H., and piper, B., A
			fast algorithm to raise the degree of spline curves, Comput. Aid. Geom. Des., Vol. 8, pp. 253-261, 1991",
			particularly in the case where the degree is to be raised by more than 1.
		*/
		GError HigherDegree(const GInt32 HowManyTimes);
		/*!
			Increases by HowManyTimes the degree of the curve (without changing its shape), and put the result into
			a destination B-spline curve.

			The steps are:\n\n

			-# extract the i-th Bezier segment from the curve.
			-# degree elevate the i-th Bezier segment.
			-# remove unnecessary knots separating the (i-1)th and i-th segments.

			\param HowManyTimes the degree elevation amount.
			\param OutputCurve the output degree-elevated B-spline curve.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note The implemented algorithm is simpler and competitive with that given in "Prautzsch, H., and piper, B., A
			fast algorithm to raise the degree of spline curves, Comput. Aid. Geom. Des., Vol. 8, pp. 253-261, 1991",
			particularly in the case where the degree is to be raised by more than 1.
		*/
		GError HigherDegree(const GInt32 HowManyTimes, GBSplineCurve1D& OutputCurve) const;
		/*!
			Decreases by one the degree of the curve.

			Degree reduction of Bezier curves is relatively well understood, and there exist a number of algorithms.
			Following the strategy developed for degree elevation, we have implemented a 3-steps algorithm for degree
			reduction (taken form The NURBS book):\n\n

			-# extract the i-th Bezier segment from B-spline
			-# degree reduce the i-th Bezier piece
			-# remove unnecessary knots between the (i-1)th and the i-th segments

			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError LowerDegree();
		/*!
			Decreases by one the degree of the curve, and put the result into
			a destination B-spline curve.

			Degree reduction of Bezier curves is relatively well understood, and there exist a number of algorithms.
			Following the strategy developed for degree elevation, we have implemented a 3-steps algorithm for degree
			reduction (taken form The NURBS book):\n\n

			-# extract the i-th Bezier segment from B-spline
			-# degree reduce the i-th Bezier piece
			-# remove unnecessary knots between the (i-1)th and the i-th segments

			\param OutputCurve the output degree-reduced B-spline curve.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError LowerDegree(GBSplineCurve1D& OutputCurve) const;
		/*!
			Knot insertion. This function is provided only for completeness.

			The meaning of knot insertion is adding a new knot into the existing knot vector without changing the shape
			of the curve. This new knot may be equal to an existing knot and, in this case, the multiplicity of that knot
			is increased by one. Because of the fundamental equality m = n + p + 1 (p is the degree, n+1 is the number
			of control points and m is the number of knots less one), after adding a new knot, the value
			of m is increased by one and, consequently, either the number of control points or the degree of the curve
			must also be increased by one.\n
			Changing the degree of the curve due to the increase of knots will change the shape of the curve globally and
			will not be considered. Therefore, inserting a new knot causes a new control point to be added.
			In fact, some existing control points are removed and replaced with new ones by corner cutting.

			\param u the knot value to insert, must be inside curve domain else a G_INVALID_PARAMETER error code will
			be returned.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError InsertKnot(const GReal u);
		/*!
			Evaluate Index-th basic function of specified degree at the given global parameter value.
			This function is provided only for completeness.

			To define B-spline basis functions, we need to specify the degree, p. The i-th B-spline basis function
			of degree p, written as Ni,p(u), is defined recursively as follows:
\f[
N_{i,0}(u) = \left\{ \begin{array}{ll}
1 & \mbox{if $u_i \leq u \leq u_{i+1}$};\\
0 & \mbox{otherwise}.\end{array} \right.
\f]
\f[
N_{i,p}(u) = \frac{u - u_i}{u_{i+p} - u_i}N_{i,p-1}(u) + \frac{u_{i+p+1} - u}{u_{i+p+1} - u_{i+1}}N_{i+1,p-1}(u)
\f]
			\param u Global parameter at which we wanna calculate the basis function value.
			\param Index the index of basis function.
			\param Degree the degree of basis function.
			\return the value of the Index-th basis function of specified degree.
		*/
		GReal EvaluateBasisFunc(const GReal u, const GInt32 Index, const GInt32 Degree) const;
		/*! 
			Return the curve value calculated at specified domain parameter.

			\param u the domain parameter at witch we wanna evaluate curve value.
			\note if specified domain parameter is out of domain, StartPoint() or EndPoint() are returned (depending of
			witch side the parameter is out).
		*/
		GReal Evaluate(const GReal u) const;
		/*!
			Return the curve derivative calculated at specified domain parameter.

			\param Order the order of derivative.
			\param u the domain parameter at witch we wanna evaluate curve derivative.
			\note specified domain parameter is clamped by domain interval.
		*/
		GReal Derivative(const GDerivativeOrder Order, const GReal u) const;
		/*!
			Construct a B-spline that interpolates given points data.

			\param Degree the degree of the interpolating B-spline.
			\param FitPoints the points array, every point will be interpolated by the constructed B-spline.
			\param MinKnotValue lower bound of the interpolating B-spline's domain.
			\param MaxKnotValue upper bound of the interpolating B-spline's domain.
			\return G_NO_ERROR if the operation succeeds, else an error code.
			\note this method is called "global" because a linear (banded) system of equations is set up and solved, in
			opposite to local methods (that are more geometric in nature, and construct the curve segment-wise using only
			local data for each step).
		*/	
		GError GlobalFit(const GInt32 Degree, const GDynArray<GReal>& FitPoints,
						 const GReal MinKnotValue = 0, const GReal MaxKnotValue = 1);
		/*!
			Construct a natural B-spline that interpolates given points data.

			Here the term 'natural' means that generated B-spline will have a null (zero) curvature at its domain
			endpoints.

			\param Degree the degree of the interpolating B-spline.
			\param FitPoints the points array, every point will be interpolated by the constructed B-spline.
			\param MinKnotValue lower bound of the interpolating B-spline's domain.
			\param MaxKnotValue upper bound of the interpolating B-spline's domain.
			\return G_NO_ERROR if the operation succeeds, else an error code.
			\note this method is called "global" because a linear (banded) system of equations is set up and solved, in
			opposite to local methods (that are more geometric in nature, and construct the curve segment-wise using only
			local data for each step).
		*/
		GError GlobalNaturalFit(const GInt32 Degree, const GDynArray<GReal>& FitPoints,
								const GReal MinKnotValue = 0, const GReal MaxKnotValue = 1);
		/*!
			Construct a B-spline that interpolates given points data, specifying also the first order derivatives that
			the curve must assume at its end points.

			\param Degree the degree of the interpolating B-spline.
			\param FitPoints the points array, every point will be interpolated by the constructed B-spline.
			\param Derivative0 first order derivative corresponding to the lower bound of new domain.
			\param Derivative1 first order derivative corresponding to the upper bound of new domain.
			\param MinKnotValue lower bound of the interpolating B-spline's domain.
			\param MaxKnotValue upper bound of the interpolating B-spline's domain.
			\return G_NO_ERROR if the operation succeeds, else an error code.
			\note this method is called "global" because a linear (banded) system of equations is set up and solved, in
			opposite to local methods (that are more geometric in nature, and construct the curve segment-wise using only
			local data for each step).
		*/
		GError GlobalFit(const GInt32 Degree, const GDynArray<GReal>& FitPoints,
						 const GReal Derivative0, const GReal Derivative1,
						 const GReal MinKnotValue = 0, const GReal MaxKnotValue = 1);
		/*!
			Construct a B-spline that interpolates given points data, specifying also the first order derivatives that
			the curve must assume at each interpolated points.

			\param Degree the degree of the interpolating B-spline.
			\param FitPoints the points array, every point will be interpolated by the constructed B-spline.
			\param Derivatives first order derivatives corresponding to each point. 
			\param MinKnotValue lower bound of the interpolating B-spline's domain.
			\param MaxKnotValue upper bound of the interpolating B-spline's domain.
			\return G_NO_ERROR if the operation succeeds, else an error code.
			\note FitPoints and Derivatives arrays must have the same dimension. This method is called "global" because
			a linear (banded) system of equations is set up and solved, in opposite to local methods (that are more
			geometric in nature, and construct the curve segment-wise using only local data for each step).
		*/
		GError GlobalFit(const GInt32 Degree, const GDynArray<GReal>& FitPoints, const GDynArray<GReal>& Derivatives,
						 const GReal MinKnotValue = 0, const GReal MaxKnotValue = 1);
		//! Get class descriptor.
		inline const GClassID& ClassID() const {
			return G_BSPLINECURVE1D_CLASSID;
		}
		//! Get base class (father class) descriptor.
		inline const GClassID& DerivedClassID() const {
			return G_CURVE1D_CLASSID;
		}
	};


	// *********************************************************************
	//                         GBSplineCurve1DProxy
	// *********************************************************************

	/*!
		\class GBSplineCurve1DProxy
		\brief This class implements a GBSplineCurve1D proxy (provider).

		This proxy provides the creation of GBSplineCurve1D class instances.
	*/
	class G_EXPORT GBSplineCurve1DProxy : public GElementProxy {
	public:
		//! Creates a new GBSplineCurve1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GBSplineCurve1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_BSPLINECURVE1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_CURVE1D_CLASSID;
		}
	};
	//! Static proxy for GBSplineCurve1D class.
	static const GBSplineCurve1DProxy G_BSPLINECURVE1D_PROXY;

};	// end namespace Amanith

#endif
