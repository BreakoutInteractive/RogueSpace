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
    _selectedUpgrade = 0;
    _option1 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item"));
    _option1Name = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item_name"));
    _option1Descrip = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item_description"));
    _option1Level = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item_tier"));

    _option2 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1"));
    _option2Name = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1_name"));
    _option2Descrip = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1_description"));
    _option2Level = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_upgrade-item-1_tier"));
    

    _confirm1 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_confirm"));
    _confirm2 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_confirm-1"));
    _confirm1->setVisible(false);
    _confirm2->setVisible(false);
    
    // Program the buttons
    _confirm1->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::UPGRADE_1;
            _selectedUpgrade = _displayedAttribute1;
        }
    });
    _confirm2->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::UPGRADE_2;
            _selectedUpgrade = _displayedAttribute2;
            
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
void UpgradesScene::setButtonText(int upgrade, int buttonType){
    std::string buttonName;
    std::string buttonDescription;
    switch (upgrade) {
        case HEALTH:
            buttonName = "HEALTH";
            buttonDescription = "Restore Health";
            break;
        case PARRY:
            buttonName = "HEALTH";
            buttonDescription = "Restore Health";
            break;
        case RANGED:
            buttonName = "BOW";
            buttonDescription = "Increases ranged attack";
            break;
        case MELEE:
            buttonName = "SWORD";
            buttonDescription = "Increases sword damage";
            break;
        case DODGE:
            buttonName = "DASH";
            buttonDescription = "Decreases dash cooldown";
            break;
        case DEFENSE:
            buttonName = "SHIELD";
            buttonDescription = "Increases defense";
            break;
        case MOVEMENT:
            buttonName = "HEALTH";
            buttonDescription = "Restore Health";
            break;
        default:
            break;
            
    }
    if (buttonType==0) {
        _option1Name->setText(buttonName);
        _option1Descrip->setText(buttonDescription);
    } else{
        _option2Name->setText(buttonName);
        _option2Descrip->setText(buttonDescription);
    }
    
}

void UpgradesScene::updateScene(std::vector<int> attributes, std::vector<std::shared_ptr<Upgradeable>> availableUpgrades){
    if (availableUpgrades.at(attributes.at(0))->getCurrentLevel()==availableUpgrades.at(attributes.at(0))->getMaxLevel()){
        _option1Level->setText(strtool::format("MAXED OUT"));
    } else {
        _option1Level->setText(strtool::format("Level %d", availableUpgrades.at(attributes.at(0))->getCurrentLevel()+1));
    }
    
    if (availableUpgrades.at(attributes.at(1))->getCurrentLevel()==availableUpgrades.at(attributes.at(1))->getMaxLevel()){
        _option2Level->setText(strtool::format("MAXED OUT"));
    } else {
        _option2Level->setText(strtool::format("Level %d", availableUpgrades.at(attributes.at(1))->getCurrentLevel()+1));
    }
    setButtonText(attributes.at(0)+3,0);
    setButtonText(attributes.at(1)+3,1);
    
    _displayedAttribute1 = attributes.at(0)+3;
    _displayedAttribute2  =attributes.at(1)+3;
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


