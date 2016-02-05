// ========================================================================================
//	TTreeNode.cpp			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
// ========================================================================================
/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FWCommon.h"

#include "TTreeNode.h"


TTreeNode::TTreeNode(TTreeNode* parent, int depth, bool expandable)
	:	fParent(parent),
		fChildren(NULL),
		fNextSibling(NULL),
		fDepth(depth),
		fExpandable(expandable),
		fExpanded(false)
{
}


TTreeNode::~TTreeNode()
{
	TTreeNode* child = fChildren;

	while (child)
	{
		TTreeNode* next = child->fNextSibling;
		delete child;
		child = next;
	}
}


void TTreeNode::Expand(bool expand)
{
	if (expand != fExpanded)
	{
		if (expand)
		{
			ASSERT(fExpandable);
			CreateChildren();
		}

		fExpanded = expand;
	}
}


void TTreeNode::AddChild(TTreeNode* child)
{
	if (!fChildren)
		fChildren = child;
	else
	{
		TTreeNode* last = fChildren;

		while (last->fNextSibling)
			last = last->fNextSibling;

		last->fNextSibling = child;
	}
}


void TTreeNode::AddChildFirst(TTreeNode* child)
{
	child->fNextSibling = fChildren;
	fChildren = child;

}


void TTreeNode::InsertSiblingAfter(TTreeNode* node)
{
	node->fNextSibling = fNextSibling;
	fNextSibling = node;

}


TTreeNode* TTreeNode::GetChild(int index) const
{
	ASSERT(index >= 0);
	TTreeNode* child = fChildren;

	for (int i = 0; i < index; i++)
	{
		if (child)
			child = child->fNextSibling;
		else
		{
			ASSERT(0);
			return NULL;
		}
	}

	ASSERT(child);
	return child;
}


int TTreeNode::GetChildCount() const
{
	int result = 0;
	TTreeNode* child = fChildren;

	while (child)
	{
		result++;
		child = child->fNextSibling;
	}

	return result;
}
