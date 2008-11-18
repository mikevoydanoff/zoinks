// ========================================================================================
//	TWindow.h		 			Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TWindow__
#define __TWindow__

#include "TCommandHandler.h"
#include "TDrawable.h"
#include "TList.h"
#include "TString.h"
#include "TColor.h"
#include "TCommandID.h"

class TWindow;
class TDocumentWindow;
class TView;
class TMenu;
class TFont;
class TCursor;
class TCommandHandler;
class TInputContext;
class TRegion;
class TTopLevelWindow;


// callback for automatic repositioning of windows.
typedef void (* TWindowPositioner)(TWindow* window, 
									const TRect& oldParentBounds,
									const TRect& newParentBounds);

enum TWindowStyle
{
	kTopLevelWindow,
	kChildWindow,
	kPopupWindow
};

enum TWindowVisibility
{
	kWindowHidden,
	kWindowLatent,
	kWindowVisible
};


class TWindow : public TCommandHandler, public TDrawable
{
public:
								TWindow(TWindow* parent, const TRect& bounds, TWindowStyle style = kChildWindow);				
	
	virtual void				Create();
	virtual void				Show(bool doShow);
	void						Raise();
	virtual void				Close();
	void						Destroy();

	inline TColor&				GetForeColor() { return fForeColor; }					
	inline void					SetForeColor(TColor& color) { fForeColor = color; }					
	inline TColor&				GetBackColor() { return fBackColor; }					
	void						SetBackColor(TColor& color);					
	
	virtual TFont*				GetFont();

	void						Redraw();
	virtual void				RedrawRect(const TRect& r);

	inline const TRect&			GetBounds() const { return fBounds; }
	inline TCoord				GetWidth() const { return fBounds.GetWidth(); }
	inline TCoord				GetHeight() const { return fBounds.GetHeight(); }
	inline void					GetLocalBounds(TRect& bounds) const { bounds.Set(0, 0, fBounds.GetWidth(), fBounds.GetHeight()); }
	void						SetBounds(const TRect& bounds);
	void						SetBounds(TCoord left, TCoord top, TCoord right, TCoord bottom);

	void						LocalToRoot(TPoint& point) const;
	void						LocalToRoot(TRect& rect) const;
	void						GetRootBounds(TRect& bounds) const;

	void						LocalToTopLevel(TPoint& point) const;
	void						LocalToTopLevel(TRect& rect) const;
	
	inline bool					IsCreated() const { return (fWindow != 0); }
	inline bool					IsVisible() const { return fMapped; }	// does not return true until window is mapped
	inline TWindowVisibility	GetVisibility() const { return fVisibility; }
	bool						IsIconified();

	virtual bool				IsTargetable() const;
	void						RequestTarget();
	
	inline void					SetWindowPositioner(TWindowPositioner positioner) { fPositioner = positioner; }

	virtual void				DoSetupMenu(TMenu* menu);
	virtual bool				DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	inline Window				GetXWindow() const { return fWindow; }
	inline TWindow*				GetParent() const { return fParent; }
	virtual void				SetParent(TWindow* parent);

	TWindow*					GetFirstSubWindow() const;
	TWindow*					GetLastSubWindow() const;
	TWindow*					GetPreviousSibling() const;
	TWindow*					GetNextSibling() const;

	virtual void				AdjustScrollBars(TView* view);
	virtual void				ChildScrollChanged(TView* child, TCoord deltaH, TCoord deltaV);	
	
	void						SetCursor(TCursor* cursor);
	static inline TCursor*		GetDefaultCursor() { return sDefaultCursor; }

	inline bool					HasFocus() const { return fHasFocus; }
	inline void					SetHasFocus(bool hasFocus) { fHasFocus = hasFocus; }
	
	void 						SetTarget();

	void						GrabPointer(Time time = CurrentTime);
	void						UngrabPointer(Time time = CurrentTime);
	inline bool					HasPointerGrab() const { return sPointerGrabWindow == this; }

	virtual void				HandleEvent(XEvent& event);

	virtual TTopLevelWindow*	GetTopLevelWindow();

	static TWindow*				GetWindow(Window window);
	static void					Initialize(Display* display);

	inline static Display*		GetDisplay() { return sDisplay; }

	inline int					GetClickCount() const { return fClickCount; }

	inline int					GetBorder() const { return fBorder; }
	void						SetBorder(int border);
	
	inline Time					GetCurrentEventTime() const { return fCurrentEventTime; }
	
	inline TInputContext*		GetInputContext() const { return fInputContext; }
	
	void						SimulateMouseMoved();	// for TMouseTrackingIdler

	void						Update();
	static void					ProcessUpdates();

protected:
	void						SetBackgroundPixel(TColor* color); // NULL for transparent

	virtual void				Draw(TRegion* clip);
	virtual void				DoClose();
	virtual void				DoDestroy();

	virtual void				DoMapped(bool mapped);
	virtual void				ParentShown(bool doShow);
	
	virtual void				DoScrollWheel(bool down);
	
	bool						CreateInputContext();

	// clipboard support
	void						RequestSelection(Atom type);
	bool						BecomeSelectionOwner();
	void						RelinquishSelectionOwnership();
	static TWindow*				GetSelectionOwner();
	inline bool					IsSelectionOwner() const { return fIsSelectionOwner; }

	virtual bool				RequestSelectionData(Atom& inOutType, unsigned char*& outData, uint32& outLength);
	virtual void				ReceiveSelectionData(Atom type, const unsigned char* data, uint32 length);
	virtual void				LostSelectionOwnership();

	virtual Drawable			GetDrawable() const;
	virtual void			 	InitDrawContext(TDrawContext& context);

	virtual						~TWindow();
	
	virtual void				AddChild(TWindow* child);
	virtual void				RemoveChild(TWindow* child);

	virtual void				NotifyBoundsChanged(const TRect& oldBounds);
	virtual void				ParentBoundsChanged(const TRect& oldParentBounds, const TRect& newParentBounds);

	static void					AddWindow(TWindow* window);
	static void					RemoveWindow(TWindow* window);

	friend class TTopLevelWindow;

protected:
	Window						fWindow;
	TWindow*					fParent;
	TColor						fForeColor;
	TColor						fBackColor;
	int							fBorder;
	TList<TWindow>				fChildren;
	TWindowVisibility			fVisibility;
	TRegion*					fUpdateRegion;
	TInputContext*				fInputContext;
	Time						fCurrentEventTime;		// time of the event being processed
	Time						fSelectionAcquiredTime;	// time we got the selection
	bool						fIsSelectionOwner;
	
	TWindowStyle				fStyle;
	TWindowPositioner			fPositioner;
	bool	 					fMapped;
	bool						fObscured;				// true if visibility state is not VisibilityUnobscured
	bool						fHasFocus;
	bool						fRaiseOnMapNotify;
	bool						fSetFocusOnMapNotify;
	
	// for simulated mouse moved events
	TPoint						fLastMouseMovedLocation;
	TModifierState				fLastMouseMovedModifiers;
	
	// double click support
	TTime						fLastClickTime;
	TPoint						fLastClick;
	int							fClickCount;
	
	static int					CompareProc(const void* item1, const void* item2, void* compareData);
	static int					SearchProc(const void* item1, const void* key, void* compareData);

	static Display*				sDisplay;
	static TList<TWindow>		sWindowList;
//	static TLock				sWindowListLock;
	static TCursor*				sDefaultCursor;
	static TWindow*				sPointerGrabWindow;

	static Atom					sDeleteWindowAtom;
	static Atom					sTakeFocusAtom;
	static Atom					sStateAtom;
	static Atom					sSelectionProperty;
	
	// linked list of windows with update regions
	static TWindow*				sFirstUpdate;
	TWindow*					fNextUpdate;
};

#endif // __TWindow__
