// ========================================================================================
//	TList.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TList.h"

#include <stdlib.h>
#include <string.h>


TListBase::TListBase(int32 initialCount, int allocationIncrement)
	:	fData(NULL),
		fSize(0),
		fAllocatedSize(0),
		fAllocationIncrement(allocationIncrement),
		fCompareFunc(NULL),
		fSearchFunc(NULL),
		fCompareData(NULL),
		fIterators(NULL)
{
	if (initialCount)
		Allocate(initialCount);
}


TListBase::TListBase(CompareFunc compareFunc, SearchFunc searchFunc, void* compareData,
					 int32 initialCount, int allocationIncrement)
	:	fData(NULL),
		fSize(0),
		fAllocatedSize(0),
		fAllocationIncrement(allocationIncrement),
		fCompareFunc(compareFunc),
		fSearchFunc(searchFunc),
		fCompareData(compareData),
		fIterators(NULL)
{
	ASSERT(compareFunc);
	// searchFunc optional - not needed if you don't search.
	
	if (initialCount)
		Allocate(initialCount);
}


TListBase::~TListBase()
{
	TListIteratorBase* iterator = fIterators;
	while (iterator)
	{
		iterator->ListDeleted();
		iterator = iterator->fNext;
	}

	if (fData)
		free(fData);
}


void TListBase::Allocate(int32 count)
{
	ASSERT(count >= 0);
	
	if (count > fAllocatedSize)
	{
		int32 newCount = fAllocatedSize + fAllocationIncrement;

		fData = (const void **)realloc(fData, newCount * sizeof(void*));

		ASSERT(fData);
		fAllocatedSize = newCount;
	}
	else if (fAllocatedSize > count + fAllocationIncrement)
	{
		if (count)
		{
			fData = (const void **)realloc(fData, count * sizeof(void*));
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


void TListBase::InsertItem(const void* item)
{
	int32 index = 0;

	if (IsSorted())
	{
		int32 low = 0;
		int32 high = fSize;
		
		while (low < high)
		{
			index = (low + high) >> 1;
			int compare = fCompareFunc(item, fData[index], fCompareData);

			if (compare < 0)
				high = index;		// item is less than index
			else if (compare > 0)
				low = index + 1;	// item is greater than index
			else 
				low = high = index;
		}

		index = low;
	}
	else
		index = fSize;

	Allocate(fSize + 1);
	memmove(&fData[index + 1], &fData[index], (fSize - index) * sizeof(void *));
	fData[index] = item;
	++fSize;

	TListIteratorBase* iterator = fIterators;
	while (iterator)
	{
		iterator->ItemInserted(index);
		iterator = iterator->fNext;
	}
}


void TListBase::InsertItemFirst(const void* item)
{
	ASSERT(! IsSorted());

	Allocate(fSize + 1);
	memmove(&fData[1], &fData[0], sizeof(const void*) * fSize++);
	fData[0] = item;
}


void TListBase::InsertItemLast(const void* item)
{
	ASSERT(! IsSorted());

	Allocate(fSize + 1);
	fData[fSize++] = item;
}


void TListBase::InsertItemAt(const void* item, int32 index)
{
	Allocate(fSize + 1);
	if (index < fSize)
		memmove(&fData[index + 1], &fData[index], (fSize - index) * sizeof(void *));
	fData[index] = item;
	++fSize;
}


void TListBase::RemoveItem(const void* item)
{
	if (IsSorted())
	{
		int32 low = 0;
		int32 high = fSize;

		while (low < high)
		{
			int32 index = (low + high) >> 1;
			int compare = fCompareFunc(item, fData[index], fCompareData);

			if (compare < 0)
				high = index;		// item is less than index
			else if (compare > 0)
				low = index + 1;	// item is greater than index
			else 
				low = high = index;
		}

		ASSERT(low == high);
		ASSERT(fData[low] == item);
		RemoveItemAt(low);
	}
	else
	{
		for (int32 i=0; i<fSize; i++)
		{
			if (fData[i] == item)
			{
				RemoveItemAt(i);
				return;
			}
		}
	}
}


void TListBase::RemoveItemAt(int32 index)
{
	ASSERT(index >= 0 && index < fSize);
	--fSize;
	memmove(&fData[index], &fData[index + 1], (fSize - index) * sizeof(void *));
	Allocate(fSize);

	TListIteratorBase* iterator = fIterators;
	while (iterator)
	{
		iterator->ItemDeleted(index);
		iterator = iterator->fNext;
	}
}


void TListBase::RemoveAll()
{
	Allocate(0);
	fSize = 0;
}


const void* TListBase::GetAt(int32 index) const
{
	ASSERT(index >= 0 && index < fSize);
	return fData[index];
}


int32 TListBase::FindIndex(const void* key) const
{
	if (IsSorted())
	{
		ASSERT(fSearchFunc);

		int32 low = 0;
		int32 high = fSize;
		
		while (low < high)
		{
			int32 index = (low + high) >> 1;
			int compare = fSearchFunc(fData[index], key, fCompareData);

			if (compare < 0)
				low = index + 1;	// index is less than key
			else if (compare > 0)
				high = index;		// index is greater than key
			else 
				return index;
		}
	}
	else
	{
		int32 i;
		for (i = 0; i < fSize; i++)
		{
			if (fData[i] == key)
				return i;
		}
	}		
	
	return -1;
}


const void* TListBase::DoSearch(const void* key) const
{
	int32 index = FindIndex(key);
	
	if (index >= 0 && index < fSize)
		return fData[index];
	else
		return NULL;
}


bool TListBase::ContainsItem(const void* item) const
{
	for (int32 i=0; i<fSize; i++)
		if (fData[i] == item)
			return true;

	return false;
}	


void TListBase::AddIterator(TListIteratorBase* iterator)
{
	iterator->fNext = fIterators;
	fIterators = iterator;
}


void TListBase::RemoveIterator(TListIteratorBase* iterator)
{
	TListIteratorBase* current = fIterators;
	TListIteratorBase* previous = NULL;

	while (current)
	{
		if (current == iterator)
		{
			if (previous)
				previous->fNext = iterator->fNext;
			else
				fIterators = iterator->fNext;

			return;
		}
		else
		{
			previous = current;
			current = current->fNext;
		}
	}

	ASSERT(0);	// iterator not found!

}


TListIteratorBase::TListIteratorBase(TListBase& list)
	:	fList(&list),
		fIndex(0),
		fNext(NULL)
{
	list.AddIterator(this);
}


TListIteratorBase::~TListIteratorBase()
{
	if (fList)
		fList->RemoveIterator(this);
}


const void* TListIteratorBase::GetNext()
{
	// fList might be NULL if list is deleted during iteration
	if (fList && fIndex < fList->GetSize())
		return fList->GetAt(fIndex++);
	else
		return NULL;
}

		
void TListIteratorBase::ItemInserted(int32 index)
{
	if (index < fIndex)
		fIndex++;
}


void TListIteratorBase::ItemDeleted(int32 index)
{
	if (index < fIndex)
		fIndex--;

}
