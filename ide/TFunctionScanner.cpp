// ========================================================================================
//	TFunctionScanner.cpp		Copyright (C) 2001-2003 Mike Lockwood. All rights reserved.
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

#include "TFunctionScanner.h"

#include <ctype.h>


inline bool IsString(const TChar* text)
{
	return ((*text == '\'' || *text == '\"') && (text[-1] != '\\' || text[-2] == '\\'));
}

inline bool IsComment(const TChar* text)
{
	return (text[0] == '/' && (text[1] == '/' || text[1] == '*'));
}


TFunctionScanner::TFunctionScanner(const TTextLayout* layout)
	:	fLayout(layout),
		fTextEnd(NULL)
{
	ASSERT(layout);
}


TFunctionScanner::~TFunctionScanner()
{
}


void TFunctionScanner::ScanFunctions(ReportFunctionProc functionProc, void* userData)
{
	const TChar* text = fLayout->GetText();
	if (!text)
		return;
		
	const TChar* textStart = text;
	fTextEnd = text + fLayout->GetTextLength();
	fScanState = kTopLevel;

	const TChar* functionName;
	STextOffset functionNameLength;	
	try
	{
		while (1)
		{
			switch (fScanState)
			{
				case kTopLevel:
					ReadFunctionName(text, functionName, functionNameLength);
					break;
	
				case kFunctionName:
					ReadFunctionArgs(text);
					break;
					
				case kFunctionArgs:
					if (ReadFunctionBody(text))
						functionProc(functionName, functionNameLength, functionName - textStart, userData);
					break;
			}
		}
	}
	catch (...)
	{
	}	
}


void TFunctionScanner::ReadFunctionName(const TChar*& text, const TChar*& name, STextOffset& nameLength)
{
	SkipWhiteSpace(text);
	
	while (*text == '*')
	{
		NextChar(text);
		SkipWhiteSpace(text);
	}
	
	name = text;
	
	while (!IsWhiteSpace(*text) && *text != '(')
		NextChar(text);
	nameLength = text - name;

	SkipWhiteSpace(text);

	if (*text == '(')
		fScanState = kFunctionName;
}


void TFunctionScanner::ReadFunctionArgs(const TChar*& text)
{
	SkipWhiteSpace(text);
	
	if (*text == '(')
	{
		while (*text != ')')
		{
			if (IsString(text))
				SkipString(text);
			else if (IsComment(text))
				SkipComment(text);
			else
				NextChar(text);
		}

		NextChar(text);
		fScanState = kFunctionArgs;
	}
	else
	{
		while (1)
		{
			TChar ch = *text;

			if (IsString(text))
				SkipString(text);
			else if (IsComment(text))
				SkipComment(text);
			else
			{
				NextChar(text);
				
				if (ch == ';')
				{
					fScanState = kTopLevel;
					break;
				}
			}
		}
	}
}


bool TFunctionScanner::ReadFunctionBody(const TChar*& text)
{
	bool result = false;
	SkipWhiteSpace(text);
	
	if (*text == ':')
	{
		// special case constructors
		NextChar(text);
		SkipWhiteSpace(text);
		
		while (*text != '{')
		{
			if (IsString(text))
				SkipString(text);
			else if (IsComment(text))
				SkipComment(text);
			else
				NextChar(text);
		}
	}
	else 
	{
		// skip "const", "throws", etc.
		// also handle K&R style function definitions
		
		while (isalnum(*text) || *text == '_')
		{
			while (isalnum(*text) || *text == '_')
				NextChar(text);
				
			SkipWhiteSpace(text);
			
			// for K&R support (pointer, function and array type arguments) 
			while (*text == '*' || *text == ';' || *text == ',' || 
				   *text == '[' || *text == ']' || *text == '(' || *text == ')')
			{
				NextChar(text);
				SkipWhiteSpace(text);
			}
		}
	}
		
	if (*text == '{')
	{
		NextChar(text);
		int bracketDepth = 1;
		
		while (bracketDepth > 0)
		{
			// need to do this to process comments correctly
			SkipWhiteSpace(text);
			
			TChar ch = *text;
			
			if (IsString(text))
			{
				SkipString(text);
				continue;	// don't call NextChar() since SkipString() already did
			}
			else if (ch == '{')
				++bracketDepth;
			else if (ch == '}')
				--bracketDepth;
			
			NextChar(text);
		}
		
		result = true;
	}
	else if (*text != ';')
		NextChar(text);
	
	fScanState = kTopLevel;
	return result;
}


void TFunctionScanner::SkipWhiteSpace(const TChar*& text)
{	
	while (1)
	{
		TChar ch = *text;
		
		if (ch == '/')
		{
			if (text + 1 < fTextEnd)
			{
				if (text[1] == '/')
				{
					NextChar(text);
					NextChar(text);
					
					TChar previous = 0;
					while (1)
					{
						TChar ch = *text;
						
						if ((ch == '\n' || ch == '\r') && previous != '\\')
							break;
					
						previous = ch;
						NextChar(text);
					}				
				}
				else if (text[1] == '*')
				{
					NextChar(text);
					NextChar(text);
				
					while (1)
					{
						if (*text == '*' && text + 1 < fTextEnd && text[1] == '/')
						{
							NextChar(text);
							NextChar(text);
							
							break;
						}
					
						NextChar(text);
					}
				}
				else
					break;
			}
			else
				break;
		}
		else if (ch == '#')
		{
			while (*text != '\n' && *text != '\r')
				NextChar(text);
		}
		else if (IsWhiteSpace(ch))
			NextChar(text);
		else
			break;
	}
}


void TFunctionScanner::SkipString(const TChar*& text)
{
	TChar	terminator = *text;
	ASSERT(terminator == '\'' || terminator == '\"');
	NextChar(text);
	
	TChar previous = 0;
	TChar previous2 = 0;
	
	while (1)
	{
		TChar ch = *text;
		
		if (ch == terminator && (previous != '\\' || previous2 == '\\'))
		{
			NextChar(text);
			break;
		}
	
		previous2 = previous;
		previous = ch;
		NextChar(text);
	}
}


void TFunctionScanner::SkipComment(const TChar*& text)
{
	ASSERT(*text == '/');
	text++;
	ASSERT(*text == '/' || *text == '*');
	
	if (*text++ == '/')
	{
		while (*text != '\n' && *text != '\r')
			++text;
	}
	else
	{
		while (text[0] != '*' || text[1] != '/')
			++text;
		
		text += 2;	// skip "*/"
	}
}


void TFunctionScanner::NextChar(const TChar*& text)
{
	if (text >= fTextEnd)
		throw 1;
	
	fLayout->NextCharacter(text);
}			

