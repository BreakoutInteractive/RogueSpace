//
//  SettingsScene.cpp
//  RS
//

#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

#include "SettingsScene.hpp"

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
bool SettingsScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
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
    std::shared_ptr<scene2::SceneNode> scene = _assets->get<scene2::SceneNode>("setting_menu");
    scene->setContentSize(dimen);
    scene->doLayout();

    _choice = Choice::NONE;
    _close = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("setting_menu_setting_menu_close_setting"));
    _volDown = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_down"));
    _volBar = _assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_bar");
    _volUp = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_up"));
    _sfxDown = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_down_7"));
    _sfxBar = _assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_bar_6");
    _sfxUp = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_up_5"));
    _musicDown = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_down_10"));
    _musicBar = _assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_bar_9");
    _musicUp = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("setting_menu_setting_menu_vol_up_8"));
    _invert = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("setting_menu_setting_menu_setting_selection"));
    _invert->setToggle(true);
    if (SaveData::hasPreferences()) {
        _prefs = SaveData::getPreferences();
        _invert->setDown(_prefs.inverted);
        //master volume bar
        auto volFront = _volBar->getChildByName("vol_front");
        CUAssertLog(_prefs.vol >= 0 && _prefs.vol <= 10, "master volume out of range (0-10)");
        for (int i = _prefs.vol; i < 10; i++) {
            volFront->getChildByName("vol_front" + (i == 0 ? "" : "_" + std::to_string(i)))->setVisible(false);
        }
        //music volume bar
        auto musicFront = _musicBar->getChildByName("vol_front");
        CUAssertLog(_prefs.BGMvol >= 0 && _prefs.BGMvol <= 10, "music volume out of range (0-10)");
        for (int i = _prefs.BGMvol; i < 10; i++) {
            musicFront->getChildByName("vol_front" + (i == 0 ? "" : "_" + std::to_string(i)))->setVisible(false);
        }
        //sfx volume bar
        auto sfxFront = _sfxBar->getChildByName("vol_front");
        CUAssertLog(_prefs.SFXvol >= 0 && _prefs.SFXvol <= 10, "sfx volume out of range (0-10)");
        for (int i = _prefs.SFXvol; i < 10; i++) {
            sfxFront->getChildByName("vol_front" + (i == 0 ? "" : "_" + std::to_string(i)))->setVisible(false);
        }
    }
    else {
        _invert->setDown(true);
        auto volFront = _volBar->getChildByName("vol_front");
        for (int i = 5; i < 10; i++) {
            volFront->getChildByName("vol_front_"+ std::to_string(i))->setVisible(false);
        }
    }

    // Program the buttons
    _close->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::CLOSE;
            SaveData::savePreferences(_prefs);
            CULog("closing (settings screen)");
        }
    });
    _volDown->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::VOLDOWN;
            if (_prefs.vol > 0) _prefs.vol--;
            std::string key = "vol_front" + (_prefs.vol == 0 ? "" : "_" + std::to_string(_prefs.vol));
            _volBar->getChildByName("vol_front")->getChildByName(key)->setVisible(false);
            CULog("master volume down (settings screen)");
        }
    });
    _volUp->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::VOLUP;
            if (_prefs.vol < 10) _prefs.vol++;
            std::string key = "vol_front" + (_prefs.vol - 1 == 0 ? "" : "_" + std::to_string(_prefs.vol - 1));
            _volBar->getChildByName("vol_front")->getChildByName(key)->setVisible(true);
            CULog("master volume up (settings screen)");
        }
    });
    _sfxDown->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::SFXDOWN;
            if (_prefs.SFXvol > 0) _prefs.SFXvol--;
            std::string key = "vol_front" + (_prefs.SFXvol == 0 ? "" : "_" + std::to_string(_prefs.SFXvol));
            _sfxBar->getChildByName("vol_front")->getChildByName(key)->setVisible(false);
            CULog("sfx volume down (settings screen)");
        }
    });
    _sfxUp->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::SFXUP;
            if (_prefs.SFXvol < 10) _prefs.SFXvol++;
            std::string key = "vol_front" + (_prefs.SFXvol - 1 == 0 ? "" : "_" + std::to_string(_prefs.SFXvol - 1));
            _sfxBar->getChildByName("vol_front")->getChildByName(key)->setVisible(true);
            CULog("sfx volume up (settings screen)");
        }
    });
    _musicDown->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::MUSICDOWN;
            if (_prefs.BGMvol > 0) _prefs.BGMvol--;
            std::string key = "vol_front" + (_prefs.BGMvol == 0 ? "" : "_" + std::to_string(_prefs.BGMvol));
            _musicBar->getChildByName("vol_front")->getChildByName(key)->setVisible(false);
            CULog("music volume down (settings screen)");
        }
    });
    _musicUp->addListener([this](const std::string& name, bool down) {
        if (down) {
            _choice = Choice::MUSICUP;
            if (_prefs.BGMvol < 10) _prefs.BGMvol++;
            std::string key = "vol_front" + (_prefs.BGMvol - 1 == 0 ? "" : "_" + std::to_string(_prefs.BGMvol - 1));
            _musicBar->getChildByName("vol_front")->getChildByName(key)->setVisible(true);
            CULog("music volume up (settings screen)");
        }
    });
    _invert->addListener([this](const std::string& name, bool down) {
        _choice = Choice::INVERT;
        _prefs.inverted = !_prefs.inverted;
        CULog("swapping bow aiming mode (settings screen)");
    });

    addChild(scene);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void SettingsScene::dispose() {
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
void SettingsScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);

        if (value) {
            _choice = NONE;
            _close->activate();
            _volDown->activate();
            _volUp->activate();
            _sfxDown->activate();
            _sfxUp->activate();
            _musicDown->activate();
            _musicUp->activate();
            _invert->activate();
        } else {
            _close->deactivate();
            _volDown->deactivate();
            _volUp->deactivate();
            _sfxDown->deactivate();
            _sfxUp->deactivate();
            _musicDown->deactivate();
            _musicUp->deactivate();
            _invert->deactivate();
            // If any were pressed, reset them
            _close->setDown(false);
            _volDown->setDown(false);
            _volUp->setDown(false);
            _sfxDown->setDown(false);
            _sfxUp->setDown(false);
            _musicDown->setDown(false);
            _musicUp->setDown(false);
            //don't reset invert since it's a toggle
        }
    }
}