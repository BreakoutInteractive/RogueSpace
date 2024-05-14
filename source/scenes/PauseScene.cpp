//
//  PauseScene.cpp
//  RS
//

#include <cugl/cugl.h>
#include "PauseScene.hpp"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Constructors

bool PauseScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Start up the input handler
    _assets = assets;
    if (assets == nullptr) {
        return false;
    }
    // Acquire the scene built by the asset loader
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("pause");
    _confirmationScene = _assets->get<scene2::SceneNode>("confirmationMenu");

    // Initialize the scene to a locked height
    Size dimen = Application::get()->getDisplaySize();
    dimen *= scene->getContentSize().height/dimen.height;
    if (!Scene2::init(dimen)) {
        return false;
    }
    
    // resize the scene
    scene->setContentSize(dimen);
    scene->doLayout();

    // retrieve the menu buttons
    _pauseBack = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("pause_pausemenu_menu_buttons_back"));
    _resume = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("pause_pausemenu_menu_buttons_resume"));
    _settings = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("pause_pausemenu_menu_buttons_setting"));
    
    _confirmBack = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("confirmationMenu_confirmation_back"));
    _confirmConfirm = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("confirmationMenu_confirmation_confirm"));

    // Program the buttons
    _pauseBack->addListener([this](const std::string& name, bool down) {
        if (down) {
            _confirmationScene->setVisible(true);
            _pauseBack->setDown(false);
        }
    });
    _resume->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::RESUME;
        }
    });
    _settings->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::SETTINGS;
        }
    });

    _confirmBack->addListener([this](const std::string& name, bool down) {
        if (down) {
            _confirmationScene->setVisible(false);
            _confirmBack->setDown(false);
        }
    });
    _confirmConfirm->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::BACK;
        }
    });
    
    // retrieve the labels
    _atk = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("pause_pausemenu_atk_level"));
    _bow = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("pause_pausemenu_bow_level"));
    _atkSpeed = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("pause_pausemenu_atkSpeed_level"));
    _shield = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("pause_pausemenu_shield_level"));
    _dash = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("pause_pausemenu_dash_level"));
    _parry = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("pause_pausemenu_parry_level"));
    _maxHealth = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("pause_pausemenu_maxHealth_level"));
    
    addChild(scene);
    _confirmationScene->setVisible(false);
    addChild(_confirmationScene);
    setActive(false);
    return true;
}

void PauseScene::dispose() {
    setActive(false);
    removeAllChildren();
}

void PauseScene::setLabels(std::vector<int> levels){
    _atk->setText("LVL " + std::to_string(levels[0]));
    _bow->setText("LVL " + std::to_string(levels[1]));
    _atkSpeed->setText("LVL " + std::to_string(levels[2]));
    _shield->setText("LVL " + std::to_string(levels[3]));
    _dash->setText("LVL " + std::to_string(levels[4]));
    _parry->setText("LVL " + std::to_string(levels[5]));
    _maxHealth->setText("LVL " + std::to_string(levels[6]));
}

void PauseScene::activateConfirmButtons(bool active){
    if (active){
        _resume->deactivate();
        _settings->deactivate();
        _pauseBack->deactivate();
        
        _confirmConfirm->activate();
        _confirmBack->activate();
    } else{
        _confirmConfirm->deactivate();
        _confirmBack->deactivate();
        
        _resume->activate();
        _settings->activate();
        _pauseBack->activate();

    }
}

void PauseScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        _choice = NONE;
        if (value) {
            _pauseBack->activate();
            _resume->activate();
            _settings->activate();
            
        } else {
            _pauseBack->deactivate();
            _resume->deactivate();
            _settings->deactivate();
            _confirmBack->deactivate();
            _confirmConfirm->deactivate();
            
            // If any were pressed, reset them
            _pauseBack->setDown(false);
            _resume->setDown(false);
            _settings->setDown(false);
            _confirmBack->setDown(false);
            _confirmConfirm->setDown(false);
            _confirmationScene->setVisible(false);
        }
    }
}

