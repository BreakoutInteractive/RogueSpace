//
//  UpgradesScene.cpp
//  RS
//
//  Created by Dasha Griffiths on 4/13/24.
//

#include <cugl/cugl.h>


#include "../models/GameConstants.hpp"
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
    
    _healthTexture = assets->get<Texture>("upgrade_maxHealth");
    _parryTexture = assets->get<Texture>("upgrade_parry");
    _shieldTexture = assets->get<Texture>("upgrade_shield");
    _atkSdTexture = assets->get<Texture>("upgrade_atkSpeed");
    _dashTexture = assets->get<Texture>("upgrade_dash");
    _bowTexture = assets->get<Texture>("upgrade_bow");
    _swordTexture = assets->get<Texture>("upgrade_atk");
    
    _option1 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_choice1_item"));
    _option1Name = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_choice1_item_name"));
    _option1Descrip = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_choice1_item_description"));
    _option1Level = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_choice1_item_tier"));
    _option1Icon = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("upgrades_choice1_item_icon"));

    _option2 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_choice2_item"));
    _option2Name = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_choice2_item_name"));
    _option2Descrip = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_choice2_item_description"));
    _option2Level = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("upgrades_choice2_item_tier"));
    _option2Icon = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("upgrades_choice2_item_icon"));
    

    _confirm1 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_choice1_confirm"));
    _confirm2 = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("upgrades_choice2_confirm"));
    _confirm1->setVisible(false);
    _confirm2->setVisible(false);
    
    _confirm1->addListener([this](const std::string& name, bool down) {
        if (down) {
            _upgrade = _displayedAttribute1.first;
            _level = _displayedAttribute1.second;
            _selectedUpgrade = true;
            AudioController::playUiFX("upgrade");
        }
    });
    _confirm2->addListener([this](const std::string& name, bool down) {
        if (down) {
            _upgrade = _displayedAttribute2.first;
            _level = _displayedAttribute2.second;
            _selectedUpgrade = true;
            AudioController::playUiFX("upgrade");
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
            AudioController::playUiFX("menuClick");
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
            AudioController::playUiFX("menuClick");
        } else{
            _confirm2->setVisible(false);
            _confirm2->deactivate();
        }
    });

    addChild(scene);
    setActive(false);
    return true;
}

void UpgradesScene::setButtonText(UpgradeType upgrade, int level, int buttonType){
    std::string upgradeName;
    std::string upgradeDescription;
    std::string upgradeType = "LVL " + std::to_string(level);
    Color4 typeColor = Color4("#FFFFFF");
    std::shared_ptr<Texture> upgradeTexture;
    switch (upgrade) {
        case UpgradeType::SWORD:
            upgradeName = "SWORD";
            upgradeDescription = "+ Sword Power";
            upgradeTexture = _swordTexture;
            break;
        case UpgradeType::PARRY:
            upgradeName = "PARRY";
            upgradeDescription = "+ Stun Time";
            upgradeTexture = _parryTexture;
            break;
        case UpgradeType::SHIELD: case UpgradeType::BLOCK:
            upgradeName = "DEFENSE";
            upgradeDescription = "+ Armor + Block";
            upgradeTexture = _shieldTexture;
            break;
        case UpgradeType::ATK_SPEED:
            upgradeName = "SLASH";
            upgradeDescription = "+ Sword Speed";
            upgradeTexture = _atkSdTexture;
            break;
        case UpgradeType::BOW:
            upgradeName = "BOW";
            upgradeDescription = "+ Shot Power";
            upgradeTexture = _bowTexture;
            break;
        case UpgradeType::DASH:
            upgradeName = "DASH";
            upgradeDescription = "+ Dash Stamina";
            upgradeTexture = _dashTexture;
            break;
        case UpgradeType::HEALTH:
            upgradeName = "Health";
            upgradeDescription = "+ Max HP";
            upgradeTexture = _healthTexture;
            break;
    }
    if (buttonType==0) {
        _option1Name->setText(upgradeName);
        _option1Descrip->setText(upgradeDescription);
        _option1Level->setText(upgradeType);
        _option1Level->setForeground(typeColor);
        _option1Icon->setTexture(upgradeTexture);
    } else{
        _option2Name->setText(upgradeName);
        _option2Descrip->setText(upgradeDescription);
        _option2Level->setText(upgradeType);
        _option2Level->setForeground(typeColor);
        _option2Icon->setTexture(upgradeTexture);
    }
    
}

void UpgradesScene::updateScene(std::pair<Upgradeable, Upgradeable> upgradeOptions){
    _displayedAttribute1.first = upgradeOptions.first.getType();
    _displayedAttribute1.second = upgradeOptions.first.getCurrentLevel();
    
    _displayedAttribute2.first = upgradeOptions.second.getType();
    _displayedAttribute2.second = upgradeOptions.second.getCurrentLevel();
    
    setButtonText(_displayedAttribute1.first, _displayedAttribute1.second, 0);
    setButtonText(_displayedAttribute2.first, _displayedAttribute2.second, 1);
}

void UpgradesScene::dispose() {
    _option1 = nullptr;
    _option2 = nullptr;
    _confirm1 = nullptr;
    _confirm2 = nullptr;
    removeAllChildren();
}

void UpgradesScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _option1->activate();
            _option2->activate();
            _selectedUpgrade = false;
        } else {
            _option1->deactivate();
            _confirm1->deactivate();
            _option2->deactivate();
            _confirm2->deactivate();
            
            // If any were pressed, reset them
            _option1->setDown(false);
            _confirm1->setDown(false);
            _option2->setDown(false);
            _confirm2->setDown(false);
        }
    }
}

bool UpgradesScene::isInputProcessed(Vec2 pos){
    if (!isActive()){ return false; }
    // an input will be processed by any of the buttons as long as they're active and input location is within bounds of the buttons.
    bool option1 = _option1->isActive() && _option1->inContentBounds(pos);
    bool option2 = _option2->isActive() && _option2->inContentBounds(pos);
    bool confirm1 = _confirm1->isActive() && _confirm1->inContentBounds(pos);
    bool confirm2 = _confirm2->isActive() && _confirm2->inContentBounds(pos);
    return option1 || option2 || confirm1 || confirm2;
}
