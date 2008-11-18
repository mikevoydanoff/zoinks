// ========================================================================================
//	THTMLCommands.h			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __THTMLCommands__
#define __THTMLCommands__

#include "fw/TCommandID.h"

const TCommandID kATagCommandID					= 5000;
const TCommandID kPTagCommandID					= 5001;
const TCommandID kBTagCommandID					= 5002;
const TCommandID kITagCommandID					= 5003;
const TCommandID kBRTagCommandID				= 5004;
const TCommandID kHRTagCommandID				= 5005;
const TCommandID kAlignCenterCommandID			= 5006;
const TCommandID kAlignRightCommandID			= 5007;
const TCommandID kBLOCKQUOTETagCommandID		= 5008;
const TCommandID kTABLETagCommandID				= 5009;
const TCommandID kTRTagCommandID				= 5010;
const TCommandID kTDTagCommandID				= 5011;
const TCommandID kTHTagCommandID				= 5012;
const TCommandID kCAPTIONTagCommandID			= 5013;
const TCommandID kOLTagCommandID				= 5014;
const TCommandID kULTagCommandID				= 5015;
const TCommandID kLITagCommandID				= 5016;
const TCommandID kDLTagCommandID				= 5017;
const TCommandID kDTTagCommandID				= 5018;
const TCommandID kDDTagCommandID				= 5019;
const TCommandID kH1TagCommandID				= 5020;
const TCommandID kH2TagCommandID				= 5021;
const TCommandID kH3TagCommandID				= 5022;
const TCommandID kH4TagCommandID				= 5023;
const TCommandID kH5TagCommandID				= 5024;
const TCommandID kH6TagCommandID				= 5025;
const TCommandID kHTMLTagCommandID				= 5026;
const TCommandID kCommentTagCommandID			= 5027;
const TCommandID kDIVTagCommandID				= 5028;

const TCommandID kFirstHTMLCommandID			= kATagCommandID;
const TCommandID kLastHTMLCommandID				= kDIVTagCommandID;

// special case tags
const TCommandID kFixSpecialCharsCommandID		= 5100;
const TCommandID kIMGTagCommandID				= 5101;
const TCommandID kThumbnailCommandID			= 5102;
const TCommandID kWebLintCommandID				= 5103;

// attribute commands

const TCommandID kAlignLeftAttrCommandID		= 5200;
const TCommandID kAlignCenterAttrCommandID		= 5201;
const TCommandID kAlignRightAttrCommandID		= 5202;
const TCommandID kClassAttrCommandID			= 5203;
const TCommandID kIDAttrCommandID				= 5204;
const TCommandID kAltAttrCommandID				= 5205;
const TCommandID kHSpaceAttrCommandID			= 5206;
const TCommandID kVSpaceAttrCommandID			= 5207;
const TCommandID kVAlignTopAttrCommandID		= 5208;
const TCommandID kVAlignMiddleAttrCommandID		= 5209;
const TCommandID kVAlignBottomAttrCommandID		= 5210;
const TCommandID kWidthAttrCommandID			= 5211;
const TCommandID kRowSpanAttrCommandID			= 5212;
const TCommandID kColSpanAttrCommandID			= 5213;
const TCommandID kBGColorAttrCommandID			= 5214;
const TCommandID kBackgroundAttrCommandID		= 5215;
const TCommandID kTextAttrCommandID				= 5216;
const TCommandID kLinkAttrCommandID				= 5217;
const TCommandID kVLinkAttrCommandID			= 5218;
const TCommandID kALinkAttrCommandID			= 5219;
const TCommandID kBorderAttrCommandID			= 5220;
const TCommandID kCellPaddingAttrCommandID		= 5221;
const TCommandID kCellSpacingAttrCommandID		= 5222;
const TCommandID kHrefAttrCommandID				= 5223;
const TCommandID kNameAttrCommandID				= 5224;
const TCommandID kStyleAttrCommandID			= 5225;

const TCommandID kFirstHTMLAttrCommandID		= kAlignLeftAttrCommandID;
const TCommandID kLastHTMLAttrCommandID			= kStyleAttrCommandID;


#endif	// __THTMLCommands__
