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
    std::shared_ptr<scene2::SceneNode> confirmationNode = _assets->get<scene2::SceneNode>("confirmationMenu");
    // Initialize the scene to a locked height
    Size dimen = Application::get()->getDisplaySize();
    dimen *= scene->getContentSize().height/dimen.height;
    if (!Scene2::init(dimen)) {
        return false;
    }
    
    // resize the pause menu scene
    scene->setContentSize(dimen);
    scene->doLayout();
    
    // separate scene root for confirmation nodes
    _confirmationScene.init(dimen);
    auto overlay = scene2::PolygonNode::allocWithPoly(Rect(0, 0, dimen.width, dimen.height));
    overlay->setColor(Color4(0, 0, 0, 200));
    _confirmationScene.addChild(overlay);
    _confirmationScene.addChild(confirmationNode);
    // auto-resize text content
    auto progressLabel = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("confirmationMenu_confirmation_progress"));
    progressLabel->setText("Current progress will not be saved!", true);
    confirmationNode->setContentSize(dimen);
    confirmationNode->doLayout();

    // retrieve the menu buttons
    _back = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("pause_pausemenu_menu_buttons_back"));
    _resume = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("pause_pausemenu_menu_buttons_resume"));
    _settings = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("pause_pausemenu_menu_buttons_setting"));
    
    _confirmBack = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("confirmationMenu_confirmation_back"));
    _confirmConfirm = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("confirmationMenu_confirmation_confirm"));

    _activateConfirmation = false;
    // Program the buttons
    _back->addListener([this](const std::string& name, bool down) {
        if (down) {
            if (_activateConfirmation) {
                _confirmationScene.setActive(true); // the buttons will be activated in the update() call.
                _back->setDown(false);
            } else{
                _choice = Choice::BACK;
            }
            AudioController::playUiFX("menuClick");
        }
    });
    _resume->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::RESUME;
            AudioController::playUiFX("menuClick");
        }
    });
    _settings->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::SETTINGS;
            AudioController::playUiFX("menuClick");
        }
    });

    _confirmBack->addListener([this](const std::string& name, bool down) {
        if (down) {
            _confirmationScene.setActive(false);
            _confirmBack->setDown(false);
            AudioController::playUiFX("menuClick");
        }
    });
    _confirmConfirm->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::BACK;
            AudioController::playUiFX("menuClick");
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
    _confirmationScene.setActive(false);
    setActive(false);
    return true;
}

void PauseScene::dispose() {
    setActive(false);
    removeAllChildren();
}

#pragma mark - Internal
void PauseScene::activatePauseMenuButtons(bool activate){
    if (activate){
        _back->activate();
        _resume->activate();
        _settings->activate();
    }
    else {
        _back->deactivate();
        _resume->deactivate();
        _settings->deactivate();
    }
}

void PauseScene::activateConfirmMenu(bool active){
    if (active){
        _confirmConfirm->activate();
        _confirmBack->activate();
    } else{
        _confirmConfirm->deactivate();
        _confirmBack->deactivate();
    }
}

#pragma mark - Public API
void PauseScene::setLabels(std::vector<int> levels){
    _atk->setText("LVL " + std::to_string(levels[0]));
    _bow->setText("LVL " + std::to_string(levels[1]));
    _atkSpeed->setText("LVL " + std::to_string(levels[2]));
    _shield->setText("LVL " + std::to_string(levels[3]));
    _dash->setText("LVL " + std::to_string(levels[4]));
    _parry->setText("LVL " + std::to_string(levels[5]));
    _maxHealth->setText("LVL " + std::to_string(levels[6]));
}

void PauseScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        activatePauseMenuButtons(value);
        if (!value){
            _confirmationScene.setActive(false);
            activateConfirmMenu(false);
        }
        _back->setDown(false);
        _resume->setDown(false);
        _settings->setDown(false);
        _confirmBack->setDown(false);
        _confirmConfirm->setDown(false);
        _choice = NONE;
    }
}

void PauseScene::update(float dt){
    if (_confirmationScene.isActive()){
        activatePauseMenuButtons(false);
        activateConfirmMenu(true);
    }
    else {
        activatePauseMenuButtons(true);
        activateConfirmMenu(false);
    }
}

void PauseScene::render(const std::shared_ptr<SpriteBatch> &batch){
    Scene2::render(batch);
    if (_confirmationScene.isActive()){
        _confirmationScene.render(batch);
    }
}

