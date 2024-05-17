//
//  PauseScene.hpp
//  RS

#ifndef PauseScene_hpp
#define PauseScene_hpp

#include <stdio.h>
#include <cugl/cugl.h>

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
    std::shared_ptr<scene2::Button> _back;
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
    /** whether a confirmation pop up is needed (for the back button) */
    bool _activateConfirmation;
    
#pragma mark - Icon Labels
    
    std::shared_ptr<scene2::Label> _atk;
    std::shared_ptr<scene2::Label> _bow;
    std::shared_ptr<scene2::Label> _atkSpeed;
    std::shared_ptr<scene2::Label> _shield;
    std::shared_ptr<scene2::Label> _dash;
    std::shared_ptr<scene2::Label> _parry;
    std::shared_ptr<scene2::Label> _maxHealth;
    
#pragma mark Internal Functionality
    
    /**
     * @param active whether to activate the confirmation menu
     */
    void activateConfirmMenu(bool active);
    
    /**
     * @param active whether to activate the button listeners on pause menu
     */
    void activatePauseMenuButtons(bool active);
    
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
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<AssetManager>& assets);

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
     * Sets the icon level labels in this menu.
     * @param levels a list of levels in the order given
     * @pre order: attack, bow, attack speed, shield, dash, parry, health
     */
    void setLabels(std::vector<int> levels);
    
    /**
     * @param value whether to show confirmation pop up when deciding to exit mode
     * @note this is particular useful if quitting interrupts progress as the pause menu is a stopping point in time until resume is chosen.
     */
    void setConfirmationAlert(bool value){ _activateConfirmation = value; }
    
    /**
     * Returns the user's menu choice.
     *
     * This will return NONE if the user had no yet made a choice.
     *
     * @return the user's menu choice.
     */
    Choice getChoice() const { return _choice; }
    
    void update(float dt) override;
    
    void render(const std::shared_ptr<SpriteBatch>& batch) override;

};


#endif /* PauseScene_hpp */
