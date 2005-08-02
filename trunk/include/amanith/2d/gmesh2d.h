/****************************************************************************
** $file: amanith/2d/gmesh2d.h   0.1.0.0   edited Jun 30 08:00
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

#ifndef GMESH2D_H
#define GMESH2D_H

#include "amanith/gglobal.h"
#include "amanith/geometry/gvect.h"
#include "amanith/gelement.h"

/*!
	\file gmesh2d.h
	\brief GMesh2D classes header file.
*/
namespace Amanith {


	// *********************************************************************
	//                            GMeshVertex2D
	// *********************************************************************
	template <typename DATA_TYPE>
	class GMesh2D;

	template <typename DATA_TYPE>
	class GMeshEdge2D;

	/*!
		\class GMeshVertex2D
		\brief This class represent a mesh vertex.

		The information stored at a vertex consists of one piece of topological information (a pointer to one of the
		outgoing edges of the vertex), plus geometric information (the 2D position), and optional attribute
		information (color, normals, etc).
		A mesh vertex is described by:

		- Its geometrical position in 2D space.
		- An arbitrary outgoing edge of this vertex.
		- A custom data pointer, useful to hang custom informations, for extensibility. You can store arbitrary (4 byte)
		information there, or pointers to additional memory (e.g. for colors, normals, or texture coordinates).
	*/
	template <typename DATA_TYPE>
	class G_EXPORT GMeshVertex2D {

		//template <typename DATA_TYPE>
		friend class GMesh2D<DATA_TYPE>;

	private:
		//! The 2D position of this vertex.
		GPoint<DATA_TYPE, 2> gPosition;
		//! The custom data associated with this vertex.
		void *gCustomData;
		//! The mesh that this vertex belongs to, it must be non-null
		GMesh2D<DATA_TYPE> *gMesh;
		//! An arbitrary outgoing edge of this vertex, NULL if isolated
		GMeshEdge2D<DATA_TYPE> *gEdge;

	protected:
		//! Constructor, initialize this vertex at the origin with no outgoing edges
		GMeshVertex2D(GMesh2D<DATA_TYPE> *Mesh);
		//! Destructor
		~GMeshVertex2D();

	public:
		//! Return the mesh for this vertex (the mesh that the vertex belongs to); it is non NULL.
		inline GMesh2D<DATA_TYPE>* Mesh() const {
			return gMesh;
		}
		//! Return custom data associated to this vertex
		inline void *CustomData() const {
			return gCustomData;
		}
		//! Set custom data for this vertex
		inline void SetCustomData(void *CustomData) {
			gCustomData = CustomData;
		}
		//! Get the 2D position of this vertex
		const GPoint<DATA_TYPE, 2>& Position() const {
			return gPosition;
		}
		//! Set vertex position
		void SetPosition(const GPoint<DATA_TYPE, 2>& NewPos) {
			gPosition = NewPos;
		}
		/*!
			Return an arbitrary outgoing edge from this vertex (an edge whose origin is this vertex).
			It's NULL if the vertex is an isolated vertex
		*/
		inline GMeshEdge2D<DATA_TYPE>* Edge() const {
			return gEdge;
		}
		//! Return G_TRUE if vertex is isolated.
		inline GBool IsIsolated() const {
			if (gEdge == NULL)
				return G_TRUE;
			return G_FALSE;
		}
		//! Add a given outgoing edge to this vertex (an edge whose origin is this vertex). Must be non NULL
		void AddEdge(GMeshEdge2D<DATA_TYPE>* NewEdge);
		/*!
			Remove a given outgoing edge from this vertex (an edge whose origin is no longer at this vertex)
			Edge must be non NULL.
		*/
		void RemoveEdge(GMeshEdge2D<DATA_TYPE>* Edge);
		//! Counts how many edges are in the origin ring specified by this vertex.
		GUInt32 EdgesInRingCount();
	};


	// *********************************************************************
	//                        GMeshVertexEdgeIterator2D
	// *********************************************************************
	// enumerates the outgoing edges of a given vertex in counterclockwise order
	template <typename DATA_TYPE>
	class GMeshVertexEdgeIterator2D {

	private:
		// the first edge to be iterated (it is non-null)
		GMeshEdge2D<DATA_TYPE> *gStart;
		// the next edge to be iterated, null if exhausted
		GMeshEdge2D<DATA_TYPE> *gEdge;

	public:
		// constructor; initialize this edge iterator over a given vertex (MUST be non-null)
		GMeshVertexEdgeIterator2D(GMeshVertex2D<DATA_TYPE> *Vertex) {
			// pick an arbitrary edge in the vertex orbit
			gStart = Vertex->Edge();
			gEdge  = gStart;
		}
		// destructor
		~GMeshVertexEdgeIterator2D() {
		}
		// return the next edge of this edge iterator, if any. Null if none
		GMeshEdge2D<DATA_TYPE> *Next();
	};


	// *********************************************************************
	//                             GMeshEdge2D
	// *********************************************************************
	template <typename DATA_TYPE>
	class GMeshFace2D;

	template <typename DATA_TYPE>
	class GQuadEdge2D;

	/*!
		\class GMeshEdge2D
		\brief A directed edge from one vertex to another, adjacent to two faces.

		The class GMeshEdge2D represents a directed edge. Given an edge, you can find the immediately adjacent
		vertexes, faces, and edges, and the "symmetric" edge that points in the opposite direction. These operators
		are all fast (just a few memory references). Because edges are directed and we always imagine ourselves
		viewing the object from the outside, we can speak of the origin and destination vertexes and left and right
		faces of an edge.
		The number of edges of a face (the face degree or valence) is 3 or greater for "real" polyhedra, but
		sometimes during construction of data structures, it is useful to have faces with 1 or 2 edges, which
		would correspond geometrically to loops or degenerate sliver polygons.

		Using Lnext(), we could loop around the edges of the face on the left of edge Start in counterclockwise order:
\code
	void LeftFromEdge(GMeshEdge2D *Start) {

		GMeshEdge2D *e = Start;
		
		do {
			// do something with edge e
			e = e->Lnext();
		} while (e != Start);
	}
\endcode

		Similarly, the edges around the origin vertex of edge Start can be visited in counterclockwise order like so:
\code
	void OrgFromEdge(GMeshEdge2D *Start) {
		
		GMeshEdge2D *e = Start;

		do {
			// do something with edge e
			e = e->Onext();
		} while (e != Start);
	}
\endcode

		Original quad-edge data structure due to Guibas and Stolfi (1985).
		"Leonidas Guibas and Jorge Stolfi, Primitives for the manipulation of general subdivisions and the computation
		of Voronoi diagrams, ACM Transactions on Graphics, 4(2), 1985, 75-123"
	*/
	template <typename DATA_TYPE>
	class G_EXPORT GMeshEdge2D {

		//template <typename DATA_TYPE>
		friend class GQuadEdge2D<DATA_TYPE>;

		//template <typename DATA_TYPE>
		friend class GMesh2D<DATA_TYPE>;

	private:
		//! The index of this edge in its quad-edge structure (between 0 and 3 inclusive).
		GInt32 gIndex;
		//! The next counterclockwise edge around (from) the origin of this edge (non NULL).
		GMeshEdge2D<DATA_TYPE> *gNext;
		//! The origin vertex of this edge, if prime (NULL if not prime).
		GMeshVertex2D<DATA_TYPE> *gVertex;
		//! The target face of this edge, if dual (NULL if not dual).
		GMeshFace2D<DATA_TYPE> *gFace;
		//! The data associated with this edge
		void *gCustomData;

	protected:
		//! Default constructor, it initialize this edge with no connections.
		GMeshEdge2D() {
		}
		//! Destructor.
		~GMeshEdge2D() {
		}

	public:
		//! Return custom data associated to this edge.
		inline void *CustomData() const {
			return gCustomData;
		}
		//! Set custom data for this edge.
		inline void SetCustomData(void *CustomData) {
			gCustomData = CustomData;
		}
		//! Return the origin vertex of this edge (NULL if currently unknown).
		inline GMeshVertex2D<DATA_TYPE> *Org() {
			return gVertex;
		}
		//! Return the destination vertex of this edge (NULL if currently unknown).
		inline GMeshVertex2D<DATA_TYPE> *Dest() {
			return Sym()->Org();
		}
		//! Change the origin vertex of this edge to a given vertex (NULL if currently unknown).
		void SetOrg(GMeshVertex2D<DATA_TYPE> *NewOrg);
		//! Change the destination vertex of this edge to a given vertex (NULL if currently unknown).
		void SetDest(GMeshVertex2D<DATA_TYPE> *NewDest);
		//! Return the left face of this edge (NULL if currently unknown).
		inline GMeshFace2D<DATA_TYPE> *Left() {
			return Rot()->gFace;
		}
		//! Return the right face of this edge (NULL if currently unknown)
		inline GMeshFace2D<DATA_TYPE> *Right() {
			return InvRot()->gFace;
		}
		//! Change the left face of this edge to a given face (NULL if currently unknown).
		void SetLeft(GMeshFace2D<DATA_TYPE> *NewLeft);
		//! Change the right face of this edge to a given face (NULL if currently unknown).
		void SetRight(GMeshFace2D<DATA_TYPE> *NewRight);
		//! Return the dual of this edge, directed from its right to its left (will be non NULL).
		inline GMeshEdge2D<DATA_TYPE>* Rot() {
			return gIndex < 3 ? this + 1 : this - 3;
		}
		//! Return the dual of this edge, directed from its left to its right (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* InvRot() {
			return gIndex > 0 ? this - 1 : this + 3;
		}
		//! Return the edge from the destination to the origin of this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* Sym() {
			return gIndex < 2 ? this + 2 : this - 2;
		}
		//! Return the next counterclockwise edge around (from) the origin of this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* Onext() {
			return gNext;
		}
		//! Return the next clockwise edge around (from) the origin of this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* Oprev() {
			return Rot()->Onext()->Rot();
		}
		//! Return the next counterclockwise edge around (into) the destination of this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE> *Dnext() {
			return Sym()->Onext()->Sym();
		}
		//! Return the next clockwise edge around (into) the destination of this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* Dprev() {
			return InvRot()->Onext()->InvRot();
		}
		//! Return the counterclockwise edge around the left face following this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* Lnext() {
			return InvRot()->Onext()->Rot();
		}
		//! Return the counterclockwise edge around the left face before this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* Lprev() {
			return Onext()->Sym();
		}
		//! Return the edge around the right face counterclockwise following this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* Rnext() {
			return Rot()->Onext()->InvRot();
		}
		//! Return the edge around the right face counterclockwise before this edge (will be non NULL)
		inline GMeshEdge2D<DATA_TYPE>* Rprev() {
			return Sym()->Onext();
		}
		//! Return the mesh this edge belongs to.
		GMesh2D<DATA_TYPE>* Mesh();
		/*!
			Determines if the point P is on the edge. The point is considered on if it is in the Tolerance-neighborhood
			of the edge.
		*/
		GBool IsOnEdge(const GPoint<DATA_TYPE, 2>& P, const DATA_TYPE Tolerance = (DATA_TYPE)G_EPSILON);
	};


	// *********************************************************************
	//                             GQuadEdge2D
	// *********************************************************************

	/*!
		\class GQuadEdge2D
		\brief This class describes a quad-edge.

		The quad-edge data structure is useful for describing the topology and geometry of polyhedra. It can be used, for
		examples, when implementing subdivision surfaces because it is elegant and it can answer adjacency queries
		efficiently.

		It can't represent all collections of polygons; it is limited to manifolds (surfaces where the neighborhood of
		each point is topologically equivalent to a disk; edges are always shared by two faces).
		In the quad-edge data structure, there are classes for vertexes, edges, and faces, but edges play the leading
		role. The edges store complete topological information; all of the topological information stored by the
		faces and vertexes is redundant with information in the edges. Figuratively speaking, the edges form the
		skeleton, and the vertexes and faces are optional decorations, hanging off of the edges.
		Vertexes hold most of the geometric (shape) information.

		Original quad-edge data structure due to Guibas and Stolfi (1985).
		"Leonidas Guibas and Jorge Stolfi, Primitives for the manipulation of general subdivisions and the computation
		of Voronoi diagrams, ACM Transactions on Graphics, 4(2), 1985, 75-123"
	*/
	template <typename DATA_TYPE>
	class GQuadEdge2D {

		friend class GMesh2D<DATA_TYPE>;

	private:
		// the edges of this quad edge:
		// index0: the oriented edge
		// index1: Rot
		// index2: Sym
		// index3: InvRot
		GMeshEdge2D<DATA_TYPE> gEdges[4];
		//! The mesh this quad-edge belong to.
		GMesh2D<DATA_TYPE> *gMesh;

	public:
		//! Constructor, initialize the edges of this quad edge with no connections.
		GQuadEdge2D(GMesh2D<DATA_TYPE> *Mesh) {

			G_ASSERT(Mesh != NULL);
			gMesh = Mesh;
			for (GUInt32 i = 0; i < 4; ++i) {
				gEdges[i].gIndex = i;
				gEdges[i].gCustomData = NULL;
				gEdges[i].gVertex = NULL;
				gEdges[i].gFace = NULL;
			}
			gEdges[0].gNext = gEdges + 0;
			gEdges[1].gNext = gEdges + 3;
			gEdges[2].gNext = gEdges + 2;
			gEdges[3].gNext = gEdges + 1;
		}
		//! Get edges
		inline GMeshEdge2D<DATA_TYPE>* Edges() {
			return gEdges;
		}
		//! Return the mesh this edge belongs to.
		inline GMesh2D<DATA_TYPE>* Mesh() {
			return gMesh;
		}
	};


	// *********************************************************************
	//                             GMeshFace2D
	// *********************************************************************

	/*!
		\class GMeshFace2D
		\brief A face of a mesh, bounded by a set of directed edges.

		Each face stores one piece of topological information, a pointer to one of the ccw-oriented edges of the
		face, plus optional attribute information (color, etc).
	*/
	template <typename DATA_TYPE>
	class G_EXPORT GMeshFace2D {

		friend class GMesh2D<DATA_TYPE>;

	private:
		//! The data associated with this face.
		void *gCustomData;
		//! The mesh that this face belongs to (it's non NULL)
		GMesh2D<DATA_TYPE> *gMesh;
		//! An arbitrary adjacent edge to this face (null if degenerate)
		GMeshEdge2D<DATA_TYPE> *gEdge;

	protected:
		//! Constructor, it initialize this face with no adjacent edges (must be non-null)
		GMeshFace2D(GMesh2D<DATA_TYPE> *Mesh);
		//! Destructor
		~GMeshFace2D();

	public:
		//! Return the mesh for this face
		inline GMesh2D<DATA_TYPE> *Mesh() const {
			return gMesh;
		}
		//! Return custom data associated to this face.
		inline void *CustomData() const {
			return gCustomData;
		}
		//! Set custom data for this face.
		inline void SetCustomData(void *CustomData) {
			gCustomData = CustomData;
		}
		//! Return an arbitrary adjacent edge for this face (null if degenerate).
		GMeshEdge2D<DATA_TYPE> *Edge() const {
			return gEdge;
		}
		//! Add a given adjacent edge to this face (must be non NULL).
		void AddEdge(GMeshEdge2D<DATA_TYPE> *Edge);
		//! Remove a given adjacent from this face (must be non NULL)
		void RemoveEdge(GMeshEdge2D<DATA_TYPE> *Edge);
		//! Return G_TRUE if a given vertex is adjacent (it belong to some edges of) the face, G_FALSE otherwise.
		GBool HasVertex(const GMeshVertex2D<DATA_TYPE> *Vertex);
		//! Return G_TRUE if the face includes all the vertexes specified, G_FALSE otherwise.
		GBool HasVertices(const GDynArray<GMeshVertex2D<DATA_TYPE> *> VertsArray);
	};

	
	// *********************************************************************
	//                       GMeshFaceEdgeIterator2D
	// *********************************************************************
	//
	// Enumerates the bounding edges of a given face in counterclockwise order
	template <typename DATA_TYPE>
	class GMeshFaceEdgeIterator2D {

	private:
		// the first edge to be iterated
		GMeshEdge2D<DATA_TYPE> *gStart;
		// the next edge to be iterated (null if exhausted)
		GMeshEdge2D<DATA_TYPE> *gEdge;

    public:
		// initialize this edge iterator over a given face (must be non-null)
		GMeshFaceEdgeIterator2D(GMeshFace2D<DATA_TYPE> *Face) {
			// pick an arbitrary edge in the face orbit
			gStart = Face->Edge();
			gEdge = gStart;
		}
		// destructor
		~GMeshFaceEdgeIterator2D() {
		}
		// return the next edge of this edge iterator, if any (null if none)
		GMeshEdge2D<DATA_TYPE> *Next() {
			// check for degeneracy or exhausted iteration
			GMeshEdge2D<DATA_TYPE> *current = gEdge;

			if (current == NULL)
				return NULL;
			// get the next edge in the left orbit of the face, but return the current edge
			// reset to null if we've come back to the start
			GMeshEdge2D<DATA_TYPE> *next = current->Lnext();
			gEdge = next != gStart ? next : 0;
			return current;
		}
	};


	// *********************************************************************
	//                               GMesh2D
	// *********************************************************************

	//! GMesh2D static class descriptor.
	//static const GClassID G_MESH2D_CLASSID = GClassID("GMesh2D", 0xF7B245BD, 0xAC374DDC, 0x92FA824B, 0x48E71646);

	template <typename DATA_TYPE>
	class GMeshVertexIterator2D;

	template <typename DATA_TYPE>
	class GMeshFaceIterator2D;

	typedef GDynArray<GUInt32> GIndexList;

	/*!
		\class GMesh2D
		\brief This class represents an enclosed volume, bounded by a set of vertexes and faces.

		A mesh is a single polyhedron, which includes sets of vertexes, edges, and faces. The routines you will need most
		are the so called Euler operators, since they maintain Euler's formula V - E + F = 2 interrelating the number
		of vertexes, edges, and faces of a polyhedron of genus 0 (topologically equivalent to a sphere). If the topology
		is a valid polyhedron before the call, it will be valid after the call, as well.
		Note that these routines update the topology, but they use the default constructors for Vertex and Face, so the
		positions of new vertexes are (0, 0), you'll have to set them yourself.
		It is permissible to have vertexes and faces of degree 1 or 2. 

		For more informations about quad-edge please take a look at this
		site: http://www-2.cs.cmu.edu/afs/andrew/scs/cs/15-463/2001/pub/src/a2/quadedge.html
	*/
	template <typename DATA_TYPE>
	class G_EXPORT GMesh2D {

		//template <typename DATA_TYPE>
		friend class GMeshVertexIterator2D<DATA_TYPE>;

		//template <typename DATA_TYPE>
		friend class GMeshFaceIterator2D<DATA_TYPE>;

	private:
		//! The vertexes in this mesh
		GDynArray< GMeshVertex2D<DATA_TYPE> *> gVertices;
		//! The faces in this mesh
		GDynArray< GMeshFace2D<DATA_TYPE> *> gFaces;
		//! The (quad)edges in the mesh
		GDynArray< GQuadEdge2D<DATA_TYPE> *> gQuadEdges;
		/*!
			Return the edge with a given left face in the vertex orbit of a given edge.
			
			\param Edge an edge of the orbit to look for the face in (must be non NULL)
			\param Left the left face to look for (must be non NULL).
			\return the edge in the same vertex orbit as Edge with left face Left, NULL if not found.
		*/
		GMeshEdge2D<DATA_TYPE> *OrbitLeft(GMeshEdge2D<DATA_TYPE> *Edge, GMeshFace2D<DATA_TYPE> *Left);
		/*!
			Set the left face of the face orbit of a given edge to a given face.

			\param Edge an edge of the orbit to set the left face of (must be non NULL)
			\param Left the new left face (must be non NULL)
		*/
		void SetOrbitLeft(GMeshEdge2D<DATA_TYPE> *Edge, GMeshFace2D<DATA_TYPE> *Left);
		/*!
			Returns an edge e, s.t. either P is on e, or e is an edge of a triangle containing P.
			The search starts from StartEdge and proceeds in the general direction of P.
		*/
		GMeshEdge2D<DATA_TYPE>* DelaunayLocate(const GPoint<DATA_TYPE, 2>& P, GMeshEdge2D<DATA_TYPE> *StartEdge);
		/*!
			Inserts a new point into a subdivision representing a Delaunay triangulation, and fixes the
			affected edges so that the result is still a Delaunay triangulation. This is based on the
			pseudo code from Guibas and Stolfi, with slight modifications and a bug fix.
		*/
		GMeshEdge2D<DATA_TYPE> *DelaunayInsertSite(const GPoint<DATA_TYPE, 2>& P, const DATA_TYPE Tolerance,
												GMeshEdge2D<DATA_TYPE> *LastIndertedEdge);
		//! Essentially turns edge e counterclockwise inside its enclosing quadrilateral
		void DelaunaySwap(GMeshEdge2D<DATA_TYPE>* e);

	protected:
		// Cloning function.
		GError BaseClone(const GMesh2D<DATA_TYPE>& Source);

	public:
		//! Detach the edge from mesh
		void DetachEdge(GMeshEdge2D<DATA_TYPE> *Edge);
		/*!
			Return the edge with a given origin vertex in the face orbit of a given edge.
			
			\param Edge an edge of the orbit to look for the vertex in (must be non NULL)
			\param Org the origin vertex to look for (must be non NULL)
			\return the edge in the same face orbit as Edge with origin vertex Org; NULL if not found.
		*/
		GMeshEdge2D<DATA_TYPE> *OrbitOrg(GMeshEdge2D<DATA_TYPE> *Edge, GMeshVertex2D<DATA_TYPE> *Org);
		/*!
			Set the origin of the vertex orbit of a given edge to a given vertex.

			\param Edge an edge of the orbit to set the origin vertex of (must be non NULL)
			\param Org the new origin vertex (must be non NULL)
		*/
		void SetOrbitOrg(GMeshEdge2D<DATA_TYPE> *Edge, GMeshVertex2D<DATA_TYPE> *Org);

		/*!
			Splice a given pair of (non NULL) edges.
			This operator affects the two edge rings around the origins of a and b,
			and, independently, the two edge rings around the left faces of a and b.
			In each case:\n
				- if the two rings are distinct, Splice will combine them into one
				- if the two are the same ring, Splice will break it into two separate pieces.
			
			Thus, Splice can be used both to attach the two edges together, and to break them apart. See Guibas
			and Stolfi (1985) p.96 for more details and illustrations.
		*/
		static void Splice(GMeshEdge2D<DATA_TYPE>* a, GMeshEdge2D<DATA_TYPE>* b);

	public:
		//! Default constructor; it initialize this mesh consisting of no vertexes and no faces.
		GMesh2D();
		// Constructor with owner (kernel) specification; it initialize this mesh consisting of no vertexes and no faces.
		//GMesh2D(const GElement* Owner);

		/*!
			Assignment operator.
			
			It assigns to this instance a perfect (physical) copy of the source mesh.
		*/
		inline GMesh2D<DATA_TYPE>& operator =(const GMesh2D<DATA_TYPE>& Source) {

			GError err = BaseClone(Source);
			if (err != G_NO_ERROR) {
				G_DEBUG("GMesh2D assignment operator, BaseClone has returned this error code: " + StrUtils::ToString(err));
			}
			return *this;
		}

		//! Clear mesh (delete all faces, vertexes and edges)
		void Clear();
		//! Build a mesh from a classic vertex-indices arrays
		GError BuildFromFaces(const GDynArray< GPoint<DATA_TYPE, 2> >& Points, const GDynArray<GIndexList>& FacesIndexes,
							  const GBool Deep2ManifoldCheck = G_TRUE);
		//! Build an incremental Delaunay triangulation.
		GError BuildFromPointsCloud(const GDynArray< GPoint<DATA_TYPE, 2> >& Points,
									const DATA_TYPE Epsilon = (DATA_TYPE)G_EPSILON);
		//! Destructor, it release the storage occupied by the contents of this mesh.
		~GMesh2D();
		//! Add a new sub manifold.
		GMeshEdge2D<DATA_TYPE> *AddSubManifold();
		/*!
			Return a new edge formed by splitting a given vertex between a given pair of faces.
			A new vertex is introduced at the destination of the new edge.
			The new edge has Left along its left and Right along its right.
			
			\param Vertex the vertex to split to make the new edge (must be non NULL), must share an edge with both
			Left and Right_
			\param Left the left face adjacent to the new edge (must be non NULL)
			\param Right the right face adjacent to the new edge (must be non NULL)
			\note Left and Right must share an edge with Vertex.
		*/
		GMeshEdge2D<DATA_TYPE> *MakeVertexEdge(GMeshVertex2D<DATA_TYPE>* Vertex,
											   GMeshFace2D<DATA_TYPE>* Left, GMeshFace2D<DATA_TYPE>* Right);
		//! Delete a given edge from this mesh, along with its destination vertex (must be non NULL).
		void KillVertexEdge(GMeshEdge2D<DATA_TYPE>* Edge);
		/*!
			Return a new edge formed by splitting a given face through a given pair of vertexes.
			A new face is introduced to the right of the new edge.
			The new edge has Org as its origin and Dest as its destination.
			\param Face the face to divide to make the new edge (must be non NULL), must have both Org and Dest
			on its perimeter.
			\param Org the origin vertex of the new edge (must be non NULL);
			\param Dest the destination vertex the new edge (must be non NULL);
			\note Org and Dest must be located on the perimeter of Face.
		*/
		GMeshEdge2D<DATA_TYPE> *MakeFaceEdge(GMeshFace2D<DATA_TYPE>* Face, GMeshVertex2D<DATA_TYPE>* Org,
											 GMeshVertex2D<DATA_TYPE>* Dest);
		//! Delete a given edge from this mesh, along with its right face (must be non NULL)
		void KillFaceEdge(GMeshEdge2D<DATA_TYPE>* Edge);
		//! Return the number of vertexes in this mesh.
		GUInt32 VerticesCount() const {
			return (GUInt32)gVertices.size();
		}
		//! Get Index-th vertex (NULL if Index is out of range).
		GMeshVertex2D<DATA_TYPE>* Vertex(const GUInt32 Index);
		//! Add a given vertex to this mesh (it will be placed geometrically at the origin).
		GMeshVertex2D<DATA_TYPE> *AddVertex();
		//! Add a vertex to this mesh, specifying also its geometric position.
		GMeshVertex2D<DATA_TYPE> *AddVertex(const GPoint<DATA_TYPE, 2>& Position);
		//! Add a new edge
		GMeshEdge2D<DATA_TYPE> *AddEdge();
		//! Return the number of faces in this mesh.
		GUInt32 FacesCount() const {
			return (GUInt32)gFaces.size();
		}
		//! Get Index-th face (NULL if Index is out of range)
		GMeshFace2D<DATA_TYPE>* Face(const GUInt32 Index);
		//! Add a given face to this mesh
		GMeshFace2D<DATA_TYPE> *AddFace();
		//! Remove edge from internal list (and delete its pointer), without doing nothing else on mesh
		void RemoveEdge(GMeshEdge2D<DATA_TYPE> *Edge);
		//! Remove face from internal list (and delete its pointer), without doing nothing else on mesh
		void RemoveFace(GMeshFace2D<DATA_TYPE> *Face);
		/*!
			Remove a given vertex from this mesh (and delete its pointer), without doing nothing else on mesh

			\param Vertex the vertex to remove (must be non NULLl and must be in the mesh)
		*/
		void RemoveVertex(GMeshVertex2D<DATA_TYPE> *Vertex);
		//! Find a mesh vertex with the corresponding geometric coordinates.
		GMeshVertex2D<DATA_TYPE> *FindVertex(const DATA_TYPE X, const DATA_TYPE Y);
		/*!
			Add a new edge connecting the destination of a to the origin of b, in such a way that
			all three have the same left face after the connection is complete.
		*/
		GMeshEdge2D<DATA_TYPE>* Connect(GMeshEdge2D<DATA_TYPE>* a, GMeshEdge2D<DATA_TYPE>* b);
		/*!
			Return G_TRUE if a given pair of vertexes is connected directly by an edge along a given left face.

			\param Vert1 a first vertex to check (must be non NULL)
			\param Vert2 a second vertex to check (must be non NULL)
			\param Left the left face to check for (must be non NULL)
		*/
		static GBool IsConnected(GMeshVertex2D<DATA_TYPE> *Vert1, GMeshVertex2D<DATA_TYPE> *Vert2,
								GMeshFace2D<DATA_TYPE> *Left);
		/*!
			Return the face to the right of a given face around a given vertex (NULL if none)
			
			\param Vertex the vertex to look for the face around (must be non NULL)
			\param Left the left face to return the right face of (must be non NULL)
		*/
		static GMeshFace2D<DATA_TYPE> *RightFace(GMeshVertex2D<DATA_TYPE> *Vertex, GMeshFace2D<DATA_TYPE> *Left);
		// Get class descriptor
		/*const GClassID& ClassID() const {
			return G_MESH2D_CLASSID;
		}*/
		// Get base class (father class) descriptor
		/*const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}*/
	};

	// *********************************************************************
	//                             GMesh2DProxy
	// *********************************************************************

	/*
		\class GMesh2DProxy
		\brief This class implements a GMesh2D proxy (provider).

		This proxy provides the creation of GMesh2D class instances.
	*/
	/*
	class G_EXPORT GMesh2DProxy : public GElementProxy {
	public:
		//! Creates a new GMesh2D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GMesh2D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_MESH2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};

	static const GMesh2DProxy G_MESH2D_PROXY;*/

	// *********************************************************************
	//                            GMeshVertexIterator2D
	// *********************************************************************

	// Enumerates the vertexes of a given mesh in arbitrary order.
	template <typename DATA_TYPE>
	class GMeshVertexIterator2D {
	private:
		// the mesh whose vertexes are being iterated
		GMesh2D<DATA_TYPE> *gMesh;
		// the number of vertexes left to iterate
		GUInt32 gCount;

	public:
		// constructor, it initialize this vertex iterator over a given mesh
		GMeshVertexIterator2D(GMesh2D<DATA_TYPE> *Mesh) {
			gMesh  = Mesh;
			gCount = Mesh->VerticesCount();
		}
		// destructor, release the storage occupied by this vertex iterator
		~GMeshVertexIterator2D() {
		}
		// return the next vertex of this vertex iterator, if any (null if none)
		GMeshVertex2D<DATA_TYPE> *Next() {
			// iterate the array in reverse order so that the current vertex can be
			// removed during iteration
			if (gCount < 1)
				return NULL;
			return gMesh->gVertices[--gCount];
		}
	};


	// *********************************************************************
	//                          GMeshFaceIterator2D
	// *********************************************************************

	// enumerates the faces of a given mesh in arbitrary order
	template <typename DATA_TYPE>
	class GMeshFaceIterator2D {

	private:
		// the mesh whose faces are being iterated
		GMesh2D<DATA_TYPE> *gMesh;
		// the number of faces left to iterate
		GUInt32 gCount;

	public:
		// constructor, it initialize this face iterator over a given mesh (must be non-null)
		GMeshFaceIterator2D(GMesh2D<DATA_TYPE> *Mesh) {
			gMesh  = Mesh;
			gCount = Mesh->FacesCount();
		}
		// release the storage occupied by this face iterator
		~GMeshFaceIterator2D() {
		}
		// return the next face of this face iterator, if any (null if none)
		GMeshFace2D<DATA_TYPE> *Next() {
			// iterate the array in reverse order so that the current face can be
			// removed during iteration
			if (gCount < 1)
				return NULL;
			return gMesh->gFaces[--gCount];
		}
	};

};	// end namespace Amanith

#endif
