// ========================================================================================
//	TLinkedList.cpp			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TLinkedList.h"


TLinkedListItem::TLinkedListItem()
	:	fNext(NULL)
{
}


TLinkedListBase::TLinkedListBase()
	:	fItems(NULL)
{
}

	
TLinkedListBase::~TLinkedListBase()
{
}


void TLinkedListBase::InsertItem(TLinkedListItem* item)
{
	item->fNext = fItems;
	fItems = item;
}


void TLinkedListBase::RemoveItem(TLinkedListItem* item)
{
	TLinkedListItem* last = NULL;
	TLinkedListItem* current = fItems;

	while (current)
	{
		if (current == item)
		{
			if (last)
				last->fNext = current->fNext;
			else
				fItems = current->fNext;

			return;
		}
		else
		{
			last = current;
			current = current->fNext;
		}
	}
}
		
