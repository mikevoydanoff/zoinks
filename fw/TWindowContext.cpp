// ========================================================================================
//	TWindowContext.cpp		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TWindowContext.h"
#include "TDocumentWindow.h"


TWindowContext::TWindowContext(TCommandHandler* nextHandler)
	:	TCommandHandler(nextHandler),
		fParentContext(NULL),
		fClosing(false)
{
}


TWindowContext::~TWindowContext()
{
	if (fParentContext)
		fParentContext->RemoveSubContext(this);
}


bool TWindowContext::AllowClose()
{
	return true;
}


void TWindowContext::Open(TDocumentWindow* window)
{
	window->SetWindowContext(this);
}


void TWindowContext::Close()
{	
	if (!fClosing)
	{
		if (CloseSubContexts())
		{
			fClosing = true;

			TListIterator<TTopLevelWindow>	iter(fWindowList);
			TTopLevelWindow* window;
			while ((window = iter.Next()) != NULL)
				window->DoClose();
		}
	}
	else
		delete this;
}


TTopLevelWindow* TWindowContext::GetMainWindow()
{
	if (fWindowList.GetSize() > 0)
		return fWindowList.First();
	else
		return NULL;
}


void TWindowContext::AddWindow(TTopLevelWindow* window)
{
	fWindowList.Insert(window);
}


void TWindowContext::RemoveWindow(TTopLevelWindow* window)
{
	fWindowList.Remove(window);
	if (fWindowList.GetSize() == 0 && fSubContexts.GetSize() == 0)
		delete this;
}


void TWindowContext::AddSubContext(TWindowContext* context)
{
	ASSERT(context->fParentContext == NULL);
	context->fParentContext = this;
	
	fSubContexts.Insert(context);
}


void TWindowContext::RemoveSubContext(TWindowContext* context)
{
	ASSERT(context->fParentContext == this);
	context->fParentContext = NULL;
	
	fSubContexts.Remove(context);
	if (fWindowList.GetSize() == 0 && fSubContexts.GetSize() == 0)
		Close();
}

bool TWindowContext::CloseSubContexts()
{
	TListIterator<TWindowContext>	iter(fSubContexts);
	TWindowContext* context;
	while ((context = iter.Next()) != NULL)
	{
		if (context->AllowClose())
			context->Close();
		else
			return false;	// bail
	}

	return true;
}
