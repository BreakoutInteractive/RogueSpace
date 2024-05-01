//
//  PauseScene.cpp
//  RS
//

#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

#include "PauseScene.hpp"

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
bool PauseScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
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
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("pausemenu");
    scene->setContentSize(dimen);
    scene->doLayout();

    _choice = Choice::NONE;
    _restart = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_pausemenu_menu_restart"));
    _resume = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_pausemenu_menu_resume"));
    _settings = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_pausemenu_menu_setting"));
    
    _sword = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_sword"));
    _bow = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_bow"));
    _parry = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_parry"));
    _dash = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_dash"));
    _shield = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_shield"));
    _speed = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pausemenu_speed"));
    
    _sword->setVisible(false);
    _bow->setVisible(false);
    _parry->setVisible(false);
    _dash->setVisible(false);
    _shield->setVisible(false);
    _speed->setVisible(false);

    // Program the buttons
    _restart->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::RESTART;
            CULog("resetting (pause screen)");
        }
    });
    _resume->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::RESUME;
            CULog("resuming (pause screen)");
        }
    });
    _settings->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::SETTINGS;
        }
    });

    addChild(scene);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void PauseScene::dispose() {
    if (_active) {
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
void PauseScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);

        if (value) {
            _choice = NONE;
            _restart->activate();
            _resume->activate();
            _settings->activate();
        } else {
            _restart->deactivate();
            _resume->deactivate();
            _settings->deactivate();
            // If any were pressed, reset them
            _restart->setDown(false);
            _resume->setDown(false);
            _settings->setDown(false);
        }
    }
}
