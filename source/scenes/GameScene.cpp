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
#include "../models/GameConstants.hpp"
#include "../models/Enemy.hpp"
#include "../models/Projectile.hpp"
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>
#include "JoyStick.hpp"

#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include "../components/Animation.hpp"

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

/** The key for the font reference */
#define PRIMARY_FONT        "retro"

/** The message to display on a level reset */
#define RESET_MESSAGE       "Resetting"

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
    } else if (!_gameRenderer.cugl::Scene2::init(dimen)){
        return false;
    }
    
    
    _offset = Vec2((dimen.width-SCENE_WIDTH)/2.0f,(dimen.height-SCENE_HEIGHT)/2.0f);
    
    _level = assets->get<LevelModel>(LEVEL_ONE_KEY);
    if (_level == nullptr) {
        CULog("Fail!");
        return false;
    }

    _assets = assets;
    
    _gameRenderer.init(_assets);
    _gameRenderer.setGameElements(getCamera(), _level);
    
    _input.init();
    _level->setAssets(_assets);
    
    // Create the world and attach the listeners.
    std::shared_ptr<physics2::ObstacleWorld> world = _level->getWorld();
    
    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    _scale = dimen.width == SCENE_WIDTH ? dimen.width/_level->getViewBounds().width :
                                          dimen.height/_level->getViewBounds().height;
    Vec2 drawScale(_scale, _scale);
    _level->setDrawScale(drawScale);
    _gameRenderer.setDrawScale(drawScale);
    
    _audioController = std::make_shared<AudioController>();
    _camController.init(getCamera(), 2.5f);
    auto p = _level->getPlayer();
    _camController.setCamPosition(p->getPosition() * p->getDrawScale());
    
    _AIController.init(_level);
    
    _collisionController.setLevel(_level);
    _audioController->init(_assets);
    _collisionController.setAssets(_assets, _audioController);
    
#pragma mark - GameScene:: Scene Graph Initialization
    
    // Create the scene graph nodes
    _debugNode = scene2::SceneNode::alloc();
    _debugNode->setContentSize(Application::get()->getDisplaySize());
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

void GameScene::restart(){
    _assets->unload<LevelModel>(LEVEL_ONE_KEY);

    // Load a new level and quit update
    _resetNode->setVisible(true);
    _assets->load<LevelModel>(LEVEL_ONE_KEY,LEVEL_ONE_FILE); //TODO: reload current level in dynamic level loading
    _AIController.init(_assets->get<LevelModel>(LEVEL_ONE_KEY));
    _collisionController.setLevel(_assets->get<LevelModel>(LEVEL_ONE_KEY));
    setComplete(false);
    setDefeat(false);
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
            _level->setDrawScale(Vec2(_scale, _scale));
            _level->setDebugNode(_debugNode); // Obtains ownership of debug node.
            _level->showDebug(_debug);
            _collisionController.setLevel(_level);
            _gameRenderer.setGameElements(getCamera(), _level);
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
    if (_input.didExit())  {
        CULog("Shutting down");
        Application::get()->quit();
    }
    
    // TODO: can be removed, but for pc devs to quickly reset
    if (_input.didReset()){
        restart();
        return;
    }
    
    // TODO: this is only a temporary win condition, revisit after Gameplay Release
    if (!_winNode->isVisible() && !_loseNode->isVisible()){
        // game not won, check if any enemies active
        int activeCount = 0;
        auto enemies = _level->getEnemies();
        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            if ((*it)->getCollider()->isEnabled()) {
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
    
#ifdef CU_TOUCH_SCREEN
    if(_input.isMotionActive()){
        _gameRenderer.setJoystickPosition(_input.getInitTouchLocation(), _input.getTouchLocation());
    }
    else
    {
        _gameRenderer.setJoystickVisible(false);
    }
#endif
    
    // update the direction the player is facing
    if (moveForce.length() > 0){
        player->setFacingDir(moveForce);
    }
    

    //only move if we're not parrying or dodging
    if (_parryCD == 0 && player->_dodgeDuration.isZero() && (player->_hitCounter.getCount() < player->_hitCounter.getMaxCount() - 5)) {
        switch (player->_weapon) {
        case Player::weapon::MELEE:
            player->getCollider()->setLinearVelocity(moveForce * 5); //TODO: use json data
            break;
        case Player::weapon::RANGED:
            //with the ranged weapon, dont move while attacking
            if (!player->isAttacking()) player->getCollider()->setLinearVelocity(moveForce * 5); //TODO: use json data
            break;
        }
        
    } else if (_dodgeCD == 0 && (player->_hitCounter.getCount() < player->_hitCounter.getMaxCount() - 5)) {
        player->getCollider()->setLinearVelocity(Vec2::ZERO);
    }
    

    std::shared_ptr<physics2::WheelObstacle> atk = _level->getAttack();
    //TODO: Determine precedence for dodge, parry, and attack. We should only allow one at a time. What should we do if the player inputs multiple at once?
    //Not sure if this will be possible on mobile, but it's definitely possible on the computer
    if (player->_parryCD.isZero() && player->_atkCD.isZero()) {
        //for now, give highest precedence to dodge
        if (_input.didDodge() && player->_dodgeCD.isZero()) {
            player->_dodgeCD.reset();
            player->_dodgeDuration.reset(); // set dodge frames
            //dodge
            auto force = _input.getDodgeDirection(player->getFacingDir());
            if (force.length() == 0) {
                // dodge in the direction currently facing. normalize so that the dodge is constant speed
                force = player->getFacingDir().getNormalization();
            }
            //player->setLinearDamping(20);
            player->getCollider()->setLinearVelocity(force * 30);
            //player->getShadow()->setLinearVelocity(force * 30);
            player->setFacingDir(force);
        }
        else if (player->_dodgeDuration.isZero()) { //not dodging
            //for now, give middle precedence to attack
            if (_input.didAttack()) {
                Vec2 direction = Vec2::ZERO;
                float ang = 0;
                switch(player->_weapon){
                case Player::weapon::MELEE:
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
                    direction = _input.getAttackDirection(player->getFacingDir());
                    ang = acos(direction.dot(Vec2::UNIT_X));
                    if (direction.y < 0) {
                        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
                        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
                    }

                    atk->setEnabled(true);
                    atk->setAwake(true);
                    atk->setAngle(ang);
                    atk->setPosition(player->getPosition().add(0, 64 / player->getDrawScale().y)); //64 is half of the pixel height of the player
                    player->animateAttack();
                    player->_atkCD.reset();
                    _level->getPlayerAtk()->reset();
                    _level->getPlayerAtk()->start();
                    break;
                case Player::weapon::RANGED:
                    //ranged attack
                    direction = _input.getAttackDirection(player->getFacingDir());
                    ang = acos(direction.dot(Vec2::UNIT_X));
                    if (direction.y < 0) {
                        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
                        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
                    }
                    std::shared_ptr<Projectile> p = Projectile::playerAlloc(player->getPosition().add(0, 64 / player->getDrawScale().y), _assets);
                    p->setDrawScale(Vec2(_scale, _scale));
                    _level->addProjectile(p);
                    std::shared_ptr<physics2::Obstacle> obs = p->getCollider();
                    obs->setLinearVelocity(Vec2(GameConstants::PROJ_SPEED_P,0).rotate(ang));
                    obs->setAngle(ang);
                    break;
                }
            }
            //for now, give lowest precendence to parry. only allow it with the melee weapon
            else if (_input.didParry() && player->_weapon == Player::weapon::MELEE) {
                //TODO: handle parry
                CULog("parried");
                player->animateParry();
                player->_parryCD.reset();
            }
        }
    }

    if (_level->getPlayerAtk()->isCompleted()) {
        atk->setEnabled(false);
    }

    if (_input.didSwap()) player->swapWeapon();

#pragma mark - Enemy movement
    _AIController.update(dt);
    // enemy attacks
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        auto enemy = *it;
        if (enemy->getHealth() <= 0) {
            enemy->setEnabled(false);
            enemy->getAttack()->setEnabled(false);
        }
        if (!enemy->_stunCD.isZero()){
            enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
            enemy->getAttack()->setEnabled(false);
        }
        if (enemy->isEnabled()) {
            // enemy can only begin an attack if not stunned and within range of player and can see them
            bool canBeginNewAttack = !enemy->isAttacking() && enemy->_atkCD.isZero() && enemy->_stunCD.isZero();
            if (canBeginNewAttack && enemy->getPosition().distance(player->getPosition()) <= enemy->getAttackRange() && enemy->getPlayerInSight()) {
                Vec2 direction = player->getPosition() * player->getDrawScale() - enemy->getPosition() * enemy->getDrawScale();
                direction.normalize();
                float ang = acos(direction.dot(Vec2::UNIT_X));
                if (direction.y < 0){
                    // handle downwards case, rotate counterclockwise by PI rads and add extra angle
                    ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
                }
                enemy->getAttack()->setPosition(enemy->getAttack()->getPosition().add(0, 64 / enemy->getDrawScale().y)); //64 is half of the pixel height of the enemy
                enemy->getAttack()->setAngle(ang);
                enemy->setAttacking();
            }
        }
        
    }
    
#pragma mark - Component Updates
    player->updateCounters();
    player->updateAnimation(dt);
    _level->getPlayerAtk()->update(dt);
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        (*it)->updateCounters();
        (*it)->updateAnimation(dt);
    }
    std::vector<std::shared_ptr<Projectile>> projs = _level->getProjectiles();
    for (auto it = projs.begin(); it != projs.end(); ++it) {
        (*it)->updateAnimation(dt);
        if ((*it)->isCompleted()) _level->delProjectile((*it));
    }
}


void GameScene::fixedUpdate(float step) {
    if (_level != nullptr){
        _level->getWorld()->update(step);     // Turn the physics engine crank.
        auto player = _level->getPlayer();
        _camController.update(step);
        _camController.setTarget(player->getPosition() * player->getDrawScale());
        _winNode->setPosition(_camController.getPosition());
        _loseNode->setPosition(_camController.getPosition());
        
        auto enemies = _level->getEnemies();
        for (auto it = enemies.begin(); it != enemies.end(); ++it){
            auto e = *it;
            e->syncPositions();
            e->getAttack()->setPosition(e->getPosition().add(0, 64 / e->getDrawScale().y)); //64 is half of the enemy pixel height
        }
        player->syncPositions();
        _level->getAttack()->setPosition(player->getPosition().add(0, 64 / player->getDrawScale().y)); //64 is half of the pixel height of the player

        auto projs = _level->getProjectiles();
        for (auto it = projs.begin(); it != projs.end(); ++it) (*it)->syncPositions();
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
