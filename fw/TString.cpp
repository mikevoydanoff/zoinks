// ========================================================================================
//	TString.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TString.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>


TChar	kEmptyString[] = { 0 };

TString::TString()
	:	fData(NULL),
		fLength(0)
{
}


TString::TString(const TChar* string)
	:	fData(NULL),
		fLength(0)
{
	if (string && string[0])
	{
		Allocate(Tstrlen(string));
		Tstrcpy(fData, string);
	}
}


TString::TString(const TChar* string, int length)
	:	fData(NULL),
		fLength(0)
{
	if (string && length)
	{
		Allocate(length);
		memcpy(fData, string, length * sizeof(TChar));
		fData[length] = 0;
	}
}


TString::TString(const TString& string)
	:	fData(NULL),
		fLength(0)
{
	if (string.fData)
	{
		Allocate(string.fLength);
		Tstrcpy(fData, string.fData);
	}
}


TString::~TString()
{
	if (fData)
		free(fData);
}


void TString::Set(const TChar* data, int length)
{
	Allocate(length);

	if (length > 0)
	{
		memcpy(fData, data, length * sizeof(TChar));
		fData[length] = 0;
	}
}


void TString::Replace(int offset, int length, const TChar* text)
{
	ASSERT(offset + length <= fLength);

	int oldLength = fLength;
	int textLength = (text ? strlen(text) : 0);
	int delta = textLength - length;

	if (delta > 0)
		Allocate(fLength + delta);

	if (delta != 0)
		memmove(fData + offset + length + delta, fData + offset + length, oldLength - offset - length + 1);

	if (text && textLength)
		memcpy(fData + offset, text, textLength);

	if (delta < 0)
		Allocate(oldLength + delta);
}


void TString::Replace(int offset, int length, const TChar* text, int textLength)
{
	ASSERT(offset + length <= fLength);

	int oldLength = fLength;
	int delta = textLength - length;

	if (delta > 0)
		Allocate(fLength + delta);

	if (delta != 0)
		memmove(fData + offset + length + delta, fData + offset + length, oldLength - offset - length + 1);

	if (text && textLength)
		memcpy(fData + offset, text, textLength);

	if (delta < 0)
		Allocate(oldLength + delta);
}

void TString::Append(const TChar* text, int length)
{
	if (length > 0)
	{
		int oldLength = fLength;
		Allocate(fLength + length);
		memcpy(fData + oldLength, text, length);
		fData[fLength] = 0;
	}
}


void TString::ToUpper()
{
	TChar* chp = fData;

	if (chp)
	{
		char ch;
		
		while ((ch = *chp) != 0)
			*chp++ = toupper(ch);
	}
}


void TString::ToLower()
{
	TChar* chp = fData;

	if (chp)
	{
		char ch;
		
		while ((ch = *chp) != 0)
			*chp++ = tolower(ch);
	}
}


TChar TString::operator[] (int i) const
{
	ASSERT(fData && i >= 0 && i < fLength);
	return fData[i];
}


TString::operator const TChar*() const
{
	if (fData)
		return fData;
	else
		return kEmptyString;
}


TString& TString::operator=(const TString& string)
{
	Allocate(string.fLength);
	if (fLength > 0)
		Tstrcpy(fData, string.fData);

	return *this;
}


TString& TString::operator=(const TChar* string)
{
	Allocate(Tstrlen(string));
	if (fLength > 0)
		Tstrcpy(fData, string);

	return *this;
}


TString& TString::operator=(const TChar ch)
{
	Allocate(1);
	fData[0] = ch;
	fData[1] = 0;

	return *this;
}


TString& TString::operator+=(const TString& string)
{
	if (string.fLength > 0)
	{
		bool wasEmpty = (fLength == 0);
		Allocate(fLength + string.fLength);
		if (wasEmpty)
			fData[0] = 0;
		Tstrcat(fData, string.fData);
	}

	return *this;
}


TString& TString::operator+=(const TChar* string)
{
	int length = Tstrlen(string);
	if (length)
	{
		bool wasEmpty = (fLength == 0);
		Allocate(fLength + length);
		if (wasEmpty)
			fData[0] = 0;
		Tstrcat(fData, string);
	}

	return *this;
}


TString& TString::operator+=(const TChar ch)
{
	Allocate(fLength + 1);
	fData[fLength - 1] = ch;
	fData[fLength] = 0;

	return *this;
}


void TString::CopyTo(char* buffer, int maxLength)
{
	const TChar* src = fData; 
	if (src)
	{
		for (int i = 0; *src && i < maxLength; i++)
			*buffer++ = (char)*src++;
	} 

	*buffer = 0;
}


void TString::SetLineEndingFormat(TLineEndingFormat format)
{
	TChar* string = fData; 
	if (!string)
		return;

	TChar ch = *string;
	int offset = 0;
	while (ch)
	{
		if (ch == kLineEnd13)
		{
			if (string[1] == kLineEnd10)
			{
				// DOS ending
				if (format == kMacLineEndingFormat || format == kUnixLineEndingFormat)
				{
					Allocate(fLength - 1);
					string = fData + offset;
					memmove(string, string + 1, fLength - offset);
					*string = (format == kMacLineEndingFormat ? kLineEnd13 : kLineEnd10);
				}
				else
				{
					++string;
					++offset;
				}
			}
			else
			{
				// Mac ending
				if (format == kUnixLineEndingFormat)
					*string = kLineEnd10;
				else if (format == kDOSLineEndingFormat)
				{
					Allocate(fLength + 1);
					string = fData + offset;
					memmove(string + 1, string, fLength - offset);
					*string++ = kLineEnd13;
					*string = kLineEnd10;
					offset++;
				}
			}
		}
		else if (ch == kLineEnd10)
		{
			// Unix Ending
			if (format == kMacLineEndingFormat)
				*string = kLineEnd13;
			else if (format == kDOSLineEndingFormat)
			{
				Allocate(fLength + 1);
				string = fData + offset;
				memmove(string + 1, string, fLength - offset);
				*string++ = kLineEnd13;
				*string = kLineEnd10;
				offset++;
			}
		}

		ch = *(++string);
		offset++;
	}
}


bool TString::GetLineEndingFormat(const TChar* text, int length, TLineEndingFormat& outFormat)
{
	const TChar* textEnd = text + length;
	
	while (text < textEnd)
	{
		TChar ch = *text++;

		if (ch == kLineEnd13)
		{
			if (text < textEnd - 1 && *text == kLineEnd10)
			{
				text++;
				outFormat = kDOSLineEndingFormat;
				return true;
			}
			else
			{
				outFormat = kMacLineEndingFormat;
				return true;
			}
		}
		else if (ch == kLineEnd10)
		{
			outFormat = kUnixLineEndingFormat;
			return true;
		}
	}

	return false;
}


int	TString::AsInteger() const
{
	const TChar* text = fData;
	if (!text)
		return 0;

	int result = 0;
	while (isdigit(text[0]))
	{
		result = 10 * result + text[0] - '0';
		text++;
	}

	return result;
}


void TString::Truncate(int length)
{
	ASSERT(length >= 0 && length <= fLength);

	if (length >= 0 && length <= fLength)
	{
		Allocate(length);

		if (length > 0)
			fData[length] = 0;
	}
}


void TString::Allocate(int newLength)
{
	if (newLength > 0)
	{
		if (fData)
			fData = (TChar *)realloc(fData, (newLength + 1) * sizeof(TChar));
		else
			fData = (TChar *)malloc((newLength + 1) * sizeof(TChar));

		ASSERT(fData);
	}
	else
	{
		if (fData)
		{
			free(fData);		
			fData = NULL;
		}
	}

	fLength = newLength;
}


int operator==(const TString& string1, const TString& string2)
{
	if (string1.fLength != string2.fLength)		// this handles zero length string cases
		return 0;
	return (Tstrcmp(string1.fData, string2.fData) == 0);
}

/*
int Tstrlen(const TChar* string)
{
	int result = 0;
	while (*string++)
		result++;
	return result;
}

void Tstrcpy(TChar* dest, const TChar* src)
{
	TChar ch;
	while ((ch = *src++) != 0)
		*dest++ = ch;

	*dest = 0;
}

void Tstrcat(TChar* dest, const TChar* src)
{
	while (*dest)
		++dest;
	
	TChar ch;
	while ((ch = *src++) != 0)
		*dest++ = ch;

	*dest = 0;
}

int Tstrcmp(const TChar* str1, const TChar* str2)
{
	while (1)
	{
		TChar ch1 = *str1++;
		TChar ch2 = *str2++;
		int diff = ch2 - ch1;

		if (!ch1 || !ch2)
			return diff;
	}
		
	// shouldn't get here, but...
	return 0;
}


int Tstrncmp(const TChar* str1, const TChar* str2, int n)
{
	return strncmp(str1, str2, n);
}


int Tstrcasecmp(const TChar* str1, const TChar* str2)
{
	return strcasecmp(str1, str2);
}
*/
