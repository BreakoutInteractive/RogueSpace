//
//  GameRenderer.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/8/24.
//

#include "GameRenderer.hpp"
#include "JoyStick.hpp"
#include "../models/LevelModel.hpp"

GameRenderer::GameRenderer(){
    Scene2();
    /** Create joystick */
    _joystick = std::make_shared<JoyStick>(Vec2::ZERO, Vec2::ZERO);
}

GameRenderer::~GameRenderer(){
    _joystick = nullptr;
    _level = nullptr;
}

bool GameRenderer::init(const std::shared_ptr<AssetManager>& assets){
    _backgroundTexture = assets->get<Texture>("background");
    // TODO: use assets to load scene nodes and add them to this scene
    _joystick->loadAssets(assets);
    return true;
}

void GameRenderer::setDrawScale(Vec2 scale){
    _joystick->setDrawScale(scale.set(0.75, 0.75));
}

void GameRenderer::setJoystickPosition(Vec2 anchorPos, Vec2 screenPos){
    if (!_joystick->getActive()) {
        _joystick->updateBasePos(screenToWorldCoords(anchorPos));
    }
    else {
        _joystick->updateBallPos(screenToWorldCoords(screenPos));
    }
}


void GameRenderer::render(const std::shared_ptr<SpriteBatch> &batch){
    // using game camera, render the game
    if (_gameCam != nullptr){
        batch->begin(_gameCam->getCombined());
        Size s = Application::get()->getDisplaySize();
        Vec3 camPos = _gameCam->getPosition();
        batch->draw(_backgroundTexture, Rect(camPos.x - s.width/2, camPos.y - s.height/2, s.width, s.height));
        if (_level != nullptr){
            _level->render(batch);
        }
        batch->end();
    }
    
    // render HUD
    batch->begin(getCamera()->getCombined());
    _joystick->draw(batch);
    batch->end();
    Scene2::render(batch);  // call base method to render scene nodes
}

