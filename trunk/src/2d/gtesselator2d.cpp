/****************************************************************************
** $file: amanith/src/2d/gtesselator2d.cpp   0.1.0.0   edited Jun 30 08:00
**
** 2D Polygonal shape tesselator implementation.
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

#define NORMAL_EDGE			1
#define LEFT_ADDED_EDGE		2
#define RIGHT_ADDED_EDGE	3

#define NO_INTERSECTION 0
#define GOOD_INTERSECTION 1
#define DEGENERATE_INTERSECTION1 2
#define DEGENERATE_INTERSECTION2 4
#define DOUBLE_INTERSECTION 8

GReal gPrecision = 2 * G_EPSILON;
GReal gPrecisionSquared = gPrecision;// * gPrecision;

// *********************************************************************
//                           GExtVertex
// *********************************************************************

/*static GInt32 PointCmp(const GPoint<GFloat, 2>& P1, const GPoint2<GFloat, 2>& P2) {

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
	//if (P1[G_Y] < P2[G_Y])
	return -1;
}*/

template<typename DATA_TYPE>
GInt32 PointCmp(const GPoint<DATA_TYPE, 2>& P1, const GPoint<DATA_TYPE, 2>& P2) {

	// they are the same point
	if (LengthSquared(P2 - P1) <= (DATA_TYPE)gPrecisionSquared)
		return 0;
	// test the x-coord first
	if (P1[G_X] > P2[G_X])
		return 1;
	if (P1[G_X] < P2[G_X])
		return -1;
	// and then test the y-coord second
	if (P1[G_Y] > P2[G_Y])
		return 1;
	//if (P1[G_Y] < P2[G_Y])
	return -1;
}

GReal SweepLineDistance(GMeshEdge2D *Edge, GMeshVertex2D* Event) {

	GReal dx, y, m, SweepX = Event->Position()[G_X];

	GPoint2 a(Edge->Org()->Position());
	GPoint2 b(Edge->Dest()->Position());

	// calculate sweep line distance from segment; here we care also of
	// vertical segments
	dx = (a[G_X] - b[G_X]);
	if (GMath::Abs(dx) <= gPrecision)
		y = GMath::Min(a[G_Y], b[G_Y]);
	else {
		m = (a[G_Y] - b[G_Y]) / dx;
		y = m * (SweepX - b[G_X]) + b[G_Y];
	}
	return y;
}

bool GTesselator2D::SweepGreater(const GExtVertex *Event1, const GExtVertex *Event2) {

	GInt32 i = PointCmp(Event1->MeshVertex->Position(), Event2->MeshVertex->Position());
	if (i < 0)
		return G_TRUE;
	return G_FALSE;
}

// *********************************************************************
//                          GDictionaryTree
// *********************************************************************

// edge compare
int GTesselator2D::GDictionaryTree::Compare(void *ItemA, void *ItemB) {

	GReal y1, y2, SweepX = this->DescPointer->CurrentEvent->Position()[G_X];
	GReal area1, area2;
	GInt32 cmp, ptCmp;
	GPoint2 v1, v2, sharedPoint;
	GMeshVertex2D *o1, *o2, *d1, *d2;

	G_ASSERT(ItemA != NULL);
	G_ASSERT(ItemB != NULL);

	GMeshEdge2D *e1 = (GMeshEdge2D *)ItemA;
	GMeshEdge2D *e2 = (GMeshEdge2D *)ItemB;

	if (e1 == e2)
		return 0;

	y1 = SweepLineDistance(e1, this->DescPointer->CurrentEvent);
	y2 = SweepLineDistance(e2, this->DescPointer->CurrentEvent);

	if (GMath::Abs(y1 - y2) <= gPrecision) {

		o1 = e1->Org();
		d1 = e1->Dest();
		o2 = e2->Org();
		d2 = e2->Dest();

		area1 = GMath::Abs(TwiceSignedArea(o1->Position(), d1->Position(), o2->Position()));
		area2 = GMath::Abs(TwiceSignedArea(o1->Position(), d1->Position(), d2->Position()));
		if (area1 <= gPrecision && area2 <= gPrecision) {
			ptCmp = PointCmp(o1->Position(), o2->Position());
			if (ptCmp != 0) {
				if (ptCmp < 0) {
					ptCmp = PointCmp(d1->Position(), o2->Position());
					if (ptCmp != 0)
						return 1;
					else
						return -1;
				}
				else {
					ptCmp = PointCmp(d2->Position(), o1->Position());
					if (ptCmp != 0)
						return -1;
					else
						return 1;
				}
			}
			else {
				ptCmp = PointCmp(d1->Position(), d2->Position());
				if (ptCmp != 0) {
					if (ptCmp < 0)
						return 1;
					else
						return -1;
				}
				else
					return -1;
			}
		}
		else {
			if (area1 <= gPrecision)
				sharedPoint = o2->Position();
			else
				sharedPoint = d2->Position();

			// we must find the 2 ends that are not shared
			if (o1->Position() == sharedPoint)
				v1 = d1->Position();
			else
			if (d1->Position() == sharedPoint)
				v1 = o1->Position();
			else {
				cmp = PointCmp(o1->Position(), d1->Position());
				if (cmp > 0)
					v1 = o1->Position();
				else
					if (cmp < 0)
						v1 = d1->Position();
					else
						G_ASSERT(0 == 1);
			}

			if (o2->Position() == sharedPoint)
				v2 = d2->Position();
			else
			if (d2->Position() == sharedPoint)
				v2 = o2->Position();
			else {
				cmp = PointCmp(o2->Position(), d2->Position());
				if (cmp > 0)
					v2 = o2->Position();
				else
				if (cmp < 0)
					v2 = d2->Position();
				else
					G_ASSERT(0 == 1);
			}
			// we have to take a downwards vector (we use v1 and v2 to take a number with a module in
			// according to vector lengths)
			GPoint2 sl(SweepX, sharedPoint[G_Y] - GMath::Abs(v1[G_Y]) - GMath::Abs(v2[G_Y]) - (GReal)1);
			GInt32 i = CCWSmallerAngleSpan(sharedPoint, v1, v2, sl);
			// both non-shared ends are on the right of sweepline
			if ((v1[G_X] >= SweepX) && (v2[G_X] >= SweepX)) {
				if (i == 1)
					return 1;
				else
					return -1;
			}
			else
			// both non-shared ends are on the left of sweepline
			if ((v1[G_X] < SweepX) && (v2[G_X] < SweepX)) {
				if (i == 2)
					return 1;
				else
					return -1;
			}
			// one non-shared point is on the right of the sweep line and the other one
			// is on the left
			else {
				ptCmp = PointCmp(v1, v2);
				G_ASSERT(ptCmp != 0);
				if (ptCmp < 1)
					return -1;
				else
					return 1;
			}
		}
	}
	// here we return above or belove; in the case of same distances we take the convention of
	// belove
	else
	if (y1 > y2)
		return 1;
	else
	if (y1 < y2)
		return -1;

	G_ASSERT(0 == 1);
	return 0;
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

// tessellation routine
GError GTesselator2D::Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
								GDynArray<GPoint2>& Triangles, const GBool OddFill) {

	GExtVertex *extVertex;
	GInt32 i, j, k, w, ofs;
	GBool revisitEvent;
	GActiveRegion *ar;

	// test input for consistency
	j = PointsPerContour.size();
	if (j == 0)
		return G_INVALID_PARAMETER;
	k = 0;
	for (i = 0; i < j; i++)
		k += PointsPerContour[i];
	if (k == 0 || k != (GInt32)Points.size())
		return G_INVALID_PARAMETER;

	// create a tessellation descriptor
	GTessDescriptor desc;

	// insert all contours
	ofs = 0;
	j = PointsPerContour.size();
	for (i = 0; i < j; i++) {
		// k = number of points of i-th contour
		k = PointsPerContour[i];
		if (k == 0)
			continue;
		BeginContour(Points[ofs][G_X], Points[ofs][G_Y], desc);
		ofs++;
		for (w = 1; w < k; w++) {
			AddContourPoint(Points[ofs][G_X], Points[ofs][G_Y], desc);
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
	// keep track of last closed region
	if (desc.LastRegion)
		desc.LastRegionEdge = desc.LastRegion->MeshUpperEdge->Sym();
	// remove all backface regions
	k = PurgeRegions(desc.ActiveRegions, G_TRUE, desc);
	// triangulate all monotone regions
	j = desc.ActiveRegions.size();
	for (i = 0; i < j; i++) {
		ar = desc.ActiveRegions[i];
		if (!ar->Valid)
			continue;
		if (OddFill) {
			if ((ar->CrossingNumber & 1) != 0)
				TessellateMonotoneRegion(ar, Triangles, desc);
		}
		else {
			if ((ar->CrossingNumber & 1) == 0)
				TessellateMonotoneRegion(ar, Triangles, desc);
		}
	}

	// free memory allocated by tessellation
	j = desc.ExtVertices.size();
	for (i = 0; i < j; i++) {
		GExtVertex *v = desc.ExtVertices[i];
		delete v;
	}
	j = desc.ExtEdges.size();
	for (i = 0; i < j; i++) {
		GMeshToAVL *data = desc.ExtEdges[i];
		delete data;
	}
	// clear created regions
	j = desc.ActiveRegions.size();
	for (i = 0; i < j; i++) {
		GActiveRegion *ar = desc.ActiveRegions[i];
		G_ASSERT(ar != NULL);
		delete ar;
	}

	return G_NO_ERROR;
}

void GTesselator2D::BeginContour(const GReal X, const GReal Y, GTessDescriptor& Descriptor) {

	G_ASSERT(Descriptor.LastEdge == NULL);
	G_ASSERT(Descriptor.StepsDone == 0);
	G_ASSERT(Descriptor.FirstPushedSteps == 0);
	G_ASSERT(Descriptor.PushedCount == 0);
	G_ASSERT(Descriptor.FirstPushedPoints[0] == NULL);
	G_ASSERT(Descriptor.FirstPushedPoints[1] == NULL);
	Descriptor.StepsDone = 1;
	Descriptor.LastPoints[0].Set(X, Y);
}

void GTesselator2D::AddContourPoint(const GReal X, const GReal Y, GTessDescriptor& Descriptor) {

	GPoint2 newPoint(X, Y);
	GReal area;
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

		GMeshVertex2D *vertex = Descriptor.LastEdge->Dest();
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

	GReal area;
	GMeshVertex2D *vertex;
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
		GMeshEdge2D *edge, *edgeTmp;
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

	GUInt32 i, j = Descriptor.MeshContours.size();
	GMeshEdge2D *e, *startEdge;

	for (i = 0; i < j; i++) {
		e = startEdge = Descriptor.MeshContours[i];
		do {
			InsertEventNoSort(e->Org(), Descriptor);
			e = e->Rprev();
		} while (e != startEdge);
	}
	Descriptor.PriorityTree.sort(SweepGreater);
}

// return true is Edge is left-going from Vertex
GBool GTesselator2D::IsLeftGoing(GMeshEdge2D *Edge, GMeshVertex2D *Vertex) {

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

GBool GTesselator2D::IsLeftGoingFast(GMeshEdge2D *Edge, GMeshVertex2D *Vertex) {

	if (Edge->Dest() == Vertex)
		return G_TRUE;
	return G_FALSE;
}

// return true is Edge is right-going from Vertex
GBool GTesselator2D::IsRightGoing(GMeshEdge2D *Edge, GMeshVertex2D *Vertex) {

	GInt32 ptCmp = PointCmp(Edge->Dest()->Position(), Vertex->Position());

	G_ASSERT(ptCmp != 0);
	if (ptCmp > 0)
		return G_TRUE;
	return G_FALSE;
}

GBool GTesselator2D::ProcessRightGoingEdges(GMeshVertex2D *EventVertex, GTessDescriptor& Descriptor) {

	GMeshEdge2D *outgoingEdge, *firstEdge;
	GMeshEdge2D *extEdge;
	GInt32 rightGoingFound;
	GBool resultFlag = G_FALSE;

	rightGoingFound = 0;
	// now we add to the edge dictionary all outgoing edges of eventVertex->MeshVertex
	firstEdge = outgoingEdge = EventVertex->Edge();
	do {
		if (IsRightGoing(outgoingEdge, EventVertex)) {
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

GMeshEdge2D *GTesselator2D::AddDictionaryEdge(GMeshEdge2D *MeshEdge, const GInt32 Flags, GBool& RevisitFlag,
											  GTessDescriptor& Descriptor) {

	G_ASSERT(MeshEdge != NULL);

	GBool alreadyExists;
	GInt32 intersected1, intersected2;
	GPoint2 intPoint1, intPoint2, minPoint, maxPoint;
	GAVLNode *above, *below;
	GBool revisitLocalFlag;
	GMeshToAVL *customData;
	GMeshEdge2D *meshEdge;

	// if edge was already inserted into dictionary, it cannot be inserted again
	customData = (GMeshToAVL *)MeshEdge->CustomData();
	if ((customData) && (customData->IsIntoDictionary))
		return NULL;

	// temporary extended edge just to make findNext and findPrev work right; it will be used also for
	// inserting it into dictionary
	GMeshEdge2D *newExtEdge = MeshEdge;

	// Bentley-Ottmann
	above = Descriptor.DictionaryTree.FindNext((void *)newExtEdge);
nextAbove:
	if (above) {
		meshEdge = (GMeshEdge2D *)above->CustomData();
		if (meshEdge->Dest() == Descriptor.CurrentEvent) {
			above = Descriptor.DictionaryTree.Next(above);
			goto nextAbove;
		}
	}
	below = Descriptor.DictionaryTree.FindPrev((void *)newExtEdge);
nextBelow:
	if (below) {
		meshEdge = (GMeshEdge2D *)below->CustomData();
		if (meshEdge->Dest() == Descriptor.CurrentEvent) {
			below = Descriptor.DictionaryTree.Prev(below);
			goto nextBelow;
		}
	}

	if (above) {
		revisitLocalFlag = G_FALSE;
		revisitLocalFlag = DoIntersection((GMeshEdge2D *)above->CustomData(), newExtEdge, intersected1, Descriptor);
		RevisitFlag |= revisitLocalFlag;
		if (revisitLocalFlag)
			return NULL;
	}

	if (below) {
		revisitLocalFlag = G_FALSE;
		revisitLocalFlag = DoIntersection(newExtEdge, (GMeshEdge2D *)below->CustomData(), intersected2, Descriptor);
		RevisitFlag |= revisitLocalFlag;			
		if (revisitLocalFlag)
			return NULL;
	}

	// now insert the specified (it could be modified by ManageIntersections, but its pointer
	// is still valid) edge into dictionary
	customData = new GMeshToAVL;
	customData->EdgeType = Flags;
	customData->Region = NULL;
	customData->IsIntoDictionary = G_TRUE;
	customData->CrossingNumber = -99;
	customData->AVLNode = Descriptor.DictionaryTree.Insert((void *)newExtEdge, alreadyExists);
	MeshEdge->SetCustomData((void *)customData);
	MeshEdge->Sym()->SetCustomData((void *)customData);
	Descriptor.ExtEdges.push_back(customData);
	return newExtEdge;
}

GMeshVertex2D *GTesselator2D::InsertEventNoSort(GMeshVertex2D *EventVertex, GTessDescriptor& Descriptor) {

	GExtVertex auxExtVertex, *newExtVertex;

	// just simply insert it into the events queue
	newExtVertex = new GExtVertex(UNDEFINED_VERTEX, EventVertex);
	Descriptor.PriorityTree.push_back(newExtVertex);
	Descriptor.ExtVertices.push_back(newExtVertex);
	return EventVertex;
}

GMeshVertex2D *GTesselator2D::InsertEventSort(GMeshVertex2D *EventVertex, GTessDescriptor& Descriptor) {

	GExtVertex *newExtVertex = new GExtVertex(UNDEFINED_VERTEX, EventVertex);
	Descriptor.ExtVertices.push_back(newExtVertex);

	std::list<GExtVertex *>::iterator pivotIt;
	pivotIt = std::lower_bound(Descriptor.PriorityTree.begin(), Descriptor.PriorityTree.end(),
							   newExtVertex, SweepGreater);
	Descriptor.PriorityTree.insert(pivotIt, newExtVertex);
	return EventVertex;
}

GMeshEdge2D *GTesselator2D::TraceLeftDiagonal(GMeshVertex2D *Origin, GMeshVertex2D *Destination,
											  GTessDescriptor& Descriptor) {

	GMeshEdge2D *newEdge, *e1, *e2;
	GReal l;

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
	customData->IsIntoDictionary = G_FALSE;
	customData->CrossingNumber = -99;
	customData->AVLNode = NULL;
	newEdge->SetCustomData((void *)customData);
	newEdge->Sym()->SetCustomData((void *)customData);
	Descriptor.ExtEdges.push_back(customData);
	return newEdge;
}

GMeshEdge2D *GTesselator2D::TraceRightDiagonal(GMeshVertex2D *Origin, GMeshVertex2D *Destination,
											   GTessDescriptor& Descriptor) {

	GMeshEdge2D *newEdge, *e1, *e2;
	GReal l;

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

	return newEdge;
}

GInt32 GTesselator2D::CheckIntersection(GMeshEdge2D *EdgeAbove, GMeshEdge2D *EdgeBelow,
										GPoint2& IntersectionPoint1, GPoint2& IntersectionPoint2) {

	G_ASSERT((EdgeAbove != NULL) && (EdgeBelow != NULL));
	GInt32 ptCmp1, ptCmp2, ptCmp3, ptCmp4;
	GUInt32 flags;
	GPoint2 intersectionPoint;
	GPoint2 intPointReal;
	GReal locParams[2];

	if (EdgeAbove == EdgeBelow)
		return G_FALSE;

	/*GPoint<GDouble, 2> o1((GDouble)EdgeAbove->Org()->Position()[G_X], (GDouble)EdgeAbove->Org()->Position()[G_Y]);
	GPoint<GDouble, 2> d1((GDouble)EdgeAbove->Dest()->Position()[G_X], (GDouble)EdgeAbove->Dest()->Position()[G_Y]);
	GPoint<GDouble, 2> o2((GDouble)EdgeBelow->Org()->Position()[G_X], (GDouble)EdgeBelow->Org()->Position()[G_Y]);
	GPoint<GDouble, 2> d2((GDouble)EdgeBelow->Dest()->Position()[G_X], (GDouble)EdgeBelow->Dest()->Position()[G_Y]);
	GLineSeg<GDouble, 2> ls1(o1, d1);
	GLineSeg<GDouble, 2> ls2(o2, d2);*/

	GPoint2 o1(EdgeAbove->Org()->Position());
	GPoint2 d1(EdgeAbove->Dest()->Position());
	GPoint2 o2(EdgeBelow->Org()->Position());
	GPoint2 d2(EdgeBelow->Dest()->Position());
	GLineSegment2 ls1(o1, d1);
	GLineSegment2 ls2(o2, d2);

	GBool found = Intersect(ls1, ls2, flags, locParams);

	if (found) { 
		if (flags & INFINITE_SOLUTIONS) {
			GPoint2 p = ls1.Origin() + locParams[0] * ls1.Direction();
			IntersectionPoint1.Set((GReal)p[G_X], (GReal)p[G_Y]);
			p = ls1.Origin() + locParams[1] * ls1.Direction();
			IntersectionPoint2.Set((GReal)p[G_X], (GReal)p[G_Y]);
			return DOUBLE_INTERSECTION;
		}
		intersectionPoint = ls1.Origin() + locParams[0] * ls1.Direction();
		ptCmp1 = PointCmp(intersectionPoint, o1);
		ptCmp2 = PointCmp(intersectionPoint, d1);
		ptCmp3 = PointCmp(intersectionPoint, o2);
		ptCmp4 = PointCmp(intersectionPoint, d2);

		if (ptCmp1 == 0 || ptCmp2 == 0) {
			if (ptCmp3 == 0 || ptCmp4 == 0)
				return NO_INTERSECTION;
			else {
				IntersectionPoint1.Set((GReal)intersectionPoint[G_X], (GReal)intersectionPoint[G_Y]);
				return DEGENERATE_INTERSECTION2;
			}
		}
		if (ptCmp3 == 0 || ptCmp4 == 0) {
			if (ptCmp1 == 0 || ptCmp2 == 0)
				return NO_INTERSECTION;
			else {
				IntersectionPoint1.Set((GReal)intersectionPoint[G_X], (GReal)intersectionPoint[G_Y]);
				return DEGENERATE_INTERSECTION1;
			}
		}
		IntersectionPoint1.Set((GReal)intersectionPoint[G_X], (GReal)intersectionPoint[G_Y]);
		return GOOD_INTERSECTION;
	}
	else
		return NO_INTERSECTION;
}

GBool GTesselator2D::DoIntersection(GMeshEdge2D *EdgeAbove, GMeshEdge2D *EdgeBelow, GInt32& IntersectionType,
									GTessDescriptor& Descriptor) {

	GPoint2 intPoint1, intPoint2;
	GInt32 intersected, ptCmp;
	GBool revisitLocalFlag;
	GMeshEdge2D *newEdge;

	revisitLocalFlag = G_FALSE;
	intersected = CheckIntersection(EdgeAbove, EdgeBelow, intPoint1, intPoint2);

	IntersectionType = intersected;
	if (intersected != NO_INTERSECTION) {
		if (intersected == GOOD_INTERSECTION) {
			ptCmp = PointCmp(intPoint1, Descriptor.CurrentEvent->Position());
			if (ptCmp == 0)
				revisitLocalFlag = G_TRUE;
			ManageIntersections(EdgeAbove, EdgeBelow, intPoint1, Descriptor);
		}
		else
		if (intersected == DEGENERATE_INTERSECTION2) {
			newEdge = ManageDegenerativeIntersections(EdgeBelow, EdgeAbove, intPoint1, Descriptor);
			ptCmp = PointCmp(intPoint1, Descriptor.CurrentEvent->Position());
			if (ptCmp == 0)
				revisitLocalFlag = G_TRUE;
		}
		else
		if (intersected == DEGENERATE_INTERSECTION1) {
			newEdge = ManageDegenerativeIntersections(EdgeAbove, EdgeBelow, intPoint1, Descriptor);
			ptCmp = PointCmp(intPoint1, Descriptor.CurrentEvent->Position());
			if (ptCmp == 0)
				revisitLocalFlag = G_TRUE;
		}
		// this is the case of a double intersection (overlapping edges)
		else {
			ptCmp = PointCmp(EdgeAbove->Org()->Position(), EdgeBelow->Org()->Position());
			if (ptCmp < 0)
				ManageOverlappingEdges(EdgeAbove, EdgeBelow, intPoint1, intPoint2, revisitLocalFlag, Descriptor);
			else
				ManageOverlappingEdges(EdgeBelow, EdgeAbove, intPoint1, intPoint2, revisitLocalFlag, Descriptor);
		}
	}
	return revisitLocalFlag;
}

void GTesselator2D::ManageIntersections(GMeshEdge2D *EdgeAbove, GMeshEdge2D *EdgeBelow,
										const GPoint2& IntersectionPoint, GTessDescriptor& Descriptor) {

	G_ASSERT((EdgeAbove != NULL) && (EdgeBelow != NULL));
	if (EdgeAbove == EdgeBelow)
		return;

	GMeshEdge2D *D;
	GMeshEdge2D *newEdgeAbove, *newEdgeBelow;
	GMeshVertex2D *newVertexAbove, *newVertexBelow;

	// lets split EdgeAbove: this involve adding 1 vertex and one edge
	D = EdgeAbove->Lnext();
	newVertexAbove = Descriptor.TargetMesh.AddVertex(IntersectionPoint);
	if (D != EdgeAbove->Sym()) {
		newEdgeAbove = Descriptor.TargetMesh.AddEdge()->Rot();
		GMesh2D::Splice(newEdgeAbove->Sym(), EdgeAbove->Sym());
		GMesh2D::Splice(newEdgeAbove, D);
		newEdgeAbove->SetDest(D->Org());
		Descriptor.TargetMesh.SetOrbitOrg(newEdgeAbove, newVertexAbove);
	}
	else {
		/*newEdgeAbove = gMesh->AddEdge();
		GMesh2D::Splice(EdgeAbove->Sym(), newEdgeAbove);
		newEdgeAbove->SetDest(EdgeAbove->Dest());
		gMesh->SetOrbitOrg(newEdgeAbove, newVertexAbove);*/
		G_ASSERT(0 == 1);
	}

	// lets split EdgeBelow: this involve adding 1 vertex and one edge
	D = EdgeBelow->Lnext();
	newVertexBelow = Descriptor.TargetMesh.AddVertex(IntersectionPoint);
	if (D != EdgeBelow->Sym()) {
		newEdgeBelow = Descriptor.TargetMesh.AddEdge()->Rot();
		GMesh2D::Splice(newEdgeBelow->Sym(), EdgeBelow->Sym());
		GMesh2D::Splice(newEdgeBelow, D);
		newEdgeBelow->SetDest(D->Org());
		Descriptor.TargetMesh.SetOrbitOrg(newEdgeBelow, newVertexBelow);
	}
	else {
		G_ASSERT(0 == 1);
		/*newEdgeBelow = gMesh->AddEdge();
		GMesh2D::Splice(EdgeBelow->Sym(), newEdgeBelow);
		newEdgeBelow->SetDest(EdgeBelow->Dest());
		gMesh->SetOrbitOrg(newEdgeBelow, newVertexBelow);*/
	}
	InsertEventSort(newVertexAbove, Descriptor);
	InsertEventSort(newVertexBelow, Descriptor);
}


GMeshEdge2D *GTesselator2D::ManageDegenerativeIntersections(GMeshEdge2D *Touched, GMeshEdge2D *UnTouched,
															const GPoint2& IntersectionPoint, GTessDescriptor& Descriptor) {

	GMeshEdge2D *N, *D;
	GMeshVertex2D *newVertex;
	
	D = Touched->Lnext();

	if ((Touched->Org()->Position() == UnTouched->Org()->Position()) &&
		(Touched->Dest()->Position() == UnTouched->Dest()->Position()))
		G_ASSERT(0 == 1);

	// create new vertex
	newVertex = Descriptor.TargetMesh.AddVertex(IntersectionPoint);
	// create new edge and make the connectivity right
	if (D != Touched->Sym()) {
		N = Descriptor.TargetMesh.AddEdge()->Rot();
		GMesh2D::Splice(N->Sym(), Touched->Sym());
		GMesh2D::Splice(N, D);
		N->SetDest(D->Org());
		Descriptor.TargetMesh.SetOrbitOrg(N, newVertex);
	}
	else {
		N = NULL;
		/*N = gMesh->AddEdge();
		GMesh2D::Splice(T->Sym(), N);
		N->SetDest(T->Dest());
		gMesh->SetOrbitOrg(N, newVertex);*/
		G_ASSERT(0 == 1);
	}
	InsertEventSort(newVertex, Descriptor);
	return N;
}

void GTesselator2D::ManageOverlappingEdges(GMeshEdge2D *Edge, GMeshEdge2D *EdgeToInsert,
										   const GPoint2& IntersectionPoint1, const GPoint2& IntersectionPoint2,
										   GBool& RevisitFlag, GTessDescriptor& Descriptor) {

	GInt32 ptCmp, ptCmp1, ptCmp2, ptCmp3, ptCmp4;
	GPoint2 minPoint, maxPoint;
	GMeshEdge2D *newEdge;

	ptCmp = PointCmp(IntersectionPoint1, IntersectionPoint2);
	// find the nearest intersection point
	if (ptCmp < 0) {
		minPoint = IntersectionPoint1;
		maxPoint = IntersectionPoint2;
	}
	else
	if (ptCmp > 0) {
		minPoint = IntersectionPoint2;
		maxPoint = IntersectionPoint1;
	}
	else
		// zero-length segment
		G_ASSERT(0 == 1);

	ptCmp1 = PointCmp(minPoint, Edge->Org()->Position());
	ptCmp2 = PointCmp(minPoint, EdgeToInsert->Org()->Position());
	ptCmp3 = PointCmp(maxPoint, Edge->Dest()->Position());
	ptCmp4 = PointCmp(maxPoint, EdgeToInsert->Dest()->Position());

	if (ptCmp1 == 0 && ptCmp2 == 0 && ptCmp3 == 0 && ptCmp4 == 0)
		// o---->  Edge
		// o---->  EdgeToInsert
		return;

	if (ptCmp4 != 0 && ptCmp1 != 0) {
		// o--------->         Edge
		//       o--------->   EdgeToInsert
		newEdge = ManageDegenerativeIntersections(EdgeToInsert, Edge, maxPoint, Descriptor);
		newEdge = ManageDegenerativeIntersections(Edge, EdgeToInsert, minPoint, Descriptor);
		ptCmp = PointCmp(minPoint, Descriptor.CurrentEvent->Position());
		if (ptCmp == 0)
			RevisitFlag |= G_TRUE;
	}
	else
	if (ptCmp2 == 0 && ptCmp1 == 0) {
		// find touched (to split) edge
		if (ptCmp4 == 0)
			// o------->   Edge
			// o---->      EdgeToInsert
			newEdge = ManageDegenerativeIntersections(Edge, EdgeToInsert, maxPoint, Descriptor);
		else
		if (ptCmp3 == 0)
			// o---->      Edge
			// o------->   EdgeToInsert
			newEdge = ManageDegenerativeIntersections(EdgeToInsert, Edge, maxPoint, Descriptor);
		else
			G_ASSERT(0 == 1);
	}
	else
	if (ptCmp3 == 0 && ptCmp4 == 0) {
		// o------->  Edge
		//    o---->  EdgeToInsert
		G_ASSERT(minPoint == EdgeToInsert->Org()->Position());
		newEdge = ManageDegenerativeIntersections(Edge, EdgeToInsert, minPoint, Descriptor);
		ptCmp = PointCmp(minPoint, Descriptor.CurrentEvent->Position());
		if (ptCmp == 0)
			RevisitFlag |= G_TRUE;
	}
	else
	if (ptCmp4 == 0 &&	ptCmp2 == 0) {
		// o--------->  Edge
		//    o-->      EdgeToInsert
		newEdge = ManageDegenerativeIntersections(Edge, EdgeToInsert, maxPoint, Descriptor);
		newEdge = ManageDegenerativeIntersections(Edge, EdgeToInsert, minPoint, Descriptor);
		ptCmp = PointCmp(minPoint, Descriptor.CurrentEvent->Position());
		if (ptCmp == 0)
			RevisitFlag |= G_TRUE;
	}
	else
		G_ASSERT(0 == 1);
}

GMeshVertex2D *GTesselator2D::MergeRings(GMeshVertex2D *Ring1Vertex, GMeshVertex2D *Ring2Vertex, GMesh2D *Mesh) {

	GUInt32 r1Count, r2Count, c;
	GMeshEdge2D *e, *startEdge, *destRingEdge, *tmpEdge;
	GMeshVertex2D *delVertex, *resVertex;

	if (Ring1Vertex == Ring2Vertex)
		return Ring1Vertex;

	// lets count how many edge goes from Ring1Vertex and from Ring2Vertex
	r1Count = Ring1Vertex->EdgesInRingCount();
	r2Count = Ring2Vertex->EdgesInRingCount();

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
		e = tmpEdge;
	}
	// insert startEdge
	DoInsertEdge(startEdge, destRingEdge, Mesh);

	Mesh->RemoveVertex(delVertex);
	return resVertex;
}

GMeshEdge2D *GTesselator2D::SafeRemoveEdgeFromVertex(GMeshEdge2D *Edge) {
	// replace the arbitrary edge with another edge (different from BadEdge) in the orbit use null
	// if this is the only edge assumes that the edge hasn't been actually removed yet
	GMeshEdge2D *edge, *startEdge;

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

void GTesselator2D::DoInsertEdge(GMeshEdge2D *EdgeToInsert, GMeshEdge2D *RingEdge, GMesh2D *Mesh) {

	GMeshEdge2D *e, *tmp, *rEdge = RingEdge;

	e = CCWSmallerAngleSpanEdge(rEdge, EdgeToInsert->Org()->Position(), EdgeToInsert->Dest()->Position());
	GMesh2D::Splice(EdgeToInsert, e);
	// check if e is still right
	if (e->Onext() != EdgeToInsert) {
		// we have to find in the RingEdge's ring who have as Onext EdgeToInsert
		tmp = rEdge;
		while (1) {
			if (tmp->Onext() == EdgeToInsert) {
				GMesh2D::Splice(tmp, e);
				goto setOrbit;
			}
			tmp = tmp->Onext();
		}
	}
setOrbit:
	Mesh->SetOrbitOrg(e, e->Org());
}


GBool GTesselator2D::CloseRegion(GMeshEdge2D *UpperEdge, GDynArray<GActiveRegion *>& ActiveRegions,
								 GTessDescriptor& Descriptor) {

	GMeshEdge2D *e;
	GInt32 correctCrossNumber;
	GActiveRegion *region;
	GInt32 ptCmp;
	GMeshToAVL *customData;
	GAVLNode *n;

	customData = (GMeshToAVL *)UpperEdge->CustomData();

	if (customData->EdgeType == NORMAL_EDGE)
		correctCrossNumber = customData->CrossingNumber;
	else {
		G_ASSERT(customData->EdgeType == RIGHT_ADDED_EDGE);
		n = Descriptor.DictionaryTree.Next(customData->AVLNode);
		G_ASSERT(n);
		e = (GMeshEdge2D *)n->CustomData();
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
		e = UpperEdge->Rnext();
		customData = (GMeshToAVL *)e->CustomData();
		while (customData && customData->EdgeType == RIGHT_ADDED_EDGE) {
			e = e->Rnext();
			customData = (GMeshToAVL *)e->CustomData();
		}
		G_ASSERT(customData);
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
	customData->Region = region;
	return G_TRUE;
}

GBool GTesselator2D::CloseRegions(GMeshVertex2D *EventVertex, GDynArray<GActiveRegion *>& ActiveRegions,
								  GAVLNode **UpperBounder, GAVLNode **LowerBounder,
								  GBool& RevisitFlag, GTessDescriptor& Descriptor) {

	GAVLNode *nUpper, *nLower, *regionUpperNode, *regionLowerNode;
	GAVLNode *upperBO;
	GMeshEdge2D *upperExtEdge, *lowerExtEdge, *extAbove, *extBelow;
	GReal area, oldArea, sweepDist;
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
		upperExtEdge = (GMeshEdge2D *)nUpper->CustomData();
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
				lowerExtEdge = (GMeshEdge2D *)nLower->CustomData();
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
				extAbove = (GMeshEdge2D *)upperBO->CustomData();
				extBelow = (GMeshEdge2D *)nLower->CustomData();
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

GBool GTesselator2D::PatchRightDiagonal(GMeshVertex2D *Event, GAVLNode *UpperBounder, GAVLNode *LowerBounder,
										GTessDescriptor& Descriptor) {

	GMeshEdge2D *startEdge, *e, *tmpEdge, *aboveUpperEdge, *belowLowerEdge;
	GInt32 rCount = 0, delCount = 0, ptCmp2;
	GPoint2 minDest;
	GBool flag, resultFlag;
	GReal area;
	GMeshVertex2D *v = NULL, *rightMeshVertex;
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
		aboveUpperEdge = (GMeshEdge2D *)UpperBounder->CustomData();
		belowLowerEdge = (GMeshEdge2D *)LowerBounder->CustomData();
		// we must trace a non intersecting right diagonal; to do this, we just pick the next event
		// point bounded by aboveUpperExtEdge and belowLowerExtEdge; in general, it's not safe to
		// find the rightmost point of AboveUpper segment and BelowLower segment
		std::list<GExtVertex *>::iterator it = Descriptor.PriorityTree.begin();
		rightMeshVertex = NULL;

		do {
			it++;
			extVertex = *it;
			G_ASSERT(extVertex != NULL);
			// we have to check if the event point is located at the rightside of above edge
			// and at the leftside of below edge
			if ((CounterClockWiseOrAligned(belowLowerEdge->Org()->Position(),
										  belowLowerEdge->Dest()->Position(),
										  extVertex->MeshVertex->Position())) &&
				(ClockWiseOrAligned(aboveUpperEdge->Org()->Position(),
									aboveUpperEdge->Dest()->Position(),
									extVertex->MeshVertex->Position())))
				rightMeshVertex = extVertex->MeshVertex;
			} while(rightMeshVertex == NULL);
		// trace diagonal between eventVertex->MeshVertex and rightMeshVertex
		// AND WE MUST ADD THIS DIAGONAL TO THE EDGE DICTIONARY (because it could close
		// a region. Its flag MUST be RIGHT_ADDED_EDGE)
		GMeshEdge2D *newEdge = TraceRightDiagonal(Event, rightMeshVertex, Descriptor);
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
	GMeshVertex2D *queueVertex;
	GInt32 ptCmp;
	std::list<GExtVertex *>::iterator it = Descriptor.PriorityTree.begin();

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

void GTesselator2D::SimplifyEdges(GMeshVertex2D *Event, GDynArray<GActiveRegion *>& ActiveRegions,
								  GTessDescriptor& Descriptor) {

	GMeshEdge2D *firstEdge, *outgoingEdge, *tmpEdge, *auxEdge;
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
			G_ASSERT(data2 == data2Sym);
			if ((data2) && (data2->IsIntoDictionary)) {
				G_ASSERT(data2->AVLNode != NULL);
				data2->IsIntoDictionary = G_FALSE;
				data2 = (GMeshToAVL *)outgoingEdge->Sym()->CustomData();
				data2->IsIntoDictionary = G_FALSE;
				data2 = (GMeshToAVL *)outgoingEdge->CustomData();
				Descriptor.DictionaryTree.DeleteNode(data2->AVLNode);
			}

			tmpEdge = outgoingEdge->Oprev();
			if (!IsRightGoing(tmpEdge, Event)) {
				ptCmp = PointCmp(tmpEdge->Dest()->Position(), outgoingEdge->Dest()->Position());
				if (ptCmp == 0) {
					// extract descriptors
					data1 = (GMeshToAVL *)tmpEdge->CustomData();
					data1Sym = (GMeshToAVL *)tmpEdge->Sym()->CustomData();
					G_ASSERT(data1 == data1Sym);

					if ((data1) && (data1->IsIntoDictionary)) {
						G_ASSERT(data1->AVLNode != NULL);
						data1->IsIntoDictionary = G_FALSE;
						data1 = (GMeshToAVL *)tmpEdge->Sym()->CustomData();
						data1->IsIntoDictionary = G_FALSE;
						data1 = (GMeshToAVL *)tmpEdge->CustomData();
						Descriptor.DictionaryTree.DeleteNode(data1->AVLNode);
					}

					if (data1 && data2) {
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
					if (data1) {
						SafeRemoveEdgeFromVertex(tmpEdge);
						SafeRemoveEdgeFromVertex(tmpEdge->Sym());
						Descriptor.TargetMesh.DetachEdge(tmpEdge);
						ringCount--;
					}
					if (data2) {
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

	j = ActiveRegions.size();
	for (i = 0; i < j; i++) {
		data1 = (GMeshToAVL *)ActiveRegions[i]->MeshUpperEdge->CustomData();
		G_ASSERT(data1 != NULL);
		if (ActiveRegions[i]->Valid) {
			tmpEdge = ActiveRegions[i]->MeshUpperEdge->Rprev();
			if (!IsLeftGoing(tmpEdge, Event) || 
				!ClockWise(ActiveRegions[i]->MeshUpperEdge->Org()->Position(), Event->Position(), tmpEdge->Dest()->Position()))
				ActiveRegions[i]->Valid = G_FALSE;
		}
	}
}


void GTesselator2D::TessellateMonotoneRegion(const GActiveRegion* Region, GDynArray<GPoint2>& Points,
											 GTessDescriptor& Descriptor) {

	GMeshEdge2D *up, *lo;
	GMeshEdge2D *tempEdge = NULL;

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
					(ClockWiseOrAligned(lo->Org()->Position(), lo->Lnext()->Dest()->Position(), lo->Dest()->Position())))) {
					
				// create new edge
				tempEdge = Descriptor.TargetMesh.AddEdge();
				G_ASSERT(tempEdge != NULL);

				// for drawing operations
				Points.push_back(lo->Dest()->Position());
				Points.push_back(lo->Lnext()->Dest()->Position());
				Points.push_back(lo->Org()->Position());

				// we have to trace a diagonal from lo->Lnext->Dest to lo->Org
				GMesh2D::Splice(lo, tempEdge->Sym());
				GMesh2D::Splice(lo->Lnext()->Lnext(), tempEdge);
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
					(CounterClockWiseOrAligned(up->Dest()->Position(), up->Lprev()->Org()->Position(), up->Org()->Position())))) {
				
				// create new edge
				tempEdge = Descriptor.TargetMesh.AddEdge();
				G_ASSERT(tempEdge != NULL);

				// for drawing operations
				Points.push_back(up->Lprev()->Org()->Position());
				Points.push_back(up->Org()->Position());
				Points.push_back(up->Dest()->Position());

				// we have to trace a diagonal from up->Dest to up->Lprev->Org
				GMesh2D::Splice(up->Lprev(), tempEdge->Sym());
				GMesh2D::Splice(up->Lnext(), tempEdge);
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
		GMeshEdge2D *tempEdge = Descriptor.TargetMesh.AddEdge();
		G_ASSERT(tempEdge != NULL);

		// for drawing operations
		Points.push_back(lo->Dest()->Position());
		Points.push_back(lo->Lnext()->Dest()->Position());
		Points.push_back(lo->Org()->Position());
		// we have to trace a diagonal from lo->Lnext->Dest to lo->Org
		GMesh2D::Splice(lo, tempEdge->Sym());
		GMesh2D::Splice(lo->Lnext()->Lnext(), tempEdge);
		tempEdge->SetOrg(lo->Lnext()->Dest());
		tempEdge->SetDest(lo->Org());
		lo = tempEdge->Sym();
	}

	Points.push_back(lo->Org()->Position());
	Points.push_back(lo->Dest()->Position());
	Points.push_back(lo->Lnext()->Dest()->Position());
}


GBool GTesselator2D::SweepEvent(GExtVertex *Event, GTessDescriptor& Descriptor) {

	GBool revisitEvent, traceLeftDiag;
	GUInt32 i, j;
	GExtVertex *extVertex;
	GMeshVertex2D *leftMeshVertex, *startLeftDiag, *endLeftDiag;
	GMeshEdge2D *regionUpperEdge, *regionLowerEdge;
	GDynArray<GActiveRegion *> tmpRegions;
	GAVLNode *upperBounder, *lowerBounder;
	GActiveRegion *ar;

	if (!Event)
		return G_FALSE;

reSweep:
	revisitEvent = G_FALSE;
	// clear temporary regions
	j = tmpRegions.size();
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

	// close regions
	upperBounder = lowerBounder = NULL;
	traceLeftDiag = CloseRegions(Descriptor.CurrentEvent, tmpRegions, &upperBounder, &lowerBounder, revisitEvent, Descriptor);
	if (revisitEvent)
		goto reSweep;
	// trace a left diagonal if there weren't left-going edges and the event is inside
	if (traceLeftDiag && upperBounder && lowerBounder) {
		regionUpperEdge = (GMeshEdge2D *)upperBounder->CustomData();
		regionLowerEdge = (GMeshEdge2D *)lowerBounder->CustomData();
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
	j = tmpRegions.size();
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
	GMeshEdge2D *e, *startEdge, *e2;
	GUInt32 i, j = ActiveRegions.size(), invalidCount = 0;
	GUInt32 k;

	if (ActiveRegions.size() == 0)
		return 0;

	if (Fast) {
		startEdge = e = Descriptor.LastRegionEdge;
		do {
			j = ActiveRegions.size();
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
GMeshEdge2D *GTesselator2D::CCWSmallerAngleSpanEdge(GMeshEdge2D *RingEdge,
													const GPoint2& Origin, const GPoint2& Destination) {

	GMeshEdge2D *e1, *e2, *eGood, *startEdge;
	GInt32 i, ptCmp;
	GPoint2 a, c;

	e1 = startEdge = RingEdge;
	e2 = e1->Oprev();
	a = e1->Dest()->Position();
	c = e2->Dest()->Position();

	// we wanna find the edge (that belongs to the Destination ring) that span the smaller angle
	// in counterclockwise direction to meet the diagonal (actually its symmetric)
	i = CCWSmallerAngleSpan(Origin, a, c, Destination);
	if (i == 1)
		eGood = e1;
	else {
		eGood = e2;
		a = c;
	}
	e2 = e2->Oprev();
	while (e2 != startEdge) {
		c = e2->Dest()->Position();
		i = CCWSmallerAngleSpan(Origin, a, c, Destination);
		if (i != 1) {
			eGood = e2;
			a = c;
		}
		e2 = e2->Oprev();
	}
	// it's possible that there are overlapping edges
	e1 = eGood;
	do {
		e1 = e1->Onext();
		ptCmp = PointCmp(eGood->Dest()->Position(), e1->Dest()->Position());
		if (ptCmp == 0)
			eGood = e1;
	} while (ptCmp == 0 && e1 != eGood);
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

};	// end namespace Amanith
