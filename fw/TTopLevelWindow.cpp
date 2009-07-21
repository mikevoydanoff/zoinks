// ========================================================================================
//	TTopLevelWindow.cpp		 	Copyright (C) 2001-2003 Mike Lockwood. All rights reserved.
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

#include "TTopLevelWindow.h"
#include "TApplication.h"
#include "TDrawContext.h"
#include "TInputContext.h"
#include "TMenuBar.h"
#include "TPixmap.h"

#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdio.h>

TPixmap* TTopLevelWindow::sDefaultIcon = NULL;
TList<TTopLevelWindow> TTopLevelWindow::sWindowList;


TTopLevelWindow::TTopLevelWindow(const TRect& bounds, const TChar* title)
	:	TWindow(NULL, bounds, kTopLevelWindow),
		fTitle(title),
		fMenuBar(NULL),
		fTarget(NULL),
		fHideOnClose(false),
		fIcon(NULL)
{
}


TTopLevelWindow::~TTopLevelWindow()
{
}


TTopLevelWindow* TTopLevelWindow::GetTopLevelWindow()
{
	return this;
}


void TTopLevelWindow::Create()
{
	TWindow::Create();

	if (fWindow)
	{
		char* title = new char[fTitle.GetLength() + 1];
		fTitle.CopyTo(title, fTitle.GetLength() + 1);
		
		XWMHints* wmHints = XGetWMHints(sDisplay, fWindow);
		if (!wmHints)
			wmHints = XAllocWMHints();
			
		wmHints->flags = StateHint | InputHint | WindowGroupHint;
		wmHints->input = True;
		wmHints->initial_state = NormalState;
		wmHints->window_group = gApplication->GetLeaderWindow();
	
		TPixmap* icon = (fIcon ? fIcon : sDefaultIcon);
		if (icon)
		{
			wmHints->icon_pixmap = icon->GetPixmap();
			wmHints->icon_mask = icon->GetMask();
		
			wmHints->flags |= IconPixmapHint;
			if (wmHints->icon_mask)
				wmHints->flags |= IconMaskHint;
		}
		
		XClassHint* classHints = XAllocClassHint();
		classHints->res_name = (char *)"Zoinks";
		classHints->res_class = (char *)"ZOINKS";
		
		int argc = 0;
		char** argv = NULL;
		
	    XmbSetWMProperties(gApplication->GetDisplay(), fWindow,
			title, title, argv, argc, NULL, wmHints, classHints);
			
		static Atom leaderAtom;
		if (!leaderAtom)
			leaderAtom = XInternAtom(sDisplay, "WM_CLIENT_LEADER", false);

		Window leaderWindow = gApplication->GetLeaderWindow();
		XChangeProperty (sDisplay, fWindow, leaderAtom, XA_WINDOW, 32, PropModeReplace, (const unsigned char *)&leaderWindow, 1);
	}
}


void TTopLevelWindow::DoClose()
{
	if (fHideOnClose)
		Show(false);
	else
	{
		sWindowList.Remove(this);
		TWindow::DoClose();
	}
}


void TTopLevelWindow::Show(bool doShow)
{
	if (doShow != (fVisibility == kWindowVisible))
	{
		if (doShow)
			sWindowList.Insert(this);
		else
			sWindowList.Remove(this);
	}

	TWindow::Show(doShow);
}


void TTopLevelWindow::SetTitle(const TChar* title)
{
	fTitle = title;

	if (fWindow)
	{
		char* title = new char[fTitle.GetLength() + 1];
		fTitle.CopyTo(title, fTitle.GetLength() + 1);
		
	    XmbSetWMProperties(gApplication->GetDisplay(), fWindow,
			title, title, NULL, 0, NULL, NULL, NULL);
	}
}


void TTopLevelWindow::DispatchKeyEvent(XEvent& event)
{
	char			buffer[20];
	KeySym			keySym;
	Status			status;	
	XComposeStatus	xstatus;
	int				count;
	
	TWindow* target = (fTarget ? fTarget : this); 
	TInputContext* inputContext = target->GetInputContext();

	if (inputContext)
		count = XmbLookupString(inputContext->GetXIC(), &event.xkey, buffer, sizeof(buffer) - 1, &keySym, &status);
	else
		count = XLookupString(&event.xkey, buffer, sizeof(buffer) - 1, &keySym, &xstatus);
		
	ASSERT(count < (int)sizeof(buffer) - 1);
	buffer[count] = 0;
	
	fCurrentEventTime = event.xkey.time;
	
	unsigned int state = event.xkey.state;
/* disabled, since Brian said this was causing problems under RedHat/KDE
	if (state & Mod2Mask)
	{
		// workaround for X server on MacOS X that sets Mod2 bit instead of Mod1
		state |= Mod1Mask;
	}
*/

	if (event.type == KeyPress)
	{
		if (state != 0)
		{
			TMenuBar* menuBar = GetMenuBar();

			if (menuBar && menuBar->DoShortCut(keySym, state, buffer))
				return;
		}

		target->HandleKeyDown(keySym, state, buffer);
	}		
	else 
	{
		target->HandleKeyUp(keySym, state, buffer);
	}
}


void TTopLevelWindow::SetTarget(TWindow* target)
{
	if (target != fTarget)
	{
		if (fTarget)
		{
			fTarget->SetHasFocus(false);
			fTarget->HandleCommand(this, fTarget, kFocusLostCommandID);		
		}
		
		fTarget = target;

		if (target)
		{		
			target->SetHasFocus(true);
			target->HandleCommand(this, fTarget, kFocusAcquiredCommandID);
		}
	}
}				


void TTopLevelWindow::GotFocus(TWindow* window)
{	
	if (fTarget)
	{
		fTarget->SetHasFocus(true);
		fTarget->HandleCommand(this, fTarget, kFocusAcquiredCommandID);
	}
}


void TTopLevelWindow::LostFocus(TWindow* window)
{	
	if (fTarget)
	{
		fTarget->SetHasFocus(false);
		fTarget->HandleCommand(this, fTarget, kFocusLostCommandID);	
	}
}


void TTopLevelWindow::SetIcon(TPixmap* icon)
{
	fIcon = icon;
	
	if (icon && IsCreated())
		DoSetIcon(icon);
}


void TTopLevelWindow::DoSetIcon(TPixmap* icon)
{
	ASSERT(IsCreated());
	ASSERT(icon);
	
	XWMHints* hints =  XGetWMHints(sDisplay, fWindow);
	if (!hints)
		hints = XAllocWMHints();
	
	hints->icon_pixmap = icon->GetPixmap();
	hints->icon_mask = icon->GetMask();
	hints->flags = IconPixmapHint;
	if (hints->icon_mask)
		hints->flags |= IconMaskHint;
			
	XSetWMHints(sDisplay, fWindow, hints);
	XFree(hints);
}


