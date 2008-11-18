// ========================================================================================
//	TFileDiffDocument.h			 	 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#ifndef __TFileDiffDocument__
#define __TFileDiffDocument__

#include "fw/TFile.h"
#include "fw/TDynamicArray.h"
#include "fw/TWindowContext.h"
#include "TDiffTextView.h"

class TDocumentWindow;
class TMenuBar;
class TWindow;
class TDiffListView;
class TPixmapButton;


struct TDiffRec
{
	// line number ranges for the difference
	uint32	leftStart;
	uint32	leftEnd;
	uint32	rightStart;
	uint32	rightEnd;
	bool	enabled;
};


class TFileDiffDocument : public TWindowContext
{
public:
	static TFileDiffDocument*	CreateDocument(const TFile& file1, const TFile& file2);

protected:
							TFileDiffDocument(const TFile& file1, const TFile& file2);
	virtual					~TFileDiffDocument();

	virtual void			Open(TDocumentWindow* window);

	virtual void			GetTitle(TString& title) const;

	void					ReadFile(TFile* file, TTextView* textView);
	void					WriteFile(TFile* file, TTextView* textView);
	void					Save();

	void					ComputeDiffs();
	static void				AddChange(int line0, int line1, int deleted, int inserted, void* userData);

	TMenuBar*				MakeMenuBar(TWindow* window);

	virtual void			DoSetupMenu(TMenu* menu);
	virtual bool			DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);
	virtual bool			AllowClose();

	void					CopyToLeft();
	void					CopyToRight();
	void					CopyAllToLeft();
	void					CopyAllToRight();
	void					RecalcDiffs();
	
	static void				LinesInsertedProc(TTextLayout* layout, uint32 line, uint32 count, void* clientData);
	static void				LinesDeletedProc(TTextLayout* layout, uint32 line, uint32 count, void* clientData);
	
	inline bool				LeftModified() const { return (fModified1 && fTextView1->NeedsSaving()); }
	inline bool				RightModified() const { return (fModified2 && fTextView2->NeedsSaving()); }

protected:
	TFile					fFile1;
	TFile					fFile2;
	TDiffTextView*			fTextView1;
	TDiffTextView*			fTextView2;
	TTextLayout*			fTextLayout1;
	TTextLayout*			fTextLayout2;
	bool					fModified1;
	bool					fModified2;
	TDiffListView*			fDiffListView;
	TPixmapButton*			fLeftButton;
	TPixmapButton*			fRightButton;
	
	TDynamicArray<TDiffRec>	fDiffList;
};


#endif // __TFileDiffDocument__
