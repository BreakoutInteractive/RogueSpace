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
    _moveHoldCounter = 0;
    _aimHoldCounter = 0;
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

    _pauseButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("HUD_pause"));
    _pauseButton->addListener([this](const std::string name, bool down){
        if (down){
            _paused=true;
            hideJoysticks();
        }
    });
    
    _joystickRing = _assets->get<scene2::SceneNode>("HUD_js-ring");
    _joystickMoveButton = _assets->get<scene2::SceneNode>("HUD_js-button");
    _joystickAimRing =  _assets->get<scene2::SceneNode>("HUD_aim-ring");;
    _joystickAimButton = _assets->get<scene2::SceneNode>("HUD_aim-button");
    
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
    hideJoysticks();
    
    addChild(scene);
    return true;
}

void GameRenderer::hideJoysticks(){
    _joystickRing->setVisible(false);
    _joystickMoveButton->setVisible(false);
    _joystickAimRing->setVisible(false);
    _joystickAimButton->setVisible(false);
}

void GameRenderer::updateMoveJoystick(bool touched, Vec2 anchorScreenPos, Vec2 screenPos){
    if (touched){
        if (!_joystickRing->isVisible()){
            if (anchorScreenPos != screenPos){
                _joystickRing->setPosition(screenToWorldCoords(anchorScreenPos));
                _joystickRing->setVisible(true);
            }
        }
        setJoystickPosition(_joystickRing, _joystickMoveButton, screenPos);
        _moveHoldCounter = std::min(_moveHoldCounter + 1, HOLD_TIME);
        Color4 tint(Vec4(1,1,1, _moveHoldCounter/(float)HOLD_TIME));
        _joystickRing->setColor(tint);
        _joystickMoveButton->setColor(tint);
    }
    else {
        _joystickRing->setVisible(false);
        _joystickMoveButton->setVisible(false);
        _moveHoldCounter = 0;
    }
}

void GameRenderer::updateAimJoystick(bool touched, Vec2 anchorScreenPos, Vec2 screenPos){
    if (touched){
        if (!_joystickAimRing->isVisible()){
            if (anchorScreenPos != screenPos){
                _joystickAimRing->setPosition(screenToWorldCoords(anchorScreenPos));
                _joystickAimRing->setVisible(true);
            }
        }
        setJoystickPosition(_joystickAimRing, _joystickAimButton, screenPos);
        _aimHoldCounter = std::min(_aimHoldCounter + 1, HOLD_TIME);
        Color4 tint(Vec4(1,1,1, _aimHoldCounter/(float)HOLD_TIME));
        _joystickAimRing->setColor(tint);
        _joystickAimButton->setColor(tint);
    }
    else {
        _joystickAimRing->setVisible(false);
        _joystickAimButton->setVisible(false);
        _aimHoldCounter = 0;
    }
}

void GameRenderer::setJoystickPosition(std::shared_ptr<scene2::SceneNode> ring, std::shared_ptr<scene2::SceneNode> button, Vec2 screenPos){
    Vec2 buttonPosition = screenToWorldCoords(screenPos);
    float radius = ring->getSize().width/2 - button->getSize().width/2;
    if (buttonPosition.distance(ring->getPosition()) < radius){
        button->setPosition(buttonPosition);
    }else{
        cugl::Vec2 touch2base(buttonPosition - ring->getPosition());
        float angle = atan2(touch2base.y, touch2base.x);
        float xDist = sin(angle-1.5708)*radius;
        float yDist = cos(angle-1.5708)*radius;
        Vec2 basePos = ring->getPosition();
        button->setPosition(Vec2(basePos.x-xDist, basePos.y+yDist));
    }
    if (ring->isVisible()){
        button->setVisible(true);
    }
}

void GameRenderer::setActivated(bool value) {
    if (value) {
        _pauseButton->setDown(false);
        _pauseButton->setVisible(true);
        _pauseButton->activate();
        _paused = false;
    } else{
        _pauseButton->setVisible(false);
        _pauseButton->deactivate();
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

