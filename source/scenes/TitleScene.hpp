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
    
    enum Choice {
        /** User has not yet made a choice */
        NONE,
        /** User wants to start a new game */
        NEW,
        /** User wants to continue existing run  */
        CONTINUE,
        /** User wants to go to settings */
        SETTINGS
    };
    
protected:

    /** the title background texture (drawn separately) */
    std::shared_ptr<Texture> _backgroundTexture;
    /** the scaling factor to apply onto the background texture */
    float _backgroundScale;
    
    /** the button node for continue button */
    std::shared_ptr<scene2::Button> _continue;
    /** the button node for new game button */
    std::shared_ptr<scene2::Button> _newGame;
    /** the button node for settings button */
    std::shared_ptr<scene2::Button> _settings;
    
    /** current menu choice */
    Choice _choice;
    
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
     * activates/deactivates the continue button. A deactivated continue button is disabled but still renders.
     */
    void setContinueButtonActive(bool value);
    
    /**
     * @return the user's menu choice (NONE if no choice has been selected).
     */
    Choice getChoice(){ return _choice; }
    
    /**
     * activates this menu scene which includes toggling button visibility and functionality.
     *
     * @param value whether the scene is active
     */
    void setActive(bool value) override;
    
    void render(const std::shared_ptr<SpriteBatch> &batch) override;
};

#endif /* TitleScene_hpp */
