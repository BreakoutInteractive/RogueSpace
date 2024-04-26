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
    _selectedUpgrade = "";
    _option1 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item"));
    _option1Lvl = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item_name"));
    _option1Change = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item_description"));
    _option1Type = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item_tier"));

    _option2 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1"));
    _option2Lvl = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1_name"));
    _option2Change = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1_description"));
    _option2Type = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1_tier"));
    

    _confirm1 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_confirm"));
    _confirm2 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_confirm-1"));
    _confirm1->setVisible(false);
    _confirm2->setVisible(false);
    
    // Program the buttons
    _confirm1->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::UPGRADE_1;
            _selectedUpgrade = _option1Type->getText();
        }
    });
    _confirm2->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::UPGRADE_2;
            _selectedUpgrade = _option2Type->getText();
            
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
        } else{
            _confirm2->setVisible(false);
            _confirm2->deactivate();
        }
    });

    addChild(scene);
    setActive(false);
    return true;
}

void UpgradesScene::getRandomUpgrade(unsigned long size){
    _displayedAttribute1 = std::rand()%size;
    //get random attributes
    
    
    _displayedAttribute2 = std::rand()%size;
    while (_displayedAttribute2==_displayedAttribute1){
        _displayedAttribute2 =std::rand()%size;
    }

}

void UpgradesScene::updateScene(std::vector<std::shared_ptr<Upgradeable>> attributes){
    
    if (_displayedAttribute1== -1 && _displayedAttribute2== -1){
        getRandomUpgrade(attributes.size());
    }
    
    
    _option1Lvl->setText(strtool::format("Level %d", attributes.at(_displayedAttribute1)->getCurrentLevel()+1));
    
    _option1Type->setText(attributes.at(_displayedAttribute1)->getType());
    if (attributes.at(_displayedAttribute1)->getCurrentLevel()==attributes.at(_displayedAttribute1)->getMaxLevel()){
        _option1Lvl->setText(strtool::format("MAXED OUT"));
        _option1Change->setText(strtool::format("%g ", attributes.at(_displayedAttribute1)->getCurrentValue()));
    } else{
        _option1Change->setText(strtool::format("%g -> %g", attributes.at(_displayedAttribute1)->getCurrentValue(), attributes.at(_displayedAttribute1)->getNextValue()));
    }
    
    
    _option2Lvl->setText(strtool::format("Level %d", attributes.at(_displayedAttribute2)->getCurrentLevel()+1));
    
    _option2Type->setText(attributes.at(_displayedAttribute2)->getType());
    
    if (attributes.at(_displayedAttribute2)->getCurrentLevel()==attributes.at(_displayedAttribute2)->getMaxLevel()){
        _option2Lvl->setText(strtool::format("MAXED OUT"));
        _option2Change->setText(strtool::format("%g ", attributes.at(_displayedAttribute2)->getCurrentValue()));
    } else{
        _option2Change->setText(strtool::format("%g -> %g", attributes.at(_displayedAttribute2)->getCurrentValue(), attributes.at(_displayedAttribute2)->getNextValue()));
    }
    
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
            _confirm1->deactivate();
            _option2->deactivate();
            _confirm1->deactivate();
            
            _displayedAttribute1 = -1;
            _displayedAttribute2 = -1;
            
            // If any were pressed, reset them
            _option1->setDown(false);
            _confirm1->setDown(false);
            _option2->setDown(false);
            _confirm2->setDown(false);
        }
    }
}


