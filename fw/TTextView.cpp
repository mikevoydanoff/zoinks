// ========================================================================================
//	TTextView.cpp			 	Copyright (C) 2001-2009 Mike Lockwood. All rights reserved.
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

#include "TTextView.h"
#include "TTextLayout.h"
#include "TDrawContext.h"
#include "TFont.h"
#include "TFWCursors.h"
#include "TMenu.h"
#include "TRegion.h"
#include "TCursor.h"
#include "TApplication.h"
#include "TClipboard.h"
#include "TCommonDialogs.h"
#include "TNumericEntryBehavior.h"
#include "TTopLevelWindow.h"
#include "TInputContext.h"

#include "intl.h"

#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

/*
 Todo:

 Make sure fUpDownHorizOffset doesn't split a character
*/

const TTime		kCursorBlinkTime = 500;

const TCoord	kLeftInset = 3;
const TCoord	kTopInset = 3;
const TCoord	kRightInset = 2;
const TCoord	kBottomInset = 2;

const int 		kTabWidth = 4;


TCursor* TTextView::sCursor = NULL;


TTextView::TTextView(TWindow* parent, const TRect& bounds, TFont* font, bool modifiable, bool multiLine)	
	:	TView(parent, bounds),
		fLayout(NULL),
		fFont(font),
		fInset(kLeftInset, kTopInset, kRightInset, kBottomInset),
		fSelectionStart(0),
		fSelectionEnd(0),
		fSelectionAnchor(0),
		fSelectionAnchorEnd(0),
		fTrackingClickCount(0),
		fInsertionPointOn(true),
		fUpDownHorizOffset(-1),
		fSpacesPerTab(0),
		fAutoIndent(true),
		fUndoRedoIndex(-1),
		fSavedUndoRedoIndex(-1),
		fAccumulateTyping(false),
		fAccumulateDeletion(false),
		fModifiable(modifiable),
		fMultiLine(multiLine),
		fLineWrap(false),
		fFilterTabAndCR(false),
		fHideInsertionPointWhenNotTarget(false),
		fCursorHidden(false),
		fMouseTrackingIdler(NULL)
{
	ASSERT(font);
	font->AddRef();

	SetIdleFrequency(kCursorBlinkTime);

	// assume tab is 4 spaces wide by default
	int tabLength = (fSpacesPerTab > 0 ? fSpacesPerTab : 4);
	fLayout = new TTextLayout(font, TPoint(fInset.left, fInset.top), tabLength, multiLine, fLineWrap);
	ComputeContentSize();
}

			
TTextView::~TTextView()
{
	ClearUndoRedo();
	
	fFont->RemoveRef();
	delete fLayout;
	delete fMouseTrackingIdler;
}


void TTextView::Create()
{
	TView::Create();

	if (! sCursor)
		sCursor = new TCursor(XC_xterm);
	SetCursor(sCursor);

	if (fModifiable && HasFocus())
		EnableIdling(true);
	
	if (fModifiable && CreateInputContext())
		SetIMLocation();
}


TFont* TTextView::GetFont()
{
	return fFont;
}


void TTextView::SetText(TChar* text, STextOffset length, bool ownsData)
{
	if (length == 0)
		length = Tstrlen(text);

	fLayout->SetText(text, length, ownsData);

	if (length < fSelectionEnd)
		fSelectionStart = fSelectionEnd = 0;

	ComputeContentSize();
	if (IsCreated())
		Redraw();
}


void TTextView::Draw(TRegion* clip)
{
	DoDraw(FirstVisibleLine(), LastVisibleLine(), clip);
}


void TTextView::RedrawRect(const TRect& r)
{
	TPoint scroll = GetScroll();
	TRect clipRect(r);
	clipRect.Offset(-scroll.h, -scroll.v);
	
	TRegion	clipRegion(clipRect);

	// draw outside our content area
	TView::Draw(&clipRegion);
	
	uint32 startLine = fLayout->VertOffsetToLine(r.top);
	if (startLine > 0)
		startLine--;
	uint32 endLine = fLayout->VertOffsetToLine(r.bottom);
	
	DoDraw(startLine, endLine, &clipRegion);
}


void TTextView::DoDraw(uint32 startLine, uint32 endLine, TRegion* clip)
{
	TDrawContext	context(this, clip);

	DrawTextRange(context, fLayout->LineToOffset(startLine), fLayout->LineToOffset(endLine) + 1, clip);
	DrawInsertionPoint(context);

	// special case empty text - need to erase a one pixel line
	if (GetTextLength() == 0)
	{
		TRect	r;
		fLayout->GetLineBounds(0, r);
		if (r.left == r.right)
			r.right++;
		context.EraseRect(r);
	}
}


void TTextView::SetContentSize(const TPoint& contentSize)
{
	TPoint oldContentSize(fContentSize);
	
	TView::SetContentSize(contentSize);


	// since we draw selection hilite outside of the content, we need to do some extra erasing
	// we also need to erase part of the inset area
	if (IsVisible() && oldContentSize.v > fContentSize.v)
	{
		TDrawContext context(this);
		// erase right area
		TRect r(fContentSize.h, fContentSize.v - fInset.bottom, GetWidth() + fScroll.h, oldContentSize.v);
		context.EraseRect(r);
		
		// erase inset
		r.Set(fInset.left - 1, fContentSize.v - fInset.bottom, (fContentSize.h > oldContentSize.h ? fContentSize.h : oldContentSize.h), fContentSize.v);
		context.EraseRect(r);
	}
}


void TTextView::EraseContentDifference(const TPoint& oldContentSize, const TPoint& newContentSize)
{
	if (oldContentSize.h > newContentSize.h || oldContentSize.v > newContentSize.v)
	{
		TDrawContext context(this);
		
		if (oldContentSize.h > newContentSize.h)
		{
			TRect	r(newContentSize.h, 0, oldContentSize.h, (oldContentSize.v > newContentSize.v ? oldContentSize.v : newContentSize.v));
			context.EraseRect(r);
		}
		if (oldContentSize.v > newContentSize.v)
		{
			TRect	r(fInset.left - 1, newContentSize.v, (oldContentSize.h > newContentSize.h ? oldContentSize.h : newContentSize.h), oldContentSize.v);
			context.EraseRect(r);
		}
	}
}


void TTextView::GetScrollableBounds(TRect& bounds) const
{
	GetLocalBounds(bounds);
	bounds.Inset(fInset);
	bounds.left -= 1;	// for the insertion point
}


void TTextView::NotifyBoundsChanged(const TRect& oldBounds)
{
	TView::NotifyBoundsChanged(oldBounds);
	
	if (fLineWrap)
	{
		TCoord newWidth = GetWidth();
		
		if (newWidth != oldBounds.GetWidth())
		{
			fLayout->SetWidth(newWidth - fInset.left - fInset.right);
			ComputeContentSize();
		}
	}
}


void TTextView::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	ASSERT(fLayout);
	
	if (fMouseTrackingIdler)
	{
		delete fMouseTrackingIdler;
		fMouseTrackingIdler = NULL;
	}
	
	if (button == kLeftButton)
	{
		int clickCount = GetClickCount();
		fTrackingClickCount = clickCount;

		if (clickCount == 1)
		{
			STextOffset offset = fLayout->PointToOffset(point);

			fUpDownHorizOffset = -1;

			if (state & ShiftMask)
			{
				if (offset < fSelectionStart)
				{
					SetSelection(offset, fSelectionEnd);
					fSelectionAnchor = fSelectionEnd;
				}
				else
				{
					SetSelection(fSelectionStart, offset);
					fSelectionAnchor = fSelectionStart;
				}
			}
			else
			{
				SetSelection(offset);
				fSelectionAnchor = offset;
			}

			ASSERT(fMouseTrackingIdler == NULL);
			fMouseTrackingIdler = new TMouseTrackingIdler(this);
		}
		else if (clickCount == 2)
		{
			// select word
			
			// don't round up, so we know which character was clicked on
			STextOffset offset = fLayout->PointToOffset(point, false);

			STextOffset start, end;
			if (fLayout->BalanceCharacter(offset, start, end) || fLayout->FindWord(offset, start, end))
			{
				SetSelection(start, end);
				fSelectionAnchor = start;
				fSelectionAnchorEnd = end;
			}	
		}
		else if (clickCount == 3)
		{
			// select entire line
			STextOffset offset = fLayout->PointToOffset(point);
			uint32 line = fLayout->OffsetToLine(offset);
			STextOffset start = fLayout->LineToOffset(line);
			STextOffset end = fLayout->LineToOffset(line + 1);
			SetSelection(start, end);
			fSelectionAnchor = start;
			fSelectionAnchorEnd = end;
		}		
	}
	else if (button == kMiddleButton && fModifiable)
	{
		STextOffset location = fLayout->PointToOffset(point);
		
		TWindow* window = GetSelectionOwner();
		TTextView* selectionTextView = dynamic_cast<TTextView*>(window);
		
		if (selectionTextView == this/* || fLastSelection.GetLength() > 0*/)
		{
			// special case copying within the view

			if (fSelectionEnd > fSelectionStart)
				GetSelectedText(fLastSelection);
			// else use previous selection
							
			if (fLastSelection.GetLength() > 0)
			{
				// make sure line endings are converted if necessary
				TLineEndingFormat format;
				if (fLastSelection.GetLineEndingFormat(format) && format != fLayout->GetLineEndingFormat())
					fLastSelection.SetLineEndingFormat(fLayout->GetLineEndingFormat());

				InsertText(location, fLastSelection, fLastSelection.GetLength());
				ScrollSelectionIntoView(true);
				
				PastedText();
			}
		}
		else if (selectionTextView)
		{
			TString	text;
			selectionTextView->GetSelectedText(text);

			// make sure line endings are converted if necessary
			TLineEndingFormat format;
			if (text.GetLineEndingFormat(format) && format != fLayout->GetLineEndingFormat())
				text.SetLineEndingFormat(fLayout->GetLineEndingFormat());

			InsertText(location, text, text.GetLength());
			ScrollSelectionIntoView();

			PastedText();
		}
		else
		{
			// request the selection from the client that owns it
			fMouseCopyLocation = location;
			RequestSelection(XA_STRING);
		}
	}
}


void TTextView::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton)
	{
		fTrackingClickCount = 0;
		
		if (fMouseTrackingIdler)
		{
			delete fMouseTrackingIdler;
			fMouseTrackingIdler = NULL;
		}
	}
}


void TTextView::DoMouseMoved(const TPoint& point, TModifierState state)
{
	if (fCursorHidden)
	{
		SetCursor(sCursor);
		fCursorHidden = false;
	}
	
	if (fTrackingClickCount > 0)
	{
		STextOffset	selectionStart = fSelectionStart;		// initialize to default values in case we don't want to change selection.
		STextOffset	selectionEnd = fSelectionEnd;
		STextOffset offset = fLayout->PointToOffset(point);
		
		if (fTrackingClickCount == 1)
		{
			if (offset < fSelectionAnchor)
			{
				selectionStart = offset;
				selectionEnd = fSelectionAnchor;
			}
			else
			{
				selectionStart = fSelectionAnchor;
				selectionEnd = offset;
			}
		}
		else if (fTrackingClickCount == 2)
		{
			STextOffset wordStart, wordEnd;
			
			if (fLayout->FindWord(offset, wordStart, wordEnd))
			{
				if (offset < fSelectionAnchor)
				{
					selectionStart = wordStart;
					selectionEnd = fSelectionAnchorEnd;
				}
				else if (offset > fSelectionAnchorEnd)
				{
					selectionStart = fSelectionAnchor;
					selectionEnd = wordEnd;
				}
			}
			else
			{
				if (offset < fSelectionAnchor)
				{
					selectionStart = offset;
					selectionEnd = fSelectionAnchorEnd;
				}
				else if (offset > fSelectionAnchorEnd)
				{
					selectionStart = fSelectionAnchor;
					selectionEnd = offset;
				}
			}
		}
		else // fTrackingClickCount >= 3
		{
			if (offset < fSelectionAnchor)
			{
				selectionStart = fLayout->GetLineOffset(fLayout->OffsetToLine(offset));
				selectionEnd = fSelectionAnchorEnd;
			}
			else if (offset > fSelectionAnchorEnd)
			{
				selectionStart = fSelectionAnchor;
				
				uint32 line = fLayout->OffsetToLine(offset);
				if (line < GetLineCount() - 1)
					selectionEnd = fLayout->GetLineOffset(line + 1);
				else
					selectionEnd = GetTextLength();
			}
		}		
		
		ScrollSelectionIntoView(true);
		
		ASSERT(selectionStart <= selectionEnd);
		
		if (selectionStart != fSelectionStart ||
			selectionEnd != fSelectionEnd)
		{
			TDrawContext	context(this);

			// erase insertion point if necessary
			if (fSelectionStart == fSelectionEnd)
				HideInsertionPoint(context);
			
			STextOffset oldStart = fSelectionStart;
			STextOffset oldEnd = fSelectionEnd;

			SetSelection(selectionStart, selectionEnd, false);
	
			if (selectionStart < oldStart)
				DrawTextRange(context, selectionStart, oldStart);
			else
				DrawTextRange(context, oldStart, selectionStart);
				
			if (selectionEnd > oldEnd)
				DrawTextRange(context, oldEnd, selectionEnd);
			else
				DrawTextRange(context, selectionEnd, oldEnd);

			// draw insertion point if necessary
			if (fSelectionStart == fSelectionEnd)
				ShowInsertionPoint(context);
		}
	}
}


bool TTextView::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	// some special Emacs equivalents
	if (state == ControlMask)
	{
		if (key == 'e')
		{
			RightArrowKey(Mod1Mask);
			ScrollSelectionIntoView(true);
			return true;
		}
	}

	// ignore return in single line edit fields
	// ignore Control and Mod1 keys
	if (key == XK_Escape ||
		(fFilterTabAndCR && (key == XK_Tab || key == XK_Return || key == XK_KP_Enter)) ||
		(!fMultiLine && (key == XK_Return || key == XK_KP_Enter)) ||
		((state & (ControlMask | Mod1Mask)) && key != XK_Left && key != XK_Right && key != XK_Up && key != XK_Down))
		return TView::DoKeyDown(key, state, string);

	if (key != XK_Up && key != XK_Down)
		fUpDownHorizOffset = -1;

	switch (key)
	{
		case XK_Left:
			LeftArrowKey(state);
			ScrollSelectionIntoView(true);
			break;

		case XK_Right:
			RightArrowKey(state);
			ScrollSelectionIntoView(true);
			break;

		case XK_Up:
			UpArrowKey(state);
			ScrollSelectionIntoView(true);
			break;
			
		case XK_Down:
			DownArrowKey(state);
			ScrollSelectionIntoView(true);
			break;

		case XK_Page_Up:
			HandleCommand(this, this, kPageUpCommandID);
			break;
			
		case XK_Page_Down:
			HandleCommand(this, this, kPageDownCommandID);
			break;		
		
		case XK_Home:
			ScrollBy(-fScroll.h, -fScroll.v);
			break;
			
		case XK_End:
			ScrollBy(0, fContentSize.v);
			break;

		case XK_BackSpace:
	  	case XK_Delete:
 			if (fModifiable)
			{
				HideCursor();
				Delete((key == XK_Delete), true);
				ScrollSelectionIntoView();
			}
			break;
		
		default:
			if (fModifiable)
			{				
				if (string[0])
				{
					HideCursor();

					if (string[0] == kLineEnd10 || string[0] == kLineEnd13)
					{
						ASSERT(string[1] == 0);
						
						const TChar* lineEnding = fLayout->GetLineEndingString();
						ReplaceSelection(lineEnding, strlen(lineEnding), true, true, true);
						if (fAutoIndent && !fLineWrap)
							AutoIndent();
					}
					else if (key == XK_Tab && fSpacesPerTab > 0 && 
						(state & ShiftMask) == 0)   // insert a real tab if shift key is pressed
						InsertTabSpaces();
					else
						ReplaceSelection(string, strlen(string), true, true, true);

					ScrollSelectionIntoView();
				}
				else if ((key == XK_Tab || key == XK_ISO_Left_Tab) && state | ShiftMask)
					ReplaceSelection("\t", 1, true, true, true);
			}
	}

	return true;
}


bool TTextView::IsTargetable() const
{
	return true;
}


bool TTextView::RequestSelectionData(Atom& inOutType, unsigned char*& outData, uint32& outLength)
{
	const TChar* src = NULL;
	
	if (fSelectionStart < fSelectionEnd)
	{
		src = fLayout->GetText() + fSelectionStart;
		outLength = fSelectionEnd - fSelectionStart;
	}
	else if (fLastSelection.GetLength() > 0)
	{
		src = fLastSelection;
		outLength = fLastSelection.GetLength();
	}

	if (src && inOutType == XA_STRING)
	{
		outData = (unsigned char*)malloc(outLength);
		memcpy(outData, src, outLength);
		return true;
	}

	outData = NULL;
	outLength = 0;
	return false;
}


void TTextView::ReceiveSelectionData(Atom type, const unsigned char* data, uint32 length)
{
	if (type == XA_STRING)
	{
		// make sure line endings are converted if necessary
		TLineEndingFormat format;
		if (TString::GetLineEndingFormat((const TChar*)data, length, format) && format != fLayout->GetLineEndingFormat())
		{
			TString	temp((const TChar*)data, length);
			temp.SetLineEndingFormat(fLayout->GetLineEndingFormat());
			InsertText(fMouseCopyLocation, temp, temp.GetLength());
		}
		else
			InsertText(fMouseCopyLocation, (const TChar*)data, length);
			
		ScrollSelectionIntoView(true);

		PastedText();
	}
}


void TTextView::LostSelectionOwnership()
{
	fLastSelection.SetEmpty();
}


void TTextView::PastedText()
{
}


void TTextView::AutoIndent()
{
	ASSERT(fSelectionStart == fSelectionEnd);

	uint32 line = fLayout->OffsetToLine(fSelectionStart);
	if (line > 0)
	{
		STextOffset length;
		const TChar* prevLine = fLayout->GetLineText(line - 1, length);

		STextOffset i;
		for (i = 0; i < length; i++)
		{
			TChar	ch = prevLine[i];
			if (ch != ' ' && ch != '\t')
				break;
		}

		if (i > 0)
		{
			// make a copy of the string since ReplaceSelection will realloc the text
			TString temp(prevLine, i);
			ReplaceSelection(temp, i, true, true);
		}
	}
}


void TTextView::InsertTabSpaces()
{
	if (fSpacesPerTab > 0)
	{
		uint32 startLine = fLayout->OffsetToLine(fSelectionStart);
		STextOffset lineOffset = fLayout->LineToOffset(startLine);
		STextOffset lineLength;
		const TChar* lineText = fLayout->GetLineText(startLine, lineLength);								

		
		// count is number of characters currently before the selection
		int count = 0;
		for (STextOffset i = lineOffset; i < fSelectionStart; i++)
		{
			if (lineText[i - lineOffset] == '\t')
				count += kTabWidth;
			else
				count++;
		}
		
		int spaces = fSpacesPerTab - (count % fSpacesPerTab);
		char* spaceString = new char[spaces];
		memset(spaceString, ' ', spaces);
		ReplaceSelection(spaceString, spaces, true, true, true);
		delete[] spaceString;
	}
}


void TTextView::InsertText(STextOffset location, const TChar* text, STextOffset length, UndoType undoType)
{
	SaveUndoMouseCopy(location, length, undoType);

	uint32 redrawStart, redrawEnd;
	fLayout->ReplaceText(location, location, text, length, redrawStart, redrawEnd);

	if (!IsTrackingMouse())
		SetSelection(location + length);
	ComputeContentSize();

	if (IsVisible())
	{
		uint32 firstVisible = FirstVisibleLine();
		uint32 lastVisible = LastVisibleLine();
		
		if (redrawStart < firstVisible)
			redrawStart = firstVisible;
		if (redrawEnd > lastVisible)
			redrawEnd = lastVisible;
	
		RedrawLines(redrawStart, redrawEnd, true);
	}

	HandleCommand(this, this, kDataModifiedCommandID);
}


void TTextView::ReplaceSelection(const TChar* text, STextOffset length, bool saveUndo, bool selectAfter, bool accumulateTyping)
{
	if (saveUndo)
		SaveUndo(fSelectionStart, fSelectionStart + length, accumulateTyping, false);

	uint32 redrawStart, redrawEnd;
	fLayout->ReplaceText(fSelectionStart, fSelectionEnd, text, length, redrawStart, redrawEnd);

	if (selectAfter)
		SetSelection(fSelectionStart + length, fSelectionStart + length, false);
	else
		SetSelection(fSelectionStart, fSelectionStart + length, false);

	ComputeContentSize();

	if (IsVisible())
	{
		uint32 firstVisible = FirstVisibleLine();
		uint32 lastVisible = LastVisibleLine();
		
		if (redrawStart < firstVisible)
			redrawStart = firstVisible;
		if (redrawEnd > lastVisible)
			redrawEnd = lastVisible;
	
		RedrawLines(redrawStart, redrawEnd, true);
	}
	
	fAccumulateTyping = accumulateTyping;

	HandleCommand(this, this, kDataModifiedCommandID);
}


void TTextView::Delete(bool forward, bool accumulateDeletion)
{	
	if (fSelectionStart == fSelectionEnd)
	{
		if (forward && fSelectionStart < GetTextLength())
			fLayout->NextCharacter(fSelectionEnd);
		else if (!forward && fSelectionStart > 0)
			fLayout->PreviousCharacter(fSelectionStart);
	}

	SaveUndo(fSelectionStart, fSelectionStart, false, accumulateDeletion);

	uint32 redrawStart, redrawEnd;
	fLayout->ReplaceText(fSelectionStart, fSelectionEnd, "", 0, redrawStart, redrawEnd);
	
	SetSelection(fSelectionStart, fSelectionStart, false);

	ComputeContentSize();

	if (IsVisible())
	{
		uint32 firstVisible = FirstVisibleLine();
		uint32 lastVisible = LastVisibleLine();
		
		if (redrawStart < firstVisible)
			redrawStart = firstVisible;
		if (redrawEnd > lastVisible)
			redrawEnd = lastVisible;
	
		RedrawLines(redrawStart, redrawEnd, true);
	}

	fAccumulateDeletion = accumulateDeletion;

	HandleCommand(this, this, kDataModifiedCommandID);
}


void TTextView::ShiftSelectionLeft()
{
	ExtendSelectionToLines();

	TString	text;
	GetSelectedText(text);

	int i;
	int length = text.GetLength();
	int deleteCount = 0;
	int deletedSpaces = 0;

	TChar lastChar = '\n';	// to ensure we operate on the first line
	
	for (i = 0; i < length; i++)
	{
		TChar ch = text[i];

		if (lastChar == kLineEnd10 || lastChar == kLineEnd13)
		{
			if (ch == '\t' || ch == ' ')
			{
				text.Replace(i, 1, "");
				--i;
				--length;
				++deleteCount;

				if (ch == ' ')
					deletedSpaces = 1;
			}
		}
		else if (ch == ' ' && deletedSpaces > 0 && deletedSpaces < kTabWidth)
		{
			text.Replace(i, 1, "");
			--i;
			--length;
			++deleteCount;
			++deletedSpaces;
		}
		else
			deletedSpaces = 0;

		lastChar = ch;
	}

	if (deleteCount > 0)
		ReplaceSelection(text, text.GetLength(), true, false);
}


void TTextView::ShiftSelectionRight()
{
	ExtendSelectionToLines();
	
	STextOffset selectionEnd = fSelectionEnd;
	if (selectionEnd > 0)
		fLayout->PreviousCharacter(selectionEnd);
	int length = selectionEnd - fSelectionStart;
	
	char* tabString = (char *)"\t";
	int tabLength = 1;  // length of tabString
	if (fSpacesPerTab > 0)
	{
		tabString = new char[fSpacesPerTab + 1];
		memset(tabString, ' ', fSpacesPerTab);
		tabString[fSpacesPerTab] = 0;
		tabLength = fSpacesPerTab;
	}

	TString	text;
	GetSelectedText(text);
	text.Replace(0, 0, tabString);
	length += tabLength;
	
	for (int i = 1; i < length; i++)
	{
		TChar ch = text[i];

		if (ch == kLineEnd10 || (ch == kLineEnd13 && !(i < length - 2 && text[i + 1] == kLineEnd10)))
		{
			text.Replace(i + 1, 0, tabString);
			i += tabLength;
			length += tabLength;
		}
	}
	
	if (fSpacesPerTab > 0)
		delete[] tabString;
		
	ReplaceSelection(text, text.GetLength(), true, false);
}


void TTextView::ExtendSelectionToLines()
{
	uint32 startLine = fLayout->OffsetToLine(fSelectionStart);
	STextOffset selectionEnd = fSelectionEnd;
	if (selectionEnd > 0)
		fLayout->PreviousCharacter(selectionEnd);
	uint32 endLine = fLayout->OffsetToLine(selectionEnd) + 1;
	if (startLine >= endLine)
		endLine = startLine + 1;
		
	STextOffset start = fLayout->LineToOffset(startLine);
	STextOffset end = fLayout->LineToOffset(endLine);
		
	SetSelection(start, end);
	AnchorSelection();
}


bool TTextView::FindString(const TChar* searchString, bool caseSensitive, bool forward, bool wrap, bool wholeWord)
{
	const TChar* text = GetText();
	STextOffset textLength = GetTextLength();
	STextOffset searchLength = Tstrlen(searchString);
	ASSERT(searchLength > 0);

	if (searchLength > textLength)
		return false;

	STextOffset start;

	if (forward)
	{
		start = fSelectionEnd;
	}
	else
	{
		start = fSelectionStart;
	}
		
	STextOffset offset = start;
	bool wrappedOnce = false;
	bool firstTime = true;

	while (1)
	{
		if (forward)
		{
			if (!firstTime)
				fLayout->NextCharacter(offset);

			if (offset + searchLength - 1 >= textLength)
			{
				if (wrap && !wrappedOnce)
				{
					offset = 0;
					wrappedOnce = true;
				}
				else
					break;
			}
		}
		else
		{
			if (offset == 0)
			{
				if (wrap && !wrappedOnce)
				{
					offset = textLength - searchLength;
					wrappedOnce = true;
				}
				else
					break;
			}
			else
				fLayout->PreviousCharacter(offset);
		}

		// stop if we wrapped around
		if (wrappedOnce && offset == start)
			break;

		firstTime = false;

		int result;
		if (caseSensitive)
			result = strncmp(text + offset, searchString, searchLength);
		else
			result = strncasecmp(text + offset, searchString, searchLength);

		if (result == 0)
		{
			if (wholeWord)
			{
				STextOffset start, end;
				
				if (fLayout->FindWord(offset, start, end))
				{
					if (start != offset)
						continue;		// start of word does not match, so keep looking
					
					// support searching for multiple words
					while (end < offset + searchLength)
					{
						start = end + 1;
						if (fLayout->FindWord(start, start, end) && end > offset + searchLength)
							break;	// did not match word boundary on the right
					}

					if (end > offset + searchLength)
						continue;		// end of word beyond end of search string
				}
			}
			
			// found!
			SetSelection(offset, offset + searchLength);
			AnchorSelection();
			ScrollSelectionIntoView();
			return true;
		}
	}

	return false;	// not found
}


void TTextView::LeftArrowKey(TModifierState state)
{
	bool extend = ((state & ShiftMask) != 0);
	
	if ((state & Mod1Mask) != 0)
	{
		// move to beginning of line
		STextOffset offset = (fSelectionAnchor == fSelectionStart ? fSelectionEnd : fSelectionStart);
		uint32 line = fLayout->OffsetToLine(offset);
		STextOffset lineStart = fLayout->LineToOffset(line);

		if (extend)
			SetSelection(lineStart, fSelectionAnchor);
		else
			SetSelection(lineStart, lineStart);
	}
	else if ((state & ControlMask) != 0)
	{
		STextOffset start, end;

		STextOffset offset = (fSelectionAnchor == fSelectionStart ? fSelectionEnd : fSelectionStart);

		const TChar* text = GetText();
		while (offset > 0)
		{
			fLayout->PreviousCharacter(offset);
			TChar ch = text[offset];
			if (ch != ' ' && ch != '\t')
				break;
		}
			
		fLayout->FindWord(offset, start, end);
			
		if (extend)
			SetSelection(start, fSelectionAnchor);
		else
			SetSelection(start, start);
	}
	else
	{
		if (extend)
		{
			if (fSelectionAnchor == fSelectionStart)
			{
				STextOffset offset = fSelectionEnd;
				if (offset > 0)
				{
					fLayout->PreviousCharacter(offset);
					SetSelection(fSelectionStart, offset);
				}
			}
			else
			{
				STextOffset offset = fSelectionStart;
				if (offset > 0)
				{
					fLayout->PreviousCharacter(offset);
					SetSelection(offset, fSelectionEnd);
				}
			}
		}
		else
		{
			if (fSelectionStart < fSelectionEnd)
				SetSelection(fSelectionStart);
			else if (fSelectionStart > 0)
			{
				STextOffset offset = fSelectionStart;
				if (offset > 0)
				{
					fLayout->PreviousCharacter(offset);
					SetSelection(offset);
				}
			}
		}
	}
}


void TTextView::RightArrowKey(TModifierState state)
{
	bool extend = ((state & ShiftMask) != 0);

	if ((state & Mod1Mask) != 0)
	{
		// move to end of line
		STextOffset offset = (fSelectionAnchor == fSelectionStart ? fSelectionEnd : fSelectionStart);
		uint32 line = fLayout->OffsetToLine(offset);
		STextOffset length;
		const TChar* lineText = fLayout->GetLineText(line, length);								
		STextOffset lineEnd = lineText + length - fLayout->GetText();
		
		if (extend)
			SetSelection(fSelectionAnchor, lineEnd);
		else
			SetSelection(lineEnd, lineEnd);
	}
	else if ((state & ControlMask) != 0)
	{
		STextOffset start, end;

		STextOffset offset = (fSelectionAnchor == fSelectionStart ? fSelectionEnd : fSelectionStart);
		if (offset < GetTextLength())
			fLayout->NextCharacter(offset);
			
		fLayout->FindWord(offset, start, end);
			
		if (extend)
			SetSelection(fSelectionAnchor, end);
		else
			SetSelection(end, end);
	}
	else
	{
		if (extend)
		{
			if (fSelectionAnchor == fSelectionStart)
			{
				STextOffset offset = fSelectionEnd;
				if (offset < GetTextLength())
				{
					fLayout->NextCharacter(offset);
					SetSelection(fSelectionStart, offset);
				}
			}
			else
			{
				STextOffset offset = fSelectionStart;
				if (offset < GetTextLength())
				{
					fLayout->NextCharacter(offset);
					SetSelection(offset, fSelectionEnd);
				}
			}
		}
		else
		{
			if (fSelectionStart < fSelectionEnd)
				SetSelection(fSelectionEnd);
			else if (fSelectionEnd < GetTextLength())
			{
				STextOffset offset = fSelectionEnd;
				if (offset < GetTextLength())
				{
					fLayout->NextCharacter(offset);
					SetSelection(offset);
				}
			}
		}
	}
}


void TTextView::UpArrowKey(TModifierState state)
{
	bool extend = ((state & ShiftMask) != 0);
	
	if (!extend && fSelectionStart < fSelectionEnd)
		SetSelection(fSelectionStart);
	else
	{
		TPoint p;
		STextOffset nonAnchor = (fSelectionAnchor == fSelectionStart ? fSelectionEnd : fSelectionStart);
		uint32 line = fLayout->OffsetToPoint(nonAnchor, p);

		if (line > 0)
		{
			TRect r;
			TCoord h;

			if (fUpDownHorizOffset >= 0)
				h = fUpDownHorizOffset;
			else
				fUpDownHorizOffset = h = p.h;
			
			fLayout->GetLineBounds(line - 1, r);
			STextOffset offset = fLayout->PointToOffset(TPoint(h, r.top));

			if (extend)
			{
				if (fLayout->OffsetToLine(fSelectionAnchor) == line - 1)
					SetSelection(fSelectionAnchor);
				else
					SetSelection(fSelectionAnchor, offset);
			}
			else
				SetSelection(offset);
		}
		else if (extend)
			SetSelection(0, fSelectionAnchor);
		else
			SetSelection(0);
	}
}


void TTextView::DownArrowKey(TModifierState state)
{
	bool extend = ((state & ShiftMask) != 0);

	if (!extend && fSelectionStart < fSelectionEnd)
		SetSelection(fSelectionEnd);
	else
	{
		TPoint p;
		STextOffset nonAnchor = (fSelectionAnchor == fSelectionStart ? fSelectionEnd : fSelectionStart);
		uint32 line = fLayout->OffsetToPoint(nonAnchor, p);

		if (line < fLayout->GetLineCount() - 1)
		{
			TRect r;
			TCoord h;

			if (fUpDownHorizOffset >= 0)
				h = fUpDownHorizOffset;
			else
				fUpDownHorizOffset = h = p.h;

			fLayout->GetLineBounds(line + 1, r);
			STextOffset offset = fLayout->PointToOffset(TPoint(h, r.top));

			if (extend)
			{
				if (fLayout->OffsetToLine(fSelectionAnchor) == line + 1)
					SetSelection(fSelectionAnchor);
				else
					SetSelection(fSelectionAnchor, offset);
			}
			else
				SetSelection(offset);
		}
		else if (extend)
			SetSelection(fSelectionAnchor, GetTextLength());
		else
			SetSelection(GetTextLength());
	}
}


TCoord TTextView::GetScrollIncrement(TScrollDirection direction) const
{
	if (direction == kScrollUp)
	{
		int32 line = FirstVisibleLine() - 1;
		if (line >= 0)
		{
			TCoord delta = fLayout->LineToVertOffset(line) - fScroll.v;
			if (line == 0)
				delta -= fInset.top;
			return delta;
		}
	}
	else if (direction == kScrollDown)
	{
		uint32 line = FirstVisibleLine() + 1;
		if (line < fLayout->GetLineCount())
		{
			TCoord delta = fLayout->LineToVertOffset(line) - fScroll.v;
//			if (line == fLayout->GetLineCount() - 1)
//				delta += fInset.bottom;
			return delta;
		}
	}

	return TView::GetScrollIncrement(direction);
}


TCoord TTextView::GetPageIncrement(TScrollDirection direction) const
{
	if (direction == kScrollUp)
	{
		int32 line = fLayout->VertOffsetToLine(fScroll.v - GetHeight());
		if (line >= 0)
		{
			TCoord delta = fLayout->LineToVertOffset(line) - fScroll.v;
			if (line == 0)
				delta -= fInset.top;
			return delta;
		}
	}
	else if (direction == kScrollDown)
	{
		uint32 line = LastVisibleLine();
		TCoord delta = fLayout->LineToVertOffset(line) - fScroll.v;
//		if (line == fLayout->GetLineCount() - 1)
//			delta += fInset.bottom;
		return delta;
	}

	return TView::GetPageIncrement(direction);
}


void TTextView::SetLineWrap(bool lineWrap)
{
	if (lineWrap != fLineWrap)
	{
		STextOffset topOffset = fLayout->LineToOffset(FirstVisibleLine());

		fLineWrap = lineWrap;
		fLayout->SetLineWrap(lineWrap, GetWidth() - fInset.left - fInset.right);
		ComputeContentSize();
		
		ScrollToLine(fLayout->OffsetToLine(topOffset, true));

		if (IsCreated())
			Redraw();
	}
}


void TTextView::ScrollSelectionIntoView(bool selecting, bool allowHorizScroll)
{
	TCoord deltaH = 0;
	TCoord deltaV = 0;
	uint32 firstLine = FirstVisibleLine();
	uint32 lastLine = LastVisibleLine();

	uint32 startSelectionLine = fLayout->OffsetToLine(fSelectionStart);
	uint32 endSelectionLine = fLayout->OffsetToLine(fSelectionEnd);

	bool revealTop = startSelectionLine < firstLine;
	bool revealBottom = endSelectionLine >= lastLine;
	
	if (selecting)
	{
		if (fSelectionAnchor == fSelectionStart)
		{
			revealTop = false;
			if (!revealBottom)
				revealBottom = endSelectionLine < firstLine;
		}
		else
		{
			revealBottom = false;
			if (!revealTop)
				revealTop = startSelectionLine >= lastLine;
		}
	}
	
	if (revealTop)
	{
		deltaV = fLayout->LineToVertOffset(startSelectionLine) - fScroll.v;
		if (startSelectionLine == 0)
			deltaV -= fInset.top;
	}
	else if (revealBottom)
	{
		uint32 topLine = fLayout->VertOffsetToLine(fLayout->LineToVertOffset(endSelectionLine) - GetHeight()) + 2;
		if (topLine >= GetLineCount())
			topLine = GetLineCount() - 1;
		deltaV = fLayout->LineToVertOffset(topLine) - fScroll.v;
	}

	// now handle the horizontal case
	if (allowHorizScroll)
	{
		TCoord leftOffset, rightOffset;
	
		bool topIsLeft = true;
		
		if (fSelectionStart == fSelectionEnd)
		{
			TPoint point;
			fLayout->OffsetToPoint(fSelectionStart, point);
			leftOffset = point.h;
			rightOffset = leftOffset + 1;
		}
		else
		{
			TPoint point1, point2;
			fLayout->OffsetToPoint(fSelectionStart, point1);
			fLayout->OffsetToPoint(fSelectionEnd, point2);
		
			if (point1.h < point2.h)
			{
				leftOffset = point1.h;
				rightOffset = point2.h;
			}
			else
			{
				leftOffset = point2.h;
				rightOffset = point1.h;
				topIsLeft = false;
			}
		}
	
		TRect bounds;
		GetScrollableBounds(bounds);
		bounds.Offset(fScroll);
		
		bool revealLeft = leftOffset < bounds.left;
		bool revealRight = rightOffset >= bounds.right;
	
		if (selecting)
		{
			if ((fSelectionAnchor == fSelectionStart && topIsLeft) ||
				(fSelectionAnchor == fSelectionEnd && !topIsLeft))
			{
				revealLeft = false;
				if (!revealRight)
					revealRight = rightOffset < bounds.left;
			}
			else
			{
				revealRight = false;
				if (!revealLeft)
					revealLeft = leftOffset >= bounds.right;
			}
		}
	
		if (revealLeft)
			deltaH = leftOffset - bounds.left - 10;
		else if (revealRight)
			deltaH = rightOffset - bounds.right + 10;
	}

	if (deltaH || deltaV)
		ScrollBy(deltaH, deltaV);
}


void TTextView::ScrollToLine(uint32 line)
{
	TPoint	point;
	
	if (line > 0)
	{
		fLayout->OffsetToPoint(fLayout->LineToOffset(line, true), point);
		ScrollBy(-fScroll.h, point.v - fScroll.v);
	}
	else
		ScrollBy(-fScroll.h, -fScroll.v);
}


void TTextView::SelectLine(uint32 line)
{
	ScrollToLine(line > 5 ? line - 5 : 0);

	if (line < GetLineCount())
	{
		STextOffset lineOffset, lineLength;
		lineOffset = fLayout->LineToOffset(line, true);
		fLayout->GetLineText(line, lineLength, true);
		
		SetSelection(lineOffset, lineOffset + lineLength);
		AnchorSelection();
	}
}


void TTextView::DrawTextRange(TDrawContext& context, STextOffset startOffset, STextOffset endOffset, TRegion* clip)
{
	ASSERT(startOffset <= endOffset);
	if (startOffset == endOffset)
		return;

	uint32	startLine = fLayout->OffsetToLine(startOffset);
	uint32	endLine = fLayout->OffsetToLine(endOffset);
	
	RedrawLines(startLine, endLine, false, clip);
}

void TTextView::DrawLine(uint32 line, TDrawContext& context, TCoord rightEdge)
{
	const TChar* text;
	STextOffset lineLength;
	TCoord vertOffset, ascent, height;	

	fLayout->GetLine(line, text, lineLength, vertOffset, ascent, height);

	STextOffset lineStart = fLayout->LineToOffset(line);
	STextOffset lineEnd = lineStart + lineLength;

	bool hilited = (fSelectionStart <= lineStart && lineStart < fSelectionEnd);
	if (context.GetDepth() < 8)
	{
		context.SetForeColor(hilited ? fBackColor : fForeColor);
		context.SetBackColor(hilited ? fForeColor : fBackColor);		
	}
	else
		context.SetBackColor(hilited ? gApplication->GetHiliteColor() : fBackColor);
		
	context.MoveTo(fInset.left, vertOffset + ascent + fInset.top);

	// clear insertion point if necessary
	TCoord top = context.GetPen().v - fLayout->GetLineAscent(line);
	TRect	r(context.GetPen().h - 1, top, context.GetPen().h, top + fLayout->GetLineHeight(line));
	context.EraseRect(r);
	
	if (lineLength > 0)
	{		
		if (fSelectionStart == fSelectionEnd || fSelectionStart >= lineEnd || fSelectionEnd <= lineStart)
		{
			DrawText(text, lineLength, context);
		}
		else
		{
			int32 length = fSelectionStart - lineStart;
			if (length > 0)
			{
				DrawText(text, length, context);
				lineStart = fSelectionStart;
				text += length;
				
				if (context.GetDepth() < 8)
				{
					context.SetForeColor(fBackColor);
					context.SetBackColor(fForeColor);
				}
				else
					context.SetBackColor(gApplication->GetHiliteColor());
			}

			STextOffset end = (lineEnd < fSelectionEnd ? lineEnd : fSelectionEnd);
			length = end - lineStart;
			DrawText(text, length, context);
			lineStart = end;
			text += length;

			if (lineStart < lineEnd)
			{
				context.SetForeColor(fForeColor);
				context.SetBackColor(fBackColor);
				DrawText(text, lineEnd - lineStart, context);
			}
		}
	}

	EraseRightEdge(line, context, rightEdge, lineEnd);
}


void TTextView::EraseRightEdge(uint32 line, TDrawContext& context, TCoord rightEdge, STextOffset lineEnd)
{
	// erase remainder of the line
	if (context.GetPen().h < rightEdge)
	{
		// special case selection boundary at end of line
		if (fSelectionStart < fSelectionEnd)
		{
			if (lineEnd == fSelectionStart)
			{
				if (context.GetDepth() < 8)
				{
					context.SetForeColor(fBackColor);
					context.SetBackColor(fForeColor);
				}
				else
					context.SetBackColor(gApplication->GetHiliteColor());
			}
			else if (lineEnd == fSelectionEnd)
			{
				context.SetForeColor(fForeColor);
				context.SetBackColor(fBackColor);
			}
		}

		TRect	r;
		fLayout->GetLineBounds(line, r);
		r.left = context.GetPen().h;
		r.right = rightEdge;
		context.EraseRect(r);
	}
}


void TTextView::RedrawLines(uint32 startLine, uint32 endLine, bool showHideInsertionPoint, TRegion* clip)
{
//printf("RedrawLines(%ld, %ld)\n", startLine, endLine);
	TRect	border;
	GetScrollableBounds(border);

	TRegion	borderClip(border);
	if (clip)
		borderClip.Intersect(clip);
	
	TDrawContext	context(this, &borderClip);
	context.SetFont(fFont);

	if (showHideInsertionPoint)
		HideInsertionPoint(context);

	TCoord rightEdge = fContentSize.h - fInset.right;
	if (rightEdge < fScroll.h + GetWidth())
		rightEdge = fScroll.h + GetWidth();

	for (uint32 line = startLine; line <= endLine; line++)
		DrawLine(line, context, rightEdge);

	if (showHideInsertionPoint)
		ShowInsertionPoint(context);
}


void TTextView::DrawText(const TChar* text, int length, TDrawContext& context)
{
	const TChar* start = text;
	const TChar* end = text + length;
	TCoord tabWidth = fLayout->GetTabWidth();
	TCoord leftInset = fInset.left;

	while (start < end)
	{
		// find next tab or end of line
		while (text < end && *text != '\t')
			fLayout->NextCharacter(text);

		if (text > start)
			context.DrawText(start, text - start, true);


		if (text < end && *text == '\t')
		{
			int tabCount = 0;
			while (text < end && *text == '\t')
			{
				++tabCount;
				++text;
			}

			TCoord	ascent, height;
			context.MeasureText(" ", 1, ascent, height);
			
			TRect	r;
			r.left = context.GetPen().h;
			r.top = context.GetPen().v - ascent;
			r.bottom = r.top + height;
			r.right = ((r.left - leftInset + tabCount * tabWidth) / tabWidth) * tabWidth + leftInset;	// skip to next tab

			context.EraseRect(r);
			context.MoveTo(r.right, context.GetPen().v); 
		}

		start = text;
	}
}


void TTextView::HideCursor()
{
	if (!fCursorHidden)
	{
		SetCursor(GetInvisibleCursor());
		fCursorHidden = true;
	}
}


uint32 TTextView::FirstVisibleLine() const
{
	return fLayout->VertOffsetToLine(fScroll.v);
}


uint32 TTextView::LastVisibleLine() const
{
	return fLayout->VertOffsetToLine(GetHeight() + fScroll.v);
}


void TTextView::DrawInsertionPoint(TDrawContext& context)
{
	ASSERT(fLayout);
	
	if (fSelectionStart == fSelectionEnd)
	{
		TPoint p;
		uint32 line = fLayout->OffsetToPoint(fSelectionStart, p);
		
		TRect	r(p.h - 1, p.v - fLayout->GetLineAscent(line), p.h, p.v);
		
		// clip to our text area
		TRect	border;
		GetScrollableBounds(border);
		border.Offset(fScroll);
		r.IntersectWith(border);
		
		if (!r.IsEmpty())
		{
			if (fInsertionPointOn)
			{
				context.SetForeColor(kRedColor);
				context.PaintRect(r);
			}
			else
				context.EraseRect(r);
		}
	}
}


void TTextView::ShowInsertionPoint(TDrawContext& context)
{
	if (! fInsertionPointOn)
	{
		fInsertionPointOn = true;
		DrawInsertionPoint(context);
	}

	if (IdlingEnabled())
		Sleep(kCursorBlinkTime);	// reset idle timer
}


void TTextView::HideInsertionPoint(TDrawContext& context)
{
	if (fInsertionPointOn)
	{
		fInsertionPointOn = false;
		DrawInsertionPoint(context);
	}
}


void TTextView::SetSelection(STextOffset start, STextOffset end, bool redraw)
{
	if (start != fSelectionStart || end != fSelectionEnd)
	{
		if (start == end)
			fSelectionAnchor = start;
		else if (end < start)
		{
			STextOffset temp = start;
			start = end;
			end = temp;
		}
		
		if (IsVisible())
		{
			TDrawContext	context(this);
	
			STextOffset oldStart = fSelectionStart;
			STextOffset oldEnd = fSelectionEnd;
	
			if (redraw)
				HideInsertionPoint(context);
			
			fSelectionStart = start;
			fSelectionEnd = end;
	
			if (redraw)
			{
				if (oldEnd <= start || end <= oldStart)
				{
					DrawTextRange(context, oldStart, oldEnd);
					DrawTextRange(context, start, end);
				}
				else
				{
					if (oldStart == oldEnd)
						DrawTextRange(context, start, end);
					else if (start == end)
						DrawTextRange(context, oldStart, oldEnd);
					else
					{
						if (start < oldStart)
							DrawTextRange(context, start, oldStart);
						else if (oldStart < start)
							DrawTextRange(context, oldStart, start);
				
						if (end < oldEnd)
							DrawTextRange(context, end, oldEnd);
						else if (oldEnd < end)
							DrawTextRange(context, oldEnd, end);
					}
				}
	
				ShowInsertionPoint(context);
			}
	
			if (start == end)
			{
				if (HasFocus())
					EnableIdling(true);
				Sleep(kCursorBlinkTime);
			}
			else
			{
				BecomeSelectionOwner();
				if (fModifiable && HasFocus())
					EnableIdling(false);
			}
		}
		else
		{
			fSelectionStart = start;
			fSelectionEnd = end;
		}

		fAccumulateTyping = fAccumulateDeletion = false;

		HandleCommand(this, this, kSelectionChangedCommandID);
	}

	SetIMLocation();
}


void TTextView::SetIMLocation()
{	
	if (fInputContext)
	{
		TPoint	point;
		fLayout->OffsetToPoint(fSelectionStart, point);
		const TPoint& scroll = GetScroll();
		point.Offset(-scroll.h, -scroll.v);
//		LocalToTopLevel(point);
		fInputContext->SetSpotLocation(point);
	}
}


void TTextView::ComputeContentSize()
{
	TPoint	contentSize;
	
	fLayout->ComputeContentSize(contentSize);

	contentSize.h += (fInset.left + fInset.right);
	contentSize.v += (fInset.top + fInset.bottom);
	
	if (fLineWrap && contentSize.h > GetWidth())
		contentSize.h = GetWidth();		// content is pinned to width of this view
	
	SetContentSize(contentSize);
}


void TTextView::DoSetupMenu(TMenu* menu)
{
	if (HasUndo())
		menu->EnableCommand(kUndoCommandID);
	if (HasRedo())
		menu->EnableCommand(kRedoCommandID);

	if (fSelectionStart < fSelectionEnd)
	{
		menu->EnableCommand(kFindSelectionCommandID);
	
		if (fModifiable)
		{
			menu->EnableCommand(kCutCommandID);
			menu->EnableCommand(kClearCommandID);
		}
		
		menu->EnableCommand(kCopyCommandID);
	}

	if (fModifiable && TClipboard::HasData(XA_STRING))
		menu->EnableCommand(kPasteCommandID);
		
	menu->EnableCommand(kSelectAllCommandID);
	menu->EnableCommand(kBalanceSelectionCommandID);
	menu->EnableCommand(kToggleLineWrapCommandID, fLineWrap);
	
	if (fModifiable)
	{
		menu->EnableCommand(kShiftLeftCommandID);
		menu->EnableCommand(kShiftRightCommandID);
	
		TLineEndingFormat lineEndingFormat = fLayout->GetLineEndingFormat();
	
		menu->EnableCommand(kUnixFormatCommandID, lineEndingFormat == kUnixLineEndingFormat);
		menu->EnableCommand(kMacFormatCommandID, lineEndingFormat == kMacLineEndingFormat);
		menu->EnableCommand(kDOSFormatCommandID, lineEndingFormat == kDOSLineEndingFormat);
	}

	if (fMultiLine)
		menu->EnableCommand(kGotoLineCommandID);

	TView::DoSetupMenu(menu);
}


bool TTextView::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kUndoCommandID:
			Undo();
			ScrollSelectionIntoView();
			return true;

		case kRedoCommandID:
			Redo();
			ScrollSelectionIntoView();
			return true;

		case kFindSelectionCommandID:
			if (fSelectionStart < fSelectionEnd)
			{
				TString	fileName(fLayout->GetText() + fSelectionStart, fSelectionEnd - fSelectionStart);
				const char* colon = strchr(fileName, ':');
				int line = 0;
				if (colon && isdigit(colon[1]))
				{
					line = atoi(colon + 1);
					fileName.Set(fileName, colon - fileName);
				}

				TFile* file = gApplication->FindFile(fileName, GetDocument());
				if (file)
					gApplication->OpenFile(file, line);
				else
					gApplication->Beep();
			}
			break;

		case kCutCommandID:
			if (fModifiable && fSelectionStart < fSelectionEnd)
			{
				TClipboard::CopyData(XA_STRING, (const unsigned char *)fLayout->GetText() + fSelectionStart, fSelectionEnd - fSelectionStart);
				Delete(false, false);
				ScrollSelectionIntoView();
				return true;
			}
			break;

		case kCopyCommandID:
			if (fSelectionStart < fSelectionEnd)
			{
				TClipboard::CopyData(XA_STRING, (const unsigned char *)fLayout->GetText() + fSelectionStart, fSelectionEnd - fSelectionStart);
				return true;
			}
			break;

		case kPasteCommandID:
		{
			if (fModifiable)
			{
				const unsigned char* data;
				uint32 length;
				
				if (TClipboard::GetData(XA_STRING, data, length))
				{
					// make sure line endings are converted if necessary
					TLineEndingFormat format;
					if (TString::GetLineEndingFormat((const TChar*)data, length, format) && format != fLayout->GetLineEndingFormat())
					{
						TString	temp((const TChar*)data, length);
						temp.SetLineEndingFormat(fLayout->GetLineEndingFormat());
						ReplaceSelection(temp, temp.GetLength(), true, true);
					}
					else
						ReplaceSelection((const TChar *)data, length, true, true);
						
					ScrollSelectionIntoView();
					return true;
				}
			}
			break;
		}

		case kClearCommandID:
			if (fModifiable && fSelectionStart < fSelectionEnd)
			{
				Delete(false, false);
				ScrollSelectionIntoView();
				return true;
			}
			break;

		case kSelectAllCommandID:
			SelectAll();
			AnchorSelection();
			ScrollSelectionIntoView();
			return true;
			
		case kBalanceSelectionCommandID:
		{
			STextOffset start = fSelectionStart;
			STextOffset end = fSelectionEnd;
			
			if (fLayout->BalanceSelection(start, end))
			{
				SetSelection(start, end);
				AnchorSelection();
				ScrollSelectionIntoView();
			}
			
			return true;
		}
		
		case kToggleLineWrapCommandID:
			SetLineWrap(!fLineWrap);
			return true;

		case kShiftLeftCommandID:
			ShiftSelectionLeft();
			return true;
			
		case kShiftRightCommandID:
			ShiftSelectionRight();
			return true;

		case kUnixFormatCommandID:
			SetLineEndingFormat(kUnixLineEndingFormat);
			return true;

		case kMacFormatCommandID:
			SetLineEndingFormat(kMacLineEndingFormat);
			return true;

		case kDOSFormatCommandID:
			SetLineEndingFormat(kDOSLineEndingFormat);
			return true;

		case kGotoLineCommandID:
		{
			TString	lineString;
			if (TCommonDialogs::TextEntryDialog(_("Line Number:"), _("Goto Line"), GetTopLevelWindow(), lineString, new TNumericEntryBehavior) && lineString.GetLength() > 0)
			{
				int lineNumber = lineString.AsInteger();
				lineNumber--;
				if (lineNumber < 0)
					lineNumber = 0;
				else if (lineNumber >= (int)GetLineCount())
					lineNumber = GetLineCount() - 1;

				STextOffset offset = fLayout->LineToOffset(lineNumber, true);
				SetSelection(offset, offset);
				AnchorSelection();
				ScrollSelectionIntoView();
			}
			return true;
		}

		case kFocusAcquiredCommandID:
			if (fInputContext)
			{
				fInputContext->SetFocus(this);
				SetIMLocation();
			}

			if (fHideInsertionPointWhenNotTarget && fSelectionStart == fSelectionEnd && IsCreated())
			{
				TDrawContext context(this);
				ShowInsertionPoint(context);
			}
					
			if (fModifiable && fSelectionStart == fSelectionEnd && IsCreated())
				EnableIdling(true);
			break;
			
		case kFocusLostCommandID:
			if (fInputContext)
				fInputContext->ClearFocus();
				
			if (fCursorHidden)
			{
				SetCursor(sCursor);
				fCursorHidden = false;
			}

			if (fHideInsertionPointWhenNotTarget)
			{
				SetSelection(0);
				TDrawContext context(this);
				HideInsertionPoint(context);
			}
			else
			{
				if (fSelectionStart == fSelectionEnd && IsCreated())
				{
					TDrawContext context(this);
					ShowInsertionPoint(context);
					EnableIdling(false);
				}
			}
			break;

		default:
			break;
	}

	return TView::DoCommand(sender, receiver, command);
}


void TTextView::DoIdle()
{
	if (fSelectionStart == fSelectionEnd)
	{
		TDrawContext	context(this);
		fInsertionPointOn = !fInsertionPointOn;
		DrawInsertionPoint(context);
	}
}


void TTextView::GetText(TString& string)
{
	string.Set(fLayout->GetText(), fLayout->GetTextLength());
}


void TTextView::GetSelectedText(TString& string)
{
	string.Set(fLayout->GetText() + fSelectionStart, fSelectionEnd - fSelectionStart);
}


void TTextView::SetInset(const TRect& inset)
{
	fInset = inset;
	fLayout->SetInset(TPoint(inset.left, inset.top));
}


void TTextView::SetLineEndingFormat(TLineEndingFormat format)
{
	ASSERT(format == kUnixLineEndingFormat || format == kDOSLineEndingFormat || format == kMacLineEndingFormat);

	fLayout->SetLineEndingFormat(format, AdjustOffsetsCallback, this);

	TListIterator<UndoRedoData> iter(fUndoRedoList);
	UndoRedoData* undoRedoData;

	while ((undoRedoData = iter.Next()) != NULL)
	{
		int oldLength = undoRedoData->fText.GetLength();
		undoRedoData->fText.SetLineEndingFormat(format);
		undoRedoData->fOldEnd += (undoRedoData->fText.GetLength() - oldLength);
	}

	HandleCommand(this, this, kDataModifiedCommandID);
	HandleCommand(this, this, kLineEndingsChangedCommandID);
}


void TTextView::SaveUndo(STextOffset newSelectionStart, STextOffset newSelectionEnd, bool accumulateTyping, bool accumulateDeletion)
{
	// delete any redo data
	for (int i = fUndoRedoList.GetSize() - 1; i > fUndoRedoIndex; i--)
		fUndoRedoList.DeleteAt(i);

	if (fSavedUndoRedoIndex > fUndoRedoIndex)
		fSavedUndoRedoIndex = -2;		// we can no longer undo/redo to saved version of document. 

	if (!accumulateTyping)	
		fAccumulateTyping = false;
	if (!accumulateDeletion)	
		fAccumulateDeletion = false;
		
	if (fAccumulateTyping)
	{
		ASSERT(fUndoRedoIndex >= 0);
		UndoRedoData* data = fUndoRedoList[fUndoRedoIndex];
		data->fNewEnd = newSelectionEnd;
		data->fNewSelectionEnd = newSelectionEnd;
	}
	else if (fAccumulateDeletion)
	{
		ASSERT(fUndoRedoIndex >= 0);
		UndoRedoData* data = fUndoRedoList[fUndoRedoIndex];
		
		data->fNewStart = data->fNewEnd = newSelectionStart;
		data->fNewSelectionStart = data->fNewSelectionEnd = newSelectionStart;
		data->fOldStart = data->fOldSelectionStart = newSelectionStart;
		data->fText.Replace(0, 0, GetText() + fSelectionStart, fSelectionEnd - fSelectionStart);
	}
	else
	{
		UndoRedoData* data = new UndoRedoData(fSelectionStart, fSelectionEnd, newSelectionStart, newSelectionEnd, fSelectionStart, fSelectionEnd, kNormal);
		GetSelectedText(data->fText);
		
		fUndoRedoList.InsertLast(data);
		++fUndoRedoIndex;
		ASSERT(fUndoRedoIndex == fUndoRedoList.GetSize() - 1);
	
		fAccumulateTyping = accumulateTyping;
		fAccumulateDeletion = accumulateDeletion;
	}
}


// this variant only used for mouse copy
void TTextView::SaveUndoMouseCopy(STextOffset offset, STextOffset length, UndoType undoType)
{
	// delete any redo data
	for (int i = fUndoRedoList.GetSize() - 1; i > fUndoRedoIndex; i--)
		fUndoRedoList.DeleteAt(i);

	if (fSavedUndoRedoIndex > fUndoRedoIndex)
		fSavedUndoRedoIndex = -2;		// we can no longer undo/redo to saved version of document. 

	fAccumulateTyping = fAccumulateDeletion = false;

	UndoRedoData* data = new UndoRedoData(offset, offset, offset, offset + length, fSelectionStart, fSelectionEnd, undoType);
		
	fUndoRedoList.InsertLast(data);
	++fUndoRedoIndex;
	ASSERT(fUndoRedoIndex == fUndoRedoList.GetSize() - 1);
}


void TTextView::Undo()
{
	ASSERT(HasUndo());
	
	UndoRedoData* data = fUndoRedoList[fUndoRedoIndex];

	SetSelection(data->fNewStart, data->fNewEnd);
	TString newText;
	GetSelectedText(newText);
	ReplaceSelection(data->fText, data->fText.GetLength(), false, true);
	SetSelection(data->fOldSelectionStart, data->fOldSelectionEnd);
	AnchorSelection();

	data->fText = newText;

	--fUndoRedoIndex;

	if (data->fUndoType == kGroupWithPrevious)
		Undo();
}


void TTextView::Redo()
{
	ASSERT(HasRedo());

	UndoRedoData* data = fUndoRedoList[++fUndoRedoIndex];

	STextOffset selectionStart, selectionEnd;
	GetSelection(selectionStart, selectionEnd);
	
	SetSelection(data->fOldStart, data->fOldEnd);
	TString oldText;
	GetSelectedText(oldText);
	ReplaceSelection(data->fText, data->fText.GetLength(), false, true);
	SetSelection(data->fNewSelectionStart, data->fNewSelectionEnd);
	AnchorSelection();

	data->fText = oldText;
	
	if (data->fUndoType == kGroupWithNext)
		Redo();
}


void TTextView::ClearUndoRedo()
{
	for (int i = fUndoRedoList.GetSize() - 1; i >= 0; i--)
		fUndoRedoList.DeleteAt(i);

	fUndoRedoIndex = -1;
	fSavedUndoRedoIndex = -1;
	fAccumulateTyping = false;
	fAccumulateDeletion = false;
}


void TTextView::SetUndoSelection()
{
	if (HasUndo())
	{
		UndoRedoData* data = fUndoRedoList[fUndoRedoIndex];
		data->fNewSelectionStart = fSelectionStart;
		data->fNewSelectionEnd = fSelectionEnd;
	}
}


void TTextView::AdjustOffsetsCallback(STextOffset offset, int shift, void* callbackData)
{
	((TTextView *)callbackData)->AdjustOffsets(offset, shift);
}


void TTextView::AdjustOffsets(STextOffset offset, int shift)
{
	if (fSelectionStart >= offset)
		fSelectionStart += shift;

	if (fSelectionEnd >= offset)
		fSelectionEnd += shift;

	if (fSelectionAnchor >= offset)
		fSelectionAnchor += shift;
		
	if (fSelectionAnchorEnd >= offset)
		fSelectionAnchorEnd += shift;

	if (fMouseCopyLocation >= offset)
		fMouseCopyLocation += shift;

	TListIterator<UndoRedoData> iter(fUndoRedoList);
	UndoRedoData* undoRedoData;

	while ((undoRedoData = iter.Next()) != NULL)
	{
		if (undoRedoData->fOldStart >= offset)
		{
			undoRedoData->fOldStart += shift;
			// fOldEnd gets incremented later based on the line ending difference for fText
			undoRedoData->fOldEnd += shift;
		}

		if (undoRedoData->fNewStart >= offset)
			undoRedoData->fNewStart += shift;

		if (undoRedoData->fNewEnd >= offset)
			undoRedoData->fNewEnd += shift;
	}
}


void TTextView::TextSaved()
{
	// reset fSavedUndoRedoIndex
	fSavedUndoRedoIndex = fUndoRedoIndex;
	
	// this is necessary to ensure we can undo/redo back to point where document last saved
	fAccumulateTyping = fAccumulateDeletion = false;
}


