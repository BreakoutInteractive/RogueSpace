//
//  GameRenderer.hpp
//  RS
//
//  This is the view-component owned by the GameScene which offers a custom
//  rendering pipeline slightly different from the Scene Graph approach that cugl
//  applications are based on. This class is responsible for rendering the HUD
//  through the use of Scene Nodes on top of the gameplay scene which is rendered
//  using the SpriteBatch.
//
//
//  Created by Zhiyuan Chen on 3/8/24.
//

#ifndef GameRenderer_hpp
#define GameRenderer_hpp
#include <cugl/cugl.h>
#include <cugl/physics2/CUObstacleWorld.h>
#include <vector>
#include <cugl/assets/CUAsset.h>

using namespace cugl;

// forward reference
class LevelModel;
class PauseScene;

class GameRenderer : public cugl::Scene2 {
    
private:
    
    std::shared_ptr<AssetManager> _assets;
    
#pragma mark -
#pragma mark HUD elements
    
    std::shared_ptr<scene2::Button> _pauseButton;
    
    std::shared_ptr<scene2::ProgressBar> _hpBar;
    
    std::shared_ptr<scene2::SceneNode> _joystickRing;
    
    std::shared_ptr<scene2::SceneNode> _joystickMoveButton;
    
    std::shared_ptr<scene2::SceneNode> _joystickAimRing;
    
    std::shared_ptr<scene2::SceneNode> _joystickAimButton;
    /** swap weapon button */
    std::shared_ptr<scene2::Button> _swapButton;
    
    /** frames of holding the move stick */
    int _moveHoldCounter;
    
    /** frames of holding the aim stick */
    int _aimHoldCounter;
    
    std::shared_ptr<scene2::ProgressBar> _stamina;

    /** the background behind the game*/
    std::shared_ptr<Texture> _backgroundTexture;
    
#pragma mark -
#pragma mark Game Components
    
    /** Reference to the game camera  */
    std::shared_ptr<Camera> _gameCam;
    /** Reference to the level to be rendered */
    std::shared_ptr<LevelModel> _level;
    /** whether the pause button has been clicked */
    bool _paused;
    
#pragma mark -
#pragma mark Internal Helper
    
    /** hides all joysticks on screen */
    void hideJoysticks();
    
    /** adjust the joystick to the position (if possible) or in the direction of the position */
    void setJoystickPosition(std::shared_ptr<scene2::SceneNode> ring, std::shared_ptr<scene2::SceneNode> button, Vec2 screenPos);

public:
#pragma mark -
#pragma mark Constructors
    /**
     * The Renderer can be allocated and must be initialized with the assets directory.
     */
    GameRenderer();
    
    /**
     * disposes of all resources used by the rendering pipeline
     */
    ~GameRenderer();
    
    /**
     * Initializes the renderer with the necessary assets and scene graph nodes.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the renderer is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<AssetManager>& assets);
    
#pragma mark -
#pragma mark Properties

    /**
     * Returns whether pause button as been pressed.
     *
     * @return true if the button has been pressed, false otherwise.
     */
    bool getPaused(){return _paused;}
    
    /**
     * sets the game scene camera and level to be used to render the game
     *
     * @param gameCam   The camera from the game scene
     * @param level         The level to be rendered
     */
    void setGameElements(std::shared_ptr<Camera> gameCam, std::shared_ptr<LevelModel> level){
        _gameCam = gameCam;
        _level = level;
    }
    
    /**
     * Input can be viewed as being processed simultaneously but only one UI element across
     * different active screen components should be processing the user input.
     *
     * @return whether an event with the given position will be processed by the HUD elements.
     */
    bool isInputProcessed(Vec2 pos){
        return _pauseButton->inContentBounds(pos) || _swapButton->inContentBounds(pos);
    }
    
#pragma mark -
#pragma mark View (Methods)
    
    /**
     * adjusts the active joystick position given an anchor screen position and the actual touch screen position
     */
    void updateMoveJoystick(bool touched, Vec2 anchorScreenPos, Vec2 screenPos);
    
    /**
     * adjusts the active aim joystick position given an anchor screen position and the actual touch screen position
     *
     * @param touched whether the aim joystick is touched
     */
    void updateAimJoystick(bool touched, Vec2 anchorScreenPos, Vec2 screenPos);
        
    /**
     * sets whether or not this HUD scene is activated.
     *
     * All nodes all hidden when this scene is not activated
     */
    void setActivated(bool value);
    
    /**
     * activates/deactivates the swap button. A deactivated swap button still renders but it is faded out.
     */
    void setSwapButtonActive(bool value);
    
    /** 
     * sets the callback to execute when the swap button is toggled
     */
    void setSwapButtonCallback(std::function<void()> callback);
        
    /**
     * Draws the game scene with the given sprite batch.
     */
    virtual void render(const std::shared_ptr<SpriteBatch>& batch) override;
    
};

#endif /* GameRenderer_hpp */
