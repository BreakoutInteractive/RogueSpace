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
#include <array>
#include "../models/Player.hpp"
#include "../controllers/AIController.hpp"
#include "../controllers/AudioController.hpp"
#include "../controllers/CameraController.hpp"
#include "../controllers/InputController.hpp"
#include "../controllers/CollisionController.hpp"
#include "../models/LevelModel.hpp"
#include "GameRenderer.hpp"
#include "../utility/LevelParser.hpp"
#include "../models/Counter.hpp"
#include "../components/Animation.hpp"
#include "TransitionScene.hpp"
#include "UpgradesScene.hpp"
#include "../utility/SaveData.hpp"

/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class GameScene : public cugl::Scene2 {
    
public:
    /**
     * Unlike other scenes where the user has choices, the player's only choice is pausing
     * the game. The other options are game-events.
     */
    enum ExitCode {
        /** the player stays in the game */
        NONE,
        /** the player finishes the game */
        VICTORY,
        /** the player loses the game */
        DEATH,
        /** the player pauses the game*/
        PAUSE
    };
    
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The maximum level number to defeat for the game to end */
    int MAX_LEVEL;
    
#pragma mark Controllers
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

#pragma mark Scenes
    /** custom renderer for this scene */
    GameRenderer _gameRenderer;
    /** the pop up effects scene (locked to 720p)*/
    Scene2 _effectsScene;
    /** The inner transition between levels (fading in and out) */
    TransitionScene _levelTransition;
    /** the upgrades menu  */
    UpgradesScene _upgrades;
    
    /**
     * @note the order is irrelevant. The function `generateUpgradeIndices` make up for this lack of specification.
     * @return the list of current upgrades for the given player
     */
    std::array<Upgradeable, 7> getPlayerUpgrades(std::shared_ptr<Player> player);
    /**
     * @note if `player` is a null pointer, the entire set of upgrades are valid and implies that there are no current upgrades.
     * @return a pair of pairs of (indices, level) indicating the two upgrades to be shown in upgrade scene.
     */
    std::pair<std::pair<int,int>, std::pair<int,int>> generateUpgradeIndices(std::shared_ptr<Player> player);

#pragma mark Scene Nodes
    /** Reference to the physics node of this scene graph */
    std::shared_ptr<cugl::scene2::SceneNode> _debugNode;
    /** Reference to the win message label */
    std::shared_ptr<cugl::scene2::Label> _winNode;

    /** the node which renders the area clear sprite */
    std::shared_ptr<scene2::SpriteNode> _areaClearNode;
    /** the node which renders the dead effect */
    std::shared_ptr<scene2::SpriteNode> _deadEffectNode;

#pragma mark Scene Animation
    /** animation manager */
    scene2::ActionManager _actionManager;
    /** the key to the area clear effect to monitor updates */
    const std::string AREA_CLEAR_KEY = "area_clear";
    /** the action corresponding to the area clear animation*/
    std::shared_ptr<scene2::Animate> _areaClearAnimation;
    /** the key to the dead  effect to monitor updates */
    const std::string DEAD_EFFECT_KEY = "dead_effect";
    /** the action corresponding to the dead effect pop-up animation*/
    std::shared_ptr<scene2::Animate> _deadEffectAnimation;
    
#pragma mark State and Model
    /** tiled parser */
    LevelParser _parser;
    /** the current level to load */
    int _levelNumber;
    /** whether the current level to load is an upgrade room */
    bool _isUpgradeRoom;
    /** whether the current level to load is a tutorial level */
    bool _isTutorial;
    /** whether the current mode (if tutorial) should return to tutorial menu */
    bool _isTutorialComplete;
    /** The scale between the physics world and the screen (MUST BE UNIFORM) */
    float _scale;
    /** The level model */
    std::shared_ptr<LevelModel> _level;
    /**
     * Whether we have completed this level's mission
     * @note completion of a level requires that the level itself is completed.
     */
    bool _complete;
    /** Whether we got defeated */
    bool _defeat;
    /** Whether or not debug mode is active */
    bool _debug;
    /** a counter for the number of frames to apply a hit-pause effect (for combo hit) */
    Counter hitPauseCounter;
    /** The screen transitioning code */
    ExitCode _exitCode;
    
#pragma mark Internal Update Function Helpers
    /**
     * handles player inputs and updates relevant HUD components.
     */
    void processPlayerInput();
    
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
     * Returns a reference to the input controller
     */
    InputController& getInput() { return _input; }

    /**
     * toggle input devices
     */
    void activateInputs(bool value){ _input.setActive(value); }
    
    /**
     * calls `setTutorialActive` and loads given tutorial level.
     */
    void activateTutorial(int level);
    /**
     * Sets whether tutorial is active or not
     */
    void setTutorialActive(bool active){
        _isTutorial = active;
        _isTutorialComplete = false;
    }
    /**
     * @return whether the running tutorial level is completed
     */
    bool isTutorialComplete(){return _isTutorial && _isTutorialComplete; }
    
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
    }

    /**
     * Sets whether the player was defeated
     *
     * If true, there will be a game over
     *
     * @param value whether the player was defeated
     */
    void setDefeat(bool value) { _defeat = value; }
    
    /**
     * @return the screen exit code, `NONE` if the user stays in this game mode.
     */
    ExitCode getExitCode(){ return _exitCode; }
    
    /**
     * @return list of levels in the order of attack, bow, attack speed, shield, dash, parry, health
     */
    std::vector<int> getPlayerLevels();
    
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
     * sets the active level to load with the given save data
     */
    void setLevel(SaveData::Data saveData);
    
    /**
     * returns the asset key for the given level
     */
    std::string getLevelKey(int level);
    
    /**
     * sets whether this room is an upgrades room.
     */
    void setUpgradeRoom(bool value){ _isUpgradeRoom = value; }
    
    /**
     * Draws the game scene with the given sprite batch. Depending on the game internal state,
     * the debug scene may be drawn.
     */
    virtual void render(const std::shared_ptr<SpriteBatch>& batch) override;

    void setActive(bool value) override;
    
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
