//
//  CustomSettingsScene.cpp
//  RS
//
//  Created by Vincent Fong on 3/21/24.
//

#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

#include "CustomSettingsScene.hpp"
#include "GameConstants.hpp"

using namespace cugl;
using namespace std;

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT  720


#pragma mark -
#pragma mark Constructors
/**
 * Initializes the controller contents, and starts the game
 *
 * In previous labs, this method "started" the scene.  But in this
 * case, we only use to initialize the scene user interface.  We
 * do not activate the user interface yet, as an active user
 * interface will still receive input EVEN WHEN IT IS HIDDEN.
 *
 * That is why we have the method {@link #setActive}.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool CustomSettingsScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Start up the input handler
    _assets = assets;
    
    // Acquire the scene built by the asset loader and resize it the scene
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("settings");
    scene->setContentSize(dimen);
    scene->doLayout();
    
    _back = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings_back"));
            
    _back->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::RESTART;
            CULog("resetting (settings screen)");
        }
    });
    
    // TODO: abstract this
    // TODO: possibly clean up the scene graph JSON
    _varMap.push_back("player-ms");
    _varMap.push_back("enemy-ms");
    _varMap.push_back("player-attack-melee-range");
    _varMap.push_back("enemy-attack-melee-range");
    _varMap.push_back("player-attack-cd");
    _varMap.push_back("enemy-attack-cd");
    _varMap.push_back("player-dodge-cd");
    _varMap.push_back("player-dodge-duration");
    _varMap.push_back("enemy-sightrange");
    _varMap.push_back("enemy-proxrange");
    
    _varMap.push_back("player-parry-cd");
    _varMap.push_back("enemy-sentry-cd");
    
    for (int i = 0; i < _varMap.size(); i++) {
        std::shared_ptr<cugl::scene2::Slider> si = std::dynamic_pointer_cast<scene2::Slider>(assets->get<scene2::SceneNode>("settings_" + _varMap[i] + "-action"));
        std::shared_ptr<cugl::scene2::Label> li  = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("settings_" + _varMap[i] + "-label"));
        float vi = si->getValue();
        _sliders.push_back(si);
        _labels.push_back(li);
        _values.push_back(vi);
        
        size_t label_length = _labels[i]->getText().find(':');
        std::string temp_label = _labels[i]->getText().substr(0, label_length + 2);
        _labels[i]->setText(temp_label + cugl::strtool::to_string(_values[i],1));
        _sliders[i]->addListener([&, i, temp_label](const std::string& name, float value) {
            if (value != _values[i]) {
                _values[i] = value;
                _labels[i]->setText(temp_label + cugl::strtool::to_string(_values[i],1));
                CustomSettingsScene::writeTo(i);
            }
        });
    }
    
    std::shared_ptr<cugl::scene2::Label> title  = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("settings_title"));
    _labels.push_back(title);
    
    addChild(scene);
    setActive(false);
    Application::get()->setClearColor(Color4f::BLACK);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void CustomSettingsScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
}

// evil necessity?
bool CustomSettingsScene::writeTo(int i) {
    std::string var = _varMap[i];
    if (var == "player-ms") {
        GameConstants::PLAYER_MOVE_SPEED = _values[i];
    } else if (var == "player-attack-melee-range") {
        GameConstants::PLAYER_MELEE_ATK_RANGE = _values[i];
    } else if (var == "player-attack-cd") {
        GameConstants::PLAYER_ATTACK_COOLDOWN = _values[i];
    } else if (var == "player-dodge-cd") {
        GameConstants::PLAYER_DODGE_COOLDOWN = _values[i];
    } else if (var == "player-dodge-duration") {
        GameConstants::PLAYER_DODGE_DURATION = _values[i];
    } else if (var == "player-parry-cd") {
        GameConstants::PLAYER_PARRY_COOLDOWN = _values[i];
    } else if (var == "enemy-ms") {
        GameConstants::ENEMY_MOVE_SPEED = _values[i];
    } else if (var == "enemy-attack-cd") {
        GameConstants::ENEMY_ATK_COOLDOWN = _values[i];
    } else if (var == "enemy-attack-melee-range") {
        GameConstants::ENEMY_MELEE_ATK_RANGE = _values[i];
    } else if (var == "enemy-sightrange") {
        GameConstants::ENEMY_SIGHT_RANGE = _values[i];
    } else if (var == "enemy-proxrange") {
        GameConstants::ENEMY_PROXIMITY_RANGE = _values[i];
    } else if (var == "enemy-sentry-cd") {
        GameConstants::ENEMY_SENTRY_COOLDOWN = _values[i];
    }
    return true;
}

/**
 * Sets whether the scene is currently active
 *
 * This method should be used to toggle all the UI elements.  Buttons
 * should be activated when it is made active and deactivated when
 * it is not.
 *
 * @param value whether the scene is currently active
 */
void CustomSettingsScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _choice = NONE;
            _back->activate();
            for (const auto& sli : _sliders) {
                sli->activate();
            }
        } else {
            _back->deactivate();
            _back->setDown(false);
            for (const auto& sli : _sliders) {
                sli->deactivate();
            }
        }
    }
}

