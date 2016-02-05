// ========================================================================================
//	TIdler.h		 		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TIdler__
#define __TIdler__

#include "TLinkedList.h"

const TTime			kMaxIdleTime = 0x7FFFFFFF;

class TIdler : public TLinkedListItem
{
public:
							TIdler();
	virtual					~TIdler();

	inline TTime			GetIdleFrequency() const { return fIdleFrequency; }
	inline TTime			GetNextIdle() const { return fNextIdle; }
	inline bool				IdlingEnabled() const { return fIdlingEnabled; }
	
	void					SetIdleFrequency(TTime frequency);
	void					EnableIdling(bool enable);
	void					Sleep(TTime delay);

	void					HandleIdle();	// called by TApplication
	virtual void			DoIdle() = 0;	// override to do the idling

private:
	TTime					fIdleFrequency;
	TTime					fNextIdle;
	bool					fIdlingEnabled;
};

#endif // __TIdler__
