// ========================================================================================
//	TTreeNode.h				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TTreeNode__
#define __TTreeNode__

class TString;

class TTreeNode
{
public:
							TTreeNode(TTreeNode* parent, int depth, bool expandable);
	virtual					~TTreeNode();

	void					Expand(bool expand);

	void					AddChild(TTreeNode* child);
	void					AddChildFirst(TTreeNode* child);
	void					InsertSiblingAfter(TTreeNode* node);

	TTreeNode*				GetChild(int index) const;
	int						GetChildCount() const;

	virtual void			GetText(int column, TString& outText) const = 0;
	virtual void			CreateChildren() = 0;	// called when expanded

	inline bool				IsExpandable() const { return fExpandable; }
	inline bool				IsExpanded() const { return fExpanded; }
	inline int				GetDepth() const { return fDepth; }
	
	inline TTreeNode*		GetChildren() const { return fChildren; }
	inline TTreeNode*		GetNextSibling() const { return fNextSibling; }


protected:
	TTreeNode*				fParent;
	TTreeNode*				fChildren;
	TTreeNode*				fNextSibling;
	int						fDepth;

	bool					fExpandable;
	bool					fExpanded;
};

#endif // __TTreeNode__
