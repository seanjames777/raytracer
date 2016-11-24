#ifndef __UTIL_VECTOR_H
#define __UTIL_VECTOR_H

#include <util/align.h>

// TODO: make sure these get inlined

namespace util {

	template<typename T, size_t A>
	class vector {
	private:

		T *_data;
		T *_curr;
		
		size_t _capacity;

	public:

		vector(size_t capacity = 4)
			: _data((T *)aligned_alloc(sizeof(T) * capacity, A)),
			  _curr(_data), // TODO: does this work?
			  _capacity(capacity)
		{
			for (int i = 0; i < capacity; i++)
				new (&_data[i])T();
		}

		~vector() {
			aligned_free(_data);
		}

		void reserve(size_t newSize) {
			size_t newCapacity = _capacity;

			while (newSize > newCapacity)
				newCapacity *= 2;

			if (_capacity != newCapacity) {
				T *newData = (T *)aligned_alloc(sizeof(T) * newCapacity, A);

				for (int i = 0; i < newCapacity; i++)
					new (&newData[i])T();

				size_t currSize = size();

				for (int i = 0; i < currSize; i++)
					newData[i] = _data[i];

				aligned_free(_data);
				_data = newData;
				_curr = _data + currSize;
				_capacity = newCapacity;
			}
		}

		inline void push_back_inbounds(const T &elem) {
			*(_curr++) = elem;
		}

		// TODO: make this inlineable
		inline void push_back(const T & elem) {
			reserve(size() + 1);
			push_back_inbounds(elem);
		}

		inline const T & back() const {
			return *(_curr - 1);
		}

		inline T & back() {
			return *(_curr - 1);
		}

		inline const T *begin() const {
			return _data;
		}

		inline T *begin() {
			return _data;
		}

		inline const T *end() const {
			return _curr;
		}

		inline T *end() {
			return _curr;
		}

		inline const T & operator[](size_t i) const {
			return _data[i];
		}

		inline T & operator[](size_t i) {
			return _data[i];
		}

		inline void pop_back() {
			--_curr;
		}

		inline size_t size() const {
			return _curr - _data;
		}

		inline bool empty() const {
			return _curr == _data;
		}

		inline void clear() {
			_curr = _data;
		}

	};

}

#endif
