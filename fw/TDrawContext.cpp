// ========================================================================================
//	TDrawContext.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TDrawContext.h"
#include "TDrawable.h"
#include "TApplication.h"
#include "TFont.h"
#include "TImage.h"
#include "TRegion.h"

#include "intl.h"

#include <string.h>
#include <stdlib.h>

Display*	TDrawContext::sDisplay = 0;
TFont*		TDrawContext::sDefaultFont = NULL;


TDrawContext::TDrawContext(TDrawable* drawable, TRegion* clip)
	:	fGC(0),
		fDrawable(drawable),
		fScroll(drawable->GetScroll()),
		fFont(NULL),
		fForeColor(kBlackColor),
		fBackColor(kWhiteColor)
{
	Initialize(drawable);

	if (clip)
	{
		// clip is in unscrolled coordinates
		XSetRegion(sDisplay, fGC, clip->GetRegion());
	}
}


void TDrawContext::Initialize(TDrawable* drawable)
{
	int screen = gApplication->GetDefaultScreen();
	XGCValues	gcv;
	memset(&gcv, 0, sizeof(gcv));

	Drawable Xdrawable =  fDrawable->GetDrawable();
	ASSERT(Xdrawable);
	fGC = XCreateGC(sDisplay, Xdrawable, 0, &gcv);
	XSetForeground(sDisplay, fGC, WhitePixel(sDisplay, screen));
	XSetBackground(sDisplay, fGC, BlackPixel(sDisplay, screen));

	SetFont(sDefaultFont);

	drawable->InitDrawContext(*this);
}		


	
TDrawContext::~TDrawContext()
{
	if (fGC)
		XFreeGC(gApplication->GetDisplay(), fGC);
}
	

void TDrawContext::DrawText(const TChar* text, int length, bool movePen)
{
	if (length == 0)
		length = Tstrlen(text);

	const TChar* start = text;
	TCoord leftOffset = 0;
	
	// avoid invalid characters
	while (length > 0)
	{
		int charLength = mblen(text, length);
		if (charLength > 0)
		{
			text += charLength;
			length -= charLength;
		}
		else
		{
			XmbDrawImageString(sDisplay, fDrawable->GetDrawable(), fFont->GetFontSet(), fGC, fPen.h - fScroll.h + leftOffset, fPen.v - fScroll.v, (const char *)start, text - start);
			leftOffset += XmbTextExtents(fFont->GetFontSet(), (const char *)start, text - start, NULL, NULL);
			leftOffset += XmbTextExtents(fFont->GetFontSet(), " ", 1, NULL, NULL);
			text += 1;
			start = text;
			length -= 1;
		}
	}
	
	if (text > start)
	{
		XmbDrawImageString(sDisplay, fDrawable->GetDrawable(), fFont->GetFontSet(), fGC, fPen.h - fScroll.h + leftOffset, fPen.v - fScroll.v, (const char *)start, text - start);
		if (movePen)
		{
			leftOffset += XmbTextExtents(fFont->GetFontSet(), (const char *)start, text - start, NULL, NULL);
		}
	}

	if (movePen)
		fPen.h += leftOffset;
}


void TDrawContext::DrawTextBox(const TChar* text, int length, const TRect& box, TTextAlign align)
{
	if (text && length)
	{
		TCoord height, ascent;
		TCoord width = fFont->MeasureText(text, length, ascent, height);

		TCoord h = box.left;
		TCoord extraH = (align == kTextAlignLeft ? 0 : (align == kTextAlignRight ? box.GetWidth() - width : (box.GetWidth() - width) / 2));
		if (extraH > 0)
			h += extraH;

		TCoord v = box.top;
		TCoord extraV = (box.GetHeight() - height) / 2;
		if (extraV > 0)
			v += extraV;

		{
			const TChar* start = text;
			TCoord leftOffset = 0;
			
			// avoid invalid characters
			while (length > 0)
			{
				int charLength = mblen(text, length);
				if (charLength > 0)
				{
					text += charLength;
					length -= charLength;
				}
				else
				{
					XmbDrawImageString(sDisplay, fDrawable->GetDrawable(), fFont->GetFontSet(), fGC, h - fScroll.h + leftOffset, v + ascent - fScroll.v, (const char *)start, text - start);
					leftOffset += XmbTextExtents(fFont->GetFontSet(), (const char *)start, text - start, NULL, NULL);
					leftOffset += XmbTextExtents(fFont->GetFontSet(), " ", 1, NULL, NULL);
					text += 1;
					start = text;
					length -= 1;
				}
			}
			
			if (text > start)
			{
				XmbDrawImageString(sDisplay, fDrawable->GetDrawable(), fFont->GetFontSet(), fGC, h - fScroll.h + leftOffset, v + ascent - fScroll.v, (const char *)start, text - start);
			}
		}
		
		// paint border
		TRect innerBox(h, v, h + width, v + height);
		TRect	r;

		if (innerBox.top > box.top)
		{
			r.Set(box.left, box.top, box.right, innerBox.top);
			EraseRect(r);
		}
		if (innerBox.left > box.left)
		{
			r.Set(box.left, innerBox.top, innerBox.left, innerBox.bottom);
			EraseRect(r);
		}
		if (innerBox.right < box.right)
		{
			r.Set(innerBox.right, innerBox.top, box.right, innerBox.bottom);
			EraseRect(r);
		}
		if (innerBox.bottom < box.bottom)
		{
			r.Set(box.left, innerBox.bottom, box.right, box.bottom);
			EraseRect(r);
		}
	}
	else
		EraseRect(box);
}


TCoord TDrawContext::MeasureText(const TChar* text, int length) const
{
	ASSERT(fFont);
	return fFont->MeasureText(text, length);
} 


TCoord TDrawContext::MeasureText(const TChar* text, int length, TCoord& ascent, TCoord& height) const
{
	ASSERT(fFont);
	return fFont->MeasureText(text, length, ascent, height);
}


void TDrawContext::PaintRect(const TRect& r)
{
	if (!r.IsEmpty())
		XFillRectangle(sDisplay, fDrawable->GetDrawable(), fGC, r.left - fScroll.h, r.top - fScroll.v, r.GetWidth(), r.GetHeight());
}


void TDrawContext::EraseRect(const TRect& r)
{
	if (!r.IsEmpty())
	{
		TColor savedColor = fForeColor;
		SetForeColor(fBackColor);
		XFillRectangle(sDisplay, fDrawable->GetDrawable(), fGC, r.left - fScroll.h, r.top - fScroll.v, r.GetWidth(), r.GetHeight());
		SetForeColor(savedColor);
	}
}


void TDrawContext::FrameRect(const TRect& r)
{
	if (!r.IsEmpty())
		XDrawRectangle(sDisplay, fDrawable->GetDrawable(), fGC, r.left - fScroll.h, r.top - fScroll.v, r.GetWidth(), r.GetHeight());
}


void TDrawContext::InvertRect(const TRect& r)
{
	if (!r.IsEmpty())
	{
		XSetFunction(sDisplay, fGC, GXinvert);		
		XFillRectangle(sDisplay, fDrawable->GetDrawable(), fGC, r.left - fScroll.h, r.top - fScroll.v, r.GetWidth(), r.GetHeight());
		XSetFunction(sDisplay, fGC, fPenMode);
	}		
}


void TDrawContext::CopyRect(const TDrawable* src, const TRect& srcRect, const TPoint& dest, bool applyScroll)
{
	if (!srcRect.IsEmpty())
	{
		if (applyScroll)
		{
			XCopyArea(sDisplay, src->GetDrawable(), fDrawable->GetDrawable(), fGC, 
						srcRect.left, srcRect.top, 
						srcRect.GetWidth(), srcRect.GetHeight(),
						dest.h - fScroll.h, dest.v - fScroll.v);
		}
		else
		{
			XCopyArea(sDisplay, src->GetDrawable(), fDrawable->GetDrawable(), fGC, 
						srcRect.left, srcRect.top, 
						srcRect.GetWidth(), srcRect.GetHeight(),
						dest.h, dest.v);
		}
	}
}


void TDrawContext::DrawPixmap(const TPixmap* pixmap, const TPoint& dest)
{
	const TRect& srcRect = pixmap->GetBounds();

	if (!srcRect.IsEmpty())
	{
		TCoord h = dest.h - fScroll.h;
		TCoord v = dest.v - fScroll.v;
		
		XSetClipMask(sDisplay, fGC, pixmap->GetMask());
		XSetClipOrigin(sDisplay, fGC, h, v);
		
		XCopyArea(sDisplay, pixmap->GetDrawable(), fDrawable->GetDrawable(), fGC, 
					srcRect.left, srcRect.top, 
					srcRect.GetWidth(), srcRect.GetHeight(),
					h, v);
		XSetClipMask(sDisplay, fGC, None);
	}
}


void TDrawContext::DrawImage(const TImage* image, const TPoint& dest)
{
	TCoord h = dest.h - fScroll.h;
	TCoord v = dest.v - fScroll.v;

#ifdef HAVE_XSHM
	if (image->UsingSharedMemory())
		XShmPutImage(sDisplay, fDrawable->GetDrawable(), fGC, image->GetImage(), 0, 0, h, v, image->GetWidth(), image->GetHeight(), false);
	else
#endif
		XPutImage(sDisplay, fDrawable->GetDrawable(), fGC, image->GetImage(), 0, 0, h, v, image->GetWidth(), image->GetHeight());
}


void TDrawContext::DrawLine(TCoord h1, TCoord v1, TCoord h2, TCoord v2)
{
	XDrawLine(sDisplay, fDrawable->GetDrawable(), fGC, h1 - fScroll.h, v1 - fScroll.v, h2 - fScroll.h, v2 - fScroll.v);
}


void TDrawContext::SetForeColor(TColor& color)
{
	fForeColor = color;
	
	XSetForeground(sDisplay, fGC, color.GetPixel());
}


void TDrawContext::SetBackColor(TColor& color)
{
	fBackColor = color;
	
	XSetBackground(sDisplay, fGC, color.GetPixel());
}


void TDrawContext::SetTile(TPixmap* tile)
{
	XSetFillStyle(sDisplay, fGC, (tile ? FillTiled : FillSolid));
	XSetTile(sDisplay, fGC, (tile ? tile->GetPixmap() : None));
}


void TDrawContext::SetStipple(TPixmap* stipple)
{
	XSetFillStyle(sDisplay, fGC, (stipple ? FillOpaqueStippled : FillSolid));
	XSetStipple(sDisplay, fGC, (stipple ? stipple->GetPixmap() : None));
}


void TDrawContext::SetFont(TFont* font)
{
	ASSERT(font);
	
	if (fFont)
		fFont->RemoveRef();
		
	fFont = font;
	
	font->AddRef();
}


void TDrawContext::SetPenMode(TPenMode penMode)
{
	fPenMode = penMode;
	XSetFunction(sDisplay, fGC, penMode);
}


void TDrawContext::ClipSubWindows(bool clip)
{
	XGCValues	values;

	values.subwindow_mode = (clip ? ClipByChildren : IncludeInferiors);
	XChangeGC(sDisplay, fGC, GCSubwindowMode, &values);
}


void TDrawContext::Initialize(Display* display)
{
	sDisplay = display;
	sDefaultFont = new TFont(_("-*-*-bold-r-normal-*-13-*-*-*-*-*-*-*"));
	sDefaultFont->AddRef();
}
