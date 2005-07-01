/****************************************************************************
** $file: amanith/support/gavltree.h   0.1.0.0   edited Jun 30 08:00
**
** AVL Tree definition.
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

#ifndef GAVLTREE_H
#define GAVLTREE_H

#include <amanith/gglobal.h>

/*!
	\file gavltree.h
	\brief Header file for AVL (Adelson-Velskii and Landis) tree classes.
*/

namespace Amanith {

	// *********************************************************************
	//                               GAVLNode
	// *********************************************************************

	class G_EXPORT GAVLNode {

	friend class GAVLTree;

	private:
		//! Node's children, at index 0 there is the left child, at index 1 there is the right child.
		GAVLNode *gChildren[2];
		//! Pointer to father node
		GAVLNode *gFather;
		//! Custom data associated with this node. This represents the ordering key.
		void *gCustomData;
		//! Balance factor associated to this node.
		GInt8 gBalanceFactor;

	public:
		//! Constructor
		GAVLNode();
		//! Destructor
		~GAVLNode();
		//! Get left child node. NULL if it doesn't exist.
		inline GAVLNode *LeftChild() const {
			return gChildren[0];
		}
		//! Get right child node. NULL if it doesn't exist.
		inline GAVLNode *RightChild() const {
			return gChildren[1];
		}
		//! Get father node. NULL if it doesn't exist (this is the case of the root node).
		inline GAVLNode *Father() const {
			return gFather;
		}
		//! Get custom data associated to this tree node.
		inline void *CustomData() const {
			return gCustomData;
		}
		//! Get the balance factor associated to this tree node.
		inline GInt8 BalanceFactor() const {
			return gBalanceFactor;
		}
	};

	// *********************************************************************
	//                              GAVLTree
	// *********************************************************************
	class G_EXPORT GAVLTree {

	private:
		//! Tree's root node
		GAVLNode *gRoot;
		//! Number of nodes present in the tree
		GUInt32 gNodesCount;

	protected:
		//! Try to insert a new node, with specified associated data (key).
		GAVLNode *TryToInsert(void *Data, GBool& AlreadyExists);

	public:
		//! Constructor
		GAVLTree();
		//! Destructor
		virtual ~GAVLTree();
		//! Get root node; if tree is empty a NULL pointer will be returned.
		inline GAVLNode *Root() const {
			return gRoot;
		}
		//! Get number of nodes present in the tree
		GUInt32 NodesCount() const {
			return gNodesCount;
		}
		//! Extract the minimum node, in according to the order induced by Compare() function.
		GAVLNode* Min();
		//! Extract the maximum node, in according to the order induced by Compare() function.
		GAVLNode* Max();
		//! Return the next node of the given one.
		GAVLNode* Next(GAVLNode* Node);		
		//! Return the previous node of the given one
		GAVLNode* Prev(GAVLNode* Node);
		//! Insert a new node
		GAVLNode *Insert(void *Data, GBool& AlreadyExists);
		//! Delete a node, specifying its data item (key)
		void *Delete(void *Data);
		//! Delete an already existing node
		void *DeleteNode (GAVLNode *Node);
		//! Find a node corresponding to specified data item (key)
		GAVLNode *Find (void *Data);
		//! Find a "previous node" corresponding to specified data item (key)
		GAVLNode *FindPrev(void *Data);
		//! Find a "next node" corresponding to specified data item (key)
		GAVLNode *FindNext(void *Data);
		//! Comparison function between two nodes. It must be implemented  by every derived classes.
		virtual GInt32 Compare(void *ItemA, void *ItemB) = 0;
	};

};	// end namespace Amanith

#endif
