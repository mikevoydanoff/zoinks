// ========================================================================================
//	TTeXBehavior.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TTeXBehavior.h"
#include "TTeXCommands.h"
#include "TLogDocument.h"
#include "fw/TApplication.h"
#include "fw/TChildProcess.h"
#include "fw/TCommonDialogs.h"
#include "fw/TFile.h"
#include "fw/TMenu.h"
#include "fw/TString.h"
#include "fw/TTextView.h"
#include "fw/TTopLevelWindow.h"

#include "fw/intl.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


struct TTeXTag
{
	const TChar*	fBeginTag;
	const TChar*	fEndTag;			// use -1 to not change selection
	int				fSelectionOffset;
	TCommandID		fCommandID;			// menu command id
};

	
const TTeXTag	gTeXTags[] = 
{
	{	"\\documentclass[12pt]{article}\n\\title{Title}\n\\author{Author}\n\\date{\\today}\n\n\\begin{document}\n\n\\maketitle\n\n\\end{document}\n",		"", 		-1,			kTexDocumentCommandID			},
	{	"\\begin{defn}\n\n\\end{defn}\n",				"",				13,					kTexDefinitionCommandID		},
	{	"\\begin{lem}\n\n\\end{lem}\n",					"",				12,					kTexLemmaCommandID			},
	{	"\\begin{thm}\n\n\\end{thm}\n",					"",				12,					kTexTheoremCommandID		},
	{	"\\begin{proof}\n\n\\end{proof}\n",				"",				14,					kTexProofCommandID			},
	{	"\\begin{cor}\n\n\\end{cor}\n",					"",				12,					kTexCorollaryCommandID		},
	{	"$",											"$",			-1,					kTexMathCommandID			},
	{	"\\emph{",										"}",			-1,					kTexEmphasisCommandID		},
	{	"\\forall",										"",				-1,					kTexForAllCommandID			},
	{	"\\exists",										"",				-1,					kTexExistsCommandID			},
	{	"\\in",											"",				-1,					kTeXElementCommandID		},
	{	"\\cdot",										"",				-1,					kTeXCenterDotCommandID		},
	{	"",												"",				-1,					kNoCommandID				}	// end of list
};


TTeXBehavior::TTeXBehavior()
	:	fLogDocument(NULL)
{
}


TTeXBehavior::~TTeXBehavior()
{
	if (fLogDocument)
	{
		fLogDocument->SetHideOnClose(false);
		fLogDocument->Close();
	}
}


void TTeXBehavior::DoSetupMenu(TMenu* menu)
{
	for (TCommandID command = kFirstTeXTagCommandID; command <= kLastTeXTagCommandID; command++)
		menu->EnableCommand(command);
	
	menu->EnableCommand(kTexDVICommandID);
	menu->EnableCommand(kTexViewXDVICommandID);
	menu->EnableCommand(kTexGeneratePDFCommandID);
}


bool TTeXBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	if (command >= kFirstTeXTagCommandID && command <= kLastTeXTagCommandID)
	{
		return DoTeXCommand(command);
	}
	else if (command == kTexDVICommandID)
	{
		GenerateDVI();
		return true;
	}
	else if (command == kTexViewXDVICommandID)
	{
		GenerateDVI();
		ViewXDVI();
		return true;
	}
	else if (command == kTexGeneratePDFCommandID)
	{
		GeneratePDF();
		return true;
	}

	return false;
}


bool TTeXBehavior::DoTeXCommand(TCommandID command)
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	const TTeXTag* tag = gTeXTags;
	while (tag)
	{
		if (tag->fCommandID == command)
			break;
		else if (tag->fCommandID == kNoCommandID)
			tag = NULL;
		else
			tag++;
	}

	if (tag)
	{
		STextOffset	start, end;
		textView->GetSelection(start, end);
		int startLength = strlen(tag->fBeginTag);
		int endLength = strlen(tag->fEndTag);
		
		TLineEndingFormat lineEndingFormat = textView->GetLineEndingFormat();
		
		int selectionOffset = tag->fSelectionOffset;

		if (startLength > 0)
		{
			if (lineEndingFormat != kUnixLineEndingFormat)
			{
				if (selectionOffset > 0 && lineEndingFormat == kDOSLineEndingFormat && tag->fBeginTag)
				{
					// adjust selectionOffset for DOS line endings
					
					int originalSelectionOffset = selectionOffset;
					const char* text = tag->fBeginTag;
					
					for (int i = 0; i < originalSelectionOffset; i++)
					{
						if (text[i] == '\n')
							selectionOffset++;
					}
				}
				
				TString temp(tag->fBeginTag);
				temp.SetLineEndingFormat(lineEndingFormat);
				startLength = temp.GetLength();
				textView->InsertText(start, temp, startLength, (endLength > 0 ? TTextView::kGroupWithNext : TTextView::kNormal));
			}
			else
				textView->InsertText(start, tag->fBeginTag, startLength, (endLength > 0 ? TTextView::kGroupWithNext : TTextView::kNormal));

			end += startLength;
		}
		
		if (endLength > 0)
		{
			if (lineEndingFormat != kUnixLineEndingFormat)
			{
				TString temp(tag->fEndTag);
				temp.SetLineEndingFormat(lineEndingFormat);
				endLength = temp.GetLength();
				textView->InsertText(end, temp, endLength, (startLength > 0 ? TTextView::kGroupWithPrevious : TTextView::kNormal));
			}
			else
				textView->InsertText(end, tag->fEndTag, endLength, (startLength > 0 ? TTextView::kGroupWithPrevious : TTextView::kNormal));
		}
		
		if (selectionOffset >= 0)
			textView->SetSelection(start + selectionOffset);
		else
			textView->SetSelection(start + startLength, end);
			
		textView->SetUndoSelection();

		return true;
	}
	
	return false;
}


void TTeXBehavior::GenerateDVI()
{
	ExecuteTeXCommand("latex -interaction=nonstopmode --src-specials ");
}


void TTeXBehavior::ViewXDVI()
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	const TFile* file = NULL;
	TDocument* document = textView->GetDocument();
	if (document)
		file = &document->GetFile();
		
	if (document->IsModified())
		document->Save();

	if (file && file->IsSpecified())
	{
		char	xdviCommand[100];
		long	leaderWindow = gApplication->GetLeaderWindow();
		
		sprintf(xdviCommand, "xdvi -editor \"zoinks -window %ld ", leaderWindow);
		TString command(xdviCommand);
		command += "+\%l \%f\" ";
		command += file->GetPath();
		
		// change file extension to .dvi
		command.Truncate(command.GetLength() - strlen(file->GetFileExtension()));
		command += "dvi";
		
		TChildProcess* child = gApplication->Execute(command, NULL, true, false);
		delete child;
	}
	else
		gApplication->Beep();	
}

void TTeXBehavior::GeneratePDF()
{
	ExecuteTeXCommand("pdflatex -interaction=nonstopmode ");
}


void TTeXBehavior::ExecuteTeXCommand(const char* command)
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	const TFile* file = NULL;
	TDocument* document = textView->GetDocument();
	if (document)
		file = &document->GetFile();
		
	if (document->IsModified())
		document->Save();

	if (file && file->IsSpecified())
	{
		TString commandLine(command);
		commandLine += file->GetPath();
		
		TString	directory;
		file->GetDirectory(directory);

		TChildProcess* child = gApplication->Execute(commandLine, directory, false, true);
	
		if (fLogDocument)
			fLogDocument->ClearText();
		
		while (1)
		{
			char buffer[1000];
				
			ssize_t count = read(child->GetStdout(), buffer, sizeof(buffer) - 1);
			if (count > 0)
			{
				if (!fLogDocument)
				{
					fLogDocument = new TLogDocument(file);
					fLogDocument->SetTitle(_("LaTeX Output"));
					gApplication->OpenWindowContext(fLogDocument, false);
					fLogDocument->SetHideOnClose(true);
				}

				fLogDocument->AppendText(buffer, count, false);
			}
			else
				break;
		}
		
		// show window if there was an error
		while (child->IsRunning())
		{
			sleep(1);
			gApplication->CheckForSignals();
		}
	
		if (child->GetExitStatus() != 0)
		{
			if (fLogDocument)
				fLogDocument->GetMainWindow()->Show(true);
			else
				TCommonDialogs::AlertDialog(N_("DVI Generation Failed."), N_("Error"), NULL, N_("OK"));
		}

		delete child;
	}
	else
		gApplication->Beep();
}


