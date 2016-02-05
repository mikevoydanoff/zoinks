// ========================================================================================
//	TWindow.cpp				   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TWindow.h"
#include "TApplication.h"
#include "TCursor.h"
#include "TDrawContext.h"
#include "TFont.h"
#include "TInputContext.h"
#include "TMenu.h"
#include "TRegion.h"
#include "TTopLevelWindow.h"

#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <stdlib.h>
#include <stdio.h>

const TTime kDoubleClickTime = 500;
const TCoord kDoubleClickDelta = 4;


TList<TWindow> TWindow::sWindowList(TWindow::CompareProc, TWindow::SearchProc);
//TLock TWindow::sWindowListLock;

Display* TWindow::sDisplay = NULL;
TCursor* TWindow::sDefaultCursor = NULL;
TWindow* TWindow::sPointerGrabWindow = NULL;
TWindow* TWindow::sFirstUpdate = NULL;

Atom TWindow::sDeleteWindowAtom;
Atom TWindow::sTakeFocusAtom;
Atom TWindow::sStateAtom;
Atom TWindow::sSelectionProperty;


void TWindow::Initialize(Display* display)
{
	sDisplay = display;
	sDeleteWindowAtom = XInternAtom(display, "WM_DELETE_WINDOW", false);
	sTakeFocusAtom = XInternAtom(display, "WM_TAKE_FOCUS", false);
	sStateAtom = XInternAtom(display, "WM_STATE", false);
	sSelectionProperty = XInternAtom(display, "SCOOBY_SNACK", false);
	sDefaultCursor = new TCursor(XC_left_ptr);
}


TWindow::TWindow(TWindow* parent, const TRect& bounds, TWindowStyle style)
	:	TCommandHandler(NULL),
		TDrawable(bounds),
		fWindow(0),
		fParent(parent),
		fForeColor(kBlackColor),
		fBackColor(kWhiteColor),
		fBorder(0),
		fVisibility(kWindowLatent),
		fUpdateRegion(NULL),
		fInputContext(NULL),
		fCurrentEventTime(0),
		fIsSelectionOwner(false),
		fStyle(style),
		fPositioner(NULL),
		fMapped(false),
		fObscured(true),
		fHasFocus(false),
		fRaiseOnMapNotify(false),
		fSetFocusOnMapNotify(false),
		fLastClickTime(0),
		fClickCount(1),
		fNextUpdate(NULL)
{
	if (parent)
	{
		SetNextHandler(parent);
		parent->AddChild(this);
		fForeColor = parent->GetForeColor();
		fBackColor = parent->GetBackColor();
	}
	else
		SetNextHandler(gApplication);
}



TWindow::~TWindow()
{
	TTopLevelWindow* topLevel = GetTopLevelWindow();
	if (topLevel && topLevel->GetTarget() == this)
		topLevel->TargetDied();

	if (fParent)
		fParent->RemoveChild(this);

	TListIterator<TWindow>	iter(fChildren);

	TWindow* window;
	
	while ((window = iter.Next()) != NULL)
	{
		ASSERT(window->fParent == this);
		window->fParent = NULL;
	}

	if (IsCreated())
		RemoveWindow(this);

	// remove from sFirstUpdate linked list
	TWindow* previous = NULL;
	TWindow* current = sFirstUpdate;
	
	delete fUpdateRegion;
	delete fInputContext;
	
	while (current)
	{
		if (current == this)
		{
			if (previous)
				previous->fNextUpdate = current->fNextUpdate;
			else
				sFirstUpdate = current->fNextUpdate;
			
			break;
		}
		
		previous = current;
		current = current->fNextUpdate;
	}
}


void TWindow::Create()
{
	ASSERT(!IsCreated());
	if (!IsCreated())
	{
		int screen = gApplication->GetDefaultScreen();
		Window parent = (fParent ? fParent->GetXWindow() : RootWindow(sDisplay, screen));

		XSetWindowAttributes	attributes;
		unsigned long attributesMask = CWBackPixel;
		int border = fBorder;

		attributes.background_pixel = fBackColor.GetPixel();

		if (fStyle == kPopupWindow)
		{
			attributes.save_under = true;
			attributes.override_redirect = true;
			attributesMask |= CWSaveUnder;
			attributesMask |= CWOverrideRedirect;
			border = 1;
		}
		
		fWindow = XCreateWindow(sDisplay, parent, fBounds.left, fBounds.top, GetWidth(), GetHeight(), 
										border,	CopyFromParent, InputOutput, CopyFromParent, attributesMask, &attributes);

		XSelectInput(sDisplay, fWindow, ExposureMask|FocusChangeMask|
									KeyPressMask|KeyReleaseMask|
									ButtonPressMask|ButtonReleaseMask|
									EnterWindowMask|LeaveWindowMask|PointerMotionMask|
									StructureNotifyMask | VisibilityChangeMask /*|SubstructureNotifyMask*/
									);
									
		XWindowAttributes	attr;
		XGetWindowAttributes(sDisplay, fWindow, &attr);
		fDepth = attr.depth;

	 	if (fStyle == kTopLevelWindow)
	 	{
	 		Atom	atoms[2] = { sDeleteWindowAtom, sTakeFocusAtom };
//ignore take focus?			XSetWMProtocols(sDisplay, fWindow, atoms, 2);
			XSetWMProtocols(sDisplay, fWindow, atoms, 1);
		}

		AddWindow(this);

		// give the positioner a chance to do its thing
//		if (fParent && fPositioner)
//			fPositioner(this, fParent->fBounds, fParent->fBounds);

		SetCursor(sDefaultCursor);

		TListIterator<TWindow> iter(fChildren);
		TWindow* child;
		while ((child = iter.Next()) != NULL)
			child->Create();
	}
}


void TWindow::Show(bool doShow)
{
	if (!IsCreated())
		Create();

	if (doShow)
	{
		XMapWindow(sDisplay, fWindow);
		fVisibility = kWindowVisible;
	}
	else
	{
		XUnmapWindow(sDisplay, fWindow);
		fVisibility = kWindowHidden;
	}

	TListIterator<TWindow> iter(fChildren);
	TWindow* child;
	while ((child = iter.Next()) != NULL)
		child->ParentShown(doShow);
}


void TWindow::Raise()
{
	if (IsIconified())
	{
		XMapWindow(sDisplay, fWindow);
		fSetFocusOnMapNotify = true;
	}

	if (fMapped)
	{
		XSetInputFocus(sDisplay, fWindow, RevertToPointerRoot, CurrentTime);
		XRaiseWindow(sDisplay, fWindow);
	}
	else
		fRaiseOnMapNotify = true;
}


void TWindow::SetBackgroundPixel(TColor* color)
{
	ASSERT(fWindow);	// window must be created
	XSetWindowAttributes	attributes;

	if (color)
		attributes.background_pixel = color->GetPixel();
	else
		attributes.background_pixel = None;

	XChangeWindowAttributes(sDisplay, fWindow, CWBackPixel, &attributes);
}


void TWindow::SetBorder(int border)
{
	int delta = fBorder - border;
	
	if (delta != 0)
	{
		if (fWindow)
		{
			TRect	bounds(fBounds);
			
			bounds.Inset(delta, delta);
			SetBounds(bounds);

			XSetWindowBorderWidth(sDisplay, fWindow, border);
		}

		fBorder = border;
	}
}


void TWindow::Close()
{
	DoClose();
}


void TWindow::Destroy()
{
	DoDestroy();
}


void TWindow::SetBackColor(TColor& color)
{
	fBackColor = color;

	if (IsCreated())
		SetBackgroundPixel(&color);
}


TFont* TWindow::GetFont()
{
	return NULL;
}				


void TWindow::Draw(TRegion* clip)
{
}


void TWindow::DoClose()
{
	DoDestroy();
}


void TWindow::DoDestroy()
{
	TListIterator<TWindow> iter(fChildren);

	TWindow* child;
	while ((child = iter.Next()) != NULL)
		child->Destroy();

	if (IsCreated())
		XDestroyWindow(sDisplay, fWindow);

	delete this;
}


void TWindow::DoMapped(bool mapped)
{
	fMapped = mapped;
	
	if (mapped && fRaiseOnMapNotify)
	{
		XRaiseWindow(sDisplay, fWindow);
		fRaiseOnMapNotify = false;
	}
	
	if (fSetFocusOnMapNotify && mapped)
	{
		XSetInputFocus(sDisplay, fWindow, RevertToPointerRoot, CurrentTime);
		fSetFocusOnMapNotify = false;
	}
}


void TWindow::ParentShown(bool doShow)
{
	if (doShow && fVisibility == kWindowLatent)
		Show(true);
	else if (!doShow && fVisibility == kWindowVisible)
	{
		Show(false);
		fVisibility = kWindowLatent;
	}
}


bool TWindow::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (command == kCloseCommandID && GetTopLevelWindow() == this)
	{
		Close();
		return true;
	}
	else
		return TCommandHandler::DoCommand(sender, receiver, command);
}


void TWindow::NotifyBoundsChanged(const TRect& oldBounds)
{
	TListIterator<TWindow> iter(fChildren);

	TWindow* window;
	
	while ((window = iter.Next()) != NULL)
		window->ParentBoundsChanged(oldBounds, fBounds);
}


void TWindow::ParentBoundsChanged(const TRect& oldParentBounds, const TRect& newParentBounds)
{
	if (fPositioner)
		fPositioner(this, oldParentBounds, newParentBounds);
}


void TWindow::DoScrollWheel(bool down)
{
	if (fParent)
		fParent->DoScrollWheel(down);
}


void TWindow::RequestSelection(Atom type)
{
	XConvertSelection(sDisplay, XA_PRIMARY, type, sSelectionProperty, fWindow, fCurrentEventTime);
}


bool TWindow::BecomeSelectionOwner()
{
	XSetSelectionOwner(sDisplay, XA_PRIMARY, fWindow, fCurrentEventTime);
	fIsSelectionOwner = (XGetSelectionOwner(sDisplay, XA_PRIMARY) == fWindow);
	if (fIsSelectionOwner)
		fSelectionAcquiredTime = fCurrentEventTime;
	return fIsSelectionOwner;
}


void TWindow::RelinquishSelectionOwnership()
{
	if (fIsSelectionOwner)
	{
		XSetSelectionOwner(sDisplay, XA_PRIMARY, None, fSelectionAcquiredTime);
		fIsSelectionOwner = false;
	}
}


TWindow* TWindow::GetSelectionOwner()
{
	Window owner = XGetSelectionOwner(sDisplay, XA_PRIMARY);
	return (owner ? GetWindow(owner) : NULL);
}


bool TWindow::RequestSelectionData(Atom& inOutType, unsigned char*& outData, uint32& outLength)
{
	outData = NULL;
	outLength = 0;
	return false;
}
	

void TWindow::ReceiveSelectionData(Atom type, const unsigned char* data, uint32 length)
{
	ASSERT(0);	// should override this if you are calling RequestSelection
}


void TWindow::LostSelectionOwnership()
{
}


Drawable TWindow::GetDrawable() const
{
	return fWindow;
}


void TWindow::InitDrawContext(TDrawContext& context)
{
	context.SetForeColor(fForeColor);
	context.SetBackColor(fBackColor);
}


void TWindow::SetParent(TWindow* parent)
{
	if (fParent != parent)
	{
		if (fParent)
			fParent->RemoveChild(this);

		fParent = parent;

		if (fWindow)
		{
//			HWND parentHandle = (parent ? parent->GetWindowHandle() : NULL);
//			::SetParent(fWindowHandle, parentHandle);
		}

		if (parent)
			parent->AddChild(this);
	}
}


bool TWindow::CreateInputContext()
{
	if (!fInputContext)
	{
		XIM xim = gApplication->GetInputMethod();
		if (!xim)
			return false;
		
		XIMStyles* supportedStyles;
		XGetIMValues(xim, XNQueryInputStyle, &supportedStyles, NULL, NULL);
		if (!supportedStyles || supportedStyles->count_styles == 0)
			return false;
			
		XIMStyle* bestStyle = NULL;
		for (unsigned short i = 0; i < supportedStyles->count_styles; i++)
		{
			XIMStyle* style = &supportedStyles->supported_styles[i];
			
			// first check to see if it is supported
			if ((*style & (XIMPreeditPosition | XIMPreeditNothing | XIMPreeditNone)) &&
				(*style & ( /*XIMStatusCallbacks |*/ XIMStatusNothing | XIMStatusNone)))
			{
				if (bestStyle)
				{
					if (((*style & XIMPreeditPosition) && !(*bestStyle & XIMPreeditPosition)) ||
						((*style & XIMStatusCallbacks) && !(*bestStyle & XIMStatusCallbacks)))
						bestStyle = style;			
					else if (((*style & XIMPreeditNothing) && !(*bestStyle & (XIMPreeditPosition | XIMPreeditNothing))) ||
							 ((*style & XIMStatusNothing) && !(*bestStyle & (XIMStatusCallbacks | XIMStatusNothing))))
						bestStyle = style;			
				}
				else
					bestStyle = style;
			}
		}
		
		XIMStyle style = (bestStyle ? *bestStyle : 0);
		XFree(supportedStyles);
		if (!bestStyle)
			return false;
		
		XPoint point;
		point.x = point.y = 0;
		XRectangle rect;
		rect.x = rect.y = 0;
		rect.width = rect.height = 0x7fff;
		
		TFont* font = GetFont();
		ASSERT(font && font->GetFontSet());
		XVaNestedList	preeditAttributes = XVaCreateNestedList(0, XNFontSet, font->GetFontSet(), XNSpotLocation, &point, XNArea, &rect, NULL);
		ASSERT(preeditAttributes);
	//	XVaNestedList	statusAttributes = XVaCreateNestedList(0, XNFontSet, font->GetFontSet(), NULL);
	//	ASSERT(statusAttributes);
	
		XIC xic = XCreateIC(xim, XNInputStyle, style, XNClientWindow, fWindow, XNFocusWindow, fWindow, XNPreeditAttributes, preeditAttributes, /*XNStatusAttributes, statusAttributes, */ NULL);
		if (xic)
			fInputContext = new TInputContext(xic);
			
		/*if (fInputContext)
		{
			long mask;
			XGetICValues(fInputContext->GetXIC(), XNFilterEvents, &mask, NULL);
			printf("XIC mask = %lx\n", mask);
 	   }*/
	}
		
	return (fInputContext != NULL);
}


TWindow* TWindow::GetFirstSubWindow() const
{
	if (fChildren.GetSize() > 0)
		return fChildren.First();
	else
		return NULL;
}


TWindow* TWindow::GetLastSubWindow() const
{
	if (fChildren.GetSize() > 0)
		return fChildren.Last();
	else
		return NULL;
}


TWindow* TWindow::GetPreviousSibling() const
{
	if (fParent)
	{
		int32 index = fParent->fChildren.FindIndex(this);

		if (index > 0)
			return fParent->fChildren[index - 1];
	}

	return NULL;
}


TWindow* TWindow::GetNextSibling() const
{
	if (fParent)
	{
		int32 index = fParent->fChildren.FindIndex(this);

		if (index >= 0 && index < fParent->fChildren.GetSize() - 1)
			return fParent->fChildren[index + 1];
	}

	return NULL;
}


void TWindow::AdjustScrollBars(TView* view)
{
}


void TWindow::ChildScrollChanged(TView* child, TCoord deltaH, TCoord deltaV)
{
}


void TWindow::SetCursor(TCursor* cursor)
{
	if (cursor)
		XDefineCursor(sDisplay, fWindow, cursor->GetCursor());
	else
		XUndefineCursor(sDisplay, fWindow);
}


void TWindow::SetTarget()
{
	TTopLevelWindow* topLevel = GetTopLevelWindow();
	if (topLevel)
		topLevel->SetTarget(this);
}


void TWindow::GrabPointer(Time time)
{
	time = CurrentTime;	// ignore time for now
	
	ASSERT(fWindow);
	if (sPointerGrabWindow)
		UngrabPointer(time);
	
	int result = XGrabPointer(sDisplay, fWindow, false, ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
					 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	ASSERT(result == GrabSuccess);

	if (result == GrabSuccess)
		sPointerGrabWindow = this;
}


void TWindow::UngrabPointer(Time time)
{
	time = CurrentTime;	// ignore time for now

//	ASSERT(sPointerGrabWindow == this);

	XUngrabPointer(sDisplay, time);
	
	sPointerGrabWindow = NULL;
}


void TWindow::HandleEvent(XEvent& event)
{
	TPoint	mouse;
	const TPoint& scroll = GetScroll();
	
	switch (event.type)
	{
		case Expose:
		{
			TRect	r(event.xexpose.x, event.xexpose.y, event.xexpose.x + event.xexpose.width, event.xexpose.y + event.xexpose.height);

			if (fUpdateRegion)
				fUpdateRegion->Union(r);
			else
			{
				fUpdateRegion = new TRegion(r);
				fNextUpdate = sFirstUpdate;
				sFirstUpdate = this;
			}
				
//			if (event.xexpose.count == 0)
//				Update();
		
			break;
		}

		case KeyPress:
		case KeyRelease:
		{			
			TTopLevelWindow* topLevel = GetTopLevelWindow();
			if (topLevel)
				topLevel->DispatchKeyEvent(event);
			break;
		}		
		
		case ButtonPress:
			if (event.xbutton.button >= 1 && event.xbutton.button <= 3)
			{
				fCurrentEventTime = event.xbutton.time;
				mouse.Set(event.xbutton.x + scroll.h, event.xbutton.y + scroll.v);

				// check for multiple clicks
				TTime now = gApplication->GetCurrentTime();
				if (now - fLastClickTime < kDoubleClickTime && 
					abs(fLastClick.h - mouse.h) <= kDoubleClickDelta &&
					abs(fLastClick.v - mouse.v) <= kDoubleClickDelta)
					++fClickCount;
				else
					fClickCount = 1;

				fLastClick = mouse;
				fLastClickTime = now;

				if (IsTargetable() && event.xbutton.button != 2)
					RequestTarget();
				
				HandleMouseDown(mouse, (TMouseButton)event.xbutton.button, event.xbutton.state);
			}
			else if (event.xbutton.button == 4 || event.xbutton.button == 5)
			{
				HandleScrollWheel(event.xbutton.button == 5);
			}
			break;

		case ButtonRelease:
			if (event.xbutton.button >= 1 && event.xbutton.button <= 3)
			{
				fCurrentEventTime = event.xbutton.time;
				mouse.Set(event.xbutton.x + scroll.h, event.xbutton.y + scroll.v);
				HandleMouseUp(mouse, (TMouseButton)event.xbutton.button, event.xbutton.state);
			}
			break;

		case EnterNotify:
			fCurrentEventTime = event.xcrossing.time;
			mouse.Set(event.xcrossing.x + scroll.h, event.xcrossing.y + scroll.v);
			HandleMouseEnter(mouse, event.xcrossing.state);
			break;

		case LeaveNotify:
			fCurrentEventTime = event.xcrossing.time;
			mouse.Set(event.xcrossing.x + scroll.h, event.xcrossing.y + scroll.v);
			HandleMouseLeave(mouse, event.xcrossing.state);
			break;

		case MotionNotify:
		{
			// avoid getting too many MotionNotify events by ignoring them if we have more in the queue
			XEvent testEvent;
			if (XPending(sDisplay) > 0 && XCheckTypedWindowEvent(sDisplay, event.xany.window, MotionNotify, &testEvent))
				break;

			fLastMouseMovedLocation.Set(event.xmotion.x, event.xmotion.y);
			fLastMouseMovedModifiers = event.xmotion.state;
			fCurrentEventTime = event.xmotion.time;
			mouse.Set(event.xmotion.x + scroll.h, event.xmotion.y + scroll.v);
			HandleMouseMoved(mouse, event.xmotion.state);
			break;
		}

		case FocusIn:
		{
			if (event.xfocus.detail == NotifyAncestor ||  
				event.xfocus.detail == NotifyInferior ||
				event.xfocus.detail == NotifyNonlinear ||
				event.xfocus.detail == NotifyNonlinearVirtual)
			{
				if (event.xfocus.mode != NotifyGrab &&
					event.xfocus.mode != NotifyUngrab)	// these events confuse XIM, according to gdk comment
				{
					TTopLevelWindow* topLevel = GetTopLevelWindow();
					if (topLevel)
						topLevel->GotFocus(this);
					break;
				}
			}
		}
			
		case FocusOut:
		{
			if (event.xfocus.detail == NotifyAncestor ||  
				event.xfocus.detail == NotifyInferior ||
				event.xfocus.detail == NotifyNonlinear ||
				event.xfocus.detail == NotifyNonlinearVirtual)
			{
				if (event.xfocus.mode != NotifyGrab &&
					event.xfocus.mode != NotifyUngrab)	// these events confuse XIM, according to gdk comment
				{
					TTopLevelWindow* topLevel = GetTopLevelWindow();
					if (topLevel)
						topLevel->LostFocus(this);
					break;
				}
			}
		}	
				
		case KeymapNotify:
//			printf("KeymapNotify\n");
			break;
			
		case GraphicsExpose:
//			printf("GraphicsExpose\n");
			break;
			
		case NoExpose:
//mgl why are we getting these when drawing scrollbars?
//			printf("NoExpose\n");
			break;
			
		case VisibilityNotify:
			fObscured = (event.xvisibility.state != VisibilityUnobscured);
			break;
			
		case CreateNotify:
			printf("CreateNotify\n");
			break;
			
		case DestroyNotify:
			printf("DestroyNotify\n");
			break;
			
		case MapNotify:
			DoMapped(true);
			break;

		case UnmapNotify:
			DoMapped(false);
			break;
			
		case MapRequest:
			printf("MapRequest\n");
			break;
			
		case ReparentNotify:
//			printf("ReparentNotify\n");
			break;
			
		case ConfigureNotify:
		{
			// we manage child window notification ourselves
			if (fParent == NULL)
			{
				TRect oldBounds(fBounds);
				fBounds.Set(event.xconfigure.x, event.xconfigure.y, 
							event.xconfigure.x + event.xconfigure.width,
							event.xconfigure.y + event.xconfigure.height);
	
				NotifyBoundsChanged(oldBounds);
			}
			break;
		}
			
		case ConfigureRequest:
			printf("ConfigureRequest\n");
			break;
			
		case GravityNotify:
			printf("GravityNotify\n");
			break;
			
		case ResizeRequest:
			printf("ResizeRequest\n");
			break;
			
		case CirculateNotify:
			printf("CirculateNotify\n");
			break;
			
		case CirculateRequest:
			printf("CirculateRequest\n");
			break;
			
		case PropertyNotify:
			printf("PropertyNotify\n");
			break;
			
		case SelectionClear:
		{
			if (fIsSelectionOwner)
			{
				LostSelectionOwnership();
				fIsSelectionOwner = false;
			}
			break;
		}
			
		case SelectionRequest:
		{
			XSelectionEvent response;

			response.type = SelectionNotify;
			response.display = event.xselectionrequest.display;
			response.requestor = event.xselectionrequest.requestor;
			response.selection = event.xselectionrequest.selection;
			response.target = event.xselectionrequest.target;
			response.property = event.xselectionrequest.property;
			response.time = event.xselectionrequest.time;
			
			Atom	targetAtom = XInternAtom(event.xselectionrequest.display, "TARGETS", false);
			
			if (event.xselectionrequest.target == targetAtom)
			{
				Atom				targets[] = { XA_STRING };
				int					result;
				
				result = XChangeProperty(event.xselectionrequest.display,
										 event.xselectionrequest.requestor,
										 event.xselectionrequest.property,
										 event.xselectionrequest.target,
										 8,
										 PropModeReplace,
										 (const unsigned char*)targets,
										 sizeof(targets));
							   
				if (result == BadAlloc || result == BadAtom || result == BadMatch || result == BadValue || result == BadWindow)
					printf ("SelectionRequest - XChangeProperty failed: %d\n", result);
				
				response.send_event = true;
			}
			else
			{
				unsigned char* data;
				uint32 length;
				Atom type = event.xselectionrequest.target;
				if (response.selection == XA_PRIMARY && RequestSelectionData(type, data, length))
				{
					response.target = type;
					XChangeProperty(response.display, response.requestor, event.xselectionrequest.property, type, 
									8, PropModeReplace, data, length);
					free(data);
				}
				else
					response.property = None;
			}

			XSendEvent(sDisplay, response.requestor, false, NoEventMask, (XEvent *)&response);	
			break;
		}
			
		case SelectionNotify:
		{
			if (event.xselection.property != None)
			{
				Atom type;
				unsigned long items, length, remaining;
				int actualFormat;
				unsigned char*	data;

				// first compute the length
				XGetWindowProperty(sDisplay, fWindow, event.xselection.property, 0, 0, false, AnyPropertyType,
								&type, &actualFormat, &items, &length, &data);
				if (data)
					XFree(data);

				// now get the data
				XGetWindowProperty(sDisplay, fWindow, event.xselection.property, 0, length, true, AnyPropertyType,
								&type, &actualFormat, &items, &remaining, &data);
				ASSERT(remaining == 0);

				ReceiveSelectionData(type, data, length);
				XFree(data);
			}			
			
			break;
		}
			
		case ColormapNotify:
			printf("ColormapNotify\n");
			break;
			
		case ClientMessage:
			if ((Atom)event.xclient.data.l[0] == sDeleteWindowAtom)
			{
				Close();
			}
			else if ((Atom)event.xclient.data.l[0] == sTakeFocusAtom)
			{
/*				TTopLevelWindow* topLevel = GetTopLevelWindow();
				ASSERT(topLevel == this);
				topLevel->TakeFocus();
*/				
			}
			else
				printf("unknown ClientMessage\n");
			break;
			
		case MappingNotify:
			printf("MappingNotify\n");
			break;
			
		default:
			break;
	}
}


void TWindow::SimulateMouseMoved()
{
	TPoint	mouse;
	const TPoint& scroll = GetScroll();

	mouse.Set(fLastMouseMovedLocation.h + scroll.h, fLastMouseMovedLocation.v + scroll.v);
	HandleMouseMoved(mouse, fLastMouseMovedModifiers);
}


void TWindow::Update()
{
	if (fUpdateRegion)
	{
		Draw(fUpdateRegion);
		delete fUpdateRegion;
		fUpdateRegion = NULL;
	}
}


void TWindow::ProcessUpdates()
{
	TWindow* window = sFirstUpdate;
	sFirstUpdate = NULL;
	
	while (window)
	{
		window->Update();
		window = window->fNextUpdate;
	}
}


TTopLevelWindow* TWindow::GetTopLevelWindow()
{
	if (fParent)
		return fParent->GetTopLevelWindow();
	else
		return NULL;
}


void TWindow::SetBounds(const TRect& bounds)
{
	if (fWindow)
		XMoveResizeWindow(sDisplay, fWindow, bounds.left, bounds.top, bounds.GetWidth(), bounds.GetHeight());
	
	// do the notification for child windows or unmapped windows.
	if ((!fWindow || fParent) && bounds != fBounds)
	{
		TRect oldBounds(fBounds);

		fBounds = bounds;
		NotifyBoundsChanged(oldBounds);
	}
} 


void TWindow::LocalToRoot(TPoint& point) const
{
	int x, y;
	Window childWindow;
	XTranslateCoordinates(sDisplay, fWindow, gApplication->GetRootWindow(),
										   0, 0, &x, &y, &childWindow);

	const TPoint& scroll = GetScroll();
	point.Offset(x - scroll.h, y - scroll.v);
}


void TWindow::LocalToRoot(TRect& rect) const
{
	int x, y;
	Window childWindow;
	XTranslateCoordinates(sDisplay, fWindow, gApplication->GetRootWindow(),
										   0, 0, &x, &y, &childWindow);

	const TPoint& scroll = GetScroll();
	rect.Offset(x - scroll.h, y - scroll.v);
}


void TWindow::LocalToTopLevel(TPoint& point) const
{
	const TWindow* window = this;
	
	const TPoint& scroll = GetScroll();
	point.Offset(-scroll.h, -scroll.v);

	while (window->fParent)
	{
		point.Offset(window->fBounds.left, window->fBounds.top);
		window = window->fParent;
	}
}


void TWindow::LocalToTopLevel(TRect& rect) const
{
	const TWindow* window = this;
	
	const TPoint& scroll = GetScroll();
	rect.Offset(-scroll.h, -scroll.v);

	while (window->fParent)
	{
		rect.Offset(window->fBounds.left, window->fBounds.top);
		window = window->fParent;
	}
}


void TWindow::GetRootBounds(TRect& bounds) const
{
	GetLocalBounds(bounds);
	LocalToRoot(bounds);
}


void TWindow::SetBounds(TCoord left, TCoord top, TCoord right, TCoord bottom)
{
	SetBounds(TRect(left, top, right, bottom));
}



bool TWindow::IsTargetable() const
{
	return false;
}


bool TWindow::IsIconified()
{
	Atom type;
	unsigned long items, length;
	int actualFormat;
	unsigned char*	data;

	XGetWindowProperty(sDisplay, fWindow, sStateAtom, 0, 1, false, AnyPropertyType,
									&type, &actualFormat, &items, &length, &data);
	if (items == 0)
		return false;

//	ASSERT(items >= 1);
	ASSERT(type == sStateAtom);
	ASSERT(actualFormat == 32);

	bool result = (*((long *)data) == IconicState);
	XFree(data);
	return result;
}


void TWindow::RequestTarget()
{
	ASSERT(IsTargetable());

	TTopLevelWindow* window = GetTopLevelWindow();
	if (window)
		window->SetTarget(this);
}


void TWindow::Redraw()
{
	if (IsCreated())
	{
		TRect	r(0, 0, GetWidth(), GetHeight());
		const TPoint& scroll = GetScroll();
		r.Offset(scroll.h, scroll.v);
		RedrawRect(r);
	}
}


void TWindow::RedrawRect(const TRect& r)
{
	TPoint scroll = GetScroll();
	TRect clipRect(r);
	clipRect.Offset(-scroll.h, -scroll.v);
	
	TRegion	clipRegion(clipRect);
	Draw(&clipRegion);
}


void TWindow::AddChild(TWindow* child)
{
	fChildren.Insert(child);
}


void TWindow::RemoveChild(TWindow* child)
{
	fChildren.Remove(child);
}


void TWindow::AddWindow(TWindow* window)
{
//	sWindowListLock.Acquire();
	sWindowList.Insert(window);
//	sWindowListLock.Release();
}


void TWindow::RemoveWindow(TWindow* window)
{
//	sWindowListLock.Acquire();
	sWindowList.Remove(window);
//	sWindowListLock.Release();
}


TWindow* TWindow::GetWindow(Window window)
{
//	sWindowListLock.Acquire();
	TWindow* result = sWindowList.Search((void *)window);
//	sWindowListLock.Release();

	return result;
}


void TWindow::DoSetupMenu(TMenu* menu)
{
	menu->EnableCommand(kCloseCommandID);

	TCommandHandler::DoSetupMenu(menu);
}


int	TWindow::CompareProc(const void* item1, const void* item2, void* compareData)
{
	TWindow* window1 = (TWindow *)item1;
	TWindow* window2 = (TWindow *)item2;
	ASSERT(window1->fWindow);
	ASSERT(window2->fWindow);

	return (window1->fWindow - window2->fWindow);
}


int TWindow::SearchProc(const void* item, const void* key, void* compareData)
{
	TWindow* window = (TWindow *)item;
	ASSERT(window->fWindow);
	
	return window->fWindow - (Window)key;
}
