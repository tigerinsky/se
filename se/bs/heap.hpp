#include "heap.h" 
#include <algorithm>

namespace tis { namespace bs {

template<typename T, typename Compare> 
void Heap<T, Compare>::push(T t) {
    _heap.push_back(t);
    std::push_heap(_heap.begin(), _heap.end(), _comp);
}

template<typename T, typename Compare>
T Heap<T, Compare>::pop() {
    std::pop_heap(_heap.begin(), _heap.end(), _comp);
    T value = _heap.back();
    _heap.pop_back();
    return value;
}

}}
