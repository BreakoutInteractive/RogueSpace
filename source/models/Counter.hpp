//
//  Counter.hpp
//  RS
//
//  Created by Zhiyuan Chen on 2/26/24.
//

#ifndef Counter_hpp
#define Counter_hpp

#include <cugl/cugl.h>

/**
 Model to represent a counter with support to allow adjustable maximums or constant maximum. Automatically
 decrements counter or resets counter through update().
 */
class Counter {
private:
    
    /** current remaining count */
    int _count;
    
    /** initial value of counter*/
    int _maxCount;
    
    /** whether the counter can be adjusted after initialization */
    bool _adjustable;
    
public:
    
    /** stores the name of this counter object for debugging purposes*/
    std::string name;
    
    /**
     * Initializes an adjustable counter at 0 with counter length (max counter value) being integer maximum.
     */
    Counter(void): _maxCount(INT_MAX), _count(0), _adjustable(true){}
    
    /**
     * Initializes a counter at 0 with counter cooldown being given maximum count
     *
     * @param maxCount counter cooldown
     * @param adjustable whether the maximum can be adjusted to another value
     */
    Counter(int maxCount, bool adjustable){
        Counter(maxCount, "", adjustable);
    }
    
    /**
     * Initializes a counter at 0 with counter cooldown being given maximum count
     *
     * @param maxCount counter cooldown
     * @param tag a name for this counter
     * @param adjustable whether the maximum can be adjusted to another value
     */
    Counter(int maxCount, std::string tag, bool adjustable):
        _maxCount(maxCount), _count(0), name(tag), _adjustable(adjustable){
            assert(maxCount > 0);
        }
    
    /**
     * Sets the internal counter to the given value
     *
     * @param count counter value
     * @pre count must be nonnegative and less than or equal to maximum
     * @throw assertion error if precondition is violated
     */
    void setCount(int count);
    
    /**
     * see `isZero` for checking if counter is complete.
     *
     * @return current counter value
     */
    int getCount(){ return _count; }
    
    /**
     * @return whether the counter is at 0
     */
    bool isZero(){ return _count == 0;}
    
    /**
     * @return whether the counter is at maximum value
     */
    bool isMaximum(){ return _count == _maxCount; }
    
    /**
     * Sets the interal counter initial value (maximum count).
     *
     * @param value counter maximum value
     * @param reset whether the counter should reset to 0 or take on a value that is the minimum of current count and new maximum
     * @pre This requires that the counter is set to adjustable.
     * @throw assertion error if precondition is violated
     */
    void setMaxCount(int value, bool reset = false);
    
    int getMaxCount(){return _maxCount;}
    
    /**
     * decrements the internals of the counter (nothing happens when counter is at 0),
     *
     * to cycle the counter, `cycle` parameter must be true.
     */
    void decrement(bool cycle = false);
    
    /** increments the internals of the counter (not to exceed the maximum count value)*/
    void increment();
    
    /**
     * Resets the counter to maximum count
     */
    void reset(){ _count = _maxCount;}
};



#endif /* Counter_hpp */
