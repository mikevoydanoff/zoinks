// ========================================================================================
//	TDynamicArray.h		 			Copyright (C) 2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TDynamicArray__
#define __TDynamicArray__

class TDynamicArrayBase
{
public:
	TDynamicArrayBase(uint32 elementSize, uint32 allocationIncrement);
	~TDynamicArrayBase();

	inline uint32		GetSize() const { return fSize; }
	void 				RemoveAll();

protected:
	void				Allocate(uint32 count);
	void 				InsertItemFirst(const void* item);
	void 				InsertItemLast(const void* item);
	void 				InsertItemAt(const void* item, uint32 index);

	void 				RemoveItemAt(uint32 index);
	const void*			GetAt(uint32 index) const;

private:	
	char*				fData;
	uint32				fSize;
	uint32				fElementSize;
	uint32				fAllocatedSize;
	uint32				fAllocationIncrement;
};


template <class T>
class TDynamicArray : public TDynamicArrayBase
{
public:
	inline TDynamicArray(int allocationIncrement = 10)
		:	TDynamicArrayBase(sizeof(T), allocationIncrement) {}

	inline void 	InsertFirst(T& item) { InsertItemFirst(&item); }
	inline void 	InsertLast(T& item) { InsertItemLast(&item); }
	inline void 	InsertAt(T& item, uint32 index) { InsertItemAt(&item, index); }
	inline void 	RemoveAt(uint32 index) { RemoveItemAt(index); }
	inline T&		operator[](int i) const { return *(T*)GetAt(i); }
	inline T&		First() const { return *(T*)GetAt(0); }
	inline T&		Last() const { return *(T*)GetAt(GetSize() - 1); }
};



#endif // __TDynamicArray__
