// ========================================================================================
//	TDrawContext.h			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TDrawContext__
#define __TDrawContext__

#include "TGeometry.h"
#include "TColor.h"
#include "TPixmap.h"

#include <X11/Xlib.h>

class TColor;
class TImage;
class TFont;
class TRegion;


enum TPenMode
{
	kCopyMode = GXcopy,
	kOrMode = GXor,
	kAndMode = GXand,
	kXorMode = GXxor,
	kInvertMode = GXinvert
};

enum TTextAlign
{
	kTextAlignLeft,
	kTextAlignCenter,
	kTextAlignRight
};

class TDrawContext
{
public:
	
							TDrawContext(TDrawable* drawable, TRegion* clip = NULL);
	virtual					~TDrawContext();
	
	void					DrawText(const TChar* text, int length = 0, bool movePen = false);
	void					DrawTextBox(const TChar* text, int length, const TRect& box, TTextAlign align);
	TCoord					MeasureText(const TChar* text, int length = 0) const;
	TCoord					MeasureText(const TChar* text, int length, TCoord& ascent, TCoord& height) const;

	void					PaintRect(const TRect& r);
	void					EraseRect(const TRect& r);
	void					FrameRect(const TRect& r);
	void					InvertRect(const TRect& r);
	void					CopyRect(const TDrawable* src, const TRect& srcRect, const TPoint& dest, bool applyScroll = true);
	void					DrawPixmap(const TPixmap* pixmap, const TPoint& dest);
	void					DrawImage(const TImage* image, const TPoint& dest);

	void					DrawLine(TCoord h1, TCoord y1, TCoord h2, TCoord y2);
	
	inline GC				GetGC() const { return fGC; }

	// color functions
	inline TColor&			GetForeColor() { return fForeColor; }
	inline TColor&			GetBackColor() { return fBackColor; }
	void					SetForeColor(TColor& foreColor);
	void					SetBackColor(TColor& color);
	
	void					SetTile(TPixmap* tile);
	void					SetStipple(TPixmap* stipple);
	
	inline int				GetDepth() const { return fDrawable->GetDepth(); }

	// font functions
	inline TFont*			GetFont() const { return fFont; }
	void					SetFont(TFont* font);

	// pen functions
	inline void				MoveTo(TCoord h, TCoord v) { fPen.h = h; fPen.v = v; }
	inline void				MoveTo(const TPoint& p) { fPen.h = p.h; fPen.v = p.v; }
	inline void				Move(TCoord h, TCoord v) { fPen.h += h; fPen.v += v; }
	inline const TPoint&	GetPen() const { return fPen; }

	inline TPenMode			GetPenMode() const { return fPenMode; }
	void					SetPenMode(TPenMode penMode);

	void					ClipSubWindows(bool clip);

	static void				Initialize(Display* display);
	inline static TFont*	GetDefaultFont() { return sDefaultFont; }

protected:
	void					Initialize(TDrawable* drawable);		

protected:
	GC						fGC;
	TDrawable*				fDrawable;
	TPoint					fPen;
	const TPoint&			fScroll;
	TPenMode				fPenMode;
	TFont*					fFont;
	TColor					fForeColor;
	TColor					fBackColor;

	static Display*			sDisplay;
	static TFont*			sDefaultFont;
};

#endif // __TDrawContext__
