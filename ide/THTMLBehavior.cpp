// ========================================================================================
//	THTMLBehavior.cpp		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "THTMLBehavior.h"
#include "THTMLCommands.h"
#include "TLogDocument.h"
#include "fw/TApplication.h"
#include "fw/TChildProcess.h"
#include "fw/TCommonDialogs.h"
#include "fw/TFile.h"
#include "fw/TMenu.h"
#include "fw/TPopupMenu.h"
#include "fw/TString.h"
#include "fw/TTextView.h"
#include "fw/TTopLevelWindow.h"

#include "fw/intl.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#ifdef HAVE_LIBIMLIB
#include <Imlib.h>

// in IDEMain.cpp
extern ImlibData* gImlibData;
#endif // HAVE_LIBIMLIB	

struct THTMLTag
{
	const TChar*	fBeginTag;
	const TChar*	fEndTag;			// use -1 to not change selection
	int				fSelectionOffset;
	TCommandID		fCommandID;			// menu command id
};

	
const THTMLTag	gHTMLTags[] = 
{
	{	"<a href=\"\">",			"</a>", 		9, 			kATagCommandID			},
	{	"<p>",						"</p>", 		-1,			kPTagCommandID			},
	{	"<div>",					"</div>", 		-1,			kDIVTagCommandID		},
	{	"<b>",						"</b>", 		-1,			kBTagCommandID			},
	{	"<i>",						"</i>", 		-1,			kITagCommandID			},
	{	"<br>",						"", 			-1,			kBRTagCommandID			},
	{	"<hr>",						"",		 		-1,			kHRTagCommandID			},
	{	"<div align=\"center\">",	"</div>",		-1,			kAlignCenterCommandID	},
	{	"<div align=\"right\">",	"</div>",		-1,			kAlignRightCommandID	},
	{	"<blockquote>",				"</blockquote>",-1,			kBLOCKQUOTETagCommandID	},
	{	"<table>",					"</table>", 	-1,			kTABLETagCommandID		},
	{	"<tr>",						"</tr>", 		-1,			kTRTagCommandID			},
	{	"<td>",						"</td>", 		-1,			kTDTagCommandID			},
	{	"<th>",						"</th>", 		-1,			kTHTagCommandID			},
	{	"<caption>",				"</caption>", 	-1,			kCAPTIONTagCommandID	},
	{	"<ol>",						"</ol>", 		-1,			kOLTagCommandID			},
	{	"<ul>",						"</ul>", 		-1,			kULTagCommandID			},
	{	"<li>",						"</li>", 		-1,			kLITagCommandID			},
	{	"<dl>",						"</dl>", 		-1,			kDLTagCommandID			},
	{	"<dt>",						"</dt>", 		-1,			kDTTagCommandID			},
	{	"<dd>",						"</dd>", 		-1,			kDDTagCommandID			},
	{	"<h1>",						"</h1>", 		-1,			kH1TagCommandID			},
	{	"<h2>",						"</h2>", 		-1,			kH2TagCommandID			},
	{	"<h3>",						"</h3>", 		-1,			kH3TagCommandID			},
	{	"<h4>",						"</h4>", 		-1,			kH4TagCommandID			},
	{	"<h5>",						"</h5>", 		-1,			kH5TagCommandID			},
	{	"<h6>",						"</h6>", 		-1,			kH6TagCommandID			},
	{	"<html>\n\t<head>\n\t\t<title></title>\n\t</head>\n\t<body>\n\t",		"\n\t</body>\n</html>", 		24,			kHTMLTagCommandID			},
	{	"<!-- ",			" -->", 		-1,					kCommentTagCommandID	},
	{	"",					"",				-1,					kNoCommandID			}	// end of list
};


struct THTMLAttribute
{
	const TChar*	fAttributeText;
	int				fSelectionOffset;
	TCommandID		fCommandID;			// menu command id
};

const THTMLAttribute	gHTMLAttributes[] = 
{
	{	" align=\"left\"",		-1,				kAlignLeftAttrCommandID		},
	{	" align=\"center\"",	-1,				kAlignCenterAttrCommandID	},
	{	" align=\"right\"",		-1,				kAlignRightAttrCommandID	},
	{	" class=\"\"",			8,				kClassAttrCommandID			},
	{	" id=\"\"",				5,				kIDAttrCommandID			},
	{	" style=\"\"",			8,				kStyleAttrCommandID			},
	{	" alt=\"\"",			6,				kAltAttrCommandID			},
	{	" hspace=\"\"",			9,				kHSpaceAttrCommandID		},
	{	" vspace=\"\"",			9,				kVSpaceAttrCommandID		},
	{	" valign=\"top\"",		-1,				kVAlignTopAttrCommandID		},
	{	" valign=\"middle\"",	-1,				kVAlignMiddleAttrCommandID	},
	{	" valign=\"bottom\"",	-1,				kVAlignBottomAttrCommandID	},
	{	" width=\"\"",			8,				kWidthAttrCommandID			},
	{	" rowspan=\"\"",		10,				kRowSpanAttrCommandID		},
	{	" colspan=\"\"",		10,				kColSpanAttrCommandID		},
	{	" bgcolor=\"\"",		10,				kBGColorAttrCommandID		},
	{	" background=\"\"",		13,				kBackgroundAttrCommandID	},
	{	" text=\"\"",			7,				kTextAttrCommandID			},
	{	" link=\"\"",			7,				kLinkAttrCommandID			},
	{	" vlink=\"\"",			8,				kVLinkAttrCommandID			},
	{	" alink=\"\"",			8,				kALinkAttrCommandID			},
	{	" border=\"\"",			9,				kBorderAttrCommandID		},
	{	" cellpadding=\"\"",	14,				kCellPaddingAttrCommandID	},
	{	" cellspacing=\"\"",	14,				kCellSpacingAttrCommandID	},
	{	"",						-1,				kNoCommandID				}	// end of list
};


struct TSpecialChar
{
	int				ch;
	const char*		replacement;
};

// this table must be sorted by ch
const TSpecialChar gSpecialChars[] = 
{
	{	'\"',		"&quot;"	},
	{	'&',		"&amp;"		},
	{	'<',		"&lt;"		},
	{	'>',		"&gt;"		},
	{	146,		"'"			},
	{	147,		"&quot;"	},	// left quote
	{	148,		"&quot;"	},	// right quote
	{	150,		"-"			},	// endash
	{	160,		"&nbsp;"	},
	{	161,		"&iexcl;"	},
	{	162,		"&cent;"	},
	{	163,		"&pound;"	},
	{	164,		"&curren;"	},
	{	165,		"&yen;"		},
	{	166,		"&brvbar;"	},
	{	167,		"&sect;"	},
	{	168,		"&uml;"		},
	{	169,		"&cpoy;"	},
	{	170,		"&ordf;"	},
	{	171,		"&laquo;"	},
	{	172,		"&not;	"	},
	{	173,		"&shy;	"	},
	{	174,		"&reg;"		},
	{	175,		"&macr;"	},
	{	176,		"&deg;"		},
	{	177,		"&plusmn;"	},
	{	178,		"&sup2;"	},
	{	179,		"&sup3;"	},
	{	180,		"&acute;"	},
	{	181,		"&micro;"	},
	{	182,		"&para;"	},
	{	183,		"&middot;"	},
	{	184,		"&cedil;"	},
	{	185,		"&sup1;"	},
	{	186,		"&ordm;"	},
	{	187,		"&raquo;"	},
	{	188,		"&frac14;"	},
	{	189,		"&frac12;"	},
	{	190,		"&frac34;"	},
	{	191,		"&iquest;"	},
	{	192,		"&Agrave;"	},
	{	193,		"&Aacute;"	},
	{	194,		"&Acirc;"	},
	{	195,		"&Atilde;"	},
	{	196,		"&Auml;"	},
	{	197,		"&Aring;"	},
	{	198,		"&AElig;"	},
	{	199,		"&Ccedil;"	},
	{	200,		"&Egrave;"	},
	{	201,		"&Eacute;"	},
	{	202,		"&Ecirc;"	},
	{	203,		"&Euml;"	},
	{	204,		"&Igrave;"	},
	{	205,		"&Iacute;"	},
	{	206,		"&Icirc;"	},
	{	207,		"&Iuml;"	},
	{	208,		"&ETH;"		},
	{	209,		"&Ntile;"	},
	{	210,		"&Ograve;"	},
	{	211,		"&Oacute;"	},
	{	212,		"&Ocirc;"	},
	{	213,		"&Otilde;"	},
	{	214,		"&Ouml;"	},
	{	215,		"&times;"	},
	{	216,		"&Oslash;"	},
	{	217,		"&Ugrave;"	},
	{	218,		"&Uacute;"	},
	{	219,		"&Ucirc;"	},
	{	220,		"&Uuml;"	},
	{	221,		"&Yacute;"	},
	{	222,		"&THORN;"	},
	{	223,		"&szlig;"	},
	{	224,		"&agrave;"	},
	{	225,		"&aacute;"	},
	{	226,		"&acirc;"	},
	{	227,		"&atilde;"	},
	{	228,		"&auml;"	},
	{	229,		"&aring;"	},
	{	230,		"&aelig;"	},
	{	231,		"&ccedil;"	},
	{	232,		"&egrave;"	},
	{	233,		"&eacute;"	},
	{	234,		"&ecirc;"	},
	{	235,		"&euml;"	},
	{	236,		"&igrave;"	},
	{	237,		"&iacute;"	},
	{	238,		"&icirc;"	},
	{	239,		"&iuml;"	},
	{	240,		"&eth;"		},
	{	241,		"&ntilde;"	},
	{	242,		"&ograve;"	},
	{	243,		"&oacute;"	},
	{	244,		"&ocirc;"	},
	{	245,		"&otilde;"	},
	{	246,		"&ouml;"	},
	{	247,		"&divide;"	},
	{	248,		"&oslash;"	},
	{	249,		"&ugrave;"	},
	{	250,		"&uacute;"	},
	{	251,		"&ucirc;"	},
	{	252,		"&uuml;"	},
	{	253,		"&yacute;"	},
	{	254,		"&thorn;"	},
	{	255,		"&yuml;"	},
	{	0,			""			}
};


static TMenuItemRec sAlignMenu[] = 
{
	{ N_("Left"), kAlignLeftAttrCommandID },
	{ N_("Center"), kAlignCenterAttrCommandID },
	{ N_("Right"), kAlignRightAttrCommandID },
	{ "" }
};


static TMenuItemRec sVAlignMenu[] = 
{
	{ N_("Top"), kVAlignTopAttrCommandID },
	{ N_("Middle"), kVAlignMiddleAttrCommandID },
	{ N_("Bottom"), kVAlignBottomAttrCommandID },
	{ "" }
};


static TMenuItemRec sDIVMenu[] = 
{
	{ N_("Align"), 0, 0, 0, sAlignMenu },
	{ N_("Class"), kClassAttrCommandID },
	{ N_("ID"), kIDAttrCommandID },
	{ N_("Style"), kStyleAttrCommandID },
	{ "" }
};

static TMenuItemRec sAMenu[] = 
{
	{ N_("href"), kHrefAttrCommandID },
	{ N_("Name"), kNameAttrCommandID },
	{ N_("Class"), kClassAttrCommandID },
	{ N_("ID"), kIDAttrCommandID },
	{ N_("Style"), kStyleAttrCommandID },
	{ "" }
};


static TMenuItemRec sIMGMenu[] = 
{
	{ N_("Align"), 0, 0, 0, sAlignMenu },
	{ N_("VAlign"), 0, 0, 0, sVAlignMenu },
	{ N_("alt"), kAltAttrCommandID },
	{ N_("hspace"), kHSpaceAttrCommandID },
	{ N_("vspace"), kVSpaceAttrCommandID },
	{ "" }
};


static TMenuItemRec sTABLEMenu[] = 
{
	{ N_("Align"), 0, 0, 0, sAlignMenu },
	{ N_("width"), kWidthAttrCommandID },
	{ N_("border"), kBorderAttrCommandID },
	{ N_("cellpadding"), kCellPaddingAttrCommandID },
	{ N_("cellspacing"), kCellSpacingAttrCommandID },
	{ "" }
};


static TMenuItemRec sTDMenu[] = 
{
	{ N_("Align"), 0, 0, 0, sAlignMenu },
	{ N_("VAlign"), 0, 0, 0, sVAlignMenu },
	{ N_("Width"), kWidthAttrCommandID },
	{ N_("Row Span"), kRowSpanAttrCommandID },
	{ N_("Column Span"), kColSpanAttrCommandID },
	{ "" }
};


static TMenuItemRec sBODYMenu[] = 
{
	{ N_("Background Color"), kBGColorAttrCommandID },
	{ N_("Background Image"), kBackgroundAttrCommandID },
	{ N_("Text Color"), kTextAttrCommandID },
	{ N_("Link Color"), kLinkAttrCommandID },
	{ N_("Visited Link Color"), kVLinkAttrCommandID },
	{ N_("Active Link Color"), kALinkAttrCommandID },
	{ N_("Style"), kStyleAttrCommandID },
	{ "" }
};


struct THTMLTagMenu
{
	const TChar*			fTagName;
	const TMenuItemRec*		fMenuRec;
};


static THTMLTagMenu sTagMenus[] = 
{
	{ "div", 	sDIVMenu },
	{ "p", 		sDIVMenu },
	{ "h1", 	sDIVMenu },
	{ "h2", 	sDIVMenu },
	{ "h3", 	sDIVMenu },
	{ "h4", 	sDIVMenu },
	{ "h5", 	sDIVMenu },
	{ "h6", 	sDIVMenu },
	{ "a",	 	sAMenu },
	{ "img", 	sIMGMenu },
	{ "table", 	sTABLEMenu },
	{ "td", 	sTDMenu },
	{ "th", 	sTDMenu },
	{ "body",	sBODYMenu },
	{ "", 		NULL }
};



inline bool IsSpecialChar(int ch)
{
	return (ch > 127 || ch == '&' || ch == '<' || ch == '>' || ch == '\"');
}


THTMLBehavior::THTMLBehavior()
	:	fLogDocument(NULL),
		fCurrentPopup(NULL)
{
}


THTMLBehavior::~THTMLBehavior()
{
	if (fLogDocument)
	{
		fLogDocument->SetHideOnClose(false);
		fLogDocument->Close();
	}
	
	delete fCurrentPopup;
}


void THTMLBehavior::DoSetupMenu(TMenu* menu)
{
	for (TCommandID command = kFirstHTMLCommandID; command <= kLastHTMLCommandID; command++)
		menu->EnableCommand(command);
		
	for (TCommandID command = kFirstHTMLAttrCommandID; command <= kLastHTMLAttrCommandID; command++)
		menu->EnableCommand(command);

	menu->EnableCommand(kFixSpecialCharsCommandID);
	menu->EnableCommand(kIMGTagCommandID);
	menu->EnableCommand(kThumbnailCommandID);
	menu->EnableCommand(kWebLintCommandID);
}


bool THTMLBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);
	
	// special case <br> tag to support adding <br>s at line endings of a range of text
	if (command == kBRTagCommandID && AddLineBreakTags())
		return true;
	// else fall through to DoHTMLCommand

	if (command >= kFirstHTMLCommandID && command <= kLastHTMLCommandID)
	{
		return DoHTMLCommand(command);
	}
	else if (command >= kFirstHTMLAttrCommandID && command <= kLastHTMLAttrCommandID)
	{
		return DoAttributeCommand(command);
	}
	else if (command == kFixSpecialCharsCommandID)
	{
		FixSpecialCharacters();
		return true;
	}
	else if (command == kIMGTagCommandID)
	{
		const TFile* file = NULL;
		
		TDocument* document = textView->GetDocument();
		if (document)
			file = &document->GetFile();

		if (file && file->IsSpecified())
			InsertImage(file->GetPath());
		else
			InsertImage(NULL);

		return true;
	}

#ifdef HAVE_LIBIMLIB	
	else if (command == kThumbnailCommandID)
	{
		const TFile* file = NULL;
		
		TDocument* document = textView->GetDocument();
		if (document)
			file = &document->GetFile();

		if (file && file->IsSpecified())
			InsertThumbnail(file->GetPath());
		else
			InsertThumbnail(NULL);

		return true;
	}
#endif
	else if (command == kWebLintCommandID)
	{
		WebLint();
		return true;
	}
	
	return false;
}


bool THTMLBehavior::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kRightButton)
	{
		TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
		ASSERT(textView);
		const TTextLayout* layout = textView->GetTextLayout();

		STextOffset start, end;
		
		// don't round up, so we know which character was clicked on
		start = end = layout->PointToOffset(point, false);

		if (layout->BalanceSelection(start, end))
		{
			const TChar* text = layout->GetText();
			const TChar* textEnd = text + layout->GetTextLength();
			text += start;
			
			if (text[0] == '<')
			{
				text++;
				const TChar* wordEnd = text;
				
				while (wordEnd < textEnd && isalnum(*wordEnd))
					wordEnd++;
				
				if (wordEnd > text)
				{
					delete fCurrentPopup;
					fCurrentPopup = NULL;
					
					TString	tag(text, wordEnd - text);

					const THTMLTagMenu* tagMenu = sTagMenus;
					while (tagMenu->fMenuRec)
					{
						if (Tstrcasecmp(tagMenu->fTagName, tag) == 0)
							break;
						else
							tagMenu++;
					}
					
					if (tagMenu->fMenuRec)
					{
						textView->SetSelection(start, end);

						fCurrentPopup = new TPopupMenu(tagMenu->fMenuRec);
						fCurrentPopup->Display(textView, point, textView->GetCurrentEventTime());
					}
				}
			}	
		}
	}
	
	return false;
}


bool THTMLBehavior::DoHTMLCommand(TCommandID command)
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	const THTMLTag* tag = gHTMLTags;
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


bool THTMLBehavior::DoAttributeCommand(TCommandID command)
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	const THTMLAttribute* attribute = gHTMLAttributes;
	while (attribute)
	{
		if (attribute->fCommandID == command)
			break;
		else if (attribute->fCommandID == kNoCommandID)
			attribute = NULL;
		else
			attribute++;
	}

	if (attribute)
	{
		STextOffset	start, end;
		textView->GetSelection(start, end);
		const TChar* text = textView->GetText();
		
		if (end - start > 2 && text[start] == '<' && text[end - 1] == '>')
		{
			int length = strlen(attribute->fAttributeText);
	
			textView->InsertText(end - 1, attribute->fAttributeText, length, TTextView::kNormal);
			
			if (attribute->fSelectionOffset >= 0)
				textView->SetSelection(end - 1 + attribute->fSelectionOffset);
			else
				textView->SetSelection(end - 1 + length);
				
			textView->SetUndoSelection();
			
			return true;
		}
	}
	
	return false;
}


bool THTMLBehavior::AddLineBreakTags()
{
	STextOffset	start, end;
	uint32 line, startLine, endLine;

	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);
	const TTextLayout* layout = textView->GetTextLayout();
	ASSERT(layout);

	textView->GetSelection(start, end);
	
	if (start == end)
		return false;
		
	startLine = layout->OffsetToLine(start);
	endLine = layout->OffsetToLine(end);
	
	if (startLine == endLine)
		return false;
	
	endLine--;				// don't count last line if we have multiple lines selected
	
	for (line = startLine; line <= endLine; line++)
	{
		STextOffset	lineEndOffset = layout->LineToOffset(line + 1);
		layout->PreviousCharacter(lineEndOffset);

		textView->InsertText(lineEndOffset, "<br>", 4);
	}
	
	return true;
}


void THTMLBehavior::FixSpecialCharacters()
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	const unsigned char* text = (const unsigned char*)textView->GetText();
	STextOffset length = textView->GetTextLength();
	STextOffset offset = 0;

	while (offset < length)
	{
		int ch = *text++;
		if (IsSpecialChar(ch))
		{
			const TSpecialChar* specialChar = gSpecialChars;

			while (specialChar->ch && specialChar->ch < ch)
				++specialChar;

			const char* replacement;
			char	buffer[20];
			
			if (specialChar->ch == ch)
			{
				replacement = specialChar->replacement;
			}
			else
			{
				sprintf(buffer, "&#%d;", ch);
				replacement = buffer;
			}

			int replacementLength = strlen(replacement);
			textView->SetSelection(offset, offset + 1);
			textView->ReplaceSelection(replacement, replacementLength, true, true);

			offset += (replacementLength - 1);
			length += (replacementLength - 1);

			// refetch the text because the view probably realloced it
			text = (const unsigned char*)textView->GetText() + offset;
		}
		else
			++offset;
	}
}


void THTMLBehavior::InsertImage(const char* documentBase)
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	TList<TFile>* fileList = TCommonDialogs::OpenFiles(textView->GetTopLevelWindow());

	if (fileList)
	{
		TListIterator<TFile>	iter(*fileList);

		TFile* file;

		for (int i = 0; (file = iter.Next()) != NULL; i++)
		{
			char	buffer[PATH_MAX + 100];
			TString imagePath;

			if (documentBase)
				TFile::ComputeRelativePath(file->GetPath(), documentBase, imagePath);
			else
				imagePath = file->GetFileName();

#ifdef HAVE_LIBIMLIB
			ImlibImage* image = Imlib_load_image(gImlibData, (char *)file->GetPath());			
			if (image)
			{
				sprintf(buffer, "<img src=\"%s\" width=\"%d\" height=\"%d\">", (const char *)imagePath, image->rgb_width, image->rgb_height);
				Imlib_destroy_image(gImlibData, image);
			}
			else
#endif // HAVE_LIBIMLIB
			{
				sprintf(buffer, "<img src=\"%s\">", (const char *)imagePath);
			}

			bool newLine = (i < fileList->GetSize() - 1 && fileList->GetSize() > 1);
			if (newLine)
				strcat(buffer, textView->GetTextLayout()->GetLineEndingString());
			
			STextOffset	start, end;
			textView->GetSelection(start, end);

			textView->InsertText(start, buffer, strlen(buffer));
			if (newLine)
				textView->AutoIndent();

			delete file;
		}

		delete fileList;
	}
}


#ifdef HAVE_LIBIMLIB
const TCoord kMaxThumbnailSize = 450;

void THTMLBehavior::InsertThumbnail(const char* documentBase)
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	TList<TFile>* fileList = TCommonDialogs::OpenFiles(textView->GetTopLevelWindow());

	if (fileList)
	{
		TListIterator<TFile>	iter(*fileList);

		TFile* file;

		for (int i = 0; (file = iter.Next()) != NULL; i++)
		{
			char	buffer[PATH_MAX + 100];
			TString imagePath;

			if (documentBase)
				TFile::ComputeRelativePath(file->GetPath(), documentBase, imagePath);
			else
				imagePath = file->GetFileName();

			ImlibImage* image = Imlib_load_image(gImlibData, (char *)file->GetPath());			
			if (image)
			{
				TCoord thumbWidth = 0;
				TCoord thumbHeight = 0;
				
				if (image->rgb_width > image->rgb_height && image->rgb_width > kMaxThumbnailSize)
				{
					thumbWidth = kMaxThumbnailSize;
					thumbHeight = (2 * kMaxThumbnailSize * image->rgb_height + 1) / ( 2 * image->rgb_width);
				}
				else if (image->rgb_height > image->rgb_width && image->rgb_height > kMaxThumbnailSize)
				{
					thumbHeight = kMaxThumbnailSize;
					thumbWidth = (2 * kMaxThumbnailSize * image->rgb_width + 1) / ( 2 * image->rgb_height);
				}
				
				if (thumbWidth > 0 && thumbHeight > 0)
				{
	  				ImlibImage* scaledImage = Imlib_clone_scaled_image(gImlibData, image, thumbWidth, thumbHeight);
					ASSERT(scaledImage);
					
					TString thumbPath(imagePath);
					int extensionOffset = thumbPath.GetLength() - strlen(file->GetFileExtension()) - 1;
					thumbPath.Replace(extensionOffset, 0, _("-small"));
					
					sprintf(buffer, "<a href=\"%s\"><img src=\"%s\" width=\"%ld\" height=\"%ld\"></a>", (const char *)imagePath, (const char *)thumbPath, thumbWidth, thumbHeight);
	
					TString thumbFilePath;
					file->GetDirectory(thumbFilePath);
					thumbFilePath += thumbPath;
					
					Imlib_save_image(gImlibData, scaledImage, (char *)(const char *)thumbFilePath, NULL);
					Imlib_destroy_image(gImlibData, scaledImage);
				}
				else
				{
					sprintf(buffer, "<img src=\"%s\" width=\"%d\" height=\"%d\">", (const char *)imagePath, image->rgb_width, image->rgb_height);
				}
				
				Imlib_destroy_image(gImlibData, image);
			}
			else
			{
				sprintf(buffer, "<img src=\"%s\">", (const char *)imagePath);
			}

			bool newLine = (i < fileList->GetSize() - 1 && fileList->GetSize() > 1);
			if (newLine)
				strcat(buffer, textView->GetTextLayout()->GetLineEndingString());
			
			STextOffset	start, end;
			textView->GetSelection(start, end);

			textView->InsertText(start, buffer, strlen(buffer));
			if (newLine)
				textView->AutoIndent();

			delete file;
		}

		delete fileList;
	}
}
#endif // HAVE_LIBIMLIB


void THTMLBehavior::WebLint()
{
	TTextView*	textView = dynamic_cast<TTextView*>(fOwner);
	ASSERT(textView);

	const TFile* file = NULL;
	TDocument* document = textView->GetDocument();
	if (document)
		file = &document->GetFile();

	if (file && file->IsSpecified())
	{
		TString command("weblint -stderr -x Microsoft -x Netscape ");
		command += file->GetPath();
		
		TChildProcess* child = gApplication->Execute(command, NULL, false, true);

		TString	directory;
		file->GetDirectory(directory);
		
		if (!fLogDocument)
		{
			fLogDocument = new TLogDocument(directory);
			fLogDocument->SetTitle(_("WebLint Output"));
			gApplication->OpenWindowContext(fLogDocument);
			fLogDocument->SetHideOnClose(true);
		}
		else
		{
			fLogDocument->ClearText();
			fLogDocument->Show();
		}
		
		while (1)
		{
			char buffer[1000];
				
			ssize_t count = read(child->GetStderr(), buffer, sizeof(buffer) - 1);
			if (count > 0)
				fLogDocument->AppendText(buffer, count, false);
			else
				break;
		}
		
		if (fLogDocument->GetTextLength() == 0)
		{
			const char* noErrors = _("weblint found no errors\n");
			fLogDocument->AppendText(noErrors, strlen(noErrors), true);
		}

		delete child;
	}
	else
		gApplication->Beep();
}


