#ifndef __RAYBUFFER_H
#define __RAYBUFFER_H

#define DEFAULT_RAY_BUFFER_CAPACITY 128 // TODO: Tune

class RayBuffer {
private:

    size_t  size;
    size_t  _capacity;
    Ray    *rays;

public:

    RayBuffer(size_t _capacity = DEFAULT_RAY_BUFFER_CAPACITY)
        : size(0),
          _capacity(_capacity),
          rays(new Ray[_capacity])
    {
    }

    ~RayBuffer() {
        delete [] rays;
    }

    inline void enqueue(const Ray & ray) {
        if (size + 1 > _capacity) {
            size_t new_capacity = _capacity << 1;

            Ray *new_rays = new Ray[new_capacity];

            for (size_t i = 0; i < _capacity; i++)
                new_rays[i] = rays[i];

            delete [] rays;
            rays = new_rays;
            _capacity = new_capacity;
        }

        rays[size++] = ray;
    }

    inline Ray dequeue() {
        return rays[--size];
    }

    inline bool empty() const {
        return size == 0;
    }

    inline size_t capacity() {
        return _capacity;
    }

};

#endif
