// ========================================================================================
//	TCommandHandler.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TCommandHandler.h"
#include "TBehavior.h"
#include "TDocument.h"


TCommandHandler::TCommandHandler(TCommandHandler* nextHandler)
	:	fNextHandler(nextHandler),
		fBehaviors(NULL)
{
}


TCommandHandler::~TCommandHandler()
{
	TBehavior* behavior = fBehaviors;
	
	while (behavior)
	{
		TBehavior* next = behavior->GetNextBehavior();
		delete behavior;
		behavior = next;
	}
}


void TCommandHandler::AddBehavior(TBehavior* behavior)
{
	ASSERT(behavior);
	ASSERT(behavior->GetOwner() == NULL);

	behavior->SetOwner(this, fBehaviors);
	fBehaviors = behavior;
}


void TCommandHandler::RemoveBehavior(TBehavior* behavior)
{
	ASSERT(behavior);
	ASSERT(behavior->GetOwner() == this);

	TBehavior* previous = NULL;
	TBehavior* current = fBehaviors;

	while (behavior)
	{
		TBehavior* next = current->GetNextBehavior();
		
		if (current == behavior)
		{	
			if (previous)
				previous->SetNextBehavior(next);
			else
				fBehaviors = next;

			break;
		}

		previous = current;
		current = next;
	}
}


void TCommandHandler::HandleSetupMenu(TMenu* menu)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		behavior->DoSetupMenu(menu);
		behavior = behavior->GetNextBehavior();
	}
	
	DoSetupMenu(menu);
}


void TCommandHandler::DoSetupMenu(TMenu* menu)
{
	if (fNextHandler)
		fNextHandler->HandleSetupMenu(menu);
}


bool TCommandHandler::HandleCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoCommand(sender, receiver, command))
			return true;
		else
			behavior = behavior->GetNextBehavior();
	}

	return DoCommand(sender, receiver, command);
}


bool TCommandHandler::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (fNextHandler)
		return fNextHandler->HandleCommand(sender, receiver, command);
	else
		return false;	// not handled
}


bool TCommandHandler::HandleKeyDown(KeySym key, TModifierState state, const char* string)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoKeyDown(key, state, string))
			return true;
		else
			behavior = behavior->GetNextBehavior();
	}

	return DoKeyDown(key, state, string);
}


bool TCommandHandler::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	if (fNextHandler)
		return fNextHandler->HandleKeyDown(key, state, string);
	else
		return false;	// not handled	
}


bool TCommandHandler::HandleKeyUp(KeySym key, TModifierState state, const char* string)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoKeyUp(key, state, string))
			return true;
		else
			behavior = behavior->GetNextBehavior();
	}

	return DoKeyUp(key, state, string);
}


bool TCommandHandler::DoKeyUp(KeySym key, TModifierState state, const char* string)
{
	if (fNextHandler)
		return fNextHandler->HandleKeyUp(key, state, string);
	else
		return false;	// not handled	
}


void TCommandHandler::HandleMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoMouseDown(point, button, state))
			return;
		else
			behavior = behavior->GetNextBehavior();
	}

	DoMouseDown(point, button, state);
}


void TCommandHandler::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
}


void TCommandHandler::HandleMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoMouseUp(point, button, state))
			return;
		else
			behavior = behavior->GetNextBehavior();
	}

	DoMouseUp(point, button, state);
}


void TCommandHandler::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
}


void TCommandHandler::HandleMouseEnter(const TPoint& point, TModifierState state)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoMouseEnter(point, state))
			return;
		else
			behavior = behavior->GetNextBehavior();
	}

	DoMouseEnter(point, state);
}


void TCommandHandler::DoMouseEnter(const TPoint& point, TModifierState state)
{
}


void TCommandHandler::HandleMouseLeave(const TPoint& point, TModifierState state)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoMouseLeave(point, state))
			return;
		else
			behavior = behavior->GetNextBehavior();
	}

	DoMouseLeave(point, state);
}


void TCommandHandler::DoMouseLeave(const TPoint& point, TModifierState state)
{
}


void TCommandHandler::HandleMouseMoved(const TPoint& point, TModifierState state)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoMouseMoved(point, state))
			return;
		else
			behavior = behavior->GetNextBehavior();
	}

	DoMouseMoved(point, state);
}


void TCommandHandler::DoMouseMoved(const TPoint& point, TModifierState state)
{
}


void TCommandHandler::HandleScrollWheel(bool down)
{
	TBehavior* behavior = fBehaviors;

	while (behavior)
	{
		if (behavior->DoScrollWheel(down))
			return;
		else
			behavior = behavior->GetNextBehavior();
	}

	DoScrollWheel(down);
}


void TCommandHandler::DoScrollWheel(bool down)
{
}


bool TCommandHandler::DismissesDialog(TCommandID command) const 
{
	return false;
}


TDocument* TCommandHandler::GetDocument()
{
	TCommandHandler* handler = this;

	while (handler)
	{
		TDocument* document = dynamic_cast<TDocument*>(handler);

		if (document)
			return document;
		else
			handler = handler->fNextHandler;
	}

	return NULL;
}


