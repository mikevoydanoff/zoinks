// ========================================================================================
//	TGraphicsUtils.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TGraphicsUtils.h"
#include "TDrawContext.h"
#include "TGeometry.h"
#include "TPixmap.h"

#include "Pixmaps/GrayStipple.xpm"


const TRect TGraphicsUtils::f3DBorderInset(2, 2, 2, 2);
TPixmap* TGraphicsUtils::fGrayStipple = NULL;


void TGraphicsUtils::Draw3DBorderAndInset(TDrawContext& context, TRect& r, bool defaultBorder)
{
	r.right--; r.bottom--;

	if (context.GetDepth() < 8 || defaultBorder)
		context.SetForeColor(kBlackColor);
	else
		context.SetForeColor(kLightGrayColor);
	context.DrawLine(r.left, r.top, r.right - 1, r.top);
	context.DrawLine(r.left, r.top + 1, r.left, r.bottom);

	context.SetForeColor(kBlackColor);
	context.DrawLine(r.left + 1, r.bottom, r.right, r.bottom);
	context.DrawLine(r.right, r.top, r.right, r.bottom - 1);

	r.left++; r.top++; r.right--; r.bottom--;
	
	if (defaultBorder)
	{
		context.DrawLine(r.left + 1, r.bottom, r.right, r.bottom);
		context.DrawLine(r.right, r.top, r.right, r.bottom - 1);
		r.right--; r.bottom--;	
	}

	context.SetForeColor(kWhiteColor);
	context.DrawLine(r.left, r.top, r.right - 1, r.top);
	context.DrawLine(r.left, r.top + 1, r.left, r.bottom);

	context.SetForeColor(kMediumGrayColor);
	context.DrawLine(r.left + 1, r.bottom, r.right, r.bottom);
	context.DrawLine(r.right, r.top, r.right, r.bottom - 1);

	r.left++; r.top++;
}

void TGraphicsUtils::DrawPressed3DBorderAndInset(TDrawContext& context, TRect& r, bool defaultBorder)
{
	r.right--; r.bottom--;

	if (1/*context.GetDepth() < 8 || defaultBorder*/)
		context.SetForeColor(kBlackColor);
	else
		context.SetForeColor(kMediumGrayColor);
	context.FrameRect(r);

	r.left++; r.top++;

	if (context.GetDepth() >= 8)
		context.SetForeColor(kLightGrayColor);
	context.DrawLine(r.left, r.top, r.right - 1, r.top);
	context.DrawLine(r.left, r.top + 1, r.left, r.bottom - 1);

	r.left++; r.top++;
}


TPixmap* TGraphicsUtils::GetGrayStipple()
{
	if (!fGrayStipple)
		fGrayStipple = new TPixmap(GrayStipple_xpm);
		
	return fGrayStipple;
}

