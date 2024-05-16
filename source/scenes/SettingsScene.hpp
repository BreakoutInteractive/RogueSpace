//
//  SettingsScene.hpp
//  RS

#ifndef SettingsScene_hpp
#define SettingsScene_hpp

#include <stdio.h>
#include <cugl/cugl.h>

#include "../utility/SaveData.hpp"
#include "../controllers/AudioController.hpp"

/**
 *
 */
class SettingsScene : public cugl::Scene2 {
public:
    /**
     * The menu choice.
     *
     * This state allows the top level application to know what the user
     * chose.
     */
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants to return to close the settings menu */
        CLOSE,
        /** User wants to increase master volume */
        VOLUP,
        /** User wants to decrease master volume */
        VOLDOWN,
        /** User wants to increase sfx volume */
        SFXUP,
        /** User wants to decrease sfx volume */
        SFXDOWN,
        /** User wants to increase music volume */
        MUSICUP,
        /** User wants to decrease music volume */
        MUSICDOWN,
        /** User wants to toggle whether the bow aiming is inverted */
        INVERT
    };

protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The button for going back to the previous screen */
    std::shared_ptr<cugl::scene2::Button> _close;
    /** The button for reducing master volume */
    std::shared_ptr<cugl::scene2::Button> _volDown;
    /** The bar indicating master volume */
    std::shared_ptr<cugl::scene2::SceneNode> _volBar;
    /** The button for increasing master volume */
    std::shared_ptr<cugl::scene2::Button> _volUp;
    /** The button for reducing sfc volume */
    std::shared_ptr<cugl::scene2::Button> _sfxDown;
    /** The bar indicating sfx volume */
    std::shared_ptr<cugl::scene2::SceneNode> _sfxBar;
    /** The button for increasing sfx volume */
    std::shared_ptr<cugl::scene2::Button> _sfxUp;
    /** The button for reducing music volume */
    std::shared_ptr<cugl::scene2::Button> _musicDown;
    /** The bar indicating music volume */
    std::shared_ptr<cugl::scene2::SceneNode> _musicBar;
    /** The button for increasing music volume */
    std::shared_ptr<cugl::scene2::Button> _musicUp;
    /** The button for toggling whether bow aiming is inverted */
    std::shared_ptr<cugl::scene2::Button> _invert;
    
    /** The player choice */
    Choice _choice;
    bool _active;

    SaveData::Preferences _prefs;
    std::shared_ptr<AudioController> _audioController;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new  menu scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    SettingsScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~SettingsScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    /**
     * Initializes the controller contents.
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
    bool init(const std::shared_ptr<cugl::AssetManager>& assets, std::shared_ptr<AudioController> audio);

    /**
     * Sets whether the scene is currently active
     *
     * This method should be used to toggle all the UI elements.  Buttons
     * should be activated when it is made active and deactivated when
     * it is not.
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) override;
    
    /**
     * Returns the user's menu choice.
     *
     * This will return NONE if the user had no yet made a choice.
     *
     * @return the user's menu choice.
     */
    Choice getChoice() const { return _choice; }

};


#endif /* SettingsScene_hpp */
