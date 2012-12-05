// ========================================================================================
//	TDirectoryDiffListView.cpp		 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#include "IDECommon.h"

#include "TDirectoryDiffListView.h"
#include "TFileDiffDocument.h"
#include "TDirectoryDiffNode.h"
#include "fw/TApplication.h"
#include "fw/TDrawContext.h"
#include "fw/TFont.h"
#include "fw/TTypeSelectBehavior.h"
#include "fw/intl.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static TFont* sFont = NULL;


TDirectoryDiffListView::TDirectoryDiffListView(TWindow* parent, const TRect& bounds, 
														const TDirectory& directory1, 
														const TDirectory& directory2,
														TList<TString>& diffList)
	:	TTreeView(parent, bounds, 1, (TTreeView::CompareFunc)CompareNodes, NULL),
		fRowHeight(0),
		fDirectory1(directory1),
		fDirectory2(directory2),
		fDiffList(diffList)
{
	AddBehavior(new TTypeSelectBehavior(false));
}


TDirectoryDiffListView::~TDirectoryDiffListView()
{
}


void TDirectoryDiffListView::InitDrawContext(TDrawContext& context)
{
	TTreeView::InitDrawContext(context);

	if (!sFont)
		sFont = new TFont("-*-*-medium-r-normal-*-13-*-*-*-*-*-*-*");

	context.SetFont(sFont);
}


TDirectoryDiffNode* TDirectoryDiffListView::GetNode(int row)
{
	return dynamic_cast<TDirectoryDiffNode*>(fRowList[row]);
}



bool TDirectoryDiffListView::AllowCellSelect(int row, int column, bool multiple)
{
	// if no differences, we have one non-selectable row
	return (column == 1 && fDiffList.GetSize() > 0);
}


void TDirectoryDiffListView::CellDoubleClicked(int row, int column)
{
	if (column == 1)
	{
		TDirectoryDiffNode* node = GetNode(row);
		ASSERT(node);
		
		if (node->IsDirectory())
		{
			ExpandCollapseRow(row);
	
			TDrawContext context(this);
			DrawCellRange(context, TRect(0, row, GetColumnCount(), GetRowCount()));
		}
		else
		{
			const char* path = *fDiffList[node->GetDiffIndex()];
			TFile	file1(fDirectory1, path);
			TFile	file2(fDirectory2, path);
		
			TFileDiffDocument* document = TFileDiffDocument::CreateDocument(file1, file2);
	
			if (document)
				gApplication->OpenWindowContext(document);
		}
	}
}


TDirectoryDiffNode* TDirectoryDiffListView::FindParentNode(TDirectoryDiffNode* node, const TChar* path, int pathLength)
{
	const TChar* testPath = node->GetPath();
	int testPathLength = strlen(testPath);
	
	if (strncmp(testPath, path, testPathLength) == 0)
	{
		if (testPathLength < pathLength)
		{
			// search children for a closer match
			TTreeNode* childNode = node->GetChildren();
			
			while (childNode)
			{
				TDirectoryDiffNode* childDiffNode = dynamic_cast<TDirectoryDiffNode*>(childNode);
				TDirectoryDiffNode* betterMatch = FindParentNode(childDiffNode, path, pathLength);
				
				if (betterMatch)
					return betterMatch;
				else
					childNode = childNode->GetNextSibling();
			}
			
		}
		
		return node;
	}

	return NULL;
}


TDirectoryDiffNode* TDirectoryDiffListView::GetDirectoryNode(const TChar* path)
{
	TListIterator<TTreeNode> iter(fRowList);
	TTreeNode* treeNode;
	TDirectoryDiffNode* parentNode = NULL;

	// compute directory path length
	const char* str = path;
	const char* lastSlash = NULL;

	while (str[0])
	{
		if (*str++ == '/')
			lastSlash = str;
	}

	int pathLength = (lastSlash ? lastSlash - path : 0);
	
	if (pathLength == 0)
		return NULL;		// file needs to be added as a root node

	// search root nodes recursively for a parent node
	while ((treeNode = iter.Next()) != NULL)
	{
		TDirectoryDiffNode* testNode = dynamic_cast<TDirectoryDiffNode*>(treeNode);
		ASSERT(testNode);
		
		if (testNode->IsDirectory())
		{
			parentNode = FindParentNode(testNode, path, pathLength);
			if (parentNode)
				break;
		}
	}

	// if we get here, we need to create a new directory node.	
	// we may also need to build one or more parent nodes if they don't already exist.
	int builtPathLength = (parentNode ? parentNode->GetPath().GetLength() : 0); // length of built parent nodes
	const TChar* currentPosition = path + builtPathLength;
	
	while (builtPathLength < pathLength)
	{
		const TChar* str = currentPosition;
		while (str[0])
		{
			if (*str++ == '/')
				break;
		}
		
		ASSERT(str[0]);		// should not get to the end here
				
		currentPosition = str;
		builtPathLength = currentPosition - path;

		TString	directoryPath(path, builtPathLength);
		int depth = (parentNode ? parentNode->GetDepth() + 1 : 0);
		TDirectoryDiffNode* node = new TDirectoryDiffNode(directoryPath, true, -1, depth);
		
		AddNode(node, parentNode);
			
		parentNode = node;
	}

	return parentNode;
}


void TDirectoryDiffListView::GenerateDiffTree()
{
	DeleteAllNodes();
	
	TListIterator<TString> iter(fDiffList);
	TString* path;
	
	for (int i = 0; (path = iter.Next()) != NULL; i++)
	{
		TDirectoryDiffNode* parentNode = GetDirectoryNode(*path);
		int depth = (parentNode ? parentNode->GetDepth() + 1 : 0);
		TDirectoryDiffNode* node = new TDirectoryDiffNode(*path, false, i, depth);
		
		AddNode(node, parentNode);
	}
}


void TDirectoryDiffListView::NotifyBoundsChanged(const TRect& oldBounds)
{
	TTreeView::NotifyBoundsChanged(oldBounds);

	// resize rightmost column
	if (GetRowCount() > 0)
	{
		TRect cellBounds;
		GetCellBounds(0, 0, cellBounds);
		
		if (fBounds.right > cellBounds.right)
			SetColumnWidth(1, fBounds.right - cellBounds.left);
	}
}


bool TDirectoryDiffListView::LeftArrowKey(TModifierState state)
{
	if (fLastCellClick.h < 0 && fLastCellClick.v < 0)
	{
		fLastCellClick.Set(1, 0);
		SelectCell(fLastCellClick.v, fLastCellClick.h);
		return true;
	}

	return false;
}


bool TDirectoryDiffListView::RightArrowKey(TModifierState state)
{
	if (fLastCellClick.h < 0 && fLastCellClick.v < 0)
	{
		fLastCellClick.Set(1, 0);
		SelectCell(fLastCellClick.v, fLastCellClick.h);
		return true;
	}

	return false;
}


bool TDirectoryDiffListView::UpArrowKey(TModifierState state)
{
	TPoint	cell(fLastCellClick);

	if (cell.h < 0 && cell.v < 0)
		cell.Set(1, 0);
		
	if (GetRowCount() > 0)
	{
		if (cell.v > 0)
			--cell.v;
		else
			cell.v = GetRowCount() - 1;
			
		bool multiple = (fMultiSelect && (state & ShiftMask));

		if (AllowCellSelect(cell.v, cell.h, multiple))
		{
			if (!multiple)
				UnselectAll();
	
			SelectCell(cell.v, cell.h);
			SetLastClick(cell.v, cell.h);
			return true;
		}
	}

	return false;
}


bool TDirectoryDiffListView::DownArrowKey(TModifierState state)
{
	TPoint	cell(fLastCellClick);

	if (cell.h < 0 && cell.v < 0)
		cell.Set(1, -1);

	if (GetRowCount() > 0)
	{
		if (cell.v < GetRowCount() - 1)
			++cell.v;
		else
			cell.v = 0;

		bool multiple = (fMultiSelect && (state & ShiftMask));

		if (AllowCellSelect(cell.v, cell.h, multiple))
		{
			if (!multiple)
				UnselectAll();
	
			SelectCell(cell.v, cell.h);
			SetLastClick(cell.v, cell.h);
			return true;
		}
	}

	return false;
}


int TDirectoryDiffListView::CompareNodes(const TDirectoryDiffNode* item1, const TDirectoryDiffNode* item2, void* compareData)
{
	TString	string1, string2;

	item1->GetText(1, string1);
	item2->GetText(1, string2);
	return strcasecmp(string1, string2);
}
