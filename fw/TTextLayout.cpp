// ========================================================================================
//	TTextLayout.cpp		 		Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TTextLayout.h"
#include "TException.h"
#include "TFont.h"
#include "TString.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>


struct LineRec
{
	STextOffset		textOffset;
	TCoord			vertOffset;
	TCoord			ascent;
	TCoord			height;
	TCoord			width;
};


static inline bool IsIdentifierChar(char ch)
{
	return (isalnum(ch) || ch == '_');
}


static inline int Tmblen(const char* s, size_t n)
{
	int result = mblen(s, n);
	if (result > 0)
		return result;
	else
		return 1;
}


static inline bool AllowBreakAfter(const TChar* text)
{
	return (isspace(text[-1]) && !isspace(text[0]));
}


TTextLayout::TTextLayout(TFont* font, const TPoint& inset, int spacesPerTab, bool multiLine, bool lineWrap)
	:	fText(NULL),
		fTextLength(0),
		fLineBreaks(NULL),
		fLineCount(0),
		fFont(font),
		fInset(inset),
		fSpacesPerTab(spacesPerTab),
		fLineEndingFormat(kUnixLineEndingFormat),
		fMultiLine(multiLine),
		fLineWrap(lineWrap),
		fLinesInsertedProc(NULL),
		fLinesDeletedProc(NULL),
		fLineChangeClientData(NULL)
{
	ASSERT(font);
	font->AddRef();

	char* spaces = new char[spacesPerTab + 1];
	memset(spaces, ' ', spacesPerTab);
	spaces[spacesPerTab] = 0;
	fTabWidth = font->MeasureText(spaces);
	delete[] spaces;

	// initialize layout with one empty line
	RecalcLineBreaks();
}


TTextLayout::~TTextLayout()
{
	fFont->RemoveRef();
	SetText(NULL);
}


void TTextLayout::SetText(TChar* text, STextOffset length, bool ownsData)
{
	if (fText)
	{
		free(fText);
		fText = NULL;
	}

	fTextLength = 0;
	fLineCount = 0;

	if (text)
	{
		if (length == 0)
			length = Tstrlen(text);

		if (ownsData)
			fText = text;
		else if (length > 0)
		{
			fText = (TChar *)malloc(length * sizeof(TChar));
			ASSERT(fText);
			memcpy(fText, text, length);
		}
		
		fTextLength = length;

		RecalcLineBreaks();
	}
}


void TTextLayout::GetLine(uint32 line, const TChar*& outText, STextOffset& outLength, 
						  TCoord& outVertOffset, TCoord& outLineAscent, TCoord& outLineHeight) const
{
	ASSERT(line >= 0 && line < fLineCount);
	LineRec& rec =  fLineBreaks[line];
	outText = fText + rec.textOffset;
	outLength = (line == fLineCount - 1 ? fTextLength - rec.textOffset : fLineBreaks[line + 1].textOffset - rec.textOffset);

	if (outLength > 0)
	{
		if (outText[outLength - 1] == kLineEnd10)
		{
			if (outLength > 1 && outText[outLength - 2] == kLineEnd13)
				outLength -= 2;
			else
				outLength -= 1;
		}
		else if (outText[outLength - 1] == kLineEnd13)
			outLength -= 1;
	}

	outVertOffset = rec.vertOffset;
	outLineAscent = rec.ascent;
	outLineHeight = rec.height;
}


STextOffset TTextLayout::GetLineOffset(uint32 line)
{ 
	ASSERT(line >= 0 && line < fLineCount);
	LineRec& rec =  fLineBreaks[line];
	return rec.textOffset;
}


const TChar* TTextLayout::GetLineText(uint32 line, STextOffset& outLength, bool ignoreWrappedLines) const							
{
	ASSERT(line >= 0 && line < fLineCount);

	LineRec& rec = GetLineRec(line, ignoreWrappedLines);
	
	const TChar* result = fText + rec.textOffset;
	const TChar* endText = FindLineBreak(result, fText + fTextLength);

	outLength = (endText ? endText - result : fText + fTextLength - result);

	if (outLength > 0)
	{
		if (result[outLength - 1] == kLineEnd10)
		{
			if (outLength > 1 && result[outLength - 2] == kLineEnd13)
				outLength -= 2;
			else
				outLength -= 1;
		}
		else if (result[outLength - 1] == kLineEnd13)
			outLength -= 1;
	}

	return result;
}


LineRec& TTextLayout::GetLineRec(uint32 line, bool ignoreWrappedLines) const
{
	if (ignoreWrappedLines && fLineWrap)
	{
		uint32 currentLine = 0;
		
		for (uint32 index = 0; index < fLineCount; index++)
		{
			if (currentLine == line)
				return fLineBreaks[index];
			
			STextOffset lineEnd = (index == fLineCount - 1 ? fTextLength : fLineBreaks[index + 1].textOffset);
			
			// check for hard line break
			TChar end = fText[lineEnd - 1];
			if (end == '\n' || end == '\r')
				currentLine++;
		}
		
		return fLineBreaks[fLineCount - 1];
	}
	else
		return fLineBreaks[line];
}	


void TTextLayout::GetLineBounds(uint32 line, TRect& r) const
{
	ASSERT(line < fLineCount);

	LineRec& rec = fLineBreaks[line];
	r.left = fInset.h;
	r.top = rec.vertOffset + fInset.v;
	r.right = r.left + rec.width;
	r.bottom = r.top + rec.height;
}


STextOffset TTextLayout::PointToOffset(const TPoint& point, bool round) const
{
	TCoord h = point.h - fInset.h;
	if (h < 0)
		h = 0;
	TCoord v = point.v - fInset.v;
	if (v < 0)
		v = 0;

	uint32 line;
	for (line = 0; line < fLineCount; line++)
	{
		if (fLineBreaks[line].vertOffset > v)
		{
			if (line > 0)
				line--;
			break;
		}
	}
	
	if (line >= fLineCount)
		line = fLineCount - 1;
		
	STextOffset result = LineToOffset(line);
	
	const TChar*	lineStart;
	STextOffset		lineLength;
	TCoord			vertOffset, ascent, height;
	
	GetLine(line, lineStart, lineLength, vertOffset, ascent, height);
	if (lineLength == 0)
		return result;

	const TChar* 	lineEnd = lineStart + lineLength;
	const TChar*	text = lineStart;
	const TChar*	lastText = text;
	TCoord			horizOffset = 0;
	TCoord			lastHorizOffset = 0;
	
	while (text < lineEnd)
	{
		NextCharacter(text);	

		TCoord horizOffset = MeasureText(lineStart, text - lineStart, 0);
		
		if (horizOffset > h)
		{
			if (round && h - lastHorizOffset + 1 >= horizOffset - h)
				return result + (text - lineStart);
			else
				return result + (lastText - lineStart);
		}
		
		lastHorizOffset = horizOffset;
		lastText = text;
	}

	if (round && h - lastHorizOffset + 1 >= horizOffset - h)
		return result + (text - lineStart);
	else
		return result + (lastText - lineStart);
}


TCoord TTextLayout::LineToVertOffset(uint32 line) const
{
	ASSERT(fLineBreaks);
	ASSERT(line < fLineCount);

	return fLineBreaks[line].vertOffset + fInset.v;
}


uint32 TTextLayout::VertOffsetToLine(TCoord vertOffset) const
{
	vertOffset -= fInset.v;
	if (vertOffset < 0)
		vertOffset = 0;

	uint32 line;
	for (line = 0; line < fLineCount; line++)
	{
		if (fLineBreaks[line].vertOffset > vertOffset)
		{
			if (line > 0)
				line--;
			break;
		}
	}
	
	if (line >= fLineCount)
		line = fLineCount - 1;

	return line;
}


uint32 TTextLayout::OffsetToPoint(STextOffset offset, TPoint& point) const
{
	uint32 line = OffsetToLine(offset);
	ASSERT(line < fLineCount);

	LineRec& rec = fLineBreaks[line];
	point.v = rec.vertOffset + rec.ascent + fInset.v;	
	if (offset == rec.textOffset)
		point.h = fInset.h;
	else
		point.h = MeasureText(fText + rec.textOffset, offset - rec.textOffset, 0) + fInset.h;
	
	return line;
}


STextOffset TTextLayout::LineToOffset(uint32 line, bool ignoreWrappedLines) const
{
	if (fLineBreaks && line < fLineCount)
	{
		LineRec& rec = GetLineRec(line, ignoreWrappedLines);
		return rec.textOffset;
	}
	else
		return fTextLength;
}


uint32 TTextLayout::OffsetToLine(STextOffset offset, bool ignoreWrappedLines) const
{
	if (fText && fLineBreaks)
	{
		if (ignoreWrappedLines && fLineWrap)
		{
			uint32 result = 0;
			
			for (uint32 index = 0; index < fLineCount; index++)
			{
				STextOffset testLow = fLineBreaks[index].textOffset;
				STextOffset testHigh = (index == fLineCount - 1 ? fTextLength : fLineBreaks[index + 1].textOffset);
				
				if (offset >= testLow && offset < testHigh)
					return result;
				
				// check for hard line break
				TChar end = fText[testHigh - 1];
				if (end == '\n' || end == '\r')
					result++;
			}
		
			return result;
		}
		else
		{
			uint32 low = 0;
			uint32 high = fLineCount;
			uint32 index = 0;
			
			while (low < high)
			{
				index = (low + high) >> 1;
				STextOffset testLow = fLineBreaks[index].textOffset;
				STextOffset testHigh = (index == fLineCount - 1 ? fTextLength : fLineBreaks[index + 1].textOffset);
				
				if (offset < testLow)
					high = index;
				else if (testHigh <= offset)
					low = index + 1;	// item is greater than index
				else 
					low = high = index;
			}
			
			ASSERT(fLineBreaks[index].textOffset <= offset);
			ASSERT(index == fLineCount -1 || fLineBreaks[index + 1].textOffset > offset);
			
			return index;
		}
	}
	else
		return 0;	
}


uint32 TTextLayout::OffsetToColumn(STextOffset offset) const
{
	if (offset > fTextLength)
		offset = fTextLength;
	const TChar* text = fText + LineToOffset(OffsetToLine(offset));
	const TChar* end = fText + offset;
	uint32 column = 0;

	while (text < end)
	{
		if (*text == '\t')
			column = ((column + fSpacesPerTab - 1) / fSpacesPerTab) * fSpacesPerTab;
		else
			column++;
		NextCharacter(text);
	}

	return column;
}


TCoord TTextLayout::GetLineAscent(uint32 line) const
{
	ASSERT(fLineBreaks);
	ASSERT(line < fLineCount);

	return fLineBreaks[line].ascent;
}


TCoord TTextLayout::GetLineHeight(uint32 line) const
{
	ASSERT(fLineBreaks);
	ASSERT(line < fLineCount);

	return fLineBreaks[line].height;
}


TCoord TTextLayout::GetLineWidth(uint32 line) const
{
	ASSERT(fLineBreaks);
	ASSERT(line < fLineCount);

	return fLineBreaks[line].width;
}


void TTextLayout::ReplaceText(STextOffset start, STextOffset end, const TChar* text, STextOffset length,  
								uint32& outRedrawLinesStart, uint32& outRedrawLinesEnd)
{
	ASSERT(start <= end);
	
	uint32 oldLineCount = fLineCount;
	uint32 deletedLines = (fText ? CountLineBreaks(fText + start, fText + end) : 0);

	int32 textDelta = length - (end - start);

	if (textDelta != 0)
	{
		if (textDelta < 0)
			memmove(fText + end + textDelta, fText + end, fTextLength - end);

		size_t newSize = fTextLength + textDelta;
		if (newSize > 0)
		{
			fText = (TChar *)realloc(fText, fTextLength + textDelta);
		}
		else
		{
			free(fText);
			fText = NULL;
		}

		if (textDelta > 0)
			memmove(fText + end + textDelta, fText + end, fTextLength - end);
			
		fTextLength += textDelta;
	}

	// special case deleting all the text
	if (!fText)
	{
		RecalcLineBreaks();
		outRedrawLinesStart = outRedrawLinesEnd = 0;
		return;
	}
	
	memcpy(fText + start, text, length);
	
	uint32 startLine = OffsetToLine(start);

	if (fLineWrap)
	{
		// find the real beginning of the line
		while (startLine > 0)
		{
			LineRec& rec = fLineBreaks[startLine];
			TChar ch = fText[rec.textOffset - 1];
			
			if (ch != '\n' && ch != '\r')
				startLine--;
			else
				break;
		}
		
		outRedrawLinesStart = startLine;
		
		int32 textDiff = length - (end - start);
		uint32 changeOffset = (start + length > end ? start + length : end);
		RecalcWrappedLineBreaks(startLine, textDiff, changeOffset, outRedrawLinesEnd);
	}
	else
	{
		// no line wrap

		uint32 line = startLine;
		LineRec* rec = &fLineBreaks[line];
	
		text = fText + start;
		const TChar* textEnd = text + length;
		bool gotLineBreak = false;
	
		if (fMultiLine)
		{
			while (text < textEnd)
			{
				const TChar* nextLine = FindLineBreak(text, textEnd);
	
				if (nextLine)
				{
					gotLineBreak = true;
					text = nextLine;
	
					++line;
					if (deletedLines > 0)
					{
						--deletedLines;		// reuse deleted line
						++rec;
					}
					else
						rec = InsertLineRecs(line, 1);
	
					rec->textOffset = text - fText;
				}
				else
					break;
			}

			outRedrawLinesStart = startLine;

			if (deletedLines > 0)
				DeleteLineRecs(line + 1, deletedLines);
			
			if (fLineCount != oldLineCount)
				outRedrawLinesEnd = fLineCount - 1;
			else
				outRedrawLinesEnd = startLine + CountLineBreaks(fText + start, fText + start + length);;
		}
		else
		{
			outRedrawLinesStart = outRedrawLinesEnd = 0;
		}
		
		uint32 endLine = line;
		rec = &fLineBreaks[startLine];
	
		TCoord vertOffset = (startLine > 0 ? rec[-1].vertOffset + rec[-1].height : 0);
		
		for (line = startLine; line <= endLine; line++)
		{	
			const TChar* text = fText + rec->textOffset;
			int32 length = fTextLength - rec->textOffset;
			for (int32 i=0; i<length; i++)
			{
				if (text[i] == kLineEnd10 || text[i] == kLineEnd13)
				{
					length = i;
					break;
				}
			}	
	
			if (length)
			{
				rec->width = MeasureText(text, length, rec->ascent, rec->height, 0);
			}
			else
			{
				// height of an empty line
				fFont->MeasureText("W", 1, rec->ascent, rec->height);
				rec->width = 0;
			}
			
			rec->vertOffset = vertOffset;
			vertOffset += rec->height;
			
			++rec;
		}
	
		if (endLine < fLineCount - 1)
		{
			TCoord vertDelta = vertOffset - rec->vertOffset;
			OffsetLinesBelow(endLine, textDelta, vertDelta);
		}
	}

#if 0
	// validate
	if (fText)
	{
		LineRec* rec = fLineBreaks;
		for (uint32 line = 0; line < fLineCount; line++, rec++)
		{
			if (line > 0)
			{
				ASSERT(rec[-1].textOffset < rec->textOffset);
				ASSERT(rec[-1].vertOffset < rec->vertOffset);
			}
			
			const TChar* text = fText + rec->textOffset;
			int32 length = (line == fLineCount - 1 ? fTextLength - rec->textOffset : rec[1].textOffset - rec->textOffset);
			int32 i;
			
			if (!fLineWrap)
			{
				for (i = 0; i < length - 2; i++)	// two, to allow for DOS line endings
				{
					ASSERT(text[i] != kLineEnd10 && text[i] != kLineEnd13);
				}
				
				ASSERT(text[length - 1] == kLineEnd10 || text[length - 1] == kLineEnd13 || line == fLineCount - 1);
			}
		}
	}
#endif // 0
}


void TTextLayout::ComputeContentSize(TPoint& contentSize)
{
	TCoord contentWidth = 1;	// minimum one pixel wide for insertion point
	uint32 line;

	for (line = 0; line < fLineCount; line++)
	{
		TCoord width = fLineBreaks[line].width;
		if (width > contentWidth)
			contentWidth = width;
	}

	contentSize.h = contentWidth;
	if (fLineCount > 0)
		contentSize.v =  fLineBreaks[fLineCount - 1].vertOffset + fLineBreaks[fLineCount - 1].height;
	else
		contentSize.v = 0;
}


LineRec* TTextLayout::InsertLineRecs(uint32 line, uint32 count)
{
	fLineBreaks = (LineRec *)realloc(fLineBreaks, (fLineCount + count) * sizeof(LineRec));
	memmove(&fLineBreaks[line + count], &fLineBreaks[line], (fLineCount - line) * sizeof(LineRec));
	fLineCount += count;

	if (fLinesInsertedProc)
		fLinesInsertedProc(this, line, count, fLineChangeClientData);
		
	return &fLineBreaks[line];
}


void TTextLayout::DeleteLineRecs(uint32 line, uint32 count)
{
	ASSERT(line + count <= fLineCount);
	
	memmove(&fLineBreaks[line], &fLineBreaks[line + count], (fLineCount - line - count) * sizeof(LineRec));
	fLineBreaks = (LineRec *)realloc(fLineBreaks, (fLineCount - count) * sizeof(LineRec));
	fLineCount -= count;
	
	if (fLinesDeletedProc)
		fLinesDeletedProc(this, line, count, fLineChangeClientData);
}


int TTextLayout::GetCharacterLength(const TChar* text) const
{
	ASSERT(text >= fText && text <= fText + fTextLength - 1);

	char ch = *text;
	if (ch == kLineEnd13 && text <= fText + fTextLength - 2 && text[1] == kLineEnd10)
		return 2;
	else
		return Tmblen(text, fTextLength - (text - fText));
}


void TTextLayout::PreviousCharacter(const TChar*& text) const
{
	ASSERT(text >= fText + 1 && text <= fText + fTextLength);

	if (text[-1] == kLineEnd10 && text + 1 > fText && text[-2] == kLineEnd13)
		text -= 2;
	else
	{
		// fix me - assuming maximum multibyte character size is 2
		if (text > fText + 1  && Tmblen(text - 2, 2) == 2)
			text -= 2;
		else
			text -= 1;
	}
}


void TTextLayout::NextCharacter(const TChar*& text) const
{
	ASSERT(text >= fText && text < fText + fTextLength);

	if (text[0] == kLineEnd13 && text < fText + fTextLength - 1 && text[1] == kLineEnd10)
		text += 2;
	else
		text += Tmblen(text, fTextLength - (text - fText));
}


void TTextLayout::PreviousCharacter(STextOffset& offset) const
{
	ASSERT(offset >= 1 && offset <= fTextLength);

	if (fText[offset - 1] == kLineEnd10 && offset > 1 && fText[offset - 2] == kLineEnd13)
		offset -= 2;
	else
	{
		// fix me - assuming maximum multibyte character size is 2
		if (offset > 1 && Tmblen(&fText[offset - 2], 2) == 2)
			offset -= 2;
		else
			offset -= 1;
	}
}


void TTextLayout::NextCharacter(STextOffset& offset) const
{
	ASSERT(offset >= 0 && offset < fTextLength);

	if (fText[offset] == kLineEnd13 && offset < fTextLength - 1 && fText[offset + 1] == kLineEnd10)
		offset += 2;
	else
		offset += Tmblen(&fText[offset], fTextLength - offset);
}


const TChar* TTextLayout::GetLineEndingString() const
{
	static const TChar unixEnding[] = { kLineEnd10, 0 };
	static const TChar macEnding[] =  { kLineEnd13, 0 };
	static const TChar dosEnding[] =  { kLineEnd13, kLineEnd10, 0 };
	
	switch (fLineEndingFormat)
	{
		case kUnixLineEndingFormat:
			return unixEnding;
			
		case kMacLineEndingFormat:
			return macEnding;

		case kDOSLineEndingFormat:
			return dosEnding;

		default:
			ASSERT(0);
	}

	return NULL;	// shouldn't happen
}


void TTextLayout::SetLineEndingFormat(TLineEndingFormat format, ShiftTextProc shiftTextCallback, void* shiftTextCallbackData)
{
	ASSERT(format == kUnixLineEndingFormat || format == kDOSLineEndingFormat || format == kMacLineEndingFormat);
	
	if (fTextLength > 0)
	{
		if (format == kDOSLineEndingFormat)
		{
			// allocate space for longer line endings
			fText = (TChar *)realloc(fText, fTextLength + fLineCount);
			ASSERT(fText);
		}
		
		STextOffset		shift = 0;
		STextOffset 	textLength = fTextLength;
	
		for (uint32 line = 0; line < fLineCount; line++)
		{
			LineRec& rec = fLineBreaks[line];
			rec.textOffset += shift;
			STextOffset lineStart = rec.textOffset;
	
			STextOffset lineEnd;
			if (line < fLineCount - 1)
			{
				lineEnd = fLineBreaks[line + 1].textOffset + shift;
				ASSERT(lineEnd <= textLength);
			} 
			else
				lineEnd = textLength;
	
			ASSERT(lineEnd >= lineStart);
			if (lineEnd == lineStart)
			{
				ASSERT(line == fLineCount - 1);
				break;
			}
	
			TChar* endText = fText + lineEnd - 1;
	
			TLineEndingFormat	currentFormat = kUnixLineEndingFormat;
			
			if (endText[0] == kLineEnd13)
				currentFormat = kMacLineEndingFormat;
			else if (endText[0] == kLineEnd10)
			{
				if (lineEnd - 1 > lineStart && endText[-1] == kLineEnd13)
					currentFormat = kDOSLineEndingFormat;
				else
					currentFormat = kUnixLineEndingFormat;
			}
			else if (line == fLineCount - 1)
				break;
			else
				ASSERT(0);
	
			if (format != currentFormat)
			{
				if (format == kDOSLineEndingFormat)
				{
					memmove(endText + 2, endText + 1, textLength - lineEnd);
					endText[0] = kLineEnd13;
					endText[1] = kLineEnd10;
					
					shift += 1;
					textLength += 1;
					shiftTextCallback(lineEnd, 1, shiftTextCallbackData);
				}
				else if (currentFormat == kDOSLineEndingFormat)
				{
					memmove(endText - 1, endText, textLength - lineEnd + 1);
					endText[-1] = (format == kUnixLineEndingFormat ? kLineEnd10 : kLineEnd13);
					
					shiftTextCallback(lineEnd, -1, shiftTextCallbackData);
					shift -= 1;
					textLength -= 1;
				}
				else
				{
					endText[0] = (format == kUnixLineEndingFormat ? kLineEnd10 : kLineEnd13);
				}
			}
		}
	
		// adjust text size again
		fText = (TChar *)realloc(fText, textLength);
		ASSERT(fText);
		fTextLength = textLength;
	}

	fLineEndingFormat = format;
}


void TTextLayout::NewLineBreak(STextOffset offset)
{
	fLineBreaks = (LineRec *)realloc(fLineBreaks, (fLineCount + 1) * sizeof(LineRec));
	ASSERT(fLineBreaks);

	// only for adding a new line break at the end!
	ASSERT(fLineCount == 0 || offset > fLineBreaks[fLineCount - 1].textOffset);
	LineRec& rec = fLineBreaks[fLineCount];
	rec.textOffset = offset;
	rec.vertOffset = 0;
	rec.ascent = 0;
	rec.height = 0;
	rec.width = 0;
	fLineCount++;

	ASSERT(fMultiLine || fLineCount == 1);
}

void TTextLayout::NewLineBreak(STextOffset offset, TCoord vertOffset, TCoord ascent, TCoord height, TCoord width)
{
	if (fLineCount > 0)
	{
		// fill in values for previous line
		LineRec& rec = fLineBreaks[fLineCount - 1];
		rec.vertOffset = vertOffset;
		rec.ascent = ascent;
		rec.height = height;
		rec.width = width;
	}
	
	NewLineBreak(offset);
}


void TTextLayout::RecalcLineBreaks()
{
	if (fLineBreaks)
	{
		free(fLineBreaks);
		fLineBreaks = NULL;
	}
	fLineCount = 0;

	bool foundLineBreak = false;
	
	TChar* text = fText;
	TChar* textStart = text;
	TChar ch;

	NewLineBreak(0);

	if (textStart)
	{
		if (fMultiLine)
		{
			TCoord	width = 0;
			TCoord	vert = 0;
			TCoord	ascent, height;
			TChar* lineStart = text;
			TChar* textEnd = text + fTextLength;
			TChar* lastBreak = NULL;
			
			while (text < textEnd)
			{
				ch = *text;
	
				if (ch == kLineEnd13)
				{
					text++;
					
					if (text < textEnd && *text == kLineEnd10)
					{
						text++;
	
						if (!foundLineBreak)
						{
							fLineEndingFormat = kDOSLineEndingFormat;
							foundLineBreak = true;
						}
					}
					else if (!foundLineBreak)
					{
						fLineEndingFormat = kMacLineEndingFormat;
						foundLineBreak = true;
					}
					
					width = MeasureText(lineStart, text - lineStart, ascent, height, 0);
	
					NewLineBreak(text - textStart, vert, ascent, height, width);
					vert += height;
					width = 0;
					lineStart = text;
					lastBreak = NULL;
				}
				else if (ch == kLineEnd10)
				{
					text++;
					
					width = MeasureText(lineStart, text - lineStart, ascent, height, 0);
	
					NewLineBreak(text - textStart, vert, ascent, height, width);
					vert += height;
					width = 0;
					lineStart = text;
					lastBreak = NULL;
					
					if (!foundLineBreak)
					{
						fLineEndingFormat = kUnixLineEndingFormat;
						foundLineBreak = true;
					}
				}
				else
				{
					TChar* charStart = text;
					text += Tmblen(text, fTextLength - (text - textStart));
	
					if (fLineWrap)
					{
						if (charStart[0] == '\t')
							width = ((width + fTabWidth) / fTabWidth) * fTabWidth;
						else
							width += fFont->MeasureText(charStart, text - charStart);
							
						if (charStart > lineStart)
						{
							if (width >= fWidth)
							{
								if (lastBreak)
									text = lastBreak;
								else
									text = charStart;
								
								width = MeasureText(lineStart, text - lineStart, ascent, height, 0);
								NewLineBreak(text - textStart, vert, ascent, height, width);
								vert += height;
								width = 0;
								lineStart = text;
								lastBreak = NULL;
							}
							else if (text > textStart && AllowBreakAfter(text))
								lastBreak = text;
						}
					}
				}
			}
			
			// finish last line
			if (fLineCount > 0)
			{
				width = MeasureText(lineStart, text - lineStart, ascent, height, 0);

				LineRec& rec = fLineBreaks[fLineCount - 1];
				rec.vertOffset = vert;
				rec.ascent = ascent;
				rec.height = height;
				rec.width = width;
			}
		}
		else
		{
			LineRec& rec = fLineBreaks[0];
			
			rec.width = MeasureText(text, fTextLength, rec.ascent, rec.height, 0);
			rec.vertOffset = 0;
		}
	}
	
	if (!foundLineBreak)
		fLineEndingFormat = kUnixLineEndingFormat;

	if (!textStart)
	{
		LineRec& rec = fLineBreaks[0];
		fFont->MeasureText("W", 1, rec.ascent, rec.height);
		rec.width = 0;
		rec.vertOffset = 0;
	}
}


void TTextLayout::RecalcWrappedLineBreaks(uint32 startLine, int32 textDiff, uint32 changeOffset, uint32& outRedrawLinesEnd)
{
	ASSERT(fMultiLine && fLineWrap);

	uint32 startOffset = LineToOffset(startLine);
	uint32 oldLineCount = fLineCount;
	LineRec* oldLineBreaks = fLineBreaks;
	ASSERT(oldLineBreaks);
	size_t length = startLine * sizeof(LineRec);
	fLineBreaks = (LineRec *)malloc(length);
	if (!fLineBreaks)
		ThrowProgramError("out of memory!");
	memcpy(fLineBreaks, oldLineBreaks, length);
	fLineCount = startLine;

	TChar* textStart = fText;
	ASSERT(textStart);
	TChar* text = textStart + startOffset;

	NewLineBreak(startOffset);

	TCoord	width = 0;
	TCoord	vert = (startLine > 0 ? fLineBreaks[startLine - 1].vertOffset + fLineBreaks[startLine - 1].height : 0);
	TCoord	ascent, height;
	TChar* lineStart = text;
	TChar* textEnd = textStart + fTextLength;
	TChar* lastBreak = NULL;
	TChar ch;
	
	while (text < textEnd)
	{
		ch = *text;

		if (ch == kLineEnd13)
		{
			text++;
			
			if (text < textEnd && *text == kLineEnd10)
			{
				text++;
			}

			width = MeasureText(lineStart, text - lineStart, ascent, height, 0);
			NewLineBreak(text - textStart, vert, ascent, height, width);
			vert += height;
			width = 0;
			lineStart = text;
			lastBreak = NULL;
		}
		else if (ch == kLineEnd10)
		{
			text++;
			
			width = MeasureText(lineStart, text - lineStart, ascent, height, 0);
			NewLineBreak(text - textStart, vert, ascent, height, width);
			vert += height;
			width = 0;
			lineStart = text;
			lastBreak = NULL;
		}
		else
		{
			TChar* charStart = text;
			text += Tmblen(text, fTextLength - (text - textStart));

			if (charStart[0] == '\t')
				width = ((width + fTabWidth) / fTabWidth) * fTabWidth;
			else
				width += fFont->MeasureText(charStart, text - charStart);

			if (charStart > lineStart)
			{
				if (width >= fWidth)
				{
					if (lastBreak)
						text = lastBreak;
					else
						text = charStart;
					
					width = MeasureText(lineStart, text - lineStart, ascent, height, 0);
					NewLineBreak(text - textStart, vert, ascent, height, width);
					vert += height;
					width = 0;
					lineStart = text;
					lastBreak = NULL;
				}
				else if (text > textStart && AllowBreakAfter(text))
					lastBreak = text;
			}
		}
	}
	
	// finish last line
	if (fLineCount > 0)
	{
		width = MeasureText(lineStart, text - lineStart, ascent, height, 0);

		LineRec& rec = fLineBreaks[fLineCount - 1];
		rec.vertOffset = vert;
		rec.ascent = ascent;
		rec.height = height;
		rec.width = width;
	}

	outRedrawLinesEnd = fLineCount - 1;
	
	if (fLineCount == oldLineCount)
	{
		for (uint32 i = oldLineCount - 1; i > startLine; i--)
		{
			uint32 oldOffset = oldLineBreaks[i].textOffset;
			uint32 newOffset = fLineBreaks[i].textOffset;
			
			if (oldOffset >= changeOffset && newOffset >= changeOffset && 
				oldOffset + textDiff == newOffset)
			{
				outRedrawLinesEnd = i - 1;
			}
			else
				break;
		}	
	}
	
	free(oldLineBreaks);
}


void TTextLayout::SetLineWrap(bool lineWrap, TCoord width)
{
	if (lineWrap != fLineWrap)
	{
		fLineWrap = lineWrap;
		SetWidth(width);
	}
}


void TTextLayout::SetWidth(TCoord width)
{
	fWidth = width;
	RecalcLineBreaks();
}


void TTextLayout::OffsetLinesBelow(uint32 line, STextOffset textDelta, TCoord vertDelta)
{
	ASSERT(fLineBreaks);

	LineRec* rec = &fLineBreaks[++line];

	while (line < fLineCount)
	{
		rec->textOffset += textDelta;
		rec->vertOffset += vertDelta;
		rec++;
		line++;
	}
}	


const TChar* TTextLayout::FindLineBreak(const TChar* text, const TChar* textEnd)
{
	while (text < textEnd)
	{
		TChar ch = *text++;

		if (ch == kLineEnd13)
		{
			if (text < textEnd && *text == kLineEnd10)
				text++;

			return text;
		}
		else if (ch == kLineEnd10)
			return text;
	}

	return NULL;
}


uint32 TTextLayout::CountLineBreaks(const TChar* text, const TChar* textEnd)
{
	uint32 result = 0;

	while (text < textEnd)
	{
		TChar ch = *text++;

		if (ch == kLineEnd13)
		{			
			if (text < textEnd && *text == kLineEnd10)
				text++;

			++result;
		}
		else if (ch == kLineEnd10)
			++result;
	}

	return result;
}


TCoord TTextLayout::MeasureText(const TChar* text, int length, TCoord& ascent, TCoord& height, TCoord leftInset) const
{
	TCoord offset = leftInset;
	const TChar* start = text;
	const TChar* end = text + length;
	TCoord tabWidth = fTabWidth;
	ascent = height = 0;

	while (start < end)
	{
		// find next tab or end of line
		while (text < end && *text != '\t')
			++text;

		if (text > start)
			offset += fFont->MeasureText(start, text - start, ascent, height);

		int tabCount = 0;
		while (text < end && *text == '\t')
		{
			++tabCount;
			++text;
		}

		if (tabCount > 0)
			offset = ((offset + tabCount * tabWidth) / tabWidth) * tabWidth;	// skip to next tab

		start = text;	
	}

	// make sure we don't have zero height lines.
	if (height == 0)
		fFont->MeasureText("W", 1, ascent, height);
		
	ASSERT(ascent < 200);
	ASSERT(height < 200);

	return (offset - leftInset);
}


bool TTextLayout::BalanceLeft(const TChar* text, STextOffset& outStart, STextOffset& outEnd, TChar balanceChar, bool stopAtLineBreak, bool excludeEdges) const
{
	const TChar* t = text;

	int nestingCount = 0;
	TChar nestingChar = *text;
	if (nestingChar == balanceChar)
		nestingChar = 0;
	
	PreviousCharacter(t);
	while (t > fText)
	{
		TChar ch = *t;
		
		if (ch == nestingChar)
			nestingCount++;
		else if (ch == balanceChar)
		{
		 	if (nestingCount == 0)
				break;
			else if (nestingCount > 0)
				nestingCount--;
		}
		else if (stopAtLineBreak && (ch == '\n' || ch == '\t'))
			break;
		
		PreviousCharacter(t);
	}

	if (*t == balanceChar)
	{
		outStart = t - fText;
		outEnd = text - fText + 1;
		
		if (excludeEdges)
		{
			outStart++;
			outEnd--;
		}
			
		return true;
	}
	else
		return false;
}


bool TTextLayout::BalanceRight(const TChar* text, STextOffset& outStart, STextOffset& outEnd, TChar balanceChar, bool stopAtLineBreak, bool excludeEdges) const
{
	const TChar* t = text;
	const TChar* textEnd = fText + fTextLength;

	int nestingCount = 0;
	TChar nestingChar = *text;
	if (nestingChar == balanceChar)
		nestingChar = 0;
	
	NextCharacter(t);
	while (t < textEnd)
	{
		TChar ch = *t;
		
		if (ch == nestingChar)
			nestingCount++;
		else if (ch == balanceChar)
		{
		 	if (nestingCount == 0)
				break;
			else if (nestingCount > 0)
				nestingCount--;
		}
		else if (stopAtLineBreak && (ch == '\n' || ch == '\t'))
			break;
		
		NextCharacter(t);
	}

	if (*t == balanceChar)
	{
		outStart = text - fText;
		outEnd = t - fText + 1;

		if (excludeEdges)
		{
			outStart++;
			outEnd--;
		}

		return true;
	}
	else
		return false;
}


bool TTextLayout::BalanceCharacter(STextOffset offset, STextOffset& outStart, STextOffset& outEnd) const
{
	if (offset >= fTextLength)
		return false;
	
	const TChar* text = fText + offset;
	TChar ch = text[0];
	
	switch (ch)
	{
		case '<':
			return BalanceRight(text, outStart, outEnd, '>', true, false);

		case '>':
			return BalanceLeft(text, outStart, outEnd, '<', true, false);
			
		case '(':
			return BalanceRight(text, outStart, outEnd, ')', false, true);

		case ')':
			return BalanceLeft(text, outStart, outEnd, '(', false, true);

		case '{':
			return BalanceRight(text, outStart, outEnd, '}', false, true);

		case '}':
			return BalanceLeft(text, outStart, outEnd, '{', false, true);

		case '[':
			return BalanceRight(text, outStart, outEnd, ']', false, true);

		case ']':
			return BalanceLeft(text, outStart, outEnd, '[', false, true);

		case '\'':
			if (BalanceRight(text, outStart, outEnd, '\'', true, true))
				return true;
			else
				return BalanceLeft(text, outStart, outEnd, '\'', true, true);

		case '\"':
			if (BalanceRight(text, outStart, outEnd, '\"', true, true))
				return true;
			else
				return BalanceLeft(text, outStart, outEnd, '\"', true, true);
	}
	
	return false;
}


bool TTextLayout::BalanceSelection(STextOffset& start, STextOffset& end) const
{
	STextOffset	offset = start;
	
	while (offset >= 0)
	{
		STextOffset newStart, newEnd;
		
		if (BalanceCharacter(offset, newStart, newEnd) && 
			newStart <= start && newEnd >= end && 
			!(newStart == start && newEnd == end))
		{
			start = newStart;
			end = newEnd;
			return true;
		}
		else if (offset > 0)
			--offset;
		else
			break;
	}
			
	return false;
}


bool TTextLayout::FindWord(STextOffset offset, STextOffset& outStart, STextOffset& outEnd)
{
	uint32 line = OffsetToLine(offset);
	const TChar* lineStart = fText + LineToOffset(line);
	const TChar* lineEnd = fText + LineToOffset(line + 1);
	const TChar* text = fText + offset;
	
	if (!text)
		return false;

	if (! IsIdentifierChar(text[0]) && offset < fTextLength - 1)
	{
		outStart = offset;
		outEnd = offset;
		NextCharacter(outEnd);
		return true;
	}

	while (text > lineStart && IsIdentifierChar(text[-1]))
		PreviousCharacter(text);
	outStart = text - fText;

	text = fText + offset;
	while (text < lineEnd && IsIdentifierChar(text[0]))
		NextCharacter(text);
	outEnd = text - fText;

	return true;
}
