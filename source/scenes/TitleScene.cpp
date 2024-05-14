//
//  TitleScene.cpp
//  RS
//
//  Created by Zhiyuan Chen on 5/1/24.
//

#include "TitleScene.hpp"
#include <cugl/cugl.h>

bool TitleScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }
    // Initialize the scene to a locked height
    _scene1 = assets->get<scene2::SceneNode>("menu1");
    _scene2 = assets->get<scene2::SceneNode>("menu2");
    auto height = _scene1->getSize().height;
    Size dimen = Application::get()->getDisplaySize();
    dimen *= height/dimen.height;
    if (!Scene2::init(dimen)) {
        return false;
    }
    _backgroundTexture = assets->get<Texture>("title_background");
    _backgroundScale = std::max(dimen.width / _backgroundTexture->getWidth(), dimen.height/_backgroundTexture->getHeight());
    
    // gather buttons
    _newGame = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu1_title_selection_new"));
    _tutorial = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu1_title_selection_tutorial"));
    _settings = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu1_setting"));
    
    _continue = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu2_title_selection_continue"));
    _newGame2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu2_title_selection_new_game"));
    _tutorial2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu2_title_selection_tutorial"));
    _settings2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu2_setting"));

    // attach listeners
    scene2::Button::Listener newGameListener = [this](std::string name, bool down){ _choice = NEW;};
    auto tutorialListener = [this](std::string name, bool down){ _choice = TUTORIAL;};
    auto settingsListener = [this](std::string name, bool down){ _choice = SETTINGS;};
    auto continueListener = [this](std::string name, bool down){ _choice = CONTINUE;};
    _newGame->addListener(newGameListener);
    _newGame2->addListener(newGameListener);
    _settings->addListener(settingsListener);
    _settings2->addListener(settingsListener);
    _tutorial->addListener(tutorialListener);
    _tutorial2->addListener(tutorialListener);
    _continue->addListener(continueListener);
    
    // resize and setup the scene
    _scene1->setContentSize(dimen);
    _scene1->doLayout();
    _scene1->setVisible(false);
    _scene2->setContentSize(dimen);
    _scene2->doLayout();
    _scene2->setVisible(false);
    addChild(_scene1);
    addChild(_scene2);
    setActive(false);
    _choice = NONE;
    _type = WITHOUT_CONTINUE;
    return true;
}

void TitleScene::dispose(){
    removeAllChildren();
    setActive(false);
}

void TitleScene::activateScene(bool value){
    if (value){
        if (_type == WITHOUT_CONTINUE){
            _scene1->setVisible(true);
            _newGame->activate();
            _settings->activate();
            _tutorial->activate();
        }
        else if (_type == WITH_CONTINUE){
            _scene2->setVisible(true);
            _continue->activate();
            _newGame2->activate();
            _settings2->activate();
            _tutorial2->activate();
        }
    }
    else {
        if (_type == WITHOUT_CONTINUE){
            _scene1->setVisible(false);
            _newGame->deactivate();
            _settings->deactivate();
            _tutorial->deactivate();
            _newGame->setDown(false);
            _settings->setDown(false);
            _tutorial->setDown(false);
            
        }
        else if (_type == WITH_CONTINUE){
            _scene2->setVisible(false);
            _continue->deactivate();
            _newGame2->deactivate();
            _settings2->deactivate();
            _tutorial2->deactivate();
            _continue->setDown(false);
            _newGame2->setDown(false);
            _settings2->setDown(false);
            _tutorial2->setDown(false);
        }
    }
        
}

void TitleScene::setSceneType(SceneType type){
    _type = type;
}

void TitleScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        activateScene(value);
        if (value) {
            _choice = NONE;
        }
    }
}

void TitleScene::render(const std::shared_ptr<SpriteBatch> &batch){
    batch->begin(getCamera()->getCombined());
    Affine2 transform = Affine2::createScale(_backgroundScale);
    transform.translate(getCamera()->getPosition());
    batch->draw(_backgroundTexture, _backgroundTexture->getSize()/2, transform);
    batch->end();
    Scene2::render(batch);
}
