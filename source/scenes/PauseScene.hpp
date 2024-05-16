//
//  PauseScene.hpp
//  RS

#ifndef PauseScene_hpp
#define PauseScene_hpp

#include <stdio.h>
#include <cugl/cugl.h>
#include "../controllers/AudioController.hpp"

using namespace cugl;

/**
 * A lovely-designed pause menu.
 */
class PauseScene : public Scene2 {
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
        /** User wants to return to previous scene */
        BACK,
        /** User wants to go back to a game */
        RESUME,
        /** User wants to go to settings */
        SETTINGS
    };

protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The player choice */
    Choice _choice;

#pragma mark - Menu Buttons
    /** The button for going back to main */
    std::shared_ptr<scene2::Button> _pauseBack;
    /** The button for going back to game */
    std::shared_ptr<scene2::Button> _resume;
    /** The button for in-game settings */
    std::shared_ptr<scene2::Button> _settings;
    /** Exit confirmation menu**/
    std::shared_ptr<scene2::Button> _confirmBack;
    /** Exit to main menu**/
    std::shared_ptr<scene2::Button> _confirmConfirm;
    /** confirmation scene**/
    Scene2 _confirmationScene;
    /** Controller to play sounds */
    std::shared_ptr<AudioController> _audioController;
    
#pragma mark - Icon Labels
    
    std::shared_ptr<scene2::Label> _atk;
    std::shared_ptr<scene2::Label> _bow;
    std::shared_ptr<scene2::Label> _atkSpeed;
    std::shared_ptr<scene2::Label> _shield;
    std::shared_ptr<scene2::Label> _dash;
    std::shared_ptr<scene2::Label> _parry;
    std::shared_ptr<scene2::Label> _maxHealth;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new  menu scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    PauseScene() : Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~PauseScene() { dispose(); }
    
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
    bool init(const std::shared_ptr<AssetManager>& assets, std::shared_ptr<AudioController> audio);

#pragma mark - Scene Functionality
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
     * @returns whether confirm menu active or not.
     */
    bool isConfirmActive(){return _confirmationScene.isActive();}
    
    /**
     * activates confrim menu buttons, deactivates pause menu buttons
     *
     * @param value whether the scene is currently active
     */
    void activateConfirmButtons(bool active);
    
    /**
     * Sets the icon level labels in this menu.
     * @param levels a list of levels in the order given
     * @pre order: attack, bow, attack speed, shield, dash, parry, health
     */
    void setLabels(std::vector<int> levels);
    
    /**
     * Returns the user's menu choice.
     *
     * This will return NONE if the user had no yet made a choice.
     *
     * @return the user's menu choice.
     */
    Choice getChoice() const { return _choice; }
    
    void render(const std::shared_ptr<SpriteBatch>& batch) override;

};


#endif /* PauseScene_hpp */
