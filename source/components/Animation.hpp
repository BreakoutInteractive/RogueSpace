//
//  Animation.hpp
//
//  Author: Zhiyuan Chen
//  Version: 3/13/24

#ifndef __ANIMATION_HPP__
#define __ANIMATION_HPP__
#include <cugl/cugl.h>

using namespace cugl;


/**
 This class provides a very simple interface to play animations and attach callback events at different timestamps of the animation. An animation can be defined as a subsequence of a spritesheet so
 multiple animations may share the same animation object.
 */
class Animation {
    
private:
    /** the set of callbacks to execute (the callbacks are sorted) */
    std::map<float, std::function<void()>> _callbacks;
    /** the next callback to execute (through an iterator)*/
    std::map<float, std::function<void()>>::iterator _nextCallback;
    
    bool _started;
    
    /** the time since the start of the animation */
    float _elapsed;
    /** how long one cycle of the animation takes */
    float _duration;
    /** whether the animation is stopped */
    bool _stop;
    /** whether the animation loops */
    bool _looping;
    
    int _startIndex;
    int _endIndex;
    int _frameCount;
    
    /** the reference to the underlying sprite sheet */
    std::shared_ptr<cugl::SpriteSheet> _filmStrip;
    
    //TODO: support non-uniform timesteps
    /** The amount of time for each frame */
    std::vector<float> _timestep;
    /** Whether or not the timestep is uniform */
    bool _uniform;
    
    void runCallbacks();
    
public:
#pragma mark -
#pragma mark Constructors
    
    /**
     * creates an animation object for the given sprite sheet and duration.
     */
    Animation(std::shared_ptr<cugl::SpriteSheet> filmStrip, float duration, bool looping, int startIndex, int endIndex);
    
    /**
     * allocates an animation object for the given sprite sheet, duration, and frame range (start, end)
     */
    static std::shared_ptr<Animation> alloc(std::shared_ptr<cugl::SpriteSheet> filmStrip, float duration, bool looping, int startIndex, int endIndex){
        return std::make_shared<Animation>(filmStrip, duration, looping, startIndex, endIndex);
    }
    
    /**
     * allocates an animation object for the given sprite sheet and duration in seconds
     */
    static std::shared_ptr<Animation> alloc(std::shared_ptr<cugl::SpriteSheet> filmStrip, float duration, bool looping){
        return std::make_shared<Animation>(filmStrip, duration, looping, 0, filmStrip->getSize()-1);
    }
    /**
     * destroys the animation object and releases all resources
     */
    ~Animation(){
        _filmStrip = nullptr;
    }
    
#pragma mark -
#pragma mark Attributes
    
    /** 
     * An animation can be paused midway.
     * @return whether the animation is stopped
     */
    bool isPaused() { return _stop; }
    
    /**
     * an active animation is one that is not stopped nor completed.
     * Obviously, this requires that the aniimation has been started.
     * @return whether the animation is active
     */
    bool isActive() { return _started && !_stop && !isCompleted();}
    
    /**
     * @return whether the animation has been started
     */
    bool isStarted() { return _started; }

    /**
     * @return the time elapsed since the start of this animation cycle
     */
    float elapsed() { return _elapsed; }
    
    /**
     * @return the time (seconds) the animation takes in one looping cycle
     */
    float getDuration(){ return _duration; }
    
    /**
     * sets the time (seconds) the animation takes in one looping cycle.
     * @note changing the duration may have undesirable side effects which should be handled before using the animation. For instance, callbacks do not adjust to the new duration.
     * @pre the animation cannot be active, as behavior would otherwise be undefined.
     */
    void setDuration(float duration);
    
    /**
     * adds the callback to be executed at the timestamp `time` seconds into the animation
     * @pre the animation cannot be started.
     */
    void addCallback(float time, std::function<void()>);
    
    /**
     * sets the callback to be executed at the end of the current animation duration
     */
    void onComplete(std::function<void()> func){
        addCallback(_duration, func);
    }
    
    /**
     * clears all internal callbacks assigned to particular times.
     */
    void clearCallbacks(){ _callbacks.clear();}
    
    /**
     * retrieve a reference to the underlying sprite sheet
     */
    std::shared_ptr<cugl::SpriteSheet> getSpriteSheet(){ return _filmStrip; }
    
    
#pragma mark -
#pragma mark Animation Methods
    
    /**
     * this must be called in order for the callback events to become actively processed and for the animation
     * to begin
     */
    void start();
    
    /**
     * resets the animation to the start. You need to call `start` again to run the animation.
     */
    void reset();
    
    /**
     * A looping animation is never complete per se.
     *
     * @return whether the animation is completed.
     */
    bool isCompleted(){
        return !_looping && _elapsed >= _duration;
    }
    
    /**
     * pauses the animation from updates
     */
    void stopAnimation(){ _stop = true; }
    
    /**
     * continues the animation (unpauses)
     */
    void continueAnimation(){ _stop = false; }
    
    /**
     * proceeds the animation forward by `delta` seconds
     * if the current time passes any callbacks, they will be executed once (per loop).
     */
    void update(float delta);

    /**
     * resets the animation and sets the range of the frames. 
     * The range must be valid. There are no other constraints.
     */
    void resetFrameRange(int startIndex, int endIndex);
    
    /**
     * sets the animation frame range to another range of the same length.
     *
     * this is useful for switching to different parts of spritesheet (say 8 filmstrips being stored in one texture)
     */
    void setFrameRange(int startIndex, int endIndex);
    
};

#endif /* __ANIMATION_HPP__ */
