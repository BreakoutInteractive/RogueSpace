//
//  TutorialScene.cpp
//  RS
//
//  Created by Dasha Griffiths on 5/12/24.
//

#include "TutorialScene.hpp"
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

using namespace cugl;
using namespace std;

bool TutorialScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }
    // Initialize the scene to a locked height
    _scene = assets->get<scene2::SceneNode>("tutorial");
    auto height = _scene->getSize().height;
    Size dimen = Application::get()->getDisplaySize();
    dimen *= height/dimen.height;
    if (!Scene2::init(dimen)) {
        return false;
    }
    _backgroundTexture = assets->get<Texture>("title_background");
    _backgroundScale = std::max(dimen.width / _backgroundTexture->getWidth(), dimen.height/_backgroundTexture->getHeight());
    
    // gather buttons
    _back = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_title_selection_new"));
    _level1 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_title_selection_continue"));
    _level2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_title_selection_tutorial"));
    _settings = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_setting"));

    // attach listeners
    scene2::Button::Listener backListener = [this](std::string name, bool down){ _choice = BACK; AudioController::playUiFX("menuClick");};
    auto settingsListener = [this](std::string name, bool down){ _choice = SETTINGS; AudioController::playUiFX("menuClick");};
    auto level1Listener = [this](std::string name, bool down){ _choice = LEVEL; _selectedLevel = 1; AudioController::playUiFX("menuClick");};
    auto level2Listener = [this](std::string name, bool down){ _choice = LEVEL; _selectedLevel = 2; AudioController::playUiFX("menuClick");};
    
    _back->addListener(backListener);
    _settings->addListener(settingsListener);
    _level2->addListener(level2Listener);
    _level1->addListener(level1Listener);
    
    // resize and setup the scene
    _scene->setContentSize(dimen);
    _scene->doLayout();
    _scene->setVisible(false);
    addChild(_scene);
    setActive(false);
    _choice = NONE;
    return true;
}

void TutorialScene::dispose(){
    removeAllChildren();
    setActive(false);
}

void TutorialScene::activateScene(bool value){
    if (value){
        _scene->setVisible(true);
        _level1->activate();
        _back->activate();
        _settings->activate();
        _level2->activate();
    }
    else {
        _scene->setVisible(false);
        _level1->deactivate();
        _back->deactivate();
        _settings->deactivate();
        _level2->deactivate();
        _level1->setDown(false);
        _back->setDown(false);
        _settings->setDown(false);
        _level2->setDown(false);
    }
        
}

void TutorialScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        activateScene(value);
        if (value) {
            _choice = NONE;
        }
    }
}

void TutorialScene::render(const std::shared_ptr<SpriteBatch> &batch){
    batch->begin(getCamera()->getCombined());
    Affine2 transform = Affine2::createScale(_backgroundScale);
    transform.translate(getCamera()->getPosition());
    batch->draw(_backgroundTexture, _backgroundTexture->getSize()/2, transform);
    batch->end();
    Scene2::render(batch);
}
