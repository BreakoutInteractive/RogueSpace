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
    std::shared_ptr<scene2::SceneNode> scene = assets->get<scene2::SceneNode>("title_scene");
    auto height = scene->getSize().height;
    Size dimen = Application::get()->getDisplaySize();
    dimen *= height/dimen.height;
    if (!Scene2::init(dimen)) {
        return false;
    }
    _backgroundTexture = assets->get<Texture>("title_background");
    _backgroundScale = std::max(dimen.width / _backgroundTexture->getWidth(), dimen.height/_backgroundTexture->getHeight());
    
    _continue = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("title_scene_title_selection_continue"));
    _newGame = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("title_scene_title_selection_new"));
    _settings = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("title_scene_title_selection_setting"));
    _continue->setColor(Color4::GRAY);
    _settings->setColor(Color4::GRAY);
    // TODO: add listeners to other buttons once finalized layout
    _newGame->addListener([this](std::string name, bool down){
        _choice = NEW;
    });
    // resize and setup the scene
    scene->setContentSize(dimen);
    scene->doLayout();
    addChild(scene);
    setActive(false);
    _choice = NONE;
    return true;
}

void TitleScene::dispose(){
    removeAllChildren();
    setActive(false);
}

void TitleScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _choice = NONE;
            _continue->activate();
            _newGame->activate();
            _settings->activate();
        } else {
            _newGame->deactivate();
            _continue->deactivate();
            _settings->deactivate();
            _continue->setDown(false);
            _newGame->setDown(false);
            _settings->setDown(false);
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
