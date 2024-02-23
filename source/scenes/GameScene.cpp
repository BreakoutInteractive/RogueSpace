//
//  GameScene.cpp
//
//  You will notice that we do not use a Scene asset this time.  While we could
//  have done this, we wanted to highlight the issues of connecting physics
//  objects to scene graph objects.  Hence we include all of the API calls.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt
//  well to data driven design.  This demo has a lot of simplifications to make
//  it a bit easier to see how everything fits together.  However, the model
//  classes and how they are initialized will need to be changed if you add
//  dynamic level loading.
//
//  Author: Walker White and Anthony Perello (3/12/17)
//  Modified: Zhiyuan Chen
//  Version: 2/17/24
//
#include "GameScene.h"
#include "../models/JSLevelConstants.h"
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>

#include <ctime>
#include <string>
#include <iostream>
#include <sstream>

using namespace cugl;

#pragma mark -
#pragma mark Level Geography

/** This is the size of the active portion of the screen */
#define SCENE_WIDTH 1024
#define SCENE_HEIGHT 576

/** Color to outline the physics nodes */
#define STATIC_COLOR    Color4::YELLOW
/** Opacity of the physics outlines */
#define DYNAMIC_COLOR   Color4::GREEN

/** The key for collisions sounds */
#define COLLISION_SOUND     "bump"
/** The key for the font reference */
#define PRIMARY_FONT        "retro"

/** The message to display on a level reset */
#define RESET_MESSAGE       "Resetting"

/** Threshold for generating sound on collision */
#define SOUND_THRESHOLD     3


#pragma mark -
#pragma mark Constructors

GameScene::GameScene() : Scene2(),
_complete(false),
_debug(false){}


bool GameScene::init(const std::shared_ptr<AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = computeActiveSize();
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    _level = assets->get<LevelModel>(LEVEL_ONE_KEY);
    if (_level == nullptr) {
        CULog("Fail!");
        return false;
    }

    _assets = assets;
    _input.init();

    // Create the world and attach the listeners.
    std::shared_ptr<physics2::ObstacleWorld> world = _level->getWorld();
    activateWorldCollisions(world);
    
    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    _scale = dimen.width == SCENE_WIDTH ? dimen.width/world->getBounds().getMaxX() :
                                          dimen.height/world->getBounds().getMaxY();
    Vec2 offset((dimen.width-SCENE_WIDTH)/2.0f,(dimen.height-SCENE_HEIGHT)/2.0f);
    
    // Create the scene graph
    _rootnode = scene2::SceneNode::alloc();
    _rootnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _rootnode->setPosition(offset);
  
    _winnode = scene2::Label::allocWithText("VICTORY!",_assets->get<Font>(PRIMARY_FONT));
    _winnode->setAnchor(Vec2::ANCHOR_CENTER);
    _winnode->setPosition(dimen/2.0f);
    _winnode->setForeground(STATIC_COLOR);
    _winnode->setVisible(false);

    _loadnode = scene2::Label::allocWithText(RESET_MESSAGE,_assets->get<Font>(PRIMARY_FONT));
    _loadnode->setAnchor(Vec2::ANCHOR_CENTER);
    _loadnode->setPosition(dimen/2.0f);
    _loadnode->setForeground(STATIC_COLOR);
    _loadnode->setVisible(false);
      
    addChild(_rootnode);
    addChild(_winnode);
    addChild(_loadnode);

    _rootnode->setContentSize(Size(SCENE_WIDTH,SCENE_HEIGHT));
    _level->setAssets(_assets);
    _level->setRootNode(_rootnode); // Obtains ownership of root.
  
    _active = true;
    _complete = false;
    setDebug(false);
    
    // XNA nostalgia
    Application::get()->setClearColor(Color4f::CORNFLOWER);
    return true;
}

void GameScene::activateWorldCollisions(const std::shared_ptr<physics2::ObstacleWorld>& world) {
    world->activateCollisionCallbacks(true);
    world->onBeginContact = [this](b2Contact* contact) {
        beginContact(contact);
    };
    world->beforeSolve = [this](b2Contact* contact, const b2Manifold* oldManifold) {
        beforeSolve(contact,oldManifold);
    };
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    if (_active) {
        _input.dispose();
        _rootnode = nullptr;
        _winnode = nullptr;
        _loadnode = nullptr;
        _level = nullptr;
        _complete = false;
        _debug = false;
        Scene2::dispose();
    }
}


#pragma mark -
#pragma mark Physics Handling
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
void GameScene::preUpdate(float dt) {
	if (_level == nullptr) {
		return;
	}

	// Check to see if new level loaded yet
	if (_loadnode->isVisible()) {
		if (_assets->complete()) {
			_level = nullptr;
      
			// Access and initialize level
			_level = _assets->get<LevelModel>(LEVEL_ONE_KEY);
			_level->setAssets(_assets);
			_level->setRootNode(_rootnode); // Obtains ownership of root.
			_level->showDebug(_debug);
      
            activateWorldCollisions(_level->getWorld());

			_loadnode->setVisible(false);
		} else {
			// Level is not loaded yet; refuse input
			return;
		}
	}
    _input.update(dt);
    
    // Process the toggled key commands
    if (_input.didDebug()) {
        setDebug(!isDebug());
    }
    if (_input.didReset()) { 
        // Unload the level but keep in memory temporarily
        _assets->unload<LevelModel>(LEVEL_ONE_KEY);

        // Load a new level and quit update
        _loadnode->setVisible(true);
        _assets->load<LevelModel>(LEVEL_ONE_KEY,LEVEL_ONE_FILE);
        setComplete(false);
        return;
    }
    if (_input.didExit())  {
        CULog("Shutting down");
        Application::get()->quit();
    }

    // Apply the force to the rocket
    std::shared_ptr<RocketModel> rocket = _level->getRocket();
    rocket->setFX(_input.getHorizontal() * rocket->getThrust());
    rocket->setFY(_input.getVertical() * rocket->getThrust());
    rocket->applyForce();

    // Animate the three burners
    updateBurner(RocketModel::Burner::MAIN,  rocket->getFY() >  1);
    updateBurner(RocketModel::Burner::LEFT,  rocket->getFX() >  1);
    updateBurner(RocketModel::Burner::RIGHT, rocket->getFX() <  -1);
}

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
void GameScene::fixedUpdate(float step) {
    // Turn the physics engine crank.
    _level->getWorld()->update(step);
}

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
void GameScene::postUpdate(float remain) {
	// TODO: Update this demo to do interpolation.
    // We will need more data structures for this
}

/**
 * Updates that animation for a single burner
 *
 * This method is here instead of the the rocket model because of our philosophy
 * that models should always be lightweight.  Animation includes sounds and other
 * assets that we do not want to process in the model
 *
 * @param  burner   The rocket burner to animate
 * @param  on       Whether to turn the animation on or off
 */
void GameScene::updateBurner(RocketModel::Burner burner, bool on) {
	std::shared_ptr<RocketModel> rocket = _level->getRocket();
    std::string sound = rocket->getBurnerSound(burner);
    if (on) {
        rocket->animateBurner(burner, true);
        if (!AudioEngine::get()->isActive(sound) && sound.size() > 0) {
            auto source = _assets->get<Sound>(sound);
            AudioEngine::get()->play(sound,source,true,source->getVolume());
        }
    } else {
        rocket->animateBurner(burner, false);
        if (AudioEngine::get()->isActive(sound)) {
            AudioEngine::get()->clear(sound);
        }
    }
}

/**
 * Processes the start of a collision
 *
 * This method is called when we first get a collision between two objects.  We use
 * this method to test if it is the "right" kind of collision.  In particular, we
 * use it to test if we make it to the win door.
 *
 * @param  contact  The two bodies that collided
 */
void GameScene::beginContact(b2Contact* contact) {
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();
    
    // If we hit the "win" door, we are done
    intptr_t rptr = reinterpret_cast<intptr_t>(_level->getRocket().get());
    intptr_t dptr = reinterpret_cast<intptr_t>(_level->getExit().get());

    if((body1->GetUserData().pointer == rptr && body2->GetUserData().pointer == dptr) ||
       (body1->GetUserData().pointer == dptr && body2->GetUserData().pointer == rptr)) {
        setComplete(true);
    }
}

/**
 * Handles any modifications necessary before collision resolution
 *
 * This method is called just before Box2D resolves a collision.  We use this method
 * to implement sound on contact, using the algorithms outlined in Ian Parberry's
 * "Introduction to Game Physics with Box2D".
 *
 * @param  contact  	The two bodies that collided
 * @param  oldManfold  	The collision manifold before contact
 */
void GameScene::beforeSolve(b2Contact* contact, const b2Manifold* oldManifold) {
    float speed = 0;

    // Use Ian Parberry's method to compute a speed threshold
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();
    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);
    b2PointState state1[2], state2[2];
    b2GetPointStates(state1, state2, oldManifold, contact->GetManifold());
    for(int ii =0; ii < 2; ii++) {
        if (state2[ii] == b2_addState) {
            b2Vec2 wp = worldManifold.points[0];
            b2Vec2 v1 = body1->GetLinearVelocityFromWorldPoint(wp);
            b2Vec2 v2 = body2->GetLinearVelocityFromWorldPoint(wp);
            b2Vec2 dv = v1-v2;
            speed = b2Dot(dv,worldManifold.normal);
        }
    }
    
    // Play a sound if above threshold
    if (speed > SOUND_THRESHOLD) {
        // These keys result in a low number of sounds.  Too many == distortion.
        physics2::Obstacle* data1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
        physics2::Obstacle* data2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

        if (data1 != nullptr && data2 != nullptr) {
            std::string key = (data1->getName()+data2->getName());
            auto source = _assets->get<Sound>(COLLISION_SOUND);
            if (!AudioEngine::get()->isActive(key)) {
                AudioEngine::get()->play(key, source, false, source->getVolume());
            }
        }
    }
}

/**
 * Returns the active screen size of this scene.
 *
 * This method is for graceful handling of different aspect
 * ratios
 */
Size GameScene::computeActiveSize() const {
    Size dimen = Application::get()->getDisplaySize();
    float ratio1 = dimen.width/dimen.height;
    float ratio2 = ((float)SCENE_WIDTH)/((float)SCENE_HEIGHT);
    if (ratio1 < ratio2) {
        dimen *= SCENE_WIDTH/dimen.width;
    } else {
        dimen *= SCENE_HEIGHT/dimen.height;
    }
    return dimen;
}
