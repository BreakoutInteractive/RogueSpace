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
    _scene1 = assets->get<scene2::SceneNode>("menu1");
    // Initialize the scene to a locked height
    auto height = _scene1->getSize().height;
    Size dimen = Application::get()->getDisplaySize();
    float aspect_ratio = dimen.width / dimen.height;
    dimen *= height/dimen.height;
    if (!Scene2::init(dimen)) {
        return false;
    }
    
    std::string layoutName = aspect_ratio >= 1.5 ? "menu2" : "menu2_tablet";
    _scene2 = assets->get<scene2::SceneNode>(layoutName);
    std::shared_ptr<scene2::SceneNode> confirmationNode = assets->get<scene2::SceneNode>("confirmation");
    

    _backgroundTexture = assets->get<Texture>("title_background");
    _backgroundScale = std::max(dimen.width / _backgroundTexture->getWidth(), dimen.height/_backgroundTexture->getHeight());
    
    _confirmationScene.init(dimen);
    auto overlay = scene2::PolygonNode::allocWithPoly(Rect(0, 0, dimen.width, dimen.height));
    overlay->setColor(Color4(0, 0, 0, 200));
    _confirmationScene.addChild(overlay);
    _confirmationScene.addChild(confirmationNode);

    // auto-resize text content
    auto confrimLabel = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("confirmation_confirmation_confirm_title"));
    confrimLabel->setText("Start New Game", true);
    auto progressLabel = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("confirmation_confirmation_progress"));
    progressLabel->setText("Past progress will be erased!", true);
    confirmationNode->setContentSize(dimen);
    confirmationNode->doLayout();

    // gather buttons
    _newGame = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu1_title_selection_new"));
    _tutorial = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu1_title_selection_tutorial"));
    _settings = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("menu1_setting"));
    
    // pick between mobile and tablet layouts, threshold at 1.47 (rounded to 1.5)
    _continue = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>(layoutName + "_title_selection_continue"));
    _newGame2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>(layoutName + "_title_selection_new_game"));
    _tutorial2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>(layoutName + "_title_selection_tutorial"));
    _settings2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>(layoutName + "_setting"));
    _back2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("confirmation_confirmation_back"));
    _confirm2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("confirmation_confirmation_confirm"));

    // attach listeners

    scene2::Button::Listener newGameListener = [this](std::string name, bool down){ _choice = NEW; AudioController::playUiFX("menuClick");};
    scene2::Button::Listener newGame2Listener = [this](std::string name, bool down){ _confirmationScene.setActive(true); _newGame2->setDown(false); AudioController::playUiFX("menuClick");};
    auto tutorialListener = [this](std::string name, bool down){ _choice = TUTORIAL; AudioController::playUiFX("menuClick"); };
    auto settingsListener = [this](std::string name, bool down){ _choice = SETTINGS; AudioController::playUiFX("menuClick"); };
    auto continueListener = [this](std::string name, bool down){ _choice = CONTINUE; AudioController::playUiFX("menuClick"); };
    auto backListener = [this](std::string name, bool down){ _confirmationScene.setActive(false); _back2->setDown(false);};
    
    _newGame->addListener(newGameListener);
    _newGame2->addListener(newGame2Listener);
    _confirm2->addListener(newGameListener);
    _back2->addListener(backListener);
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
    _confirmationScene.setActive(false);
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
            _confirmationScene.setActive(false);
            
            _continue->deactivate();
            _newGame2->deactivate();
            _settings2->deactivate();
            _tutorial2->deactivate();
            _confirm2->deactivate();
            _back2->deactivate();
            
            _continue->setDown(false);
            _newGame2->setDown(false);
            _settings2->setDown(false);
            _tutorial2->setDown(false);
            _confirm2->setDown(false);
            _back2->setDown(false);
        }
    }
}

void TitleScene::activateConfirmButtons(bool active){
    if (active && _type == WITH_CONTINUE){
        _continue->deactivate();
        _newGame2->deactivate();
        _tutorial2->deactivate();
        _settings2->deactivate();
        _continue->setDown(false);
        _newGame2->setDown(false);
        _settings2->setDown(false);
        _tutorial2->setDown(false);

        _back2->activate();
        _confirm2->activate();
    } else if (_type == WITH_CONTINUE){
        _confirm2->deactivate();
        _back2->deactivate();
        _confirmationScene.setActive(false);

        _newGame2->activate();
        _continue->activate();
        _tutorial2->activate();
        _settings2->activate();
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
    if (_confirmationScene.isActive()){
        _confirmationScene.render(batch);
    }
}
