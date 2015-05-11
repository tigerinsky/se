#ifndef __BS_HEAP_H_
#define __BS_HEAP_H_

#include <vector>
#include <functional>

namespace tis { namespace bs {

template<typename T, typename Compare=std::greater<T> >
class Heap {
public:
    Heap() {}
    virtual ~Heap() {}

    void push(T t);
    T pop();
    
    void clear() { _heap.clear(); }
    size_t size() { return _heap.size(); }
    typename std::vector<T>::const_iterator begin() { return _heap.begin(); }
    typename std::vector<T>::const_iterator end() { return _heap.end(); }

private:
    std::vector<T> _heap; 
    Compare _comp;
};

}}

#endif
