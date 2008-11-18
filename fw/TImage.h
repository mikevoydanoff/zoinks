// ========================================================================================
//	TImage.h		 			Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TImage__
#define __TImage__

#include "TGeometry.h"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>


class TImage
{
public:
							TImage(TCoord width, TCoord height);						
	virtual 				~TImage();

	inline XImage*			GetImage() const { return fImage; }
	inline bool				UsingSharedMemory() const { return fUsingShm; }
	inline TCoord			GetWidth() const { return fWidth; }
	inline TCoord			GetHeight() const { return fHeight; }
	inline int				GetRowBytes() const { return fRowBytes; }
	inline int				GetDepth() const { return fDepth; }
	inline char*			GetBuffer() const { return fBuffer; }

protected:
	XImage*					fImage;
	XShmSegmentInfo			fShmInfo;	
	char*					fBuffer;
	bool					fUsingShm;
	TCoord					fWidth;
	TCoord					fHeight;
	int						fRowBytes;
	int						fDepth;
};

#endif // __TImage__
