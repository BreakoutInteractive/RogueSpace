//
//  GameRenderer.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/8/24.
//

#include "GameRenderer.hpp"
#include "JoyStick.hpp"

GameRenderer::GameRenderer(){
    /** Create joystick */
    Scene2();
    _joystick = std::make_shared<JoyStick>(Vec2::ZERO, Vec2::ZERO);
}


GameRenderer::~GameRenderer(){
    _joystick = nullptr;
}


void GameRenderer::render(const std::shared_ptr<SpriteBatch> &batch){
    _joystick->draw(batch);
}

