//
//  TutorialScene.hpp
//  RS
//
//  Created by Dasha Griffiths on 5/12/24.
//

#ifndef TutorialScene_hpp
#define TutorialScene_hpp

#include <stdio.h>
#include <cugl/cugl.h>
#include "../controllers/AudioController.hpp"

using namespace cugl;

class TutorialScene : public Scene2 {
    
public:
        
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants to play a tutorial level */
        LEVEL,
        /** User wants to go back to Main */
        BACK,
        /** User wants to go to settings */
        SETTINGS
    };
    
protected:
    /** the title background texture (drawn separately) */
    std::shared_ptr<Texture> _backgroundTexture;
    /** the scaling factor to apply onto the background texture */
    float _backgroundScale;
    /** current menu choice */
    Choice _choice;
    /** tutorial level selected */
    int _selectedLevel;

#pragma mark - Tutorial
    /** the parent node of the tutorial scene*/
    std::shared_ptr<scene2::SceneNode> _scene;
    /** the button node for continue button */
    std::shared_ptr<scene2::Button> _level1;
    /** the button node for new game button */
    std::shared_ptr<scene2::Button> _back;
    /** the button node for tutorial button */
    std::shared_ptr<scene2::Button> _level2;
    /** the button node for settings button */
    std::shared_ptr<scene2::Button> _settings;
    
#pragma mark - Internal Scene Helpers
    /**
     * turns on/off the buttons that are currently used by the active scene type
     */
    void activateScene(bool value);
    
public:
    
#pragma mark Constructors
    /**
     * Creates a new title scene
     */
    TutorialScene() : Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    ~TutorialScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    /**
     * initializes the title scene
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the scene is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
#pragma mark Scene Functionality
    
    
    /**
     * @return the user's menu choice (NONE if no choice has been selected).
     */
    Choice getChoice(){ return _choice; }

    /**
     * returns selected tutorial level.
     */
    int getSelectedLevel(){return _selectedLevel;}
        
    /**
     * activates this menu scene which includes toggling button visibility and functionality.
     *
     * @param value whether the scene is active
     */
    void setActive(bool value) override;
    
    void render(const std::shared_ptr<SpriteBatch> &batch) override;
};

#endif /* TutorialScene_hpp */
