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
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("pause");
    
    std::shared_ptr<scene2::PolygonNode> overlay = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("pause_bg"));
    
    // remove the overlay from scene graph and make it scale to parent container size (the screen)
    overlay->removeFromParent();
    overlay->setContentSize(dimen);
    
    scene->setContentSize(dimen);
    scene->doLayout();
    _choice = Choice::NONE;
    _restart = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pause_restart"));
    _back = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pause_back"));
    
    // Program the buttons
    _restart->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::RESTART;
            CULog("resetting (pause screen)");
        }
    });
    _back->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::GAME;
        }
    });

    addChild(overlay);
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
            _back->activate();
        } else {
            _restart->deactivate();
            _back->deactivate();
            // If any were pressed, reset them
            _restart->setDown(false);
            _back->setDown(false);
        }
    }
}
