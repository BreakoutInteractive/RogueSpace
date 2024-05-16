//
//  DeathScene.hpp
//  RS
//
//  Created by Zhiyuan Chen on 5/11/24.
//

#ifndef DeathScene_hpp
#define DeathScene_hpp

#include <cugl/cugl.h>
#include "../controllers/AudioController.hpp"

using namespace cugl;

/**
 * This is the Gameover screen with basic menu options and animated background.
 */
class DeathScene : public cugl::Scene2 {

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
    /** Controller to play sounds */
    std::shared_ptr<AudioController> _audioController;
    
#pragma mark Scene Animation
    /** animation manager */
    scene2::ActionManager _actionManager;
    /** the key to the area clear effect to monitor updates */
    const std::string CAPE_ANIM_KEY = "cape_animation";
    /** the action corresponding to the area clear animation*/
    std::shared_ptr<scene2::Animate> _capeAnimation;
    
#pragma mark Scene Nodes
    std::shared_ptr<scene2::SpriteNode> _cape;
    /** The button for restarting game */
    std::shared_ptr<scene2::Button> _restart;
    /** The button for going back to main menu */
    std::shared_ptr<scene2::Button> _main;
    
public:
#pragma mark - Constructors
    /**
     * Creates a new death scene
     */
    DeathScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    ~DeathScene() { dispose(); }
    
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
    bool init(const std::shared_ptr<cugl::AssetManager>& assets, std::shared_ptr<AudioController> audio);
    
#pragma mark - Scene Functionality
    
    void update(float dt) override;
    
    void setActive(bool value) override;
    
    /**
     * Returns the user's menu choice.
     *
     * This will return NONE if the user had no yet made a choice.
     *
     * @return the user's menu choice.
     */
    Choice getChoice(){ return _choice; }
};

#endif /* DeathScene_hpp */
