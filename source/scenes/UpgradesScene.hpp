//
//  UpgradesScene.hpp
//  RS
//
//  Created by Dasha Griffiths on 4/13/24.
//

#ifndef UpgradesScene_hpp
#define UpgradesScene_hpp

#include <stdio.h>
#include <cugl/cugl.h>

/**
 *
 */
class UpgradesScene : public cugl::Scene2 {
public:
    /**
     * The upgrade choice.
     *
     * This state allows the top level application to know what the user
     * chose.
     */
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants upgrade option 1 */
        UPGRADE_1,
        /** User wants upgrade option 2 */
        UPGRADE_2
    };
protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The button for upgrade option 1 */
    std::shared_ptr<cugl::scene2::Button> _option1;
    /** The button for upgrade option 2 */
    std::shared_ptr<cugl::scene2::Button> _option2;
    /** The button to confirm upgrade option 1*/
    std::shared_ptr<cugl::scene2::Button> _confirm1;
    /** The button to confirm upgrade option 2*/
    std::shared_ptr<cugl::scene2::Button> _confirm2;
    /** The player choice */
    Choice _choice;
    bool _active;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new  menu scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    UpgradesScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~UpgradesScene() { dispose(); }
    
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
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

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

#endif /* UpgradesScene_hpp */
    
