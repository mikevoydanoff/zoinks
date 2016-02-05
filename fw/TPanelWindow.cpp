// ========================================================================================
//	TPanelWindow.cpp		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TPanelWindow.h"
#include "TDrawContext.h"
#include "TColor.h"
#include "TFont.h"
#include "TGraphicsUtils.h"


TPanelWindow::TPanelWindow(TWindow* parent, const TRect& bounds)
	:	TWindow(parent, bounds, kChildWindow)
{
	SetBackColor(kLightGrayColor);
}


TPanelWindow::~TPanelWindow()
{
}


void TPanelWindow::Draw(TRegion* clip)
{
	TDrawContext	context(this, clip);
	TRect bounds;
	
	GetLocalBounds(bounds);
	TGraphicsUtils::Draw3DBorderAndInset(context, bounds);
}

