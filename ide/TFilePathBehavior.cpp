// ========================================================================================
//	TFilePathBehavior.cpp	   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TFilePathBehavior.h"
#include "fw/TTextView.h"
#include "fw/TStatusBar.h"
#include "fw/TDocument.h"

#include <stdio.h>


TFilePathBehavior::TFilePathBehavior(TDocument* document, TStatusBar* statusBar, int statusIndex)
	:	fDocument(document),
		fStatusBar(statusBar),
		fStatusIndex(statusIndex)
{
	ASSERT(document);
	ASSERT(statusBar);
	
	DoCommand(document, document, kFilePathChangedCommandID);
}


TFilePathBehavior::~TFilePathBehavior()
{
}


bool TFilePathBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (command == kFilePathChangedCommandID && fStatusBar->GetItemCount() >= 2)
	{
		fStatusBar->SetStatus(fDocument->GetFile().GetPath(), fStatusIndex);
	}

	return false;
}
