//
//  WinScene.hpp
//  RS
//
//  Created by Dasha Griffiths on 5/17/24.
//

#ifndef WinScene_hpp
#define WinScene_hpp

#include <cugl/cugl.h>
#include "../controllers/AudioController.hpp"

using namespace cugl;

/**
 * This is the Gameover screen with basic menu options.
 */
class WinScene : public cugl::Scene2 {

public:
    /**
     * The menu choice. This state allows the top level application to know what the user
     * chose.
     */
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants to restart game */
        RESTART,
        /** User wants to go to main menu */
        MAIN_MENU
    };
    
protected:
    /** The asset manager for this scene. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The player choice */
    Choice _choice;
    
#pragma mark Scene Nodes
    /** the title background texture (drawn separately) */
    std::shared_ptr<Texture> _backgroundTexture;
    /** the scaling factor to apply onto the background texture */
    float _backgroundScale;
    /** The button for restarting game */
    std::shared_ptr<scene2::Button> _restart;
    /** The button for going back to main menu */
    std::shared_ptr<scene2::Button> _main;
    
public:
#pragma mark - Constructors
    /**
     * Creates a new death scene
     */
    WinScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    ~WinScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    /**
     * Initializes the controller contents.
     *
     * @param assets    The (loaded) assets for this game mode
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
#pragma mark - Scene Functionality
        
    void setActive(bool value) override;
    
    /**
     * Returns the user's menu choice.
     *
     * This will return NONE if the user had no yet made a choice.
     *
     * @return the user's menu choice.
     */
    Choice getChoice(){ return _choice; }
    
    void render(const std::shared_ptr<SpriteBatch> &batch) override;
};

#endif /* winScene_hpp */
