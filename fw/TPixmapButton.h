// ========================================================================================
//	TPixmapButton.h				Copyright (C) 2001-2003 Mike Lockwood. All rights reserved.
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

#ifndef __TPixmapButton__
#define __TPixmapButton__

#include "TButton.h"
#include "TCommandID.h"

class TPixmap;


class TPixmapButton : public TButton
{
public:
								TPixmapButton(TWindow* parent, const TRect& bounds, 
											  const TPixmap* enabledPixmap, const TPixmap* disabledPixmap, 
											  TCommandID commandID = kNoCommandID, bool dismissesDialog = false);
	virtual 					~TPixmapButton();

	void						SetDefault();

protected:
	virtual void				Draw(TRegion* clip);

protected:
	const TPixmap*				fEnabledPixmap;
	const TPixmap*				fDisabledPixmap;
};

#endif // __TPixmapButton__
