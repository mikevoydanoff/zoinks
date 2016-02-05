// ========================================================================================
//	TFunctionScanner.h		   Copyright (C) 2001-2003 Mike Voydanoff. All rights reserved.
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

#ifndef __TFunctionScanner__
#define __TFunctionScanner__

#include "fw/TTextLayout.h"


class TFunctionScanner
{
public:
	typedef void			(* ReportFunctionProc)(const TChar* functionName, int functionNameLength, STextOffset offset, void* userData);
	
							TFunctionScanner(const TTextLayout* layout);
	virtual					~TFunctionScanner();
	
	void					ScanFunctions(ReportFunctionProc functionProc, void* userData);
	
protected:
	void					ReadFunctionName(const TChar*& text, const TChar*& name, STextOffset& nameLength);
	void					ReadFunctionArgs(const TChar*& text);
	bool					ReadFunctionBody(const TChar*& text);
	void					SkipWhiteSpace(const TChar*& text);
	void					SkipString(const TChar*& text);
	void					SkipComment(const TChar*& text);
	
	void					NextChar(const TChar*& text);
	inline bool				IsWhiteSpace(TChar ch) { return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f'); }

	enum TScanState
	{
		kTopLevel,
		kFunctionName,
		kFunctionArgs
	};
	
	const TTextLayout*		fLayout;
	const TChar*			fTextEnd;
	TScanState				fScanState;
};


#endif // TFunctionScanner
