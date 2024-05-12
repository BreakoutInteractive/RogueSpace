//
//  Animation.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/13/24.
//

#include "Animation.hpp"

Animation::Animation(std::shared_ptr<cugl::SpriteSheet> filmStrip, float duration, bool looping, int startIndex, int endIndex){
    _filmStrip = filmStrip;
    _duration = duration;
    _looping = looping;
    _elapsed = 0;
    _started = false;
    _stop = false;
    _startIndex = startIndex;
    _endIndex = endIndex;
    _frameCount = _endIndex - _startIndex + 1;
    CUAssertLog(duration > 0, "an animation must last longer than 0 ms");
}

void Animation::start(){
    _nextCallback = _callbacks.begin();
    _started = true;
    _filmStrip->setFrame(_startIndex);
}

void Animation::reset(){
    _started = false;
    _stop = false;
    _elapsed = 0;
}

void Animation::resetFrameRange(int startIndex, int endIndex){
    CUAssertLog(endIndex >= startIndex, "end index must be greater than or equal to start");
    CUAssertLog(startIndex >= 0, "start index must be valid");
    reset();
    _startIndex = startIndex;
    _endIndex = endIndex;
    _frameCount = _endIndex - _startIndex + 1;
}

void Animation::setFrameRange(int startIndex, int endIndex){
    CUAssertLog(endIndex >= startIndex && endIndex + 1 - startIndex == _frameCount
                && startIndex >= 0 &&
                endIndex < _filmStrip->getSize() , "start and end does not satisfy prereq");
    _startIndex = startIndex;
    _endIndex = endIndex;
}


void Animation::update(float delta){
    if (!_started || isCompleted()){
        return;
    }
    if (!_stop){
        _elapsed += delta;
        auto time = fmod(_elapsed, _duration);
        int frameIndex = _startIndex + std::min(_frameCount - 1, (int)((time/_duration) * _frameCount));
        _filmStrip->setFrame(frameIndex);
        runCallbacks();
    }
}

void Animation::addCallback(float time, std::function<void ()> callback){
    CUAssertLog(!_started, "You cannot add a callback after animation starts");
    CUAssertLog(time >= 0, "Cannot add event callback when animation is not playing");
    _callbacks[time] = callback;
}

void Animation::runCallbacks(){
    while (_nextCallback != _callbacks.end() && _elapsed >= _nextCallback->first){
        _nextCallback->second();
        _nextCallback++;
    }
    if (_looping && _nextCallback == _callbacks.end()){
        _nextCallback = _callbacks.begin();
    }
}

void Animation::setDuration(float duration){
    CUAssertLog(!_started, "It is unsafe to change animation duration after animation starts");
    CUAssertLog(duration > 0, "an animation must last longer than 0 ms");
    _duration = duration;
}
