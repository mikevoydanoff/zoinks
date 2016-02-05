// ========================================================================================
//	TCommandID.h			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TCommandID__
#define __TCommandID__

typedef int TCommandID;

const TCommandID kNoCommandID 					= 0;

// File Menu
const TCommandID kNewCommandID					= 100;
const TCommandID kOpenCommandID					= 101;
const TCommandID kFindFileCommandID				= 102;
const TCommandID kFindSelectionCommandID		= 103;
const TCommandID kSaveCommandID					= 104;
const TCommandID kSaveAsCommandID				= 105;
const TCommandID kSaveCopyCommandID				= 106;
const TCommandID kSaveAllCommandID				= 107;
const TCommandID kCloseCommandID				= 108;
const TCommandID kQuitCommandID					= 109;

// Edit Menu
const TCommandID kUndoCommandID					= 110;
const TCommandID kRedoCommandID					= 111;
const TCommandID kCutCommandID					= 112;
const TCommandID kCopyCommandID					= 113;
const TCommandID kPasteCommandID				= 114;
const TCommandID kClearCommandID				= 115;
const TCommandID kSelectAllCommandID			= 116;

// Search Menu
const TCommandID kFindCommandID					= 120;
const TCommandID kFindNextCommandID				= 121;
const TCommandID kFindPreviousCommandID			= 122;
const TCommandID kFindNextSelectionCommandID	= 123;
const TCommandID kFindPreviousSelectionCommandID = 124;
const TCommandID kReplaceCommandID				= 125;
const TCommandID kReplacePreviousCommandID		= 126;
const TCommandID kReplaceNextCommandID			= 127;
const TCommandID kReplaceAllCommandID			= 128;
const TCommandID kGotoLineCommandID				= 129;

// File Format
const TCommandID kUnixFormatCommandID			= 130;
const TCommandID kMacFormatCommandID			= 131;
const TCommandID kDOSFormatCommandID			= 132;

const TCommandID kAboutCommandID				= 140;

// For Controls, Views, etc.
const TCommandID kValueChangedCommandID			= 200;
const TCommandID kScrollLeftCommandID			= 201;
const TCommandID kScrollRightCommandID			= 202;
const TCommandID kScrollUpCommandID				= 203;
const TCommandID kScrollDownCommandID			= 204;
const TCommandID kPageLeftCommandID				= 205;
const TCommandID kPageRightCommandID			= 206;
const TCommandID kPageUpCommandID				= 207;
const TCommandID kPageDownCommandID				= 208;
const TCommandID kDataModifiedCommandID			= 209;
const TCommandID kFocusAcquiredCommandID		= 210;
const TCommandID kFocusLostCommandID			= 211;
const TCommandID kSelectionChangedCommandID		= 212;
const TCommandID kFlushTypeSelectCommandID		= 213;
const TCommandID kLineEndingsChangedCommandID	= 214;

// For Dialogs
const TCommandID kOKCommandID					= 300;
const TCommandID kCancelCommandID				= 301;

// For CommonDialogs
const TCommandID kParentDirectoryCommandID		= 400;
const TCommandID kHomeDirectoryCommandID		= 401;
const TCommandID kWorkingDirectoryCommandID		= 402;

// For TTextView
const TCommandID kShiftLeftCommandID			= 500;
const TCommandID kShiftRightCommandID			= 501;
const TCommandID kBalanceSelectionCommandID		= 502;
const TCommandID kToggleLineWrapCommandID		= 503;

// For Documents
const TCommandID kFilePathChangedCommandID		= 600;

#endif // __TCommandID__
