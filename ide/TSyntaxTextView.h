// ========================================================================================
//	TSyntaxTextView.h		 	Copyright (C) 2001-2007 Mike Lockwood. All rights reserved.
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

#ifndef __TSyntaxTextView__
#define __TSyntaxTextView__

#include "fw/TTextView.h"
#include "TSyntaxScanner.h"
#include "TLanguage.h"

class TSettingsFile;


class TSyntaxTextView : public TTextView
{
public:
	static const char kSpacesPerTab[];

								TSyntaxTextView(TWindow* parent, const TRect& bounds, TFont* font, bool modifiable);
	
	void						SetUseSyntaxHiliting(bool doit);
	inline void					SetLanguage(ELanguage language) { fLanguage = language; }

	static void					ReadFromSettingsFile(TSettingsFile* settingsFile);
	static void					WriteToSettingsFile(TSettingsFile* settingsFile);
	
	static inline bool			DefaultLineWrap()	{ return sDefaultLineWrap; }

protected:
	virtual						~TSyntaxTextView();

	// TTextView overrides
	void						Draw(TRegion* clip);
    virtual void                EraseRightEdge(uint32 line, TDrawContext& context, TCoord rightEdge, STextOffset lineEnd);
	virtual void				RedrawLines(uint32 startLine, uint32 endLine, bool showHideInsertionPoint, TRegion* clip = NULL);
	virtual void				DrawText(const TChar* text, int length, TDrawContext& context);
	virtual void				SetSelection(STextOffset start, STextOffset end, bool redraw = true);

	virtual void				EraseContentDifference(const TPoint& oldContentSize, const TPoint& newContentSize);
	
	void						NextSyntaxState();
	bool						IsKeyword(const TChar* text, uint32 length);

	TCoord						GetLineLimit(TDrawContext& context);

protected:
	ELanguage					fLanguage;
	bool						fUseSyntaxHiliting;
	TSyntaxScanner				fSyntaxScanner;
	TSyntaxScanner::TScanState	fSyntaxState;
	STextOffset					fSyntaxOffset;
	uint32						fSyntaxLength;
	
	TSyntaxScanner::TScanState	fCachedSyntaxScanStateStart;	// cached scan state at start of selection
	TSyntaxScanner::TScanState	fCachedSyntaxScanStateEnd;		// cached scan state at line after end of selection
	
	static TColor				sCommentColor;
	static TColor				sPreprocessorColor;
	static TColor				sKeywordColor;
	static TColor				sStringColor;

	static TColor				sForeColor;
	static TColor				sBackColor;
	
	static bool					sUseSyntaxHiliting;
	static bool					sDefaultLineWrap;
	static int					sSpacesPerTab;
	static int                  sLineLimit;

};

#endif // __TEditorTextView__
