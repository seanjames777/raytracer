#ifndef __UTIL_VECTOR_H
#define __UTIL_VECTOR_H

#include <util/align.h>

#include <iostream>

namespace util {

template<typename T, size_t A>
class vector {
private:

    T *_data;
    T *_curr;
    
    size_t _capacity;

public:

    inline vector(size_t capacity = 4)
        : _data((T *)aligned_alloc(sizeof(T) * capacity, A)),
          _curr(_data),
          _capacity(capacity)
    {
    }

    inline vector(const vector& copy)
        : _data((T *)aligned_alloc(sizeof(T) * copy._capacity, A)),
          _curr(_data + (copy._curr - copy._data)),
          _capacity(copy._capacity)
    {
        for (size_t i = 0; i < size(); i++)
            new (&_data[i]) T(copy._data[i]);
    }

    inline vector(vector&& move)
        : _data(move._data),
          _curr(move._curr),
          _capacity(move._capacity)
    {
        move._data = nullptr;
    }

    inline ~vector() {
        if (_data) {
            for (size_t i = 0; i < size(); i++)
                _data[i].~T();

            aligned_free(_data);
        }
    }

    void reserve(size_t newSize) {
        size_t newCapacity = _capacity;

        while (newSize > newCapacity)
            newCapacity *= 2;

        if (_capacity != newCapacity) {
            T *newData = (T *)aligned_alloc(sizeof(T) * newCapacity, A);

            for (size_t i = 0; i < size(); i++) {
                new (&newData[i]) T(std::move(_data[i]));
                _data[i].~T();
            }

            aligned_free(_data);
            _curr = _curr - _data + newData;
            _data = newData;
            _capacity = newCapacity;
        }
    }

    inline void push_back_inbounds(T&& elem) {
        new (_curr++) T(std::move(elem));
    }

    inline void push_back_inbounds(const T& elem) {
        new (_curr++) T(elem);
    }

    inline void push_back(const T& elem) {
        reserve(size() + 1);
        push_back_inbounds(elem);
    }

    inline void push_back(T&& elem) {
        reserve(size() + 1);
        push_back_inbounds(std::move(elem));
    }

    inline const T& back() const {
        return *(_curr - 1);
    }

    inline T& back() {
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

    inline const T& operator[](size_t i) const {
        return _data[i];
    }

    inline T& operator[](size_t i) {
        return _data[i];
    }

    inline void pop_back() {
        (--_curr)->~T();
    }

    inline size_t size() const {
        return _curr - _data;
    }

    inline bool empty() const {
        return _curr == _data;
    }

    inline void clear() {
        for (size_t i = 0; i < size(); i++)
            _data[i].~T();

        _curr = _data;
    }

    inline size_t capacity() const {
        return _capacity;
    }

};

}

#endif
