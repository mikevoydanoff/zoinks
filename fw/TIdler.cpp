// ========================================================================================
//	TIdler.cpp				   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TIdler.h"
#include "TApplication.h"


TIdler::TIdler()
	:	fIdleFrequency(kMaxIdleTime),
		fNextIdle(kMaxIdleTime),
		fIdlingEnabled(false)
{
}


TIdler::~TIdler()
{
	if (fIdlingEnabled)
		gApplication->RemoveIdler(this);
}

	
void TIdler::SetIdleFrequency(TTime frequency)
{
	if (fIdleFrequency != frequency)
	{
		fIdleFrequency = frequency;
		gApplication->NextIdleChanged(this);
	}
}


void TIdler::EnableIdling(bool enable)
{
	if (fIdlingEnabled != enable)
	{
		fIdlingEnabled = enable;

		if (enable)
		{
			fNextIdle = gApplication->GetCurrentTime() + fIdleFrequency;
			gApplication->AddIdler(this);
		}
		else
			gApplication->RemoveIdler(this);
	}
}


void TIdler::Sleep(TTime delay)
{
	fNextIdle = gApplication->GetCurrentTime() + fIdleFrequency;
	gApplication->NextIdleChanged(this);
}


void TIdler::HandleIdle()
{
	DoIdle();
	fNextIdle = gApplication->GetCurrentTime() + fIdleFrequency;
}
