/**
 * @file util/queue.h
 *
 * @brief Queue/FIFO
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __UTIL_QUEUE_H
#define __UTIL_QUEUE_H

#include <util/align.h>

namespace util {

template<typename T, size_t A>
class queue {
private:

    T      *_data;
    size_t  _head;
    size_t  _tail;
    size_t  _size;
    size_t  _capacity;

public:

    inline queue(size_t capacity = 4)
        : _data((T *)aligned_alloc(sizeof(T) * capacity, A)),
          _head(0),
          _tail(0),
          _size(0),
          _capacity(capacity)
    {
    }

    inline queue(const queue& copy)
        : _data((T *)aligned_alloc(sizeof(T) * copy._capacity, A)),
          _head(copy._head),
          _tail(copy._tail),
          _size(copy._size),
          _capacity(copy._capacity)
    {
        if (_tail == _head && _size > 0) {
            for (size_t i = 0; i < _capacity; i++)
                new (&_data[i]) T(copy._data[i]);
        }
        else if (_tail < _head) {
            for (size_t i = _tail; i < _head; i++)
                new (&_data[i]) T(copy._data[i]);
        }
        else {
            for (size_t i = _tail; i < _capacity; i++)
                new (&_data[i]) T(copy._data[i]);

            for (size_t i = 0; i < _head; i++)
                new (&_data[i]) T(copy._data[i]);
        }
    }

    inline queue(queue&& move)
        : _data(move._data),
          _head(move._head),
          _tail(move._tail),
          _size(move._size),
          _capacity(move._capacity)
    {
        move._data = nullptr;
    }

    inline ~queue() {
        if (_data) {
            if (_tail == _head && _size > 0) {
                for (size_t i = 0; i < _capacity; i++)
                    _data[i].~T();
            }
            else if (_tail < _head) {
                for (size_t i = _tail; i < _head; i++)
                    _data[i].~T();
            }
            else {
                for (size_t i = _tail; i < _capacity; i++)
                    _data[i].~T();

                for (size_t i = 0; i < _head; i++)
                    _data[i].~T();
            }

            aligned_free(_data);
        }
    }

     void reserve(size_t newSize) {
        size_t newCapacity = _capacity;

        while (newSize > newCapacity)
            newCapacity *= 2;

        if (_capacity != newCapacity) {
            T *newData = (T *)aligned_alloc(sizeof(T) * newCapacity, A);

            if (_tail == _head && _size > 0) {
                for (size_t i = 0; i < _capacity - _tail; i++) {
                    new (&newData[i]) T(std::move(_data[i + _tail]));
                    _data[i + _tail].~T();
                }

                for (size_t i = 0; i < _head; i++) {
                    new (&newData[i + _capacity - _tail]) T(std::move(_data[i]));
                    _data[i].~T();
                }

                _tail = 0;
                _head = _capacity;
            }
            else if (_tail < _head) {
                for (size_t i = 0; i < _head - _tail; i++) {
                    new (&newData[i]) T(std::move(_data[i + _tail]));
                    _data[i + _tail].~T();
                }

                _tail = 0;
                _head = _head - _tail;
            }
            else {
                for (size_t i = 0; i < _capacity - _tail; i++) {
                    new (&newData[i]) T(std::move(_data[i + _tail]));
                    _data[i + _tail].~T();
                }

                for (size_t i = 0; i < _head; i++) {
                    new (&newData[i + _capacity - _tail]) T(std::move(_data[i]));
                    _data[i].~T();
                }

                _tail = 0;
                _head = _capacity - _tail + _head;
            }

            // Delete the old queue
            aligned_free(_data);
            _data = newData;
            _capacity = newCapacity;
        }
    }

    inline void push_back(const T& elem) {
        reserve(size() + 1);

        new (&_data[_head]) T(elem);

        _head++;
        _size++;

        if (_head == _capacity)
            _head = 0;
    }

    inline void push_back(T&& elem) {
        reserve(size() + 1);

        new (&_data[_head]) T(elem);

        _head++;
        _size++;

        if (_head == _capacity)
            _head = 0;
    }

    inline const T& front() const {
        return _data[_tail];
    }

    inline T& front() {
        return _data[_tail];
    }

    inline void pop_front() {
        _data[_tail].~T();

        _tail++;

        if (_tail == _capacity)
            _tail = 0;

        _size--;
    }

    inline bool empty() const {
        return _size == 0;
    }

    inline size_t size() const {
        return _size;
    }

    inline size_t capacity() const {
        return _capacity;
    }

    inline void clear() {
        if (_tail == _head && _size > 0) {
            for (size_t i = 0; i < _capacity; i++)
                _data[i].~T();
        }
        else if (_tail < _head) {
            for (size_t i = _tail; i < _head; i++)
                _data[i].~T();
        }
        else {
            for (size_t i = _tail; i < _capacity; i++)
                _data[i].~T();

            for (size_t i = 0; i < _head; i++)
                _data[i].~T();
        }

        _head = 0;
        _tail = 0;
        _size = 0;
    }

};

}

#endif
