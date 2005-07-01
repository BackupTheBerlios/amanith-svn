/****************************************************************************
** $file: amanith/src/2d/gmesh2d.cpp   0.1.0.0   edited Jun 30 08:00
**
** 2D Mesh definition.
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

#include "amanith/support/gutilities.h"
#include "amanith/2d/gmesh2d.h"
#include "amanith/geometry/garea.h"
#include "amanith/geometry/gaabox.h"
#include <algorithm>
#include <set>
#include <utility>

/*!
	\file gmesh2d.cpp
	\brief GMesh2D classes implementation file.
*/

namespace Amanith {

class GMeshSector;

// arc of consecutive edges emanating from a vertex in counterclockwise order
// (a linked list of pointers to other vertex)
// http://www-2.cs.cmu.edu/afs/andrew/scs/cs/15-463/2001/pub/src/a2/quadedge.html
struct GMeshArc {
	GDynArray<GMeshSector> Sectors;
	GBool Debug;
};
// list of arcs
typedef GDynArray<GMeshArc> GMeshArcList;
// For example, for vertex v below,
//
//       c------ b------i
//      / \     /      /
//     /   \   /      /
//    /     \ /      /
//   d------ v -----a--h
//    \     / \        |
//     \   /   \       |
//      \ /     \      |
//       e-------f-----g
//
// some valid Arcs are the lists (a,b), (a,b,c), (b,c), (c,d),
// (f,a), (e,f,a,b), etc. because those are the other endpoints of
// edges emanating from v, in counterclockwise (ccw) order.
// An arc always consists of at least two vertexes.
// A valid Arclist is any set of disjoint arcs, in arbitrary order.
// When done, the Arclist for this vertex would be a single Arc.
// It would be a cyclic permutation of (a,b,c,d,e,f).
class GImpExpMeshVertex;
class GImpExpMeshFace;

class GMeshSector {
public:
	GImpExpMeshVertex *gFirstCCW;		// first ccw vertex
	GImpExpMeshFace *gFace;				// intervening face
	GImpExpMeshVertex *gSecondCCW;		// second ccw vertex

	GMeshSector(GImpExpMeshVertex *FirstCCW, GImpExpMeshFace *Face, GImpExpMeshVertex *SecondCCW) {
		this->gFirstCCW = FirstCCW;
		this->gFace = Face;
		this->gSecondCCW = SecondCCW;
	}
};

class GImpExpMeshVertex {
public:
	GBool gDone;				// is topology fully set & arclist complete?
	GPoint2 gPosition;
	GMeshArcList gArcs;		// info about the vertexes adjacent to this one
	GMeshVertex2D *gVertex;		// final vertex in mesh, null if not id. yet
	GBool gInstantiated;		// true if identified and instantiated
};

class GImpExpMeshFace {
public:
	GDynArray<GImpExpMeshVertex *> gVertexList;
	GMeshFace2D *gMeshFace;

	GImpExpMeshFace() {
	}
	~GImpExpMeshFace() {
	}
};

// return a face that can be used to instantiate a given GImpExpMeshFace
// Mesh -> the mesh to get the face from (must be non-null)
// Face -> GImpExpMeshFace to get the face for (must be non-null)
// <- a face that can be used to instantiate Face; null if none are available
static GMeshFace2D *FindFace(GMesh2D *Mesh, GImpExpMeshFace *Face) {

	G_ASSERT(Mesh != NULL);
	G_ASSERT(Face != NULL);
	GUInt32 i, j = Face->gVertexList.size();
	// build a list of all identified vertexes of Face
	GDynArray<GMeshVertex2D *> tmpVerts;
	GMeshVertex2D *v;
	for (i = 0; i < j; i++) {
		v = Face->gVertexList[i]->gVertex;
		// if v is not NULL is an identified vertex
		if (v)
			tmpVerts.push_back(v);
	}
	// locate and unused face in the mesh that include all the identified vertexes of Face
	GMeshFaceIterator2D iterator(Mesh);
	GMeshFace2D *f;
	while ((f = iterator.Next()) != NULL)
		// CustomData will be null for every unidentified (unfinished) face
		if ((f->CustomData() == NULL) && (f->HasVertices(tmpVerts)))
			return f;
	return NULL;
}

static GBool ClosedPolyhedron(GDynArray<GImpExpMeshVertex>& Verts) {

    // check to see if polyhedron is closed
    GUInt32 i, j = Verts.size();

    for (i = 0; i < j; i++) {
		//GMeshArcList &a = Verts[i].gArcs;
		if (Verts[i].gArcs.size() != 1)
			return G_FALSE;

		 GBool b = Verts[i].gDone;
		 if (b) {
			 Verts[i].gDone = G_TRUE;
		 }
		/*if (!Verts[i].gDone || Verts[i].gArcs.size() != 1) {
			// unused vertex
			if (Verts[i].gArcs.size() == 0)
				return G_FALSE;
			else
			// vertex is not surrounded by polygons
			if (!Verts[i].gDone)
				return G_FALSE;
			else
				// repeated face
				return G_FALSE;
		}*/
    }
	return G_TRUE;
}

// instantiate a given GImpExpMeshFace in a given mesh by identifying its vertexes
// Mesh -> the mesh to instantiate the face in (must be non-null)
// Face -> the GImpExpMeshFace to instantiate (must be non-null)
static void MakeFace(GMesh2D *Mesh, GImpExpMeshFace *Face) {

	G_ASSERT(Mesh != NULL);
	G_ASSERT(Face != NULL);

	// get the face to use for the Face
	GMeshFace2D *face = FindFace(Mesh, Face);
	G_ASSERT(face != NULL);
	// connect all pairs of identified vertexes on the face, as necessary
	{
		GDynArray<GImpExpMeshVertex *>::iterator vi;
		vi = Face->gVertexList.begin();

		for (; vi != Face->gVertexList.end(); ++vi) {

			GMeshVertex2D *vertex1 = (*vi)->gVertex;
			GMeshVertex2D *vertex2;

			if (vertex1) {
				// find the next identified vertex, even if just itself
				GDynArray<GImpExpMeshVertex *>::iterator vj = vi;
				for (;;) {
					vj++;
					if (vj == Face->gVertexList.end())
						vj = Face->gVertexList.begin();
					vertex2 = (*vj)->gVertex;
					if (vertex2 != NULL)
						break;
				}
				// connect the vertexes, if necessary
				if (!GMesh2D::IsConnected(vertex1, vertex2, face))
					(void)Mesh->MakeFaceEdge(face, vertex1, vertex2)->Right();
			}
		}
	}
	// find the first identified vertex
	GDynArray<GImpExpMeshVertex *>::iterator vi0 = Face->gVertexList.begin();
	while ((*vi0)->gVertex == NULL)
		vi0++;
	// identify all the following and preceding vertexes
	GDynArray<GImpExpMeshVertex *>::iterator vi = vi0;
	GMeshVertex2D *vertex = (*vi0)->gVertex;
	for (;;) {
		vi++;
		if (vi == Face->gVertexList.end())
			vi = Face->gVertexList.begin();
		if (vi == vi0)
			break;
		GImpExpMeshVertex *v = (*vi);
		if (v->gVertex == NULL) {
			GMeshFace2D *right = GMesh2D::RightFace(vertex, face);
			G_ASSERT(right != NULL);
			v->gVertex = Mesh->MakeVertexEdge(vertex, face, right)->Dest();
			v->gVertex->SetPosition(v->gPosition);
		}
		vertex = v->gVertex;
	}
	// the face is now instantiated
	Face->gMeshFace = face;
	face->SetCustomData(Face);
}

// instantiate a given identified GImpExpMeshVertex in a given mesh by instantiating its adjacent faces
// Mesh -> the mesh to instantiate the GImpExpMeshVertex in (must be non-null)
// Vert -> the GImpExpMeshVertex to instantiate (must be non-null)
static void MakeVertex(GMesh2D *Mesh, GImpExpMeshVertex *Vert) {

	GImpExpMeshFace *f;

	G_ASSERT(Mesh != NULL);
	G_ASSERT(Vert != NULL);
	// find the first sector with an identified FirstCCW vertex
	GMeshArc &arc = Vert->gArcs.front();
	GDynArray<GMeshSector>::iterator it = arc.Sectors.begin();
	while (it->gFirstCCW->gVertex == NULL)
		it++;
	// instantiate all following sectors of the vertex in counterclockwise order
	GDynArray<GMeshSector>::iterator it2 = it;
	do {
		f = it2->gFace;
		if (f->gMeshFace == NULL)
			MakeFace(Mesh, f);
		it2++;
		// make a circular tour
		if (it2 == arc.Sectors.end())
			it2 = arc.Sectors.begin();
	} while (it2 != it);
	// the vertex is now instantiated
	Vert->gInstantiated = G_TRUE;
}

// Merge the arc (p, q) into the list of arcs around vertex v.
static void MergeArc(GImpExpMeshVertex *v, GImpExpMeshVertex *p, GImpExpMeshVertex *q, GImpExpMeshFace *f) {

    // Cases:
    //  1. ( bef &&  aft) it connects two existing arcs
    //  2. ( bef && !aft) it goes on the end of an existing arc
    //  3. (!bef &&  aft) it goes on the beginning of an existing arc
    //  4. (!bef && !aft) it does not connect with an existing arc
    GMeshArc *bef = NULL, *aft = NULL;
    GMeshSector sector(p, f, q);
	GDynArray<GMeshArc>::iterator a;
	GDynArray<GMeshArc>::iterator delItem;

    for (a = v->gArcs.begin(); a != v->gArcs.end(); ++a) {
		GImpExpMeshVertex *w;

		w = a->Sectors.back().gSecondCCW;
		if (w == p)
			bef = &(*a);
		w = a->Sectors.front().gFirstCCW;
		if (w == q) {
			aft = &(*a);
			delItem = a;
		}
    }
    // now concatenate the three arcs bef, (p,q), and aft
    // where bef and aft might be null
    if (bef) {
		if (aft) {
			// 1. ( bef &&  aft) it connects two existing arcs
			bef->Sectors.push_back(sector);		// insert new sector
			if (bef == aft) {
				// done with vertex! connecting these would make arc circular
				v->gDone = G_TRUE;
				return;
			}
			// now we'll merge two arcs in the arclist (remove following arc, and concatenate it into previous)
			v->gArcs.erase(delItem);
			// concatenation
			GUInt32 i, j = aft->Sectors.size();
			for (i = 0; i < j; i++)
				bef->Sectors.push_back((*aft).Sectors[i]);			
		}
		else
			// 2. ( bef && !aft) it goes on the end of existing arc
			bef->Sectors.push_back(sector);
	}
    else {
		// 3. (!bef &&  aft) it goes on beg. of existing arc
		if (aft)
			aft->Sectors.insert(aft->Sectors.begin(), sector);
		else {
			// 4. (!bef && !aft) it doesn't connect w. existing arc
			GMeshArc arc;
			arc.Sectors.push_back(sector);
			v->gArcs.push_back(arc);
		}
    }
}

static void AddArcs(GDynArray<GImpExpMeshVertex *>& vlist, GImpExpMeshFace *f) {

    // vlist is not a circular list, but we need to step through all
    // consecutive triples as if it were circular
    GImpExpMeshVertex *u, *v, *w;
	GUInt32 i, j;

	j = vlist.size();
	for (i = 0; i < j; i++) {
		u = vlist[i % j];
		v = vlist[(i + 1) % j];
		w = vlist[(i + 2) % j];
		MergeArc(v, w, u, f);
	}
}

static GMesh2D *BuildQuadedge(GMesh2D *mesh, GDynArray<GImpExpMeshVertex>& verts) {

	// check for a good polyhedron
	ClosedPolyhedron(verts);

	mesh->AddSubManifold();
	GMeshVertex2D *vertex1 = mesh->Vertex(0);

	// instantiate a face of the initial vertex
	GImpExpMeshVertex *v = &verts[0];
	v->gVertex = vertex1;
	v->gVertex->SetPosition(v->gPosition);
	GImpExpMeshFace *f = v->gArcs[0].Sectors[0].gFace;
	MakeFace(mesh, f);
	// instantiate identified vertexes until all are instantiated
	for (;;) {
		GBool instantiated = G_TRUE;
		GInt32 j = (GInt32)verts.size();
		for (GInt32 i = 0; i < j; i++) {
			
			GImpExpMeshVertex *v = &verts[i];
			if (v->gVertex != NULL && !v->gInstantiated)
				MakeVertex(mesh, v);
			instantiated &= v->gInstantiated;
		}
		if (instantiated)
			break;
	}
	// reset the data pointers of all faces
	GMeshFaceIterator2D iterator(mesh);
	GMeshFace2D *face;
	while ((face = iterator.Next()) != NULL)
		face->SetCustomData(NULL);
	return mesh;
}


// *********************************************************************
//                             GMeshVertex2D
// *********************************************************************

// add a given outgoing edge to this vertex (an edge whose origin is this vertex). MUST be non-null
void GMeshVertex2D::AddEdge(GMeshEdge2D* NewEdge) {

	G_ASSERT(NewEdge != NULL);
	// only keep track of one edge in the orbit--this one is as good as any
	gEdge = NewEdge;
}

// remove a given outgoing edge from this vertex (an edge whose origin is no longer at this vertex)
// Edge must be non-null
void GMeshVertex2D::RemoveEdge(GMeshEdge2D* Edge) {

	G_ASSERT(Edge != NULL);
	// replace the arbitrary edge with another edge in the orbit use null if this is the only edge
	// assumes that the edge hasn't been actually removed yet
	GMeshEdge2D *next = Edge->Onext();
	gEdge = next != Edge ? next : NULL;
}

// constructor (initialize this vertex at the origin with no outgoing edges)
GMeshVertex2D::GMeshVertex2D(GMesh2D *Mesh) {

	G_ASSERT(Mesh != NULL);

	gPosition.Set(0, 0);
	gMesh = Mesh;
	gCustomData = NULL;
	gEdge = NULL;
}

// destructor
GMeshVertex2D::~GMeshVertex2D() {
}

// counts how many edges are in the origin ring specified by this vertex
GUInt32 GMeshVertex2D::EdgesInRingCount() {

	// lets count how many edge goes from this vertex
	GUInt32 rCount = 1;
	// pick an outgoing edge
	GMeshEdge2D *startEdge = this->Edge();
	GMeshEdge2D *e = startEdge->Onext();
	// lets visit origin orbit
	while (e != startEdge) {
		rCount++;
		e = e->Onext();
	}
	return rCount;
}

// *********************************************************************
//                        GMeshVertexEdgeIterator2D
// *********************************************************************

// return the next edge of this edge iterator, if any. Null if none
GMeshEdge2D *GMeshVertexEdgeIterator2D::Next() {
	// check for degeneracy or exhausted iteration
	GMeshEdge2D *current = gEdge;

	if (!current)
		return NULL;
	// get the next edge in the counterclockwise orbit of the vertex, but
	// return the current edge; reset to null if we've come back to the start
	GMeshEdge2D *next = current->Onext();
	gEdge = next != gStart ? next : NULL;
	return current;
}


// *********************************************************************
//                              GMeshEdge
// *********************************************************************

// return the mesh this edge belongs to
GMesh2D* GMeshEdge2D::Mesh() {

	GQuadEdge2D *q = (GQuadEdge2D*)(this - this->gIndex);
	G_ASSERT(q != NULL);
	return q->Mesh();
}

// change the origin vertex of this edge to a given vertex (null if currently unknown)
void GMeshEdge2D::SetOrg(GMeshVertex2D *NewOrg) {
	
	// add this edge to the (vertex) orbit of _org_
	gVertex = NewOrg;
	NewOrg->AddEdge(this);
}

// change the destination vertex of this edge to a given vertex (null if currently unknown)
void GMeshEdge2D::SetDest(GMeshVertex2D *NewDest) {

	// add this edge to the (vertex) orbit of _dest_
	Sym()->gVertex = NewDest;
	NewDest->AddEdge(Sym());
}

// change the left face of this edge to a given face (null if currently unknown)
void GMeshEdge2D::SetLeft(GMeshFace2D *NewLeft) {

	// add this edge to the (face) orbit of _left_
	Rot()->gFace = NewLeft;
	NewLeft->AddEdge(this);
}

// change the right face of this edge to a given face (null if currently unknown)
void GMeshEdge2D::SetRight(GMeshFace2D *NewRight) {

	// add this edge to the (face) orbit of _right_
	InvRot()->gFace = NewRight;
	NewRight->AddEdge(Sym());
}

// determines if the point P is on the edge. The point is considered on if it is in the EPS-neighborhood
// of the edge
GBool GMeshEdge2D::IsOnEdge(const GPoint2& P, const GReal Tolerance) {

	GReal t1, t2;
	const GPoint2& o = Org()->Position();
	const GPoint2& d = Dest()->Position();

	t1 = Length(P - o);
	t2 = Length(P - d);
	if (t1 < Tolerance || t2 < Tolerance)
	    return G_TRUE;

	GVector2 t = d - o;
	GReal t3 = t.Length();
	if (t1 > t3 || t2 > t3)
	    return G_FALSE;
	// computes the normalized line equation through the points Org and Dest
	GReal a = t[G_Y] / t3;
	GReal b = -t[G_X] / t3;
	GReal c = -(a * o[G_X] + b * o[G_Y]);
	GReal eval = a * P[G_X] + b * P[G_Y] + c;
	return (GMath::Abs(eval) < Tolerance);
}

// *********************************************************************
//                              GMeshFace
// *********************************************************************
// constructor; it initialize this face with no adjacent edges (must be non-null)
GMeshFace2D::GMeshFace2D(GMesh2D *Mesh) {

	G_ASSERT(Mesh != NULL);
	gMesh = Mesh;
	gCustomData = NULL;
	gEdge = NULL;
}

// destructor
GMeshFace2D::~GMeshFace2D() {
}

// add a given adjacent edge to this face (must be non-null)
void GMeshFace2D::AddEdge(GMeshEdge2D *Edge) {
	
	G_ASSERT(Edge != NULL);
	// only keep track of one edge in the orbit--this one is as good as any
	this->gEdge = Edge;
}

// remove a given adjacent from this face (must be non-null)
void GMeshFace2D::RemoveEdge(GMeshEdge2D *Edge) {

	G_ASSERT(Edge != NULL);
	// replace the arbitrary edge with another edge in the orbit, use null if this is the only edge
	// assumes that the edge hasn't been actually removed yet
	GMeshEdge2D *next = Edge->Onext();
	gEdge = next != Edge ? next : NULL;
}

// return true if a given vertex is adjacent to this face
GBool GMeshFace2D::HasVertex(const GMeshVertex2D *Vertex) {

	G_ASSERT(Vertex != NULL);
	// check the origin vertex of each edge on the face
	GMeshFaceEdgeIterator2D edges(this);
	GMeshEdge2D *edge;
	while ((edge = edges.Next()) != NULL)
		if (edge->Org() == Vertex)
			return G_TRUE;
	return G_FALSE;
}

// return true if the face includes all the vertexes specified
GBool GMeshFace2D::HasVertices(const GDynArray<GMeshVertex2D *> VertsArray) {

	GUInt32 i, j = VertsArray.size();
	const GMeshVertex2D *v;

	for (i = 0; i < j; i++) {
		v = VertsArray[i];
		if ((v) && !HasVertex(v))
			return G_FALSE;
	}
	return G_TRUE;
}

// *********************************************************************
//                                GMesh
// *********************************************************************

// get Index-th vertex (null if Index is out of range)
GMeshVertex2D* GMesh2D::Vertex(const GUInt32 Index) {

	if (Index >= gVertices.size())
		return NULL;
	return gVertices[Index];
}

// get Index-th face (null if Index is out of range)
GMeshFace2D* GMesh2D::Face(const GUInt32 Index) {

	if (Index >= gFaces.size())
		return NULL;
	return gFaces[Index];
}

// add a new vertex
GMeshVertex2D *GMesh2D::AddVertex() {

	GMeshVertex2D *v = new GMeshVertex2D(this);
	gVertices.push_back(v);
	return v;
}

GMeshVertex2D *GMesh2D::AddVertex(const GPoint2& Position) {

	GMeshVertex2D *v = new GMeshVertex2D(this);
	v->SetPosition(Position);
	gVertices.push_back(v);
	return v;
}

// add a new face
GMeshFace2D *GMesh2D::AddFace() {

	GMeshFace2D *f = new GMeshFace2D(this);
	gFaces.push_back(f);
	return f;
}

// add a new edge
GMeshEdge2D *GMesh2D::AddEdge() {

	GQuadEdge2D *q = new GQuadEdge2D(this);
	gQuadEdges.push_back(q);
	return q->Edges();
}

// add a new sub manifold
GMeshEdge2D *GMesh2D::AddSubManifold() {

	// - create a looping edge that connects to itself at a single vertex
	// - the edge delimits two faces
	// - this is the smallest mesh that is consistent with our invariants
	GMeshVertex2D *vertex = AddVertex();
	GMeshFace2D *left = AddFace();
	GMeshFace2D *right = AddFace();
	GMeshEdge2D *edge = AddEdge()->InvRot();
	edge->SetOrg(vertex);
	edge->SetDest(vertex);
	edge->SetLeft(left);
	edge->SetRight(right);
	return edge;
}

// constructor
GMesh2D::GMesh2D() : GElement() {
}

GMesh2D::GMesh2D(const GElement* Owner) : GElement(Owner) {
}

// destructor
GMesh2D::~GMesh2D() {

	// just clear all memory
	Clear();
}


typedef std::pair<const GMeshVertex2D *, GUInt32> VertexPair;
typedef std::pair<const GMeshFace2D *, GUInt32> FacePair;
typedef std::pair<const GQuadEdge2D *, GUInt32> EdgePair;

inline bool VertexPairSortAscending(const VertexPair& Pair1, const VertexPair& Pair2) {

	if (Pair1.first < Pair2.first)
		return G_TRUE;
	return G_FALSE;
}

inline bool FacePairSortAscending(const FacePair& Pair1, const FacePair& Pair2) {

	if (Pair1.first < Pair2.first)
		return G_TRUE;
	return G_FALSE;
}

inline bool EdgePairSortAscending(const EdgePair& Pair1, const EdgePair& Pair2) {

	if (Pair1.first < Pair2.first)
		return G_TRUE;
	return G_FALSE;
}

static GUInt32 FindQuadEdgeIndex(const GDynArray<EdgePair>& EdgeMap, const GQuadEdge2D *QuadEdge) {

	GDynArray<EdgePair>::const_iterator it;
	EdgePair tmpEdgePair(QuadEdge, 0xFFFFFFFF);

	it = std::lower_bound(EdgeMap.begin(), EdgeMap.end(), tmpEdgePair, EdgePairSortAscending);
	G_ASSERT(it != EdgeMap.end());
	G_ASSERT(it->first == QuadEdge);
	return it->second;
}

static GUInt32 FindVertexIndex(const GDynArray<VertexPair>& VertexMap, const GMeshVertex2D *Vertex) {

	GDynArray<VertexPair>::const_iterator it;
	VertexPair tmpVertexPair(Vertex, 0xFFFFFFFF);

	it = std::lower_bound(VertexMap.begin(), VertexMap.end(), tmpVertexPair, VertexPairSortAscending);
	G_ASSERT(it != VertexMap.end());
	G_ASSERT(it->first == Vertex);
	return it->second;
}

static GUInt32 FindFaceIndex(const GDynArray<FacePair>& FaceMap, const GMeshFace2D *Face) {

	GDynArray<FacePair>::const_iterator it;
	FacePair tmpFacePair(Face, 0xFFFFFFFF);

	it = std::lower_bound(FaceMap.begin(), FaceMap.end(), tmpFacePair, FacePairSortAscending);
	G_ASSERT(it != FaceMap.end());
	G_ASSERT(it->first == Face);
	return it->second;
}

GError GMesh2D::BaseClone(const GElement& Source) {

	const GMesh2D& s = (const GMesh2D&)Source;
	GUInt32 i, j, k, w;
	GQuadEdge2D *q;

	Clear();

	// create associative arrays for vertex, face and edge pointers
	j = s.gVertices.size();
	GDynArray<VertexPair> vertexMap(j);
	gVertices.resize(j);
	for (i = 0; i < j; i++)
		vertexMap[i] = VertexPair(s.gVertices[i], i);
	std::sort(vertexMap.begin(), vertexMap.end(), VertexPairSortAscending);

	j = s.gQuadEdges.size();
	GDynArray<EdgePair> edgeMap(j);
	gQuadEdges.resize(j);
	for (i = 0; i < j; i++)
		edgeMap[i] = EdgePair(s.gQuadEdges[i], i);
	std::sort(edgeMap.begin(), edgeMap.end(), EdgePairSortAscending);


	j = s.gFaces.size();
	GDynArray<FacePair> faceMap(j);
	gFaces.resize(j);
	for (i = 0; i < j; i++)
		faceMap[i] = FacePair(s.gFaces[i], i);
	std::sort(faceMap.begin(), faceMap.end(), FacePairSortAscending);

	// allocate new vertexes
	j = s.gVertices.size();
	for (i = 0; i < j; i++) {
		GMeshVertex2D *newVertex = new GMeshVertex2D(this);
		newVertex->gPosition = s.gVertices[i]->gPosition;
		newVertex->gCustomData = s.gVertices[i]->gCustomData;
		gVertices[i] = newVertex;
	}
	j = s.gQuadEdges.size();
	// allocate new quad edges
	for (i = 0; i < j; i++) {
		GQuadEdge2D *newQuadEdge = new GQuadEdge2D(this);
		newQuadEdge->gEdges[0].gCustomData = s.gQuadEdges[i]->gEdges[0].gCustomData;
		newQuadEdge->gEdges[0].gIndex = s.gQuadEdges[i]->gEdges[0].gIndex;
		newQuadEdge->gEdges[1].gCustomData = s.gQuadEdges[i]->gEdges[1].gCustomData;
		newQuadEdge->gEdges[1].gIndex = s.gQuadEdges[i]->gEdges[1].gIndex;
		newQuadEdge->gEdges[2].gCustomData = s.gQuadEdges[i]->gEdges[2].gCustomData;
		newQuadEdge->gEdges[2].gIndex = s.gQuadEdges[i]->gEdges[2].gIndex;
		newQuadEdge->gEdges[3].gCustomData = s.gQuadEdges[i]->gEdges[3].gCustomData;
		newQuadEdge->gEdges[3].gIndex = s.gQuadEdges[i]->gEdges[3].gIndex;
		gQuadEdges[i] = newQuadEdge;
	}
	j = s.gFaces.size();
	// allocate new faces
	for (i = 0; i < j; i++) {
		GMeshFace2D *newFace = new GMeshFace2D(this);
		newFace->gCustomData = s.gFaces[i]->gCustomData;
		gFaces[i] = newFace;
	}
	// link vertexes internal 'edge' field
	j = s.gVertices.size();
	for (i = 0; i < j; i++) {
		if (s.gVertices[i]->gEdge) {
			q = (GQuadEdge2D*)(s.gVertices[i]->gEdge - s.gVertices[i]->gEdge->gIndex);
			k = FindQuadEdgeIndex(edgeMap, q);
			G_ASSERT(k < s.gQuadEdges.size());
			gVertices[i]->gEdge = &gQuadEdges[k]->gEdges[s.gVertices[i]->gEdge->gIndex];
		}
		else
			gVertices[i]->gEdge = NULL;
	}
	// link faces
	j = s.gFaces.size();
	for (i = 0; i < j; i++) {
		if (s.gFaces[i]->gEdge) {
			q = (GQuadEdge2D*)(s.gFaces[i]->gEdge - s.gFaces[i]->gEdge->gIndex);
			k = FindQuadEdgeIndex(edgeMap, q);
			G_ASSERT(k < s.gQuadEdges.size());
			gFaces[i]->gEdge = &gQuadEdges[k]->gEdges[s.gFaces[i]->gEdge->gIndex];
		}
		else
			gFaces[i]->gEdge = NULL;
	}
	// link quad edges
	j = s.gQuadEdges.size();
	for (i = 0; i < j; i++) {
		for (w = 0; w < 4; w++) {
			// link gNext field
			GMeshEdge2D *e = &s.gQuadEdges[i]->gEdges[w];
			q = (GQuadEdge2D*)(e->gNext - e->gNext->gIndex);
			k = FindQuadEdgeIndex(edgeMap, q);
			G_ASSERT(k < s.gQuadEdges.size());
			gQuadEdges[i]->gEdges[w].gNext = &gQuadEdges[k]->gEdges[e->gNext->gIndex];
			// link gVertex field
			if (e->gVertex) {
				k = FindVertexIndex(vertexMap, e->gVertex);
				G_ASSERT(k < s.gVertices.size());
				gQuadEdges[i]->gEdges[w].gVertex = gVertices[k];
			}
			else
				gQuadEdges[i]->gEdges[w].gVertex = NULL;
			// link gFace field
			if (e->gFace) {
				k = FindFaceIndex(faceMap, e->gFace);
				G_ASSERT(k < s.gFaces.size());
				gQuadEdges[i]->gEdges[w].gFace = gFaces[k];
			}
			else
				gQuadEdges[i]->gEdges[w].gFace = NULL;
		}
	}
	return G_NO_ERROR;
}

struct GIndexPair {
	GUInt32 A;
	GUInt32 B;

	GIndexPair(const GUInt32 NewValA, const GUInt32 NewValB) {
		if (NewValA > NewValB) {
			A = NewValB;
			B = NewValA;
		}
		else {
			A = NewValA;
			B = NewValB;
		}
	}
};

struct GIndexPairTrait {
	bool operator()(const GIndexPair& s1, const GIndexPair& s2) const	{
		if (s1.B < s2.B)
			return G_TRUE;
		else
		if (s1.B > s2.B)
			return G_FALSE;
		else {
			if (s1.A < s2.A)
				return G_TRUE;
			return G_FALSE;
		}
	}
};

GError GMesh2D::BuildFromFaces(const GDynArray<GPoint2>& Points, const GDynArray<GIndexList>& FacesIndexes,
							   const GBool Deep2ManifoldCheck) {

	GUInt32 i, j, k, w, q, edgesCount;

	if (Points.size() < 3 || FacesIndexes.size() < 1)
		return G_INVALID_PARAMETER;

	// check input data
	j = FacesIndexes.size();
	for (i = 0; i < j; i++) {
		k = FacesIndexes[i].size();
		// a face must be built of at least 3 points
		if (k < 3)
			return G_INVALID_PARAMETER;
	}

	// check for 2 manifold consistency
	if (Deep2ManifoldCheck) {
		std::set<GIndexPair, GIndexPairTrait> idxSet;
		j = FacesIndexes.size();
		for (i = 0; i < j; i++) {
			k = FacesIndexes[i].size();
			for (w = 0; w < k; w++) {
				if (w == k - 1)
					q = 0;
				else
					q = w + 1;
				GIndexPair pair(FacesIndexes[i][w], FacesIndexes[i][q]);
				idxSet.insert(pair);
			}
		}
		GInt32 ii = (GInt32)Points.size();
		GInt32 jj = (GInt32)FacesIndexes.size() + 1;
		GInt32 qq = (GInt32)idxSet.size();
		if (ii - qq + jj != 2)
			return G_INVALID_PARAMETER;
	}


	// first clear mesh
	Clear();

	j = Points.size();
	GDynArray<GImpExpMeshVertex> verts(j);
	for (i = 0; i < j; i++) {
		verts[i].gDone = G_FALSE;
		verts[i].gPosition = Points[i];
		verts[i].gVertex = NULL;
		verts[i].gInstantiated = G_FALSE;
	}

	edgesCount = 0;
	// build arcs structures
	j = FacesIndexes.size();
	GDynArray<GImpExpMeshFace> faces(j);
	for (i = 0; i < j; i++) {
		faces[i].gMeshFace = NULL;
		k = FacesIndexes[i].size();
		// a face must be built of at least 3 points
		if (k < 3)
			return G_INVALID_PARAMETER;

		faces[i].gVertexList.resize(k);
		for (w = 0; w < k; w++) {
			if (w == k - 1)
				q = 0;
			else
				q = w + 1;
			// every two consecutive indexes must be different for a valid face
			if (FacesIndexes[i][w] == FacesIndexes[i][q])
				return G_INVALID_PARAMETER;

			faces[i].gVertexList[w] = &verts[FacesIndexes[i][w]];
		}
		AddArcs(faces[i].gVertexList, &faces[i]);
	}
	// connect arcs and build respective quad-edges
	BuildQuadedge(this, verts);
	return G_NO_ERROR;
}

// remove edge from internal list, without doing nothing else on mesh
void GMesh2D::RemoveEdge(GMeshEdge2D *Edge) {

	G_ASSERT(Edge != NULL);
	GMeshEdge2D *edges;
	
	// remove edge form internal list
	GDynArray<GQuadEdge2D *>::iterator it;
	for (it = gQuadEdges.begin(); it != gQuadEdges.end(); ++it) {
		GQuadEdge2D *qe = *it;
		edges = qe->Edges();
		if (&edges[0] == Edge || &edges[1] == Edge || &edges[2] == Edge || &edges[3] == Edge) {
			gQuadEdges.erase(it);
			// free the quad-edge that the edge belongs to
			delete qe;
			return;
		}
	}
	G_ASSERT(0);
}

// Splice a given pair of (non-null) edges.
void GMesh2D::Splice(GMeshEdge2D *a, GMeshEdge2D *b) {

	G_ASSERT(a != NULL);
	G_ASSERT(b != NULL);
	// see Guibas and Stolfi
	GMeshEdge2D* alpha = a->Onext()->Rot();
	GMeshEdge2D* beta  = b->Onext()->Rot();
	GMeshEdge2D* t1 = b->Onext();
	GMeshEdge2D* t2 = a->Onext();
	GMeshEdge2D* t3 = beta->Onext();
	GMeshEdge2D* t4 = alpha->Onext();
	a->gNext = t1;
	b->gNext = t2;
	alpha->gNext = t3;
	beta->gNext = t4;
}

// detach the edge from mesh
void GMesh2D::DetachEdge(GMeshEdge2D *Edge) {

	G_ASSERT(Edge != NULL);
	// detach the edge from its mesh
	Splice(Edge, Edge->Oprev());
	Splice(Edge->Sym(), Edge->Sym()->Oprev());
}

// remove vertex from internal list, without doing nothing else on mesh
void GMesh2D::RemoveVertex(GMeshVertex2D *Vertex) {

	GDynArray<GMeshVertex2D *>::iterator it;
	for (it = gVertices.begin(); it != gVertices.end(); ++it) {
		if ((*it) == Vertex) {
			gVertices.erase(it);
			delete Vertex;
			return;
		}
	}
}

GMeshEdge2D *GMesh2D::MakeVertexEdge(GMeshVertex2D *Vertex, GMeshFace2D *Left, GMeshFace2D *Right) {

	G_ASSERT(Vertex != NULL);
	G_ASSERT(Left != NULL);
	G_ASSERT(Right != NULL);
	// locate the edges to the right of each of the faces in the orbit of the vertex
	GMeshEdge2D *edge = Vertex->Edge();
	GMeshEdge2D *edge1 = NULL;
	GMeshEdge2D *edge2 = NULL;

	edge1 = OrbitLeft(edge, Right);
	edge2 = OrbitLeft(edge, Left);

	GPoint2 a, b, c, d;

	a = edge1->Org()->Position();
	b = edge1->Dest()->Position();
	c = edge2->Org()->Position();
	d = edge2->Dest()->Position();


	if (!edge1) {
		GString s = "GMesh::MakeVertexEdge: unable to locate right face";
		G_DEBUG(s);
		std::abort();
	}
	if (!edge2)	{
		GString s = "GMesh::MakeVertexEdge: unable to locate left face";
		G_DEBUG(s);
		std::abort();
	}
	// create a new vertex and copy the position of the vertex of origin
	GMeshVertex2D *vertexNew = AddVertex(Vertex->Position());
	// create a new edge and rotate it to make a clockwise loop
	GMeshEdge2D *edgeNew = AddEdge()->Rot();
	// connect the origin (and destination) of the new edge to _vertex_ so that
	// the left face of the edge is _left_
	// this makes a loop on the inside of _left_
	Splice(edge2, edgeNew);
	// split the origin and destination of the loop so that the right face of the
	// edge is now _right_
	// this results in a non-loop edge dividing _left_ from _right_
	Splice(edge1, edgeNew->Sym());
	// initialize the secondary attributes of the new edge
	edgeNew->SetOrg(edge1->Org());
	edgeNew->SetLeft(edge2->Left());
	edgeNew->SetRight(edge1->Left());
	// all edges leaving the destination orbit of the new edge now have the new
	// vertex as their vertex of origin
	SetOrbitOrg(edgeNew->Sym(), vertexNew);
	return edgeNew;
}

void GMesh2D::KillVertexEdge(GMeshEdge2D *Edge) {

	G_ASSERT(Edge != NULL);
	// locate edge1 and edge2 as in MakeVertexEdge
	GMeshEdge2D *edge1 = Edge->Oprev();
	GMeshEdge2D *edge2 = Edge->Lnext();
	// use edge1 for edge2 if the destination vertex is isolated
	if (edge2 == Edge->Sym())
		edge2 = edge1;
	// inverse of MakeVertexEdge
	Splice(edge1, Edge->Sym());
	Splice(edge2, Edge);
	// all edges leaving the destination orbit of the deleted edge now have its
	// origin vertex as their vertex of origin
	SetOrbitOrg(edge2, edge1->Org());
	// don't use the deleted edge as a reference edge any more
	edge1->Org()->AddEdge(edge1);
	edge1->Left()->AddEdge(edge1);
	edge2->Left()->AddEdge(edge2);
	// reclaim the vertex and the edge
	GMeshVertex2D *v = Edge->Dest();
	RemoveVertex(v);
	// delete edge
	DetachEdge(Edge);
	RemoveEdge(Edge);
}

// remove face from internal list, without doing nothing else on mesh
void GMesh2D::RemoveFace(GMeshFace2D *Face) {

	GDynArray<GMeshFace2D *>::iterator it;
	for (it = gFaces.begin(); it != gFaces.end(); ++it) {
		if ((*it) == Face) {
			gFaces.erase(it);
			delete Face;
			return;
		}
	}
}

GMeshEdge2D *GMesh2D::MakeFaceEdge(GMeshFace2D *Face, GMeshVertex2D *Org, GMeshVertex2D *Dest) {

	G_ASSERT(Face != NULL);
	G_ASSERT(Org != NULL);
	G_ASSERT(Dest != NULL);
	// locate the edges leaving each of the vertexes in the orbit of the face
	GMeshEdge2D *edge = Face->Edge();
	GMeshEdge2D *edge1 = OrbitOrg(edge, Org);
	GMeshEdge2D *edge2 = OrbitOrg(edge, Dest);
	if (!edge1) {
		GString s = "GMesh::MakeFaceEdge: unable to locate origin vertex";
		G_DEBUG(s);
		std::abort();
	}
	if (!edge2) {
		GString s = "GMesh::MakeFaceEdge: unable to locate destination vertex";
		G_DEBUG(s);
		std::abort();
	}
	// create a new face
	GMeshFace2D *faceNew = AddFace();
	// create a new (non-loop) edge
	GMeshEdge2D *edgeNew = AddEdge();
	// connect the destination of the new edge to the origin of edge2
	// both faces of the edge are now face
	Splice(edge2, edgeNew->Sym());
	// connect the origin of the new edge to _edge1_
	// face is split in half along the new edge, with the new face introduced
	// on the right
	Splice(edge1, edgeNew);
	// initialize the secondary attributes of the new edge
	edgeNew->SetOrg(edge1->Org());
	edgeNew->SetDest(edge2->Org());
	edgeNew->SetLeft(edge2->Left());
	// all edges in the right orbit of the new edge (i.e. the left orbit of its
	// Sym) now have the new face as their left face
	SetOrbitLeft(edgeNew->Sym(), faceNew);
	return edgeNew;
}

void GMesh2D::KillFaceEdge(GMeshEdge2D *Edge) {

	G_ASSERT(Edge != NULL);
	// locate edge1 and edge2 as in MakeFaceEdge
	GMeshEdge2D *edge1 = Edge->Oprev();
	GMeshEdge2D *edge2 = Edge->Lnext();
	// use edge2 for edge1 if the right face is inside a loop
	if (edge1 == Edge->Sym())
		edge1 = edge2;
	// inverse of MakeFaceEdge
	Splice(edge2, Edge->Sym());
	Splice(edge1, Edge);
	// all edges in the right orbit of the deleted edge now have its left face
	// as their left face
	SetOrbitLeft(edge1, edge2->Left());
	// don't use the deleted edge as a reference edge any more
	edge1->Org()->AddEdge(edge1);
	edge2->Org()->AddEdge(edge2);
	edge2->Left()->AddEdge(edge2);
	// reclaim the face and the edge
	GMeshFace2D *f = Edge->Right();
	RemoveFace(f);
	// delete edge
	DetachEdge(Edge);
	RemoveEdge(Edge);
}

GMeshEdge2D *GMesh2D::OrbitOrg(GMeshEdge2D *Edge, GMeshVertex2D *Org) {

	G_ASSERT(Edge != NULL);
	G_ASSERT(Org != NULL);
	// traverse the Lnext orbit of edge looking for an edge whose origin is org
	GMeshEdge2D *scan = Edge;
	do {
		if (scan->Org() == Org)
			return scan;
		scan = scan->Lnext();
	} while (scan != Edge);
	return NULL;
}

void GMesh2D::SetOrbitOrg(GMeshEdge2D *Edge, GMeshVertex2D *Org) {

	G_ASSERT(Edge != NULL);
	G_ASSERT(Org != NULL);
	// traverse the Onext orbit of edge, setting the origin of each edge to org
	GMeshEdge2D *scan = Edge;
	do {
		scan->SetOrg(Org);
		scan = scan->Onext();
	} while (scan != Edge);
}

GMeshEdge2D *GMesh2D::OrbitLeft(GMeshEdge2D *Edge, GMeshFace2D *Left) {

	G_ASSERT(Edge != 0);
	G_ASSERT(Left != 0);
	// traverse the Onext orbit of edge looking for an edge whose left face is left
	GMeshEdge2D *scan = Edge;
	do {
		if (scan->Left() == Left)
			return scan;
		scan = scan->Onext();
	} while (scan != Edge);
	return NULL;
}

void GMesh2D::SetOrbitLeft(GMeshEdge2D *Edge, GMeshFace2D *Left) {

	G_ASSERT(Edge != NULL);
	G_ASSERT(Left != NULL);
	// traverse the Lnext orbit of edge, setting the left face of each edge to left
	GMeshEdge2D *scan = Edge;
	do {
		scan->SetLeft(Left);
		scan = scan->Lnext();
	} while (scan != Edge);
}

// find a mesh vertex with the corresponding geometric coordinates
GMeshVertex2D *GMesh2D::FindVertex(const GReal X, const GReal Y) {

	GUInt32 i, j = gVertices.size();
	GMeshVertex2D *v;

	for (i = 0; i < j; i++) {
		v = gVertices[i];
		if (v->Position()[G_X] == X && v->Position()[G_Y] == Y)
			return v;
	}
	return NULL;
}

// return true if a given pair of vertexes is connected directly by an edge along a given left face.
GBool GMesh2D::IsConnected(GMeshVertex2D *Vert1, GMeshVertex2D *Vert2, GMeshFace2D *Left) {

	G_ASSERT(Vert1 != NULL);
	G_ASSERT(Vert2 != NULL);
	G_ASSERT(Left != NULL);
	// check the orbit of vertex1 for an edge to vertex2
	GMeshVertexEdgeIterator2D edges(Vert1);
	GMeshEdge2D *edge;
	while ((edge = edges.Next()) != NULL)
		if (edge->Dest() == Vert2 && edge->Left() == Left)
			return G_TRUE;
	return G_FALSE;
}

// return the face to the right of a given face around a given vertex (null if none)
GMeshFace2D *GMesh2D::RightFace(GMeshVertex2D *Vertex, GMeshFace2D *Left) {

	G_ASSERT(Vertex != NULL);
	G_ASSERT(Left != NULL);
	// check the left face of each edge in the orbit of the vertex
	GMeshEdge2D *start = Vertex->Edge();
	GMeshEdge2D *scan  = start;
	do {
		if (scan->Left() == Left)
			return scan->Right();
		scan = scan->Onext();
	} while (scan != start);
	return NULL;
}

// add a new edge e connecting the destination of a to the origin of b, in such a way that
// all three have the same left face after the connection is complete
GMeshEdge2D* GMesh2D::Connect(GMeshEdge2D* a, GMeshEdge2D* b) {

	GMeshEdge2D* e = AddEdge();
	Splice(e, a->Lnext());
	Splice(e->Sym(), b);

	e->SetOrg(a->Dest());
	e->SetDest(b->Org());
	return e;
}

// clear mesh (delete all faces, vertexes and edges)
void GMesh2D::Clear() {

	GUInt32 i, j;
	// reclaim each of the vertexes and faces still owned by the mesh
	j = gVertices.size();
	for (i = 0; i < j; i++) {
		GMeshVertex2D *v = gVertices[i];
		delete v;
	}
	j = gFaces.size();
	for (i = 0; i < j; i++) {
		GMeshFace2D *f = gFaces[i];
		delete f;
	}
	j = gQuadEdges.size();
	for (i = 0; i < j; i++) {
		GQuadEdge2D *q = gQuadEdges[i];
		delete q;
	}
	// reclaim the vertex, edge and face arrays
	gVertices.clear();
	gFaces.clear();
	gQuadEdges.clear();
}

// essentially turns edge e counterclockwise inside its enclosing quadrilateral
void GMesh2D::DelaunaySwap(GMeshEdge2D* e) {

	GMeshEdge2D* a = e->Oprev();
	GMeshEdge2D* b = e->Sym()->Oprev();
	GMeshFace2D *left = e->Left();
	GMeshFace2D *right = e->Right();

	// this two instructions are the same of DetachEdge(e)
	Splice(e, a);
	Splice(e->Sym(), b);
	// now build the swapped edge
	Splice(e, a->Lnext());
	Splice(e->Sym(), b->Lnext());
	// now we have to fix edge orbits
	SetOrbitLeft(e, left);
	SetOrbitLeft(e->Sym(), right);
	// set new origin and destination
	e->SetOrg(a->Dest());
	e->SetDest(b->Dest());
}

// returns an edge e, s.t. either P is on e, or e is an edge of a triangle containing P.
// The search starts from startingEdge and proceeds in the general direction of P
GMeshEdge2D* GMesh2D::DelaunayLocate(const GPoint2& P, GMeshEdge2D *StartEdge) {

	#define RIGHTOF(x, edge) CounterClockWise(x, edge->Dest()->Position(), edge->Org()->Position())
	GMeshEdge2D* e = StartEdge;

	while (1) {
		if (P == e->Org()->Position() || P == e->Dest()->Position())
		    return e;
		else
		if (RIGHTOF(P, e))
			 e = e->Sym();
		else
		if (!RIGHTOF(P, e->Onext()))
			 e = e->Onext();
		else
		if (!RIGHTOF(P, e->Dprev()))
			 e = e->Dprev();
		else
		    return e;
	}
	#undef RIGHTOF
}

// inserts a new point into a subdivision representing a Delaunay triangulation, and fixes the
// affected edges so that the result is still a Delaunay triangulation. This is based on the
// pseudo code from Guibas and Stolfi, with slight modifications and a bug fix.
GMeshEdge2D *GMesh2D::DelaunayInsertSite(const GPoint2& P, const GReal Tolerance, GMeshEdge2D *LastIndertedEdge) {

	#define RIGHTOF(x, edge) CounterClockWise(x, edge->Dest()->Position(), edge->Org()->Position())
	GMeshEdge2D *edge, *e, *newEdge1, *newEdge2, *tmpEdge, *stop, *newDelaunayEdge;
	GMeshFace2D *newFace;

	e = DelaunayLocate(P, LastIndertedEdge);
	if ((P == e->Org()->Position()) || (P == e->Dest()->Position()))  // point is already in
	    return NULL;

	if (e->IsOnEdge(P, Tolerance)) {
		e = e->Oprev();
		KillFaceEdge(e->Onext());
	}
	// find the "stop" edge (the edge where we have to stop diagonal tracing
	stop = e->Lprev();
	// connect the new point to the vertexes of the containing triangle (or quadrilateral, if the new
	// point fell on an existing edge)
	GMeshVertex2D *v = AddVertex(P);
	v->SetCustomData((void *)1);

	// now we create two edges and a new face
	newEdge1 = AddEdge();
	newEdge2 = AddEdge();
	newFace = AddFace();
	// we set the new face to be composed by e, newEdg1 and newEdg2
	Splice(e->Lnext(), newEdge2);
	Splice(e, newEdge1->Sym());
	Splice(newEdge1, newEdge2->Sym());
	// set edge2 parameters
	newEdge2->SetLeft(e->Left());
	newEdge2->SetOrg(e->Dest());
	newEdge2->SetDest(v);
	// set edge1 parameters
	newEdge1->SetLeft(e->Left());
	newEdge1->SetOrg(v);
	newEdge1->SetDest(e->Org());
	// now we must set new orbit left for edg1->Sym
	SetOrbitLeft(newEdge1->Sym(), newFace);
	// lets trace diagonal(s) (two diagonal if the new point fell on an existing edge, else just one)
	newDelaunayEdge = newEdge2;
	tmpEdge = newEdge2->Oprev();
	while (tmpEdge != stop) {
		edge = MakeFaceEdge(tmpEdge->Left(), tmpEdge->Dest(), v);
		tmpEdge = edge->Oprev();
	}

	// examine suspect edges to ensure that the Delaunay condition is satisfied
	do {
		tmpEdge = e->Oprev();
		if (RIGHTOF(tmpEdge->Dest()->Position(), e) &&
			IsInCircle(e->Org()->Position(), tmpEdge->Dest()->Position(), e->Dest()->Position(), P)) {
				// turns edge e counterclockwise inside its enclosing quadrilateral
				DelaunaySwap(e);
				// next suspect edge
				e = e->Oprev();
		}
		else
		if (e->Onext() == newDelaunayEdge)  // no more suspect edges
			return newDelaunayEdge;
		else
			// pop a suspect edge
		    e = e->Onext()->Lprev();
	} while (1);
	#undef RIGHTOF
	return newDelaunayEdge;
}


// build an incremental Delaunay triangulation
GError GMesh2D::BuildFromPointsCloud(const GDynArray<GPoint2>& Points, const GReal Epsilon) {

	GMeshVertex2D *v1, *v2, *v3, *v4;
	GMeshEdge2D *DelaunayStartingEdge = NULL; 	// last inserted edge for Delanunay triangulation
	GMeshEdge2D *tmpEdge;
	GMeshFace2D *left, *right, *front;
	GUInt32 i, j = Points.size();
	// just to be sure that a positive precision is specified to DelaunayInsertSite routine
	GReal eps = GMath::Abs(Epsilon);

	// test for a degenerative case
	if (j < 3)
		return G_INVALID_PARAMETER;

	// calculate bounding box for points cloud
	GPoint2 p;
	GAABox2 extBox(Points);
	p = extBox.Min();
	p = p - GPoint2(1, 1);
	extBox.SetMin(p);
	p = extBox.Max();
	p = p + GPoint2(1, 1);
	extBox.SetMax(p);

	// clear mesh
	Clear();

	// add start submanifold
	AddSubManifold();
	// create a first 2D triangulation, generated from bounding box ends (min and max points)
	v1 = Vertex(0);
	DelaunayStartingEdge = v1->Edge();
	left = DelaunayStartingEdge->Left();
	right = DelaunayStartingEdge->Right();

	v1->SetPosition(extBox.Max());
	v1->SetCustomData(NULL);

	v2 = MakeVertexEdge(v1, left, right)->Dest();
	v2->SetPosition(GPoint2(extBox.Min()[G_X], extBox.Max()[G_Y]));
	v2->SetCustomData(NULL);

	v3 = MakeVertexEdge(v2, left, right)->Dest();
	v3->SetPosition(extBox.Min());
	v3->SetCustomData(NULL);

	v4 = MakeVertexEdge(v3, left, right)->Dest();
	v4->SetPosition(GPoint2(extBox.Max()[G_X], extBox.Min()[G_Y]));
	v4->SetCustomData(NULL);

	// make a diagonal
	front = MakeFaceEdge(left, v3, v1)->Right();
	// insert every specified input point
	for (i = 0; i < j; i++) {
		tmpEdge = DelaunayInsertSite(Points[i], eps, DelaunayStartingEdge);
		if (tmpEdge)
			DelaunayStartingEdge = tmpEdge;
	}
	return G_NO_ERROR;
}

};	// end namespace Amanith
