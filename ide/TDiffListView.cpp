// ========================================================================================
//	TDiffListView.cpp		 		 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#include "TDiffListView.h"
#include "TFileDiffDocument.h"
#include "fw/TDrawContext.h"
#include "fw/TFont.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/keysym.h>

#include "fw/intl.h"

static TFont* sEnabledFont = NULL;
static TFont* sDisabledFont = NULL;


TDiffListView::TDiffListView(TWindow* parent, const TRect& bounds, TDynamicArray<TDiffRec>& diffList)
	:	TTextListView(parent, bounds),
		fRowHeight(0),
		fDiffList(diffList)
{
	fColumnWidth = 10000;
}


TDiffListView::~TDiffListView()
{
}


void TDiffListView::GetText(int row, TString& outText)
{
	if (fDiffList.GetSize() == 0)
	{
		ASSERT(row == 0);
		outText = "No differences";
	}
	else
	{
		const TDiffRec& diff = fDiffList[row];
		int leftCount = diff.leftEnd - diff.leftStart;
		int rightCount = diff.rightEnd - diff.rightStart;
		
		char	buffer[100];
		
		if (leftCount == 0)
		{
			sprintf(buffer, "%d line%s inserted on right", rightCount, (rightCount == 1 ? "" : "s"));
			outText = buffer;
		}
		else if (rightCount == 0)
		{
			sprintf(buffer, "%d line%s inserted on left", leftCount, (leftCount == 1 ? "" : "s"));
			outText = buffer;
		}
		else
		{
			outText = "nonmatching lines";
		}
	}
}


void TDiffListView::DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context)
{
	bool enabled = false;

	// use italic font for disabled diffs
	if (fDiffList.GetSize() > 0)
	{
		const TDiffRec& diff = fDiffList[row];	
		enabled = diff.enabled;
	}
	
	if (enabled)
	{
		if (!sEnabledFont)
			sEnabledFont = new TFont(_("-adobe-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*"));
			
		context.SetFont(sEnabledFont);
	}
	else
	{
		if (!sDisabledFont)
			sDisabledFont = new TFont(_("-adobe-helvetica-medium-o-normal-*-12-*-*-*-*-*-*-*"));
			
		context.SetFont(sDisabledFont);
	}


	TTextListView::DrawCell(row, column, cellBounds, context);
}


int TDiffListView::GetRowCount() const
{
	// make sure we have atleast one row
	int result = fDiffList.GetSize();
	return (result == 0 ? 1 : result);
}


bool TDiffListView::AllowCellSelect(int row, int column, bool multiple)
{
	// if no differences, we have one non-selectable row
	return (fDiffList.GetSize() > 0);
}


void TDiffListView::DiffListChanged()
{
	ComputeContentSize();
	Redraw();

	UnselectAll();

	if (fDiffList.GetSize() > 0)
	{
		SelectCell(0, 0);	// select first diff
		ScrollSelectionIntoView();
		DoCommand(this, this, kSelectionChangedCommandID);	
	}
}
