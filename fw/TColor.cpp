// ========================================================================================
//	TColor.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TColor.h"

Display* TColor::sDisplay = NULL;
int TColor::sScreen = 0;


void TColor::Initialize(Display* display, int screen)
{
	sDisplay = display;
	sScreen = screen;
}


unsigned long TColor::GetPixel()
{
	if (!fInitialized)
	{
		ASSERT(sDisplay);

		XColor	xcolor;
		xcolor.red = fRed << 8;
		xcolor.green = fGreen << 8;
		xcolor.blue = fBlue << 8;
		
		Colormap colorMap = DefaultColormap(sDisplay, sScreen);
		XAllocColor(sDisplay, colorMap, &xcolor);	

		fPixel = xcolor.pixel;
		fInitialized = true;
	}

	return fPixel;
}


void TColor::Set(TColorCoord red, TColorCoord green, TColorCoord blue)
{
	ASSERT(fInitialized == false);		// otherwise we need to call XFreeColors?
	
	fRed = red;
	fGreen = green;
	fBlue = blue;
	fInitialized = false;
}



// some useful colors

TColor kWhiteColor(255, 255, 255);
TColor kBlackColor(0, 0, 0);
TColor kRedColor(255, 0, 0);
TColor kGreenColor(0, 255, 0);
TColor kBlueColor(0, 0, 255);
TColor kYellowColor(255, 255, 0);
TColor kOrangeColor(255, 128, 0);
TColor kGrayColor(127, 127, 127);

TColor kLightGrayColor(213, 214, 213);
TColor kMediumGrayColor(148, 149, 148);
TColor kDarkGrayColor(85, 85, 85);

