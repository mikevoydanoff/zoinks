// ========================================================================================
//	TTypeSelectable.h			 	Copyright (C) 2006 Mike Voydanoff. All rights reserved.
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

#ifndef __TTypeSelectable__
#define __TTypeSelectable__


class TTypeSelectable
{
public:
	virtual ~TTypeSelectable() {}
	
	virtual bool	TTypeSelectable_IsMultiSelect() = 0;
	virtual void	TTypeSelectable_SelectAll() = 0;
	virtual int		TTypeSelectable_GetCount() = 0;
	virtual bool	TTypeSelectable_AllowSelect(int i) = 0;
	virtual void	TTypeSelectable_GetText(int i, TString& text) = 0;
	virtual int		TTypeSelectable_GetSelection() = 0;
	virtual void 	TTypeSelectable_Select(int i) = 0;
};

#endif // __TTypeSelectable__
