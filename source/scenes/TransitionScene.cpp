//
//  TransitionScene.cpp
//  RS
//
//  Created by Zhiyuan Chen on 4/30/24.
//

#include <cugl/cugl.h>
#include "TransitionScene.hpp"

using namespace cugl;

bool TransitionScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    Size dimen = Application::get()->getDisplaySize();
    Scene2::init(dimen);
    _overlay = scene2::PolygonNode::allocWithPoly(Rect(0, 0, dimen.width, dimen.height));
    addChild(_overlay);
    setActive(false);
    _assets = assets; // if we ever need anything from asset directory
    _fadeInDuration = _fadeOutDuration = 1;
    resetTransition();
    resetCallbacks();
    _startColor = Color4(0,0,0,0);
    _fadeInColor = Color4::WHITE;
    _fadeOutColor = _startColor;
    return true;
}

void TransitionScene::resetCallbacks(){
    _fadeInCallback = [](){};
    _fadeOutCallback = [](){};
}

void TransitionScene::resetTransition(){
    _fadeInElapsed = 0;
    _fadeOutElapsed = 0;
}

void TransitionScene::dispose(){
    if (isActive()) {
        removeAllChildren();
        setActive(false);
    }
}

void TransitionScene::setFadeIn(float duration, Color4 color){
    CUAssertLog(duration > 0, "fade in must last for time greater than 0");
    _fadeInDuration = duration;
    _fadeInColor = color;
}

void TransitionScene::setFadeOut(float duration, Color4 color){
    CUAssertLog(duration > 0, "fade out must last for time greater than 0");
    _fadeOutDuration = duration;
    _fadeOutColor = color;
}

void TransitionScene::setActive(bool value){
    if (isActive() != value) {
        Scene2::setActive(value);
        resetTransition();
        if (value){
            _transitionState = FADEIN;
        }
    }
}

void TransitionScene::update(float dt){
    if (isActive()){
        if (_transitionState == FADEIN){
            _fadeInElapsed += dt;
            float alpha = _fadeInElapsed/_fadeInDuration;
            _overlay->setColor(_startColor.getLerp(_fadeInColor, alpha));
            if (alpha >= 1){
                _transitionState = FADEOUT;
                _fadeInCallback();
            }
        }
        else {
            _fadeOutElapsed += dt;
            float alpha = _fadeOutElapsed/_fadeOutDuration;
            _overlay->setColor(_fadeInColor.getLerp(_fadeOutColor, alpha));
            if (alpha >= 1){
                _fadeOutCallback();
                setActive(false);
            }
        }
    }
}
