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
#include "GameScene.hpp"
#include "../models/LevelConstants.hpp"
#include "../models/Enemy.hpp"
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
_complete(false), _defeat(false),
_debug(false){}


bool GameScene::init(const std::shared_ptr<AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = computeActiveSize();
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    _offset = Vec2((dimen.width-SCENE_WIDTH)/2.0f,(dimen.height-SCENE_HEIGHT)/2.0f);
    
    _level = assets->get<LevelModel>(LEVEL_ONE_KEY);
    if (_level == nullptr) {
        CULog("Fail!");
        return false;
    }

    _assets = assets;
    _input.init();
    // _aiEngine.init(dimen);
    _level->setAssets(_assets);
    _backgroundTexture = assets->get<Texture>("background");
    
    // Create the world and attach the listeners.
    std::shared_ptr<physics2::ObstacleWorld> world = _level->getWorld();
    activateWorldCollisions(world);
    
    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    _scale = dimen.width == SCENE_WIDTH ? dimen.width/_level->getViewBounds().width :
                                          dimen.height/_level->getViewBounds().height;
    _level->setDrawScale(Vec2(_scale, _scale));
    
    _camController.init(getCamera(), 2.5f);
    auto p = _level->getPlayer();
    _camController.setCamPosition(p->getPosition() * p->getDrawScale());
    
#pragma mark - GameScene:: Scene Graph Initialization
    
    // Create the scene graph nodes
    _debugNode = scene2::SceneNode::alloc();
    _debugNode->setContentSize(Size(SCENE_WIDTH,SCENE_HEIGHT));
    _debugNode->setPosition(Vec2::ZERO);
    _debugNode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugNode->setVisible(true);
  
    // TODO: This works as starter but victory screens are usually separate game modes (scenes)
    // We make this game scene inactive and transition to other scenes
    _winNode = scene2::Label::allocWithText("VICTORY!",_assets->get<Font>(PRIMARY_FONT));
    _winNode->setAnchor(Vec2::ANCHOR_CENTER);
    _winNode->setPosition(dimen/2.0f);
    _winNode->setForeground(STATIC_COLOR);
    _winNode->setVisible(false);

    _loseNode = scene2::Label::allocWithText("GAME OVER", _assets->get<Font>(PRIMARY_FONT));
    _loseNode->setAnchor(Vec2::ANCHOR_CENTER);
    _loseNode->setPosition(dimen / 2.0f);
    _loseNode->setForeground(Color4::RED);
    _loseNode->setVisible(false);

    _resetNode = scene2::Label::allocWithText(RESET_MESSAGE,_assets->get<Font>(PRIMARY_FONT));
    _resetNode->setAnchor(Vec2::ANCHOR_CENTER);
    _resetNode->setPosition(dimen/2.0f);
    _resetNode->setForeground(STATIC_COLOR);
    _resetNode->setVisible(false);
      
    addChild(_debugNode); //this we keep
    addChild(_winNode); //TODO: remove
    addChild(_loseNode); //TODO: remove
    addChild(_resetNode); //TODO: remove

    _debugNode->setContentSize(Size(SCENE_WIDTH,SCENE_HEIGHT));
    _level->setDebugNode(_debugNode); // Obtains ownership of root.
  
#pragma mark - Game State Initialization
    _active = true;
    _complete = false;
    _defeat = false;
    setDebug(false);
    
    Application::get()->setClearColor(Color4f::WHITE);
    return true;
}

void GameScene::dispose() {
    if (_active) {
        _input.dispose();
        _debugNode = nullptr;
        _winNode = nullptr; // TODO: remove
        _loseNode = nullptr; //TODO: remove
        _resetNode = nullptr; // TODO: remove
        _level = nullptr;
        _complete = false;
        _defeat = false;
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
        setDefeat(false);
        return;
    }
    if (_input.didExit())  {
        CULog("Shutting down");
        Application::get()->quit();
    }
    
    // TODO: this is only a temporary win condition, revisit after Gameplay Release
    if (!_winNode->isVisible() && !_loseNode->isVisible()){
        // game not won, check if any enemies active
        int activeCount = 0;
        auto enemies = _level->getEnemies();
        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            if ((*it)->isEnabled()) {
                activeCount += 1;
            }
        }
        if (activeCount == 0){
            setComplete(true);
        }

        if (_level->getPlayer()->_hp==0) setDefeat(true);
    }

#pragma mark - handle player input
    // Apply the force to the player
    std::shared_ptr<Player> player = _level->getPlayer();
    Vec2 moveForce = _input.getMoveDirection();

    auto _atkCD = player->_atkCD.getCount();
    auto _parryCD = player->_parryCD.getCount();
    auto _dodgeCD = player->_dodgeCD.getCount();
    
    // update the direction the player is facing
    if (moveForce.length() > 0){
        player->setFacingDir(moveForce);
    }
    

    //only move fast if we're not attacking, parrying, or dodging
    if (_atkCD == _parryCD == _dodgeCD == 0 && player->_dodgeDuration.isZero()) {
        //if all abilities are active
        //player->setForce(moveForce * 5); //TODO: use json data
        //player->applyForce();
        player->setLinearVelocity(moveForce * 5);
    } else if (_dodgeCD == 0) {
        player->setLinearVelocity(Vec2::ZERO);
    }

    std::shared_ptr<physics2::WheelObstacle> atk = _level->getAttack();
    atk->setPosition(player->getPosition());
    //TODO: Determine precedence for dodge, parry, and attack. We should only allow one at a time. What should we do if the player inputs multiple at once?
    //Not sure if this will be possible on mobile, but it's definitely possible on the computer
    if (player->_parryCD.isZero() && player->_atkCD.isZero()) {
        //for now, give highest precedence to dodge
        if (_input.didDodge() && player->_dodgeCD.isZero()) {
            player->_dodgeCD.reset();
            player->_dodgeDuration.reset(); // set dodge frames
            //dodge
            auto force = _input.getDodgeDirection();
            if (force.length() == 0) {
                // dodge in the direction currently facing. normalize so that the dodge is constant speed
                force = player->getFacingDir().getNormalization();
            }
            //player->setLinearDamping(20);
            player->setLinearVelocity(force * 30);
            player->setFacingDir(force);
        }
        else if (player->_dodgeDuration.isZero()) { //not dodging
            //for now, give middle precedence to attack
            if (_input.didAttack()) {
                // TODO: keep or remove.
                /////// ATTACK POINTS FROM PLAYER TO MOUSE ///////
                //Vec2 direction = _input.getAttackDirection();
                //Vec2 playerPos = player->getPosition() * player->getDrawScale();
                //convert from screen to drawing coords
                //direction.y = SCENE_HEIGHT - direction.y;
                //convert to player coords
                //direction -= playerPos;
                //direction.normalize();
                //compute angle from x-axis
                //float ang = acos(direction.dot(Vec2::UNIT_X));
                //if (SCENE_HEIGHT - _input.getAttackDirection().y < playerPos.y) ang = 2*M_PI - ang;
                /////// END COMPUTATION OF ATTACK DIRECTION ///////
                
                //Vec2 direction = player->getFacingDir();
                Vec2 direction = _input.getAttackDirection();
                float ang = acos(direction.dot(Vec2::UNIT_X));
                if (direction.y < 0){
                    // handle downwards case, rotate counterclockwise by PI rads and add extra angle
                    ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
                }
  
                atk->setEnabled(true);
                atk->setAwake(true);
                atk->setAngle(ang);
                player->animateAttack();
                player->_atkCD.reset();
            }
            //for now, give lowest precendence to parry
            else if (_input.didParry()) {
                //TODO: handle parry
                CULog("parried");
                player->animateParry();
                player->_parryCD.reset();
            }
        }     
    }
    //if (!player->_dodgeDuration.isZero()) {
    //    auto force = _input.getDodgeDirection();
    //    if (force.length() == 0){
    //        // dodge in the direction currently facing. normalize so that the dodge is constant speed
    //        force = player->getFacingDir().getNormalization();
    //    }
    //    //player->setLinearDamping(20);
    //    player->setLinearVelocity(force * 50);
    //    player->setSensor(true);
    //    //player->applyForce();
    //    player->setFacingDir(force);
    //}
    if (player->_atkCD.isZero()) {
        atk->setEnabled(false);
    }
    //if/when we create a dodge animation, add a check for it here
    if (player->_parryCD.isZero() && player->_atkCD.isZero()) player->animateDefault();
    
    //// if we not dodging or move
    //if (moveForce.length() == 0 && player->_dodgeDuration.isZero()){
    //    // dampen
    //    float dampen = _atkCD > 0 ? -10.0f : -2.0f ; //dampen faster when attacking
    //    player->setForce(dampen * player->getLinearVelocity());
    //    player->applyForce();
    //}
    
    player->updateCounters();
    
#pragma mark - Enemy movement
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        (*it)->updateCounters();
        if ((*it)->getHealth() <= 0) {
            (*it)->setEnabled(false);
        }
        // Vec2 f = _aiEngine.lineOfSight((*it), player);
        // (*it)->setForce(f);
        // (*it)->applyForce();
    }
}


void GameScene::fixedUpdate(float step) {
    // Turn the physics engine crank.
    if (_level != nullptr){
        _level->getWorld()->update(step);
        auto player = _level->getPlayer();
        _camController.update(step);
        _camController.setTarget(player->getPosition() * player->getDrawScale());
        _winNode->setPosition(_camController.getPosition());
        _loseNode->setPosition(_camController.getPosition());
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
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();    
    //attack
    intptr_t aptr = reinterpret_cast<intptr_t>(_level->getAttack().get());
    intptr_t pptr = reinterpret_cast<intptr_t>(_level->getPlayer().get());
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        intptr_t eptr = reinterpret_cast<intptr_t>((*it).get());
        if ((body1->GetUserData().pointer == aptr && body2->GetUserData().pointer == eptr) ||
                 (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == aptr)) {
            //attack hitbox is a circle, but we only want it to hit in a semicircle
            Vec2 dir = (*it)->getPosition()*(*it)->getDrawScale() - _level->getPlayer()->getPosition()*_level->getPlayer()->getDrawScale();
            dir.normalize();
            float ang = acos(dir.dot(Vec2::UNIT_X));
            if ((*it)->getPosition().y * (*it)->getDrawScale().y < _level->getPlayer()->getPosition().y * _level->getPlayer()->getDrawScale().y) ang = 2*M_PI-ang;
            if (abs(ang-_level->getAttack()->getAngle())<=M_PI_2 || abs(ang - _level->getAttack()->getAngle()) >= 3*M_PI_2){
                (*it)->hit();
                CULog("Hit an enemy!");
            }
        }
        else if ((body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == eptr) ||
            (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == pptr)) {
            //player takes damage if running into enemy while not dodging
            if (_level->getPlayer()->_dodgeDuration.isZero()) _level->getPlayer()->hit();
        }
    }
    //TODO: player should only collide with walls, borders during dodge. should not collide with enemies, enemy attacks, etc.
    //this is handled in beforeSolve by disabling the contact if the player is dodging and collides with enemies or their attacks
    
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

    intptr_t pptr = reinterpret_cast<intptr_t>(_level->getPlayer().get());
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        intptr_t eptr = reinterpret_cast<intptr_t>((*it).get());
        if ((body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == eptr) ||
            (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == pptr)) {
            //phase through enemies while dodging
            if (!_level->getPlayer()->_dodgeDuration.isZero()) contact->SetEnabled(false);
        }
    }
    //TODO: add another check here for enemy attacks once they can do so
}

#pragma mark -
#pragma mark Rendering

void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch)  {
    // render background
    batch->begin(getCamera()->getCombined());
    Size s = Application::get()->getDisplaySize();
    Vec3 camPos = getCamera()->getPosition();
    batch->draw(_backgroundTexture, Rect(camPos.x - s.width/2, camPos.y - s.height/2, s.width, s.height));

    batch->end();
    if (_level != nullptr){
        batch->begin(getCamera()->getCombined());
        _level->render(batch);
        batch->end();
    }
    // draw the debug component
    Scene2::render(batch);
    CULog("drawn");
}
