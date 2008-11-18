// ========================================================================================
//	TDirectoryDiffListView.h		 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#ifndef __TDirectoryDiffListView__
#define __TDirectoryDiffListView__

#include "fw/TDirectory.h"
#include "fw/TList.h"
#include "fw/TTreeView.h"

struct TDiffRec;
class TDirectoryDiffNode;

class TDirectoryDiffListView : public TTreeView
{	
public:
								TDirectoryDiffListView(TWindow* parent, const TRect& bounds, 
														const TDirectory& directory1, 
														const TDirectory& directory2,
														TList<TString>& diffList);

	void						DiffListChanged();
	void						GenerateDiffTree();

protected:				
	virtual						~TDirectoryDiffListView();

	virtual void				InitDrawContext(TDrawContext& context);

	TDirectoryDiffNode* 		GetNode(int row);

	virtual bool				AllowCellSelect(int row, int column, bool multiple);

	virtual void				CellDoubleClicked(int row, int column);
	virtual void				NotifyBoundsChanged(const TRect& oldBounds);
	
	virtual bool				LeftArrowKey(TModifierState state);
	virtual bool				RightArrowKey(TModifierState state);
	virtual bool				UpArrowKey(TModifierState state);
	virtual bool				DownArrowKey(TModifierState state);

	TDirectoryDiffNode* 		FindParentNode(TDirectoryDiffNode* node, const TChar* path, int pathLength);
	TDirectoryDiffNode*			GetDirectoryNode(const TChar* path);

	static int					CompareNodes(const TDirectoryDiffNode* item1, const TDirectoryDiffNode* item2, void* compareData);

protected:
	TCoord						fRowHeight;
	
	const TDirectory& 			fDirectory1; 
	const TDirectory& 			fDirectory2; 
	TList<TString>& 			fDiffList;
};

#endif // __TDirectoryDiffListView__
