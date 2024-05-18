//
//  WinScene.cpp
//  RS
//
//  Created by Dasha Griffiths on 5/17/24.
//

#include "WinScene.hpp"
#include <cugl/cugl.h>

using namespace cugl;

bool WinScene::init(const std::shared_ptr<cugl::AssetManager> &assets){
    if (assets == nullptr) {
        return false;
    }
    _assets = assets;
    //  Acquire the scene built by the asset loader and Initialize the scene to a locked height
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("win");
    float height = scene->getContentHeight();
    Size dimen = Application::get()->getDisplaySize();
    dimen *= height/dimen.height;
    if (!Scene2::init(dimen)) {
        return false;
    }
    // resize the scene
    scene->setContentSize(dimen);
    scene->doLayout();
    _backgroundTexture = assets->get<Texture>("win_background");
    _backgroundScale = std::max(dimen.width / _backgroundTexture->getWidth(), dimen.height/_backgroundTexture->getHeight());

    // get scene nodes
    _restart = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("win_dead_selection_try_again"));
    _main = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("win_dead_selection_main_menu"));
    // program the buttons
    _restart->addListener([this](const std::string& name, bool down) {
        if (down) { _choice = Choice::RESTART; AudioController::playUiFX("menuClick");}
    });
    _main->addListener([this](const std::string& name, bool down) {
        if (down) { _choice = Choice::MAIN_MENU; AudioController::playUiFX("menuClick");}
    });
    
    addChild(scene);
    setActive(false);
    _choice = Choice::NONE;
    return true;
}


void WinScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
}

void WinScene::setActive(bool value){
    if (isActive() != value){
        _choice = NONE;
        Scene2::setActive(value);
        if (value){
            AudioController::updateMusic("win", 1);
            _main->activate();
            _restart->activate();
        }
        else {
            _main->deactivate();
            _restart->deactivate();
            _main->setDown(false);
            _restart->setDown(false);
        }
        _main->setVisible(value);
        _restart->setVisible(value);
    }
}

void WinScene::render(const std::shared_ptr<SpriteBatch> &batch){
    batch->begin(getCamera()->getCombined());
    Affine2 transform = Affine2::createScale(_backgroundScale);
    transform.translate(getCamera()->getPosition());
    batch->draw(_backgroundTexture, _backgroundTexture->getSize()/2, transform);
    batch->end();
    Scene2::render(batch);
}
