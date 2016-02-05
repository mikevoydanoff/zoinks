// ========================================================================================
//	TSubWindowIterator.cpp	   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TSubWindowIterator.h"
#include "TWindow.h"


TSubWindowIterator::TSubWindowIterator(TWindow* root, TWindow* first, bool forward)
	:	fRootWindow(root),
		fFirstWindow(first),
		fCurrentWindow(first),
		fForward(forward)
{
	ASSERT(root);
	
	if (!first)
	{
		if (forward)
			first = root->GetFirstSubWindow();
		else
			first = root->GetLastSubWindow();
	}
}

		
TSubWindowIterator::~TSubWindowIterator()
{
}


TWindow* TSubWindowIterator::Next()
{	
	// no windows to iterate through
	if (!fFirstWindow)
		return NULL;

	TWindow* current = fCurrentWindow;
	TWindow* next = NULL;

	if (fForward)
	{
		// try descending first
		TWindow* window = current->GetFirstSubWindow();

		// then try next sibling
		if (!window)
			window = current->GetNextSibling();

		// go back up
		while (!window)
		{
			if (current == fRootWindow)
				break;
			
			current = current->GetParent();
			if (!current)
				break;

			window = current->GetNextSibling();
		}		

		if (!window)
			window = fRootWindow->GetFirstSubWindow();	// loop around to the beginning

		next = window;
	}
	else
	{
		// try descending first
		TWindow* window = current->GetLastSubWindow();

		// then try previous sibling
		if (!window)
			window = current->GetPreviousSibling();

		// go back up
		while (!window)
		{
			if (current == fRootWindow)
				break;
			
			current = current->GetParent();
			if (!current)
				break;

			window = current->GetPreviousSibling();
		}		

		if (!window)
			window = fRootWindow->GetLastSubWindow();	// loop around to the end

		next = window;
	}
	
	if (next == fFirstWindow)
		return NULL;	// got to the end
	else
	{
		fCurrentWindow = next;
		return next;
	}
}
