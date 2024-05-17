//
//  DeathScene.cpp
//  RS
//
//  Created by Zhiyuan Chen on 5/11/24.
//

#include "DeathScene.hpp"
#include <cugl/cugl.h>

using namespace cugl;

bool DeathScene::init(const std::shared_ptr<cugl::AssetManager> &assets){
    if (assets == nullptr) {
        return false;
    }
    _assets = assets;
    //  Acquire the scene built by the asset loader and Initialize the scene to a locked height
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("death");
    float height = scene->getContentHeight();
    Size dimen = Application::get()->getDisplaySize();
    dimen *= height/dimen.height;
    if (!Scene2::init(dimen)) {
        return false;
    }
    // resize the scene
    scene->setContentSize(dimen);
    scene->doLayout();
    // get scene nodes
    _cape = std::dynamic_pointer_cast<scene2::SpriteNode>(_assets->get<scene2::SceneNode>("death_dead_animation"));
    _capeAnimation = scene2::Animate::alloc(0, _cape->getSpan()-1, 2.0f);
    _restart = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("death_dead_selection_try_again"));
    _main = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("death_dead_selection_main_menu"));
    // program the buttons
    _restart->addListener([this](const std::string& name, bool down) {
        if (down) { _choice = Choice::RESTART; AudioController::playUiFX("menuClick");}
    });
    _main->addListener([this](const std::string& name, bool down) {
        if (down) { _choice = Choice::MAIN_MENU; AudioController::playUiFX("menuClick");}
    });
    
    // add an overlay layer to separate game background from UI
    auto overlay = scene2::PolygonNode::allocWithPoly(Rect(0, 0, dimen.width, dimen.height));
    overlay->setColor(Color4(0, 0, 0, 128));
    addChild(overlay);
    addChild(scene);
    setActive(false);
    _choice = Choice::NONE;
    return true;
}


void DeathScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
}

void DeathScene::update(float dt){
    _actionManager.update(dt);
}


void DeathScene::setActive(bool value){
    if (isActive() != value){
        _choice = NONE;
        Scene2::setActive(value);
        if (value){
            AudioController::updateMusic("lose", 1);
            _main->activate();
            _restart->activate();
            _cape->setFrame(0);
            _actionManager.activate(CAPE_ANIM_KEY, _capeAnimation, _cape);
        }
        else {
            _main->deactivate();
            _restart->deactivate();
            _main->setDown(false);
            _restart->setDown(false);
            _actionManager.remove(CAPE_ANIM_KEY);
        }
        _main->setVisible(value);
        _restart->setVisible(value);
    }
}
