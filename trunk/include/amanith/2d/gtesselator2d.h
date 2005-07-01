/****************************************************************************
** $file: amanith/2d/gtesselator2d.h   0.1.0.0   edited Jun 30 08:00
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
#include "amanith/support/gavltree.h"

/*!
	\file gtesselator2d.h
	\brief 2D shape triangulator.
*/
namespace Amanith {


	// *********************************************************************
	//                          GTesselator2D
	// *********************************************************************
	class G_EXPORT GTesselator2D {

	private:

		#define UNDEFINED_VERTEX	-1
		#define NORMAL_VERTEX		0
		#define SPLIT_VERTEX		1
		#define MERGE_VERTEX		2

		struct GExtVertex {
			GInt32 Flags;
			GMeshVertex2D *MeshVertex;
			// constructor
			GExtVertex() {
				Flags = UNDEFINED_VERTEX;
				MeshVertex = NULL;
			}
			GExtVertex(const GInt32 VertFlags, GMeshVertex2D* VertMesh)	: Flags(VertFlags), MeshVertex(VertMesh) {
			}
		};

		struct GActiveRegion {
			GMeshEdge2D *MeshUpperEdge;  // upper edge, directed right to left
			GInt32 CrossingNumber;
			GBool Valid;
		};

		struct GMeshToAVL {
			GInt32 EdgeType;
			GInt32 CrossingNumber;
			GBool IsIntoDictionary;
			GAVLNode *AVLNode;
			GActiveRegion *Region;
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
			int Compare(void *ItemA, void *ItemB);
		};

		struct GTessDescriptor {
			GMesh2D TargetMesh;
			GMeshEdge2D *LastEdge;
			GPoint2 LastPoints[2];
			GMeshVertex2D *FirstPushedPoints[2];
			GInt32 StepsDone;
			GInt32 FirstPushedSteps;
			GInt32 PushedCount;
			GMeshVertex2D *CurrentEvent;
			GActiveRegion *LastRegion;
			GMeshEdge2D *LastRegionEdge;
			std::list<GExtVertex *> PriorityTree;
			GDictionaryTree DictionaryTree;

			GDynArray<GMeshEdge2D *> MeshContours;
			GDynArray<GExtVertex *> ExtVertices;
			GDynArray<GMeshToAVL *> ExtEdges;
			GDynArray<GActiveRegion *> ActiveRegions;

			// constructor
			GTessDescriptor() {
				LastEdge = NULL;
				FirstPushedPoints[0] = FirstPushedPoints[1] = NULL;
				StepsDone = FirstPushedSteps = PushedCount = 0;
				CurrentEvent = NULL;
				LastRegion = NULL;
				LastRegionEdge = NULL;
				DictionaryTree.DescPointer = this;
			}
		};

		static bool SweepGreater(const GExtVertex *Event1, const GExtVertex *Event2);

	protected:

		void BeginContour(const GReal X, const GReal Y, GTessDescriptor& Descriptor);
		void AddContourPoint(const GReal X, const GReal Y, GTessDescriptor& Descriptor);
		void EndContour(GTessDescriptor& Descriptor);
		void EndTesselletionData(GTessDescriptor& Descriptor);

		// return true is Edge is left-going from Vertex
		GBool IsLeftGoing(GMeshEdge2D *Edge, GMeshVertex2D *Vertex);
		GBool IsLeftGoingFast(GMeshEdge2D *Edge, GMeshVertex2D *Vertex);

		// return true is Edge is right-going from Vertex
		GBool IsRightGoing(GMeshEdge2D *Edge, GMeshVertex2D *Vertex);
		GBool ProcessRightGoingEdges(GMeshVertex2D *EventVertex, GTessDescriptor& Descriptor);
		// add to the edge dictionary a mesh edge
		GMeshEdge2D *AddDictionaryEdge(GMeshEdge2D *MeshEdge, const GInt32 Flags, GBool& RevisitFlag,
									   GTessDescriptor& Descriptor);
		// events queue functions
		GMeshVertex2D *InsertEventNoSort(GMeshVertex2D *EventVertex, GTessDescriptor& Descriptor);
		GMeshVertex2D *InsertEventSort(GMeshVertex2D *EventVertex, GTessDescriptor& Descriptor);
		// diagonal tracing functions
		GMeshEdge2D *TraceLeftDiagonal(GMeshVertex2D *Origin, GMeshVertex2D *Destination, GTessDescriptor& Descriptor);
		GMeshEdge2D *TraceRightDiagonal(GMeshVertex2D *Origin, GMeshVertex2D *Destination, GTessDescriptor& Descriptor);
		// intersection routines
		GInt32 CheckIntersection(GMeshEdge2D *EdgeAbove, GMeshEdge2D *EdgeBelow,
								 GPoint2& IntersectionPoint1, GPoint2& IntersectionPoint2);
		GBool DoIntersection(GMeshEdge2D *EdgeAbove, GMeshEdge2D *EdgeBelow, GInt32& IntersectionType,
							 GTessDescriptor& Descriptor);
		void ManageIntersections(GMeshEdge2D *EdgeAbove, GMeshEdge2D *EdgeBelow, const GPoint2& IntersectionPoint,
								 GTessDescriptor& Descriptor);
		GMeshEdge2D *ManageDegenerativeIntersections(GMeshEdge2D *Touched, GMeshEdge2D *UnTouched,
													 const GPoint2& IntersectionPoint, GTessDescriptor& Descriptor);
		void ManageOverlappingEdges(GMeshEdge2D *Edge, GMeshEdge2D *EdgeToInsert,
									const GPoint2& IntersectionPoint1, const GPoint2& IntersectionPoint2,
									GBool& RevisitFlag, GTessDescriptor& Descriptor);
		// merging rings functions
		static GMeshVertex2D *MergeRings(GMeshVertex2D *Ring1Vertex, GMeshVertex2D *Ring2Vertex, GMesh2D *Mesh);
		static GMeshEdge2D *SafeRemoveEdgeFromVertex(GMeshEdge2D *Edge);
		static void DoInsertEdge(GMeshEdge2D *EdgeToInsert, GMeshEdge2D *RingEdge, GMesh2D *Mesh);
		// tessellation routines
		GBool CloseRegion(GMeshEdge2D *UpperEdge, GDynArray<GActiveRegion *>& ActiveRegions,
						  GTessDescriptor& Descriptor);
		GBool CloseRegions(GMeshVertex2D *EventVertex, GDynArray<GActiveRegion *>& ActiveRegions,
						   GAVLNode **UpperBounder, GAVLNode **LowerBounder, GBool& RevisitFlag,
						   GTessDescriptor& Descriptor);

		GBool PatchRightDiagonal(GMeshVertex2D *Event, GAVLNode *UpperBounder, GAVLNode *LowerBounder,
								GTessDescriptor& Descriptor);
		GExtVertex *MergeCoincidentVertices(GTessDescriptor& Descriptor);
		void SimplifyEdges(GMeshVertex2D *Event, GDynArray<GActiveRegion *>& ActiveRegions,
						   GTessDescriptor& Descriptor);
		GBool SweepEvent(GExtVertex *Event, GTessDescriptor& Descriptor);
		GUInt32 PurgeRegions(GDynArray<GActiveRegion *>& ActiveRegions, const GBool Fast,
							 GTessDescriptor& Descriptor);
		void TessellateMonotoneRegion(const GActiveRegion* Region, GDynArray<GPoint2>& Points,
									  GTessDescriptor& Descriptor);

		// searching into RingEdge's origin ring, return that edge that span the smaller angle in CCW
		// direction to meet an edge specified by its Origin and its Destination
		static GMeshEdge2D *CCWSmallerAngleSpanEdge(GMeshEdge2D *RingEdge,
													const GPoint2& Origin, const GPoint2& Destination);

	public:
		// constructor
		GTesselator2D();
		// destructor
		~GTesselator2D();
		// tessellation routine
		GError Tesselate(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& PointsPerContour,
						 GDynArray<GPoint2>& Triangles, const GBool OddFill = G_TRUE);

	};

};	// end namespace Amanith

#endif
