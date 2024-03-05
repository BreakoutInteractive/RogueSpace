//
//  JoyStick.cpp
//  RS
//
//  Created by Dasha Griffiths on 3/4/24.
//

#include <math.h>
#include "JoyStick.hpp"

using namespace cugl;

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
    _basePosition = p;
    _active = false;
    _drawBaseScale=scale;
    _joyBall = std::make_shared<Ball>(_basePosition);
}


void JoyStick::updateBasePos(cugl::Vec2 inputPos) { //stick a little ahead of input  //negate y because screen origin is different from game origin
    //reposition joystick at player touch position then set active to true
    _basePosition = inputPos.subtract(107, 3);
    
    _active = true;
    _joyBall->position = _basePosition;
    
     _basePosition = inputPos.divide(_drawBaseScale);
    //    _basePosition = inputPos.subtract(Vec2(_baseTexture->getWidth()/2,_baseTexture->getHeight()/2)); x=377.160309 y 293.618317    1.40625
    
//    _basePosition = inputPos.subtract(Vec2(_drawBaseScale.x*_baseTexture->getWidth()/2,_drawBaseScale.y*_baseTexture->getHeight()/2));
//    _basePosition = inputPos;
//    _joyBall->position = inputPos;
    
}

/**
 * Moves the active joystick.
 */
void JoyStick::updateBallPos(cugl::Vec2 inputDir, cugl::Vec2 inputPos) { //stick a little ahead of input
//    CULog("pos x: %f, y: %f", inputDir.x,inputDir.y);
//    CULog("rad x: %f, y: %f", (_radius/_drawBaseScale).x,(_radius).y); 45,45
    Vec2 ballPos = _joyBall->position;
    
    if (_joyBall->position-inputPos <_radius/_drawBaseScale) {
        //check if ball within radius
//        _joyBall->position=inputPos;
    }

    
}

void JoyStick::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _ballTexture = assets->get<Texture>("stick");
    _baseTexture = assets->get<Texture>("joystick_base");
    _radius = fmax(_baseTexture->getHeight(), _baseTexture->getWidth())/2;
    
}

void JoyStick::setDrawScale(cugl::Vec2 scale){
    _drawBaseScale = (Vec2)_baseTexture->getSize()/(2*scale);
    _joyBall->_drawBallScale=(Vec2)_ballTexture->getSize()/(2*scale);
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
        transBase.translate(_basePosition);
        
        Affine2 transBall;
        transBall.scale(_drawBaseScale);
        transBall.translate(_joyBall->position);
        
        batch->draw(_baseTexture, baseOrigin, transBase);
        batch->draw(_ballTexture, ballOrigin, transBall);

        
        
        
        //        Vec2 copyPos = _joyBall->position;
        //        copyPos.add(_radius,_radius);
        //        transBall.translate(copyPos);

//          batch draw(texture, color, origin, scale, angle, offset)
//        batch->draw(_baseTexture, baseOrigin, _drawBaseScale, 0, (_basePosition)*_drawBaseScale);
//        batch->draw(_ballTexture, ballOrigin, _joyBall->_drawBallScale, 180, (_joyBall->position)*_joyBall->_drawBallScale);
//        batch->draw(_baseTexture, baseOrigin, _drawBaseScale, 0, (_baseTexture->getSize())*_drawBaseScale);
//        batch->draw(_ballTexture, ballOrigin, _joyBall->_drawBallScale, _joyBall->angle, _ballTexture->getSize()*_joyBall->_drawBallScale);

    }
}
