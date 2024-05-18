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
    _selectedLevel = 1;
    
    // gather buttons
    _back = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_tutorial_top_bar_back"));
    _play = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_play_tutorial"));
    _levelLabel = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("tutorial_tutorial_display_tutorial_title"));
    _levelDescrip = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("tutorial_tutorial_display_tutorial_description"));
    _screenshot = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("tutorial_tutorial_display_screenshot"));
    
    _level1 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_tutorial_left_tutorial_selection_dash"));
    _screenshotDash = assets->get<Texture>("screenshotDash");
    _level2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_tutorial_left_tutorial_selection_melee"));
    _screenshotMelee = assets->get<Texture>("screenshotMelee");
    _level3 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_tutorial_left_tutorial_selection_range"));
    _screenshotRange = assets->get<Texture>("screenshotRange");
    _level4 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("tutorial_tutorial_left_tutorial_selection_parry"));
    _screenshotParry = assets->get<Texture>("screenshotParry");
    
    // attach listeners
    auto backListener = [this](std::string name, bool down){ _choice = BACK; _selectedLevel=1; AudioController::playUiFX("menuClick");};
    //if user exits tutorial screen, reset to first option
    auto playListener = [this](std::string name, bool down){ _choice = LEVEL; AudioController::playUiFX("menuClick");};
    //if they play a level, same level should be selected
    auto level1Listener = [this](std::string name, bool down){
        if (down) {
            if (_selectedLevel!=1) {
                AudioController::playUiFX("menuClick");
                _selectedLevel = 1;
            }
            
            setScreenText();
            _level1->setDown(true);
            _level2->setDown(false);
            _level3->setDown(false);
            _level4->setDown(false);
        } else if (_selectedLevel==1){
            _level1->setDown(true);
        }
    };
    auto level2Listener = [this](std::string name, bool down){
        if (down) {
            if (_selectedLevel!=2) {
                AudioController::playUiFX("menuClick");
                _selectedLevel = 2;
            }
            
            setScreenText();
            _level1->setDown(false);
            _level2->setDown(true);
            _level3->setDown(false);
            _level4->setDown(false);
        } else if (_selectedLevel==2){
            _level2->setDown(true);
        }
    };
    auto level3Listener = [this](std::string name, bool down){
        if (down) {
            if (_selectedLevel!=3) {
                AudioController::playUiFX("menuClick");
                _selectedLevel = 3;
            }

            setScreenText();
            _level1->setDown(false);
            _level2->setDown(false);
            _level3->setDown(true);
            _level4->setDown(false);
        } else if (_selectedLevel==3){
            _level3->setDown(true);
        }
    };
    auto level4Listener = [this](std::string name, bool down){
        if (down) {
            if (_selectedLevel!=4) {
                AudioController::playUiFX("menuClick");
                _selectedLevel = 4;
            }

            setScreenText();
            _level1->setDown(false);
            _level2->setDown(false);
            _level3->setDown(false);
            _level4->setDown(true);
        } else if (_selectedLevel==4){
            _level4->setDown(true);
        }
    };
    
    _back->addListener(backListener);
    _play->addListener(playListener);
    _level1->addListener(level1Listener);
    _level2->addListener(level2Listener);
    _level3->addListener(level3Listener);
    _level4->addListener(level4Listener);
    
    auto overlay = scene2::PolygonNode::allocWithPoly(Rect(0, 0, dimen.width, dimen.height));
    overlay->setColor(Color4(128, 128, 128, 200));
    addChild(overlay);
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

void TutorialScene::setScreenText(){
    int levelType = _selectedLevel-1;
    std::string tutName;
    std::string tutDescription;
    std::shared_ptr<Texture> levelSS;

    switch (levelType) {
        case DASH:
            tutName = "DASH";
            tutDescription = "Dash to avoid damage and phase through enemies.";
            levelSS = _screenshotDash;
            _level1->setDown(true);
            break;
        case MELEE:
            tutName = "MELEE";
            tutDescription = "Attack enemies up close to deplete their health.";
            levelSS = _screenshotMelee;
            _level2->setDown(true);
            break;
        case RANGE:
            tutName = "RANGE";
            tutDescription = "Equip the bow. Charge and fire a bolt of energy \nat distant foes.";
            levelSS = _screenshotRange;
            _level3->setDown(true);
            break;
        case PARRY:
            tutName = "PARRY";
            tutDescription = "Hold the blocking stance. Let go before being \nhit to stun melee enemies.";
            levelSS = _screenshotParry;
            _level4->setDown(true);
            break;
    }
    _levelLabel->setText(tutName);
    _levelDescrip->setText(tutDescription, true);
    _screenshot->setTexture(levelSS);

}

void TutorialScene::activateScene(bool value){
    if (value){
        _scene->setVisible(true);
        _back->activate();
        _play->activate();

        _level1->activate();
        _level2->activate();
        _level3->activate();
        _level4->activate();
        setScreenText();
    }
    else {
        _scene->setVisible(false);
        _back->deactivate();
        _play->deactivate();
        _level1->deactivate();
        _level2->deactivate();
        _level3->deactivate();
        _level4->deactivate();

        _back->setDown(false);
        _play->setDown(false);
        _level1->setDown(false);
        _level2->setDown(false);
        _level3->setDown(false);
        _level4->setDown(false);
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
