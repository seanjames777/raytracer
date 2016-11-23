#ifndef __UTIL_VECTOR_H
#define __UTIL_VECTOR_H

#include <util/align.h>

// TODO: make sure these get inlined

namespace util {

	template<typename T, size_t A>
	class vector {
	private:

		T     *_data;
		size_t _size;
		size_t _capacity;

	public:

		vector(size_t capacity = 4)
			: _data((T *)aligned_alloc(sizeof(T) * capacity, A)),
			  _size(0),
			  _capacity(capacity)
		{
			for (int i = 0; i < capacity; i++)
				new (&_data[i])T();
		}

		~vector() {
			aligned_free(_data);
		}

		void push_back(const T & elem) {
			size_t newCapacity = _capacity;

			while (_size + 1 > newCapacity)
				newCapacity *= 2;

			if (_capacity != newCapacity) {
				T *newData = (T *)aligned_alloc(sizeof(T) * newCapacity, A);

				for (int i = 0; i < newCapacity; i++)
					new (&newData[i])T();

				for (int i = 0; i < _size; i++)
					newData[i] = _data[i];

				aligned_free(_data);
				_data = newData;
				_capacity = newCapacity;
			}

			_data[_size++] = elem;
		}

		const T & back() const {
			return _data[_size - 1];
		}

		T & back() {
			return _data[_size - 1];
		}

		const T *begin() const {
			return &_data[0];
		}

		T *begin() {
			return &_data[0];
		}

		const T *end() const {
			return &_data[_size];
		}

		T *end() {
			return &_data[_size];
		}

		const T & operator[](size_t i) const {
			return _data[i];
		}

		T & operator[](size_t i) {
			return _data[i];
		}

		void pop_back() {
			--_size;
		}

		size_t size() const {
			return _size;
		}

		bool empty() const {
			return _size == 0;
		}

		void clear() {
			_size = 0;
		}

	};

}

#endif
