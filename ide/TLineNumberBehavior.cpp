// ========================================================================================
//	TLineNumberBehavior.cpp	   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TLineNumberBehavior.h"
#include "fw/TTextView.h"
#include "fw/TStatusBar.h"
#include "fw/TDocument.h"

#include "fw/intl.h"

#include <stdio.h>


TLineNumberBehavior::TLineNumberBehavior(TTextView* textView, TStatusBar* statusBar, int statusIndex)
	:	fTextView(textView),
		fStatusBar(statusBar),
		fStatusIndex(statusIndex),
		fSelectionStart(0),
		fSelectionEnd(0)
{
	ASSERT(textView);
	ASSERT(statusBar);
	
	DoCommand(textView, textView, kSelectionChangedCommandID);
}


TLineNumberBehavior::~TLineNumberBehavior()
{
}


bool TLineNumberBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (command == kSelectionChangedCommandID && sender == fTextView)
	{
		STextOffset	start, end;
		fTextView->GetSelection(start, end);
		TTextLayout* layout = fTextView->GetTextLayout();
		uint32 line;
		if (start != fSelectionStart)
			line = layout->OffsetToLine(start, true);
		else
			line = layout->OffsetToLine(end, true);

		fSelectionStart = start;
		fSelectionEnd = end;
				
		char buffer[30];
		if (start == end)
			sprintf(buffer, _("Line %ld:%ld"), line + 1, layout->OffsetToColumn(start));
		else
			sprintf(buffer, _("Line %ld"), line + 1);

		fStatusBar->SetStatus(buffer, fStatusIndex);
	}

	return false;
}
