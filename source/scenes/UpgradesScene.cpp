//
//  UpgradesScene.cpp
//  RS
//
//  Created by Dasha Griffiths on 4/13/24.
//

#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

#include "UpgradesScene.hpp"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

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
bool UpgradesScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
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
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("upgrades");
    scene->setContentSize(dimen);
    scene->doLayout();

    _choice = Choice::NONE;
    _option1 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item"));
    _option2 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1"));

    
    _confirm1 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_confirm"));
    _confirm2 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_confirm-1"));
    _confirm1->setVisible(false);
    _confirm2->setVisible(false);
    
    // Program the buttons
    _confirm1->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::UPGRADE_1;
            
        }
    });
    _confirm2->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::UPGRADE_2;
        }
    });
    
    
    _option1->addListener([this](const std::string& name, bool down) {
        if (down) {
            _option2->setDown(false);
            _confirm2->setVisible(false);
            _confirm2->deactivate();
            _confirm2->setDown(false);

            _option1->setToggle(true);
            _confirm1->setVisible(true);
            _confirm1->activate();
            CULog("player choice opt 1");
        } else{
            _confirm1->setVisible(false);
            _confirm1->deactivate();
        }
    });
    _option2->addListener([this](const std::string& name, bool down) {
        if (down) {
            _option1->setDown(false);
            _confirm1->setVisible(false);
            _confirm1->deactivate();
            _confirm1->setDown(false);

            _option2->setToggle(true);
            _confirm2->setVisible(true);
            _confirm2->activate();
            CULog("player choice opt 2");
        } else{
            _confirm2->setVisible(false);
            _confirm2->deactivate();
        }
    });

    addChild(scene);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void UpgradesScene::dispose() {
    if (_active) {
        _option1 = nullptr;
        _option2 = nullptr;
        _confirm1 = nullptr;
        _confirm2 = nullptr;
        removeAllChildren();
        _active = false;
    }
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
void UpgradesScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);

        if (value) {
            _choice = NONE;
            _option1->activate();
            _option2->activate();
        } else {
            _option1->deactivate();
            _option2->deactivate();
            // If any were pressed, reset them
            _option1->setDown(false);
            _option2->setDown(false);
        }
    }
}


