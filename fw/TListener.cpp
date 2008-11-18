// ========================================================================================
//	TListener.cpp			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TListener.h"
#include "TBroadcaster.h"

TListener::TListener()
{
}


TListener::~TListener()
{
	TListIterator<TBroadcaster>	iter(fBroadcasters);
	TBroadcaster* broadcaster;
	
	while ((broadcaster = iter.Next()) != NULL)
		broadcaster->RemoveListener(this);
}


void TListener::BroadcasterDied(TBroadcaster* broadcaster)
{
	ASSERT(broadcaster);
	fBroadcasters.Remove(broadcaster);
}


void TListener::AddBroadcaster(TBroadcaster* broadcaster)
{
	ASSERT(broadcaster);
	fBroadcasters.Insert(broadcaster);
}


void TListener::RemoveBroadcaster(TBroadcaster* broadcaster)
{
	ASSERT(broadcaster);
	fBroadcasters.Remove(broadcaster);
}
