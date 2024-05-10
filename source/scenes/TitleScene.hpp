//
//  TitleScene.hpp
//  RS
//
//  Created by Zhiyuan Chen on 5/1/24.
//

#ifndef TitleScene_hpp
#define TitleScene_hpp

#include <cugl/cugl.h>
using namespace cugl;

class TitleScene : public Scene2 {
    
public:
    
    enum SceneType {
        /** Continue option is provided for previous save*/
        WITH_CONTINUE,
        /** Scene without the continue option*/
        WITHOUT_CONTINUE
    };
    
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants to start a new game */
        NEW,
        /** User wants to continue existing run  */
        CONTINUE,
        /** User wants to go to settings */
        SETTINGS,
        /** User wants to go to tutorial menu */
        TUTORIAL
    };
    
protected:
    /** the title background texture (drawn separately) */
    std::shared_ptr<Texture> _backgroundTexture;
    /** the scaling factor to apply onto the background texture */
    float _backgroundScale;
    /** current menu choice */
    Choice _choice;
    /** the variant of the title screen to render */
    SceneType _type;

#pragma mark - Scene with No Saves
    /** the parent node of the title scene without continue button */
    std::shared_ptr<scene2::SceneNode> _scene1;
    /** the button node for new game button */
    std::shared_ptr<scene2::Button> _newGame;
    /** the button node for tutorial button */
    std::shared_ptr<scene2::Button> _tutorial;
    /** the button node for settings button */
    std::shared_ptr<scene2::Button> _settings;
    
#pragma mark - Scene with Previous Saves
    /** the parent node of the title scene with continue button */
    std::shared_ptr<scene2::SceneNode> _scene2;
    /** the button node for continue button */
    std::shared_ptr<scene2::Button> _continue;
    std::shared_ptr<scene2::Button> _newGame2;
    std::shared_ptr<scene2::Button> _tutorial2;
    std::shared_ptr<scene2::Button> _settings2;

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
    TitleScene() : Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    ~TitleScene() { dispose(); }
    
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
     * sets the scene type to render. 
     * @note if you would like to switch the scene, make sure to deactivate the current scene, set the new scene, and make the new scene active.
     */
    void setSceneType(SceneType type);
    
    /**
     * activates this menu scene which includes toggling button visibility and functionality.
     *
     * @param value whether the scene is active
     */
    void setActive(bool value) override;
    
    void render(const std::shared_ptr<SpriteBatch> &batch) override;
};

#endif /* TitleScene_hpp */
