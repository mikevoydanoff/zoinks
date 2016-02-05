// ========================================================================================
//	TLogDocument.h			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TLogDocument__
#define __TLogDocument__

#include "fw/TDocument.h"
#include "fw/TString.h"
#include "fw/TTextView.h"

class TLogDocumentOwner;
class TDocumentWindow;
class TMenuBar;
class TWindow;


class TLogDocument : public TDocument
{
public:
								TLogDocument(const char* workingDirectory, bool doubleClickLines = true);
								TLogDocument(const TFile* sourceFile);
						
	void						AppendText(const TChar* text, uint32 length, bool scrollDown);
	void						Truncate(uint32 chars);		// truncates specified number of characters from end
	void						ClearText();
	
	void						AppendStdout(const TChar* text, uint32 length, bool scrollDown);
	void						AppendStderr(const TChar* text, uint32 length, bool scrollDown);
	void						FlushBuffers(bool scrollDown);
	
	void						Show();
	inline void					SetAllowClose(bool allowClose) { fAllowClose = allowClose; }
	void						SetHideOnClose(bool hideOnClose);
	
	inline const TChar*			GetText() const { return fTextView->GetText(); }
	inline uint32				GetTextLength() const { return fTextView->GetTextLength(); }
	
	inline TLogDocumentOwner*	GetOwner() const { return fOwner; }
	void						SetOwner(TLogDocumentOwner* owner);

protected:
	virtual						~TLogDocument();

	virtual void				Open(TDocumentWindow* window);
	virtual void				WriteToFile(TFile* file);

	virtual bool				AllowClose();

protected:
	TMenuBar*					MakeMenuBar(TWindow* window);

protected:
	TLogDocumentOwner*			fOwner;
	TTextView*					fTextView;
	TString						fWorkingDirectory;
	TFile*						fSourceFile;
	bool						fAllowClose;
	bool						fDoubleClickLines;
	
	TString						fStdoutBuffer;
	TString						fStderrBuffer;
};


#endif // __TLogDocument__
