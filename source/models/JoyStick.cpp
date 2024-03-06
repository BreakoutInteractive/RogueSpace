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
JoyStick::Ball::Ball() {
    position = cugl::Vec2::ZERO;
    angle = 0;
}
/**
 * Creates an joystick with the default values.
 */
JoyStick::Ball::Ball(const cugl::Vec2 p, float ang) {
    position = p;
    angle = ang;
}

JoyStick::Ball::Ball(const cugl::Vec2 p) {
    position = p;
    angle = 0;
}

/**
 * Moves the joystick ball one animation frame
 */
void JoyStick::Ball::update(const cugl::Vec2 p, float ang) {
    position = p;
    angle=ang;
}

#pragma mark The Base

JoyStick::JoyStick() {
    _basePosition = cugl::Vec2::ZERO;
}

JoyStick::JoyStick(const cugl::Vec2 p, cugl::Vec2 scale){

    _basePosition = Vec2(1024/2,576/2);
    _active = true;
    time=0;
    _drawBaseScale=scale;
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
void JoyStick::updateBallPos(cugl::Vec2 inputDir, cugl::Vec2 inputPos) { //stick a little ahead of input
    if (inputPos.distance(_basePosition)<_radius){
        _joyBall->position=inputPos;
    }else{
        cugl::Vec2 touch2base(inputPos.x-_basePosition.x,inputPos.y-_basePosition.y);

        float angle = atan2(touch2base.y, touch2base.x);
        float xDist = sin(angle-1.5708)*_radius;
        float yDist = cos(angle-1.5708)*_radius;
        
        _joyBall->position = Vec2(_basePosition.x-xDist, _basePosition.y+yDist);
        _joyBall->angle=angle;

    }
}

void JoyStick::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _ballTexture = assets->get<Texture>("stick");
    _baseTexture = assets->get<Texture>("joystick_base");
    _radius = fmax(_baseTexture->getHeight(), _baseTexture->getWidth())/2;
    
}

void JoyStick::setActive(bool active){
    _active=active;
    if(!active){
        _joyBall->position = _basePosition;
        time = 0;
    }
}

void JoyStick::setDrawScale(cugl::Vec2 scale){
    _drawBaseScale = (Vec2)_baseTexture->getSize()/(4*scale);
    _joyBall->_drawBallScale=(Vec2)_ballTexture->getSize()/(4*scale);

}
/**
 * Draws active joystick to the sprite batch within the given bounds.
 *
 * @param batch     The sprite batch to draw to
 */
void JoyStick::draw(const std::shared_ptr<SpriteBatch>& batch) {
    if (_active &&_baseTexture && _ballTexture) {
        Vec2 baseOrigin(_baseTexture->getWidth()/2,_baseTexture->getHeight()/2);
        Vec2 ballOrigin(_ballTexture->getWidth()/2,_ballTexture->getHeight()/2);

        Affine2 transBase;
        transBase.scale(_drawBaseScale);
        transBase.translate(_basePosition.x, _basePosition.y);
        
        Affine2 transBall;
        transBall.scale(_drawBaseScale);
        transBall.translate(_joyBall->position.x, _joyBall->position.y);

        
        batch->draw(_ballTexture, Color4(Vec4(1,1,1,.5*time/HOLD_TIME)), ballOrigin, transBall);
        batch->draw(_baseTexture, Color4(Vec4(1,1,1,.5*time/HOLD_TIME)), baseOrigin, transBase);
        
        batch->draw(_ballTexture, Color4(Vec4(1,1,1,1)), ballOrigin, transBall);
        batch->draw(_baseTexture, Color4(Vec4(1,1,1,1)), baseOrigin, transBase);
        
        if (time<HOLD_TIME){
            time+=1;
        }
                
    }
}
