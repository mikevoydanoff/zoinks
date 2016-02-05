// ========================================================================================
//	TTextLayout.h			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TTextLayout__
#define __TTextLayout__

#include "TGeometry.h"
#include "TString.h"

class TFont;
class TTextLayout;


typedef uint32 STextOffset;

struct LineRec;

typedef void (* ShiftTextProc)(STextOffset offset, int shift, void* callbackData);
typedef void (* LinesInsertedProc)(TTextLayout* layout, uint32 line, uint32 count, void* clientData);
typedef void (* LinesDeletedProc)(TTextLayout* layout, uint32 line, uint32 count, void* clientData);


class TTextLayout
{		
public:
								TTextLayout(TFont* font, const TPoint& inset, int spacesPerTab, bool multiLine, bool lineWrap);		
	virtual						~TTextLayout();

	void						SetText(TChar* text, STextOffset length = 0, bool ownsData = false);

	void						GetLine(uint32 line, const TChar*& outText, STextOffset& outLength,
										TCoord& outVertOffset, TCoord& outLineAscent, TCoord& outLineHeight) const;
	const TChar*				GetLineText(uint32 line, STextOffset& outLength, bool ignoreWrappedLines = false) const;	
	STextOffset					GetLineOffset(uint32 line);						
	void						GetLineBounds(uint32 line, TRect& r) const;
								
	STextOffset					PointToOffset(const TPoint& point, bool round = true) const;
	uint32						OffsetToPoint(STextOffset offset, TPoint& point) const;	// returns line

	TCoord						LineToVertOffset(uint32 line) const;
	uint32						VertOffsetToLine(TCoord vertOffset) const;

	STextOffset					LineToOffset(uint32 line, bool ignoreWrappedLines = false) const;
	uint32						OffsetToLine(STextOffset offset, bool ignoreWrappedLines = false) const;
	uint32						OffsetToColumn(STextOffset offset) const;

	void						ReplaceText(STextOffset offset, STextOffset endOffset, const TChar* text, STextOffset length,
											uint32& outRedrawLinesStart, uint32& outRedrawLinesEnd);

	inline const TChar*			GetText() const { return fText; }
	inline STextOffset			GetTextLength() const { return fTextLength; }
	inline uint32				GetLineCount() const { return fLineCount; }
	
	TCoord						GetLineAscent(uint32 line) const;
	TCoord						GetLineHeight(uint32 line) const;
	TCoord						GetLineWidth(uint32 line) const;
	inline TCoord				GetTabWidth() const { return fTabWidth; }

	void						ComputeContentSize(TPoint& contentSize);

	TCoord						MeasureText(const TChar* text, int length, TCoord leftInset) const;
	TCoord						MeasureText(const TChar* text, int length, TCoord& ascent, TCoord& height, TCoord leftInset) const;

	bool						BalanceCharacter(STextOffset offset, STextOffset& outStart, STextOffset& outEnd) const;
	bool						BalanceSelection(STextOffset& start, STextOffset& end) const;

	bool						FindWord(STextOffset offset, STextOffset& outStart, STextOffset& outEnd);

	inline void					SetInset(const TPoint& inset) { fInset = inset; }

	int							GetCharacterLength(const TChar* text) const;
	void						PreviousCharacter(const TChar*& text) const;
	void						NextCharacter(const TChar*& text) const;
	void						PreviousCharacter(STextOffset& offset) const;
	void						NextCharacter(STextOffset& offset) const;

	inline TLineEndingFormat	GetLineEndingFormat() const { return fLineEndingFormat; }
	const TChar*				GetLineEndingString() const;
	void						SetLineEndingFormat(TLineEndingFormat format, ShiftTextProc shiftTextCallback, void* shiftTextCallbackData);
	
	inline bool					HasLineWrap() const { return fLineWrap; }
	void						SetLineWrap(bool lineWrap, TCoord width);
	void						SetWidth(TCoord width);
	
	inline void					SetLineChangeCallbacks(LinesInsertedProc insertProc, LinesDeletedProc deleteProc, void* clientData)
										{ fLinesInsertedProc = insertProc; fLinesDeletedProc = deleteProc; fLineChangeClientData = clientData; }

protected:
	void						NewLineBreak(STextOffset offset);
	void						NewLineBreak(STextOffset offset, TCoord vertOffset, TCoord ascent, TCoord height, TCoord width);	// this variant fills in values for previous line

	LineRec* 					InsertLineRecs(uint32 line, uint32 count);
	void						DeleteLineRecs(uint32 line, uint32 count);
	void						RecalcLineBreaks();
	void						RecalcWrappedLineBreaks(uint32 startLine, int32 textDiff, uint32 changeOffset, uint32& outRedrawLinesEnd);
	LineRec&					GetLineRec(uint32 line, bool ignoreWrappedLines) const;

	void						OffsetLinesBelow(uint32 line, STextOffset textDelta, TCoord vertDelta);
	
	bool 						BalanceLeft(const TChar* text, STextOffset& outStart, STextOffset& outEnd, TChar balanceChar, bool stopAtLineBreak, bool excludeEdges) const;
	bool 						BalanceRight(const TChar* text, STextOffset& outStart, STextOffset& outEnd, TChar balanceChar, bool stopAtLineBreak, bool excludeEdges) const;

	static const TChar* 		FindLineBreak(const TChar* text, const TChar* textEnd);
	static uint32				CountLineBreaks(const TChar* text, const TChar* textEnd);

protected:
	TChar*						fText;
	STextOffset					fTextLength;	// number of TChars (not bytes)
	LineRec*					fLineBreaks;
	STextOffset					fLineCount;
	TFont*						fFont;
	TPoint						fInset;
	int							fSpacesPerTab;
	TCoord						fTabWidth;
	TCoord						fWidth;			// width of containing view (for line wrap)
	TLineEndingFormat			fLineEndingFormat;
	bool						fMultiLine;
	bool						fLineWrap;	
	LinesInsertedProc			fLinesInsertedProc;
	LinesDeletedProc			fLinesDeletedProc;
	void*						fLineChangeClientData;
};

inline TCoord TTextLayout::MeasureText(const TChar* text, int length, TCoord leftInset) const
{
	TCoord ascent, height;
	return MeasureText(text, length, ascent, height, leftInset);
}


#endif // __TTextLayout__
