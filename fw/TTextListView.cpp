// ========================================================================================
//	TTextListView.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TTextListView.h"
#include "TDrawContext.h"
#include "TApplication.h"
#include "TFont.h"

#include "intl.h"

#include <string.h>
#include <stdio.h>

static TFont* sFont = NULL;

TTextListView::TTextListView(TWindow* parent, const TRect& bounds)
	:	TListView(parent, bounds),
		fRowHeight(0)
{
	fColumnWidth = 10000;
}


TTextListView::~TTextListView()
{
}


void TTextListView::InitDrawContext(TDrawContext& context)
{
	TListView::InitDrawContext(context);

	if (!sFont)
		sFont = new TFont(_("-*-*-medium-r-normal-*-13-*-*-*-*-*-*-*"));

	context.SetFont(sFont);
}


void TTextListView::Create()
{
	TListView::Create();

	TDrawContext	context(this);
	TCoord ascent, height;
	context.MeasureText("W", 1, ascent, height);
		
	fRowHeight = height;

	ComputeContentSize();
}


TCoord TTextListView::GetRowHeight(int row) const
{
	return fRowHeight;
}


void TTextListView::DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context)
{
	bool selected = CellSelected(row, 0);
	TColor	savedForeColor(context.GetForeColor());
	TColor	savedBackColor(context.GetBackColor());

	if (selected)
	{
		if (context.GetDepth() < 8)
		{
			context.SetForeColor(fBackColor);
			context.SetBackColor(fForeColor);		
		}
		else
			context.SetBackColor(gApplication->GetHiliteColor());
	}

	TString text;
	GetText(row, text);

	context.DrawTextBox(text, text.GetLength(), cellBounds, kTextAlignLeft);

	if (selected)
	{
		context.SetForeColor(savedForeColor);
		context.SetBackColor(savedBackColor);
	}
}


bool TTextListView::TTypeSelectable_IsMultiSelect()
{
	return IsMultiSelect();
}


void TTextListView::TTypeSelectable_SelectAll()
{
	SelectAll();
}


int TTextListView::TTypeSelectable_GetCount()
{
	return GetRowCount();
}


bool TTextListView::TTypeSelectable_AllowSelect(int i)
{
	return AllowCellSelect(i, 0, false);
}


void TTextListView::TTypeSelectable_GetText(int i, TString& text)
{
	GetText(i, text);
}


int TTextListView::TTypeSelectable_GetSelection()
{
	return GetFirstSelectedRow();
}


void TTextListView::TTypeSelectable_Select(int i)
{
	UnselectAll();
	SelectCell(i, 0);
	SetLastClick(i, 0);
	ScrollSelectionIntoView();
	HandleCommand(this, this, kSelectionChangedCommandID);	// should be automatic
}
