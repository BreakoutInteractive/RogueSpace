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
#include <cugl/io/CUJsonReader.h>
#include "JoyStick.hpp"

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
    
    std::shared_ptr<scene2::SceneNode> _joystickAimButton;
    
    std::vector<std::shared_ptr<scene2::SceneNode>> _stamina;
    
    /** Reference to the joystick object */
    std::shared_ptr<JoyStick> _joystick;

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
    
#pragma mark -
#pragma mark View (Methods)
    
    /**
     * sets the rendering scale (from world to screen)
     */
    void setDrawScale(Vec2 scale);
    
    /**
     * sets the joystick position given an anchor screen position and the actual touch screen position
     */
    void setJoystickPosition(Vec2 anchorPos, Vec2 screenPos);
    
    /**
     * sets whether the joystick should be visible
     */
    void setJoystickVisible(bool visible) { _joystick->setActive(visible);}
    
    /**
     * sets whether the cooldown should be visible
     */
    void setCooldownVisible (bool visible) {
        for (auto it = _stamina.begin(); it != _stamina.end(); it++){
            (*it)->setVisible(visible);
        }
    }
    
    /**
     * sets whether or not this HUD scene is activated.
     */
    void setActivated(bool value);
        
    /**
     * Draws the game scene with the given sprite batch.
     */
    virtual void render(const std::shared_ptr<SpriteBatch>& batch) override;
    
};

#endif /* GameRenderer_hpp */
