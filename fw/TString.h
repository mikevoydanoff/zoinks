// ========================================================================================
//	TString.h				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TString__
#define __TString__

#include <string.h>

enum TLineEndingFormat
{
	kUnixLineEndingFormat,
	kMacLineEndingFormat,
	kDOSLineEndingFormat
};

const char kLineEnd10 = 10;
const char kLineEnd13 = 13;

class TString
{
public:
	TString();
	TString(const TChar* string);
	TString(const TChar* string, int length);
	TString(const TString& string);
	
	~TString(); 

	void				Set(const TChar* data, int length);

	void				Replace(int offset, int length, const TChar* text);
	void				Replace(int offset, int length, const TChar* text, int textLength);
	void				Append(const TChar* text, int length);

	void				ToUpper();
	void				ToLower();
	
	TChar				operator[] (int i) const;
						operator const TChar*() const;

	TString&			operator=(const TString& string);
	TString&			operator=(const TChar* string);
	TString&			operator=(const TChar ch);

	TString&			operator+=(const TString& string);
	TString&			operator+=(const TChar* string);
	TString&			operator+=(const TChar ch);

	void				CopyTo(char* buffer, int maxLength);

	inline const int	GetLength() const { return fLength; }
	void				Truncate(int length);
	
	inline bool			IsEmpty() const { return (fLength == 0); }
	inline void			SetEmpty() { Allocate(0); }

	inline bool		 	GetLineEndingFormat(TLineEndingFormat& outFormat) const { return GetLineEndingFormat(fData, fLength, outFormat); }
	void				SetLineEndingFormat(TLineEndingFormat format);
	static bool 		GetLineEndingFormat(const TChar* text, int length, TLineEndingFormat& outFormat);

	int					AsInteger() const;

protected:
	friend int operator==(const TString& string1, const TString& string2);

	void				Allocate(int newLength);
	
	TChar* 				fData;
	int					fLength;
};

int operator==(const TString& string1, const TString& string2);

extern TChar	kEmptyString[];

inline int Tstrlen(const TChar* string)
{
	return strlen(string);
}

inline void Tstrcpy(TChar* dest, const TChar* src)
{
	strcpy(dest, src);
}

inline void Tstrcat(TChar* dest, const TChar* src)
{
	strcat(dest, src);
}

inline int Tstrcmp(const TChar* str1, const TChar* str2)
{
	return strcmp(str1, str2);
}

inline int Tstrncmp(const TChar* str1, const TChar* str2, int n)
{
	return strncmp(str1, str2, n);
}

inline int Tstrcasecmp(const TChar* str1, const TChar* str2)
{
	return strcasecmp(str1, str2);
}

#endif // __TString__
