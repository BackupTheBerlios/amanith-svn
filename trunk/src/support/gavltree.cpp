/****************************************************************************
** $file: amanith/src/support/gavltree.h   0.1.1.0   edited Sep 24 08:00
**
** AVL Tree implementation.
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

#include "amanith/support/gavltree.h"
#include <new>
/*!
	\file gavltree.cpp
	\brief Implementation file for AVL (Adelson-Velskii and Landis) tree classes.
*/

namespace Amanith {

// *********************************************************************
//                               GAVLNode
// *********************************************************************

/*!
	\class GAVLNode
	\brief This class implements a parented AVL node.

	Each node is characterized by its two subtrees pointers, a father node pointer, a balance factor and a custom data
	pointer. All these informations can be retrieved using public members.
*/

// constructor
GAVLNode::GAVLNode() {

	gChildren[0] = NULL;
	gChildren[1] = NULL;
	gFather = NULL;
	gCustomData = NULL;
	gBalanceFactor = 0;
}

// destructor
GAVLNode::~GAVLNode() {
}


// *********************************************************************
//                              GAVLTree
// *********************************************************************

/*!
	\class GAVLTree
	\brief This class implements an AVL (Adelson-Velskii and Landis) tree.

	An AVL tree is a balanced binary search tree. Named after their inventors, Adelson-Velskii and Landis, they were
	the first dynamically balanced trees to be proposed. Like red-black trees, they are not perfectly balanced, but
	pairs of sub-trees differ in height by at most 1, maintaining an O(Logn) search time.\n
	Addition and deletion operations also take O(Logn) time.
	In an AVL tree, nodes are ordered according to the Compare() function, and each node has a unique key associated (named
	as 'CustomData'). This means that in an AVL tree there's a 1-1 relation between keys and nodes.
	Nodes keys are passed to the Compare() function, that will compare them, inducing nodes arrangement.
*/

/*!
	It sets root node to NULL and nodes counter to 0.
*/
GAVLTree::GAVLTree() {

	gRoot = NULL;
    gNodesCount = 0;
}

/*!
	Deletes all nodes, but it doesn't free any custom data associated with nodes.
*/
GAVLTree::~GAVLTree() {

	GAVLNode *p, *q;

	for (p = Root(); p != NULL; p = q) {
		if (p->LeftChild() == NULL) {
			q = p->RightChild();
			delete p;
		}
		else {
			q = p->LeftChild();
			p->gChildren[0] = q->RightChild();
			q->gChildren[1] = p;
		}
	}
}

GAVLNode *GAVLTree::TryToInsert(void *Data, GBool& AlreadyExists) {

	GAVLNode *y;     // Top node to update balance factor, and parent
	GAVLNode *p, *q; // Iterator, and parent
	GAVLNode *n;     // Newly inserted node
	GAVLNode *w;     // New root of rebalanced subtree
	GInt32 dir = 0;  // Direction to take

	y = gRoot;
	for (q = NULL, p = gRoot; p != NULL; q = p, p = p->gChildren[dir]) {
		
		GInt32 cmp = Compare(Data, p->gCustomData);
		if (cmp == 0) {
			// signal that item already exists
			AlreadyExists = G_TRUE;
			return p;
		}
		dir = cmp > 0;
		if (p->BalanceFactor() != 0)
			y = p;
	}

	n = new( std::nothrow ) GAVLNode();
	if (n == NULL)
		return NULL;
	// signal that item does not already exist
	AlreadyExists = G_FALSE;

	gNodesCount++;
	n->gFather = q;
	n->gCustomData = Data;
	if (q != NULL)
		q->gChildren[dir] = n;
	else
		gRoot = n;

	if (gRoot == n)
		return n;

	for (p = n; p != y; p = q) {
		q = p->Father();
		dir = q->LeftChild() != p;
		if (dir == 0)
			q->gBalanceFactor--;
		else
			q->gBalanceFactor++;
	}

	if (y->BalanceFactor() == -2) {
		
		GAVLNode *x = y->LeftChild();
		if (x->gBalanceFactor == -1) {
			w = x;
			y->gChildren[0] = x->RightChild();
			x->gChildren[1] = y;
			x->gBalanceFactor = y->gBalanceFactor = 0;
			x->gFather = y->gFather;
			y->gFather = x;
			if (y->gChildren[0] != NULL)
				y->gChildren[0]->gFather = y;
		}
		else {
			G_ASSERT(x->gBalanceFactor == +1);
			w = x->RightChild();
			x->gChildren[1] = w->LeftChild();
			w->gChildren[0] = x;
			y->gChildren[0] = w->RightChild();
			w->gChildren[1] = y;
			if (w->gBalanceFactor == -1) {
				x->gBalanceFactor = 0;
				y->gBalanceFactor = 1;
			}
			else
			if (w->gBalanceFactor == 0)
				x->gBalanceFactor = y->gBalanceFactor = 0;
			else {
				x->gBalanceFactor = -1;
				y->gBalanceFactor = 0;
			}
			w->gBalanceFactor = 0;
			w->gFather = y->Father();
			x->gFather = y->gFather = w;
			if (x->RightChild() != NULL)
				x->gChildren[1]->gFather = x;
			if (y->LeftChild() != NULL)
				y->gChildren[0]->gFather = y;
		}
	}
	else
	if (y->gBalanceFactor == 2) {
		GAVLNode *x = y->RightChild();
		if (x->gBalanceFactor == 1) {
			w = x;
			y->gChildren[1] = x->LeftChild();
			x->gChildren[0] = y;
			x->gBalanceFactor = y->gBalanceFactor = 0;
			x->gFather = y->Father();
			y->gFather = x;
			if (y->gChildren[1] != NULL)
				y->gChildren[1]->gFather = y;
		}
		else {
			G_ASSERT(x->gBalanceFactor == -1);
			w = x->LeftChild();
			x->gChildren[0] = w->RightChild();
			w->gChildren[1] = x;
			y->gChildren[1] = w->LeftChild();
			w->gChildren[0] = y;
			if (w->gBalanceFactor == 1) {
				x->gBalanceFactor = 0;
				y->gBalanceFactor = -1;
			}
			else
			if (w->gBalanceFactor == 0) {
				x->gBalanceFactor = 0;
				y->gBalanceFactor = 0;
			}
			else {
				x->gBalanceFactor = 1;
				y->gBalanceFactor = 0;
			}
			w->gBalanceFactor = 0;
			w->gFather = y->Father();
			x->gFather = y->gFather = w;
			if (x->LeftChild() != NULL)
				x->gChildren[0]->gFather = x;
			if (y->RightChild() != NULL)
				y->gChildren[1]->gFather = y;
		}
	}
	else
		return n;
	if (w->Father() != NULL)
		w->gFather->gChildren[y != w->gFather->LeftChild()] = w;
	else
		gRoot = w;
	return n;
}

/*!
	This function tries to insert a new node in the tree, specifying associated key (custom data). If a node with the same
	key is already present, no nodes will be inserted.

	\param Data the key that will be associated with node to insert.
	\param AlreadyExists an output flag, if G_TRUE it means that a node with the specified key already exists, else
	G_FALSE.
	\return The new created node (or the one already existing).
*/
GAVLNode *GAVLTree::Insert(void *Data, GBool& AlreadyExists) {

	GAVLNode *p = TryToInsert(Data, AlreadyExists);
	return p;
}

/*!
	Delete a node from tree.

	\param Node the node to be removed from tree. Must be non-NULL.
	\return The key (CustomData) of the deleted node.
	\note After this function the memory associated with Node is not valid (because the node has been delete and its
	memory freed).
*/
void *GAVLTree::DeleteNode(GAVLNode *Node) {

	GAVLNode *p; // Traverses tree to find node to delete
	GAVLNode *q; // Parent of p
	GInt32 dir;  // Side of q on which p is linked
	void *item;

	if (!Node) {
		G_DEBUG("GAVLTree::DeleteNode warning: a NULL parameter has been specified");
		return NULL;
	}

	p = Node;

	item = p->CustomData();

	q = p->Father();
	if (q == NULL) {
		q = (GAVLNode *)&gRoot;
		dir = 0;
	}
	else {
		if (q->LeftChild() == p)
			dir = 0;
		else
			dir = 1;
	}

	if (p->RightChild() == NULL) {
		q->gChildren[dir] = p->LeftChild();
		if (q->gChildren[dir] != NULL)
			q->gChildren[dir]->gFather = p->Father();
	}
	else {
		GAVLNode *r = p->RightChild();
		if (r->LeftChild() == NULL) {
			r->gChildren[0] = p->LeftChild();
			q->gChildren[dir] = r;
			r->gFather = p->Father();
			if (r->LeftChild() != NULL)
				r->gChildren[0]->gFather = r;
			r->gBalanceFactor = p->BalanceFactor();
			q = r;
			dir = 1;
		}
		else {
			GAVLNode *s = r->LeftChild();
			while (s->LeftChild() != NULL)
				s = s->LeftChild();
			r = s->Father();
			r->gChildren[0] = s->RightChild();
			s->gChildren[0] = p->LeftChild();
			s->gChildren[1] = p->RightChild();
			q->gChildren[dir] = s;
			if (s->LeftChild() != NULL)
				s->gChildren[0]->gFather = s;
			s->gChildren[1]->gFather = s;
			s->gFather = p->Father();
			if (r->LeftChild() != NULL)
				r->gChildren[0]->gFather = r;
			s->gBalanceFactor = p->BalanceFactor();
			q = r;
			dir = 0;
		}
	}
	delete p;

	while (q != (GAVLNode *)&gRoot)	{
		GAVLNode *y = q;

		if (y->Father() != NULL)
			q = y->Father();
		else
			q = (GAVLNode *)&gRoot;

		if (dir == 0) {
			dir = (q->LeftChild() != y);
			y->gBalanceFactor++;
			if (y->BalanceFactor() == 1)
				break;
			else
			if (y->BalanceFactor() == 2) {
				GAVLNode *x = y->RightChild();
				if (x->BalanceFactor() == -1) {

					GAVLNode *w;

					G_ASSERT(x->BalanceFactor() == -1);
					w = x->LeftChild();
					x->gChildren[0] = w->RightChild();
					w->gChildren[1] = x;
					y->gChildren[1] = w->LeftChild();
					w->gChildren[0] = y;
					if (w->gBalanceFactor == 1) {
						x->gBalanceFactor = 0;
						y->gBalanceFactor = -1;
					}
					else
					if (w->gBalanceFactor == 0)
						x->gBalanceFactor = y->gBalanceFactor = 0;
					else {
						x->gBalanceFactor = 1;
						y->gBalanceFactor = 0;
					}
					w->gBalanceFactor = 0;
					w->gFather = y->Father();
					x->gFather = y->gFather = w;
					if (x->LeftChild() != NULL)
						x->gChildren[0]->gFather = x;
					if (y->RightChild() != NULL)
						y->gChildren[1]->gFather = y;
					q->gChildren[dir] = w;
				}
				else {
					y->gChildren[1] = x->LeftChild();
					x->gChildren[0] = y;
					x->gFather = y->Father();
					y->gFather = x;
					if (y->RightChild() != NULL)
						y->gChildren[1]->gFather = y;
					q->gChildren[dir] = x;
					if (x->BalanceFactor() == 0)	{
						x->gBalanceFactor = -1;
						y->gBalanceFactor = 1;
						break;
					}
					else {
						x->gBalanceFactor = y->gBalanceFactor = 0;
						y = x;
					}
				}
			}
		}
		else {
			dir = (q->LeftChild() != y);
			y->gBalanceFactor--;
			if (y->BalanceFactor() == -1)
				break;
			else
			if (y->BalanceFactor() == -2) {
				GAVLNode *x = y->LeftChild();
				if (x->BalanceFactor() == 1) {

					GAVLNode *w;
					G_ASSERT(x->BalanceFactor() == 1);
					w = x->RightChild();
					x->gChildren[1] = w->LeftChild();
					w->gChildren[0] = x;
					y->gChildren[0] = w->RightChild();
					w->gChildren[1] = y;
					if (w->BalanceFactor() == -1) {
						x->gBalanceFactor = 0;
						y->gBalanceFactor = +1;
					}
					else
					if (w->gBalanceFactor == 0)
						x->gBalanceFactor = y->gBalanceFactor = 0;
					else {
						x->gBalanceFactor = -1;
						y->gBalanceFactor = 0;
					}
					w->gBalanceFactor = 0;
					w->gFather = y->Father();
					x->gFather = y->gFather = w;
					if (x->RightChild() != NULL)
						x->gChildren[1]->gFather = x;
					if (y->LeftChild() != NULL)
						y->gChildren[0]->gFather = y;
					q->gChildren[dir] = w;
				}
				else {
					y->gChildren[0] = x->RightChild();
					x->gChildren[1] = y;
					x->gFather = y->Father();
					y->gFather = x;
					if (y->LeftChild() != NULL)
						y->gChildren[0]->gFather = y;
					q->gChildren[dir] = x;
					if (x->BalanceFactor() == 0)	{
						x->gBalanceFactor = 1;
						y->gBalanceFactor = -1;
						break;
					}
					else {
						x->gBalanceFactor = y->gBalanceFactor = 0;
						y = x;
					}
				}
			}
		}
	}
	gNodesCount--;
	return item;
}

/*!
	Delete a node from tree, specifying a search key.

	\param Data the search key, used to find the node to delete.
	\return The key (CustomData) of the deleted node. This is the same value of data parameter.
	\note After this function the memory associated with node is not valid (because the node has been delete and its
	memory freed).
*/
void *GAVLTree::Delete(void *Data) {

	GAVLNode *p;	// Traverses tree to find node to delete
	GInt32 dir;		// Side of |q| on which |p| is linked

	if (Root() == NULL)
		return NULL;

	p = gRoot;
	for (;;) {
		GInt32 cmp = Compare(Data, p->gCustomData);
		if (cmp == 0)
			break;

		dir = cmp > 0;
		p = p->gChildren[dir];
		if (p == NULL)
			return NULL;
	}
	return DeleteNode(p);
}

/*!
	\return A pointer to the node corresponding to the specified key. If such node does not exist a NULL
	pointer is returned.
*/
GAVLNode *GAVLTree::Find(void *Data) {

	GAVLNode *p;

	for (p = Root(); p != NULL;) {
		GInt32 cmp = Compare(Data, p->CustomData());
		if (cmp < 0)
			p = p->LeftChild();
		else
		if (cmp > 0)
			p = p->RightChild();
		else
			return p;
	}
	return NULL;
}

/*!
	Previous means the node whose key is the maximum key lesser than the specified one. If such node does not exist a NULL
	pointer is returned.
	\note The order is induced by the Compare() function.
*/
GAVLNode *GAVLTree::FindPrev(void *Data) {

	GAVLNode *p, *candidate;

	if (!Root())
		return NULL;

	GInt32 cmp, cmpTmp, cmpNew = 0;

	// find a first lesser node
	p = Root();
	cmp = Compare(Data, p->CustomData());
	while ((cmp <= 0) && (p != NULL)) {
		p = p->LeftChild();
		if (p)
			cmp = Compare(Data, p->CustomData());
	}
	// if we haven't found a first good (lesser than item) candidate, it means that a "next" do not exist
	if (!p)
		return NULL;

	candidate = p;
	while (p != NULL) {
		// left direction
		if (cmp < 0) {
			p = p->LeftChild();
			if (p) {
				cmpNew = Compare(Data, p->CustomData());
				if (cmpNew > 0) {
					cmpTmp = Compare(p->CustomData(), candidate->CustomData());
					if (cmpTmp > 0)
						candidate = p;
				}
			}
		}
		else
		// right direction
		if (cmp > 0) {
			p = p->RightChild();
			if (p) {
				cmpNew = Compare(Data, p->CustomData());
				if (cmpNew > 0)
					candidate = p;
			}
		}
		// we have found the item
		else
			return Prev(p);
		// next step
		cmp = cmpNew;
	}
	return candidate;
}

/*!
	Next means the node whose key is the minimum key greater than the specified one. If such node does not exist a NULL
	pointer is returned.
	\note The order is induced by the Compare() function.
*/
GAVLNode *GAVLTree::FindNext(void *Data) {

	GAVLNode *p, *candidate;

	if (!Root())
		return NULL;

	GInt32 cmp, cmpTmp, cmpNew = 0;

	// find a first greater node
	p = Root();
	cmp = Compare(Data, p->CustomData());
	while ((cmp >= 0) && (p != NULL)) {
		p = p->RightChild();
		if (p)
			cmp = Compare(Data, p->CustomData());
	}
	// if we haven't found a first good (grater than item) candidate, it means that a "next" do not exist
	if (!p)
		return NULL;

	candidate = p;
	while (p != NULL) {
		// right direction
		if (cmp > 0) {
			p = p->RightChild();
			if (p) {
				cmpNew = Compare(Data, p->CustomData());
				if (cmpNew < 0) {
					cmpTmp = Compare(p->CustomData(), candidate->CustomData());
					if (cmpTmp < 0)
						candidate = p;
				}
			}
		}
		else
		// left direction
		if (cmp < 0) {
			p = p->LeftChild();
			if (p) {
				cmpNew = Compare(Data, p->CustomData());
				if (cmpNew < 0)
					candidate = p;
			}
		}
		// we have found the item
		else
			return Next(p);
		// next step
		cmp = cmpNew;
	}
	return candidate;
}

/*!
	\note If the tree is empty, a NULL pointer will be returned.
*/
GAVLNode* GAVLTree::Min() {
	
	GAVLNode* node = Root();

	if (node) {
		while (node->LeftChild())
			node = node->LeftChild();
		return node;
	}
	return NULL;
}

/*!
	\note If the tree is empty, a NULL pointer will be returned.
*/
GAVLNode* GAVLTree::Max() {
			
	GAVLNode* node = Root();

	if (node) {
		while (node->RightChild())
			node = node->RightChild();
		return node;
	}
	return NULL;
}

/*!
	Next means the node whose key is the minimum key greater than the one associated to Node. If such
	'next' node does not exist a NULL pointer is returned.
	\note The order is induced by the Compare() function.
*/
GAVLNode* GAVLTree::Next(GAVLNode* Node) {

	GAVLNode* tmpNode = Node;

	// make iteration circular
	if (!tmpNode)
		return Min();
	else
	if (!tmpNode->RightChild()) {
		// current node and its father
		GAVLNode *fatherNode, *curNode = Node;
		for (fatherNode = curNode->Father(); ; curNode = fatherNode, fatherNode = fatherNode->Father()) {
			if (fatherNode == NULL || curNode == fatherNode->LeftChild())
				return fatherNode;
		}
	}
	else {
		tmpNode = tmpNode->RightChild();
		// visit tmpNode's left subtree (here, every element is lesser than tmpNode and
		// greater than Node)
		while (tmpNode->LeftChild())
			tmpNode = tmpNode->LeftChild();
		return tmpNode;
	}
}

/*!
	Previous means the node whose key is the maximum key lesser than the one associated to Node. If such
	'previous' node does not exist a NULL pointer is returned.
	\note The order is induced by the Compare() function.
*/
GAVLNode* GAVLTree::Prev(GAVLNode* Node) {

	GAVLNode* tmpNode = Node;

	// make iteration circular
	if (!tmpNode)
		return Max();
	else
	if (!tmpNode->LeftChild()) {
		// current node and its father
		GAVLNode *fatherNode, *curNode = Node; 
		for (fatherNode = curNode->Father(); ;curNode = fatherNode, fatherNode = fatherNode->Father()) {
			if (fatherNode == NULL || curNode == fatherNode->RightChild())
				return fatherNode;
		}
	}
	else {
		tmpNode = tmpNode->LeftChild();
		// visit tmpNode's right subtree (here, every element is greater than tmpNode and
		// lesser than Node)
		while (tmpNode->RightChild())
			tmpNode = tmpNode->RightChild();
		return tmpNode;
	}
}

};	// end namespace Amanith
