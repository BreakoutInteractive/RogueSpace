//
//  Counter.cpp
//  RS
//
//  Created by Zhiyuan Chen on 2/26/24.
//

#include "Counter.hpp"

void Counter::setCount(int count){
    assert (count >= 0 && count <= _maxCount);
    _count = count;
}

void Counter::setMaxCount(int value, bool reset){
    assert (_adjustable);
    assert (value > 0);
    _maxCount = value;
    if (reset){
        _count = 0;
    }
    else {
        _count = std::min(_count, _maxCount);
    }
}

void Counter::decrement(bool cycle){
    if (cycle && _count == 0){
        _count = _maxCount;
        return;
    }
    _count = std::max(0, _count - 1);
}

void Counter::increment(){
    if (_count == _maxCount){
        return;
    }
    _count+=1;
}
