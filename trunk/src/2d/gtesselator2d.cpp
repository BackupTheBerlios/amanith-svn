/****************************************************************************
** $file: amanith/src/2d/gtesselator2d.cpp   0.3.0.0   edited Jan, 30 2006
**
** 2D Polygonal shape tesselator implementation.
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

#include "amanith/2d/gtesselator2d.h"
#include "amanith/geometry/gintersect.h"
#include "amanith/gerror.h"
#include "amanith/geometry/gvect.h"
#include <algorithm>

/*!
	\file gtesselator2d.cpp
	\brief 2D shape triangulator implementation.
*/

namespace Amanith {

#define UNDEFINED_EDGE		0
#define NORMAL_EDGE			1
#define LEFT_ADDED_EDGE		2
#define RIGHT_ADDED_EDGE	3

#define NO_INTERSECTION 0
#define GOOD_INTERSECTION 1
#define DEGENERATE_INTERSECTION1 2
#define DEGENERATE_INTERSECTION2 4
#define DOUBLE_INTERSECTION 8

#define PointLE_X(P1, P2) ((P1[G_X] < P2[G_X]) || (P1[G_X] == P2[G_X] && P1[G_Y] <= P2[G_Y]))
#define PointLE_Y(P1, P2) ((P1[G_Y] < P2[G_Y]) || (P1[G_Y] == P2[G_Y] && P1[G_X] <= P2[G_X]))

static GDouble gPrecision = 2.2204460492503131e-16;
static GDouble gPrecisionSquared = gPrecision;

// debug stuff
std::FILE *debugFile = NULL;
GBool DebugActivated = G_FALSE;
//#define TESSELATOR_DEBUG_ACTIVATED


GDouble SweepLineDistance(GMeshEdge2D<GDouble> *Edge, GMeshVertex2D<GDouble>* Event) {

	GDouble SweepX = Event->Position()[G_X];
	GPoint<GDouble, 2> a(Edge->Org()->Position());
	GPoint<GDouble, 2> b(Edge->Dest()->Position());
	GDouble deltaLeft, deltaRight;

	deltaLeft = SweepX - a[G_X];
	deltaRight = b[G_X] - SweepX;
	if (b[G_X] > a[G_X]) {
		if (deltaLeft < deltaRight)
			return a[G_Y] - (a[G_Y] - b[G_Y]) * (deltaLeft / (deltaLeft + deltaRight));
		else
			return b[G_Y] - (b[G_Y] - a[G_Y]) * (deltaRight / (deltaLeft + deltaRight));
	}
	// vertical edge
	else
		return GMath::Min(a[G_Y], b[G_Y]);
}

// *********************************************************************
//                          Debug stuff
// *********************************************************************
void DebugOpenFile(std::FILE **File, const GChar8 *FileName) {

	if (DebugActivated && *File == NULL) {
	#if defined(G_OS_WIN) && _MSC_VER >= 1400
		errno_t openErr = fopen_s(File, FileName, "wt");
		if (openErr)
			*File = NULL;
	#else
		*File = std::fopen(FileName, "wt");
	#endif
	}
}

void DebugCloseFile(std::FILE **File) {
	if (DebugActivated && *File != NULL) {
		std::fclose(*File);
		*File = NULL;
	}
}

void DebugWrite(std::FILE *File, const GChar8* Text) {

	if (DebugActivated && File != NULL) {
		fprintf(File, "%s\n", Text);
		fflush(File);
	}
}

void GTesselator2D::DebugDumpDictionary(std::FILE *File, GDictionaryTree& Dictionary, GMeshVertex2D<GDouble> *Event) {

	if (DebugActivated == G_FALSE)
		return;

	GAVLNode *nUpper;
	GMeshEdge2D<GDouble> *upperExtEdge;
	GDouble sweepDist;
	GInt32 crossNumber;
	GMeshToAVL *data;
	GString s;

	s = "Dump dictionary at event " + StrUtils::ToString(Event->Position(), ";", "%5.2f");
	DebugWrite(File, StrUtils::ToAscii(s));

	nUpper = Dictionary.Max();
	crossNumber = 1;
	while (nUpper) {
		upperExtEdge = (GMeshEdge2D<GDouble> *)nUpper->CustomData();
		// update crossing number
		data = (GMeshToAVL *)upperExtEdge->CustomData();
		// edge is into dictionary, so it MUST include a descriptor
		G_ASSERT(data != NULL);
		if (data->EdgeType != RIGHT_ADDED_EDGE) {
			data->CrossingNumber = crossNumber;
			crossNumber++;
		}
		sweepDist = SweepLineDistance(upperExtEdge, Event);
		s = "Sweep dist = " + StrUtils::ToString(sweepDist, "%5.2f") + ", ";
		s += "Org = " + StrUtils::ToString(upperExtEdge->Org()->Position(), ";", "%5.2f") + ", ";
		s += "Dest = " + StrUtils::ToString(upperExtEdge->Dest()->Position(), ";", "%5.2f") + ", ";
		s += "Cros num. = " + StrUtils::ToString(data->CrossingNumber) + ", ";
		if (data->EdgeType == RIGHT_ADDED_EDGE)
			s += "Type = RIGHT DIAGONAL";
		else
			if (data->EdgeType == LEFT_ADDED_EDGE)
				s += "Type = LEFT DIAGONAL";
			else
				if (data->EdgeType == UNDEFINED_EDGE)
					s += "Type = UNDEFINED";
				else
					if (data->EdgeType == NORMAL_EDGE)
						s += "Type = NORMAL";

		if (PointLE_X(upperExtEdge->Dest()->Position(), Event->Position()))
			s += " ********";

		DebugWrite(File, StrUtils::ToAscii(s));
		nUpper = Dictionary.Prev(nUpper);
	}
}

void GTesselator2D::DebugDumpOrgRing(std::FILE *File, GMeshVertex2D<GDouble> *Vertex) {

	if (DebugActivated == G_FALSE)
		return;

	GMeshEdge2D<GDouble> *startEdge, *e;
	GString s;

	e = startEdge = Vertex->Edge();
	s = "Dump origin ring " + StrUtils::ToString(startEdge->Org()->Position(), ";", "%5.2f");
	DebugWrite(File, StrUtils::ToAscii(s));

	do {
		s = "Dest " + StrUtils::ToString(e->Dest()->Position(), ";", "%5.2f");
		DebugWrite(File, StrUtils::ToAscii(s));
		e = e->Onext();
	} while(e != startEdge);
}

// *********************************************************************
//                           GExtVertex
// *********************************************************************

static GInt32 PointCmp(const GPoint<GDouble, 2>& P1, const GPoint<GDouble, 2>& P2) {

	// they are the same point
	if (LengthSquared(P2 - P1) <= gPrecisionSquared)
		return 0;
	// test the x-coord first
	if (P1[G_X] > P2[G_X])
		return 1;
	if (P1[G_X] < P2[G_X])
		return -1;
	// and then test the y-coord second
	if (P1[G_Y] > P2[G_Y])
		return 1;
	return -1;
}

#ifdef TESSELATOR_DEBUG_ACTIVATED
static GInt32 Point_Cmp(const GPoint<GDouble, 2>& P1, const GPoint<GDouble, 2>& P2) {

	if (P1[G_X] < P2[G_X])
		return -1;
	else
	if (P1[G_X] > P2[G_X])
		return 1;
	else {
		if (P1[G_Y] < P2[G_Y])
			return -1;
		else
		if (P1[G_Y] > P2[G_Y])
			return 1;
		else
			return 0;
	}
}
#endif

static GDouble EdgeSignYX(const GPoint<GDouble, 2>& Org, const GPoint<GDouble, 2>& P, const GPoint<GDouble, 2>& Dst) {

	GDouble deltaLeft, deltaRight;
	G_ASSERT(PointLE_Y(Org, P) && PointLE_Y(P, Dst));

	deltaLeft = P[G_Y] - Org[G_Y];
	deltaRight = Dst[G_Y] - P[G_Y];
	if (Dst[G_Y] > Org[G_Y])
		return (P[G_X] - Dst[G_X]) * deltaLeft + (P[G_X] - Org[G_X]) * deltaRight;
	// vertical edge
	return 0;
}

static GDouble EdgeSignXY(const GPoint<GDouble, 2>& Org, const GPoint<GDouble, 2>& P, const GPoint<GDouble, 2>& Dst) {

	GDouble deltaLeft, deltaRight;
	G_ASSERT(PointLE_X(Org, P) && PointLE_X(P, Dst));

	deltaLeft = P[G_X] - Org[G_X];
	deltaRight = Dst[G_X] - P[G_X];
	if (Dst[G_X] > Org[G_X])
		return (P[G_Y] - Dst[G_Y]) * deltaLeft + (P[G_Y] - Org[G_Y]) * deltaRight;
	// vertical edge
	return 0;
}

static GDouble EdgeDistX(const GPoint<GDouble, 2>& Org, const GPoint<GDouble, 2>& P, const GPoint<GDouble, 2>& Dst) {

	GDouble deltaLeft, deltaRight;
	G_ASSERT(PointLE_Y(Org, P) && PointLE_Y(P, Dst));

	deltaLeft = P[G_Y] - Org[G_Y];
	deltaRight = Dst[G_Y] - P[G_Y];

	if (Dst[G_Y] > Org[G_Y]) {
		if (deltaLeft < deltaRight)
			return (P[G_X] - Org[G_X]) + (Org[G_X] - Dst[G_X]) * (deltaLeft / (Dst[G_Y] - Org[G_Y]));
		else
			return (P[G_X] - Dst[G_X]) + (Dst[G_X] - Org[G_X]) * (deltaRight / (Dst[G_Y] - Org[G_Y]));
	}
	// vertical edge
	return 0;
}

static GDouble EdgeDistY(const GPoint<GDouble, 2>& Org, const GPoint<GDouble, 2>& P, const GPoint<GDouble, 2>& Dst) {

	GDouble deltaLeft, deltaRight;
	G_ASSERT(PointLE_X(Org, P) && PointLE_X(P, Dst));

	deltaLeft = P[G_X] - Org[G_X];
	deltaRight = Dst[G_X] - P[G_X];
	if (Dst[G_X] > Org[G_X]) {
		if (deltaLeft < deltaRight)
			return (P[G_Y] - Org[G_Y]) + (Org[G_Y] - Dst[G_Y]) * (deltaLeft / (Dst[G_X] - Org[G_X]));
		else
			return (P[G_Y] - Dst[G_Y]) + (Dst[G_Y] - Org[G_Y]) * (deltaRight / (Dst[G_X] - Org[G_X]));
	}
	// vertical edge
	return 0;
}

static GBool EdgeIntersect(const GPoint<GDouble, 2>& o1, const GPoint<GDouble, 2>& d1, const GPoint<GDouble, 2>& o2,
						   const GPoint<GDouble, 2>& d2, GDouble& Param1, GDouble& Param2) {

	#define Swap(a, b) { \
		GPoint<GDouble, 2> t(a); \
		a = b; \
		b = t; \
	}

	#define DOUBLE_EPSILON 2.2204460492503131e-14

	GDouble z1, z2;
	GPoint<GDouble, 2> oo1(o1);
	GPoint<GDouble, 2> dd1(d1);
	GPoint<GDouble, 2> oo2(o2);
	GPoint<GDouble, 2> dd2(d2);

	if (!PointLE_X(oo1, oo2)) {
		Swap(oo1, oo2);
		Swap(dd1, dd2);
	}

	if (!PointLE_X(oo2, dd1)) {
		// no intersection
		return G_FALSE;
	}

	GDouble area1 = TwiceSignedArea(oo1, dd1, oo2);
	GDouble area2 = TwiceSignedArea(oo1, dd1, dd2);
	GDouble area1Abs = GMath::Abs(area1);
	GDouble area2Abs = GMath::Abs(area2);
	GDouble maxAbs;

	// area normalization, we wanna area1 the less one
	if (area1Abs > area2Abs) {
		GDouble tmpArea = area1;
		area1 = area2 / area1Abs;
		area2 = tmpArea;
		maxAbs = area1Abs;
	}
	else {
		area1 /= area2Abs;
		maxAbs = area2Abs;
	}

	// pure overlapping edges
	if (maxAbs <= DOUBLE_EPSILON) {
		Param1 = oo2[G_X];
		Param2 = oo2[G_Y];
		return G_TRUE;
	}

#ifdef TESSELATOR_DEBUG_ACTIVATED
	GString s;
	if (GMath::Abs(area1) > DOUBLE_EPSILON && GMath::Abs(area1) <= 10 * DOUBLE_EPSILON) {
		s = "Area1 zero suspect: o1 = " + StrUtils::ToString(o1, ";", "%5.2f") + " d1 = " + StrUtils::ToString(d1, ";", "%5.2f");
		s += " o2 = " + StrUtils::ToString(o2, ";", "%5.2f") + " d2 = " + StrUtils::ToString(d2, ";", "%5.2f");
		s += " minArea = " + StrUtils::ToString(area1, "%e");
		DebugWrite(debugFile, StrUtils::ToAscii(s));
	}
#endif

	if ((area1 > DOUBLE_EPSILON && area2 > 0) || (area1 < -DOUBLE_EPSILON && area2 < 0))
		// no intersection
		return G_FALSE;

	area1 = TwiceSignedArea(oo2, dd2, oo1);
	area2 = TwiceSignedArea(oo2, dd2, dd1);
	area1Abs = GMath::Abs(area1);
	area2Abs = GMath::Abs(area2);
	// area normalization, we wanna area1 the less one
	if (area1Abs > area2Abs) {
		GDouble tmpArea = area1;
		area1 = area2 / area1Abs;
		area2 = tmpArea;
	}
	else {
		area1 /= area2Abs;
	}

#ifdef TESSELATOR_DEBUG_ACTIVATED
	if (GMath::Abs(area1) > DOUBLE_EPSILON && GMath::Abs(area1) <= 10 * DOUBLE_EPSILON) {
		s = "Area2 zero suspect: o1 = " + StrUtils::ToString(o1, ";", "%5.2f") + " d1 = " + StrUtils::ToString(d1, ";", "%5.2f");
		s += " o2 = " + StrUtils::ToString(o2, ";", "%5.2f") + " d2 = " + StrUtils::ToString(d2, ";", "%5.2f");
		s += " minArea = " + StrUtils::ToString(area1, "%e");
		DebugWrite(debugFile, StrUtils::ToAscii(s));
	}
#endif

	if ((area1 > DOUBLE_EPSILON && area2 > 0) || (area1 < -DOUBLE_EPSILON && area2 < 0))
		// no intersection
		return G_FALSE;


	if (PointLE_X(dd1, dd2)) {
		// Interpolate between o2 and d1
		z1 = EdgeDistY(oo1, oo2, dd1);
		z2 = EdgeDistY(oo2, dd1, dd2);
		if (z1 + z2 < 0) {
			z1 = -z1;
			z2 = -z2;
		}
		if (z1 == 0 && z2 == 0)
			Param1 = oo2[G_X];
		else {
			if (z1 < 0)
				z1 = 0;
			if (z2 < 0)
				z2 = 0;
			Param1 = GMath::BarycentricConvexSum(z1, oo2[G_X], z2, dd1[G_X]);
		}
	}
	else {
		// Interpolate between o2 and d2
		z1 = EdgeSignXY(oo1, oo2, dd1);
		z2 = -EdgeSignXY(oo1, dd2, dd1);
		if (z1 + z2 < 0) {
			z1 = -z1;
			z2 = -z2;
		}
		if (z1 == 0 && z2 == 0)
			Param1 = oo2[G_X];
		else {
			if (z1 < 0)
				z1 = 0;
			if (z2 < 0)
				z2 = 0;
			Param1 = GMath::BarycentricConvexSum(z1, oo2[G_X], z2, dd2[G_X]);
		}
	}
	
	// Now repeat the process for t
	if (!PointLE_Y(oo1, dd1)) {
		Swap(oo1, dd1);
	}
	if (!PointLE_Y(oo2, dd2)) {
		Swap(oo2, dd2);
	}
	if (!PointLE_Y(oo1, oo2)) {
		Swap(oo1, oo2);
		Swap(dd1, dd2);
	}

	if (!PointLE_Y(oo2, dd1)) {
		// no intersection
		return G_FALSE;
	}
	else
	if (PointLE_Y(dd1, dd2)) {
		// Interpolate between o2 and d1
		z1 = EdgeDistX(oo1, oo2, dd1);
		z2 = EdgeDistX(oo2, dd1, dd2);
		if (z1 + z2 < 0) {
			z1 = -z1;
			z2 = -z2;
		}
		if (z1 == 0 && z2 == 0)
			Param2 = oo2[G_Y];
		else {
			if (z1 < 0)
				z1 = 0;
			if (z2 < 0)
				z2 = 0;
			Param2 = GMath::BarycentricConvexSum(z1, oo2[G_Y], z2, dd1[G_Y]);
		}
	}
	else {
		// Interpolate between o2 and d2
		z1 = EdgeSignYX(oo1, oo2, dd1);
		z2 = -EdgeSignYX(oo1, dd2, dd1);
		if (z1 + z2 < 0) {
			z1 = -z1;
			z2 = -z2;
		}
		if (z1 == 0 && z2 == 0)
			Param2 = oo2[G_Y];
		else {
			if (z1 < 0)
				z1 = 0;
			if (z2 < 0)
				z2 = 0;
			Param2 = GMath::BarycentricConvexSum(z1, oo2[G_Y], z2, dd2[G_Y]);
		}
	}
	return G_TRUE;
	#undef Swap
	#undef DOUBLE_EPSILON
}

bool GTesselator2D::SweepGreater(const GExtVertex* Event1, const GExtVertex* Event2) {

	GInt32 i = PointCmp(Event1->MeshVertex->Position(), Event2->MeshVertex->Position());
	if (i < 0)
		return G_TRUE;
	return G_FALSE;
}

// *********************************************************************
//                          GDictionaryTree
// *********************************************************************

GInt32 GTesselator2D::GDictionaryTree::Compare(void *ItemA, void *ItemB) {

	GMeshVertex2D<GDouble> *event = this->DescPointer->CurrentEvent;
	GMeshEdge2D<GDouble> *e1 = (GMeshEdge2D<GDouble> *)ItemA;
	GMeshEdge2D<GDouble> *e2 = (GMeshEdge2D<GDouble> *)ItemB;
	GDouble t1, t2;

	// debug stuff
#ifdef TESSELATOR_DEBUG_ACTIVATED
	GPoint<GDouble, 2> a, b, c, d;

	a = e1->Org()->Position();
	b = e1->Dest()->Position();
	c = e2->Org()->Position();
	d = e2->Dest()->Position();
#endif

	if (e1->Org() == event) {
		if (e2->Org() == event) {
			// sort them by slope
			if (PointLE_X(e1->Dest()->Position(), e2->Dest()->Position())) {
				if (EdgeSignXY(e2->Org()->Position(), e1->Dest()->Position(), e2->Dest()->Position()) <= 0)
					return -1;
				return 1;
			}
			if (EdgeSignXY(e1->Org()->Position(), e2->Dest()->Position(), e1->Dest()->Position()) >= 0)
				return -1;
			return 1;
		}
		if (EdgeSignXY(e2->Org()->Position(), event->Position(), e2->Dest()->Position()) <= 0)
			return -1;
		return 1;
	}
	if (e2->Org() == event) {
		if (EdgeSignXY(e1->Org()->Position(), event->Position(), e1->Dest()->Position()) >= 0)
			return -1;
		return 1;
	}

	t1 = SweepLineDistance(e1, event);
	t2 = SweepLineDistance(e2, event);
	if (t1 <= t2)
		return -1;
	return 1;
}


// *********************************************************************
//                           GTesselator2D
// *********************************************************************

// constructor
GTesselator2D::GTesselator2D() {
}

// destructor
GTesselator2D::~GTesselator2D() {
}

void GTesselator2D::FreeTessellation(GTessDescriptor& Descriptor) {

	GUInt32 i, j;

	// free memory allocated by tessellation
	j = (GUInt32)Descriptor.ExtVertices.size();
	for (i = 0; i < j; i++) {
		GExtVertex *v = Descriptor.ExtVertices[i];
		delete v;
	}
	j = (GUInt32)Descriptor.ExtEdges.size();
	for (i = 0; i < j; i++) {
		GMeshToAVL *data = Descriptor.ExtEdges[i];
		delete data;
	}
	// clear created regions
	j = (GUInt32)Descriptor.ActiveRegions.size();
	for (i = 0; i < j; i++) {
		GActiveRegion *ar = Descriptor.ActiveRegions[i];
		G_ASSERT(ar != NULL);
		delete ar;
	}
}

GBool GTesselator2D::ValidateInput(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour) {

	GInt32 i, j, k;

	// test input for consistency
	j = (GInt32)PointsPerContour.size();
	if (j == 0)
		return G_FALSE;
	k = 0;
	for (i = 0; i < j; i++)
		k += PointsPerContour[i];
	if (k == 0 || k != (GInt32)Points.size())
		return G_FALSE;
	return G_TRUE;
}

// tessellation routine
GError GTesselator2D::Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
								GDynArray< GPoint<GDouble, 2> >& Triangles, const GFillBehavior FillRule) {

	GExtVertex* extVertex;
	GInt32 i, j, k, w, ofs;
	GBool revisitEvent;
	GActiveRegion *ar;

	// test input for consistency
	if (ValidateInput(Points, PointsPerContour) == G_FALSE)
		return G_INVALID_PARAMETER;

	// create a tessellation descriptor
	GTessDescriptor desc;

	// insert all contours
	ofs = 0;
	j = (GInt32)PointsPerContour.size();
	for (i = 0; i < j; i++) {
		// k = number of points of i-th contour
		k = PointsPerContour[i];
		if (k == 0)
			continue;
		BeginContour((GDouble)Points[ofs][G_X], (GDouble)Points[ofs][G_Y], desc);
		ofs++;
		for (w = 1; w < k; w++) {
			AddContourPoint((GDouble)Points[ofs][G_X], (GDouble)Points[ofs][G_Y], desc);
			ofs++;
		}
		EndContour(desc);
	}
	EndTesselletionData(desc);

	// main loop
	extVertex = desc.PriorityTree.front();
	while (!desc.PriorityTree.empty()) {
		// sweep event
		revisitEvent = SweepEvent(extVertex, desc);
		// next event
		desc.PriorityTree.pop_front();
		if (!desc.PriorityTree.empty())
			extVertex = desc.PriorityTree.front();
	}

	// at the end of process, dictionary must be empty
	G_ASSERT(desc.DictionaryTree.NodesCount() == 0);

	// keep track of last closed region
	if (desc.LastRegion)
		desc.LastRegionEdge = desc.LastRegion->MeshUpperEdge->Sym();
	// remove all backface regions
	k = PurgeRegions(desc.ActiveRegions, G_TRUE, desc);
	// triangulate all monotone regions
	j = (GInt32)desc.ActiveRegions.size();
	for (i = 0; i < j; i++) {
		ar = desc.ActiveRegions[i];
		if (!ar->Valid)
			continue;
		// use specified fill rule
		if (FillRule == G_ANY_RULE)
			TessellateMonotoneRegion(ar, Triangles, desc);
		else {
			if (FillRule == G_ODD_EVEN_RULE) {
				if ((ar->CrossingNumber & 1) != 0)
					TessellateMonotoneRegion(ar, Triangles, desc);
			}
			else {
				if ((ar->CrossingNumber & 1) == 0)
					TessellateMonotoneRegion(ar, Triangles, desc);
			}
		}
	}
	// free memory used for tessellation
	FreeTessellation(desc);
	return G_NO_ERROR;
}

// tessellation routine with bounding box calculus
GError GTesselator2D::Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
								GDynArray< GPoint<GDouble, 2> >& Triangles, GAABox2& BoundingBox, const GFillBehavior FillRule) {

	GExtVertex* extVertex;
	GInt32 i, j, k, w, ofs;
	GBool revisitEvent;
	GActiveRegion *ar;
	GPoint<GDouble, 2> pMin, pMax, p;

	// test input for consistency
	if (ValidateInput(Points, PointsPerContour) == G_FALSE)
		return G_INVALID_PARAMETER;

	// create a tessellation descriptor
	GTessDescriptor desc;

	// insert all contours
	ofs = 0;
	j = (GInt32)PointsPerContour.size();
	for (i = 0; i < j; i++) {
		// k = number of points of i-th contour
		k = PointsPerContour[i];
		if (k == 0)
			continue;
		BeginContour((GDouble)Points[ofs][G_X], (GDouble)Points[ofs][G_Y], desc);
		ofs++;
		for (w = 1; w < k; w++) {
			AddContourPoint((GDouble)Points[ofs][G_X], (GDouble)Points[ofs][G_Y], desc);
			ofs++;
		}
		EndContour(desc);
	}
	EndTesselletionData(desc);

	if (desc.PriorityTree.size() > 0) {
		pMin = desc.PriorityTree.front()->MeshVertex->Position();
		pMax = desc.PriorityTree.back()->MeshVertex->Position();
	}

	// main loop
	extVertex = desc.PriorityTree.front();
	while (!desc.PriorityTree.empty()) {
		// sweep event
		revisitEvent = SweepEvent(extVertex, desc);

		// bounding box calculation
		p = extVertex->MeshVertex->Position();
		if (p[G_Y] < pMin[G_Y])
			pMin[G_Y] = p[G_Y];
		if (p[G_Y] > pMax[G_Y])
			pMax[G_Y] = p[G_Y];

		// next event
		desc.PriorityTree.pop_front();
		if (!desc.PriorityTree.empty())
			extVertex = desc.PriorityTree.front();
	}

	// at the end of process, dictionary must be empty
	G_ASSERT(desc.DictionaryTree.NodesCount() == 0);

	// keep track of last closed region
	if (desc.LastRegion)
		desc.LastRegionEdge = desc.LastRegion->MeshUpperEdge->Sym();
	// remove all backface regions
	k = PurgeRegions(desc.ActiveRegions, G_TRUE, desc);
	// triangulate all monotone regions
	j = (GInt32)desc.ActiveRegions.size();
	for (i = 0; i < j; i++) {
		ar = desc.ActiveRegions[i];
		if (!ar->Valid)
			continue;
		// use specified fill rule
		if (FillRule == G_ANY_RULE)
			TessellateMonotoneRegion(ar, Triangles, desc);
		else {
			if (FillRule == G_ODD_EVEN_RULE) {
				if ((ar->CrossingNumber & 1) != 0)
					TessellateMonotoneRegion(ar, Triangles, desc);
			}
			else {
				if ((ar->CrossingNumber & 1) == 0)
					TessellateMonotoneRegion(ar, Triangles, desc);
			}
		}
	}
	// free memory used for tessellation
	FreeTessellation(desc);
	BoundingBox.SetMinMax(GPoint2((GReal)pMin[G_X], (GReal)pMin[G_Y]), GPoint2((GReal)pMax[G_X], (GReal)pMax[G_Y]));
	return G_NO_ERROR;
}

GError GTesselator2D::Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
								GDynArray< GPoint<GDouble, 2> >& TriangPoints, GDynArray< GULong >& TriangIds,
								const GFillBehavior FillRule) {

	GExtVertex* extVertex;
	GInt32 i, j, k, w, ofs;
	GBool revisitEvent;
	GActiveRegion *ar;

	// test input for consistency
	if (ValidateInput(Points, PointsPerContour) == G_FALSE)
		return G_INVALID_PARAMETER;

	// create a tessellation descriptor
	GTessDescriptor desc;

	// insert all contours
	ofs = 0;
	j = (GInt32)PointsPerContour.size();
	for (i = 0; i < j; i++) {
		// k = number of points of i-th contour
		k = PointsPerContour[i];
		if (k == 0)
			continue;
		BeginContour((GDouble)Points[ofs][G_X], (GDouble)Points[ofs][G_Y], desc);
		ofs++;
		for (w = 1; w < k; w++) {
			AddContourPoint((GDouble)Points[ofs][G_X], (GDouble)Points[ofs][G_Y], desc);
			ofs++;
		}
		EndContour(desc);
	}
	EndTesselletionData(desc);

#ifdef TESSELATOR_DEBUG_ACTIVATED
	DebugOpenFile(&debugFile, "debug.txt");
#endif

	// main loop
	extVertex = desc.PriorityTree.front();
	while (!desc.PriorityTree.empty()) {
		// sweep event
		revisitEvent = SweepEvent(extVertex, desc);

		// dump dictionary
#ifdef TESSELATOR_DEBUG_ACTIVATED
		DebugDumpDictionary(debugFile, desc.DictionaryTree, extVertex->MeshVertex);
		DebugDumpOrgRing(debugFile, extVertex->MeshVertex);
#endif

		// assign vertex ID
		extVertex->MeshVertex->SetCustomData((void *)desc.VertexID);
		TriangPoints.push_back(extVertex->MeshVertex->Position());
		desc.VertexID++;
		// next event
		desc.PriorityTree.pop_front();
		if (!desc.PriorityTree.empty())
			extVertex = desc.PriorityTree.front();
	}

#ifdef TESSELATOR_DEBUG_ACTIVATED
	DebugCloseFile(&debugFile);
#endif

	// at the end of process, dictionary must be empty
	if (desc.DictionaryTree.NodesCount() != 0) {
		G_ASSERT(desc.DictionaryTree.NodesCount() == 0);
	}

	// keep track of last closed region
	if (desc.LastRegion)
		desc.LastRegionEdge = desc.LastRegion->MeshUpperEdge->Sym();
	// remove all backface regions
	k = PurgeRegions(desc.ActiveRegions, G_TRUE, desc);
	// triangulate all monotone regions
	j = (GInt32)desc.ActiveRegions.size();
	for (i = 0; i < j; i++) {
		ar = desc.ActiveRegions[i];
		if (!ar->Valid)
			continue;
		// use specified fill rule
		if (FillRule == G_ANY_RULE)
			TessellateMonotoneRegion(ar, TriangIds, desc);
		else {
			if (FillRule == G_ODD_EVEN_RULE) {
				if ((ar->CrossingNumber & 1) != 0)
					TessellateMonotoneRegion(ar, TriangIds, desc);
			}
			else {
				if ((ar->CrossingNumber & 1) == 0)
					TessellateMonotoneRegion(ar, TriangIds, desc);
			}
		}
	}
	// free memory used for tessellation
	FreeTessellation(desc);
	return G_NO_ERROR;
}

GError GTesselator2D::Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
								GDynArray< GPoint<GDouble, 2> >& TriangPoints, GDynArray< GULong >& TriangIds,
								GAABox2& BoundingBox, const GFillBehavior FillRule) {

	GExtVertex* extVertex;
	GInt32 i, j, k, w, ofs;
	GBool revisitEvent;
	GActiveRegion *ar;
	GPoint<GDouble, 2> pMin, pMax, p;

	// test input for consistency
	if (ValidateInput(Points, PointsPerContour) == G_FALSE)
		return G_INVALID_PARAMETER;

	// create a tessellation descriptor
	GTessDescriptor desc;

	// insert all contours
	ofs = 0;
	j = (GInt32)PointsPerContour.size();
	for (i = 0; i < j; i++) {
		// k = number of points of i-th contour
		k = PointsPerContour[i];
		if (k == 0)
			continue;
		BeginContour((GDouble)Points[ofs][G_X], (GDouble)Points[ofs][G_Y], desc);
		ofs++;
		for (w = 1; w < k; w++) {
			AddContourPoint((GDouble)Points[ofs][G_X], (GDouble)Points[ofs][G_Y], desc);
			ofs++;
		}
		EndContour(desc);
	}
	EndTesselletionData(desc);

	if (desc.PriorityTree.size() > 0) {
		pMin = desc.PriorityTree.front()->MeshVertex->Position();
		pMax = desc.PriorityTree.back()->MeshVertex->Position();
	}

#ifdef TESSELATOR_DEBUG_ACTIVATED
	DebugOpenFile(&debugFile, "debug.txt");
#endif

	// main loop
	extVertex = desc.PriorityTree.front();
	while (!desc.PriorityTree.empty()) {
		// sweep event
		revisitEvent = SweepEvent(extVertex, desc);

		// dump dictionary
#ifdef TESSELATOR_DEBUG_ACTIVATED
		DebugDumpDictionary(debugFile, desc.DictionaryTree, extVertex->MeshVertex);
		DebugDumpOrgRing(debugFile, extVertex->MeshVertex);
#endif

		// bounding box calculation
		p = extVertex->MeshVertex->Position();
		if (p[G_Y] < pMin[G_Y])
			pMin[G_Y] = p[G_Y];
		if (p[G_Y] > pMax[G_Y])
			pMax[G_Y] = p[G_Y];

		// assign vertex ID
		extVertex->MeshVertex->SetCustomData((void *)desc.VertexID);
		TriangPoints.push_back(extVertex->MeshVertex->Position());
		desc.VertexID++;
		// next event
		desc.PriorityTree.pop_front();
		if (!desc.PriorityTree.empty())
			extVertex = desc.PriorityTree.front();
	}

#ifdef TESSELATOR_DEBUG_ACTIVATED
	DebugCloseFile(&debugFile);
#endif

	// at the end of process, dictionary must be empty
	if (desc.DictionaryTree.NodesCount() != 0) {
		G_ASSERT(desc.DictionaryTree.NodesCount() == 0);
	}

	// keep track of last closed region
	if (desc.LastRegion)
		desc.LastRegionEdge = desc.LastRegion->MeshUpperEdge->Sym();
	// remove all backface regions
	k = PurgeRegions(desc.ActiveRegions, G_TRUE, desc);
	// triangulate all monotone regions
	j = (GInt32)desc.ActiveRegions.size();
	for (i = 0; i < j; i++) {
		ar = desc.ActiveRegions[i];
		if (!ar->Valid)
			continue;
		// use specified fill rule
		if (FillRule == G_ANY_RULE)
			TessellateMonotoneRegion(ar, TriangIds, desc);
		else {
			if (FillRule == G_ODD_EVEN_RULE) {
				if ((ar->CrossingNumber & 1) != 0)
					TessellateMonotoneRegion(ar, TriangIds, desc);
			}
			else {
				if ((ar->CrossingNumber & 1) == 0)
					TessellateMonotoneRegion(ar, TriangIds, desc);
			}
		}
	}
	// free memory used for tessellation
	FreeTessellation(desc);
	BoundingBox.SetMinMax(GPoint2((GReal)pMin[G_X], (GReal)pMin[G_Y]), GPoint2((GReal)pMax[G_X], (GReal)pMax[G_Y]));
	return G_NO_ERROR;
}

void GTesselator2D::BeginContour(const GDouble X, const GDouble Y, GTessDescriptor& Descriptor) {

	G_ASSERT(Descriptor.LastEdge == NULL);
	G_ASSERT(Descriptor.StepsDone == 0);
	G_ASSERT(Descriptor.FirstPushedSteps == 0);
	G_ASSERT(Descriptor.PushedCount == 0);
	G_ASSERT(Descriptor.FirstPushedPoints[0] == NULL);
	G_ASSERT(Descriptor.FirstPushedPoints[1] == NULL);
	Descriptor.StepsDone = 1;
	Descriptor.LastPoints[0].Set(X, Y);
}

void GTesselator2D::AddContourPoint(const GDouble X, const GDouble Y, GTessDescriptor& Descriptor) {

	GPoint<GDouble, 2> newPoint(X, Y);
	GDouble area;
	GInt32 ptCmp1, ptCmp2;

	if (Descriptor.StepsDone < 2) {
		ptCmp1 = PointCmp(newPoint, Descriptor.LastPoints[0]);
		if (ptCmp1 != 0) {
			Descriptor.LastPoints[Descriptor.StepsDone] = newPoint;
			Descriptor.StepsDone++;
		}
		return;
	}
	area = GMath::Abs(TwiceSignedArea(Descriptor.LastPoints[0], Descriptor.LastPoints[1], newPoint));
	if (area <= gPrecision) {
		Descriptor.LastPoints[1] = newPoint;
	}
	else {
		ptCmp2 = PointCmp(Descriptor.LastPoints[1], newPoint);
		if (ptCmp2 == 0)
			return;

		if (Descriptor.LastEdge == NULL)
			Descriptor.LastEdge = Descriptor.TargetMesh.AddSubManifold();
		else
			Descriptor.LastEdge = Descriptor.TargetMesh.MakeVertexEdge(Descriptor.LastEdge->Dest(),
																		Descriptor.LastEdge->Left(),
																		Descriptor.LastEdge->Right());
		// count this edge
		Descriptor.PushedCount++;

		GMeshVertex2D<GDouble> *vertex = Descriptor.LastEdge->Dest();
		vertex->SetPosition(Descriptor.LastPoints[0]);
		// trace first two pushed points
		if (Descriptor.FirstPushedSteps < 2) {
			Descriptor.FirstPushedPoints[Descriptor.FirstPushedSteps] = vertex;
			Descriptor.FirstPushedSteps++;
		}
		// update last points queue
		Descriptor.LastPoints[0] = Descriptor.LastPoints[1];
		Descriptor.LastPoints[1] = newPoint;
	}
}

void GTesselator2D::EndContour(GTessDescriptor& Descriptor) {

	GDouble area;
	GMeshVertex2D<GDouble> *vertex;
	GInt32 i;

	// single point contour
	if (Descriptor.FirstPushedPoints[0] == NULL)
		goto cleanMesh;

	// this is the case of a zero-lngth last segment (first and last point of the contour are
	// coincident)
	if (PointCmp(Descriptor.LastPoints[1], Descriptor.FirstPushedPoints[0]->Position()) == 0) {
		// this is the case of a 3 vertex contour (a triangle)
		if (Descriptor.FirstPushedPoints[1] == NULL)
			goto cleanMesh;
		area = GMath::Abs(TwiceSignedArea(Descriptor.LastPoints[0], Descriptor.LastPoints[1],
										  Descriptor.FirstPushedPoints[1]->Position()));
		if (area > gPrecision) {
			if (Descriptor.LastEdge == NULL)
				Descriptor.LastEdge = Descriptor.TargetMesh.AddSubManifold();
			else
				Descriptor.LastEdge = Descriptor.TargetMesh.MakeVertexEdge(Descriptor.LastEdge->Dest(),
																			Descriptor.LastEdge->Left(),
																			Descriptor.LastEdge->Right());
			vertex = Descriptor.LastEdge->Dest();
			vertex->SetPosition(Descriptor.LastPoints[0]);
			// count this edge
			Descriptor.PushedCount++;
		}
		else
			Descriptor.FirstPushedPoints[0]->SetPosition(Descriptor.LastPoints[0]);
		goto cleanMesh;
	}

	area = GMath::Abs(TwiceSignedArea(Descriptor.LastPoints[0], Descriptor.LastPoints[1],
									  Descriptor.FirstPushedPoints[0]->Position()));
	if (area > gPrecision) {
		if (Descriptor.LastEdge == NULL)
			Descriptor.LastEdge = Descriptor.TargetMesh.AddSubManifold();
		else
			Descriptor.LastEdge = Descriptor.TargetMesh.MakeVertexEdge(Descriptor.LastEdge->Dest(),
																		Descriptor.LastEdge->Left(),
																		Descriptor.LastEdge->Right());
		vertex = Descriptor.LastEdge->Dest();
		vertex->SetPosition(Descriptor.LastPoints[0]);
		// count this edge
		Descriptor.PushedCount++;
		// this is the case of a 3 vertex contour (a triangle)
		if (Descriptor.FirstPushedPoints[1] == NULL)
			Descriptor.FirstPushedPoints[1] = vertex;
	}
	else
		Descriptor.LastPoints[1] = Descriptor.LastPoints[0];

	area = GMath::Abs(TwiceSignedArea(Descriptor.LastPoints[1], Descriptor.FirstPushedPoints[0]->Position(),
									  Descriptor.FirstPushedPoints[1]->Position()));
	if (area > gPrecision) {
		if (Descriptor.LastEdge == NULL)
			Descriptor.LastEdge = Descriptor.TargetMesh.AddSubManifold();
		else
			Descriptor.LastEdge = Descriptor.TargetMesh.MakeVertexEdge(Descriptor.LastEdge->Dest(),
																		Descriptor.LastEdge->Left(),
																		Descriptor.LastEdge->Right());
		vertex = Descriptor.LastEdge->Dest();
		vertex->SetPosition(Descriptor.LastPoints[1]);
		// count this edge
		Descriptor.PushedCount++;
	}
	else
		Descriptor.FirstPushedPoints[0]->SetPosition(Descriptor.LastPoints[1]);

cleanMesh:
	// lets see if this contour has at least 3 vertices; if not, we simply delete it
	if (Descriptor.PushedCount < 3 && Descriptor.PushedCount > 0) {

		GMeshEdge2D<GDouble> *edge, *edgeTmp;

		// delete faces
		Descriptor.TargetMesh.RemoveFace(Descriptor.LastEdge->Left());
		Descriptor.TargetMesh.RemoveFace(Descriptor.LastEdge->Right());
		// lets remove vertex and edges
		edge = Descriptor.LastEdge;
		for (i = 0; i < Descriptor.PushedCount; i++) {
			Descriptor.TargetMesh.RemoveVertex(edge->Org());
			edgeTmp = edge->Rprev();
			Descriptor.TargetMesh.RemoveEdge(edge);
			edge = edgeTmp;
		}
	}
	else
	if (Descriptor.PushedCount > 0)
		// keep track of this contour
		Descriptor.MeshContours.push_back(Descriptor.LastEdge);

	Descriptor.LastEdge = NULL;
	Descriptor.StepsDone = 0;
	Descriptor.FirstPushedSteps = 0;
	Descriptor.PushedCount = 0;
	Descriptor.FirstPushedPoints[0] = NULL;
	Descriptor.FirstPushedPoints[1] = NULL;
}

void GTesselator2D::EndTesselletionData(GTessDescriptor& Descriptor) {

	GUInt32 i, j = (GUInt32)Descriptor.MeshContours.size();
	GMeshEdge2D<GDouble> *e, *startEdge;
	GMeshToAVL *customData;

	for (i = 0; i < j; i++) {
		e = startEdge = Descriptor.MeshContours[i];
		do {
			// allocate a new edge-avl descriptor
			customData = new GMeshToAVL;
			customData->EdgeType = UNDEFINED_EDGE;
			customData->CrossingNumber = -99;
			customData->IsIntoDictionary = G_FALSE;
			customData->AVLNode = NULL;
			customData->Region = NULL;
			customData->HasBeenIntoDictionary = G_FALSE;
			e->SetCustomData(customData);
			e->Sym()->SetCustomData(customData);
			Descriptor.ExtEdges.push_back(customData);
			
			InsertEventNoSort(e->Org(), Descriptor);
			e = e->Rprev();
		} while (e != startEdge);
	}
	Descriptor.PriorityTree.sort(SweepGreater);
}

// return true is Edge is left-going from Vertex
GBool GTesselator2D::IsLeftGoing(GMeshEdge2D<GDouble> *Edge, GMeshVertex2D<GDouble> *Vertex) {

	GInt32 ptCmp;

	if ((Edge->Org() != Vertex) && (Edge->Dest() != Vertex))
		return G_FALSE;

	if (Edge->Org() == Vertex)
		ptCmp = PointCmp(Edge->Dest()->Position(), Vertex->Position());
	else
		ptCmp = PointCmp(Edge->Org()->Position(), Vertex->Position());

	if (ptCmp < 0)
		return G_TRUE;
	return G_FALSE;
}

GBool GTesselator2D::IsLeftGoingFast(GMeshEdge2D<GDouble> *Edge, GMeshVertex2D<GDouble> *Vertex) {

	if (Edge->Dest() == Vertex)
		return G_TRUE;
	return G_FALSE;
}

// return true is Edge is right-going from Vertex
GBool GTesselator2D::IsRightGoing(GMeshEdge2D<GDouble> *Edge, GMeshVertex2D<GDouble> *Vertex) {

	if (PointLE_X(Vertex->Position(), Edge->Dest()->Position()))
		return G_TRUE;
	return G_FALSE;
}

GBool GTesselator2D::ProcessRightGoingEdges(GMeshVertex2D<GDouble> *EventVertex, GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *outgoingEdge, *firstEdge;
	GMeshEdge2D<GDouble> *extEdge;
	GInt32 rightGoingFound;
	GMeshToAVL *customData;
	GBool resultFlag = G_FALSE;

	rightGoingFound = 0;
	// now we add to the edge dictionary all outgoing edges of eventVertex->MeshVertex
	firstEdge = outgoingEdge = EventVertex->Edge();
	do {
		if (IsRightGoing(outgoingEdge, EventVertex)) {

			customData = (GMeshToAVL *)outgoingEdge->CustomData();
			G_ASSERT(customData != NULL);
			// if this edge is a right diagonal (due to a previous right diagonal split) we must
			// insert it into dictionary like a right diagonal, else crossing numbers will be wrong!!!
			if (customData->EdgeType == RIGHT_ADDED_EDGE)
				extEdge = AddDictionaryEdge(outgoingEdge, RIGHT_ADDED_EDGE, resultFlag, Descriptor);
			else
				extEdge = AddDictionaryEdge(outgoingEdge, NORMAL_EDGE, resultFlag, Descriptor);
			// check if we need to re-sweep event
			if (resultFlag)
				return resultFlag;
			if (extEdge)
				rightGoingFound = 1;
		}
		// next edge around eventVertex->MeshVertex
		outgoingEdge = outgoingEdge->Oprev();
	} while (outgoingEdge != firstEdge);
	return resultFlag;
}

GMeshEdge2D<GDouble> *GTesselator2D::AddDictionaryEdge(GMeshEdge2D<GDouble> *MeshEdge, const GInt32 Flags,
													   GBool& RevisitFlag, GTessDescriptor& Descriptor) {

	G_ASSERT(MeshEdge != NULL);

	GBool alreadyExists;
	GInt32 intersected1, intersected2;
	GPoint<GDouble, 2> intPoint1, intPoint2, minPoint, maxPoint;
	GAVLNode *above, *below;
	GBool revisitLocalFlag;
	GMeshToAVL *customData;
	GMeshEdge2D<GDouble> *meshEdge;

	// if edge was already inserted into dictionary, it cannot be inserted again
	customData = (GMeshToAVL *)MeshEdge->CustomData();
	G_ASSERT(customData != NULL);
	if ((customData->HasBeenIntoDictionary) && (customData->IsIntoDictionary))
		return NULL;

	// temporary extended edge just to make findNext and findPrev work right; it will be used also for
	// inserting it into dictionary
	GMeshEdge2D<GDouble> *newExtEdge = MeshEdge;

	// Bentley-Ottmann
	above = Descriptor.DictionaryTree.FindNext((void *)newExtEdge);
nextAbove:
	if (above) {
		meshEdge = (GMeshEdge2D<GDouble> *)above->CustomData();
		if (meshEdge->Dest() == Descriptor.CurrentEvent) {
			above = Descriptor.DictionaryTree.Next(above);
			goto nextAbove;
		}
	}
	below = Descriptor.DictionaryTree.FindPrev((void *)newExtEdge);
nextBelow:
	if (below) {
		meshEdge = (GMeshEdge2D<GDouble> *)below->CustomData();
		if (meshEdge->Dest() == Descriptor.CurrentEvent) {
			below = Descriptor.DictionaryTree.Prev(below);
			goto nextBelow;
		}
	}

	if (above) {
		revisitLocalFlag = G_FALSE;
		revisitLocalFlag = DoIntersection((GMeshEdge2D<GDouble> *)above->CustomData(), newExtEdge, intersected1, Descriptor);
		RevisitFlag |= revisitLocalFlag;
		if (revisitLocalFlag)
			return NULL;
	}

	if (below) {
		revisitLocalFlag = G_FALSE;
		revisitLocalFlag = DoIntersection(newExtEdge, (GMeshEdge2D<GDouble> *)below->CustomData(), intersected2, Descriptor);
		RevisitFlag |= revisitLocalFlag;			
		if (revisitLocalFlag)
			return NULL;
	}

	// now insert the specified (it could be modified by ManageIntersections, but its pointer
	// is still valid) edge into dictionary
	customData = (GMeshToAVL *)MeshEdge->CustomData();
	G_ASSERT(customData != NULL);
	customData->EdgeType = Flags;
	customData->Region = NULL;
	customData->HasBeenIntoDictionary = G_TRUE;
	customData->IsIntoDictionary = G_TRUE;
	customData->CrossingNumber = -99;
	customData->AVLNode = Descriptor.DictionaryTree.Insert((void *)newExtEdge, alreadyExists);
	return newExtEdge;
}

GMeshVertex2D<GDouble>* GTesselator2D::InsertEventNoSort(GMeshVertex2D<GDouble> *EventVertex, GTessDescriptor& Descriptor) {

	GExtVertex *newExtVertex;

	// just simply insert it into the events queue
	newExtVertex = new GExtVertex(EventVertex);
	Descriptor.PriorityTree.push_back(newExtVertex);
	Descriptor.ExtVertices.push_back(newExtVertex);
	return EventVertex;
}

GMeshVertex2D<GDouble>* GTesselator2D::InsertEventSort(GMeshVertex2D<GDouble> *EventVertex, GTessDescriptor& Descriptor) {

	GExtVertex *newExtVertex = new GExtVertex(EventVertex);
	Descriptor.ExtVertices.push_back(newExtVertex);

	std::list< GExtVertex* >::iterator pivotIt;
	pivotIt = std::lower_bound(Descriptor.PriorityTree.begin(), Descriptor.PriorityTree.end(),
							   newExtVertex, SweepGreater);
	Descriptor.PriorityTree.insert(pivotIt, newExtVertex);
	return EventVertex;
}

GMeshEdge2D<GDouble> *GTesselator2D::TraceLeftDiagonal(GMeshVertex2D<GDouble> *Origin,
													   GMeshVertex2D<GDouble> *Destination,
													   GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *newEdge, *e1, *e2;
	GDouble l;

	// first check for zero-length diagonals
	l = LengthSquared(Destination->Position() - Origin->Position());
	if (l <= gPrecisionSquared) {
		G_ASSERT(0 == 1);
		return NULL;
	}

	// create new edge
	newEdge = Descriptor.TargetMesh.AddEdge()->Rot();
	e1 = Origin->Edge();
	e2 = Destination->Edge();
	newEdge->SetOrg(Origin);
	newEdge->SetDest(Destination);
	e1->SetOrg(Origin);
	e2->SetOrg(Destination);
	DoInsertEdge(newEdge, e1, &Descriptor.TargetMesh);

	newEdge->Sym()->SetOrg(Destination);
	newEdge->Sym()->SetDest(Origin);
	e1->SetOrg(Origin);
	e2->SetOrg(Destination);
	DoInsertEdge(newEdge->Sym(), e2, &Descriptor.TargetMesh);

	// allocate a descriptor for this new edge
	GMeshToAVL *customData = new GMeshToAVL;
	customData->Region = NULL;
	customData->EdgeType = LEFT_ADDED_EDGE;
	customData->HasBeenIntoDictionary = G_FALSE;
	customData->IsIntoDictionary = G_FALSE;
	customData->CrossingNumber = -99;
	customData->AVLNode = NULL;
	newEdge->SetCustomData((void *)customData);
	newEdge->Sym()->SetCustomData((void *)customData);
	Descriptor.ExtEdges.push_back(customData);
	return newEdge;
}

GMeshEdge2D<GDouble> *GTesselator2D::TraceRightDiagonal(GMeshVertex2D<GDouble> *Origin,
														GMeshVertex2D<GDouble> *Destination,
														GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *newEdge, *e1, *e2;
	GDouble l;
	GMeshToAVL *customData;

	// first check for zero-length diagonals
	l = LengthSquared(Destination->Position() - Origin->Position());
	if (l <= gPrecisionSquared) {
		G_ASSERT(0 == 1);
		return NULL;
	}

	// create new edge
	newEdge = Descriptor.TargetMesh.AddEdge()->Rot();

	customData = new GMeshToAVL;
	customData->AVLNode = NULL;
	customData->CrossingNumber = -99;
	customData->EdgeType = RIGHT_ADDED_EDGE;
	customData->HasBeenIntoDictionary = G_FALSE;
	customData->IsIntoDictionary = G_FALSE;
	customData->Region = NULL;
	newEdge->SetCustomData(customData);
	newEdge->Sym()->SetCustomData(customData);
	Descriptor.ExtEdges.push_back(customData);

	e1 = Origin->Edge();
	e2 = Destination->Edge();

	newEdge->SetOrg(Origin);
	newEdge->SetDest(Destination);
	e1->SetOrg(Origin);
	e2->SetOrg(Destination);
	DoInsertEdge(newEdge, e1, &Descriptor.TargetMesh);

	newEdge->Sym()->SetOrg(Destination);
	newEdge->Sym()->SetDest(Origin);
	e1->SetOrg(Origin);
	e2->SetOrg(Destination);
	DoInsertEdge(newEdge->Sym(), e2, &Descriptor.TargetMesh);

	return newEdge;
}

/*
	Check intersection for the two segments. In case of overlapping edges, the less (in lexicographic order)
	one will be	returned.
*/
GInt32 GTesselator2D::CheckIntersection(const GPoint<GDouble, 2>& Event,
										GMeshEdge2D<GDouble> *EdgeAbove, GMeshEdge2D<GDouble> *EdgeBelow,
										GPoint<GDouble, 2>& IntersectionPoint) {

	G_ASSERT((EdgeAbove != NULL) && (EdgeBelow != NULL));
	GInt32 ptCmp1, ptCmp2, ptCmp3, ptCmp4;
	GPoint<GDouble, 2> intersectionPoint;
	GMeshEdge2D<GDouble> *tmpEdge;

	G_ASSERT(EdgeAbove != EdgeBelow);

	if (EdgeAbove->Org() == EdgeBelow->Org() || EdgeAbove->Dest() == EdgeBelow->Dest())
		return NO_INTERSECTION;
	
	const GPoint<GDouble, 2>& o1 = (EdgeAbove->Org()->Position());
	const GPoint<GDouble, 2>& d1 = (EdgeAbove->Dest()->Position());
	const GPoint<GDouble, 2>& o2 = (EdgeBelow->Org()->Position());
	const GPoint<GDouble, 2>& d2 = (EdgeBelow->Dest()->Position());

	GDouble intX, intY;
	GBool found = EdgeIntersect(o1, d1, o2, d2, intX, intY);

	if (found) {

		intersectionPoint.Set(intX, intY);

		if (PointLE_X(intersectionPoint, Event))
			intersectionPoint = Event;

		if (PointLE_X(EdgeAbove->Dest()->Position(), EdgeBelow->Dest()->Position()))
			tmpEdge = EdgeBelow;
		else
			tmpEdge = EdgeAbove;

		if (PointLE_X(tmpEdge->Dest()->Position(), intersectionPoint))
			intersectionPoint = tmpEdge->Dest()->Position();

		ptCmp1 = PointCmp(intersectionPoint, o1);
		ptCmp2 = PointCmp(intersectionPoint, d1);
		ptCmp3 = PointCmp(intersectionPoint, o2);
		ptCmp4 = PointCmp(intersectionPoint, d2);

		if (ptCmp1 == 0 || ptCmp2 == 0) {
			if (ptCmp3 == 0 || ptCmp4 == 0)
				return NO_INTERSECTION;
			else {
				IntersectionPoint.Set(intersectionPoint[G_X], intersectionPoint[G_Y]);
				return DEGENERATE_INTERSECTION2;
			}
		}
		if (ptCmp3 == 0 || ptCmp4 == 0) {
			if (ptCmp1 == 0 || ptCmp2 == 0)
				return NO_INTERSECTION;
			else {
				IntersectionPoint.Set(intersectionPoint[G_X], intersectionPoint[G_Y]);
				return DEGENERATE_INTERSECTION1;
			}
		}
		IntersectionPoint.Set(intersectionPoint[G_X], intersectionPoint[G_Y]);
		return GOOD_INTERSECTION;
	}
	else
		return NO_INTERSECTION;
}

GBool GTesselator2D::DoIntersection(GMeshEdge2D<GDouble> *EdgeAbove, GMeshEdge2D<GDouble> *EdgeBelow,
									GInt32& IntersectionType, GTessDescriptor& Descriptor) {

	GPoint<GDouble, 2> intPoint, auxPoint;
	GInt32 intersected, ptCmp;
	GBool revisitLocalFlag;
	GMeshEdge2D<GDouble> *newEdge;

	revisitLocalFlag = G_FALSE;
	intersected = CheckIntersection(Descriptor.CurrentEvent->Position(), EdgeAbove, EdgeBelow, intPoint);

	IntersectionType = intersected;
	if (intersected != NO_INTERSECTION) {

		if (intersected == GOOD_INTERSECTION) {
			ptCmp = PointCmp(intPoint, Descriptor.CurrentEvent->Position());
			if (ptCmp == 0)
				revisitLocalFlag = G_TRUE;
			ManageIntersections(EdgeAbove, EdgeBelow, intPoint, Descriptor);
		}
		else
		if (intersected == DEGENERATE_INTERSECTION2) {
			newEdge = ManageDegenerativeIntersections(EdgeBelow, EdgeAbove, intPoint, Descriptor);
			ptCmp = PointCmp(intPoint, Descriptor.CurrentEvent->Position());
			if (ptCmp == 0)
				revisitLocalFlag = G_TRUE;
		}
		else
		if (intersected == DEGENERATE_INTERSECTION1) {
			newEdge = ManageDegenerativeIntersections(EdgeAbove, EdgeBelow, intPoint, Descriptor);
			ptCmp = PointCmp(intPoint, Descriptor.CurrentEvent->Position());
			if (ptCmp == 0)
				revisitLocalFlag = G_TRUE;
		}
	}
	return revisitLocalFlag;
}

void GTesselator2D::ManageIntersections(GMeshEdge2D<GDouble> *EdgeAbove, GMeshEdge2D<GDouble> *EdgeBelow,
										const GPoint<GDouble, 2>& IntersectionPoint, GTessDescriptor& Descriptor) {

	G_ASSERT((EdgeAbove != NULL) && (EdgeBelow != NULL));
	if (EdgeAbove == EdgeBelow)
		return;

	GMeshEdge2D<GDouble> *D;
	GMeshEdge2D<GDouble> *newEdgeAbove, *newEdgeBelow;
	GMeshVertex2D<GDouble> *newVertexAbove, *newVertexBelow;
	GMeshToAVL *customData, *tmpData;

	// debug stuff
#ifdef TESSELATOR_DEBUG_ACTIVATED
	if (Point_Cmp(EdgeAbove->Org()->Position(), IntersectionPoint) == 0)
		D = NULL;
	if (Point_Cmp(EdgeAbove->Dest()->Position(), IntersectionPoint) == 0)
		D = NULL;
	if (Point_Cmp(EdgeBelow->Org()->Position(), IntersectionPoint) == 0)
		D = NULL;
	if (Point_Cmp(EdgeBelow->Dest()->Position(), IntersectionPoint) == 0)
		D = NULL;
	if (Point_Cmp(Descriptor.CurrentEvent->Position(), IntersectionPoint) == 0)
		D = NULL;
#endif

	// lets split EdgeAbove: this involve adding 1 vertex and one edge
	D = EdgeAbove->Lnext();
	newVertexAbove = Descriptor.TargetMesh.AddVertex(IntersectionPoint);

	G_ASSERT(D != EdgeAbove->Sym());

	//if (D != EdgeAbove->Sym()) {
		newEdgeAbove = Descriptor.TargetMesh.AddEdge()->Rot();
		GMesh2D<GDouble>::Splice(newEdgeAbove->Sym(), EdgeAbove->Sym());
		GMesh2D<GDouble>::Splice(newEdgeAbove, D);
		newEdgeAbove->SetDest(D->Org());
		Descriptor.TargetMesh.SetOrbitOrg(newEdgeAbove, newVertexAbove);
	//}
	//else {
		/*newEdgeAbove = gMesh->AddEdge();
		GMesh2D::Splice(EdgeAbove->Sym(), newEdgeAbove);
		newEdgeAbove->SetDest(EdgeAbove->Dest());
		gMesh->SetOrbitOrg(newEdgeAbove, newVertexAbove);*/
	//	G_ASSERT(0 == 1);
	//}
	tmpData = (GMeshToAVL *)EdgeAbove->CustomData();
	G_ASSERT(tmpData != NULL);
	customData = new GMeshToAVL;
	customData->AVLNode = NULL;
	customData->CrossingNumber = -99;
	customData->EdgeType = tmpData->EdgeType;
	customData->HasBeenIntoDictionary = G_FALSE;
	customData->IsIntoDictionary = G_FALSE;
	customData->Region = NULL;
	newEdgeAbove->SetCustomData(customData);
	newEdgeAbove->Sym()->SetCustomData(customData);
	Descriptor.ExtEdges.push_back(customData);

	// lets split EdgeBelow: this involve adding 1 vertex and one edge
	D = EdgeBelow->Lnext();
	newVertexBelow = Descriptor.TargetMesh.AddVertex(IntersectionPoint);

	G_ASSERT(D != EdgeBelow->Sym());
	//if (D != EdgeBelow->Sym()) {
		newEdgeBelow = Descriptor.TargetMesh.AddEdge()->Rot();
		GMesh2D<GDouble>::Splice(newEdgeBelow->Sym(), EdgeBelow->Sym());
		GMesh2D<GDouble>::Splice(newEdgeBelow, D);
		newEdgeBelow->SetDest(D->Org());
		Descriptor.TargetMesh.SetOrbitOrg(newEdgeBelow, newVertexBelow);
	//}
	//else {
	//	G_ASSERT(0 == 1);
		/*newEdgeBelow = gMesh->AddEdge();
		GMesh2D::Splice(EdgeBelow->Sym(), newEdgeBelow);
		newEdgeBelow->SetDest(EdgeBelow->Dest());
		gMesh->SetOrbitOrg(newEdgeBelow, newVertexBelow);*/
	//}

	tmpData = (GMeshToAVL *)EdgeBelow->CustomData();
	G_ASSERT(tmpData != NULL);
	customData = new GMeshToAVL;
	customData->AVLNode = NULL;
	customData->CrossingNumber = -99;
	customData->EdgeType = tmpData->EdgeType;
	customData->HasBeenIntoDictionary = G_FALSE;
	customData->IsIntoDictionary = G_FALSE;
	customData->Region = NULL;
	newEdgeBelow->SetCustomData(customData);
	newEdgeBelow->Sym()->SetCustomData(customData);
	Descriptor.ExtEdges.push_back(customData);
	// insert a "merge" event
	InsertEventSort(newVertexAbove, Descriptor);
	InsertEventSort(newVertexBelow, Descriptor);
}


GMeshEdge2D<GDouble> *GTesselator2D::ManageDegenerativeIntersections(GMeshEdge2D<GDouble> *Touched,
																	 GMeshEdge2D<GDouble> *UnTouched,
																	 const GPoint<GDouble, 2>& IntersectionPoint,
																	 GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *N, *D;
	GMeshVertex2D<GDouble> *newVertex;
	GMeshToAVL *customData, *tmpData;
	
	D = Touched->Lnext();

	if ((Touched->Org()->Position() == UnTouched->Org()->Position()) &&
		(Touched->Dest()->Position() == UnTouched->Dest()->Position()))
		G_ASSERT(0 == 1);

	// create new vertex
	newVertex = Descriptor.TargetMesh.AddVertex(IntersectionPoint);

	// create new edge and make the connectivity right
	G_ASSERT(D != Touched->Sym());
	//if (D != Touched->Sym()) {
		N = Descriptor.TargetMesh.AddEdge()->Rot();
		GMesh2D<GDouble>::Splice(N->Sym(), Touched->Sym());
		GMesh2D<GDouble>::Splice(N, D);
		N->SetDest(D->Org());
		Descriptor.TargetMesh.SetOrbitOrg(N, newVertex);
	//}
	//else {
		//N = NULL;
		/*N = gMesh->AddEdge();
		GMesh2D::Splice(T->Sym(), N);
		N->SetDest(T->Dest());
		gMesh->SetOrbitOrg(N, newVertex);*/
		//G_ASSERT(0 == 1);
	//}
	tmpData = (GMeshToAVL *)Touched->CustomData();
	G_ASSERT(tmpData != NULL);
	customData = new GMeshToAVL;
	customData->AVLNode = NULL;
	customData->CrossingNumber = -99;
	customData->EdgeType = tmpData->EdgeType;
	customData->HasBeenIntoDictionary = G_FALSE;
	customData->IsIntoDictionary = G_FALSE;
	customData->Region = NULL;
	N->SetCustomData(customData);
	N->Sym()->SetCustomData(customData);
	Descriptor.ExtEdges.push_back(customData);

	InsertEventSort(newVertex, Descriptor);
	return N;
}

GMeshVertex2D<GDouble> *GTesselator2D::MergeRings(GMeshVertex2D<GDouble> *Ring1Vertex,
												  GMeshVertex2D<GDouble> *Ring2Vertex,
												  GMesh2D<GDouble> *Mesh) {

	GUInt32 r1Count, r2Count, c;
	GMeshEdge2D<GDouble> *e, *startEdge, *destRingEdge, *tmpEdge;
	GMeshVertex2D<GDouble> *delVertex, *resVertex;

	if (Ring1Vertex == Ring2Vertex)
		return Ring1Vertex;

	// lets count how many edge goes from Ring1Vertex and from Ring2Vertex
	r1Count = Ring1Vertex->EdgesInRingCount();
	r2Count = Ring2Vertex->EdgesInRingCount();

#ifdef TESSELATOR_DEBUG_ACTIVATED
	// debug stuff
	GString s;

	s = "Merging rings " + StrUtils::ToString(Ring1Vertex->Position(), ";", "%5.2f") + " - ";
	s += StrUtils::ToString(Ring2Vertex->Position(), ";", "%5.2f");
	s += "  r1Count = " + StrUtils::ToString(r1Count) + "  r2Count = " + StrUtils::ToString(r2Count);
#endif

	// we have to "insert" the ring that has the small number of edges
	c = GMath::Min(r1Count, r2Count);

	// keep track of edges that will be inserted into the other ring
	if (c == r1Count) {
		startEdge = Ring1Vertex->Edge();
		delVertex = Ring1Vertex;
		resVertex = Ring2Vertex;
		destRingEdge = Ring2Vertex->Edge();
	}
	else {
		startEdge = Ring2Vertex->Edge();
		delVertex = Ring2Vertex;
		resVertex = Ring1Vertex;
		destRingEdge = Ring1Vertex->Edge();
	}

	e = startEdge->Onext();
	while (e != startEdge) {
		tmpEdge = e->Onext();
		DoInsertEdge(e, destRingEdge, Mesh);
#ifdef TESSELATOR_DEBUG_ACTIVATED
		DebugDumpOrgRing(debugFile, destRingEdge->Org());
#endif
		e = tmpEdge;
	}
	// insert startEdge
	DoInsertEdge(startEdge, destRingEdge, Mesh);
#ifdef TESSELATOR_DEBUG_ACTIVATED
	DebugDumpOrgRing(debugFile, destRingEdge->Org());
#endif

	Mesh->RemoveVertex(delVertex);

	// debug stuff
#ifdef TESSELATOR_DEBUG_ACTIVATED
	if (resVertex->EdgesInRingCount() != r1Count + r2Count)
		s += " *******";
	DebugWrite(debugFile, StrUtils::ToAscii(s));
#endif
	return resVertex;
}

GMeshEdge2D<GDouble> *GTesselator2D::SafeRemoveEdgeFromVertex(GMeshEdge2D<GDouble> *Edge) {
	// replace the arbitrary edge with another edge (different from BadEdge) in the orbit use null
	// if this is the only edge assumes that the edge hasn't been actually removed yet
	GMeshEdge2D<GDouble> *edge, *startEdge;

	startEdge = Edge;
	edge = startEdge->Onext();
	while (edge != startEdge) {
		if (PointCmp(startEdge->Dest()->Position(), edge->Dest()->Position()) != 0) {
			Edge->Org()->AddEdge(edge);
			return edge;
		}
		edge = edge->Onext();
	}
	return NULL;
}

void GTesselator2D::DoInsertEdge(GMeshEdge2D<GDouble> *EdgeToInsert, GMeshEdge2D<GDouble> *RingEdge,
								 GMesh2D<GDouble> *Mesh) {

	GMeshEdge2D<GDouble> *e, *tmp, *rEdge = RingEdge;

	e = CCWSmallerAngleSpanEdge(rEdge, EdgeToInsert->Org()->Position(), EdgeToInsert->Dest()->Position());
	GMesh2D<GDouble>::Splice(EdgeToInsert, e);
	// check if e is still right
	if (e->Onext() != EdgeToInsert) {
		// we have to find in the RingEdge's ring who have as Onext EdgeToInsert
		tmp = rEdge;
		while (1) {
			if (tmp->Onext() == EdgeToInsert) {
				GMesh2D<GDouble>::Splice(tmp, e);
				goto setOrbit;
			}
			tmp = tmp->Onext();
		}
	}
setOrbit:
	Mesh->SetOrbitOrg(e, e->Org());
}


GBool GTesselator2D::CloseRegion(GMeshEdge2D<GDouble> *UpperEdge, GDynArray<GActiveRegion *>& ActiveRegions,
								 GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *e;
	GInt32 correctCrossNumber;
	GActiveRegion *region;
	GInt32 ptCmp;
	GMeshToAVL *customData;
	GAVLNode *n;

#ifdef TESSELATOR_DEBUG_ACTIVATED
	// debug stuff
	GString s;
#endif

	customData = (GMeshToAVL *)UpperEdge->CustomData();
	G_ASSERT(customData != NULL);

#ifdef TESSELATOR_DEBUG_ACTIVATED
	s = "Closing region, identify edge is: " + StrUtils::ToString(UpperEdge->Org()->Position(), ";", "%5.2f") + ", ";
	s += StrUtils::ToString(UpperEdge->Dest()->Position(), ";", "%5.2f");
	DebugWrite(debugFile, StrUtils::ToAscii(s));
#endif

	if (customData->EdgeType == NORMAL_EDGE)
		correctCrossNumber = customData->CrossingNumber;
	else {
		G_ASSERT(customData->EdgeType == RIGHT_ADDED_EDGE);
		n = Descriptor.DictionaryTree.Next(customData->AVLNode);
		if (n) {
			e = (GMeshEdge2D<GDouble> *)n->CustomData();
#ifdef TESSELATOR_DEBUG_ACTIVATED
			s = StrUtils::ToString(UpperEdge->Org()->Position(), ";", "%5.2f") + ", " + StrUtils::ToString(e->Org()->Position(), ";", "%5.2f");
			DebugWrite(debugFile, StrUtils::ToAscii(s));
#endif
			G_ASSERT(e);
			ptCmp = PointCmp(UpperEdge->Org()->Position(), e->Org()->Position());
			if (ptCmp == 0) {
				ptCmp = PointCmp(UpperEdge->Dest()->Position(), e->Dest()->Position());
				if (ptCmp == 0) {
					customData = (GMeshToAVL *)e->CustomData();
					correctCrossNumber = customData->CrossingNumber;
					goto doCloseRegion;
				}
			}
		}

		e = UpperEdge;
		G_ASSERT(e != NULL);

		do {
			e = e->Rnext();
#ifdef TESSELATOR_DEBUG_ACTIVATED
			// debug stuff
			s = "Analyzing edge " + StrUtils::ToString(e->Org()->Position(), ";", "%5.2f") + ", ";
			s += StrUtils::ToString(e->Dest()->Position(), ";", "%5.2f");
			DebugWrite(debugFile, StrUtils::ToAscii(s));
#endif
			G_ASSERT(e != NULL);
			customData = (GMeshToAVL *)e->CustomData();
			G_ASSERT(customData != NULL);
		} while(customData->EdgeType == RIGHT_ADDED_EDGE);
		G_ASSERT(customData != NULL);
		G_ASSERT(e != NULL);
		if (IsRightGoing(e, e->Org()))
			correctCrossNumber = customData->CrossingNumber;
		else
			correctCrossNumber = customData->CrossingNumber - 1;
	}

doCloseRegion:
	region = new GActiveRegion;
	region->CrossingNumber = correctCrossNumber;
	region->MeshUpperEdge = UpperEdge;
	region->Valid = G_TRUE;
	ActiveRegions.push_back(region);
	customData = (GMeshToAVL *)UpperEdge->CustomData();
	customData->Region = region;
	return G_TRUE;
}

GBool GTesselator2D::CloseRegions(GMeshVertex2D<GDouble> *EventVertex, GDynArray<GActiveRegion *>& ActiveRegions,
								  GAVLNode **UpperBounder, GAVLNode **LowerBounder,
								  GBool& RevisitFlag, GTessDescriptor& Descriptor) {

	GAVLNode *nUpper, *nLower, *regionUpperNode, *regionLowerNode;
	GAVLNode *upperBO;
	GMeshEdge2D<GDouble> *upperExtEdge, *lowerExtEdge, *extAbove, *extBelow;
	GDouble area, oldArea, sweepDist;
	GInt32 crossNumber, intersected, lCount, delCount, area0Count, ptCmp;
	GBool leftGoingFound, regionClosed, localRevisitFlag;
	GMeshToAVL *data;

	leftGoingFound = G_FALSE;
	regionUpperNode = regionLowerNode = NULL;
	nUpper = Descriptor.DictionaryTree.Max();
	upperBO = NULL;

	crossNumber = 1;
	lCount = delCount = area0Count = 0;
	area = oldArea = 1;
	while (nUpper) {
		upperExtEdge = (GMeshEdge2D<GDouble> *)nUpper->CustomData();
		// update crossing number
		data = (GMeshToAVL *)upperExtEdge->CustomData();
		// edge is into dictionary, so it MUST include a descriptor
		G_ASSERT(data != NULL);
		if (data->EdgeType != RIGHT_ADDED_EDGE) {
			data->CrossingNumber = crossNumber;
			crossNumber++;
		}

		sweepDist = SweepLineDistance(upperExtEdge, Descriptor.CurrentEvent);
		if (sweepDist > EventVertex->Position()[G_Y]) {
			ptCmp = PointCmp(upperExtEdge->Org()->Position(), EventVertex->Position());
			if (ptCmp != 0) {
				ptCmp = PointCmp(upperExtEdge->Dest()->Position(), EventVertex->Position());
				if (ptCmp != 0)
					regionUpperNode = nUpper;
			}
		}
		else
		if ((sweepDist < EventVertex->Position()[G_Y]) && (regionLowerNode == NULL)) {
			ptCmp = PointCmp(upperExtEdge->Org()->Position(), EventVertex->Position());
			if (ptCmp != 0) {
				ptCmp = PointCmp(upperExtEdge->Dest()->Position(), EventVertex->Position());
				if (ptCmp != 0)
					regionLowerNode = nUpper;
			}
		}

		// extract the edge immediately below upperExtEdge
		nLower = Descriptor.DictionaryTree.Prev(nUpper);

		// now test if eventVertex->MeshVertex is a left-going vertex of upperExtEdge->MeshEdge
		if (IsLeftGoingFast(upperExtEdge, EventVertex)) {
			leftGoingFound = G_TRUE;
			lCount++;
			// now test if eventVertex->MeshVertex is a left-going vertex of lowerExtEdge->MeshEdge
			if (nLower) {
				lowerExtEdge = (GMeshEdge2D<GDouble> *)nLower->CustomData();
				if (IsLeftGoingFast(lowerExtEdge, EventVertex)) {
					area = GMath::Abs(TwiceSignedArea(upperExtEdge->Org()->Position(),
														EventVertex->Position(),
														lowerExtEdge->Org()->Position()));
					if (area > gPrecision) {
						// now we are sure that both edges are left-going edges, so we can output an
						// active region and classify it according to upperExtEdge cross number
						regionClosed = CloseRegion(upperExtEdge, ActiveRegions, Descriptor);
						if (area0Count & 1)
							delCount += (area0Count + 1);
						else
							delCount += area0Count;
						area0Count = 0;
					}
					else
						area0Count++;
				}
			}

			if (upperBO && nLower) {
				// test Bentley-Ottmann
				extAbove = (GMeshEdge2D<GDouble> *)upperBO->CustomData();
				extBelow = (GMeshEdge2D<GDouble> *)nLower->CustomData();
				localRevisitFlag = DoIntersection(extAbove, extBelow, intersected, Descriptor);
				RevisitFlag |= localRevisitFlag;
				if (localRevisitFlag)
					return leftGoingFound;
			}
		}
		else
			upperBO = nUpper;

		nUpper = nLower;
	}
	*UpperBounder = regionUpperNode;
	*LowerBounder = regionLowerNode;
	if (area <= gPrecision) {
		if (area0Count & 1)
			delCount += (area0Count + 1);
		else
			delCount += area0Count;
	}

	if ((lCount == delCount) || (leftGoingFound == G_FALSE))
		return G_TRUE;
	else
		return G_FALSE;
}

GBool GTesselator2D::PatchRightDiagonal(GMeshVertex2D<GDouble> *Event, GAVLNode *UpperBounder, GAVLNode *LowerBounder,
										GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *startEdge, *e, *tmpEdge, *aboveUpperEdge, *belowLowerEdge;
	GInt32 rCount = 0, delCount = 0, ptCmp2;
	GPoint<GDouble, 2> minDest;
	GBool flag, resultFlag;
	GDouble area;
	GMeshVertex2D<GDouble> *v = NULL, *rightMeshVertex;
	GExtVertex *extVertex;

	startEdge = Event->Edge();
	e = startEdge;

	do {
		e = e->Oprev();
		if (IsRightGoing(e, Event)) {
			rCount++;
			tmpEdge = e->Oprev();
			if (IsRightGoing(tmpEdge, Event)) {
				area = TwiceSignedArea(Event->Position(), e->Dest()->Position(), tmpEdge->Dest()->Position());
				if (GMath::Abs(area) <= gPrecision) {
					if (v == NULL) {
						v = e->Dest();
						minDest = v->Position(); 
					}
					else {
						ptCmp2 = PointCmp(minDest, e->Dest()->Position());
						if (ptCmp2 > 0) {
							v = e->Dest();
							minDest = v->Position(); 
						}
					}
					delCount += 2;
					if (e == startEdge)
						break;
					e = e->Oprev();
					rCount++;
				}
			}
		}
	} while(e != startEdge);

	if ((rCount == 0) && UpperBounder && LowerBounder) {
		aboveUpperEdge = (GMeshEdge2D<GDouble> *)UpperBounder->CustomData();
		belowLowerEdge = (GMeshEdge2D<GDouble> *)LowerBounder->CustomData();
		// we must trace a non intersecting right diagonal; to do this, we just pick the next event
		// point bounded by aboveUpperExtEdge and belowLowerExtEdge; in general, it's not safe to
		// find the rightmost point of AboveUpper segment and BelowLower segment
		std::list< GExtVertex* >::iterator it = Descriptor.PriorityTree.begin();
		rightMeshVertex = NULL;

		it++;
		while (rightMeshVertex == NULL && it != Descriptor.PriorityTree.end()) {

			extVertex = *it;
			G_ASSERT(extVertex != NULL);
			// we have to check if the event point is located at the right side of above edge
			// and at the left side of below edge
			if ((CounterClockWiseOrAligned(belowLowerEdge->Org()->Position(),
										  belowLowerEdge->Dest()->Position(),
										  extVertex->MeshVertex->Position(), gPrecision)) &&
				(ClockWiseOrAligned(aboveUpperEdge->Org()->Position(),
									aboveUpperEdge->Dest()->Position(),
									extVertex->MeshVertex->Position(), gPrecision)))
				rightMeshVertex = extVertex->MeshVertex;
			it++;
		}

#ifdef TESSELATOR_DEBUG_ACTIVATED
		// debug stuff
		GString s;
#endif

		// some numerical instabilities, due to ccw/cw calculation, may lead to not find a good destination
		// vertex where to trace diagonal; we take as a valid point the leftmost destination point of
		// bounders
		if (rightMeshVertex == NULL) {
#ifdef TESSELATOR_DEBUG_ACTIVATED
			s = "Trace right diag. Not found on while, lets try bounders. ";
#endif
			if (PointLE_X(aboveUpperEdge->Dest()->Position(), belowLowerEdge->Dest()->Position())) {
				rightMeshVertex = aboveUpperEdge->Dest();
#ifdef TESSELATOR_DEBUG_ACTIVATED
				s += "Above edge is good: " + StrUtils::ToString(aboveUpperEdge->Dest()->Position(), ";", "%5.2f");
#endif
			}
			else {
				rightMeshVertex = belowLowerEdge->Dest();
#ifdef TESSELATOR_DEBUG_ACTIVATED
				s += "Below edge is good: " + StrUtils::ToString(belowLowerEdge->Dest()->Position(), ";", "%5.2f");
#endif
			}
		}
		else {
#ifdef TESSELATOR_DEBUG_ACTIVATED
			s = "Trace right diag. Found on while. Good destination: ";
			s += StrUtils::ToString(rightMeshVertex->Position(), ";", "%5.2f");
#endif
		}

#ifdef TESSELATOR_DEBUG_ACTIVATED
		// debug stuff
		DebugWrite(debugFile, StrUtils::ToAscii(s));
#endif

		// trace diagonal between eventVertex->MeshVertex and rightMeshVertex
		// AND WE MUST ADD THIS DIAGONAL TO THE EDGE DICTIONARY (because it could close
		// a region. Its flag MUST be RIGHT_ADDED_EDGE)
		GMeshEdge2D<GDouble> *newEdge = TraceRightDiagonal(Event, rightMeshVertex, Descriptor);
		if (newEdge)
			AddDictionaryEdge(newEdge, RIGHT_ADDED_EDGE, resultFlag, Descriptor);
		return G_FALSE;
	}

	if ((delCount == rCount) && rCount > 0 && UpperBounder && LowerBounder) {
		e = TraceRightDiagonal(Event, v, Descriptor);
		if (e)
			AddDictionaryEdge(e, RIGHT_ADDED_EDGE, flag, Descriptor);
		return G_TRUE;
	}
	else
		return G_FALSE;
}

GTesselator2D::GExtVertex* GTesselator2D::MergeCoincidentVertices(GTessDescriptor& Descriptor) {

	GExtVertex *curEvent, *nextEvent;
	GMeshVertex2D<GDouble> *queueVertex;
	GInt32 ptCmp;
	std::list< GExtVertex* >::iterator it = Descriptor.PriorityTree.begin();

	curEvent = *it;
	while (curEvent) {
		it++;
		// check if we are at the end of list
		if (it == Descriptor.PriorityTree.end())
			goto cleanRing2;
		// now compare
		nextEvent = *it;
		G_ASSERT(nextEvent != NULL);
		ptCmp = PointCmp(curEvent->MeshVertex->Position(), nextEvent->MeshVertex->Position());
		if (ptCmp == 0) {
			queueVertex = MergeRings(curEvent->MeshVertex, nextEvent->MeshVertex, &Descriptor.TargetMesh);
			G_ASSERT(queueVertex != NULL);
			nextEvent->MeshVertex = queueVertex;
			Descriptor.PriorityTree.pop_front();
			curEvent = nextEvent;
		}
		else
			goto cleanRing2;
	}
cleanRing2:
	return curEvent;
}

void GTesselator2D::SimplifyEdges(GMeshVertex2D<GDouble> *Event, GDynArray<GActiveRegion *>& ActiveRegions,
								  GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *firstEdge, *outgoingEdge, *tmpEdge, *auxEdge;
	GInt32 ptCmp, ringCount;
	GUInt32 i, j;
	GMeshToAVL *data1, *data2, *data2Sym, *data1Sym;

	ringCount = Event->EdgesInRingCount();

cleanRing:
	if (ringCount <= 0)
		goto cleanRegions;
	firstEdge = outgoingEdge = Event->Edge();
	do {
		tmpEdge = outgoingEdge->Onext();
		ptCmp = PointCmp(tmpEdge->Dest()->Position(), outgoingEdge->Dest()->Position());
		outgoingEdge = tmpEdge;
	} while (ptCmp == 0 && outgoingEdge != firstEdge);
	firstEdge = outgoingEdge->Oprev();
	outgoingEdge = firstEdge;

	do {
		if (!IsRightGoing(outgoingEdge, Event)) {

			data2 = (GMeshToAVL *)outgoingEdge->CustomData();
			data2Sym = (GMeshToAVL *)outgoingEdge->Sym()->CustomData();
			G_ASSERT(data2 == data2Sym && data2 != NULL);
			if ((data2->HasBeenIntoDictionary) && (data2->IsIntoDictionary)) {
				G_ASSERT(data2->AVLNode != NULL);
				data2->IsIntoDictionary = G_FALSE;
				Descriptor.DictionaryTree.DeleteNode(data2->AVLNode);
			}

			tmpEdge = outgoingEdge->Oprev();
			if (!IsRightGoing(tmpEdge, Event)) {
				ptCmp = PointCmp(tmpEdge->Dest()->Position(), outgoingEdge->Dest()->Position());
				if (ptCmp == 0) {
					// extract descriptors
					data1 = (GMeshToAVL *)tmpEdge->CustomData();
					data1Sym = (GMeshToAVL *)tmpEdge->Sym()->CustomData();
					G_ASSERT(data1 == data1Sym && data1 != NULL);

					if ((data1->HasBeenIntoDictionary) && (data1->IsIntoDictionary)) {
						G_ASSERT(data1->AVLNode != NULL);
						data1->IsIntoDictionary = G_FALSE;
						Descriptor.DictionaryTree.DeleteNode(data1->AVLNode);
					}

					if (data1->HasBeenIntoDictionary && data2->HasBeenIntoDictionary) {
						if (data1->Region) {
							auxEdge = tmpEdge->Oprev();
							ptCmp = PointCmp(auxEdge->Dest()->Position(), tmpEdge->Dest()->Position());
							if (ptCmp == 0) {
								data1->Region->MeshUpperEdge = auxEdge->Sym();
							}
							else
								data1->Region->Valid = G_FALSE;
						}
						else
						if (data2->Region) {
							auxEdge = tmpEdge->Oprev();
							ptCmp = PointCmp(auxEdge->Dest()->Position(), tmpEdge->Dest()->Position());
							if (ptCmp == 0) {
								data2->Region->MeshUpperEdge = auxEdge->Sym();
							}
							else
								data2->Region->Valid = G_FALSE;
						}
					}
					// detach tmpEdge and outgoingEdge
					if (data1->HasBeenIntoDictionary) {
						SafeRemoveEdgeFromVertex(tmpEdge);
						SafeRemoveEdgeFromVertex(tmpEdge->Sym());
						Descriptor.TargetMesh.DetachEdge(tmpEdge);
						ringCount--;
					}
					if (data2->HasBeenIntoDictionary) {
						SafeRemoveEdgeFromVertex(outgoingEdge);
						SafeRemoveEdgeFromVertex(outgoingEdge->Sym());
						Descriptor.TargetMesh.DetachEdge(outgoingEdge);
						ringCount--;
					}
					// start again (this simplify code)
					goto cleanRing;
				}
			}
		}
		outgoingEdge = outgoingEdge->Oprev();
	} while (outgoingEdge != firstEdge);

cleanRegions:

	j = (GUInt32)ActiveRegions.size();
	for (i = 0; i < j; i++) {
		data1 = (GMeshToAVL *)ActiveRegions[i]->MeshUpperEdge->CustomData();
		G_ASSERT(data1 != NULL && data1->HasBeenIntoDictionary == G_TRUE);
		if (ActiveRegions[i]->Valid) {
			tmpEdge = ActiveRegions[i]->MeshUpperEdge->Rprev();
			if (!IsLeftGoing(tmpEdge, Event) || 
				!ClockWise(ActiveRegions[i]->MeshUpperEdge->Org()->Position(), Event->Position(), tmpEdge->Dest()->Position(), gPrecision))
				ActiveRegions[i]->Valid = G_FALSE;
		}
	}
}


void GTesselator2D::TessellateMonotoneRegion(const GActiveRegion* Region, GDynArray<GULong>& PointsIds,
											 GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *up, *lo;
	GMeshEdge2D<GDouble> *tempEdge = NULL;

	// All edges are oriented CCW around the boundary of the region.
	// First, find the half-edge whose origin vertex is rightmost.
	// Since the sweep goes from left to right, face->anEdge should be close to the edge we want
	up = Region->MeshUpperEdge->Sym();
	lo = up->Lprev();

	while (up->Lnext() != lo) {
		if (PointCmp(up->Dest()->Position(), lo->Org()->Position()) <= 0) {
			// up->Dst is on the left.  It is safe to form triangles from lo->Org.
			// The EdgeGoesLeft test guarantees progress even when some triangles
			// are CW, given that the upper and lower chains are truly monotone.
			while ((lo->Lnext() != up) &&
					((PointCmp(lo->Lnext()->Dest()->Position(), lo->Lnext()->Org()->Position()) <= 0) ||
					(ClockWiseOrAligned(lo->Org()->Position(), lo->Lnext()->Dest()->Position(), lo->Dest()->Position(), gPrecision)))) {
				// create new edge
				tempEdge = Descriptor.TargetMesh.AddEdge();
				G_ASSERT(tempEdge != NULL);
				
				// output indexes
				PointsIds.push_back((GULong)lo->Dest()->CustomData());
				PointsIds.push_back((GULong)lo->Lnext()->Dest()->CustomData());
				PointsIds.push_back((GULong)lo->Org()->CustomData());

				// we have to trace a diagonal from lo->Lnext->Dest to lo->Org
				GMesh2D<GDouble>::Splice(lo, tempEdge->Sym());
				GMesh2D<GDouble>::Splice(lo->Lnext()->Lnext(), tempEdge);
				tempEdge->SetOrg(lo->Lnext()->Dest());
				tempEdge->SetDest(lo->Org());
				lo = tempEdge->Sym();
			}
			lo = lo->Lprev();
		}
		else {
			// lo->Org is on the left; we can make CCW triangles from up->Dest
			while ((lo->Lnext() != up) &&
					((PointCmp(up->Lprev()->Dest()->Position(), up->Lprev()->Org()->Position()) >= 0) ||
					(CounterClockWiseOrAligned(up->Dest()->Position(), up->Lprev()->Org()->Position(), up->Org()->Position(), gPrecision)))) {
				// create new edge
				tempEdge = Descriptor.TargetMesh.AddEdge();
				G_ASSERT(tempEdge != NULL);
				// output indexes
				PointsIds.push_back((GULong)up->Lprev()->Org()->CustomData());
				PointsIds.push_back((GULong)up->Org()->CustomData());
				PointsIds.push_back((GULong)up->Dest()->CustomData());

				// we have to trace a diagonal from up->Dest to up->Lprev->Org
				GMesh2D<GDouble>::Splice(up->Lprev(), tempEdge->Sym());
				GMesh2D<GDouble>::Splice(up->Lnext(), tempEdge);
				tempEdge->SetOrg(up->Dest());
				tempEdge->SetDest(up->Lprev()->Org());
				up = tempEdge->Sym();
			}
			up = up->Lnext();
		}
	}

	// Now lo->Org == up->Dst == the leftmost vertex.  The remaining region
	// can be tessellated in a fan from this leftmost vertex.
	G_ASSERT(lo->Lnext() != up);
	while (lo->Lnext()->Lnext() != up) {
		// create new edge
		GMeshEdge2D<GDouble> *tempEdge = Descriptor.TargetMesh.AddEdge();
		G_ASSERT(tempEdge != NULL);
		// output indexes
		PointsIds.push_back((GULong)lo->Dest()->CustomData());
		PointsIds.push_back((GULong)lo->Lnext()->Dest()->CustomData());
		PointsIds.push_back((GULong)lo->Org()->CustomData());
		// we have to trace a diagonal from lo->Lnext->Dest to lo->Org
		GMesh2D<GDouble>::Splice(lo, tempEdge->Sym());
		GMesh2D<GDouble>::Splice(lo->Lnext()->Lnext(), tempEdge);
		tempEdge->SetOrg(lo->Lnext()->Dest());
		tempEdge->SetDest(lo->Org());
		lo = tempEdge->Sym();
	}
	// output indexes
	PointsIds.push_back((GULong)lo->Org()->CustomData());
	PointsIds.push_back((GULong)lo->Dest()->CustomData());
	PointsIds.push_back((GULong)lo->Lnext()->Dest()->CustomData());
}

void GTesselator2D::TessellateMonotoneRegion(const GActiveRegion* Region, GDynArray< GPoint<GDouble, 2> >& Points,
											 GTessDescriptor& Descriptor) {

	GMeshEdge2D<GDouble> *up, *lo;
	GMeshEdge2D<GDouble> *tempEdge = NULL;

	// All edges are oriented CCW around the boundary of the region.
	// First, find the half-edge whose origin vertex is rightmost.
	// Since the sweep goes from left to right, face->anEdge should be close to the edge we want
	up = Region->MeshUpperEdge->Sym();
	lo = up->Lprev();

	while (up->Lnext() != lo) {
		if (PointCmp(up->Dest()->Position(), lo->Org()->Position()) <= 0) {
			// up->Dst is on the left.  It is safe to form triangles from lo->Org.
			// The EdgeGoesLeft test guarantees progress even when some triangles
			// are CW, given that the upper and lower chains are truly monotone.
			while ((lo->Lnext() != up) &&
					((PointCmp(lo->Lnext()->Dest()->Position(), lo->Lnext()->Org()->Position()) <= 0) ||
					(ClockWiseOrAligned(lo->Org()->Position(), lo->Lnext()->Dest()->Position(), lo->Dest()->Position(), gPrecision)))) {
				// create new edge
				tempEdge = Descriptor.TargetMesh.AddEdge();
				G_ASSERT(tempEdge != NULL);
				// output plain vertexes
				Points.push_back(lo->Dest()->Position());
				Points.push_back(lo->Lnext()->Dest()->Position());
				Points.push_back(lo->Org()->Position());
				// we have to trace a diagonal from lo->Lnext->Dest to lo->Org
				GMesh2D<GDouble>::Splice(lo, tempEdge->Sym());
				GMesh2D<GDouble>::Splice(lo->Lnext()->Lnext(), tempEdge);
				tempEdge->SetOrg(lo->Lnext()->Dest());
				tempEdge->SetDest(lo->Org());
				lo = tempEdge->Sym();
			}
			lo = lo->Lprev();
		}
		else {
			// lo->Org is on the left; we can make CCW triangles from up->Dest
			while ((lo->Lnext() != up) &&
					((PointCmp(up->Lprev()->Dest()->Position(), up->Lprev()->Org()->Position()) >= 0) ||
					(CounterClockWiseOrAligned(up->Dest()->Position(), up->Lprev()->Org()->Position(), up->Org()->Position(), gPrecision)))) {
				// create new edge
				tempEdge = Descriptor.TargetMesh.AddEdge();
				G_ASSERT(tempEdge != NULL);
				// output plain vertexes
				Points.push_back(up->Lprev()->Org()->Position());
				Points.push_back(up->Org()->Position());
				Points.push_back(up->Dest()->Position());
				// we have to trace a diagonal from up->Dest to up->Lprev->Org
				GMesh2D<GDouble>::Splice(up->Lprev(), tempEdge->Sym());
				GMesh2D<GDouble>::Splice(up->Lnext(), tempEdge);
				tempEdge->SetOrg(up->Dest());
				tempEdge->SetDest(up->Lprev()->Org());
				up = tempEdge->Sym();
			}
			up = up->Lnext();
		}
	}

	// Now lo->Org == up->Dst == the leftmost vertex.  The remaining region
	// can be tessellated in a fan from this leftmost vertex.
	G_ASSERT(lo->Lnext() != up);
	while (lo->Lnext()->Lnext() != up) {
		// create new edge
		GMeshEdge2D<GDouble> *tempEdge = Descriptor.TargetMesh.AddEdge();
		G_ASSERT(tempEdge != NULL);
		// output plain vertexes
		Points.push_back(lo->Dest()->Position());
		Points.push_back(lo->Lnext()->Dest()->Position());
		Points.push_back(lo->Org()->Position());
		// we have to trace a diagonal from lo->Lnext->Dest to lo->Org
		GMesh2D<GDouble>::Splice(lo, tempEdge->Sym());
		GMesh2D<GDouble>::Splice(lo->Lnext()->Lnext(), tempEdge);
		tempEdge->SetOrg(lo->Lnext()->Dest());
		tempEdge->SetDest(lo->Org());
		lo = tempEdge->Sym();
	}
	// output plain vertexes
	Points.push_back(lo->Org()->Position());
	Points.push_back(lo->Dest()->Position());
	Points.push_back(lo->Lnext()->Dest()->Position());
}


GBool GTesselator2D::SweepEvent(GExtVertex* Event, GTessDescriptor& Descriptor) {

	GBool revisitEvent, traceLeftDiag;
	GUInt32 i, j;
	GMeshVertex2D<GDouble> *leftMeshVertex, *startLeftDiag, *endLeftDiag;
	GExtVertex* extVertex;
	GMeshEdge2D<GDouble> *regionUpperEdge, *regionLowerEdge;
	GDynArray<GActiveRegion *> tmpRegions;
	GAVLNode *upperBounder, *lowerBounder;
	GActiveRegion *ar;

	if (!Event)
		return G_FALSE;

reSweep:

#ifdef TESSELATOR_DEBUG_ACTIVATED
	// debug stuff
	GString s;
	s = "Entering event: " + StrUtils::ToString(Event->MeshVertex->Position(), ";", "%5.2f");
	DebugWrite(debugFile, StrUtils::ToAscii(s));
#endif

	revisitEvent = G_FALSE;
	// clear temporary regions
	j = (GUInt32)tmpRegions.size();
	for (i = 0; i < j; i++) {
		ar = tmpRegions[i];
		G_ASSERT(ar != NULL);
		delete ar;
	}
	tmpRegions.clear();
	startLeftDiag = endLeftDiag = NULL;
	// merge all rings at the same geometric location; NB: we specify to not simplify identical edges
	extVertex = MergeCoincidentVertices(Descriptor);
	Descriptor.CurrentEvent = extVertex->MeshVertex;
	Event->MeshVertex = extVertex->MeshVertex;

	// close regions
	upperBounder = lowerBounder = NULL;
	traceLeftDiag = CloseRegions(Descriptor.CurrentEvent, tmpRegions, &upperBounder, &lowerBounder, revisitEvent, Descriptor);
	if (revisitEvent)
		goto reSweep;
	// trace a left diagonal if there weren't left-going edges and the event is inside
	if (traceLeftDiag && upperBounder && lowerBounder) {
		regionUpperEdge = (GMeshEdge2D<GDouble> *)upperBounder->CustomData();
		regionLowerEdge = (GMeshEdge2D<GDouble> *)lowerBounder->CustomData();
		if (regionUpperEdge->Org()->Position()[G_X] < regionUpperEdge->Dest()->Position()[G_X])
			leftMeshVertex = regionUpperEdge->Org();
		else
			leftMeshVertex = regionUpperEdge->Dest();
		if (regionLowerEdge->Org()->Position()[G_X] < regionLowerEdge->Dest()->Position()[G_X]) {
			if (regionLowerEdge->Org()->Position()[G_X] > leftMeshVertex->Position()[G_X])
				leftMeshVertex = regionLowerEdge->Org();
		}
		else {
			if (regionLowerEdge->Dest()->Position()[G_X] > leftMeshVertex->Position()[G_X])
				leftMeshVertex = regionLowerEdge->Dest();
		}
		// trace left diagonal between eventVertex->MeshVertex and leftMeshVertex
		startLeftDiag = Descriptor.CurrentEvent;
		endLeftDiag = leftMeshVertex;
	}
	// process right-going edges in the mesh
	revisitEvent = ProcessRightGoingEdges(Descriptor.CurrentEvent, Descriptor);
	if (revisitEvent)
		goto reSweep;

	// trace left diagonal
	if (startLeftDiag && endLeftDiag)
		TraceLeftDiagonal(Descriptor.CurrentEvent, endLeftDiag, Descriptor);

	// trace a right diagonal
	PatchRightDiagonal(Descriptor.CurrentEvent, upperBounder, lowerBounder, Descriptor);

	SimplifyEdges(Descriptor.CurrentEvent, tmpRegions, Descriptor);

	// now these regions are good, lets push them
	j = (GUInt32)tmpRegions.size();
	for (i = 0; i < j; i++) {
		ar = tmpRegions[i];
		G_ASSERT(ar != NULL);
		if (ar->Valid)
			Descriptor.ActiveRegions.push_back(ar);
	}
	if (j > 0)
		Descriptor.LastRegion = tmpRegions[0];
	return revisitEvent;
}

GUInt32 GTesselator2D::PurgeRegions(GDynArray<GActiveRegion *>& ActiveRegions, const GBool Fast,
									GTessDescriptor& Descriptor) {

	GActiveRegion *ar;
	GMeshEdge2D<GDouble> *e, *startEdge, *e2;
	GUInt32 i, j = (GUInt32)ActiveRegions.size(), invalidCount = 0;
	GUInt32 k;

	if (ActiveRegions.size() == 0)
		return 0;

	if (Fast) {
		startEdge = e = Descriptor.LastRegionEdge;
		do {
			j = (GUInt32)ActiveRegions.size();
			for (i = 0; i < j; i++) {
				ar = ActiveRegions[i];
				G_ASSERT(ar != NULL);
				if (ar->MeshUpperEdge == e && ar->Valid) {
					ar->Valid = G_FALSE;
					invalidCount++;
				}
			}
			e = e->Rprev();
		} while (e != startEdge);
	}
	else {
		for (i = 0; i < j; i++) {
			ar = ActiveRegions[i];
			if (ar->Valid == G_FALSE)
				continue;
			e = ar->MeshUpperEdge;
			k = 0;
			do {
				e2 = e->Rprev();

				if (IsRightGoing(e->Sym(), e->Dest()) && IsRightGoing(e2, e->Dest()))
					k++;
				else
				if (IsLeftGoing(e->Sym(), e->Dest()) && IsLeftGoing(e2, e->Dest()))
					k++;

				e = e2;
			} while (e != ar->MeshUpperEdge && k <= 2);
			if (k > 2) {
				ar->Valid = G_FALSE;
				invalidCount++;
			}
		}
	}
	return invalidCount;
}


// searching into RingEdge's origin ring, return that edge that span the smaller angle in CCW
// direction to meet DestEdge
GMeshEdge2D<GDouble> *GTesselator2D::CCWSmallerAngleSpanEdge(GMeshEdge2D<GDouble> *RingEdge,
															 const GPoint<GDouble, 2>& Origin,
															 const GPoint<GDouble, 2>& Destination) {

	GMeshEdge2D<GDouble> *e1, *e2, *eGood, *startEdge;
	GInt32 i;
	GPoint<GDouble, 2> a, c;

	#define AREA_EPSILON 2.2204460492503131e-16

	e1 = startEdge = RingEdge;
	e2 = e1->Oprev();
	a = e1->Dest()->Position();
	c = e2->Dest()->Position();

	// we wanna find the edge (that belongs to the Destination ring) that span the smaller angle
	// in counterclockwise direction to meet the diagonal (actually its symmetric)
	i = CCWSmallerAngleSpan(Origin, a, c, Destination, AREA_EPSILON);
	if (i == 1)
		eGood = e1;
	else {
		eGood = e2;
		a = c;
	}
	e2 = e2->Oprev();
	while (e2 != startEdge) {
		c = e2->Dest()->Position();
		i = CCWSmallerAngleSpan(Origin, a, c, Destination, AREA_EPSILON);
		if (i != 1) {
			eGood = e2;
			a = c;
		}
		e2 = e2->Oprev();
	}
	// it's possible that there are overlapping edges
	GDouble area;
	GBool ok;
	e1 = eGood;
	do {
		e1 = e1->Onext();

		ok = G_FALSE;
		if (PointLE_X(eGood->Org()->Position(), eGood->Dest()->Position())) {

			if (PointLE_X(eGood->Org()->Position(), e1->Dest()->Position())) {
				area = GMath::Abs(TwiceSignedArea(eGood->Org()->Position(), eGood->Dest()->Position(), e1->Dest()->Position()));
				if (area < AREA_EPSILON)
					eGood = e1;
				else
					ok = G_TRUE;
			}
			else
				ok = G_TRUE;

		}
		else {
			if (PointLE_X(e1->Dest()->Position(), eGood->Org()->Position())) {
				area = GMath::Abs(TwiceSignedArea(eGood->Org()->Position(), eGood->Dest()->Position(), e1->Dest()->Position()));
				if (area < AREA_EPSILON)
					eGood = e1;
				else
					ok = G_TRUE;
			}
		}
	} while (!ok && e1 != eGood);
	return eGood;
}

#undef NORMAL_EDGE
#undef LEFT_ADDED_EDGE
#undef RIGHT_ADDED_EDGE
#undef NO_INTERSECTION
#undef GOOD_INTERSECTION
#undef DEGENERATE_INTERSECTION1
#undef DEGENERATE_INTERSECTION2
#undef DOUBLE_INTERSECTION
#undef PointLE_X
#undef PointLE_Y

};	// end namespace Amanith
