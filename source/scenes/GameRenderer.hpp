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
#include "../controllers/AudioController.hpp"

using namespace cugl;

// forward reference
class LevelModel;
class Animation;

class GameRenderer : public cugl::Scene2 {
    
private:
    
    std::shared_ptr<AssetManager> _assets;
    
#pragma mark -
#pragma mark HUD elements
    
    std::shared_ptr<scene2::Button> _pauseButton;
    
    std::shared_ptr<scene2::ProgressBar> _hpBar;
    std::shared_ptr<scene2::ProgressBar> _stamina;
    
    std::shared_ptr<scene2::SceneNode> _joystickRing;
    std::shared_ptr<scene2::SceneNode> _joystickMoveButton;
    std::shared_ptr<scene2::SceneNode> _joystickAimRing;
    std::shared_ptr<scene2::SceneNode> _joystickAimButton;
    /** frames of holding the move stick */
    int _moveHoldCounter;
    /** frames of holding the aim stick */
    int _aimHoldCounter;
    
    /** swap weapon button */
    std::shared_ptr<scene2::Button> _swapButton;
    
    std::shared_ptr<scene2::SpriteNode> _dashNode;
    /** the dash now indicator animation on stamina */
    std::shared_ptr<Animation> _dashNowEffect;
    
#pragma mark -
#pragma mark Game Components
    
    /** Reference to the game camera  */
    std::shared_ptr<OrthographicCamera> _gameCam;
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
     * sets the game scene camera
     *
     * @param gameCam   The camera from the game scene
     * @param level         The level to be rendered
     */
    void setGameCam(std::shared_ptr<Camera> camera){
        _gameCam = std::dynamic_pointer_cast<OrthographicCamera>(camera);
    }
    
    /**
     * sets the level to be rendered.
     * @pre `setGameCam` must have been given a camera.
     */
    void setGameLevel(std::shared_ptr<LevelModel> level){
        _level = level;
    }
    
    /**
     * Input can be viewed as being processed simultaneously but only one UI element across
     * different active screen components should be processing the user input.
     *
     * @return whether an event with the given position will be processed by the HUD elements.
     */
    bool isInputProcessed(Vec2 pos){
        if (!isActive()){ return false; }
        return _pauseButton->inContentBounds(pos) || _swapButton->inContentBounds(pos);
    }
    
    /**
     * joysticks are rendered to the window/screen space from the world space. The size of the joystick in pixel screen size is useful to determine a deadzone on the joystick.
     * @return joystick screen pixel radius
     */
    float getJoystickScreenRadius();
    
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
     * sets whether or not this HUD scene is active.
     * The HUD contains joysticks and buttons and status bar (that is always active).
     */
    void setActive(bool value) override;
    
    /**
     * activates/deactivates the swap button.
     */
    void setSwapButtonActive(bool value);
    
    /** 
     * sets the callback to execute when the swap button is toggled and whether
     * the button is down/up.
     *
     * @param down true when player is using sword
     * @param callback game event to trigger when button is toggled
     */
    void configureSwapButton(bool down, std::function<void()> callback);
    
    /**
     * updates running animation by `dt` seconds.
     */
    void update(float dt) override;
        
    /**
     * Draws the game scene with the given sprite batch.
     */
    virtual void render(const std::shared_ptr<SpriteBatch>& batch) override;
    
};

#endif /* GameRenderer_hpp */
