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

using namespace cugl;

class JoyStick;

class GameRenderer : public cugl::Scene2 {
    
#pragma mark -
#pragma mark HUD elements
private:
    /** Reference to the joystick object */
    std::shared_ptr<JoyStick> _joystick;
    
    

public:
    
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
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
    /**
     * @return the joystick reference
     */
    const std::shared_ptr<JoyStick> getJoystick() {return _joystick; }
    
    /**
     * Draws the game scene with the given sprite batch.
     */
    virtual void render(const std::shared_ptr<SpriteBatch>& batch) override;
    
};

#endif /* GameRenderer_hpp */
