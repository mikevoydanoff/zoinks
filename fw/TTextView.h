// ========================================================================================
//	TTextView.h				 	Copyright (C) 2001-2007 Mike Lockwood. All rights reserved.
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

#ifndef __TTextView__
#define __TTextView__

#include "TView.h"
#include "TIdler.h"
#include "TMouseTrackingIdler.h"
#include "TTextLayout.h"
#include "TString.h"
#include "TList.h"


class TFont;
class TDrawContext;


class TTextView : public TView, public TIdler
{
public:
	enum UndoType
	{
		kNormal,
		kGroupWithPrevious,
		kGroupWithNext
	};

private:	
	struct UndoRedoData
	{
		inline UndoRedoData(STextOffset oldStart, STextOffset oldEnd, STextOffset newStart, STextOffset newEnd, 
							 STextOffset oldSelectionStart, STextOffset oldSelectionEnd, UndoType undoType)
				:	fOldStart(oldStart), fOldEnd(oldEnd), fNewStart(newStart), fNewEnd(newEnd), 
					fOldSelectionStart(oldSelectionStart), fOldSelectionEnd(oldSelectionEnd), 
					fNewSelectionStart(newStart), fNewSelectionEnd(newEnd),fUndoType(undoType) {}
			
		TString			fText;
		STextOffset		fOldStart;
		STextOffset		fOldEnd;
		STextOffset		fNewStart;
		STextOffset		fNewEnd;
		STextOffset		fOldSelectionStart;
		STextOffset		fOldSelectionEnd;
		STextOffset		fNewSelectionStart;
		STextOffset		fNewSelectionEnd;
		UndoType		fUndoType;
	};
		
public:
								TTextView(TWindow* parent, const TRect& bounds, TFont* font, bool modifiable = true, bool multiLine = true);

	virtual void				Create();
	virtual TFont*				GetFont();

	inline const TChar*			GetText() const { return fLayout->GetText(); }
	inline TTextLayout*			GetTextLayout() const { return fLayout; }
	inline STextOffset			GetTextLength() const { return fLayout->GetTextLength(); }
	inline uint32				GetLineCount() const { return fLayout->GetLineCount(); }
	inline TLineEndingFormat	GetLineEndingFormat() const { return fLayout->GetLineEndingFormat(); }

	void						SetText(TChar* text, STextOffset length = 0, bool ownsData = false);
	inline void					SetText(const TString& text) { SetText((TChar *)(const TChar *)text, text.GetLength(), false); }

	void						GetText(TString& string);
	void						GetSelectedText(TString& string);
	
	inline const TRect&			GetInset() const { return fInset; }
	void						SetInset(const TRect& inset);

	
	void						ScrollToLine(uint32 line);
	void						SelectLine(uint32 line);

	inline void					GetSelection(STextOffset& start, STextOffset& end) const { start = fSelectionStart; end = fSelectionEnd; }
	virtual void				SetSelection(STextOffset start, STextOffset end, bool redraw = true);
	inline void					SetSelection(STextOffset offset) { SetSelection(offset, offset); }
	inline void					SelectAll() { SetSelection(0, GetTextLength()); }
	inline void					AnchorSelection() { fSelectionAnchor = fSelectionStart; }

	void						InsertTabSpaces();
	void						InsertText(STextOffset location, const TChar* text, STextOffset length, UndoType undoType = kNormal);
	void						ReplaceSelection(const TChar* text, STextOffset length, bool saveUndo, bool selectAfter, bool accumulateTyping = false);
	void						Delete(bool forward, bool accumulateDeletion);
	void						AutoIndent();
	void						SetUndoSelection();

	void						ShiftSelectionLeft();
	void						ShiftSelectionRight();
	void						ExtendSelectionToLines();

	bool						FindString(const TChar* searchString, bool caseSensitive, bool forward, bool wrap, bool wholeWord);

	inline bool					FilterTabAndCR() const { return fFilterTabAndCR; }
	inline void					SetFilterTabAndCR(bool filter) { fFilterTabAndCR = filter; }
	
	inline bool					IsModifiable() const { return fModifiable; }
	inline void					SetModifiable(bool modifiable) { fModifiable = modifiable; }
	
	inline bool					HasLineWrap() const { return fLineWrap; }
	void						SetLineWrap(bool lineWrap);
	
	void						ScrollSelectionIntoView(bool selecting = false, bool allowHorizScroll = true);

	void						TextSaved();
	inline bool					NeedsSaving() const { return  fSavedUndoRedoIndex != fUndoRedoIndex; }

	void						ClearUndoRedo();
	
	inline void					SetSpacesPerTab(int spacesPerTab) { fSpacesPerTab = spacesPerTab; }
	
protected:
	virtual						~TTextView();
	virtual void				Draw(TRegion* clip);
	virtual void				RedrawRect(const TRect& r);
	void						DoDraw(uint32 startLine, uint32 endLine, TRegion* clip);

	virtual void				SetContentSize(const TPoint& contentSize);
	virtual void				EraseContentDifference(const TPoint& oldContentSize, const TPoint& newContentSize);
	virtual void				GetScrollableBounds(TRect& bounds) const;
	virtual void				NotifyBoundsChanged(const TRect& oldBounds);
	
	virtual void				DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				DoMouseMoved(const TPoint& point, TModifierState state);
	virtual bool				DoKeyDown(KeySym key, TModifierState state, const char* string);

	virtual bool				IsTargetable() const;

	virtual bool				RequestSelectionData(Atom& inOutType, unsigned char*& outData, uint32& outLength);
	virtual void				ReceiveSelectionData(Atom type, const unsigned char* data, uint32 length);
	virtual void				LostSelectionOwnership();
	
	virtual void				PastedText();

	void						DrawTextRange(TDrawContext& context, STextOffset startOffset, STextOffset endOffset, TRegion* clip = NULL);

	void						DrawInsertionPoint(TDrawContext& context);
	void						ShowInsertionPoint(TDrawContext& context);
	void						HideInsertionPoint(TDrawContext& context);
	
    virtual void                DrawLine(uint32 line, TDrawContext& context, TCoord rightEdge);
    virtual void                EraseRightEdge(uint32 line, TDrawContext& context, TCoord rightEdge, STextOffset lineEnd);
	virtual void				RedrawLines(uint32 startLine, uint32 endLine, bool showHideInsertionPoint, TRegion* clip = NULL);
	virtual void				DrawText(const TChar* text, int length, TDrawContext& context);
	
	void						HideCursor();

	uint32						FirstVisibleLine() const;
	uint32						LastVisibleLine() const;

	void						LeftArrowKey(TModifierState state);
	void						RightArrowKey(TModifierState state);
	void						UpArrowKey(TModifierState state);
	void						DownArrowKey(TModifierState state);
	
	virtual TCoord				GetScrollIncrement(TScrollDirection direction) const;
	virtual TCoord				GetPageIncrement(TScrollDirection direction) const;
	
	void						ComputeContentSize();
	
	virtual void				DoSetupMenu(TMenu* menu);
	virtual bool				DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	virtual void				DoIdle();

	inline bool					HasUndo() const { return fUndoRedoIndex >= 0; }
	inline bool					HasRedo() const { return fUndoRedoList.GetSize() > 0 && fUndoRedoIndex < fUndoRedoList.GetSize() - 1; }

	void						SetLineEndingFormat(TLineEndingFormat format);

	void						SaveUndo(STextOffset newSelectionStart, STextOffset newSelectionEnd, bool accumulateTyping, bool accumulateDeletion);
	void						SaveUndoMouseCopy(STextOffset offset, STextOffset length, UndoType undoType);
	void						Undo();
	void						Redo();

	static void					AdjustOffsetsCallback(STextOffset offset, int shift, void* callbackData);
	void						AdjustOffsets(STextOffset offset, int shift);
	
	void						SetIMLocation();

protected:
	TTextLayout*				fLayout;
	TFont*						fFont;
	TRect						fInset;
	STextOffset					fSelectionStart;
	STextOffset					fSelectionEnd;
	STextOffset					fSelectionAnchor;
	STextOffset					fSelectionAnchorEnd;	// only used for tracking double and triple clicks
	STextOffset					fMouseCopyLocation;
	int							fTrackingClickCount;	// if > 0, we are tracking the mouse, and this is the click count
	bool						fInsertionPointOn;
	TCoord						fUpDownHorizOffset;		// for up/down arrow keys
	int							fSpacesPerTab;			// if non-zero, number of spaces per tab stop
	bool						fAutoIndent;

	// undo/redo support
	TList<UndoRedoData>			fUndoRedoList;
	int							fUndoRedoIndex;
	int							fSavedUndoRedoIndex;				// fUndoRedoIndex when document was last saved
	bool						fAccumulateTyping;
	bool						fAccumulateDeletion;

	bool 						fModifiable;
	bool						fMultiLine;
	bool						fLineWrap;
	bool						fFilterTabAndCR;
	bool						fHideInsertionPointWhenNotTarget;	// if false, will show insertion point when not target
	bool						fCursorHidden;						// cursor obscured due to typing

	TString						fLastSelection;
	TMouseTrackingIdler*		fMouseTrackingIdler;
	static TCursor*				sCursor;
};

#endif // __TTextView__
