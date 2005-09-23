/****************************************************************************
** $file: amanith/2d/ganimtrsnode2d.h   0.1.1.0   edited Sep 24 08:00
**
** 2D TRS (Translation-Rotation-Scale) animated node definition.
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

#ifndef GANIMTRSNODE2D_H
#define GANIMTRSNODE2D_H

#include "amanith/gelement.h"

/*!
	\file ganimtrsnode2d.h
	\brief Header file for GAnimTRSNode2D class.
*/

namespace Amanith {

	// *********************************************************************
	//                            GAnimTRSNode2D
	// *********************************************************************
	//! GAnimTRSNode2D static class descriptor.
	static const GClassID G_ANIMTRSNODE2D_CLASSID = GClassID("GAnimTRSNode2D", 0x8510F345, 0xDC5A493E, 0xA26B6033, 0x95095825);

	//! Available coordinates space systems.
	enum GSpaceSystem {
		//! Local space.
		G_LOCAL_SPACE = 0,
		//! World space.
		G_WORLD_SPACE = 1
	};

	/*!
		\class GAnimTRSNode2D
		\brief This class implements a simple TRS (Translate-Rotation-Scale) transformation node, in a 2D space.

		A TRS node can have a father and children, as well none of them. If a father is linked, all internal
		animated TRS track are stored relative to father's coordinate system. A local pivot point (intended as a
		full TRS point) is also supported.
	*/
	class G_EXPORT GAnimTRSNode2D : public GAnimElement {

	private:
		//! Pivot position, it's expressed in local space.
		GPoint2 gPivotPosition;
		//! Pivot rotation, it's expressed in local space.
		GReal gPivotRotation;
		//! Pivot scale, it's expressed in local space.
		GVector2 gPivotScale;
		//! Father node.
		GAnimTRSNode2D *gFather;
		//! List of children.
		GDynArray<GAnimTRSNode2D *>gChildren;
		//! Associated custom/user data
		void *gCustomData;

	protected:
		//! Move all position keys by an additive offset vector.
		void OffsetPositionTrack(const GVector2& OffsetVector);
		//! Move all rotation keys by an additive offset angle (expressed in radiant).
		void OffsetRotationTrack(const GReal OffsetAngle);
		//! Scale all scaling keys by an offset factor.
		void OffsetScaleTrack(const GVectBase<GReal, 2>& OffsetFactors);
		/*!
			Link child node, adding the specified node to the internal children list; NB: it's just a matter of
			add the node pointer to the internal list, remove the node pointer form its father and anything else.
			G_TRUE is returned if Node already exists into internal list, otherwise G_FALSE.
		*/
		GBool AttachChildNode(GAnimTRSNode2D *Node);
		//! Remove the specified child (by index) from the internal children list.
		void DetachChildNode(const GUInt32 ChildIndex);
		/*!
			Remove the specified child (by pointer) from the internal children list; if specified ChildNode
			was not found inside children list, a G_FALSE value is returned, G_TRUE otherwise.
		*/
		GBool DetachChildNode(const GAnimTRSNode2D *ChildNode);
		//! Basic (physical) cloning function.
		GError BaseClone(const GElement& Source);

	public:
		//! Default constructor.
		GAnimTRSNode2D();
		//! Default constructor with owner specification.
		GAnimTRSNode2D(const GElement* Owner);
		//! Destructor, delete all keys and internal ease curve (if it exists).
		~GAnimTRSNode2D();
		//! Get custom/user data associated to this node.
		inline void *CustomData() const {
			return gCustomData;
		}
		//! Set custom/user data associated to this node.
		inline void SetCustomData(void* NewData) {
			gCustomData = NewData;
		}
		//! Get the parent node (NULL is root).
		inline GAnimTRSNode2D *Father() const {
			return gFather;
		}
		/*!
			Set a new father, taking care to transform all internal animated tracks relative to father's tracks.
			If AffectTracks is G_TRUE, all TRS tracks are modified to respect the new father (all existing
			values are calculate relative to the new father).
		*/
		GError SetFather(GAnimTRSNode2D *NewFather, const GBool AffectTracks = G_TRUE);
		//! Get a flag that indicate if this is a root (fatherless) node.
		inline GBool IsRoot() const {
			if (gFather)
				return G_FALSE;
			return G_TRUE;
		}
		//! Get a flag that indicate if this is a leaf node.
		inline GBool IsLeaf() const {
			if (ChildrenCounts() == 0)
				return G_TRUE;
			return G_FALSE;
		}
		//! Get number of children.
		inline GUInt32 ChildrenCounts() const {
			return (GUInt32)gChildren.size();
		}
		//! Get Index-th child (NULL if it does not exist).
		inline GAnimTRSNode2D *Child(const GUInt32 Index) const {
			if (Index >= (GUInt32)gChildren.size())
				return NULL;
			return gChildren[Index];
		}
		//! Get pivot matrix, it include only pivot transformations.
		GMatrix33 PivotMatrix() const;
		//! Get pivot inverse matrix.
		GMatrix33 InversePivotMatrix() const;
		/*!
			Get node matrix, specifying coordinates space system. It does not include pivot matrix.
			ValidInterval is the validity interval for the returned matrix.
		*/
		GMatrix33 Matrix(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const;
		/*!
			Get inverse node matrix, specifying coordinates space system. It does not include (inverse) pivot matrix.
			ValidInterval is the validity interval for the returned matrix.
		*/
		GMatrix33 InverseMatrix(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const;
		/*!
			Get position component, specifying time and space type (local or world).
			ValidInterval is the validity interval for the returned point.
		*/
		GPoint2 Position(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const;
		/*!
			Set position component, specifying a father-related position and time.
		*/
		GError SetPosition(const GTimeValue TimePos, const GVectBase<GReal, 2>& RelPosition);
		/*!
			Get rotation component (in radiants), specifying time and space type (local or world).
			ValidInterval is the validity interval for the returned point.
		*/
		GReal Rotation(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const;
		/*!
			Set rotation component (in radiants), specifying a father-related rotation and time.
		*/
		GError SetRotation(const GTimeValue TimePos, const GReal& RelRotation);
		/*!
			Get scale component, specifying time and space type (local or world).
			ValidInterval is the validity interval for the returned point.
		*/
		GVectBase<GReal, 2> Scale(const GTimeValue TimePos, const GSpaceSystem Space, GTimeInterval& ValidInterval) const;
		/*!
			Set scale component, specifying a father-related scale and time.
		*/
		GError SetScale(const GTimeValue TimePos, const GVectBase<GReal, 2>& RelScale);
		//! Get pivot position.
		inline const GPoint2& PivotPosition() const {
			return gPivotPosition;
		}
		/*!
			Set pivot position (in local space). AffectChildren flag, if G_TRUE, indicates that all
			children must be counter-transformed (they tracks will be transformed) to maintain their actual
			settings.
		*/
		GError SetPivotPosition(const GVectBase<GReal, 2>& NewPosition, const GBool AffectChildren = G_TRUE);
		//! Get pivot rotation (in radiants).
		inline GReal PivotRotation() const {
			return gPivotRotation;
		}
		/*!
			Set pivot rotation (in local space, expressed in radiants). AffectChildren flag, if G_TRUE, indicates
			that all children must be counter-transformed (they tracks will be transformed) to maintain their actual
			settings.
		*/
		GError SetPivotRotation(const GReal& NewAngle, const GBool AffectChildren = G_TRUE);
		//! Get pivot scale.
		inline const GVectBase<GReal, 2>& PivotScale() const {
			return gPivotScale;
		}
		/*!
			Set pivot scale (in local space). AffectChildren flag, if G_TRUE, indicates that all
			children must be counter-transformed (they tracks will be transformed) to maintain their actual
			settings.
		*/
		GError SetPivotScale(const GVectBase<GReal, 2>& NewScaleFactors, const GBool AffectChildren = G_TRUE);
		//! Get class descriptor
		inline const GClassID& ClassID() const {
			return G_ANIMTRSNODE2D_CLASSID;
		}
		//! Get base class (father class) descriptor
		inline const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
	};

	// *********************************************************************
	//                            GAnimTRSNode2DProxy
	// *********************************************************************
	/*!
		\class GAnimTRSNode2DProxy
		\brief This class implements a GAnimTRSNode2D proxy (provider).

		This proxy provides the creation of GAnimTRSNode2D class instances.
	*/
	class G_EXPORT GAnimTRSNode2DProxy : public GElementProxy {
	public:
		//! Creates a new GAnimTRSNode2D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GAnimTRSNode2D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		inline const GClassID& ClassID() const {
			return G_ANIMTRSNODE2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		inline const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
	};
	//! Static proxy for GProperty class.
	static const GAnimTRSNode2DProxy G_ANIMTRSNODE2D_PROXY;

};	// end namespace Amanith

#endif
