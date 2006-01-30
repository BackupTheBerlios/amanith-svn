/****************************************************************************
** $file: amanith/2d/gpath2d.h   0.3.0.0   edited Jan, 30 2006
**
** 2D Path definition.
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

#ifndef GPATH2D_H
#define GPATH2D_H

#include "amanith/gelement.h"
#include "amanith/2d/gmulticurve2d.h"

/*!
	\file gpath2d.h
	\brief 2D Path header file.
*/
namespace Amanith {


	// *********************************************************************
	//                            GPath2D
	// *********************************************************************

	//! GPath2D static class descriptor.
	static const GClassID G_PATH2D_CLASSID = GClassID("GPath2D", 0x706A40F7, 0xED1747FC, 0x85028ECF, 0xA3557B68);


	/*!
		\class GPath2D
		\brief This class represents a path, intended as a series of "linked" curves of different flavors.

		A parametric path is a parametric 2D shape. The shape is made of curves (GCurve2D classes, so also
		GMultiCurve2D classes are included)) linked together.
		As for curves, a path has a parameter domain. Giving a domain value, this class can provide several informations, like
		derivatives, curvature, length and so on.
		Here's an example that build a closed path using two Bezier traits and one BSpline trait:
\code
	// temporary array of points, used to build curve segments
	GDynArray<GPoint2> pts;

	// Bezier segment, domain [0; 0.2]
	pts.push_back(GPoint2(2, 8));
	pts.push_back(GPoint2(5, 13));
	pts.push_back(GPoint2(8, 11));
	bezCurve.SetPoints(pts);
	bezCurve.SetDomain(0, 0.2);
	path->AppendSegment(bezCurve);

	// another Bezier segment, domain [0.2; 0.5]
	pts.clear();
	pts.push_back(GPoint2(8, 11));
	pts.push_back(GPoint2(11, 15));
	pts.push_back(GPoint2(15, 10));
	pts.push_back(GPoint2(13, 5));
	pts.SetPoints(pts);
	pts.SetDomain(0.2, 0.5);
	path->AppendSegment(bezCurve);
	
	// third degree BSpline segment, domain [0.5; 1]
	pts.clear();
	pts.push_back(GPoint2(13, 5));
	pts.push_back(GPoint2(5, 3));
	pts.push_back(GPoint2(10, 8));
	pts.push_back(GPoint2(8, 3));
	pts.SetPoints(pts, 3, 0.5, 1);
	path->AppendSegment(bsplineCurve);

	// close the path
	path->ClosePath();
\endcode

		As for curves, a path can be flattened using a maximum variation parameter (squared chordal distance). This
		can be useful for an OpenGL rendering.
		Every path point can be moved and changed, furthermore points "on path" can be added too.
		At the design level, GPath2D class has been derived from GMultiCurve2D class because of the non
		differentiable global parameter (at shared points), so calls like DerivativeLR, TangentLR and so on are
		meaningful.	The main differences are:\n

		- AddPoint() method (the one that permit adding points outside domain) is valid if and only if
		first/last path curve segment is a GMultiCurve2D.

		- RemovePoint() method is not valid. This is due to the fact that removing a (control) point of some kind of curves
		(for example B-Spline and Bezier) has no sense in the general case.
		
		- PointParameter() method is not supported. This is due to the fact that for some kind of curves
		(for example B-Spline and Bezier) have not a direct correspondence between control points and domain
		parameter.

		- SetPointParameter() method is not supported for the same reason of PointParameter().
	*/
	class G_EXPORT GPath2D : public GMultiCurve2D {

	private:
		//! The list of curve segments that build this path
		GDynArray<GCurve2D *> gSegments;
		//! Close flag, if G_TRUE the path is closed, else is open.
		GBool gClosed;

		//! Delete and remove all curve segments.
		void DeleteSegments();
		/*!
			Clone segments, given a source array.

			\param Source the source array, used to get curve segments to clone.
			\param StartIndex the first segment index to clone.
			\param EndIndex the last segment index to clone.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError CloneSegments(const GDynArray<GCurve2D *>& Source, const GInt32 StartIndex, const GInt32 EndIndex);

	protected:
		//! Cloning function.
		GError BaseClone(const GElement& Source);
		//! Return the number of "global" points, and curve index relative to the specified "global" index
		GUInt32 PointsCountAndLocate(const GUInt32 GlobalIndex, GUInt32& SegmentIndex,
									GUInt32& LocalIndex, GBool& Shared) const;
		/*!
			Calculate path length for the specified domain sub-interval, with the assumption that
			StartParam <= EndParam.

			\param StartParam the lower bound of interested interval
			\param EndParam the upper bound of interested interval
			\param MaxError the maximum relative error (precision) at witch we wanna calculate length.
		*/
		GReal CalcLength(const GReal StartParam, const GReal EndParam, const GReal MaxError = 2 * G_EPSILON) const;
		//! Push back a curve into internal array, taking care that curve is made of at least 2 points.
		GBool PushBackCurve(GCurve2D *Curve);
		//! Push back a curve into internal array, taking care that curve is made of at least 2 points.
		GBool PushFrontCurve(GCurve2D *Curve);

		// Unsupported by this class; a G_INVALID_OPERATION error code is always returned.
		GError DoGetPointParameter(const GUInt32 Index, GReal& Parameter) const;
		// Unsupported by this class; a G_INVALID_OPERATION error code is always returned.
		GError DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
								   GUInt32& NewIndex, GBool& AlreadyExists);
		// Unsupported by this class; a G_INVALID_OPERATION error code is always returned.
		GError DoRemovePoint(const GUInt32 Index);
		/*!
			Add a new (key)point for the path.

			\param Parameter the domain parameter where to add the (key)point.
			\param NewPoint if not specified (NULL value) the point is created on path. In this case the specified
			domain Parameter must reside completely inside the domain, else an G_OUT_OF_RANGE error is returned.
			If Newpoint is specified (non NULL) the point is created also outside domain range.
			\param Index the position (internal index) occupied by the created (key)point.
			\param AlreadyExists a G_TRUE value means that the point has been created at a domain position already occupied
			by another (key)point. In this case the already existing point is overridden by the created point.
			\return G_NO_ERROR if the function succeeds, an error code otherwise.
		*/
		GError DoAddPoint(const GReal Parameter, const GPoint2 *NewPoint, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Path subdivision.

			Cuts the path at specified parameter, and return left and right path arcs.

			\param u domain parameter specifying where to cut the path.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\note The domain parameter is ensured to be completely inside the path domain. Furthermore RightCurve and
			LeftCurve parameters, if specified, are ensured to be GPath2D classes (so cast is type-safe).
		*/
		GError DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const;
		//! Given a segment index, returns the global point-index of the first point of the specified segment.
		GError FirstPointInSegment(const GUInt32 SegmentIndex, GUInt32& PointIndex) const;
		/*!
			Append a new curve segment at the begin or at the end of this path.

			The path must be open, this operation is not valid for closed paths.
			To make a valid append, the passed curve must satisfy these requirements:\n\n

			- Append to back:
				- Curve's domain lower bound must be equal (under G_EPSILON precision) to the path's domain upper bound.
				- Curve's start point, must be geometrically equal (under G_EPSILON precision) to the path end point.

			- Append to front:
				- Curve's domain upper bound must be equal (under G_EPSILON precision) to the path's domain lower bound.
				- Curve's end point, must be geometrically equal (under G_EPSILON precision) to the path start point.

			\param Curve the curve segment to append. It must be made of at least 2 points, else G_INVALID_PARAMETER will
			be returned.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note if the path is empty, the segment is always added, and the path domain will be set to respect
			the specified segment domain.
		*/
		GError AppendSegment(const GCurve2D& Curve);
		/*!
			Append a path at the begin or at the end of this path.

			Both paths must be open, this operation is not valid for closed paths.
			To make a valid append, the passed path must satisfy these requirements:\n\n

			- Append to back:
				- Path's domain lower bound must be equal (under G_EPSILON precision) to this path's domain upper bound.
				- Path's start point, must be geometrically equal (under G_EPSILON precision) to this path end point.

			- Append to front:
				- Path's domain upper bound must be equal (under G_EPSILON precision) to this path's domain lower bound.
				- Path's end point, must be geometrically equal (under G_EPSILON precision) to this path start point.

			\param Path the path to append. It must be made of at least one curve segment.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note if the path is empty, Path is always added (in this case a physical copy is called).
		*/
		GError AppendPath(const GPath2D& Path);
		/*
			Get max variation (squared chordal distance) in the range [u0; u1]; here are necessary also
			curve evaluations at the interval ends.

			\param u0 lower bound of interested interval.
			\param u1 upper bound of interested interval.
			\param p0 the point corresponding to the curve evaluation at u0.
			\param p1 the point corresponding to the curve evaluation at u1.
			\note The interval is ensured to be completely inside the curve domain.	<b>This method must be implemented
			by every derived classes</b>.
		
		GReal Variation(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1) const;*/

	public:
		//! Default constructor, build an empty path.
		GPath2D();
		//! Constructor with owner (kernel) specification, build an empty path.
		GPath2D(const GElement* Owner);
		//! Destructor, free all curve segments.
		~GPath2D();
		//! Clear the entire path, remove all curve segments and set an empty domain.
		void Clear();
		//! Set path domain.
		GError SetDomain(const GReal NewMinValue, const GReal NewMaxValue);
		/*!
			Intersect the path with a ray, and returns a list of intersections.

			\param NormalizedRay a normalized ray used for intersection test. If the ray has not been
			normalized incorrect results are possible.
			\param Intersections every found intersection will be appended to this array. Each intersection is a 2D
			vector; it has at position 0 the curve parameter (domain) value corresponding to the intersection, and at
			position 1 the ray parameter value corresponding to the intersection.
			\param Precision the precision used to find every solution.
			\param MaxIterations number of max iterations this method can loop for each found solution.
			If the current solution calculus does not reach the specified Precision within MaxIterations iterations, the
			current solution calculus is stopped, the solution is appended into output array, and calculus go for the
			next found solution.
			The more this value is, the more is the accuracy of each solution (and the more is the time taken from this
			method).
		*/
		GBool IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
						   const GReal Precision = G_EPSILON, const GUInt32 MaxIterations = 100) const;
		/*!
			Flats the path specifying a max error/variation (squared chordal distance).

			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
		*/
		GError Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation, const GBool IncludeLastPoint = G_TRUE) const;
		/*!
			Get variation (squared chordal distance) in the current domain range.
		*/
		GReal Variation() const;
		/*! 
			Return the path value calculated at specified domain parameter.

			\param u the domain parameter at witch we wanna evaluate curve value.
			\note if specified domain parameter is out of domain, StartPoint() or EndPoint() are returned (depending of
			witch side the parameter is out).
		*/
		GPoint2 Evaluate(const GReal u) const;
		/*!
			Return the curve derivative calculated at specified domain parameter.

			\param Order the order of derivative.
			\param u the domain parameter at witch we wanna evaluate path derivative.
			\note specified domain parameter is clamped by domain interval.
		*/
		GVector2 Derivative(const GDerivativeOrder Order, const GReal u) const;
		/*!
			Return the path derivative calculated at specified domain parameter. This method differs from
			the one of base GCurve2D class in the number of returned values. This is due to the possibility
			that the path is continuous but not derivable (in the sense that left and right derivatives
			are different).

			\param Order the order of derivative.
			\param u the domain parameter at witch we wanna evaluate path derivative.
			\param LeftDerivative the left derivative.
			\param RightDerivative the right derivative.
			\note specified domain parameter is clamped by domain interval.
		*/
		void DerivativeLR(const GDerivativeOrder Order, const GReal u,
						  GVector2& LeftDerivative, GVector2& RightDerivative) const;
		/*!
			Returns the length of the path between the 2 specified domain parameter values.

			Formally, a length of a differentiable curve segment is the finite integral of first derivative module, over
			the desired interval:

			\f[
				L=\int_{u_0}^{u_1} |{C^.}(t)| dt
			\f]

			This method loops over all interested curve segments, and sums partial results.

			\param u0 the lower bound of interested interval.
			\param u1 the upper bound of interested interval.
			\param MaxError the maximum relative error (precision) at witch we wanna calculate length.
			\return The length of path, calculated in the domain interval [u0; u1].
			\note Before integration, the specified interval is checked and clamped to be be valid for this path domain.
		*/
		GReal Length(const GReal u0, const GReal u1, const GReal MaxError = 2 * G_EPSILON) const;
		//! Returns the total length of the curve. It uses Length() function passing it the whole domain as interval.
		/*inline GReal Length(const GReal MaxError = 2 * G_EPSILON) const {
			return Length(DomainStart(), DomainEnd(), MaxError);
		}*/
		/*!
			Given a global (domain) parameter value, extract its relative relative segment index.

			\param Param the domain parameter we wanna get the segment index; it must be valid for domain range, else
			a G_OUT_OF_RANGE error will be returned.
			\param SegmentIndex the segment index that corresponds to the specified domain parameter value.
			\param PointShared it'll be set to G_TRUE if the specified domain parameter value corresponds
			to a shared path point (a point that belongs to 2 consecutive curve segments).
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError ParamToSegmentIndex(const GReal Param, GUInt32& SegmentIndex, GBool& PointShared) const;
		//! Returns number of "global" points of the path (typically control points).
		GUInt32 PointsCount() const;
		/*!
			Add a point on path.

			This function do not modify the shape of path, it just add a point on the path shape.
			If the specified domain parameter correspond to a shared point, this method do nothing.

			\param Param the domain parameter where to add the point.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		//GError AddPoint(const GReal Param);
		/*!
			Set a path point to the specified new value.

			\param Index the index of point wanna set, must be valid.
			\param NewValue the point's new value.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError SetPoint(const GUInt32 Index, const GPoint2& NewValue);
		/*!
			Get a path point.

			\param Index the index of point wanna get; it must be valid, else a point with infinitive components is returned.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GPoint2 Point(const GUInt32 Index) const;
		/*!
			Append a new curve segment at the begin or at the end of this path. The specified curve can be
			also a GPath2D.

			The path must be open, this operation is not valid for closed paths. If the specified curve is a path,
			then it must be open too, elsewhere the operation is not valid.
			To make a valid append, the passed curve must satisfy these requirements:\n\n

			- Append to back:
				- Curve's domain lower bound must be equal (under G_EPSILON precision) to the path's domain upper bound.
				- Curve's start point, must be geometrically equal (under G_EPSILON precision) to the path end point.

			- Append to front:
				- Curve's domain upper bound must be equal (under G_EPSILON precision) to the path's domain lower bound.
				- Curve's end point, must be geometrically equal (under G_EPSILON precision) to the path start point.

			\param Curve the curve segment to append. It must be made of at least 2 points, else G_INVALID_PARAMETER will
			be returned. This curve can be also a path.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note if the path is empty, the specified curve is always added, and the path domain will be set to respect
			the specified curve domain. Implementation first check if the curve is a GPath2D; if so AppendPath is
			used, else AppendSegment is used.
		*/
		inline GError AppendCurve(const GCurve2D& Curve) {
			if (Curve.IsOfType(G_PATH2D_CLASSID))
				return AppendPath((GPath2D&)Curve);
			else
				return AppendSegment(Curve);
		}
		//! Return the number of curve segments.
		inline GUInt32 SegmentsCount() const {
			return (GUInt32)gSegments.size();
		}
		/*!
			Get the type of Index-th curve segment.
			\param Index the index of segment that we wanna get the type of; it must be valid.
			\param Type the class descriptor of the requested segment.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError SegmentType(const GUInt32 Index, GClassID& Type) const;
		/*!
			Get a copy of a curve segment.

			\param Index the index of segment we wanna get the copy of. It must be valid.
			\param Curve the output segment, that will contain the copy of internal Index-th curve segment.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note implementation calls Curve.CopyFrom() function.
		*/
		GError Segment(const GUInt32 Index, GCurve2D& Curve) const;
		/*!
			Get a pointer to a curve segment maintained by this path. Note that this pointer is
			constant, so you can use only constant methods of the returned segment.

			\param Index the index of segment we wanna get the copy of. It must be valid, else a NULL pointer
			will be returned.
		*/
		const GCurve2D* Segment(const GUInt32 Index) const;
		/*!
			Replace/set a curve segment (already existing inside path) with a new one specified.

			\param Index Index the index of segment we wanna replace/set. It must be valid.
			\param Curve the new curve segment. This curve MUST have the same domain of the existing
			segment, and MUST have the same (geometrically) StartPoint() and EndPoint().
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note Curve can be also a GPath2D class.
		*/
		GError SetSegment(const GUInt32 Index, const GCurve2D& Curve);
		/*!
			Remove a curve segment from this path.

			\param Index he index of segment we wanna delete. It must be valid.
			\param RightPath an optional path that will contain the right "part" of split path. This parameter
			has sense if and only if the original path is open.
			\note If the path is closed RightPath has no sense, and it won't be touched.
		*/
		GError RemoveSegment(const GUInt32 Index, GPath2D *RightPath);
		/*!
			Giving CurvePos = Length(t), this function solves for t = Inverse(Length(s))

			This function uses the Newton method to find the numerical root solution. This method has a quadratic
			convergence.

			\param Result the solution, the domain parameter that correspond to the specified position on path.
			\param PathPos the position along the path track.
			\param MaxError the wanted result precision, it must be a positive value. The less this parameter is, the
			more iteration have to be done by this method.
			\param MaxIterations the number of max iterations. If the specified MaxError has not been reached in
			MaxIterations iterations, this method exits.
			\return G_TRUE if solution has been found (respecting MaxError parameter) before MaxIterations iterations.
			G_FALSE otherwise.
			\note If PathPos is less than 0, the lower bound of domain in returned. If PathPos is greater than
			total curve length, then the upper bound of domain is returned.
		*/
		GBool GlobalParameter(GReal& Result, const GReal PathPos, const GReal MaxError = 2 * G_EPSILON,
							  const GUInt32 MaxIterations = 100) const;
		/*!
			Cuts a path slice corresponding to specified domain interval, and return the path arc.

			If the path is closed, this cut operation is not symmetrical, in the sense that cutting the
			domain subinterval [a; b] is not the same as cutting domain subinterval [b; a].
			If the path is open, this call is equivalent to Cut(u0, u1, OutCurve) call.

			\param u0 lower domain parameter specifying where to begin to cut the path.
			\param u1 upper domain parameter specifying where to end to cut the path.
			\param OutCurve the path arc generated by cutting operation.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note specified domain interval must be a valid interval for this path. The returned path will be
			ensured to be open.
		*/
		GError TwoWaysCut(const GReal u0, const GReal u1, GCurve2D *OutCurve) const;
		//! Return if the path is closed.
		inline GBool IsClosed() const {
			return gClosed;
		}
		//! Return if the path is open.
		inline GBool IsOpen() const {
			return !gClosed;
		}
		/*!
			Close the path, moving an end point to the other one.
		*/
		void ClosePath(const GBool MoveStartPoint = G_FALSE);
		//! Open the path without modifying it.
		inline void OpenPath() {
			gClosed = G_TRUE;
		}
		/*!
			Open the path at a specified domain parameter.

			The behavior is to open the path at specified parameter. Possible cases are:

			- Parameter correspond to StartDomain() or EndDomain(): in this case path will be opened without
			any modifications.

			- Parameter corresponds to an already existing point: in this case path will be opened and only
			internal segments domains will be changed; path domain will remain unchanged.

			- Parameter do not correspond to any already existing point: in this case a point will be added on
			path, and only internal segments domains will be changed; path domain will remain unchanged.
			The path will result to have a one more segment and to be opened.

			Example:
\code
	// temporary array of points, used to build curve segments
	GDynArray<GPoint2> pts;

	// a new brand empty path
	GPath2D path;

	// add a Bezier segment, domain [0.0; 30.0]
	pts.push_back(GPoint2(2, 8));
	pts.push_back(GPoint2(5, 13));
	pts.push_back(GPoint2(8, 11));
	bezCurve.SetPoints(pts);
	bezCurve.SetDomain(0, 30.0);
	path->AppendSegment(bezCurve);
	// close the path
	path->ClosePath();

	// lets open the path at domain parameter 10.0; note that this value do not correspond to any existing point
	gPath.OpenPath(10.0);

	// now the path will have a domain of [0.0; 30.0] and consist of 2 Bezier segment
	GReal dStart = path.DomainStart();  // dStart will be 0.0
	GReal dEnd = path.DomainEnd();  // dEnd will be 30.0
	GUInt32 segsCount = path.SegmentsCount();  // segsCount will be 2
\endcode
			\param Parameter domain parameter where to open this path. It must be inside current domain.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError OpenPath(const GReal Parameter);
		/*!
			Apply an affine transformation to all curve segments.

			\param Matrix a 2x3 matrix, specifying the affine transformation.
			\note the leftmost 2x2 matrix contains the rotation/scale portion, the last column vector contains the
			translation.
		*/
		void XForm(const GMatrix23& Matrix);
		/*!
			Apply full transformation to all curve segments.

			\param Matrix a 3x3 matrix, specifying the transformation.
			\param DoProjection if G_TRUE the projective transformation (described by the last row vector of matrix) will
			be done. In this case all transformed vertexes will be divided by the last W component. If G_FALSE only the
			affine portion will be used for transformation, and no projective division will be executed.
		*/
		void XForm(const GMatrix33& Matrix, const GBool DoProjection = G_TRUE);
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_PATH2D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
	};


	// *********************************************************************
	//                             GPath2DProxy
	// *********************************************************************

	/*!
		\class GPath2DProxy
		\brief This class implements a GPath2D proxy (provider).

		This proxy provides the creation of GPath2D class instances.
	*/
	class G_EXPORT GPath2DProxy : public GElementProxy {
	public:
		//! Creates a new GPath2D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GPath2D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_PATH2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
	};
	//! Static proxy for GPath2D class.
	static const GPath2DProxy G_PATH2D_PROXY;

};	// end namespace Amanith

#endif
