// ========================================================================================
//	TGraphicsUtils.h		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TGraphicsUtils__
#define __TGraphicsUtils__

class TRect;
class TDrawContext;
class TPixmap;


class TGraphicsUtils
{
public:
	static void				Draw3DBorderAndInset(TDrawContext& context, TRect& r, bool defaultBorder = false);
	static void				DrawPressed3DBorderAndInset(TDrawContext& context, TRect& r, bool defaultBorder = false);
	
	static TPixmap*			GetGrayStipple();

	static const TRect		f3DBorderInset;
	static TPixmap*			fGrayStipple;

};

#endif // __TGraphicsUtils__