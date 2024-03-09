//
//  JoyStick.cpp
//  RS
//
//  Created by Dasha Griffiths on 3/4/24.
//

#include <math.h>
#include "JoyStick.hpp"


using namespace cugl;

#define HOLD_TIME 25


#pragma mark JoyStick
/**
 * Creates an joystick with the default values.
 */

JoyStick::Ball::Ball(const cugl::Vec2 p) {
    position = p;
}

/**
 * Moves the joystick ball one animation frame
 */
void JoyStick::Ball::update(const cugl::Vec2 p) {
    position = p;
}

#pragma mark The Base

JoyStick::JoyStick() {
    _basePosition = Vec2::ZERO;;
}

JoyStick::JoyStick(const cugl::Vec2 p, cugl::Vec2 scale){
    _basePosition = p;
    _active = false;
    _time=0;
    _joyBall = std::make_shared<Ball>(_basePosition);
}


void JoyStick::updateBasePos(cugl::Vec2 inputPos) {
    _basePosition = inputPos;
    _joyBall->position = _basePosition;
    _active = true;
}

/**
 * Moves the active joystick.
 */
void JoyStick::updateBallPos(cugl::Vec2 inputDir, cugl::Vec2 inputPos) {
    if (inputPos.distance(_basePosition)<_radius){
        _joyBall->position=inputPos;
    }else{
        cugl::Vec2 touch2base(inputPos.x-_basePosition.x,inputPos.y-_basePosition.y);

        float angle = atan2(touch2base.y, touch2base.x);
        float xDist = sin(angle-1.5708)*_radius;
        float yDist = cos(angle-1.5708)*_radius;
        
        _joyBall->position = Vec2(_basePosition.x-xDist, _basePosition.y+yDist);
    }
}

void JoyStick::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _ballTexture = assets->get<Texture>("stick");
    _baseTexture = assets->get<Texture>("joystick_base");
}

void JoyStick::setActive(bool active){
    _active=active;
    if(!active){
        _joyBall->position = _basePosition;
        _time = 0;
    }
}

void JoyStick::setDrawScale(cugl::Vec2 scale){
    _drawBaseScale=scale;
    _joyBall->_drawBallScale=scale;
}
/**
 * Draws active joystick to the sprite batch within the given bounds.
 *
 * @param batch     The sprite batch to draw to
 */
void JoyStick::draw(const std::shared_ptr<SpriteBatch>& batch) {
    if (_active && _baseTexture && _ballTexture) {
        Vec2 baseOrigin(_baseTexture->getWidth()/2,_baseTexture->getHeight()/2);
        Vec2 ballOrigin(_ballTexture->getWidth()/2,_ballTexture->getHeight()/2);

        _radius = _drawBaseScale.x*(fmax(_baseTexture->getHeight(), _baseTexture->getWidth())/2);
            
        Affine2 transBase;
        transBase.scale(_drawBaseScale);
        transBase.translate(_basePosition.x-(_drawBaseScale.x*baseOrigin.x), _basePosition.y);
            
        Affine2 transBall;
        transBall.scale(Vec2(_drawBaseScale));
        transBall.translate(_joyBall->position.x-(_drawBaseScale.x*baseOrigin.x), _joyBall->position.y);
            
        batch->draw(_ballTexture, Color4(Vec4(1,1,1,.5*_time/HOLD_TIME)), ballOrigin, transBall);
        batch->draw(_baseTexture, Color4(Vec4(1,1,1,.5*_time/HOLD_TIME)), baseOrigin, transBase);
            
        if(_time<HOLD_TIME){
            _time+=1;
        }
    }
}