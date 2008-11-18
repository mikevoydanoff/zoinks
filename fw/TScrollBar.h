// ========================================================================================
//	TScrollBar.h			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TScrollBar__
#define __TScrollBar__

#include "TWindow.h"
#include "TIdler.h"

class TDrawContext;
class TPixmap;

enum ScrollBarPart
{
	kNone,
	kArrow1,
	kArrow2,
	kThumb,
	kPageUp,
	kPageDown
};


class TScrollBar : public TWindow, public TIdler
{
protected:
	enum ArrowPixmap
	{
		kLeftArrow,
		kRightArrow,
		kUpArrow,
		kDownArrow
	};

public:
	enum Orientation
	{
		kHorizontal,
		kVertical
	};
	
public:
								TScrollBar(TWindow* parent, const TRect& bounds,
										   Orientation orientation, bool proportionalThumbs);
							
	virtual 					~TScrollBar();

	inline int32				GetMinimum() const { return fMinimum; }
	inline int32				GetMaximum() const { return fMaximum; }
	inline int32				GetValue() const { return fValue; }

	void						SetRange(int32 minimum, int32 maximum);
	void						SetValue(int32 value);

	void						ScrollDown();
	void						ScrollUp();
	void						PageDown();
	void						PageUp();


	inline Orientation			GetOrientation() const { return fOrientation; }
	inline bool					IsVertical() const { return (fOrientation == kVertical); }
	inline bool					IsHorizontal() const { return (fOrientation == kHorizontal); }

protected:
	virtual void				Draw(TRegion* clip);
	virtual void				DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				DoMouseMoved(const TPoint& point, TModifierState state);
	virtual void				DoIdle();

	void						GetArrow1(TRect& r) const;
	void						GetArrow2(TRect& r) const;
	void						GetThumbArea(TRect& r) const;
	void						GetThumb(TRect& r) const;
	ScrollBarPart				IdentifyPoint(const TPoint& point, TRect& outTrackingRect) const;

	void						DrawArrow1(TDrawContext& context, bool pressed);
	void						DrawArrow2(TDrawContext& context, bool pressed);
	void						DrawThumb(TDrawContext& context);

	static TPixmap*				GetArrowPixmap(ArrowPixmap pixmap);

protected:
	int32						fMinimum;
	int32						fMaximum;
	int32						fValue;
	Orientation					fOrientation;

	TPoint						fLastPoint;
	ScrollBarPart				fTrackingPart;
	TRect						fTrackingRect;
	bool						fInTrackingRect;
	bool						fProportionalThumbs;

	static TPixmap*				fArrowPixmaps[4];
	static char**				fArrowPixmapData[4];
};

#endif // __TScrollBar__
