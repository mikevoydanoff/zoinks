// ========================================================================================
//	TSyntaxScanner.cpp			Copyright (C) 2001-2003 Mike Lockwood. All rights reserved.
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

#include "TSyntaxScanner.h"

#include <ctype.h>


const char* kCKeywords[] =
{
	"asm",
	"auto",
	"break",
	"case",
	"char",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"enum",
	"extern",
	"float",
	"for",
	"goto",
	"if",
	"int",
	"long",
	"register",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"struct",
	"switch",
	"typedef",
	"union",
	"unsigned",
	"void",
	"volatile",
	"while",
	""
};

const char* kCPlusPlusKeywords[] =
{
	"asm",
	"auto",
	"bool",
	"break",
	"case",
	"catch",
	"char",
	"class",
	"const",
	"const_cast",
	"continue",
	"default",
	"delete",
	"do",
	"double",
	"dynamic_cast",
	"else",
	"enum",
	"explicit",
	"export",
	"extern",
	"false",
	"float",
	"for",
	"friend",
	"goto",
	"if",
	"inline",
	"int",
	"long",
	"mutable",
	"namespace",
	"new",
	"operator",
	"private",
	"protected",
	"public",
	"reinterpret_cast",
	"register",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"static_cast",
	"struct",
	"switch",
	"template",
	"this",
	"throw",
	"true",
	"typeid",
	"typename",
	"try",
	"typedef",
	"union",
	"unsigned",
	"using",
	"virtual",
	"void",
	"volatile",
	"wchar_t",
	"while",
	""
};

const char* kJavaKeywords[] =
{
	"abstract",
	"boolean",
	"break",
	"byte",
	"case",
	"catch",
	"char",
	"class",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"extends",
	"float",
	"final",
	"finally",
	"for",
	"goto",
	"if",
	"implements",
	"import",
	"instanceof",
	"int",
	"interface",
	"long",
	"native",
	"new",  
	"package",
	"private",
	"protected",
	"public",
	"return",
	"short",
	"static",
	"strictfp",
	"super",
	"switch",
	"synchronized",
	"this",
	"throw",
	"throws",
	"transient",
	"try",
	"void",
	"volatile",
	"while",
	""
};

const char* kRubyKeywords[] = 
{
	"BEGIN",
	"END",
	"alias",
	"and",
	"begin",
	"break",
	"case",
	"class",
	"def",
	"defined",
	"do",
	"else",
	"elsif",
	"end",
	"ensure",
	"false",
	"for",
	"if",
	"in",
	"module",
	"next",
	"nil",
	"not",
	"or",
	"redo",
	"rescue",
	"retry",
	"return",
	"self",
	"super",
	"then",
	"true",
	"undef",
	"unless",
	"until",
	"when",
	"while",
	"yield",
	""
};

const char* kPythonKeywords[] =
{
	"and",
	"as",
	"assert",
	"break",
	"class",
	"continue",
	"def",
	"del",
	"elif",
	"else",
	"except",
	"exec",
	"finally",
	"for",
	"from",
	"global",
	"if",
	"import",
	"in",
	"is",
	"lambda",
	"not",
	"or",
	"pass",
	"print",
	"raise",
	"return",
	"try",
	"while",
	"with",
	"yield",
	""
};	

inline bool IsString(const TChar* text)
{
	return ((*text == '\'' || *text == '\"') && (text[-1] != '\\' || text[-2] == '\\'));
}


TSyntaxScanner::TSyntaxScanner(const TTextLayout* layout)
	:	fLayout(layout),
		fLanguage(kLanguageNone),
		fCurrentPosition(NULL),
		fTextEnd(NULL)
{
	ASSERT(layout);
	
	Reset(fLanguage);
}


TSyntaxScanner::~TSyntaxScanner()
{
}


TSyntaxScanner::TScanState TSyntaxScanner::NextSyntaxRange(STextOffset& offset, uint32& length)
{
	const TChar* text = fCurrentPosition;
	
	if (!text)
	{
		offset = length = 0;
		return kUnknown;
	}
	
	TScanState state = fScanState;

	try
	{
		if (fLanguage == kLanguageHTML)
		{
			switch (fScanState)
			{
				case kWhiteSpace:
					SkipWhiteSpace(text);
					break;

				case kComment:
					SkipHTMLComment(text);
					break;

				case kString:
					SkipString(text);
					break;
					
				case kTag:
					SkipTag(text);
					break;

				case kContent:
					SkipContent(text);
					break;
			
				case kIdentifier:
				case kPreprocessor:
				case kTeXCommand:
				case kTeXSpecialChar:
				case kTeXComment:
				case kNoScanState:
					ASSERT(0);
					break;
					
				case kUnknown:
					fScanState = kContent;
			}
	
			TChar ch = *text;
	
			if (fScanState == kTag && (ch == '"' || ch == '\''))
			{
				fScanState = kString;
			}
			else if (fScanState == kContent && ch == '<')
			{				
				if (text + 4 < fTextEnd && text[0] == '<' && text[1] == '!' && text[2] == '-' && text[3] == '-')
					fScanState = kComment;
				else
					fScanState = kTag;
			}
		}
		else if (fLanguage == kLanguageTeX)
		{
			// TeX
			
			switch (fScanState)
			{
				case kWhiteSpace:
					SkipWhiteSpace(text);
					break;
					
				case kTeXComment:
					SkipTeXComment(text);
					break;

				case kTeXCommand:
					SkipTeXCommand(text);
					break;
					
				case kTeXSpecialChar:
					NextChar(text);
					break;
	
				case kString:
				case kIdentifier:
				case kComment:
				case kPreprocessor:
				case kTag:
				case kContent:
				case kNoScanState:
					ASSERT(0);
					break;

				case kUnknown:
					SkipTeXUnknown(text);
					break;
			}
			
			TChar ch = *text;
	
			if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
				fScanState = kWhiteSpace;
			else if (ch == '%')
				fScanState = kTeXComment;
			else if (ch == '\\' && isalpha(text[1]))
				fScanState = kTeXCommand;
			else if (IsTeXSpecialChar(ch))
				fScanState = kTeXSpecialChar;
			else
				fScanState = kUnknown;
		}
		else
		{
			// C, C++, Java and Ruby
			
			switch (fScanState)
			{
				case kWhiteSpace:
					SkipWhiteSpace(text);
					break;
					
				case kComment:
					SkipComment(text);
					break;
	
				case kPreprocessor:
					SkipPreprocessor(text);
					break;
	
				case kIdentifier:
					SkipIdentifier(text);
					break;
	
				case kString:
					SkipString(text);	
					break;
					
				case kTag:
				case kContent:
				case kTeXCommand:
				case kTeXComment:
				case kTeXSpecialChar:
				case kNoScanState:
					ASSERT(0);
					break;

				case kUnknown:
					SkipUnknown(text);
					break;
			}
			
			TChar ch = *text;
	
			if (ch == '"' || ch == '\'')
				fScanState = kString;
			else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
				fScanState = kWhiteSpace;
			else if (ch == '#')
			{
				if (fLanguage == kLanguageRuby || fLanguage == kLanguagePython)
				{
					if (text[1] == '!')		// treat #! directives as preprocessor
						fScanState = kPreprocessor;
					else
						fScanState = kComment;
				}
				else
					fScanState = kPreprocessor;
			}
			else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
				fScanState = kIdentifier;
			else if (fLanguage != kLanguageRuby && ch == '/' && (text[1] == '*' || text[1] == '/'))
				fScanState = kComment;
			else
				fScanState = kUnknown;
		}
	}
	catch (...)
	{
		fScanState = kUnknown;
	}
	
	offset = fCurrentPosition - fLayout->GetText();
	length = text - fCurrentPosition;
	fCurrentPosition = text;
	
	return state;
}


void TSyntaxScanner::Reset(ELanguage language)
{
	fCurrentPosition = fLayout->GetText();
	fTextEnd = fCurrentPosition + fLayout->GetTextLength();
	fScanState = kUnknown;
	fLanguage = language;
}



void TSyntaxScanner::SkipWhiteSpace(const TChar*& text)
{
	ASSERT(IsWhiteSpace(*text));

	while (IsWhiteSpace(*text))
		NextChar(text);
}


void TSyntaxScanner::SkipComment(const TChar*& text)
{
	ASSERT(*text == '/' || ((fLanguage == kLanguageRuby || fLanguage == kLanguagePython) && *text == '#'));
	ASSERT(fLanguage != kLanguageHTML && fLanguage != kLanguageTeX);
	NextChar(text);
	
	if (fLanguage == kLanguageRuby || fLanguage == kLanguagePython)
	{
		while (1)
		{
			// read until end of line
			TChar ch = *text;
			
			if (ch == '\n' || ch == '\r')
				break;
		
			NextChar(text);
		}
	}
	else if (*text == '*')
	{
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
	else if (*text == '/')
	{
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
	else
		ASSERT(0);
}


void TSyntaxScanner::SkipTeXComment(const TChar*& text)
{
	ASSERT(*text == '%');
	ASSERT(fLanguage == kLanguageTeX);
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



void TSyntaxScanner::SkipHTMLComment(const TChar*& text)
{
	ASSERT(fLanguage == kLanguageHTML);

	ASSERT(*text == '<');
	NextChar(text);
	ASSERT(*text == '!');
	NextChar(text);
	ASSERT(*text == '-');
	NextChar(text);
	ASSERT(*text == '-');
	NextChar(text);
		
	while (1)
	{
		if (text + 3 < fTextEnd && text[0] == '-' && text[1] == '-' && text[2] == '>')
		{
			NextChar(text);
			NextChar(text);
			NextChar(text);
			
			break;
		}
	
		NextChar(text);
	}
	
	fScanState = kContent;
}


void TSyntaxScanner::SkipPreprocessor(const TChar*& text)
{
	ASSERT(*text == '#');

	NextChar(text);
	
	while (1)
	{
		TChar ch = *text;
		
		// look for comments after preprocessor
		if (ch == '/' && (text[1] == '*' || text[1] == '/'))
			break;

		// handle multi-line macros 
		if (ch == '\\')
		{
			do
				NextChar(text);
			while (*text != kLineEnd13 && *text != kLineEnd10);

			if (*text == kLineEnd10)
				NextChar(text);
			else if (*text == kLineEnd13)
			{
				NextChar(text);
				
				// handle DOS line endings
				if (*text == kLineEnd10)
					NextChar(text);
			}
			
			ch = *text;
		}
		
		if (ch == '\n' || ch == '\r')
			break;
	
		NextChar(text);
	}
}


void TSyntaxScanner::SkipIdentifier(const TChar*& text)
{
	while (1)
	{
		TChar ch = *text;
		
		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= '0' && ch <= '9'))
			NextChar(text);
		else
			break;
	}
}


void TSyntaxScanner::SkipTeXCommand(const TChar*& text)
{
	ASSERT(*text == '\\');

	NextChar(text);
	
	while (1)
	{
		TChar ch = *text;
		
		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
			NextChar(text);
		else
			break;
	}
}


void TSyntaxScanner::SkipString(const TChar*& text)
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
	
	if (fLanguage == kLanguageHTML)
		fScanState = kTag;	// only dealing with strings within tags
}


void TSyntaxScanner::SkipTag(const TChar*& text)
{	
	ASSERT(fLanguage == kLanguageHTML);
	
	while (1)
	{
		TChar ch = *text;
		
		if (ch == '>')
		{
			NextChar(text);
			fScanState = kUnknown;
			break;
		}
		else if (ch == '\"' || ch == '\'')
			break;	// string
	
		NextChar(text);
	}
}


void TSyntaxScanner::SkipContent(const TChar*& text)
{	
	ASSERT(fLanguage == kLanguageHTML);
	
	while (1)
	{		
		if (*text == '<')
			break;
	
		NextChar(text);
	}
}


void TSyntaxScanner::SkipUnknown(const TChar*& text)
{
	while (1)
	{
		TChar ch = *text;
		
		if (ch == '"' || ch == '\'')
			// fScanState = kString;
			break;
		else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
			// fScanState = kWhiteSpace;
			break;
		else if (ch == '#')
			// fScanState = kPreprocessor;
			break;
		else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
			// fScanState = kIdentifier;
			break;
		else if (ch == '/' && (text[1] == '*' || text[1] == '/') && fLanguage != kLanguageRuby)
			// fScanState = kComment;
			break;
		else
			NextChar(text);
	}
}


void TSyntaxScanner::SkipTeXUnknown(const TChar*& text)
{
	while (1)
	{
		TChar ch = *text;
		
		if (ch == '%')
			// fScanState = kComment;
			break;
		else if (ch == '\\' && isalpha(text[1]))
			// fScanState = kTeXCommand;
			break;
		else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
			// fScanState = kWhiteSpace;
			break;
		else if (IsTeXSpecialChar(ch))
			// fScanState = kTeXSpecialChar;
			break;
		else
			NextChar(text);
	}
}


bool TSyntaxScanner::IsTeXSpecialChar(TChar ch)
{
	return (ch == '{' || ch == '}' || ch == '$' || ch == '^' || ch == '_');
}


void TSyntaxScanner::NextChar(const TChar*& text)
{
	if (text >= fTextEnd)
		throw 1;
	
	fLayout->NextCharacter(text);
}			

