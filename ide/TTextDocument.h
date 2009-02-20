// ========================================================================================
//	TTextDocument.h			 	Copyright (C) 2001-2009 Mike Lockwood. All rights reserved.
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

#ifndef __TTextDocument__
#define __TTextDocument__

#include "fw/TDocument.h"
#include "TEditorTextView.h"

class TDocumentWindow;
class TMenuBar;
class TWindow;
class TFunctionsMenu;


class TTextDocument : public TDocument
{
public:
	static TDocument*		CreateDocument(TFile* file);

	inline TTextView*		GetTextView() const { return fTextView; }

	// scroll to this line in next created document
	inline static void		SetNextLineNumber(long lineNumber) { sNextLineNumber = lineNumber; }

protected:
							TTextDocument(TFile* file);
	virtual					~TTextDocument();

	virtual void			Open(TDocumentWindow* window);

	virtual void			ReadFromFile(TFile* file);
	virtual void			WriteToFile(TFile* file);

	virtual void			SetTitle(const TChar* title);
	virtual bool			IsModified() const;
	
	virtual void			ShowLine(int line);

protected:
	TMenuBar*				MakeMenuBar(TWindow* window);

	virtual void			DoSetupMenu(TMenu* menu);
	virtual bool			DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);
	virtual bool			DoKeyDown(KeySym key, TModifierState state, const char* string);
	
	void					AddFunctionsMenu();
	void					RemoveFunctionsMenu();

	void					AddHTMLMenu();
	void					RemoveHTMLMenu();

	void					AddTeXMenu();
	void					RemoveTeXMenu();

protected:
	TEditorTextView*		fTextView;
	TFunctionsMenu*			fFunctionsMenu;
	TMenu*					fHTMLMenu;
	TMenu*					fTeXMenu;
	TMenu*					fWindowsMenu;
	TMenuBar*				fMenuBar;
	bool					fLineEndingsChanged;
	
	static long				sNextLineNumber;
};


#endif // __TTextDocument__
