// ========================================================================================
//	TDiffTextView.h			 		 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#ifndef __TDiffTextView__
#define __TDiffTextView__

#include "TEditorTextView.h"


class TDiffTextView : public TEditorTextView
{
public:
								TDiffTextView(TWindow* parent, const TRect& bounds, TFont* font);

	void						SelectLineRange(uint32 startLine, uint32 endLine);
									
protected:
	virtual						~TDiffTextView();
};

#endif // __TDiffTextView__
