// ========================================================================================
//	TDynamicArray.cpp				 Copyright (C) 2002 Mike Lockwood. All rights reserved.
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

#include "TDynamicArray.h"

#include <stdlib.h>
#include <string.h>


TDynamicArrayBase::TDynamicArrayBase(uint32 elementSize, uint32 allocationIncrement)
	:	fData(NULL),
		fSize(0),
		fElementSize(elementSize),
		fAllocatedSize(0),
		fAllocationIncrement(allocationIncrement)
{
}


TDynamicArrayBase::~TDynamicArrayBase()
{
	if (fData)
		free(fData);
}


void TDynamicArrayBase::Allocate(uint32 count)
{
	ASSERT(count >= 0);
	
	if (count > fAllocatedSize)
	{
		int32 newCount = fAllocatedSize + fAllocationIncrement;

		fData = (char *)realloc(fData, newCount * fElementSize);
		
		ASSERT(fData);
		fAllocatedSize = newCount;
	}
	else if (fAllocatedSize > count + fAllocationIncrement)
	{
		if (count)
		{
			fData = (char *)realloc(fData, count * fElementSize);
			ASSERT(fData);
		}
		else if (fData)
		{
			free(fData);
			fData = NULL;
		}

		fAllocatedSize = count;
	}
}


void TDynamicArrayBase::InsertItemFirst(const void* item)
{
	Allocate(fSize + 1);
	memmove(&fData[fElementSize], &fData[0], fElementSize * fSize++);
	memcpy(&fData[0], item, fElementSize);
}


void TDynamicArrayBase::InsertItemLast(const void* item)
{
	Allocate(fSize + 1);
	memcpy(&fData[fSize * fElementSize], item, fElementSize);
	fSize++;
}


void TDynamicArrayBase::InsertItemAt(const void* item, uint32 index)
{
	uint32 newSize = (index > fSize ? index + 1 : fSize + 1);
	Allocate(newSize);
	if (index < fSize)
		memmove(&fData[(index + 1) * fElementSize], &fData[index + fElementSize], (fSize - index) * fElementSize);
	memcpy(&fData[index * fElementSize], item, fElementSize);
	fSize = newSize;
}


void TDynamicArrayBase::RemoveItemAt(uint32 index)
{
	ASSERT(index >= 0 && index < fSize);
	--fSize;
	memmove(&fData[index * fElementSize], &fData[(index + 1) * fElementSize], (fSize - index) * fElementSize);
	Allocate(fSize);
}


void TDynamicArrayBase::RemoveAll()
{
	Allocate(0);
	fSize = 0;
}


const void* TDynamicArrayBase::GetAt(uint32 index) const
{
	ASSERT(index >= 0 && index < fSize);
	return &fData[index * fElementSize];
}
