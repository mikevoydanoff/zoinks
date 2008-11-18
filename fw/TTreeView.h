// ========================================================================================
//	TTreeView.h				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TTreeView__
#define __TTreeView__

#include "TGridView.h"
#include "TTypeSelectable.h"
#include "TList.h"

class TTreeNode;

class TTreeView : public TGridView, public TTypeSelectable
{
public:
	typedef int (* CompareFunc)(const TTreeNode* item1, const TTreeNode* item2, void* compareData);

							TTreeView(TWindow* parent, const TRect& bounds, int columnCount, CompareFunc compareFunc, void* compareData);

	virtual void			Create();
	virtual int				GetRowCount() const;
	virtual int				GetColumnCount() const;
	virtual TCoord			GetRowHeight(int row) const;
	virtual TCoord			GetColumnWidth(int column) const;

	virtual bool			IsColumnResizable(int column) const;
	virtual void			SetColumnWidth(int column, TCoord width);

	void					DeleteAllNodes();

	void					AddNode(TTreeNode* node, TTreeNode* parentNode);

protected:				
	virtual					~TTreeView();

	virtual void			DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context);

	virtual void			DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void			DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void			DoMouseMoved(const TPoint& point, TModifierState state);
	virtual bool			DoKeyDown(KeySym key, TModifierState state, const char* string);

	void					ExpandCollapseRow(int row);
	void					ExpandRow(int& row);
	
	// TTypeSelectable interface
	virtual bool			TTypeSelectable_IsMultiSelect();
	virtual void			TTypeSelectable_SelectAll();
	virtual int				TTypeSelectable_GetCount();
	virtual bool			TTypeSelectable_AllowSelect(int i);
	virtual void			TTypeSelectable_GetText(int i, TString& text);
	virtual int				TTypeSelectable_GetSelection();
	virtual void 			TTypeSelectable_Select(int i);

protected:
	TList<TTreeNode>		fRowList;
	TCoord					fRowHeight;
	TCoord*					fColumnWidths;
	int						fColumnCount;
	CompareFunc 			fCompareFunc;
	void*					fCompareData;
	int						fTrackingRow;		// row we are tracking a click on, or -1
	bool					fTriangleTurning;	// true if we should draw fTrackingRow as partially turned triangle
};

#endif // __TTreeView__
