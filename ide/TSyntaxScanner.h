// ========================================================================================
//	TSyntaxScanner.h		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TSyntaxScanner__
#define __TSyntaxScanner__

#include "TLanguage.h"
#include "fw/TTextLayout.h"

extern const char* kCKeywords[];
extern const char* kCPlusPlusKeywords[];
extern const char* kJavaKeywords[];
extern const char* kRubyKeywords[];
extern const char* kPythonKeywords[];

class TSyntaxScanner
{
public:
	enum TScanState
	{
		kNoScanState,
		kWhiteSpace,
		kComment,
		kPreprocessor,
		kIdentifier,
		kTeXCommand,
		kTeXSpecialChar,
		kTeXComment,
		kString,
		kTag,			// for HTML
		kContent,		// for HTML
		kUnknown,
	};
	
							TSyntaxScanner(const TTextLayout* layout);
	virtual					~TSyntaxScanner();
	
	TScanState				NextSyntaxRange(STextOffset& offset, uint32& length);
	void					Reset(ELanguage language);
	
protected:
	void					SkipWhiteSpace(const TChar*& text);
	void					SkipComment(const TChar*& text);
	void					SkipTeXComment(const TChar*& text);
	void					SkipHTMLComment(const TChar*& text);
	void					SkipPreprocessor(const TChar*& text);
	void					SkipIdentifier(const TChar*& text);
	void					SkipTeXCommand(const TChar*& text);
	void					SkipString(const TChar*& text);
	void					SkipTag(const TChar*& text);
	void					SkipContent(const TChar*& text);
	void					SkipUnknown(const TChar*& text);
	void					SkipTeXUnknown(const TChar*& text);
	
	bool					IsTeXSpecialChar(TChar ch);
	void					NextChar(const TChar*& text);
	inline bool				IsWhiteSpace(TChar ch) { return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f'); }

	
	const TTextLayout*		fLayout;
	ELanguage				fLanguage;

	const TChar*			fCurrentPosition;
	const TChar*			fTextEnd;
	TScanState				fScanState;
};


#endif // TSyntaxScanner
