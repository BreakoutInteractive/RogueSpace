//
//  GameScene.cpp
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
    _level->setAssets(_assets);
    _backgroundTexture = assets->get<Texture>("background");
    
    // Create the world and attach the listeners.
    std::shared_ptr<physics2::ObstacleWorld> world = _level->getWorld();
    activateWorldCollisions(world);
    
    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    _scale = dimen.width == SCENE_WIDTH ? dimen.width/world->getBounds().getMaxX() :
                                          dimen.height/world->getBounds().getMaxY();
    _level->setDrawScale(Vec2(_scale, _scale));
#pragma mark - GameScene:: Scene Graph Initialization
    
    // Create the scene graph nodes
    _debugNode = scene2::SceneNode::alloc();
    _debugNode->setContentSize(Size(SCENE_WIDTH,SCENE_HEIGHT));
  
    // TODO: This works as starter but victory screens are usually separate game modes (scenes)
    // We make this game scene inactive and transition to other scenes
    _winNode = scene2::Label::allocWithText("VICTORY!",_assets->get<Font>(PRIMARY_FONT));
    _winNode->setAnchor(Vec2::ANCHOR_CENTER);
    _winNode->setPosition(dimen/2.0f);
    _winNode->setForeground(STATIC_COLOR);
    _winNode->setVisible(false);

    _resetNode = scene2::Label::allocWithText(RESET_MESSAGE,_assets->get<Font>(PRIMARY_FONT));
    _resetNode->setAnchor(Vec2::ANCHOR_CENTER);
    _resetNode->setPosition(dimen/2.0f);
    _resetNode->setForeground(STATIC_COLOR);
    _resetNode->setVisible(false);
      
    addChild(_debugNode); //this we keep
    addChild(_winNode); //TODO: remove
    addChild(_resetNode); //TODO: remove

    _level->setDebugNode(_debugNode); // Obtains ownership of root.
  
#pragma mark - Game State Initialization
    _active = true;
    _complete = false;
    setDebug(false);
    
    Application::get()->setClearColor(Color4f::WHITE);
    return true;
}

void GameScene::dispose() {
    if (_active) {
        _input.dispose();
        _debugNode = nullptr;
        _winNode = nullptr; // TODO: remove
        _resetNode = nullptr; // TODO: remove
        _level = nullptr;
        _complete = false;
        _debug = false;
        Scene2::dispose();
    }
}

#pragma mark -
#pragma mark Physics Initialization
void GameScene::activateWorldCollisions(const std::shared_ptr<physics2::ObstacleWorld>& world) {
    world->activateCollisionCallbacks(true);
    world->onBeginContact = [this](b2Contact* contact) {
        beginContact(contact);
    };
    world->beforeSolve = [this](b2Contact* contact, const b2Manifold* oldManifold) {
        beforeSolve(contact,oldManifold);
    };
}


#pragma mark -
#pragma mark Physics Handling

void GameScene::preUpdate(float dt) {
	if (_level == nullptr) {
		return;
	}

	// Check to see if new level loaded yet
	if (_resetNode->isVisible()) {
		if (_assets->complete()) {
			_level = nullptr;
      
			// Access and initialize level
			_level = _assets->get<LevelModel>(LEVEL_ONE_KEY); //TODO: dynamic level loading
			_level->setAssets(_assets);
			_level->setDebugNode(_debugNode); // Obtains ownership of debug node.
			_level->showDebug(_debug);
            _level->setDrawScale(Vec2(_scale, _scale));
            activateWorldCollisions(_level->getWorld());

			_resetNode->setVisible(false);
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
        _resetNode->setVisible(true);
        _assets->load<LevelModel>(LEVEL_ONE_KEY,LEVEL_ONE_FILE); //TODO: reload current level in dynamic level loading
        setComplete(false);
        return;
    }
    if (_input.didExit())  {
        CULog("Shutting down");
        Application::get()->quit();
    }

#pragma mark - handle player input
    // Apply the force to the player
    std::shared_ptr<Player> player = _level->getPlayer();
    Vec2 moveForce = _input.getMoveDirection();
    
    auto _atkCD = player->_atkCD.getCount();
    auto _parryCD = player->_parryCD.getCount();
    auto _dodgeCD = player->_dodgeCD.getCount();
    //only move fast if we're not attacking, parrying, or dodging
    if (_atkCD == _parryCD == _dodgeCD == 0 && player->_dodgeDuration.isZero()) {
        //if all abilities are active
        player->setForce(moveForce * 5); //TODO: use json data
        player->applyForce();
    }

    std::shared_ptr<physics2::CapsuleObstacle> atk = _level->getAttack();

    //TODO: Determine precedence for dodge, parry, and attack. We should only allow one at a time. What should we do if the player inputs multiple at once?
    //Not sure if this will be possible on mobile, but it's definitely possible on the computer
    
    //for now, give higher precedence to dodge than to attack but only allow one at a time
    if (_input.didAttack() && player->_atkCD.isZero() && player->_parryCD.isZero() && player->_dodgeCD.isZero() && !_input.didDodge()) {
        //attack points from player to mouse
        Vec2 direction = _input.getAttackDirection();
        Vec2 playerPos = player->getPosition() * player->getDrawScale();
        //convert from screen to drawing coords
        direction.y = SCENE_HEIGHT - direction.y;
        //convert to player coords
        direction -= playerPos;
        direction.normalize();
        //compute angle from x-axis (since that is where the right cap of a capsule, i.e. the attack hitbox, points)
        float ang = acos(direction.dot(Vec2::UNIT_X));
        if (SCENE_HEIGHT - _input.getAttackDirection().y < playerPos.y) ang *= -1;
        atk->setEnabled(true);
        atk->setAngle(ang);
        atk->setPosition(player->getPosition());
        player->_atkCD.reset();
    }
    else if (player->_atkCD.isZero()){
        atk->setEnabled(false);
    }

    //for now, give higher precendence to attack than to parry but only allow one at a time
    if (_input.didParry() && player->_parryCD.isZero() && player->_atkCD.isZero() && player->_dodgeCD.isZero() && !_input.didAttack() && !_input.didDodge()) {
        //TODO: handle parry
        CULog("parried");
        player->animateParry();
        player->_parryCD.reset();
    }
    else if (player->_parryCD.isZero()) player->animateDefault();

    //for now, give highest precedence to dodge but only allow one at a time
    if (_input.didDodge() && player->_dodgeCD.isZero() && player->_parryCD.isZero() && player->_atkCD.isZero()) {
        CULog("dodged");
        player->_dodgeCD.reset();
        player->_dodgeDuration.reset(); // set dodge frames
    }
    if (!player->_dodgeDuration.isZero()){
        CULog("dodging");
        auto force = _input.getDodgeDirection();
        //player->setLinearDamping(20);
        player->setForce(force*50);
        player->applyForce();
    }
    
    // if we not dodging or move
    if (moveForce.length() == 0 && player->_dodgeDuration.isZero()){
        // dampen
        float dampen = _atkCD > 0 ? -10.0f : -2.0f ; //dampen faster when attacking
        player->setForce(dampen * player->getLinearVelocity());
        player->applyForce();
    }
    
    player->updateCounters();
    
}


void GameScene::fixedUpdate(float step) {
    // Turn the physics engine crank.
    if (_level != nullptr){
        _level->getWorld()->update(step);
    }
}


void GameScene::postUpdate(float remain) {
	// TODO: possibly apply interpolation.
    // We will need more data structures for this
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


#pragma mark -
#pragma mark Collision Handling

void GameScene::beginContact(b2Contact* contact) {
//    b2Body* body1 = contact->GetFixtureA()->GetBody();
//    b2Body* body2 = contact->GetFixtureB()->GetBody();
//    
//    // If we hit the "win" door, we are done
//    intptr_t rptr = reinterpret_cast<intptr_t>(_level->getRocket().get());
//    intptr_t dptr = reinterpret_cast<intptr_t>(_level->getExit().get());
//
//    if((body1->GetUserData().pointer == rptr && body2->GetUserData().pointer == dptr) ||
//       (body1->GetUserData().pointer == dptr && body2->GetUserData().pointer == rptr)) {
//        setComplete(true);
//    }
    
    //TODO: player should only collide with walls, borders during dodge. should not collide with enemies, enemy attacks, etc.
    //TODO: parry

}


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

#pragma mark -
#pragma mark Rendering

void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch)  {
    if (_level != nullptr){
        batch->begin(getCamera()->getCombined());
        Size s = Application::get()->getDisplaySize();
        batch->draw(_backgroundTexture, Rect(0, 0, s.width, s.height));
        _level->render(batch);
        batch->end();
    }
    // draw the debug component
    Scene2::render(batch);
}
