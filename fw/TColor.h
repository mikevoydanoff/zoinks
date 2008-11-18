// ========================================================================================
//	TColor.h				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TColor__
#define __TColor__

#include <X11/Xlib.h>

typedef unsigned char TColorCoord;

class TColor
{
public:
	inline TColor() 
		: fRed(0), fGreen(0), fBlue(0), fPixel(0), fInitialized(false) {}

	inline TColor(TColorCoord red, TColorCoord green, TColorCoord blue) 
		: fRed(red), fGreen(green), fBlue(blue), fPixel(0), fInitialized(false) {}

	inline TColorCoord		Red() const { return fRed; }
	inline TColorCoord		Green() const { return fGreen; }
	inline TColorCoord		Blue() const { return fBlue; } 

	static void				Initialize(Display* display, int screen);

	unsigned long			GetPixel();
	
	void					Set(TColorCoord red, TColorCoord green, TColorCoord blue);
	
private:
	TColorCoord		fRed;
	TColorCoord		fGreen;
	TColorCoord		fBlue;

	unsigned long	fPixel;
	bool			fInitialized;

	static Display*	sDisplay;
	static int		sScreen;
};

// some useful colors

extern TColor kWhiteColor;
extern TColor kBlackColor;
extern TColor kRedColor;
extern TColor kGreenColor;
extern TColor kBlueColor;
extern TColor kYellowColor;
extern TColor kOrangeColor;
extern TColor kGrayColor;

extern TColor kLightGrayColor;
extern TColor kMediumGrayColor;
extern TColor kDarkGrayColor;


#endif // __TColor__
