/****************************************************************************
** $file: amanith/2d/gpath2d.h   0.1.0.0   edited Jun 30 08:00
**
** 2D Path definition.
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

#ifndef GPATH2D_H
#define GPATH2D_H

#include "amanith/gelement.h"
#include "amanith/2d/gcurve2d.h"

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

		A parametric path is a parametric 2D shape. The shape is made of curves (GCurve2D classes) linked together.
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
	*/
	class G_EXPORT GPath2D : public GElement {

	private:
		//! Curve's domain.
		GInterval<GReal> gDomain;
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

	public:
		//! Default constructor, build an empty path.
		GPath2D();
		//! Constructor with owner (kernel) specification, build an empty path.
		GPath2D(const GElement* Owner);
		//! Destructor, free all curve segments.
		~GPath2D();
		//! Clear the entire path, remove all curve segments and set an empty domain.
		void Clear();
		//! Get domain lower bound (it corresponds to the start point).
		inline GReal DomainStart() const {
			return gDomain.Start();
		}
		//! Get domain upper bound (it corresponds to the end point).
		inline GReal DomainEnd() const {
			return gDomain.End();
		}
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
						   const GReal Precision = G_EPSILON, const GInt32 MaxIterations = 100) const;
		/*!
			Flats the path specifying a max error/variation (squared chordal distance).

			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
		*/
		GError Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation, const GBool IncludeLastPoint = G_TRUE) const;
		/*! 
			Return the path value calculated at specified domain parameter.

			\param Param the domain parameter at witch we wanna evaluate curve value.
			\note if specified domain parameter is out of domain, StartPoint() or EndPoint() are returned (depending of
			witch side the parameter is out).
		*/
		GPoint2 Evaluate(const GReal Param) const;
		/*!
			Return the path derivative calculated at specified domain parameter.

			\param Param the domain parameter at witch we wanna evaluate path derivative.
			\param Order the order of derivative.
			\param Right if the specified domain parameter correspond to the begin of a curve segment, the point
			is shared between 2 curve segments. If G_TRUE, this Right parameter specifies that must be returned the
			derivative of the "out-coming" curve segment; a G_FALSE value, instead, specifies that must be returned the
			derivative of the "in-coming" curve segment.
			\note specified domain parameter is clamped by domain interval.
		*/
		GVector2 Derivative(const GDerivativeOrder Order, const GReal Param, const GBool Right = G_TRUE) const;
		/*!
			Returns the length of the path between the 2 specified domain parameter values.

			Formally, a length of a differentiable curve segment is the finite integral of first derivative module, over
			the desired interval:

			\f[
				L=\int_{u_0}^{u_1} |{C^.}(t)| dt
			\f]

			This method loops over all interested curve segments, and sums partial results.

			\param StartParam the lower bound of interested interval.
			\param EndParam the upper bound of interested interval.
			\param MaxError the maximum relative error (precision) at witch we wanna calculate length.
			\return The length of path, calculated in the domain interval [StartParam; EndParam].
			\note Before integration, the specified interval is checked and clamped to be be valid for this path domain.
		*/
		GReal Length(const GReal StartParam, const GReal EndParam, const GReal MaxError = 2 * G_EPSILON) const;
		//! Returns the total length of the curve. It uses Length() function passing it the whole domain as interval.
		inline GReal Length(const GReal MaxError = 2 * G_EPSILON) const {
			return Length(DomainStart(), DomainEnd(), MaxError);
		}
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
		GError AddPoint(const GReal Param);
		/*!
			Set a path point to the specified new value.

			\param Index the index of point wanna set, must be valid.
			\param NewValue the point's new value.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError SetPoint(const GUInt32 Index, const GPoint2& NewValue);
		/*!
			Get a path point.

			\param Index the index of point wanna get, must be valid.
			\param WantedPoint the point's value.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError Point(const GUInt32 Index, GPoint2& WantedPoint) const;
		/*!
			Append a new curve segments at the begin or at the end of this path.

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
		GError SetSegment(const GUInt32 Index, GCurve2D& Curve);
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
			Cuts (subdivides) the path at specified domain parameter, and return left and right paths.

			\param Param domain parameter specifying where to cut the path, must be valid.
			\param RightPath if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftPath if non-NULL, the function must return the left arc generated by cutting operation.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note if specified domain parameter is out of the domain, an G_OUT_OF_RANGE error code is returned. If the
			original path is closed, returned paths will be open.
		*/
		GError Cut(const GReal Param, GPath2D *RightPath, GPath2D *LeftPath);
		/*!
			Cuts a path slice corresponding to specified domain interval, and return the path arc.

			If the path is closed, this cut operation is not symmetrical, in the sense that cutting the
			domain subinterval [a; b] is not the same as cutting domain subinterval [b; a].

			\param StartParam lower domain parameter specifying where to begin to cut the path.
			\param EndParam upper domain parameter specifying where to end to cut the path.
			\param OutPath the path arc generated by cutting operation.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note specified domain interval must be a valid interval for this path. If the original path
			is closed, the return path will be open.
		*/
		GError Cut(const GReal StartParam, const GReal EndParam, GPath2D *OutPath);
		/*!
			Cuts (subdivides) the path at specified path (length) parameter, and return left and right paths.

			\param PathPos the path parameter specifying where to cut the curve.
			\param RightPath if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftPath if non-NULL, the function must return the left arc generated by cutting operation.
			\param MaxError the precision to use to do the inverse mapping between path position and domain value.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note if specified path parameter is out of the length-domain, an G_OUT_OF_RANGE error code is returned.
		*/
		GError CutByLength(const GReal PathPos, GPath2D *RightPath, GPath2D *LeftPath,
						   const GReal MaxError = 2 * G_EPSILON);
		/*!
			Cuts a path slice corresponding to specified path (length) interval, and return the path arc.

			\param PathPos0 lower curve (length) parameter specifying where to begin to cut the path.
			\param PathPos1 upper curve (length) parameter specifying where to end to cut the path.
			\param OutPath the path arc generated by cutting operation.
			\param MaxError the precision to use to do the inverse mapping between curve position and domain value.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError CutByLength(const GReal PathPos0, const GReal PathPos1, GPath2D *OutPath,
						  const GReal MaxError = 2 * G_EPSILON);
		/*!
			Get tangent vector, specifying domain parameter.

			The returned value is the normalized (unit length) tangent vector.

			\param Param the domain parameter, where we wanna know tangent vector.
			\param Right if the specified domain parameter correspond to the begin of a curve segment, the point
			is shared between 2 curve segments. If G_TRUE, this Right parameter specifies that must be returned the
			tangent of the "out-coming" curve segment; a G_FALSE value, instead, specifies that must be returned the
			tangent of the "in-coming" curve segment.
		*/
		GVector2 Tangent(const GReal Param, const GBool Right = G_TRUE) const;
		/*!
			Get normal vector, specifying domain parameter.

			The returned value is a normalized vector perpendicular to the path Tangent().

			\param Param the domain parameter, where we wanna know normal vector.
			\param Right if the specified domain parameter correspond to the begin of a curve segment, the point
			is shared between 2 curve segments. If G_TRUE, this Right parameter specifies that must be returned the
			normal of the "out-coming" curve segment; a G_FALSE value, instead, specifies that must be returned the
			normal of the "in-coming" curve segment.
		*/
		GVector2 Normal(const GReal Param, const GBool Right = G_TRUE) const;
		/*!
			Get path curvature, specifying domain parameter.

			For detailed information about curvature, please check this
			site http://planetmath.org/encyclopedia/CurvatureOfACurve.html

			\param Param the domain parameter, where we wanna know curvature value.
			\param Right if the specified domain parameter correspond to the begin of a curve segment, the point
			is shared between 2 curve segments. If G_TRUE, this Right parameter specifies that must be returned the
			curvature of the "out-coming" curve segment; a G_FALSE value, instead, specifies that must be returned the
			curvature of the "in-coming" curve segment.
		*/
		GReal Curvature(const GReal Param, const GBool Right = G_TRUE) const;
		/*!
			Get path speed, specifying domain parameter.

			With 'speed', here's intended the length of the path's first derivative vector.
			\param Param the domain parameter, where we wanna know curvature value.
			\param Right if the specified domain parameter correspond to the begin of a curve segment, the point
			is shared between 2 curve segments. If G_TRUE, this Right parameter specifies that must be returned the
			speed of the "out-coming" curve segment; a G_FALSE value, instead, specifies that must be returned the
			speed of the "in-coming" curve segment.
		*/
		GReal Speed(const GReal Param, const GBool Right = G_TRUE) const;
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
		// open the path without modifying it
		inline void OpenPath() {
			gClosed = G_TRUE;
		}
		//! Get start point of path; this is the point corresponding to the domain lower bound.
		GPoint2 StartPoint() const;
		//! Get end point of path; this is the point corresponding to the domain upper bound.
		GPoint2 EndPoint() const;
		//! Set start point of path; this is the point corresponding to the domain lower bound.
		void SetStartPoint(const GPoint2& NewValue);
		//! Set end point of path; this is the point corresponding to the domain upper bound.
		void SetEndPoint(const GPoint2& NewValue);
		/*!
			Translates all curve segments, points by the specified vector offset.

			\param Translation the offset vector, added to each curve segment.
		*/
		void Translate(const GVector2& Translation);
		/*!
			Rotate all curve segments around a pivot point.

			\param Pivot the pivot point (the center of rotation)
			\param RadAmount the rotation (in radians) amount.
		*/
		void Rotate(const GPoint2& Pivot, const GReal RadAmount);
		/*!
			Scale all curve segments around a pivot point.

			\param Pivot the pivot point (the center of scaling)
			\param XScaleAmount the scale factor used for X axis
			\param YScaleAmount the scale factor used for Y axis
		*/
		void Scale(const GPoint2& Pivot, const GReal XScaleAmount, const GReal YScaleAmount);
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
			return G_ELEMENT_CLASSID;
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
		//! Creates a new GBezierCurve2D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GPath2D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_PATH2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};

	static const GPath2DProxy G_PATH2D_PROXY;

};	// end namespace Amanith

#endif
