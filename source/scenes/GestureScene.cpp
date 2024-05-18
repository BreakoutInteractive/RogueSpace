//
//  GestureScene.cpp
//  RS
//
//  Created by Zhiyuan Chen on 5/17/24.
//

#include "GestureScene.hpp"
#include <cugl/cugl.h>

using namespace cugl;

bool GestureScene::init(std::shared_ptr<AssetManager> assets){
    if (assets == nullptr){
        return false;
    }
    auto gestureScene = assets->get<scene2::SceneNode>("gestures");
    float sceneHeight = gestureScene->getHeight();
    auto dimen = Application::get()->getDisplaySize();
    dimen *= sceneHeight/dimen.height;
    if (!Scene2::init(dimen)){
        return false;
    }
    
    _moveGesture = std::dynamic_pointer_cast<scene2::SpriteNode>(assets->get<scene2::SceneNode>("gestures_move_gesture"));
    _dashGesture = std::dynamic_pointer_cast<scene2::SpriteNode>(assets->get<scene2::SceneNode>("gestures_dash_gesture"));
    _attackGesture = std::dynamic_pointer_cast<scene2::SpriteNode>(assets->get<scene2::SceneNode>("gestures_attack_gesture"));
    _shootGesture = std::dynamic_pointer_cast<scene2::SpriteNode>(assets->get<scene2::SceneNode>("gestures_shoot_gesture"));
    _parryGesture = std::dynamic_pointer_cast<scene2::SpriteNode>(assets->get<scene2::SceneNode>("gestures_parry_gesture"));
    _moveGesture->setVisible(false);
    _dashGesture->setVisible(false);
    _attackGesture->setVisible(false);
    _shootGesture->setVisible(false);
    _parryGesture->setVisible(false);
    
    gestureScene->setContentSize(dimen);
    gestureScene->doLayout();
    
    _animateAction = scene2::Animate::alloc();
    _fadeInAction = scene2::FadeIn::alloc(1.0f);
    _fadeOutAction = scene2::FadeOut::alloc(1.0f);
    
    // make an opaque overlay for the gestures (because they're white)
    _overlays.init(Application::get()->getDisplaySize());
    _leftOverlay = scene2::PolygonNode::allocWithPoly(Rect(0, 0, dimen.width/3, sceneHeight));
    _rightOverlay = scene2::PolygonNode::alloc();
    
    _leftOverlay->setVisible(false);
    _rightOverlay->setVisible(false);
    addChild(gestureScene);
    
    Scene2::setActive(false);
    _activeFlag = false;
    return true;
}

void GestureScene::update(float dt){
    _actionManager.update(dt);
    // still transitioning
    if (isActive() != this->_activeFlag){
        if (this->_activeFlag && !_actionManager.isActive(FADE_IN_KEY)){
            // done fading in
            Scene2::setActive(true);
        }
        else if (!this->_activeFlag && !_actionManager.isActive(FADE_OUT_KEY)){
            // done fading out
            _actionManager.remove(ANIMATE_KEY);
            _activeGesture->setVisible(false);
            _activeOverlay->setVisible(false);
            Scene2::setActive(false);
        }
    }
    else{  // finished transitioning, so just animate if active
        if (isActive()){
            if (!_actionManager.isActive(ANIMATE_KEY)){
                _actionManager.activate(ANIMATE_KEY, _animateAction, _activeGesture);
            }
        }
    }
    
}

void GestureScene::setGesture(Gesture g){
    // skip if setting the same values
    if (_activeOverlay != nullptr && _activeGesture != nullptr &&
        gesture == g){
        return;
    }
    // hide anything active
    if (_activeOverlay != nullptr){
        _activeOverlay->setVisible(false);
        _actionManager.clearAllActions(_activeOverlay);
    }
    if (_activeGesture != nullptr){
        _activeGesture->setVisible(false);
        _actionManager.clearAllActions(_activeGesture);
    }
    switch (g) {
        case Gesture::MOVE:
            _activeOverlay = _leftOverlay;
            _activeGesture = _moveGesture;
            _animateAction->init(0, 6, 2.0f); // first frame index, last frame index, duration
            break;
        case Gesture::DASH:
            _activeOverlay = _rightOverlay;
            _activeGesture = _dashGesture;
            _animateAction->init(0, 9, 2.0f);
            break;
        case Gesture::ATTACK:
            _activeOverlay = _rightOverlay;
            _activeGesture = _attackGesture;
            _animateAction->init(0, 9, 1.25f);
            break;
        case Gesture::SHOOT:
            _activeOverlay = _rightOverlay;
            _activeGesture = _shootGesture;
            _animateAction->init(0, 7, 3.0f);
            break;
        case Gesture::PARRY:
            _activeOverlay = _rightOverlay;
            _activeGesture = _parryGesture;
            _animateAction->init(0, 9, 2.0f);
            break;
    }
    gesture = g;
}

GestureScene::Gesture GestureScene::getGestureFromName(std::string name){
    if (name == "PARRY"){
        return Gesture::PARRY;
    }
    if (name == "SHOOT"){
        return Gesture::SHOOT;
    }
    if (name == "ATTACK"){
        return Gesture::ATTACK;
    }
    if (name == "DASH"){
        return Gesture::DASH;
    }
    return Gesture::MOVE;
}

void GestureScene::reset(){
    Scene2::setActive(false);
    this->_activeFlag = false;
    _actionManager.remove(FADE_IN_KEY);
    _actionManager.remove(FADE_OUT_KEY);
    _actionManager.remove(FADE_IN_GESTURE_KEY);
    _actionManager.remove(FADE_OUT_GESTURE_KEY);
    _leftOverlay->setVisible(false);
    _rightOverlay->setVisible(false);
    if (_activeGesture != nullptr){
        _activeGesture->setVisible(false);
        _activeGesture->setFrame(0);
    }
    _activeGesture = nullptr;
    _activeOverlay = nullptr;
}

void GestureScene::setActive(bool value){
    if (isActive() != value && this->_activeFlag != value){
        this->_activeFlag = value;
        if (value){
            // fade in, remove fade out effects
            _actionManager.remove(FADE_OUT_KEY);
            _actionManager.remove(FADE_OUT_GESTURE_KEY);
            // set colors to fade into
            _activeOverlay->setColor(Color4(0,0,0,75));
            _activeGesture->setColor(Color4(255,255,255,255));
            // apply fade-in animations
            _actionManager.activate(FADE_IN_KEY, _fadeInAction, _activeOverlay);
            _actionManager.activate(FADE_IN_GESTURE_KEY, _fadeInAction, _activeGesture);
            // animate the finger
            _actionManager.activate(ANIMATE_KEY, _animateAction, _activeGesture);
            _activeGesture->setVisible(true);
            _activeOverlay->setVisible(true);
        }
        else {
            // fade out, remove fade in effects
            _actionManager.remove(FADE_IN_KEY);
            _actionManager.remove(FADE_IN_GESTURE_KEY);
            // apply fade-out animations
            _actionManager.activate(FADE_OUT_KEY, _fadeOutAction, _activeOverlay);
            _actionManager.activate(FADE_OUT_GESTURE_KEY, _fadeOutAction, _activeGesture);
        }
    }
}

void GestureScene::render(const std::shared_ptr<SpriteBatch> &batch){
    if (_activeOverlay != nullptr && _activeOverlay->isVisible()){
        batch->begin(_overlays.getCamera()->getCombined());
        Vec2 camPos = _overlays.getCamera()->getPosition();
        Size screenSize = Application::get()->getDisplaySize();
        if (_activeOverlay == _leftOverlay){
            // left side 1/3
            batch->draw(nullptr, _leftOverlay->getColor(), Rect(camPos.x - screenSize.width/2, camPos.y - screenSize.height/2, screenSize.width/3, screenSize.height));
        }
        else if (_activeOverlay == _rightOverlay){
            // right side half
            batch->draw(nullptr, _rightOverlay->getColor(), Rect(camPos.x, camPos.y - screenSize.height/2, screenSize.width/2, screenSize.height));
        }
        batch->end();
    }
    Scene2::render(batch);
}
