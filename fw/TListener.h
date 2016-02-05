// ========================================================================================
//	TListener.h				   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TListener__
#define __TListener__

#include "TList.h"

class TBroadcaster;

class TListener
{
public:
				TListener();
	virtual		~TListener();

	void		BroadcasterDied(TBroadcaster* broadcaster);

protected:
	friend class TBroadcaster;
	void AddBroadcaster(TBroadcaster* broadcaster);
	void RemoveBroadcaster(TBroadcaster* broadcaster);

	TList<TBroadcaster>	fBroadcasters;	
};

#define	NOTIFY_ALL(T, F, x)											\
	{ 																\
		TListIterator<TListener>	iter(fListeners);				\
		TListener* listener; 										\
		while ((listener = iter.Next()) != NULL)					\
		{															\
			try														\
			{														\
				T* tListener = dynamic_cast<T*>(listener);			\
				if (tListener)										\
					tListener->F(x);								\
			}														\
			catch(...) {}											\
		}															\
	}

#define	NOTIFY_ALL_2(T, F, x, y)									\
	{ 																\
		TListIterator<TListener>	iter(fListeners);				\
		TListener* listener; 										\
		while ((listener = iter.Next()) != NULL)					\
		{															\
			try														\
			{														\
				T* tListener = dynamic_cast<T*>(listener);			\
				if (tListener)										\
					tListener->F(x, y);								\
			}														\
			catch(...) {}											\
		}															\
	}

#define	NOTIFY_ALL_3(T, F, x, y, z)									\
	{ 																\
		TListIterator<TListener>	iter(fListeners);				\
		TListener* listener; 										\
		while ((listener = iter.Next()) != NULL)					\
		{															\
			try														\
			{														\
				T* tListener = dynamic_cast<T*>(listener);			\
				if (tListener)										\
					tListener->F(x, y, z);							\
			}														\
			catch(...) {}											\
		}															\
	}

#endif // __TListener__
