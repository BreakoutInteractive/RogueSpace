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
#include "../models/MeleeEnemy.hpp"
#include "../models/MeleeLizard.hpp"
#include "../models/RangedEnemy.hpp"
#include "../models/RangedLizard.hpp"
#include "../models/MageAlien.hpp"
#include "../models/Wall.hpp"
#include "../models/HealthPack.hpp"
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>
#include <array>

#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include "../components/Animation.hpp"
#include "../utility/SaveData.hpp"
using namespace cugl;

#pragma mark -
#pragma mark Level Geography

/** This is the size of the active portion of the screen */
#define SCENE_WIDTH 1024
#define SCENE_HEIGHT 576

/** The key for the font reference */
#define PRIMARY_FONT        "retro"

/** The message to display on a level reset */
#define RESET_MESSAGE       "Resetting"

#define NUM_LEVELS_TO_UPGRADE       3

#pragma mark -
#pragma mark Constructors

GameScene::GameScene() : Scene2(),
_complete(false), _defeat(false), _debug(false){}


bool GameScene::init(const std::shared_ptr<AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = computeActiveSize();
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    srand((uint32)time(NULL));

    // initalize controllers with the assets
    _assets = assets;
    _parser.loadTilesets(assets);
    _levelNumber = 1;
    MAX_LEVEL = _assets->get<JsonValue>("constants")->getInt("max-level");
    _gameRenderer.init(_assets);
    auto preprocessor = [this](Vec2 pos){
        return _gameRenderer.isInputProcessed(pos) || _upgrades.isInputProcessed(pos);
    };
    _input.init(preprocessor);
    activateInputs(false);
    _audioController = std::make_shared<AudioController>();
    _audioController->init(_assets);
    _collisionController.setAssets(_assets, _audioController);
    
    CameraController::CameraConfig config;
    config.speed = GameConstants::GAME_CAMERA_SPEED;
    config.minSpeed = GameConstants::GAME_CAMERA_SPEED;
    config.maxSpeed = GameConstants::GAME_CAMERA_MAX_SPEED;
    config.maxZoom = GameConstants::GAME_CAMERA_MAX_ZOOM_OUT;
    _camController.init(getCamera(), config);
    
    // necessary (starting at any actual level implies it is not an upgrade room)
    _isUpgradeRoom = false;
    
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
    _winNode->setForeground(Color4::YELLOW);
    _winNode->setVisible(false);
      
    addChild(_debugNode);  //this we keep
    addChild(_winNode);   //TODO: remove

    _debugNode->setContentSize(Size(SCENE_WIDTH,SCENE_HEIGHT));
    
    // set up the effects scene for large effects (eg. level clear/death)
    Size size = Application::get()->getDisplaySize();
    if (!_effectsScene.init(Size(720 * size.width/size.height, 720))){
        return false;
    }
    std::shared_ptr<scene2::SceneNode> effectsNode = _assets->get<scene2::SceneNode>("gameplay");
    effectsNode->setContentSize(_effectsScene.getSize());
    effectsNode->doLayout();
    _effectsScene.addChild(effectsNode);
    
    _areaClearNode = std::dynamic_pointer_cast<scene2::SpriteNode>(_assets->get<scene2::SceneNode>("gameplay_area_clear_effect"));
    _areaClearAnimation = scene2::Animate::alloc(0, _areaClearNode->getSpan()-1, 1.2f);
    _deadEffectNode = std::dynamic_pointer_cast<scene2::SpriteNode>(_assets->get<scene2::SceneNode>("gameplay_dead_effect"));
    _deadEffectAnimation = scene2::Animate::alloc(0,_deadEffectNode->getSpan()-1, 1.2f);
    
    _levelTransition.init(assets);
    _levelTransition.setInitialColor(Color4(255, 255, 255, 0));
    _levelTransition.setFadeIn(GameConstants::TRANSITION_FADE_IN_TIME);
    _levelTransition.setFadeOut(GameConstants::TRANSITION_FADE_OUT_TIME, Color4(255, 255, 255, 0));
    _levelTransition.setFadeInCallBack([this](){
        if (_isUpgradeRoom){
            // current room was upgrades, just go to the current level number
            _isUpgradeRoom = false;
        }
        else {
            this->_levelNumber+=1;
            this->_isUpgradeRoom = _levelNumber % 3 == 1; // check if an upgrades room should be offered before the next level
        }
        
        // save game data as level ends
        auto p = _level->getPlayer();
        SaveData::Data data;
        data.level = _levelNumber;
        data.hp = p->getHP();
        data.atkLvl = p->getMeleeUpgrade().getCurrentLevel();
        data.dashLvl = p->getDodgeUpgrade().getCurrentLevel();
        data.defLvl = p->getDamageReductionUpgrade().getCurrentLevel();
        data.hpLvl = p->getHPUpgrade().getCurrentLevel();
        data.parryLvl = p->getStunUpgrade().getCurrentLevel();
        data.rangedLvl = p->getBowUpgrade().getCurrentLevel();
        data.weapon = p->getWeapon();
        SaveData::makeSave(data);
        // load the next level
        this->setLevel(data);
    });
    
    _upgrades.init(assets);
  
#pragma mark - Game State Initialization
    setActive(false);
    setComplete(false);
    setDefeat(false);
    _exitCode = NONE;
    hitPauseCounter.setMaxCount(GameConstants::HIT_PAUSE_FRAMES + 3);
    Application::get()->setClearColor(Color4("#c9a68c"));
    return true;
}

void GameScene::dispose() {
    if (isActive()) {
        _input.dispose();
        _debugNode = nullptr;
        _winNode = nullptr; // TODO: remove
        _level = nullptr;
        _complete = false;
        _defeat = false;
        _debug = false;
        Scene2::dispose();
    }
}

void GameScene::restart(){
    _winNode->setVisible(false);
    _levelTransition.setActive(false);
    _isUpgradeRoom = true;  // first level is always upgrades
    SaveData::removeSave();
    SaveData::Data data;
    data.level = 1;
    data.hp = GameConstants::PLAYER_MAX_HP;
    setLevel(data);
    auto player = _level->getPlayer();
    player->setHP(player->getMaxHP());
}

void GameScene::setLevel(SaveData::Data saveData){
    _debugNode->removeAllChildren();
    std::string levelToParse;
    auto level = saveData.level;
    _levelNumber = level;
    if (_isUpgradeRoom){
        levelToParse = "upgrades";
    }
    else{
        levelToParse = getLevelKey(_levelNumber);
    }
    
    CULog("currLevel %d", _levelNumber);
    auto parsed = _parser.parseTiled(_assets->get<JsonValue>(levelToParse));
    Size dimen = computeActiveSize();

    _level = LevelModel::alloc(_assets->get<JsonValue>("constants"), parsed);
    _level->setAssets(_assets);
    
    auto scales = dimen/_level->getViewBounds();
    _scale = dimen.width == SCENE_WIDTH ? scales.width : scales.height;
    _level->setDrawScale(Vec2(_scale, _scale));
    _level->setDebugNode(_debugNode);
    setDebug(isDebug());
    _AIController.init(_level);
    _collisionController.setLevel(_level);
    _gameRenderer.setGameElements(getCamera(), _level);
    
    auto p = _level->getPlayer();
    p->setMaxHPLevel(saveData.hpLvl);
    p->setMeleeLevel(saveData.atkLvl);
    p->setBowLevel(saveData.rangedLvl);
    p->setBlockLevel(saveData.defLvl);
    p->setArmorLevel(saveData.defLvl);
    p->setStunLevel(saveData.parryLvl);
    p->setDodgeLevel(saveData.dashLvl);
    p->setHP(saveData.hp);
    auto weapon = static_cast<Player::Weapon>(saveData.weapon);
    p->setWeapon(weapon);
    _camController.setCamPosition(p->getPosition() * p->getDrawScale());
    
    if (_isUpgradeRoom) {
        _level->getRelic()->setActive(true);
        // turn off the energy walls first
        auto energyWalls = _level->getEnergyWalls();
        for (auto it = energyWalls.begin(); it != energyWalls.end(); ++it) {
            (*it)->deactivate();
        }
        configureUpgradeMenu(p);
    }
    
    setComplete(false);
    setDefeat(false);
    bool usingMelee = weapon == Player::Weapon::MELEE;
    if (usingMelee) _input.activateMeleeControls();
    else _input.activateRangeControls();
    _input.setActive(true);

    _gameRenderer.configureSwapButton(usingMelee, [this](){
        _level->getPlayer()->swapWeapon();
        _input.swapControlMode();
    });
    
    // hide effect nodes
    _areaClearNode->setVisible(false);
    _deadEffectNode->setVisible(false);
}

void GameScene::configureUpgradeMenu(std::shared_ptr<Player> player){
    // first find the stats that can be upgraded
    std::array<Upgradeable, 6> all = {
        player->getMeleeUpgrade(), player->getBowUpgrade(), player->getHPUpgrade(), player->getStunUpgrade(), player->getDodgeUpgrade(), player->getDamageReductionUpgrade()};
    std::vector<Upgradeable> options;
    for (Upgradeable& upgrade : all){
        if (!upgrade.isMaxLevel()){
            options.push_back(upgrade);
        }
    }
    int opt1 = std::rand() % options.size();
    int opt2 = std::rand() % options.size();
    while (opt2==opt1){
        opt2 = std::rand() % options.size();
    }
    std::pair<Upgradeable, Upgradeable> randomOptions(options[opt1], options[opt2]);
    _upgrades.updateScene(randomOptions);
}

std::string GameScene::getLevelKey(int level){
    return "level"+std::to_string(level);
}


#pragma mark -
#pragma mark Physics Handling

void GameScene::processPlayerInput(){
    std::shared_ptr<Player> player = _level->getPlayer();
    Vec2 moveForce = _input.getMoveDirection();
        
#ifdef CU_TOUCH_SCREEN
    if (player->isRangedAttackActive()){
        _gameRenderer.updateAimJoystick(_input.isCombatActive(), _input.getInitCombatLocation(), _input.getCombatTouchLocation());
    }
    _gameRenderer.updateMoveJoystick(_input.isMotionActive(), _input.getInitTouchLocation(), _input.getTouchLocation());
#endif
    
    if (player->isDodging() && player->getCollider()->isBullet()){
        player->getCollider()->setBullet(false);
    }
    
    // set player direction
    if (moveForce.length() > 0 && !player->isDodging() && !player->isAttacking()){
        player->setFacingDir(moveForce);
    }

    // Priority order: Dodge, Attack/Shoot, Parry
    // Only read inputs when player is idle, preparing a shot or waiting to parry
    if (player->isIdle() || player->isRangedAttackActive() || player->isBlocking()) {
        if (_input.didDodge() && player->canDodge()) {
            // player->dodgeCD.reset(); // reset cooldown
            //dodge
            auto force = _input.getDodgeDirection(player->getFacingDir());
            if (force.length() == 0) force = player->getFacingDir().getNormalization();
            Vec2 velocity = force * GameConstants::PLAYER_DODGE_SPEED;
            player->getCollider()->setLinearVelocity(velocity);
            player->getCollider()->setBullet(true);
            player->setFacingDir(force);
            player->setDodging();
            player->reduceStamina();
        }
        else if (!player->isRecovering()){
            //for now, give middle precedence to attack
            if (_input.didAttack()) {
                Vec2 direction = Vec2::ZERO;
                float ang = 0;
                switch(player->getWeapon()){
                case Player::Weapon::MELEE:
                        if (player->canMeleeAttack()){
                            direction = _input.getAttackDirection(player->getFacingDir());
                            ang = acos(direction.dot(Vec2::UNIT_X));
                            if (direction.y < 0) {
                                // handle downwards case, rotate counterclockwise by PI rads and add extra angle
                                ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
                            }
                            player->enableMeleeAttack(ang);
                            player->animateAttack();
                            player->resetAttackCooldown();
                        }
                        break;
                case Player::Weapon::RANGED:
                        player->animateCharge();
                        player->getCollider()->setLinearVelocity(Vec2::ZERO);
                        break;
                }
            }
            else if (_input.didCharge()) {
                Vec2 direction = _input.getAttackDirection(player->getFacingDir());
                if (player->getWeapon() == Player::Weapon::RANGED && player->isRangedAttackActive()) {
                    // this lets you rotate the player while holding the bow in charge mode
                    player->setFacingDir(direction);
                }
            }
            else if (_input.didShoot() && player->getWeapon() == Player::Weapon::RANGED) {
                Vec2 direction = Vec2::ZERO;
                float ang = 0;
                if (player->isRangedAttackActive()) {
                    //ranged attack
                    direction = _input.getAttackDirection(player->getFacingDir());
                    ang = acos(direction.dot(Vec2::UNIT_X));
                    if (direction.y < 0) {
                        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
                        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
                    }
                    std::shared_ptr<Projectile> p = Projectile::playerAlloc(player->getPosition().add(0, 64 / player->getDrawScale().y), player->getBowDamage(), ang, _assets);
                    p->setDrawScale(Vec2(_scale, _scale));
                    _level->addProjectile(p);
                    player->animateShot();
                }
                else player->animateDefault();

            }
            //give lowest precendence to parry. only allow it with the melee weapon
            else if (_input.didParry() && player->getWeapon() == Player::Weapon::MELEE) {
                player->animateParryStart();
            }
            else if (_input.didParryRelease() && player->getWeapon() == Player::Weapon::MELEE) {
                if (player->isBlocking()) {
                    CULog("parried");
                    player->animateParry();
                }
                else player->animateDefault();
            }
        }
    }
    
    // disable the swap button based on player state (for anything requiring the weapon to maintain the same)
    if (!player->isMeleeAttacking()){
        // melee attack is usually fast, does not require visual button lock
        _gameRenderer.setSwapButtonActive(player->isIdle() || player->isDodging());
    }
    
    // the duration of the knockback applied onto the player is part of the iframe counter.
    // only allow the player to change their velocity after the knockback frames
    if (player->getIframes() < GameConstants::PLAYER_IFRAME - GameConstants::PLAYER_INCOMING_KB_FRAMES){
        //stay still while parrying or blocking or recovering
        if (player->isParrying() || player->isBlocking() || player->isRecovering()){
            player->getCollider()->setLinearVelocity(Vec2::ZERO);
        }
        else if (!player->isDodging()){
            switch (player->getWeapon()) {
                case Player::Weapon::MELEE:
                    if (player->isAttacking()) player->getCollider()->setLinearVelocity(moveForce * GameConstants::PLAYER_ATK_MOVE_SPEED);
                    else player->getCollider()->setLinearVelocity(moveForce * GameConstants::PLAYER_MOVE_SPEED);
                    break;
                case Player::Weapon::RANGED:
                    //with the ranged weapon, dont move while attacking
                    if (player->isAttacking()) player->getCollider()->setLinearVelocity(Vec2::ZERO);
                    else player->getCollider()->setLinearVelocity(moveForce * GameConstants::PLAYER_MOVE_SPEED);
                    break;
            }
        }
    }
    
    // TODO: could remove, this is PC-only
    if (_input.didSwap()){
        if (player->isIdle() || player->isDodging()){
            //other states are weapon-dependent, so don't allow swapping while in them
            player->swapWeapon();
            _input.swapControlMode(); // must do for mobile controls
        }
    }
}

void GameScene::preUpdate(float dt) {
    if (_level == nullptr) {
        return;
    }
    
    _input.update(dt);
    
    // Process the toggled key commands
    if (_input.didDebug()) {
        CULog("debug toggled");
        setDebug(!isDebug());
    }
    if (_input.didExit())  {
        CULog("Shutting down");
        Application::get()->quit();
        // _level->getPlayer()->setHP(0); // could use as a toggle to auto-kill player
    }
    
    // TODO: can be removed, but for pc devs to quickly reset
    if (_input.didReset()){
        restart();
        return;
    }
    
    // update the effects that appears on screen for clearing room / dying
    _actionManager.update(dt);
    _areaClearNode->setVisible(_actionManager.isActive(AREA_CLEAR_KEY));
    _deadEffectNode->setVisible(_actionManager.isActive(DEAD_EFFECT_KEY));
    
    if (!isComplete() && !isDefeat()){
        // game not won or lost, check if any enemies active
        int activeCount = 0;
        auto enemies = _level->getEnemies();
        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            if ((*it)->isEnabled()) {
                activeCount += 1;
            }
        }
        // player finishes current level
        if (activeCount == 0){
            setComplete(true);
            auto energyWalls = _level->getEnergyWalls();
            for (auto it = energyWalls.begin(); it != energyWalls.end(); ++it) {
                (*it)->deactivate();
            }
            if (_level->getEnemies().size() > 0){
                // no more enemies remain, but there were enemies initially
                _actionManager.remove(AREA_CLEAR_KEY);
                _actionManager.activate(AREA_CLEAR_KEY, _areaClearAnimation, _areaClearNode);
            }
        }
    }
    
    if (!isDefeat()){
        // player dies
        if (_level->getPlayer()->getHP() == 0){
            SaveData::removeSave();
            setDefeat(true);
            setComplete(false);
            _levelTransition.setActive(false);
            // start playing dead effect, stop the area clear if we happen to have cleared the room and got killed by a flying projectile
            _actionManager.remove(AREA_CLEAR_KEY);
            _actionManager.remove(DEAD_EFFECT_KEY);
            _actionManager.activate(DEAD_EFFECT_KEY, _deadEffectAnimation, _deadEffectNode);
        }
    }
    
    // player sees dead effect and is then shown the dead screen when effect is finished
    if (isDefeat()){
        if (!_actionManager.isActive(DEAD_EFFECT_KEY)){
            // the dead effect is no longer active, so send player to dead screen (and disable player)
            _level->getPlayer()->setEnabled(false);
            _exitCode = DEATH;
            return;
        }
    }
    
    // level is completed when player successfully exits the room
    if (isComplete() && _level->isCompleted()){
        if (_levelNumber < MAX_LEVEL){
            // begin transitioning to next level
            if (!_levelTransition.isActive()){
                _levelTransition.setActive(true);
            }
        }
        else{
            _winNode->setVisible(true); // for now
            // TODO: save data: save game is won by setting level to be greater than max level?
        }
    }
    
    
    if (_collisionController.isComboContact() && hitPauseCounter.isZero()){
        hitPauseCounter.reset();
    }
    if (!hitPauseCounter.isZero()){
        hitPauseCounter.decrement();
        if (hitPauseCounter.getCount() <= GameConstants::HIT_PAUSE_FRAMES){
            return; // this gives the vague "lag" effect
        }
    }

#pragma mark - handle player input

    if (!isDefeat()){
        processPlayerInput();
    }
    else {
        // make sure to stop player from moving
        _level->getPlayer()->getCollider()->setLinearVelocity(Vec2::ZERO);
    }

#pragma mark - Enemy movement
    auto player = _level->getPlayer();
    _AIController.update(dt);
    // enemy attacks
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        auto enemy = *it;
        if (enemy->getHealth() <= 0) {
            //drop health pack
            if (enemy->isEnabled() && rand() % 100 < GameConstants::HEALTHPACK_DROP_RATE) {
                auto healthpack = HealthPack::alloc(enemy->getPosition(), _assets);
                healthpack->setDrawScale(Vec2(_scale, _scale));
                _level->addHealthPack(healthpack);
            }
            //remove enemy
            enemy->setEnabled(false);
            enemy->getAttack()->setEnabled(false);
        }
        if (enemy->isStunned()){
            enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
            enemy->getAttack()->setEnabled(false);
        }
        if (enemy->isEnabled()) {
            // enemy can only begin an attack if not stunned and within range of player and can see them
            bool canBeginNewAttack = !enemy->isAttacking() && enemy->_atkCD.isZero() && !enemy->isStunned();
            if (canBeginNewAttack && enemy->getPosition().distance(player->getPosition()) <= enemy->getAttackRange() && enemy->getPlayerInSight()) {
                if (enemy->getType() == "melee lizard") {
                    enemy->attack(_level, _assets);
                }
                enemy->setAttacking();
            }
            if (enemy->isAttacking()) {
                if (enemy->getType() == "ranged lizard" ||
                    enemy->getType() == "mage alien") {
                    if (enemy->getCharged()) {
                        enemy->attack(_level, _assets);
                    }
                }
            }
        }
    }

#pragma mark - Component Updates
    
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        (*it)->updateCounters();
    }
    std::vector<std::shared_ptr<Projectile>> projs = _level->getProjectiles();
    for (auto it = projs.begin(); it != projs.end(); ++it) {
        (*it)->updateAnimation(dt);
        if ((*it)->isCompleted()) _level->delProjectile((*it));
    }

    std::vector<std::shared_ptr<HealthPack>> hps = _level->getHealthPacks();
    for (auto it = hps.begin(); it != hps.end(); ++it) {
        (*it)->updateAnimation(dt);
        if ((*it)->_delMark) _level->delHealthPack((*it));
    }
    
    // update every animation in game objects
    for (auto& gameobject : _level->getDynamicObjects()){
        gameobject->updateAnimation(dt);
    }
    
    player->update(dt); // updates counters, hitboxes
    _levelTransition.update(dt); // does nothing when not active
    
#pragma mark - Upgrade System
    
    if (_isUpgradeRoom && _level->getRelic() != nullptr){
        auto relic = _level->getRelic();
        _upgrades.setActive(relic->getTouched() && relic->getActive());
        if (_upgrades.isActive() && _upgrades.hasSelectedUpgrade()){
            // get the selected upgrade, apply to player
            float prevMaxHP = player->getMaxHP();
            switch (_upgrades.getChoice()) {
                case UpgradeType::SWORD:
                    player->setMeleeLevel(_upgrades.getUpgradeLevel());
                    break;
                case PARRY:
                    player->setStunLevel(_upgrades.getUpgradeLevel());
                    break;
                case ATK_SPEED:
                    CULog("atk spd");
                    break;
                case DASH:
                    player->setDodgeLevel(_upgrades.getUpgradeLevel());
                    break;
                case UpgradeType::BOW:
                    player->setBowLevel(_upgrades.getUpgradeLevel());
                    break;
                case HEALTH:
                    player->setMaxHPLevel(_upgrades.getUpgradeLevel());
                    player->setHP(player->getHP() + player->getMaxHP() - prevMaxHP);
                    break;
                case SHIELD: case BLOCK:
                    player->setArmorLevel(_upgrades.getUpgradeLevel());
                    player->setBlockLevel(_upgrades.getUpgradeLevel());
                    break;
            }
            // turn off relic
            relic->setActive(false);
        }
    }
}


void GameScene::fixedUpdate(float step) {
    if (_level != nullptr){
        auto player = _level->getPlayer();
        if (player->getHP() == 0){
            // do not update on death
            return;
        }
        if (player->isDodging()) _camController.setAcceleration(GameConstants::GAME_CAMERA_ACCEL);
        else _camController.setAcceleration(GameConstants::GAME_CAMERA_DECEL);
        _camController.setTarget(player->getPosition() * player->getDrawScale());
        
        if (player->getWeapon() == Player::Weapon::RANGED) _camController.setZoomSpeed(GameConstants::GAME_CAMERA_ZOOM_SPEED);
        else _camController.setZoomSpeed(-GameConstants::GAME_CAMERA_ZOOM_SPEED);
        
        _camController.update(step);
        _winNode->setPosition(_camController.getPosition());
        
        if (!hitPauseCounter.isZero()){
            if (hitPauseCounter.getCount() <= GameConstants::HIT_PAUSE_FRAMES){
                return; // this gives the vague "lag" effect (quitting physics update)
            }
        }
        
        _level->getWorld()->update(step);     // Turn the physics engine crank.
        auto enemies = _level->getEnemies();
        for (auto it = enemies.begin(); it != enemies.end(); ++it){
            auto e = *it;
            e->syncPositions();
            e->getAttack()->setPosition(e->getPosition().add(0, 64 / e->getDrawScale().y)); //64 is half of the enemy pixel height
        }
        player->syncPositions();

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

void GameScene::setActive(bool value){
    if (isActive() != value){
        Scene2::setActive(value);
        _exitCode = NONE;
        _gameRenderer.setActivated(value);
        activateInputs(value);
        _levelTransition.setActive(false); // transition should always be off when scene is first on and when game scene is turned off.
        _upgrades.setActive(false); // upgrades is only on by interaction
    }
}

void GameScene::render(const std::shared_ptr<SpriteBatch> &batch){
    Scene2::render(batch); // this is mainly for the debug
    _gameRenderer.render(batch);
    _effectsScene.render(batch);
    if (_upgrades.isActive()){
        _upgrades.render(batch);
    }
    if (_levelTransition.isActive()){
        _levelTransition.render(batch);
    }

}
