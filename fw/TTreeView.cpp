// ========================================================================================
//	TTreeView.cpp			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TApplication.h"
#include "TTreeView.h"
#include "TTreeNode.h"
#include "TDrawContext.h"
#include "TPixmap.h"

#include "Pixmaps/TriangleOpened.xpm"
#include "Pixmaps/TriangleClosed.xpm"
#include "Pixmaps/TriangleTurning.xpm"

#include <X11/keysym.h>

static TPixmap* sTriangleOpenedIcon = NULL;
static TPixmap* sTriangleClosedIcon = NULL;
static TPixmap* sTriangleTurningIcon = NULL;

const TCoord kChildInset = 20;


TTreeView::TTreeView(TWindow* parent, const TRect& bounds, int columnCount, CompareFunc compareFunc, void* compareData)
	:	TGridView(parent, bounds),
		fColumnWidths(NULL),
		fColumnCount(columnCount),
		fCompareFunc(compareFunc),
		fCompareData(compareData),
		fTrackingRow(-1),
		fTriangleTurning(false)
{
	if (!sTriangleOpenedIcon)
		sTriangleOpenedIcon = new TPixmap(TriangleOpened_xpm);
	if (!sTriangleClosedIcon)
		sTriangleClosedIcon = new TPixmap(TriangleClosed_xpm);
	if (!sTriangleTurningIcon)
		sTriangleTurningIcon = new TPixmap(TriangleTurning_xpm);
		
	fColumnWidths = new TCoord[fColumnCount + 1];
	ASSERT(fColumnWidths);

	for (int i = 0; i <= fColumnCount; i++)
		fColumnWidths[i] = 0;
}


TTreeView::~TTreeView()
{
	delete[] fColumnWidths;
}


void TTreeView::Create()
{
	// skip TGridView::Create to avoid call to ComputeContentSize too early
	TView::Create();

	TDrawContext	context(this);
	TCoord ascent, height;
	context.MeasureText("W", 1, ascent, height);
		
	fRowHeight = height;

	fColumnWidths[0] = 12;
	for (int i = 1; i <= fColumnCount; i++)
		fColumnWidths[i] = (GetWidth() - 12) / fColumnCount;
		
	ComputeContentSize();
}


int TTreeView::GetRowCount() const
{
	return fRowList.GetSize();
}


int TTreeView::GetColumnCount() const
{
	return fColumnCount + 1;
}


TCoord TTreeView::GetRowHeight(int row) const
{
	return fRowHeight;	
}


TCoord TTreeView::GetColumnWidth(int column) const
{
	ASSERT(column >= 0 && column < GetColumnCount());
	
	return fColumnWidths[column];
}


bool TTreeView::IsColumnResizable(int column) const
{
	ASSERT(column >= 0 && column < GetColumnCount());
	return (column > 0);	// first column not resizable
}


void TTreeView::SetColumnWidth(int column, TCoord width)
{
	ASSERT(width >= 0);
	fColumnWidths[column] = width;
	
	ComputeContentSize();
	Redraw();
}


void TTreeView::DeleteAllNodes()
{
	TListIterator<TTreeNode>	iter(fRowList);
	TTreeNode* node;

	while ((node = iter.Next()) != NULL)
	{
		if (node->GetDepth() == 0)
			delete node;
	}
	
	fRowList.RemoveAll();
}


void TTreeView::AddNode(TTreeNode* node, TTreeNode* parentNode)
{
	if (parentNode)
	{
		if (fCompareFunc)
		{
			TTreeNode* child = parentNode->GetChildren();
			TTreeNode* last = NULL;
			
			while (child)
			{
				if (fCompareFunc(node, child, fCompareData) < 0)
					break;
				
				last = child;
				child = child->GetNextSibling();
			}
			
			if (last)
				last->InsertSiblingAfter(node);
			else
				parentNode->AddChildFirst(node);
		}
		else
			parentNode->AddChild(node);
	}
	else
	{
		if (fCompareFunc)
		{
			int i;
			
			for (i = 0; i < fRowList.GetSize(); i++)
			{
				TTreeNode* testNode = fRowList[i];
		
				if (testNode->GetDepth() == 0 && fCompareFunc(node, testNode, fCompareData) < 0)
					break;
			}
			
			fRowList.InsertAt(node, i);
		}
		else
			fRowList.InsertLast(node);
	}
}


void TTreeView::DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context)
{
	ASSERT(row >= 0 && row < fRowList.GetSize());

	TRect bounds(cellBounds);
	
	// so expanding works correctly
	context.EraseRect(bounds);
	
	if (column >= 2)
	{
		// draw border on left
		context.DrawLine(bounds.left, bounds.top, bounds.left, bounds.bottom - 1);
		bounds.left += 2;
	}

	TTreeNode* node = fRowList[row];
	ASSERT(node);
	
	if (column == 0)
	{
		if (node->IsExpandable())
		{
			TPixmap* pixmap;

			if (fTriangleTurning && fTrackingRow == row)
				pixmap = sTriangleTurningIcon;
			else if (node->IsExpanded())
				pixmap = sTriangleOpenedIcon;
			else
				pixmap = sTriangleClosedIcon;
			
			context.DrawPixmap(pixmap, TPoint(bounds.left + 2,  bounds.top + 2));
		}
	}
	else
	{
		bool selected = CellSelected(row, column);
		TColor	savedColor(context.GetBackColor());
	
		if (selected)
			context.SetBackColor(gApplication->GetHiliteColor());

		TString text;
		node->GetText(column, text);

		if (column == 1)
			bounds.left += (node->GetDepth() * kChildInset);
			
		bounds.right = bounds.left + context.MeasureText(text, text.GetLength());
		context.DrawTextBox(text, text.GetLength(), bounds, kTextAlignLeft);	

		if (selected)
			context.SetBackColor(savedColor);
	}
}


void TTreeView::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	int row, column;
	
	if (button == kLeftButton && fTrackingRow == -1 && FindCell(point, row, column) && column == 0)
	{
		TTreeNode* node = fRowList[row];
		ASSERT(node);
		
		if (node->IsExpandable())
		{
			fTrackingRow = row;
			fTriangleTurning = true;

			TDrawContext context(this);
			DrawCellRange(context, TRect(0, row, 1, row + 1));
		}
	}
	else
		TGridView::DoMouseDown(point, button, state);
}


void TTreeView::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (fTrackingRow != -1)
	{
		bool redraw = false;
		
		if (fTriangleTurning)
		{
			ExpandCollapseRow(fTrackingRow);
			redraw = true;
		}
	
		int row = fTrackingRow;
		fTrackingRow = -1;
		fTriangleTurning = false;

		if (redraw)
		{
			TDrawContext context(this);
			DrawCellRange(context, TRect(0, row, GetColumnCount(), GetRowCount()));
		}
	}
	else
		TGridView::DoMouseUp(point, button, state);
}


void TTreeView::DoMouseMoved(const TPoint& point, TModifierState state)
{
	if (fTrackingRow != -1)
	{
		TRect bounds;
		GetCellBounds(fTrackingRow, 0, bounds);

		if (fTriangleTurning != bounds.Contains(point))
		{
			fTriangleTurning = !fTriangleTurning;

			TDrawContext context(this);
			DrawCellRange(context, TRect(0, fTrackingRow, 1, fTrackingRow + 1));
		}
	}
	else
		TGridView::DoMouseMoved(point, state);
}


bool TTreeView::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	int row, column;
	TTreeNode* node;

	if (GetFirstSelectedCell(row, column))			
	{
		switch (key)
		{
			case XK_Return:
			case XK_KP_Enter:
				CellDoubleClicked(row, column);
				return true;

			case XK_Left:
				node = fRowList[row];
				if (node && node->IsExpandable() && node->IsExpanded())
					CellDoubleClicked(row, column);
				return true;
	
			case XK_Right:
				node = fRowList[row];
				if (node && node->IsExpandable() && !node->IsExpanded())
					CellDoubleClicked(row, column);
				return true;
	
			default:
				break;
		}
	}

	return TGridView::DoKeyDown(key, state, string);
}


void TTreeView::ExpandCollapseRow(int row)
{
	ASSERT(row >= 0 && row < fRowList.GetSize());
	
	TTreeNode* node = fRowList[row];
	int startRow = row;

	// hide all children and descendants
	if (node->IsExpanded())
	{		
		int deletedRows = 0;

		int r, c;	
		if (GetFirstSelectedCell(r, c) && r > startRow)
			UnselectAll();

		row++;	
		
		int depth = node->GetDepth();
		
		while (row < fRowList.GetSize())
		{
			TTreeNode* node = fRowList[row];
			if (node->GetDepth() <= depth)
				break;
				
			fRowList.RemoveAt(row);
			deletedRows++;
		}

		node->Expand(false);
		
		if (r > startRow + deletedRows)
			SelectCell(r - deletedRows, c);
	}
	else
	{		
		node->Expand(true);
		ExpandRow(row);

		int r, c;
		if (GetFirstSelectedCell(r, c) && r > startRow)
		{
			UnselectAll();
			SelectCell(r + row - startRow, c);
		}
	}

	ComputeContentSize();
}


void TTreeView::ExpandRow(int& row)
{
	TTreeNode* node = fRowList[row];	
	int childCount = node->GetChildCount();

	for (int i = 0; i < childCount; i++)
	{
		fRowList.InsertAt(node->GetChild(i), ++row);

		if (fRowList[row]->IsExpanded())
			ExpandRow(row);
	}
}


bool TTreeView::TTypeSelectable_IsMultiSelect()
{
	return false;
}


void TTreeView::TTypeSelectable_SelectAll()
{
	SelectAll();
}


int TTreeView::TTypeSelectable_GetCount()
{
	return GetRowCount();
}


bool TTreeView::TTypeSelectable_AllowSelect(int i)
{
	return AllowCellSelect(i, 1, false);
}


void TTreeView::TTypeSelectable_GetText(int i, TString& text)
{
	TTreeNode* node = fRowList[i];
	node->GetText(1, text);
}


int TTreeView::TTypeSelectable_GetSelection()
{
	for (int i = 0; i < GetRowCount(); i++)
		if (CellSelected(i, 0))
			return i;

	return -1;
}


void TTreeView::TTypeSelectable_Select(int i)
{
	UnselectAll();
	SelectCell(i, 1);
	SetLastClick(i, 1);
	ScrollSelectionIntoView();
	HandleCommand(this, this, kSelectionChangedCommandID);	// should be automatic
}

