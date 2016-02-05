// ========================================================================================
//	TLogViewBehavior.cpp	   Copyright (C) 2001-2004 Mike Voydanoff. All rights reserved.
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

#include "TLogViewBehavior.h"
#include "TTextDocument.h"
#include "fw/TTextView.h"
#include "fw/TTextLayout.h"
#include "fw/TApplication.h"
#include "fw/TTopLevelWindow.h"

#include <ctype.h>


TLogViewBehavior::TLogViewBehavior(const char* workingDirectory)
	:	fTextView(NULL),
		fWorkingDirectory(workingDirectory),
		fSourceFile(NULL)		
{
}


TLogViewBehavior::TLogViewBehavior(TFile* sourceFile)
	:	fTextView(NULL),
		fWorkingDirectory(NULL),
		fSourceFile(sourceFile)
{
}


TLogViewBehavior::~TLogViewBehavior()
{
}


void TLogViewBehavior::SetOwner(TCommandHandler* owner, TBehavior* nextBehavior)
{
	TBehavior::SetOwner(owner, nextBehavior);

	if (owner)
	{
		fTextView = dynamic_cast<TTextView*>(owner);
		ASSERT(fTextView);
	}
	else
		fTextView = NULL;
}


bool TLogViewBehavior::ParseMessage(const char* message, TString& path, int& lineNumber)
{
	const char* line = NULL;
	lineNumber = 0;
	
	// TeX errors start with "l." followed by line number
	if (fSourceFile && sscanf(message, "l.%d ", &lineNumber) == 1)
		return true;

	const char* pathEnd = NULL;
	const char* paren = strchr(message, '(');
	const char* colon = strchr(message, ':');
	const char* file = strstr(message, "File \"");

	if (paren && colon)
		pathEnd = (paren < colon ? paren : colon);
	else if (paren)
		pathEnd = paren;
	else if (colon)
		pathEnd = colon;
	else if (file)
	{
		message = file + strlen("File \"");
		pathEnd = strchr(message, '\"');
		if (pathEnd)
		{
			line = strstr(message, "line ");
			if (line)
				line += strlen("line ");
		}
	}

	if (pathEnd)
	{
		path.Set(message, pathEnd - message);

		if (!line)
		{
			// weblint has line number in parentheses
			line = pathEnd + 1;
		}
	}

	// sanity check for line number after colon or paren
	if ((colon || paren) && line && !isdigit(*line))
		return false;

	if (line)
	{
		while (isdigit(*line))
		{
			lineNumber = 10 * lineNumber + (*line - '0');
			line++;
		}

		return true;
	}
	
	return false;
}


bool TLogViewBehavior::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (fTextView && fTextView->GetClickCount() == 2)
	{
		const TTextLayout* layout = fTextView->GetTextLayout();
		ASSERT(layout);

		uint32 line = layout->VertOffsetToLine(point.v);
		
		STextOffset lineOffset, lineLength;
		lineOffset = layout->LineToOffset(line);
		const TChar* text = layout->GetLineText(line, lineLength);
		ASSERT(text);

		TString path;
		int lineNumber;
		TString message(text, lineLength); 
		if (ParseMessage(message, path, lineNumber))
		{
			// tweak for Java compiler - remove leading ".\"
			if (strncmp(path, ".\\", 2) == 0)
				path.Replace(0, 2, "");

			fTextView->SetSelection(lineOffset, lineOffset + lineLength);

			// successfully parsed the line number
			if (lineNumber > 0)
				lineNumber--;		// adjust for zero based offset

			TString	fullPath;
			
			if (fSourceFile)
				fullPath = fSourceFile->GetPath();
			else
			{
				if (path[0] != '/')
				{
					fullPath = fWorkingDirectory;
					fullPath += path;
					
					// make sure the path is normalized
					TFile::NormalizePath(fullPath);
				}
				else
					fullPath = path;
			}
				
			TTextDocument* textDocument = dynamic_cast<TTextDocument*>(gApplication->FindOrOpenDocument(fullPath));
	
			if (textDocument)
			{
				TTopLevelWindow* window = textDocument->GetMainWindow();
				if (window)
					window->Raise();

				textDocument->GetTextView()->SelectLine(lineNumber);
			}
			else
			{
				TFile* file = gApplication->FindFile(path, NULL);
				if (file)
				{
					TTextDocument* textDocument = dynamic_cast<TTextDocument*>(gApplication->OpenFile(file));
					if (textDocument)
						textDocument->GetTextView()->SelectLine(lineNumber);
				}			
			}

			return true;	// we handled the event
		}	
	}

	// otherwise, take default action.
	return false;
}


