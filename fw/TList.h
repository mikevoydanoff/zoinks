// ========================================================================================
//	TList.h		 				Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TList__
#define __TList__

class TListIteratorBase;

class TListBase
{
public:
	typedef int (* CompareFunc)(const void* item1, const void* item2, void* compareData);
	typedef int (* SearchFunc)(const void* item, const void* key, void* compareData);

	// unsorted list constructor
	TListBase(int32 initialCount, int allocationIncrement);
	// sorted list constructor
	TListBase(CompareFunc compareFunc, SearchFunc searchFunc, void* compareData,
					int32 initialCount, int allocationIncrement);
	~TListBase();

	inline int32		GetSize() const { return fSize; }
	inline bool			IsSorted() const { return (fCompareFunc != NULL); }

	inline void			SetCompareData(void* data) { fCompareData = data; }

	int32				FindIndex(const void* key) const;
	void 				RemoveAll();

protected:
	friend class TListIteratorBase;

	void				AddIterator(TListIteratorBase* iterator);
	void				RemoveIterator(TListIteratorBase* iterator);

	void				Allocate(int32 count);
	void 				InsertItem(const void* item);
	void 				InsertItemFirst(const void* item);
	void 				InsertItemLast(const void* item);
	void 				InsertItemAt(const void* item, int32 index);

	void 				RemoveItem(const void* item);
	void 				RemoveItemAt(int32 index);
	const void*			GetAt(int32 index) const;
	const void*			DoSearch(const void* key) const;
	bool				ContainsItem(const void* item) const;	

private:	
	const void**		fData;
	int32				fSize;
	int32				fAllocatedSize;
	int32				fAllocationIncrement;

	// for searching
	CompareFunc			fCompareFunc;
	CompareFunc			fSearchFunc;
	void*				fCompareData;
	
	TListIteratorBase*	fIterators;				// linked list
};

template <class T>
class TList : public TListBase
{
public:
	inline TList(int32 initialCount = 0, int allocationIncrement = 10)
		:	TListBase(initialCount, allocationIncrement) {}
	inline TList(CompareFunc compareFunc, SearchFunc searchFunc, void* compareData = NULL,
						int32 initialCount = 0, int allocationIncrement = 10)
		:	TListBase(compareFunc, searchFunc, compareData, initialCount, allocationIncrement) {}

	inline void 	Insert(T* item) { InsertItem(item); }
	inline void 	InsertFirst(T* item) { InsertItemFirst(item); }
	inline void 	InsertLast(T* item) { InsertItemLast(item); }
	inline void 	InsertAt(T* item, int32 index) { InsertItemAt(item, index); }
	inline void 	Remove(T* item) { RemoveItem(item); }
	inline void 	RemoveAt(int32 index) { RemoveItemAt(index); }
	inline void 	DeleteAt(int32 index) { T* item = (T*)GetAt(index); RemoveItemAt(index); delete item; }
	void			DeleteAll();
	inline bool		Contains(const T* item) const { return ContainsItem(item); }
	inline T*		operator[](int i) const { return (T*)GetAt(i); }
	inline T*		First() const { return (T*)GetAt(0); }
	inline T*		Last() const { return (T*)GetAt(GetSize() - 1); }
	inline T*	 	Search(const void* key) const { return (T*)DoSearch(key); }	// can we templatize "key"?
};


class TListIteratorBase
{
public:
					TListIteratorBase(TListBase& list);
					~TListIteratorBase();
	const void*		GetNext();

	inline int32 	CurrentIndex() const { return fIndex; }
private:
	friend class TListBase;
		
	void			ItemInserted(int32 index);
	void			ItemDeleted(int32 index);
	inline void		ListDeleted() { fList = NULL; }

private:
	TListBase*			fList;
	int32				fIndex;
	TListIteratorBase*	fNext;	// next iterator in linked list
};


template <class T>
class TListIterator : public TListIteratorBase
{
public:
	inline		TListIterator(TList<T>& list) : TListIteratorBase(list) {}
	inline		~TListIterator() {}
	inline T* 	Next() { return (T*)GetNext(); }

private:
};


template <class T>
inline void TList<T>::DeleteAll()
{
	TListIterator<T> iter(*this);
	T* item;
	while ((item = iter.Next()) != NULL)
		delete item;

	RemoveAll();
}

#endif // __TList__
