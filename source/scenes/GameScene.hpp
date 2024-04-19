//
//  GameScene.h
//
//  This class manages the gameplay. It also handles collision detection and in particular, executing callback events when collision has been detected. 
//  There is not much to do for collisions; our ObstacleWorld class takes care of all
//  of that for us.  This controller mainly transforms input into gameplay.
//
//  Author: Walker White and Anthony Perello (3/12/17)
//  Modified: Zhiyuan Chen
//  Version: 2/17/24
//
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__
#include <cugl/cugl.h>
#include <box2d/b2_world_callbacks.h>
#include <vector>
#include "../models/Player.hpp"
#include "../controllers/AIController.hpp"
#include "../controllers/AudioController.hpp"
#include "../controllers/CameraController.hpp"
#include "../controllers/InputController.hpp"
#include "../controllers/CollisionController.hpp"
#include "../models/LevelModel.hpp"
#include "GameRenderer.hpp"
#include "../utility/LevelParser.hpp"

/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class GameScene : public cugl::Scene2 {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** tiled parser */
    LevelParser _parser;
    
    int _levelNumber;
    
    // CONTROLLERS
    /** Controller for abstracting out input across multiple platforms */
    InputController _input;
    /** Engine to process mob movement */
    AIController _AIController;
    /** Controller to modify camera behavior */
    CameraController _camController;
    /** Controller for handling collisions */
    CollisionController _collisionController;
    /** Controller to play sounds */
    std::shared_ptr<AudioController> _audioController;
    
    // VIEW
    /** Reference to the physics node of this scene graph */
    std::shared_ptr<cugl::scene2::SceneNode> _debugNode;
    /** Reference to the win message label */
    std::shared_ptr<cugl::scene2::Label> _winNode;
    /** Reference to the lose message label */
    std::shared_ptr<cugl::scene2::Label> _loseNode;

    /** content offset to prevent displays on notch/adjusting aspect ratios*/
    //cugl::Vec2 _offset;
    /** custom renderer for this scene */
    GameRenderer _gameRenderer;

    // MODEL

    /** The scale between the physics world and the screen (MUST BE UNIFORM) */
    float _scale;

    /** The level model */
    std::shared_ptr<LevelModel> _level;
    /**Reference to upgadeable player attributes**/
    std::vector<std::shared_ptr<Upgradeable>> _playerAttributes;

    /** Whether we have completed this "game" */
    bool _complete;
    /** Whether we got defeated */
    bool _defeat;
    /** Whether or not debug mode is active */
    bool _debug;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game world with the default values.
     *
     * This constructor does not allocate any objects or start the controller.
     * This allows us to use a controller without a heap pointer.
     */
    GameScene();
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GameScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    /**
     * Initializes the controller contents, and starts the game
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * The game world is scaled so that the screen coordinates do not agree
     * with the Box2d coordinates.  This initializer uses the default scale.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
#pragma mark -
#pragma mark State Access
    /**
     * Returns true if debug mode is active.
     *
     * If true, all objects will display their physics bodies.
     *
     * @return true if debug mode is active.
     */
    bool isDebug( ) const { return _debug; }
    
    /**
     * Sets whether debug mode is active.
     *
     * If true, all objects will display their physics bodies.
     *
     * @param value whether debug mode is active.
     */
    void setDebug(bool value) { _debug = value; _level->showDebug(value); }
    
    /**
     * Returns a reference to the game renderer
     */
    GameRenderer& getRenderer(){return _gameRenderer;}
    /**
     * Returns a reference to the player's attributes
     */
    std::vector<std::shared_ptr<Upgradeable>> getAttributes() {return _playerAttributes= _level->getPlayer()->getPlayerAttributes();}
    
    /**
     * toggle input devices
     */
    void activateInputs(bool value){ _input.setActive(value); }
    
    /**
     * Returns true if the level is completed.
     *
     * If true, the level will advance after a countdown
     *
     * @return true if the level is completed.
     */
    bool isComplete( ) const { return _complete; }

    /**
     * Returns true if the player was defeated.
     *
     * If true, there will be a game over
     *
     * @return true if the player was defeated.
     */
    bool isDefeat() const { return _defeat; }
    
    /**
     * Sets whether the level is completed.
     *
     * If true, the level will advance after a countdown
     *
     * @param value whether the level is completed.
     */
    void setComplete(bool value) {
        _complete = value;
        //_winNode->setVisible(value);
    }

    /**
     * Sets whether the player was defeated
     *
     * If true, there will be a game over
     *
     * @param value whether the player was defeated
     */
    void setDefeat(bool value) { _defeat = value; _loseNode->setVisible(value); }
    
    
#pragma mark -
#pragma mark Gameplay Handling
    /**
     * The method called to indicate the start of a deterministic loop.
     *
     * This method is used instead of {@link #update} if {@link #setDeterministic}
     * is set to true. It marks the beginning of the core application loop,
     * which is concluded with a call to {@link #postUpdate}.
     *
     * This method should be used to process any events that happen early in
     * the application loop, such as user input or events created by the
     * {@link schedule} method. In particular, no new user input will be
     * recorded between the time this method is called and {@link #postUpdate}
     * is invoked.
     *
     * Note that the time passed as a parameter is the time measured from the
     * start of the previous frame to the start of the current frame. It is
     * measured before any input or callbacks are processed. It agrees with
     * the value sent to {@link #postUpdate} this animation frame.
     *
     * @param dt    The amount of time (in seconds) since the last frame
     */
    void preUpdate(float dt);
    
    /**
     * The method called to provide a deterministic application loop.
     *
     * This method provides an application loop that runs at a guaranteed fixed
     * timestep. This method is (logically) invoked every {@link getFixedStep}
     * microseconds. By that we mean if the method {@link draw} is called at
     * time T, then this method is guaranteed to have been called exactly
     * floor(T/s) times this session, where s is the fixed time step.
     *
     * This method is always invoked in-between a call to {@link #preUpdate}
     * and {@link #postUpdate}. However, to guarantee determinism, it is
     * possible that this method is called multiple times between those two
     * calls. Depending on the value of {@link #getFixedStep}, it can also
     * (periodically) be called zero times, particularly if {@link #getFPS}
     * is much faster.
     *
     * As such, this method should only be used for portions of the application
     * that must be deterministic, such as the physics simulation. It should
     * not be used to process user input (as no user input is recorded between
     * {@link #preUpdate} and {@link #postUpdate}) or to animate models.
     *
     * The time passed to this method is NOT the same as the one passed to
     * {@link #preUpdate}. It will always be exactly the same value.
     *
     * @param step  The number of fixed seconds for this step
     */
    void fixedUpdate(float step);

    /**
     * The method called to indicate the end of a deterministic loop.
     *
     * This method is used instead of {@link #update} if {@link #setDeterministic}
     * is set to true. It marks the end of the core application loop, which was
     * begun with a call to {@link #preUpdate}.
     *
     * This method is the final portion of the update loop called before any
     * drawing occurs. As such, it should be used to implement any final
     * animation in response to the simulation provided by {@link #fixedUpdate}.
     * In particular, it should be used to interpolate any visual differences
     * between the the simulation timestep and the FPS.
     *
     * This method should not be used to process user input, as no new input
     * will have been recorded since {@link #preUpdate} was called.
     *
     * Note that the time passed as a parameter is the time measured from the
     * last call to {@link #fixedUpdate}. That is because this method is used
     * to interpolate object position for animation.
     *
     * @param remain    The amount of time (in seconds) last fixedUpdate
     */
    void postUpdate(float remain);
    
    /**
     * Restarts game scene to initial state.
     */
    void restart();
    
    /**
     * sets the active level to load
     */
    void setLevel(int level);
    
    /**
     * returns the asset key for the given level
     */
    std::string getLevelKey(int level);
    
    /**
     * Applies selected attribute to player.
     */
    void updatePlayerAttributes(std::string selectedAttribute);
    
    /**
     * Draws the game scene with the given sprite batch. Depending on the game internal state,
     * the debug scene may be drawn.
     */
    virtual void render(const std::shared_ptr<SpriteBatch>& batch) override {
        _gameRenderer.render(batch);
        Scene2::render(batch);
    }

protected:
#pragma mark -
#pragma mark Helpers
    
    /**
     * Returns the active screen size of this scene.
     *
     * This method is for graceful handling of different aspect
     * ratios
     */
    cugl::Size computeActiveSize() const;
};

#endif /* __GAME_SCENE_H__ */
