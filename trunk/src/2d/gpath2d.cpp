/****************************************************************************
** $file: amanith/src/2d/gpath2d.cpp   0.1.0.0   edited Jun 30 08:00
**
** 2D Path implementation.
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

#include "amanith/2d/gpath2d.h"
#include "amanith/2d/gmulticurve2d.h"
#include "amanith/gerror.h"

/*!
	\file gpath2d.cpp
	\brief 2D Path implementation file.
*/

namespace Amanith {


// *********************************************************************
//                             GPath2D
// *********************************************************************

// constructor
GPath2D::GPath2D() : GElement(), gDomain(G_MIN_REAL, G_MIN_REAL) {

	gClosed = G_FALSE;
}
// constructor
GPath2D::GPath2D(const GElement* Owner) : GElement(Owner), gDomain(G_MIN_REAL, G_MIN_REAL) {

	gClosed = G_FALSE;
}

// constructor
GPath2D::~GPath2D() {

	Clear();
}

// clear the entire path
void GPath2D::Clear() {

	//DeleteSubPaths();
	DeleteSegments();
	gDomain.Set(G_MIN_REAL, G_MIN_REAL);
	gClosed = G_FALSE;
}

// delete and remove curve segments
void GPath2D::DeleteSegments() {

	GUInt32 i, j = (GUInt32)gSegments.size();
	GCurve2D *seg;

	for (i = 0; i < j; i++) {
		seg = gSegments[i];
		G_ASSERT(seg != NULL);
		delete seg;
	}
	gSegments.clear();
}

// clone segments, given a source array
GError GPath2D::CloneSegments(const GDynArray<GCurve2D *>& Source,
							  const GInt32 StartIndex, const GInt32 EndIndex) {

	GInt32 i, i0, i1, j;
	GCurve2D *seg, *newSeg;
	GError err;

	i0 = GMath::Max(StartIndex, 0);
	i1 = GMath::Min(EndIndex, (GInt32)Source.size() - 1);

	for (i = i0; i <= i1; i++) {
		seg = Source[i];
		G_ASSERT(seg != NULL);
		G_ASSERT(seg->PointsCount() > 1);
		// create a new segments
		newSeg = (GCurve2D *)CreateNew(seg->ClassID());
		// at this point we have to check, just to be sure that a kernel exist
		if (newSeg) {
			// do copy
			err = newSeg->CopyFrom(*seg);
			if (err == G_NO_ERROR)
				// push this path into internal array
				PushBackCurve(newSeg);
			else {
				delete newSeg;
				// rollback: delete all appended segments (free memory and remove from internal gSegments array)
				for (j = i - 1; j >= i0; j--) {
					seg = gSegments.back();
					delete seg;
					gSegments.pop_back();
				}
				return err;
			}
		}
		else {
			// rollback: delete all appended segments (free memory and remove from internal gSegments array)
			for (j = i; j >= i0; j--) {
				seg = gSegments.back();
				delete seg;
				gSegments.pop_back();
			}
			return G_UNSUPPORTED_CLASSID;
		}
	}
	return G_NO_ERROR;
}

// cloning function
GError GPath2D::BaseClone(const GElement& Source) {

	const GPath2D& k = (const GPath2D&)Source;
	GError err;

	// copy segments
	DeleteSegments();
	err = CloneSegments(k.gSegments, 0, (GInt32)k.gSegments.size() - 1);
	if (err == G_NO_ERROR) {
		// copy "closed" flag
		gClosed = k.gClosed;
		// copy knots interval
		gDomain = k.gDomain;
		// GAnimElement cloning
		return GElement::BaseClone(Source);
	}
	else
		return err;
}

// push back a curve into internal array, watching that curve is made of at least 2 points
GBool GPath2D::PushBackCurve(GCurve2D *Curve) {

	if (Curve && Curve->PointsCount() > 1) {
		gSegments.push_back(Curve);
		return G_TRUE;
	}
	return G_FALSE;
}

// push back a curve into internal array, watching that curve is made of at least 2 points
GBool GPath2D::PushFrontCurve(GCurve2D *Curve) {

	if (Curve && Curve->PointsCount() > 1) {
		gSegments.insert(gSegments.begin(), Curve);
		return G_TRUE;
	}
	return G_FALSE;
}

// set global parameters corresponding to the start point and to the end point
GError GPath2D::SetDomain(const GReal NewMinValue, const GReal NewMaxValue) {

	GInterval<GReal> newInterval(NewMinValue, NewMaxValue);
	GInt32 i, j = (GInt32)gSegments.size();
	GReal lSeg, start, end, ratio;
	GCurve2D *curve;

	if (newInterval.IsEmpty())
		return G_INVALID_PARAMETER;

	ratio = newInterval.Length() / gDomain.Length();
	start = newInterval.Start();
	for (i = 0; i < j - 1; i++) {
		curve = gSegments[i];
		G_ASSERT(curve != NULL);
		lSeg = curve->Domain().Length();
		end = start + (lSeg * ratio);
		curve->SetDomain(start, end);
		start = end;
	}
	// do last segment reparametrization (avoiding imprecision due to rescaling)
	curve = gSegments[i];
	G_ASSERT(curve != NULL);
	curve->SetDomain(start, newInterval.End());
	// now assign internal interval
	gDomain = newInterval;
	return G_NO_ERROR;
}

// given a global parameter value, extract its relative segment index
GError GPath2D::ParamToSegmentIndex(const GReal Param, GUInt32& SegmentIndex, GBool& PointShared) const {

	GUInt32 i, j = (GUInt32)gSegments.size();
	GReal d;

	if (j == 0)
		return G_INVALID_OPERATION;

	if (Param < DomainStart() || Param > DomainEnd())
		return G_OUT_OF_RANGE;

	for (i = 0; i < j; i++) {
		d = GMath::Abs(Param - gSegments[i]->DomainStart());
		if ((Param > gSegments[i]->DomainStart() || d <= G_EPSILON) &&
			(Param < gSegments[i]->DomainEnd())) {
			SegmentIndex = i;
			if (d <= 2 * G_EPSILON) {
				if (i == 0)
					PointShared = gClosed;
				else
					PointShared = G_TRUE;
			}
			else
				PointShared = G_FALSE;
			return G_NO_ERROR;
		}
	}
	if (GMath::Abs(Param - gSegments[j - 1]->DomainEnd()) <= G_EPSILON) {
		if (gClosed)
			SegmentIndex = 0;
		else
			SegmentIndex = j - 1;
		PointShared = gClosed;
	}
	return G_NO_ERROR;
}

// return the number of "global" points
GUInt32 GPath2D::PointsCount() const {

	GUInt32 c = 0, i, j = (GUInt32)gSegments.size();

	for (i = 0; i < j; i++)
		c += (gSegments[i]->PointsCount() - 1);

	if (gClosed)
		return c;
	else {
		if (c == 0)
			return 0;
		else
			return (c + 1);
	}
}

GUInt32 GPath2D::PointsCountAndLocate(const GUInt32 GlobalIndex,
									  GUInt32& SegmentIndex, GUInt32& LocalIndex, GBool& Shared) const {

	GUInt32 c, i, j = (GUInt32)gSegments.size(), ptsCount;
	GUInt32 index;

	ptsCount = PointsCount();
	index = GlobalIndex % ptsCount;

	c = 0;
	i = 0;
	while (c < index && i < j) {
		c += (gSegments[i]->PointsCount() - 1);
		i++;
	}

	if (c == index) {
		if (i == 0) {
			Shared = gClosed;
			SegmentIndex = 0;
			LocalIndex = 0;
		}
		else
		if (i == j) {
			Shared = gClosed;
			if (gClosed) {
				SegmentIndex = 0;
				LocalIndex = 0;
			}
			else {
				SegmentIndex = i - 1;
				LocalIndex = gSegments[SegmentIndex]->PointsCount() - 1;
			}
		}
		else {
			Shared = G_TRUE;
			SegmentIndex = i;
			LocalIndex = 0;
		}
	}
	else {
		Shared = G_FALSE;
		SegmentIndex = i - 1;
		LocalIndex = (GInt32)index - (GInt32)c + (GInt32)(gSegments[SegmentIndex]->PointsCount() - 1);
	}
	return ptsCount;
}

GError GPath2D::SetPoint(const GUInt32 Index, const GPoint2& NewValue) {

	GUInt32 i, segIndex, locIndex;
	GBool shared;
	GError err;

	i = PointsCountAndLocate(Index, segIndex, locIndex, shared);
	// if this path has no point, return error
	if (i == 0)
		return G_INVALID_OPERATION;

	// if point is shared we have to set first point of returned segment, and last point of
	// previous segment
	if (shared) {
		G_ASSERT(locIndex == 0);
		err = gSegments[segIndex]->SetPoint(locIndex, NewValue);
		if (err == G_NO_ERROR) {
			if (segIndex > 0)
				gSegments[segIndex - 1]->SetEndPoint(NewValue);
			else
				gSegments[gSegments.size() - 1]->SetEndPoint(NewValue);
		}
	}
	else
		// if point is not shared just set its new value
		err = gSegments[segIndex]->SetPoint(locIndex, NewValue);
	return err;
}

GError GPath2D::Point(const GUInt32 Index, GPoint2& WantedPoint) const {

	GUInt32 i, segIndex, locIndex;
	GBool shared;

	i = PointsCountAndLocate(Index, segIndex, locIndex, shared);
	// if this path has no point, return error
	if (i == 0)
		return G_INVALID_OPERATION;

	WantedPoint = gSegments[segIndex]->Point(locIndex);
	return G_NO_ERROR;
}

GError GPath2D::AddPoint(const GReal Param) {

	GUInt32 i;
	GError err;
	GBool shared;
	GCurve2D *rightCurve, *leftCurve, *cuttedCurve;
	GMultiCurve2D *multiCurve;

	if (Param < DomainStart() || Param > DomainEnd())
		return G_OUT_OF_RANGE;

	// now we are sure that we are inside knots interval
	err = ParamToSegmentIndex(Param, i, shared);
	if (err != G_NO_ERROR)
		return err;

	if (shared || Param == DomainStart() || Param == DomainEnd()) {
		// just do nothing...
	}
	else {
		// first we must check if the segment is a multicurve; in this case just call AddPoint of the
		// multicurve...
		if (gSegments[i]->IsOfType(G_MULTICURVE2D_CLASSID)) {
			multiCurve = (GMultiCurve2D *)gSegments[i];
			err = multiCurve->AddPoint(Param, i, shared);
			return err;
		}
		// ...else cut the segment
		cuttedCurve = gSegments[i];
		// first try to create the 2 new pieces
		rightCurve = (GCurve2D *)CreateNew(cuttedCurve->ClassID());
		if (!rightCurve)
			return G_UNSUPPORTED_CLASSID;
		leftCurve = (GCurve2D *)CreateNew(cuttedCurve->ClassID());
		if (!leftCurve) {
			delete rightCurve;
			return G_UNSUPPORTED_CLASSID;
		}
		// cut identified segment
		err = cuttedCurve->Cut(Param, rightCurve, leftCurve);
		if (err == G_NO_ERROR) {
			// curve will be replaced, in order, by leftCurve and rightCurve
			delete cuttedCurve;
			gSegments[i] = leftCurve;
			GDynArray<GCurve2D *>::iterator it = gSegments.begin();
			it += (i + 1);
			gSegments.insert(it, rightCurve);
		}
		else {
			// if something was gone wrong with cutting operation, we have to free memory allocated
			// for cut pieces
			delete rightCurve;
			delete leftCurve;
		}
	}
	return err;
}

GError GPath2D::SegmentType(const GUInt32 Index, GClassID& Type) const {

	GUInt32 i = (GUInt32)gSegments.size();

	if (i == 0)
		return G_INVALID_OPERATION;
	Type = gSegments[Index % i]->ClassID();
	return G_NO_ERROR;
}

GError GPath2D::AppendSegment(const GCurve2D& Curve) {

	GCurve2D *newCurve;
	GError err;

	if (gClosed)
		return G_INVALID_OPERATION;

	if (Curve.PointsCount() < 2)
		return G_INVALID_PARAMETER;

	// when path is empty, appending operation is always valid
	if (gSegments.size() == 0) {
		// append the curve at the end
		newCurve = (GCurve2D *)CreateNew(Curve.ClassID());
		if (!newCurve)
			return G_UNSUPPORTED_CLASSID;
		err = newCurve->CopyFrom(Curve);
		if (err != G_NO_ERROR) {
			delete newCurve;
			return err;
		}
		// now we can insert the new curve at the end of internal array
		if (!PushBackCurve(newCurve)) {
			delete newCurve;
			return G_INVALID_PARAMETER;
		}
		// update path knots interval
		gDomain = newCurve->Domain();
		return G_NO_ERROR;
	}

	if ((Curve.DomainEnd() < DomainStart()) ||
		(GMath::Abs(Curve.DomainEnd() - DomainStart()) <= 2 * G_EPSILON)) {
		// check if last point is equal (under a machine precision) to the first point of current curve
		if (LengthSquared(StartPoint() - Curve.EndPoint()) > 4 * G_EPSILON * G_EPSILON)
			return G_INVALID_PARAMETER;
		// append the curve at the beginning
		newCurve = (GCurve2D *)CreateNew(Curve.ClassID());
		if (!newCurve)
			return G_UNSUPPORTED_CLASSID;
		err = newCurve->CopyFrom(Curve);
		if (err != G_NO_ERROR) {
			delete newCurve;
			return err;
		}
		err = newCurve->SetDomain(DomainStart() - Curve.Domain().Length(), DomainStart());
		if (err != G_NO_ERROR) {
			delete newCurve;
			return err;
		}
		// now we can insert the new curve at beginning of internal array
		if (!PushFrontCurve(newCurve)) {
			delete newCurve;
			return G_INVALID_PARAMETER;
		}
		// update path start global parameter value
		gDomain.SetStart(newCurve->Domain().Start());
		return G_NO_ERROR;
	}
	else
	if ((Curve.DomainStart() > DomainEnd()) ||
		(GMath::Abs(Curve.DomainStart() - DomainEnd()) <= 2 * G_EPSILON)) {
		// check if start point is equal (under a machine precision) to the end point of current curve
		if (LengthSquared(EndPoint() - Curve.StartPoint()) > 4 * G_EPSILON * G_EPSILON)
			return G_INVALID_PARAMETER;
		// append the curve at the end
		newCurve = (GCurve2D *)CreateNew(Curve.ClassID());
		if (!newCurve)
			return G_UNSUPPORTED_CLASSID;
		err = newCurve->CopyFrom(Curve);
		if (err != G_NO_ERROR) {
			delete newCurve;
			return err;
		}
		err = newCurve->SetDomain(DomainEnd(), DomainEnd() + Curve.Domain().Length());
		if (err != G_NO_ERROR) {
			delete newCurve;
			return err;
		}
		// now we can insert the new curve at the end of internal array
		if (!PushBackCurve(newCurve)) {
			delete newCurve;
			return G_INVALID_PARAMETER;
		}
		// update path end global parameter value
		gDomain.SetEnd(newCurve->Domain().End());
		return G_NO_ERROR;
	}
	else
		// we can't add a curve segment in the middle of the path
		return G_INVALID_OPERATION;
}

GError GPath2D::AppendPath(const GPath2D& Path) {

	GInt32 i, j = (GInt32)Path.gSegments.size(), k;
	GError err;
	GReal d;
	GCurve2D *tmpCurve;
	GInterval<GReal> tmpInterval(gDomain);  // for rollback purpose

	if (gClosed || Path.gClosed)
		return G_INVALID_OPERATION;

	if (j == 0)
		return G_NO_ERROR;

	// if this path is empty, append operation is always valid (and in this case append = copy)
	if (gSegments.size() == 0) {
		return BaseClone(Path);
	}

	d = GMath::Abs(Path.DomainEnd() - DomainStart());
	if (Path.DomainEnd() < DomainStart() || d <= 2 * G_EPSILON) {
		// check if last point is equal (under a machine precision) to the first point of current path
		if (LengthSquared(StartPoint() - Path.EndPoint()) > 4 * G_EPSILON * G_EPSILON)
			return G_INVALID_PARAMETER;

		err = G_NO_ERROR;
		for (i = j - 1; (i >= 0) && (err == G_NO_ERROR); i--) {
			G_ASSERT(Path.gSegments[i] != NULL);
			err = AppendSegment(*Path.gSegments[i]);
			// rollback
			if (err != G_NO_ERROR) {
				// delete all previous appended segments
				for (k = 0; k < (j - 1) - i; k++) {
					tmpCurve = gSegments.front();
					G_ASSERT(tmpCurve != NULL);
					delete tmpCurve;
					gSegments.erase(gSegments.begin());
				}
				// restore knots interval
				gDomain = tmpInterval;
			}
		}
		return err;
	}
	else {
		d = GMath::Abs(Path.DomainStart() - DomainEnd());
		if (Path.DomainStart() > DomainEnd() || d <= 2 * G_EPSILON) {
			// check if start point is equal (under a machine precision) to the end point of current path
			if (LengthSquared(EndPoint() - Path.StartPoint()) > 4 * G_EPSILON * G_EPSILON)
				return G_INVALID_PARAMETER;

			err = G_NO_ERROR;
			for (i = 0; (i < j) && (err == G_NO_ERROR); i++) {
				G_ASSERT(Path.gSegments[i] != NULL);
				err = AppendSegment(*Path.gSegments[i]);
				// rollback
				if (err != G_NO_ERROR) {
					// delete all previous appended segments
					for (k = i; k >= 0; k--) {
						tmpCurve = gSegments.back();
						G_ASSERT(tmpCurve != NULL);
						delete tmpCurve;
						gSegments.pop_back();
					}
					// restore knots interval
					gDomain = tmpInterval;
				}
			}
			return err;
		}
		else
			// we can't add a curve segment in the middle of the path
			return G_INVALID_OPERATION;
	}
}

// translate
void GPath2D::Translate(const GVector2& Translation) {

	GUInt32 i, j = (GUInt32)gSegments.size();

	for (i = 0; i < j; i++)
		gSegments[i]->Translate(Translation);
}

// rotate
void GPath2D::Rotate(const GPoint2& Pivot, const GReal RadAmount) {

	GUInt32 i, j = (GUInt32)gSegments.size();

	for (i = 0; i < j; i++)
		gSegments[i]->Rotate(Pivot, RadAmount);
}

// scale
void GPath2D::Scale(const GPoint2& Pivot, const GReal XScaleAmount, const GReal YScaleAmount) {

	GUInt32 i, j = (GUInt32)gSegments.size();

	for (i = 0; i < j; i++)
		gSegments[i]->Scale(Pivot, XScaleAmount, YScaleAmount);
}

// transform
void GPath2D::XForm(const GMatrix23& Matrix) {

	GUInt32 i, j = (GUInt32)gSegments.size();

	for (i = 0; i < j; i++)
		gSegments[i]->XForm(Matrix);
}

// transform
void GPath2D::XForm(const GMatrix33& Matrix, const GBool DoProjection) {

	GUInt32 i, j = (GUInt32)gSegments.size();

	for (i = 0; i < j; i++)
		gSegments[i]->XForm(Matrix, DoProjection);
}

GError GPath2D::Segment(const GUInt32 Index, GCurve2D& Curve) const {

	GUInt32 i = (GUInt32)gSegments.size();

	if (i == 0)
		return G_INVALID_OPERATION;
	// copy Index-thm segment into output curve
	return Curve.CopyFrom(*gSegments[Index % i]);
}

GError GPath2D::SetSegment(const GUInt32 Index, GCurve2D& Curve) {

	#define PRECISION 4 * G_EPSILON * G_EPSILON
	GUInt32 i = (GUInt32)gSegments.size();
	GCurve2D *newCurve, *existingCurve;
	GError err;

	if (i == 0)
		return G_INVALID_OPERATION;
	if (Curve.PointsCount() < 2)
		return G_INVALID_PARAMETER;

	existingCurve = gSegments[Index % i];
	// first check if end-points are geometrically identical under the specified precision
	if (LengthSquared(Curve.StartPoint() - existingCurve->StartPoint()) > PRECISION)
		return G_INVALID_PARAMETER;
	if (LengthSquared(Curve.EndPoint() - existingCurve->EndPoint()) > PRECISION)
		return G_INVALID_PARAMETER;

	// create a new curve of the same type
	newCurve = (GCurve2D *)CreateNew(Curve.ClassID());
	if (!newCurve)
		return G_UNSUPPORTED_CLASSID;
	// copy the source curve
	err = newCurve->CopyFrom(Curve);
	if (err != G_NO_ERROR) {
		delete newCurve;
		return err;
	}
	// to avoid numeric instabilities, force end-points
	newCurve->SetStartPoint(existingCurve->StartPoint());
	newCurve->SetEndPoint(existingCurve->EndPoint());
	// reparametrize knots interval to match existingCurve's one
	err = newCurve->SetDomain(existingCurve->DomainStart(), existingCurve->DomainEnd());
	if (err == G_NO_ERROR) {
		// now replace existing curve with the new one
		gSegments[Index % i] = newCurve;
		// delete "old" curve
		delete existingCurve;
		return G_NO_ERROR;
	}
	delete newCurve;
	return err;
}

GError GPath2D::RemoveSegment(const GUInt32 Index, GPath2D *RightPath) {

	GInt32 i = (GInt32)gSegments.size(), j, k;
	GError err;

	if (i == 0)
		return G_INVALID_OPERATION;

	k = (GInt32)Index % i;
	if (gClosed) {
		// open path at gSegments[k]->DomainEnd()
		err = Cut(gSegments[k]->DomainEnd(), (GPath2D *)NULL, (GPath2D *)NULL);
		if (err == G_NO_ERROR) {
			// after cut, the segment that we wanna remove has become the last one (into internal gSegments array)
			GCurve2D *curve = gSegments[i - 1];
			gDomain.SetEnd(curve->DomainStart());
			gSegments.pop_back();
			delete curve;
		}
		return err;
	}
	else {
		if (RightPath) {
			// RightPath is composed by remaining right segments
			RightPath->Clear();
			// copy all foregoing segments
			err = RightPath->CloneSegments(gSegments, k + 1, i - 1);
			if (err != G_NO_ERROR)
				return err;
			RightPath->gDomain.Set(gSegments[k + 1]->DomainStart(), DomainEnd());
		}
		// now remove all foregoing segments from this path
		gDomain.SetEnd(gSegments[k]->DomainStart());
		for (j = i - 1; j >= k; j--) {
			GCurve2D *curve = gSegments[j];
			G_ASSERT(curve != NULL);
			delete curve;
			gSegments.pop_back();
		}
		return G_NO_ERROR;
	}
}

// cut the path, giving the 2 paths
GError GPath2D::Cut(const GReal Param, GPath2D *RightPath, GPath2D *LeftPath) {

	GInt32 i, j, k;
	GBool shared;
	GError err;
	GCurve2D *cuttedCurve, *rightCurve, *leftCurve;

	err = ParamToSegmentIndex(Param, (GUInt32 &)i, shared);
	if (err != G_NO_ERROR)
		return err;

	if (gClosed) {
		if (shared) {
			GDynArray<GCurve2D *> tmpSegs;
			GReal newMinKnotParam, newMaxKnotParam, l;

			j = (GInt32)gSegments.size();
			newMinKnotParam = Param;
			// first push all foregoing segments
			for (k = i; k < j; k++)
				tmpSegs.push_back(gSegments[k]);
			// for all previous segments we reparametrize their knots and push them into temp array
			newMaxKnotParam = DomainEnd();
			for (k = 0; k <= i - 1; k++) {
				l = gSegments[k]->Domain().Length();
				gSegments[k]->SetDomain(newMaxKnotParam, newMaxKnotParam + l);
				tmpSegs.push_back(gSegments[k]);
				newMaxKnotParam += l;
			}
			// copy temporary segments container into the internal one
			gSegments = tmpSegs;
			// shift internal knots interval
			SetDomain(DomainStart(), DomainEnd());
			// now path is open
			gClosed = G_FALSE;
			return G_NO_ERROR;
		}
		else {
			err = AddPoint(Param);
			if (err == G_NO_ERROR) {
				GDynArray<GCurve2D *> tmpSegs;
				GReal newMinKnotParam, newMaxKnotParam, l;

				j = (GInt32)gSegments.size();
				newMinKnotParam = Param;
				// first push all foregoing segments
				for (k = i + 1; k < j; k++)
					tmpSegs.push_back(gSegments[k]);
				// for all previous segments we reparametrize their knots and push them into temp array
				newMaxKnotParam = DomainEnd();
				for (k = 0; k <= i; k++) {
					l = gSegments[k]->Domain().Length();
					gSegments[k]->SetDomain(newMaxKnotParam, newMaxKnotParam + l);
					tmpSegs.push_back(gSegments[k]);
					newMaxKnotParam += l;
				}
				// copy temporary segments container into the internal one
				gSegments = tmpSegs;
				// shift internal knots interval
				SetDomain(DomainStart(), DomainEnd());
				// now path is open
				gClosed = G_FALSE;
				return G_NO_ERROR;
			}
			else
				return err;
		}
	}
	else {
		if (!LeftPath && !RightPath)
			return G_NO_ERROR;

		if (GMath::Abs(Param - DomainEnd()) <= 2 * G_EPSILON) {
			if (LeftPath)
				LeftPath->CopyFrom(*this);
			if (RightPath)
				RightPath->Clear();
			return G_NO_ERROR;
		}
		else {
			j = (GInt32)gSegments.size();
			if (shared) {
				err = G_NO_ERROR;
				// first empty output paths, then set knots interval and finally copy segments
				if (LeftPath) {
					LeftPath->Clear();
					LeftPath->gDomain.Set(DomainStart(), Param);
					err = LeftPath->CloneSegments(gSegments, 0, i - 1);
				}
				if (RightPath) {
					RightPath->Clear();
					RightPath->gDomain.Set(Param, DomainEnd());
					err = RightPath->CloneSegments(gSegments, i, j - 1);
				}
				return err;
			}
			else {
				cuttedCurve = gSegments[i];
				G_ASSERT(cuttedCurve != NULL);
				rightCurve = leftCurve = NULL;
				// first try to create cut pieces (NB: memory allocation will be done by "destination" path's
				// kernel, just because to be memory consistent and avoid that if "my" kernel will be destroyed
				// also cut pieces will be destroyed)
				if (RightPath) {
					rightCurve = (GCurve2D *)RightPath->CreateNew(cuttedCurve->ClassID());
					if (!rightCurve)
						return G_UNSUPPORTED_CLASSID;
				}
				if (LeftPath) {
					leftCurve = (GCurve2D *)LeftPath->CreateNew(cuttedCurve->ClassID());
					if (!leftCurve) {
						delete rightCurve;
						return G_UNSUPPORTED_CLASSID;
					}
				}

				err = cuttedCurve->Cut(Param, rightCurve, leftCurve);
				if (err == G_NO_ERROR) {
					if (LeftPath) {
						// first empty output path
						LeftPath->Clear();
						// LeftPath will be composed by all previous uncut segments and the left piece
						// of cut segment
						err = LeftPath->CloneSegments(gSegments, 0, i - 1);
						if (err != G_NO_ERROR)
							goto rollBack;
						LeftPath->PushBackCurve(leftCurve);
						// set knots interval
						LeftPath->gDomain.Set(DomainStart(), Param);
						LeftPath->gClosed = G_FALSE;
					}
					if (RightPath) {
						// first empty output path
						RightPath->Clear();
						// RightPath will be composed by the right piece of cut segment plus all foregoing
						// uncut segments
						RightPath->PushBackCurve(rightCurve);
						err = RightPath->CloneSegments(gSegments, i + 1, j - 1);
						if (err != G_NO_ERROR) {
							RightPath->gSegments.pop_back();
							goto rollBack;
						}
						// set knots interval
						RightPath->gDomain.Set(Param, DomainEnd());
						RightPath->gClosed = G_FALSE;
					}
					return G_NO_ERROR;
				}
rollBack:
				// if something was gone wrong with cutting operation, we have to free memory allocated
				// for cut pieces
				delete rightCurve;
				delete leftCurve;
				return err;
			}
		}
	}
}

// cut a slice from the path, giving the path that represents the cut away part
GError GPath2D::Cut(const GReal StartParam, const GReal EndParam, GPath2D *OutPath) {

	GError err;

	if (!OutPath)
		return G_NO_ERROR;

	// ensure that output path is of the same type of source (uncut) path
	if (ClassID() != OutPath->ClassID())
		/*!
			\todo a possible power solution would be to temporary instance a path of the same type,
			then do cutting and if possible convert cut path into wanted type.
		*/
		return G_MISSED_FEATURE;

	if (gClosed) {
		if (!gDomain.IsInInterval(StartParam) || !gDomain.IsInInterval(EndParam))
			return G_INVALID_PARAMETER;

		if (StartParam <= EndParam) {
			// temp path used just for support
			/*GPath2D *tmpPath = (GPath2D *)OutPath->CreateNew(ClassID());
			if (!tmpPath)
				return G_UNSUPPORTED_CLASSID;
			// the trick here is to clone me (only curve segments, we doesn't care about sub-paths)
			// and set "closed" flag to false; then call Cut function again
			err = tmpPath->CloneSegments(gSegments, 0, gSegments.size() - 1);
			if (err == G_NO_ERROR) {
				tmpPath->gClosed = G_FALSE;
				err = tmpPath->Cut(StartParam, EndParam, OutPath);
			}
			// delete temporary path and return error code
			delete tmpPath;*/

			// here is a version without cloning, maybe it's not thread safe now 
			gClosed = G_FALSE;
			err = Cut(StartParam, EndParam, OutPath);
			gClosed = G_TRUE;
			return err;
		}
		else {
			// temp path used just for support
			GPath2D *tmpPath = (GPath2D *)OutPath->CreateNew(ClassID());
			if (!tmpPath)
				return G_UNSUPPORTED_CLASSID;
			// clone me (only curve segments and interval flags, we doesn't care about sub-paths)
			tmpPath->gClosed = G_TRUE;
			tmpPath->gDomain = gDomain;
			err = tmpPath->CloneSegments(gSegments, 0, (GInt32)gSegments.size() - 1);
			if (err == G_NO_ERROR) {
				// just open the tmpPath
				err = tmpPath->Cut(StartParam, (GPath2D *)NULL, (GPath2D *)NULL);
				if (err == G_NO_ERROR)
					err = tmpPath->Cut(EndParam + (DomainEnd() - StartParam), (GPath2D *)NULL, OutPath);
			}
			// delete temporary path and return error code
			delete tmpPath;
			return err;
		}
	}
	else {
		GPath2D *left;
		GInterval<GReal> requestedInterval(StartParam, EndParam);
		requestedInterval &= gDomain;
		if (requestedInterval.IsEmpty())
			return G_INVALID_OPERATION;
		// first try to create cut pieces (NB: memory allocation will be done by "destination" path's
		// kernel, just because to be memory consistent and avoid that if "my" kernel will be destroyed
		// also cut pieces will be destroyed)
		left = (GPath2D *)OutPath->CreateNew(ClassID());
		if (!left)
			return G_UNSUPPORTED_CLASSID;

		err = Cut(requestedInterval.End(), (GPath2D *)NULL, left);
		if (err == G_NO_ERROR)
			err = left->Cut(requestedInterval.Start(), OutPath, (GPath2D *)NULL);
		// path is not closed
		OutPath->gClosed = G_FALSE;
		// free temporary paths
		delete left;
		return err;
	}
}

// giving CurvePos = Length(t), it solve for t = Inverse(Length(s))
GBool GPath2D::GlobalParameter(GReal& Result, const GReal PathPos,
							   const GReal MaxError, const GUInt32 MaxIterations) const {

	GUInt32 i, j;
	GCurve2D *curve;
	GReal len, oldLen;

	if (PathPos <= G_EPSILON) {
		Result = DomainStart();
		return G_TRUE;
	}

	// loops over all segments
	j = (GInt32)gSegments.size();
	i = 0;
	len = 0;
	while (i < j) {
		curve = gSegments[i];
		G_ASSERT(curve != NULL);
		// keep old accumulated length and update the new one
		oldLen = len;
		len += curve->TotalLength(MaxError);
		if (len == PathPos) {
			Result = curve->DomainEnd();
			return G_TRUE;
		}
		else
		if (len > PathPos)
			return curve->GlobalParameter(Result, PathPos - oldLen, MaxError, MaxIterations);
		// next curve segment
		i++;
	}
	Result = DomainEnd();
	return G_TRUE;
}

// cut the path by length
GError GPath2D::CutByLength(const GReal PathPos, GPath2D *RightPath, GPath2D *LeftPath, const GReal MaxError) {

	GReal u;

	if (!RightPath && !LeftPath)
		return G_NO_ERROR;

	// first do inverse mapping
	GlobalParameter(u, PathPos, MaxError);
	// and then cut
	return Cut(u, RightPath, LeftPath);
}

// cut the path by length
GError GPath2D::CutByLength(const GReal PathPos0, const GReal PathPos1, GPath2D *OutPath, const GReal MaxError) {

	GReal u0, u1;

	if (!OutPath)
		return G_NO_ERROR;

	// first do inverse mapping
	GlobalParameter(u0, PathPos0, MaxError);
	GlobalParameter(u1, PathPos1, MaxError);
	// and then cut
	return Cut(u0, u1, OutPath);
}

// intersect the curve with a ray, and returns a list of intersections
GBool GPath2D::IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
							const GReal Precision, const GInt32 MaxIterations) const {

	GUInt32 i, j = (GUInt32)gSegments.size(), k, w;
	GReal lastIntersection, tolerance;
	GBool intFound = G_FALSE;
	GDynArray<GVector2> tmpSolutions;
	GVector2 solution;

	tolerance = GMath::Max(G_EPSILON, Precision);
	lastIntersection = -1;
	k = 0;
	for (i = 0; i < j; i++) {
		intFound |= gSegments[i]->IntersectRay(NormalizedRay, tmpSolutions, tolerance, MaxIterations);
		// check to not push identical solutions
		w = (GUInt32)tmpSolutions.size();
		for (; k < w; k++) {
			if (GMath::Abs(lastIntersection - tmpSolutions[k][0]) > tolerance) {
				Intersections.push_back(tmpSolutions[k]);
				lastIntersection = tmpSolutions[k][0];
			}
		}
	}
	return intFound;
}

// flats (tessellates) the curve specifying a max error/variation (chordal distance)
GError GPath2D::Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
						const GBool IncludeLastPoint) const {

	GInt32 i, j = (GInt32)gSegments.size();
	GError err;

	if (j == 0)
		return G_NO_ERROR;

	for (i = 0; i < j - 1; i++) {
		err = gSegments[i]->Flatten(Contour, MaxDeviation, G_FALSE);
		if (err != G_NO_ERROR)
			return err;
	}
	// if path is closed we are sure that the first and last point are geometrically identical, so
	// it's not needed twice into Contour
	if (gClosed)
		err = gSegments[i]->Flatten(Contour, MaxDeviation, G_FALSE);
	else
		err = gSegments[i]->Flatten(Contour, MaxDeviation, IncludeLastPoint);
	return err;
}

// return the curve value calculated at global parameter u
GPoint2 GPath2D::Evaluate(const GReal Param) const {

	GUInt32 i;
	GReal u;
	GError err;
	GBool shared;

	// clamp parameter into permitted interval
	if (Param < DomainStart())
		u = DomainStart();
	else
	if (Param > DomainEnd())
		u = DomainEnd();
	else
		u = Param;

	err = ParamToSegmentIndex(u, i, shared);
	if (err == G_NO_ERROR)
		return gSegments[i]->Evaluate(u);
	else
		return GPoint2(G_MIN_REAL, G_MIN_REAL);
}

// return the derivate Order-th calculated at global parameter u
GVector2 GPath2D::Derivative(const GDerivativeOrder Order, const GReal Param, const GBool Right) const {

	GUInt32 i;
	GReal u;
	GError err;
	GBool shared;

	// clamp parameter into permitted interval
	if (Param < DomainStart())
		u = DomainStart();
	else
	if (Param > DomainEnd())
		u = DomainEnd();
	else
		u = Param;

	err = ParamToSegmentIndex(u, i, shared);
	if (err != G_NO_ERROR)
		return GVector2(0, 0);

	if (!shared)
		return gSegments[i]->Derivative(Order, u);
	else {
		if (Right)
			return gSegments[i]->Derivative(Order, u);
		else {
			if (i > 0)
				return gSegments[i - 1]->Derivative(Order, u);
			else
				return gSegments[gSegments.size() - 1]->Derivative(Order, u);
		}
	}
}

// get curve tangent (specifying global parameter)
GVector2 GPath2D::Tangent(const GReal Param, const GBool Right) const {

	GUInt32 i;
	GReal u;
	GError err;
	GBool shared;

	// clamp parameter into permitted interval
	if (Param < DomainStart())
		u = DomainStart();
	else
	if (Param > DomainEnd())
		u = DomainEnd();
	else
		u = Param;

	err = ParamToSegmentIndex(u, i, shared);
	if (err != G_NO_ERROR)
		return GVector2(0, 0);

	if (!shared)
		return gSegments[i]->Tangent(u);
	else {
		if (Right)
			return gSegments[i]->Tangent(u);
		else {
			if (i > 0)
				return gSegments[i - 1]->Tangent(u);
			else
				return gSegments[gSegments.size() - 1]->Tangent(u);
		}
	}
}

// get curve normal (specifying global parameter)
GVector2 GPath2D::Normal(const GReal Param, const GBool Right) const {

	GUInt32 i;
	GReal u;
	GError err;
	GBool shared;

	// clamp parameter into permitted interval
	if (Param < DomainStart())
		u = DomainStart();
	else
	if (Param > DomainEnd())
		u = DomainEnd();
	else
		u = Param;

	err = ParamToSegmentIndex(u, i, shared);
	if (err != G_NO_ERROR)
		return GVector2(0, 0);

	if (!shared)
		return gSegments[i]->Normal(u);
	else {
		if (Right)
			return gSegments[i]->Normal(u);
		else {
			if (i > 0)
				return gSegments[i - 1]->Normal(u);
			else
				return gSegments[gSegments.size() - 1]->Normal(u);
		}
	}
}

// get curvature (specifying global parameter)
GReal GPath2D::Curvature(const GReal Param, const GBool Right) const {

	GUInt32 i;
	GReal u;
	GError err;
	GBool shared;

	// clamp parameter into permitted interval
	if (Param < DomainStart())
		u = DomainStart();
	else
	if (Param > DomainEnd())
		u = DomainEnd();
	else
		u = Param;

	err = ParamToSegmentIndex(u, i, shared);
	if (err != G_NO_ERROR)
		return 0;

	if (!shared)
		return gSegments[i]->Curvature(u);
	else {
		if (Right)
			return gSegments[i]->Curvature(u);
		else {
			if (i > 0)
				return gSegments[i - 1]->Curvature(u);
			else
				return gSegments[gSegments.size() - 1]->Curvature(u);
		}
	}
}

// get curve speed (specifying global parameter)
GReal GPath2D::Speed(const GReal Param, const GBool Right) const {

	GUInt32 i;
	GReal u;
	GError err;
	GBool shared;

	// clamp parameter into permitted interval
	if (Param < DomainStart())
		u = DomainStart();
	else
	if (Param > DomainEnd())
		u = DomainEnd();
	else
		u = Param;

	err = ParamToSegmentIndex(u, i, shared);
	if (err != G_NO_ERROR)
		return 0;

	if (!shared)
		return gSegments[i]->Speed(u);
	else {
		if (Right)
			return gSegments[i]->Speed(u);
		else {
			if (i > 0)
				return gSegments[i - 1]->Speed(u);
			else
				return gSegments[gSegments.size() - 1]->Speed(u);
		}
	}
}

void GPath2D::ClosePath(const GBool MoveStartPoint) {

	if (gClosed)
		return;
	if (MoveStartPoint)
		SetStartPoint(EndPoint());
	else
		SetEndPoint(StartPoint());
	gClosed = G_TRUE;
}

// set path start point
void GPath2D::SetStartPoint(const GPoint2& NewValue) {

	GUInt32 i = (GUInt32)gSegments.size();

	if (i == 0)
		return;
	gSegments[0]->SetStartPoint(NewValue);
	// if path is closed last point must be changed also
	if (gClosed)
		gSegments[i - 1]->SetEndPoint(NewValue);
}

// set path end point
void GPath2D::SetEndPoint(const GPoint2& NewValue) {

	GUInt32 i = (GUInt32)gSegments.size();

	if (i == 0)
		return;
	// if path is closed first point must be changed also
	if (gClosed)
		gSegments[0]->SetStartPoint(NewValue);
	gSegments[i - 1]->SetEndPoint(NewValue);
}

// calculate path piece length, with the assumption that StartParam <= EndParam
GReal GPath2D::CalcLength(const GReal StartParam, const GReal EndParam, const GReal MaxError) const {

	G_ASSERT(StartParam <= EndParam);

	GUInt32 i, j;
	GCurve2D *curve;
	GReal len;
	GError err;
	GBool shared;
	GInterval<GReal> requestedInterval(StartParam, EndParam);

	// check if requested interval is not empty
	requestedInterval &= gDomain;
	if (requestedInterval.IsEmpty())
		return 0;
	// find the start segment index
	err = ParamToSegmentIndex(requestedInterval.Start(), i, shared);
	if (err != G_NO_ERROR)
		return 0;
	// loops over interested segments
	j = (GUInt32)gSegments.size();
	len = 0;
	while (i < j) {
		curve = gSegments[i];
		G_ASSERT(curve != NULL);
		if (requestedInterval.End() > curve->DomainEnd()) {
			if (curve->DomainStart() < requestedInterval.Start())
				len += curve->Length(requestedInterval.Start(), curve->DomainEnd(), MaxError);
			else
				len += curve->TotalLength(MaxError);
			// jump to the next segment
			i++;
		}
		// this is the case of the last interested segment, so calculate the remaining curve piece and exit
		else {
			if (curve->DomainStart() < requestedInterval.Start())
				len += curve->Length(requestedInterval.Start(), requestedInterval.End(), MaxError);
			else
				len += curve->Length(curve->DomainStart(), requestedInterval.End(), MaxError);
			break;
		}
	}
	return len;
}

// returns the length of the path curve between the 2 specified global parameter values
GReal GPath2D::Length(const GReal StartParam, const GReal EndParam, const GReal MaxError) const {

	// for a closed path, we must consider a "wrapping" global parameter; so for example, if a path goes
	// from 0.0 to 10.0, and we call Length(8.0, 2.0) we must return Length(0.0, 2.0) + Length(8.0, 10.0)
	if (gClosed) {
		if (gDomain.IsInInterval(StartParam) && gDomain.IsInInterval(EndParam)) {
			if (StartParam <= EndParam)
				return CalcLength(StartParam, EndParam, MaxError);
			else {
				GReal l1 = CalcLength(StartParam, DomainEnd(), MaxError);
				GReal l2 = CalcLength(DomainStart(), EndParam, MaxError);
				return (l1 + l2);
			}
		}
		else
			return 0;
	}
	else {
		if (StartParam <= EndParam)
			return CalcLength(StartParam, EndParam, MaxError);
		else
			return CalcLength(EndParam, StartParam, MaxError);
	}
}

// get curve start point
GPoint2 GPath2D::StartPoint() const {

	if (gSegments.size() == 0)
		return GPoint2(G_MIN_REAL, G_MIN_REAL);
	else
		return gSegments[0]->StartPoint();
}

// get curve end point
GPoint2 GPath2D::EndPoint() const {

	GUInt32 i = (GUInt32)gSegments.size();
	if (i == 0)
		return GPoint2(G_MIN_REAL, G_MIN_REAL);
	else
		return gSegments[i - 1]->EndPoint();
}

};	// end namespace Amanith
