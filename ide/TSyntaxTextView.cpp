// ========================================================================================
//	TSyntaxTextView.cpp		 	Copyright (C) 2001-2007 Mike Lockwood. All rights reserved.
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

#include "IDECommon.h"

#include "TSyntaxTextView.h"
#include "fw/TCursor.h"
#include "fw/TDrawContext.h"
#include "fw/TSettingsFile.h"

#include <X11/cursorfont.h>


TColor TSyntaxTextView::sCommentColor(0xb4, 0, 0);
TColor TSyntaxTextView::sPreprocessorColor(0, 0xb4, 0);
TColor TSyntaxTextView::sKeywordColor(0, 0, 0xb4);
TColor TSyntaxTextView::sStringColor(0x62, 0x65, 0x62);
TColor TSyntaxTextView::sForeColor(0, 0, 0);
TColor TSyntaxTextView::sBackColor(255, 255, 255);
bool TSyntaxTextView::sUseSyntaxHiliting = true;
bool TSyntaxTextView::sDefaultLineWrap = false;
int TSyntaxTextView::sSpacesPerTab = 0;
int TSyntaxTextView::sLineLimit = 0;

const char kUseSyntaxHilitingSetting[]	= "UseSyntaxHiliting";
const char kPreprocessorColorSetting[]	= "PreprocessorColor";
const char kCommentColorSetting[]		= "CommentColor";
const char kKeywordColorSetting[]		= "KeywordColor";
const char kStringColorSetting[]		= "StringColor";
const char kForeColorSetting[]			= "EditorForeColor";
const char kBackColorSetting[]			= "EditorBackColor";
const char kDefaultLineWrapSetting[]	= "DefaultLineWrap";
const char kLineLimit[]			      	= "LineLimit";

const char TSyntaxTextView::kSpacesPerTab[]	= "SpacesPerTab";


TSyntaxTextView::TSyntaxTextView(TWindow* parent, const TRect& bounds, TFont* font, bool modifiable)
	:	TTextView(parent, bounds, font, modifiable),
		fLanguage(kLanguageNone),
		fUseSyntaxHiliting(false),
		fSyntaxScanner(fLayout),
		fCachedSyntaxScanStateStart(TSyntaxScanner::kNoScanState),
		fCachedSyntaxScanStateEnd(TSyntaxScanner::kNoScanState)
{
	SetForeColor(sForeColor);
	SetBackColor(sBackColor);
	
	if (sSpacesPerTab > 0)
		fSpacesPerTab = sSpacesPerTab;
}	


TSyntaxTextView::~TSyntaxTextView()
{
}


void TSyntaxTextView::SetUseSyntaxHiliting(bool doit)
{
	if (fUseSyntaxHiliting != doit)
	{
		fUseSyntaxHiliting = doit; 
		Redraw();
	}
}


void TSyntaxTextView::RedrawLines(uint32 startLine, uint32 endLine, bool showHideInsertionPoint, TRegion* clip)
{
	TSyntaxScanner::TScanState savedCachedSyntaxStateStart = fCachedSyntaxScanStateStart;
	TSyntaxScanner::TScanState savedCachedSyntaxStateEnd = fCachedSyntaxScanStateEnd;
	
	if (fUseSyntaxHiliting)
	{
		fSyntaxScanner.Reset(fLanguage);
		if (GetTextLength() > 0)
			NextSyntaxState();
	}
	
	TTextView::RedrawLines(startLine, endLine, showHideInsertionPoint, clip);

	if (fUseSyntaxHiliting)
	{
		// this is a tweak to make sure fCachedSyntaxScanStateStart and fCachedSyntaxScanStateEnd are updated properly 
		// when typing at the end of a line
		NextSyntaxState();
	
		// redraw more if necessary
		if (savedCachedSyntaxStateStart != fCachedSyntaxScanStateStart || 
		    savedCachedSyntaxStateEnd != fCachedSyntaxScanStateEnd ||
		    fCachedSyntaxScanStateStart == TSyntaxScanner::kNoScanState)
		{
			uint32 lastVisible = LastVisibleLine();
			
			if (endLine < lastVisible)
				RedrawLines(endLine + 1, lastVisible, showHideInsertionPoint, clip);
		}
	}
}


void TSyntaxTextView::DrawText(const TChar* text, int length, TDrawContext& context)
{
	if (length <= 0)
		return;

	if (fUseSyntaxHiliting && context.GetDepth() >= 8)
	{	
		const TChar* textStart = fLayout->GetText();
		const TChar* drawTextEnd = text + length;
	
		STextOffset offset = text - textStart;
		if (offset < fSyntaxOffset)
		{
			fSyntaxScanner.Reset(fLanguage);
			NextSyntaxState();
		}

		while (fSyntaxOffset + fSyntaxLength < offset)
			NextSyntaxState();
		
		const TChar* syntaxEnd = textStart + fSyntaxOffset + fSyntaxLength;
		
		while (length > 0)
		{
			uint32 drawLength = (syntaxEnd > drawTextEnd ? drawTextEnd - text : syntaxEnd - text);
				
			switch (fSyntaxState)
			{
				case TSyntaxScanner::kWhiteSpace:
					break;
	
				case TSyntaxScanner::kComment:
				case TSyntaxScanner::kTeXSpecialChar:
					context.SetForeColor(sCommentColor);
					break;

				case TSyntaxScanner::kPreprocessor:
				case TSyntaxScanner::kTeXComment:
					context.SetForeColor(sPreprocessorColor);
					break;
	
				case TSyntaxScanner::kIdentifier:
					if (IsKeyword(textStart + fSyntaxOffset, fSyntaxLength))
						context.SetForeColor(sKeywordColor);
					else
						context.SetForeColor(fForeColor);
					break;
	
				case TSyntaxScanner::kTeXCommand:
				case TSyntaxScanner::kTag:
					context.SetForeColor(sKeywordColor);
					break;

				case TSyntaxScanner::kString:
					context.SetForeColor(sStringColor);
					break;
	
				case TSyntaxScanner::kContent:
				case TSyntaxScanner::kUnknown:
					context.SetForeColor(fForeColor);
					break;
					
				case TSyntaxScanner::kNoScanState:
					ASSERT(0);
					break;
			}

			TTextView::DrawText(text, drawLength, context);
			
			text += drawLength;
			length -= drawLength;
			
			if (length > 0)
			{
				NextSyntaxState();
				syntaxEnd = textStart + fSyntaxOffset + fSyntaxLength;;
			}
		}
	}
	else
	{
		TTextView::DrawText(text, length, context);
	}
}


TCoord TSyntaxTextView::GetLineLimit(TDrawContext& context)
{
    if (fUseSyntaxHiliting)
	    return (sLineLimit > 0 ? sLineLimit * context.MeasureText("W") : 0);
	else
	    return 0;
}


void TSyntaxTextView::EraseRightEdge(uint32 line, TDrawContext& context, TCoord rightEdge, STextOffset lineEnd)
{
    TCoord lineLimit = GetLineLimit(context);
	
	if (lineLimit > 0)
		rightEdge = lineLimit;
	
	TTextView::EraseRightEdge(line, context, rightEdge, lineEnd);
}


void TSyntaxTextView::SetSelection(STextOffset start, STextOffset end, bool redraw)
{
	TTextView::SetSelection(start, end, redraw);
	
	if (redraw)
		fCachedSyntaxScanStateStart = fCachedSyntaxScanStateEnd = TSyntaxScanner::kNoScanState;
}


void TSyntaxTextView::NextSyntaxState()
{
	do 
		fSyntaxState = fSyntaxScanner.NextSyntaxRange(fSyntaxOffset, fSyntaxLength);
	while (fSyntaxLength == 0 && fSyntaxOffset < GetTextLength());
	
	if (fSyntaxOffset < GetTextLength())
	{
		if (fSyntaxOffset <= fSelectionStart && fSelectionStart < fSyntaxOffset + fSyntaxLength)
			fCachedSyntaxScanStateStart = fSyntaxState;
		
		uint32 endLine = fLayout->OffsetToLine(fSelectionEnd);
		if (endLine < GetLineCount() + 1)
		{
			STextOffset nextLineOffset = fLayout->LineToOffset(endLine + 1);
		
			if (fSyntaxOffset <= nextLineOffset && nextLineOffset < fSyntaxOffset + fSyntaxLength)
				fCachedSyntaxScanStateEnd = fSyntaxState;
		}
	}
}


bool TSyntaxTextView::IsKeyword(const TChar* text, uint32 length)
{
	ASSERT(length > 0);
	
	const char** keyword;
	
	switch (fLanguage)
	{
		case kLanguageC:
			keyword = kCKeywords;
			break;
			
		case kLanguageCPlusPlus:
			keyword = kCPlusPlusKeywords;
			break;
			
		case kLanguageJava:
			keyword = kJavaKeywords;
			break;
			
		case kLanguageRuby:
			keyword = kRubyKeywords;
			break;

		case kLanguagePython:
			keyword = kPythonKeywords;
			break;
						
		default:
			return false;
	}
	
	while (**keyword)
	{
		if (length == strlen(*keyword) && strncmp(*keyword, text, length) == 0)
			return true;
		else
			keyword++;
	}
	
	return false;
}


void TSyntaxTextView::Draw(TRegion* clip)
{
	TDrawContext	context(this, clip);
	context.SetFont(fFont);
	TCoord lineLimit = GetLineLimit(context);
	
	if (lineLimit > 0)
	{
		TCoord	extraV = GetHeight() + fScroll.v - fContentSize.v;
		
		if (extraV > 0)
		{
			TRect	r(lineLimit, 0, GetWidth() + fScroll.h, GetHeight() + fScroll.v);
			context.SetBackColor(kLightGrayColor);
			context.EraseRect(r);
		}
	}
	
	TTextView::Draw(clip);
}


void TSyntaxTextView::EraseContentDifference(const TPoint& oldContentSize, const TPoint& newContentSize)
{
	if (oldContentSize.h > newContentSize.h || oldContentSize.v > newContentSize.v)
	{
		TDrawContext context(this);
		
		if (oldContentSize.h > newContentSize.h)
		{
		    TCoord lineLimit = GetLineLimit(context);

			if (lineLimit > newContentSize.h && lineLimit < newContentSize.v)
			{
				TRect	r1(newContentSize.h, 0, lineLimit, (oldContentSize.v > newContentSize.v ? oldContentSize.v : newContentSize.v));
				context.EraseRect(r1);

				context.SetBackColor(kLightGrayColor);
				TRect	r2(lineLimit, 0, oldContentSize.h, (oldContentSize.v > newContentSize.v ? oldContentSize.v : newContentSize.v));
				context.EraseRect(r2);
				context.SetBackColor(sBackColor);
			}
			else
			{
				TRect	r(newContentSize.h, 0, oldContentSize.h, (oldContentSize.v > newContentSize.v ? oldContentSize.v : newContentSize.v));
				context.EraseRect(r);
			}
		}
		if (oldContentSize.v > newContentSize.v)
		{
			TRect	r(0, newContentSize.v, (oldContentSize.h > newContentSize.h ? oldContentSize.h : newContentSize.h), oldContentSize.v);
			context.EraseRect(r);
		}
	}
}


void TSyntaxTextView::ReadFromSettingsFile(TSettingsFile* settingsFile)
{
	sUseSyntaxHiliting = settingsFile->GetBoolSetting(kUseSyntaxHilitingSetting, sUseSyntaxHiliting);
	sDefaultLineWrap = settingsFile->GetBoolSetting(kDefaultLineWrapSetting, sDefaultLineWrap);
	sSpacesPerTab = settingsFile->GetIntSetting(kSpacesPerTab, 0);
	sLineLimit = settingsFile->GetIntSetting(kLineLimit, 0);

	settingsFile->GetColorSetting(kCommentColorSetting, sCommentColor);
	settingsFile->GetColorSetting(kPreprocessorColorSetting, sPreprocessorColor);
	settingsFile->GetColorSetting(kKeywordColorSetting, sKeywordColor);
	settingsFile->GetColorSetting(kStringColorSetting, sStringColor);
	settingsFile->GetColorSetting(kForeColorSetting, sForeColor);
	settingsFile->GetColorSetting(kBackColorSetting, sBackColor);
}


void TSyntaxTextView::WriteToSettingsFile(TSettingsFile* settingsFile)
{
	settingsFile->SetBoolSetting(kUseSyntaxHilitingSetting, sUseSyntaxHiliting);
	settingsFile->SetBoolSetting(kDefaultLineWrapSetting, sDefaultLineWrap);
	settingsFile->SetIntSetting(kSpacesPerTab, sSpacesPerTab);
	settingsFile->SetIntSetting(kLineLimit, sLineLimit);

	settingsFile->SetColorSetting(kCommentColorSetting, sCommentColor);
	settingsFile->SetColorSetting(kPreprocessorColorSetting, sPreprocessorColor);
	settingsFile->SetColorSetting(kKeywordColorSetting, sKeywordColor);
	settingsFile->SetColorSetting(kStringColorSetting, sStringColor);
	settingsFile->SetColorSetting(kForeColorSetting, sForeColor);
	settingsFile->SetColorSetting(kBackColorSetting, sBackColor);
}

