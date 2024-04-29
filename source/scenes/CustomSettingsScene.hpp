//
//  CustomSettingsScene.hpp
//  RS
//
//  Created by Vincent Fong on 3/21/24.
//

#ifndef CustomSettingsScene_hpp
#define CustomSettingsScene_hpp

#include <stdio.h>
#include <cugl/cugl.h>

/**
 *
 */
class CustomSettingsScene : public cugl::Scene2 {
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
        /** User wants to restart game */
        RESTART
    };
    

protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** Whether the scene is active or not */
    bool _active;
    
    /** The button for going back to game */
    std::shared_ptr<cugl::scene2::Button> _back;
    
    std::vector<std::shared_ptr<cugl::scene2::Slider>> _sliders;
    std::vector<std::shared_ptr<cugl::scene2::Label>> _labels;
    std::vector<float> _values;
    
    // TODO: temp, fix this
    std::vector<std::string> _varMap;
    
    /** The player choice */
    Choice _choice;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new  menu scene with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    CustomSettingsScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~CustomSettingsScene() { dispose(); }
    
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
    
    /**
     *
     * Write settings values to game constants level
     */
    bool writeTo(int i);

};


#endif /* CustomSettingsScene_hpp */
