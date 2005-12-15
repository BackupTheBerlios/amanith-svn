/****************************************************************************
** $file: amanith/2d/gtesselator2d.h   0.2.0.0   edited Dec, 12 2005
**
** 2D Polygonal shape tesselator definition.
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

#ifndef GTESSELATOR2D_H
#define GTESSELATOR2D_H

#include "amanith/2d/gmesh2d.h"
#include "amanith/geometry/garea.h"
#include "amanith/geometry/gaabox.h"
#include "amanith/support/gavltree.h"

/*!
	\file gtesselator2d.h
	\brief 2D shape triangulator.
*/
namespace Amanith {

	// *********************************************************************
	//                          GTesselator2D
	// *********************************************************************

	//! Fill rules for tessellation
	enum GFillBehavior {
		//! Odd-even rule
		G_ODD_EVEN_RULE,
		//! Even-odd rule
		G_EVEN_ODD_RULE,
		//! Non-zero rule (not yet supported)
		G_NON_ZERO_RULE,
		//! Any rule
		G_ANY_RULE
	};

	/*!
		\class GTesselator2D
		\brief This class implements a 2D shape triangulator.

		Amanith triangulator handles holes and non-simple contours. Some common degenerations are supported too, for example
		repeated points, coincident points and (partially)overlapping edges.
		The implemented algorithm is a robust sweep-line. It consist of two main steps:\n\n

		-# Decompose the 'interior' region (the plane region that will be solid/filled) into X-axis monotone disjointed
		regions.
		-# Triangulate each monotone piece with a greedy linear (in time complexity) algorithm.\n\n

		The general step is to sweep a vertical line, as the classic Bentley-Ottmann algorithm do. The so called
		Y-structure (also known as edges dictionary) is an AVL tree (maybe next version will use a skip list), and the
		so called X-structure is a priority queue (implemented as a simple sorted list).\n
		The overall complexity is (N + K)Log2(N), where N is the total number of segments, K is the number of
		intersections that occur between segments (intersections can derive from non simple contours or they can be due
		to contours intersections), and Log2 is the logarithm to base 2 function.
		Internally, the topology of generated monotone regions, is maintained by a GMesh2D, that is used to quickly
		manage adjacencies and split/merge operations.\n
		For a simple triangulation history and theory you can check this clear
		document (we have based out work also on this paper):
		http://www.cs.ucsb.edu/~suri/cs235/Triangulation.pdf
	*/
	class G_EXPORT GTesselator2D {

	private:

		struct GExtVertex {
			GMeshVertex2D<GDouble> *MeshVertex;
			// constructor
			GExtVertex() {
				MeshVertex = NULL;
			}
			GExtVertex(GMeshVertex2D<GDouble>* VertMesh) : MeshVertex(VertMesh) {
			}
		};
		// #define GExtVertex GMeshVertex2D<GDouble>

		struct GActiveRegion {
			GMeshEdge2D<GDouble> *MeshUpperEdge;  // upper edge, directed right to left
			GInt32 CrossingNumber;
			GBool Valid;
		};

		struct GMeshToAVL {
			GInt32 EdgeType;
			GInt32 CrossingNumber;
			GBool IsIntoDictionary;
			GAVLNode *AVLNode;
			GActiveRegion *Region;
			GBool HasBeenIntoDictionary;
		};

		struct GTessDescriptor;

		class GDictionaryTree : public GAVLTree {
		public:
			GTessDescriptor *DescPointer;

			// constructor
			GDictionaryTree() {
			}
			// destructor
			~GDictionaryTree() {
			}
			GInt32 Compare(void *ItemA, void *ItemB);
		};

		struct GTessDescriptor {
			GMesh2D<GDouble> TargetMesh;
			GMeshEdge2D<GDouble> *LastEdge;
			GPoint<GDouble, 2> LastPoints[2];
			GMeshVertex2D<GDouble> *FirstPushedPoints[2];
			GInt32 StepsDone;
			GInt32 FirstPushedSteps;
			GInt32 PushedCount;
			GMeshVertex2D<GDouble> *CurrentEvent;
			GActiveRegion *LastRegion;
			GMeshEdge2D<GDouble> *LastRegionEdge;
			std::list< GExtVertex* > PriorityTree;
			GDictionaryTree DictionaryTree;

			GDynArray< GMeshEdge2D<GDouble> *> MeshContours;
			GDynArray<GExtVertex *> ExtVertices;
			GDynArray<GMeshToAVL *> ExtEdges;
			GDynArray<GActiveRegion *> ActiveRegions;
			GULong VertexID;

			// constructor
			GTessDescriptor() {
				LastEdge = NULL;
				FirstPushedPoints[0] = FirstPushedPoints[1] = NULL;
				StepsDone = FirstPushedSteps = PushedCount = 0;
				CurrentEvent = NULL;
				LastRegion = NULL;
				LastRegionEdge = NULL;
				DictionaryTree.DescPointer = this;
				VertexID = 0;
			}
		};

		static bool SweepGreater(const GExtVertex* Event1, const GExtVertex* Event2);

		// debug functions
		/*static void DebugOpenFile(std::FILE **File, const GChar8 *FileName);
		static void DebugCloseFile(std::FILE **File);
		static void DebugWrite(std::FILE *File, const GChar8* Text);*/
		static void DebugDumpDictionary(std::FILE *File, GDictionaryTree& Dictionary, GMeshVertex2D<GDouble> *Event);
		static void DebugDumpOrgRing(std::FILE *File, GMeshVertex2D<GDouble> *Vertex);

	protected:

		void BeginContour(const GDouble X, const GDouble Y, GTessDescriptor& Descriptor);
		void AddContourPoint(const GDouble X, const GDouble Y, GTessDescriptor& Descriptor);
		void EndContour(GTessDescriptor& Descriptor);
		void EndTesselletionData(GTessDescriptor& Descriptor);

		// return true is Edge is left-going from Vertex
		GBool IsLeftGoing(GMeshEdge2D<GDouble> *Edge, GMeshVertex2D<GDouble> *Vertex);
		GBool IsLeftGoingFast(GMeshEdge2D<GDouble> *Edge, GMeshVertex2D<GDouble> *Vertex);

		// return true is Edge is right-going from Vertex
		GBool IsRightGoing(GMeshEdge2D<GDouble> *Edge, GMeshVertex2D<GDouble> *Vertex);
		GBool ProcessRightGoingEdges(GMeshVertex2D<GDouble> *EventVertex, GTessDescriptor& Descriptor);
		// add to the edge dictionary a mesh edge
		GMeshEdge2D<GDouble> *AddDictionaryEdge(GMeshEdge2D<GDouble> *MeshEdge, const GInt32 Flags, GBool& RevisitFlag,
												GTessDescriptor& Descriptor);
		// events queue functions
		GMeshVertex2D<GDouble>* InsertEventNoSort(GMeshVertex2D<GDouble> *EventVertex, GTessDescriptor& Descriptor);
		GMeshVertex2D<GDouble>* InsertEventSort(GMeshVertex2D<GDouble> *EventVertex, GTessDescriptor& Descriptor);
		// diagonal tracing functions
		GMeshEdge2D<GDouble> *TraceLeftDiagonal(GMeshVertex2D<GDouble> *Origin, GMeshVertex2D<GDouble> *Destination,
												GTessDescriptor& Descriptor);
		GMeshEdge2D<GDouble> *TraceRightDiagonal(GMeshVertex2D<GDouble> *Origin, GMeshVertex2D<GDouble> *Destination,
												GTessDescriptor& Descriptor);
		// intersection routines
		GInt32 CheckIntersection(const GPoint<GDouble, 2>& Event,
								 GMeshEdge2D<GDouble> *EdgeAbove, GMeshEdge2D<GDouble> *EdgeBelow,
								 GPoint<GDouble, 2>& IntersectionPoint);
		GBool DoIntersection(GMeshEdge2D<GDouble> *EdgeAbove, GMeshEdge2D<GDouble> *EdgeBelow,
							 GInt32& IntersectionType, GTessDescriptor& Descriptor);
		void ManageIntersections(GMeshEdge2D<GDouble> *EdgeAbove, GMeshEdge2D<GDouble> *EdgeBelow,
								 const GPoint<GDouble, 2>& IntersectionPoint, GTessDescriptor& Descriptor);
		GMeshEdge2D<GDouble> *ManageDegenerativeIntersections(GMeshEdge2D<GDouble> *Touched,
															  GMeshEdge2D<GDouble> *UnTouched,
															  const GPoint<GDouble, 2>& IntersectionPoint,
															  GTessDescriptor& Descriptor);
		// merging rings functions
		static GMeshVertex2D<GDouble> *MergeRings(GMeshVertex2D<GDouble> *Ring1Vertex,
												  GMeshVertex2D<GDouble> *Ring2Vertex,
												  GMesh2D<GDouble> *Mesh);
		static GMeshEdge2D<GDouble> *SafeRemoveEdgeFromVertex(GMeshEdge2D<GDouble> *Edge);
		static void DoInsertEdge(GMeshEdge2D<GDouble> *EdgeToInsert, GMeshEdge2D<GDouble> *RingEdge, GMesh2D<GDouble> *Mesh);
		// tessellation routines
		GBool CloseRegion(GMeshEdge2D<GDouble> *UpperEdge, GDynArray<GActiveRegion *>& ActiveRegions,
						  GTessDescriptor& Descriptor);
		GBool CloseRegions(GMeshVertex2D<GDouble> *EventVertex, GDynArray<GActiveRegion *>& ActiveRegions,
						   GAVLNode **UpperBounder, GAVLNode **LowerBounder, GBool& RevisitFlag,
						   GTessDescriptor& Descriptor);

		GBool PatchRightDiagonal(GMeshVertex2D<GDouble> *Event, GAVLNode *UpperBounder, GAVLNode *LowerBounder,
								GTessDescriptor& Descriptor);
		GExtVertex *MergeCoincidentVertices(GTessDescriptor& Descriptor);
		void SimplifyEdges(GMeshVertex2D<GDouble> *Event, GDynArray<GActiveRegion *>& ActiveRegions,
						   GTessDescriptor& Descriptor);
		GBool SweepEvent(GExtVertex* Event, GTessDescriptor& Descriptor);
		GUInt32 PurgeRegions(GDynArray<GActiveRegion *>& ActiveRegions, const GBool Fast,
							 GTessDescriptor& Descriptor);

		void TessellateMonotoneRegion(const GActiveRegion* Region, GDynArray< GPoint<GDouble, 2> >& Points,
									  GTessDescriptor& Descriptor);
		void TessellateMonotoneRegion(const GActiveRegion* Region, GDynArray<GULong>& PointsIds,
									  GTessDescriptor& Descriptor);

		// searching into RingEdge's origin ring, return that edge that span the smaller angle in CCW
		// direction to meet an edge specified by its Origin and its Destination
		static GMeshEdge2D<GDouble> *CCWSmallerAngleSpanEdge(GMeshEdge2D<GDouble> *RingEdge,
															 const GPoint<GDouble, 2>& Origin,
															 const GPoint<GDouble, 2>& Destination);

		static GBool ValidateInput(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour);
		static void FreeTessellation(GTessDescriptor& Descriptor);

	public:
		//! Default constructor.
		GTesselator2D();
		//! Destructor.
		~GTesselator2D();
		/*!
			Build a valid triangulation of given contours and holes.
			The triangulation is based on a robust sweep-line algorithm, see GTesselator2D class reference
			and forum posts for more details and use example. Unlike other triangulators, Amanith tesselator
			does not require that contours (be they holes or not) must be given in ccw or cw order. This make
			Amanith tesselator	more general and adaptable to other high-level libraries.
			Here's an example of the typical use:
\code
	GDynArray< GPoint<GDouble, 2> > pts;
	GDynArray<GInt32> idx;

	// push point for a square
	pts.push_back(GPoint2(0, 0));
	pts.push_back(GPoint2(10, 0));
	pts.push_back(GPoint2(10, 10));
	pts.push_back(GPoint2(0, 10));
	idx.push_back(4);
	// push points for a triangle hole (inside the previous square).
	pts.push_back(GPoint2(3, 3));
	pts.push_back(GPoint2(7, 3));
	pts.push_back(GPoint2(5, 8));
	idx.push_back(3);

	GDynArray< GPoint<GDouble, 2> > triPoints;
	GTesselator2D tesselator;

	tesselator.Tesselate(pts, idx, triPoints);
	for (GUInt32 i = 0; i < triPoints.size(); i+=3)
		DrawTriangle(triPoints[i], triPoints[i + 1], triPoints[i + 2]);
\endcode
			\param Points the array containing the contours points.
			\param PointsPerContour an array containing the number of point for every contour.
			\param Triangles the outputted array of triangles. Every triangle is built by 3 vertexes.
			\param FillRule the filling rule.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note Please keep OddFill parameter G_TRUE. This is the only supported mode for this version.
		*/
		GError Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
						 GDynArray< GPoint<GDouble, 2> >& Triangles, const GFillBehavior FillRule = G_ODD_EVEN_RULE);

		GError Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
						GDynArray< GPoint<GDouble, 2> >& Triangles, GAABox2& BoundingBox, const GFillBehavior FillRule = G_ODD_EVEN_RULE);

		/*!
			Build a valid triangulation of given contours and holes.
			The triangulation is based on a robust sweep-line algorithm, see GTesselator2D class reference
			and forum posts for more details and use example. Unlike other triangulators, Amanith tesselator
			does not require that contours (be they holes or not) must be given in ccw or cw order. This make
			Amanith tesselator more general and adaptable to other high-level libraries.
			Here's an example of the typical use:
\code
	GDynArray< GPoint<GDouble, 2> > pts;
	GDynArray<GInt32> idx;

	// push point for a square
	pts.push_back(GPoint2(0, 0));
	pts.push_back(GPoint2(10, 0));
	pts.push_back(GPoint2(10, 10));
	pts.push_back(GPoint2(0, 10));
	idx.push_back(4);
	// push points for a triangle hole (inside the previous square).
	pts.push_back(GPoint2(3, 3));
	pts.push_back(GPoint2(7, 3));
	pts.push_back(GPoint2(5, 8));
	idx.push_back(3);

	GDynArray< GPoint<GDouble, 2> > triPoints;
	GDynArray<GULong> triIndexes;
	GTesselator2D tesselator;

	tesselator.Tesselate(pts, idx, triPoints, triIndexes);
	for (GUInt32 i = 0; i < triPoints.size(); i+=3)
		DrawTriangle(triPoints[triIndexes[i]], triPoints[triIndexes[i + 1]], triPoints[triIndexes[i + 2]]);
\endcode
			\param Points the array containing the contours points.
			\param PointsPerContour an array containing the number of point for every contour.
			\param TriangPoints the outputted array of triangles points.
			\param TriangIds the outputted array of triangles indexes. Each triangle is made of 3 vertexes, so
			in this array there will be triplets of indexes, one triple for each triangle. Indexes are
			consecutive, so first triplet describes the first triangle, second triplet describes the second
			triangle, and so on.
			\param FillRule the filling rule.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note Please keep OddFill parameter G_TRUE. This is the only supported mode for this version.
		*/
		GError Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
						 GDynArray< GPoint<GDouble, 2> >& TriangPoints, GDynArray< GULong >& TriangIds,
						 const GFillBehavior FillRule = G_ODD_EVEN_RULE);


		GError Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
						GDynArray< GPoint<GDouble, 2> >& TriangPoints, GDynArray< GULong >& TriangIds,
						GAABox2& BoundingBox, const GFillBehavior FillRule = G_ODD_EVEN_RULE);

	};

};	// end namespace Amanith

#endif
