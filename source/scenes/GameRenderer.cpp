//
//  GameRenderer.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/8/24.
//

#include "GameRenderer.hpp"
#include "../models/LevelModel.hpp"
#include "../models/Player.hpp"

#define HOLD_TIME 25

GameRenderer::GameRenderer(){
    Scene2();
    _holdCounter = 0;
}

GameRenderer::~GameRenderer(){
    _level = nullptr;
    _gameCam = nullptr;
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

    
    // acquire the HUD nodes
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("HUD");
    _paused=false;
    _cust=false;

    _pauseButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("HUD_pause"));
    _pauseButton->addListener([this](const std::string name, bool down){
        if (down){
            _paused=true;
            hideJoysticks();
        }
    });
    _custButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("HUD_settings"));
    _custButton->addListener([this](const std::string name, bool down){
        if (down){
            _cust=true;
            hideJoysticks();
        }
    });
    
    
    _joystickRing = _assets->get<scene2::SceneNode>("HUD_js-ring");
    _joystickMoveButton = _assets->get<scene2::SceneNode>("HUD_js-button");
    _joystickAimButton = _assets->get<scene2::SceneNode>("HUD_aim-button");
    _activeJoystick = _joystickMoveButton;
    
    int count = 2; // possibility that this is retrievable from scene graph?
        for (int i = 1; i <= count; i++){
            _stamina.push_back(_assets->get<scene2::SceneNode>("HUD_status_cooldown-" + std::to_string(i)));
        }
        
        // testing disable
        for (auto it = _stamina.begin(); it != _stamina.end(); it++){
            (*it)->setVisible(true);
        }
    
    _hpBar = std::dynamic_pointer_cast<scene2::ProgressBar>(_assets->get<scene2::SceneNode>("HUD_status_hp"));
    
    // readjust scene to screen
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD
    
    // activate UI
    _pauseButton->activate();
    _custButton->activate();
    hideJoysticks();
    
    addChild(scene);
    return true;
}

void GameRenderer::hideJoysticks(){
    _joystickRing->setVisible(false);
    _joystickMoveButton->setVisible(false);
    _joystickAimButton->setVisible(false);
}

void GameRenderer::setJoystickMode(){
    _activeJoystick = _joystickMoveButton;
    _joystickAimButton->setVisible(false);
}

void GameRenderer::setAimJoystickMode(){
    _activeJoystick = _joystickAimButton;
    _joystickMoveButton->setVisible(false);
}

void GameRenderer::updateJoystick(bool touched, Vec2 anchorScreenPos, Vec2 screenPos){
    if (touched){
        if (!_joystickRing->isVisible()){
            if (anchorScreenPos != screenPos){
                _joystickRing->setPosition(screenToWorldCoords(anchorScreenPos));
                _joystickRing->setVisible(true);
            }
        }
        setJoystickPosition(screenPos);
        _holdCounter = std::min(_holdCounter + 1, HOLD_TIME);
        Color4 tint(Vec4(1,1,1, _holdCounter/(float)HOLD_TIME));
        _joystickRing->setColor(tint);
        _activeJoystick->setColor(tint);
    }
    else {
        hideJoysticks();
        _holdCounter = 0;
    }
    
}

void GameRenderer::setJoystickPosition(Vec2 screenPos){
    Vec2 buttonPosition = screenToWorldCoords(screenPos);
    float radius = _joystickRing->getSize().width/2 - _activeJoystick->getSize().width/2;
    if (buttonPosition.distance(_joystickRing->getPosition()) < radius){
        _activeJoystick->setPosition(buttonPosition);
    }else{
        cugl::Vec2 touch2base(buttonPosition - _joystickRing->getPosition());
        float angle = atan2(touch2base.y, touch2base.x);
        float xDist = sin(angle-1.5708)*radius;
        float yDist = cos(angle-1.5708)*radius;
        Vec2 basePos = _joystickRing->getPosition();
        _activeJoystick->setPosition(Vec2(basePos.x-xDist, basePos.y+yDist));
    }
    if (_joystickRing->isVisible()){
        _activeJoystick->setVisible(true);
    }
}

void GameRenderer::setActivated(bool value) {
    if (value) {
        _pauseButton->setDown(false);
        _pauseButton->setVisible(true);
        _pauseButton->activate();
        _paused = false;
        
        _custButton->setDown(false);
        _custButton->setVisible(true);
        _custButton->activate();
        _cust = false;
    } else{
        _pauseButton->setVisible(false);
        _pauseButton->deactivate();
        
        _custButton->setVisible(false);
        _custButton->deactivate();
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
    
    Scene2::render(batch);  // call base method to render scene nodes
}

