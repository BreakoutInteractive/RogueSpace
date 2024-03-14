//
//  GameRenderer.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/8/24.
//

#include "GameRenderer.hpp"
#include "JoyStick.hpp"
#include "../models/LevelModel.hpp"
#include "../models/Player.hpp"

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
    Size dimen = Application::get()->getDisplaySize();
    dimen *= 720/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    _assets = assets;
    _backgroundTexture = assets->get<Texture>("background");
    _joystick->loadAssets(assets);
    
    // acquire the HUD nodes
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("HUD");
    _paused=false;

    _pauseButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("HUD_pause"));
    _pauseButton->addListener([this](const std::string name, bool down){
        if (down){
            _paused=true;
            _pauseButton->setVisible(false);
        }
    });
    
    _joystickRing = _assets->get<scene2::SceneNode>("HUD_js_ring");
    _joystickButton = _assets->get<scene2::SceneNode>("HUD_js_button");
    
    _hpBar = std::dynamic_pointer_cast<scene2::ProgressBar>(_assets->get<scene2::SceneNode>("HUD_status_hp"));
    
    // readjust scene to screen
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD
    
    // activate UI
    _pauseButton->activate();
    _joystickRing->setVisible(false);
    _joystickButton->setVisible(false);
    
    addChild(scene);
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


void GameRenderer::configurePauseButton(bool pause) {
    if (pause) {
        _pauseButton->deactivate();
    } else{
        _paused = pause;
        _pauseButton->setDown(false);
        _pauseButton->setVisible(!pause);
        _pauseButton->activate();
    }
}

void GameRenderer::render(const std::shared_ptr<SpriteBatch> &batch){
    
    auto player = _level->getPlayer();
    _hpBar->setProgress(player->_hp / (float) player->getMaxHP());
    
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

