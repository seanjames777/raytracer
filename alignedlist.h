/*
 * Sean James
 *
 * alignedlist.h
 *
 * A simple list implementation that guarantees alignedment of elements
 *
 */

#ifndef _ALIGNEDLIST_H
#define _ALIGNEDLIST_H

#define AL_STARTINGCAP 1024

template <class T>
class AlignedList {
	T *items;
	int _size;
	int capacity;
	size_t alignment;

public:

	inline AlignedList() 
		: capacity(AL_STARTINGCAP),
		  _size(0),
		  alignment(sizeof(void *))
	{
		//items = (T *)_aligned_malloc(sizeof(T) * capacity, alignment);
		items = new T[capacity];
	}

	inline AlignedList(size_t Alignment) 
		: capacity(AL_STARTINGCAP),
		  _size(0),
		  alignment(Alignment)
	{
		//items = (T *)_aligned_malloc(sizeof(T) * capacity, alignment);
		items = new T[capacity];
	}

	inline AlignedList(size_t Alignment, int StartingCapacity)
		: capacity(StartingCapacity),
		  _size(0),
		  alignment(Alignment)
	{
		//items = (T *)_aligned_malloc(sizeof(T) * capacity, alignment);
		items = new T[capacity];
	}

	inline ~AlignedList() {
		//_aligned_free(items);
	}

	inline void push(const T & item) {
		if (_size == capacity) {
			//T *newItems = (T *)_aligned_malloc(sizeof(T) * capacity * 2, alignment);
			T *newItems = new T[capacity * 2];

			for (int i = 0; i < _size; i++)
				newItems[i] = items[i];

			//memcpy(newItems, items, sizeof(T) * capacity);
			capacity *= 2;

			delete [] items;

			//_aligned_free(items);
			items = newItems;
		}

		items[_size++] = item;
	}

	inline T & get(int idx) {
		return items[idx];
	}

	inline int size() {
		return _size;
	}

	inline T & operator[] (int idx) {
		return items[idx];
	}

	inline void free() {
		//_aligned_free(items);

		delete [] items;
	}
};

#endif