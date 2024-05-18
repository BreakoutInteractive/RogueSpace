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
#include "../models/BossEnemy.hpp"
#include "../models/ExplodingAlien.hpp"
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
#include <array>
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
    
    srand((uint32) time(NULL));

    // initalize controllers with the assets
    _assets = assets;
    _parser.loadTilesets(assets);
    _levelNumber = 1;
    MAX_LEVEL = _assets->get<JsonValue>("constants")->getInt("max-level");
    _gameRenderer.init(_assets);
    _gameRenderer.setGameCam(getCamera());
    std::function<bool (Vec2)> preprocessor = [this](Vec2 pos) {
        return _gameRenderer.isInputProcessed(pos) || _upgrades.isInputProcessed(pos);
    };
    _input.init(preprocessor);
    _input.setMinDragRadius(_gameRenderer.getJoystickScreenRadius() / 4);
    activateInputs(false);
    _collisionController.setAssets(_assets);
    
    CameraController::CameraConfig config;
    config.speed = GameConstants::GAME_CAMERA_SPEED;
    config.minSpeed = GameConstants::GAME_CAMERA_SPEED;
    config.maxSpeed = GameConstants::GAME_CAMERA_MAX_SPEED;
    config.maxZoom = GameConstants::GAME_CAMERA_MAX_ZOOM_OUT;
    _camController.init(getCamera(), config);
    
    // necessary (starting at any actual level implies it is not an upgrade room)
    _isUpgradeRoom = false;
    _isTutorial = false;
    _isTutorialComplete = false;
    
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
        if (_isTutorial){
            _isTutorialComplete = true; // this flags allows switching to tutorial menu
            return;
        }
        
        SaveData::Data data;
        auto p = _level->getPlayer();
        bool upgradeAvailable = false;
        if (_isUpgradeRoom){
            // current room was upgrades, just go to the current level number
            _isUpgradeRoom = false;
        }
        else {
            this->_levelNumber+=1;
            this->_isUpgradeRoom = _levelNumber % 3 == 1; // check if an upgrades room should be offered before the next level
            if (_isUpgradeRoom){
                upgradeAvailable = true;
                // generate random new upgrades
                std::pair<std::pair<int, int>, std::pair<int, int>> indices = generateUpgradeIndices(p);
                data.upgradeOpt1 = indices.first.first;
                data.upgradeOpt1Level = indices.first.second;
                data.upgradeOpt2 = indices.second.first;
                data.upgradeOpt2Level = indices.second.second;
            }
        }
        // save game data as level ends
        data.isUpgradeRoom = _isUpgradeRoom;
        data.upgradeAvailable = upgradeAvailable;
        data.level = _levelNumber;
        data.hp = p->getHP();
        data.atkLvl = p->getMeleeUpgrade().getCurrentLevel();
        data.atkSpLvl = p->getMeleeSpeedUpgrade().getCurrentLevel();
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
    _input.dispose();
    _debugNode = nullptr;
    _winNode = nullptr; // TODO: remove
    _level = nullptr;
    _complete = false;
    _defeat = false;
    _debug = false;
    Scene2::dispose();
}

void GameScene::activateTutorial(int level){
    _levelTransition.setActive(false);
    setTutorialActive(true);
    _exitCode = NONE;
    SaveData::Data data;
    data.level = level;
    data.hp = GameConstants::PLAYER_MAX_HP;
    setLevel(data);
}

void GameScene::restart(){
    SaveData::removeSave();
    SaveData::Data data;
    data.level = 1;
    data.isUpgradeRoom = true; // first level is always upgrades
    data.upgradeAvailable = true;
    // pick two random upgrades
    std::pair<std::pair<int, int>, std::pair<int, int>> indices = generateUpgradeIndices(nullptr);
    data.upgradeOpt1 = indices.first.first;
    data.upgradeOpt1Level = indices.first.second;
    data.upgradeOpt2 = indices.second.first;
    data.upgradeOpt2Level = indices.second.second;
    data.hp = GameConstants::PLAYER_MAX_HP;
    setLevel(data);
}

void GameScene::setLevel(SaveData::Data saveData){
    _debugNode->removeAllChildren();
    _winNode->setVisible(false);
    std::string levelToParse;
    auto level = saveData.level;
    _levelNumber = level;
    _isUpgradeRoom = saveData.isUpgradeRoom;
    _upgrades.setActive(false);
    if (_isUpgradeRoom){
        levelToParse = "upgrades";
        _isTutorial = false;
        AudioController::updateMusic("oasis", 1.0f);
    } else if (_isTutorial){
        levelToParse = getLevelKey(_levelNumber);
        AudioController::updateMusic("oasis", 0.0f);
    }
    else{
        levelToParse = getLevelKey(_levelNumber);
        _isTutorial = false;
        AudioController::updateMusic("pursuit", 1.0f);
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
    _gameRenderer.setGameLevel(_level);
    
    auto p = _level->getPlayer();
    p->setMaxHPLevel(saveData.hpLvl);
    p->setMeleeLevel(saveData.atkLvl);
    p->setMeleeSpeedLevel(saveData.atkSpLvl);
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
        if (saveData.upgradeAvailable){
            _level->getRelic()->setActive(true);
            std::array<Upgradeable, 7> upgradeOptions = getPlayerUpgrades(p);
            int idx1 = saveData.upgradeOpt1;
            int idx2 = saveData.upgradeOpt2;
            upgradeOptions[idx1].setCurrentLevel(saveData.upgradeOpt1Level);
            upgradeOptions[idx2].setCurrentLevel(saveData.upgradeOpt2Level);
            _upgrades.updateScene({upgradeOptions[idx1], upgradeOptions[idx2]});
        }
        // turn off the energy walls first
        auto energyWalls = _level->getEnergyWalls();
        for (auto it = energyWalls.begin(); it != energyWalls.end(); ++it) {
            (*it)->deactivate();
        }
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
    _actionManager.remove(AREA_CLEAR_KEY);
    _actionManager.remove(DEAD_EFFECT_KEY);
    _areaClearNode->setVisible(false);
    _deadEffectNode->setVisible(false);
}

std::array<Upgradeable, 7> GameScene::getPlayerUpgrades(std::shared_ptr<Player> player){
    std::array<Upgradeable, 7> upgradeOptions = {
        player->getMeleeUpgrade(), player->getMeleeSpeedUpgrade(), player->getBowUpgrade(), player->getHPUpgrade(), player->getStunUpgrade(), player->getDodgeUpgrade(), player->getDamageReductionUpgrade()};
    return upgradeOptions;
}

std::pair<std::pair<int,int>, std::pair<int,int>> GameScene::generateUpgradeIndices(std::shared_ptr<Player> player){
    std::vector<int> options;
    std::vector<int> levels;
    if (player != nullptr){
        // first find the stats that can be upgraded
        std::array<Upgradeable, 7> upgradeOptions = getPlayerUpgrades(player);
        for (int i = 0; i < 7; i++){
            Upgradeable u = upgradeOptions[i];
            if (!u.isMaxLevel()){
                options.push_back(i);
                levels.push_back(std::min(u.getCurrentLevel() + 1, u.getMaxLevel()));
            }
        }
        
        if (options.size() == 1){
            // duplicate, since only 1 thing to upgrade
            options.push_back(options[0]);
            levels.push_back(levels[0]);
        }
        else if (options.size() == 0){
            // anything will work, nothing to upgrade
            options.push_back(0);
            options.push_back(1);
            levels.push_back(upgradeOptions[0].getCurrentLevel());
            levels.push_back(upgradeOptions[1].getCurrentLevel());
        }
    }
    else {
        options = {0, 1, 2, 3, 4, 5, 6};
        levels = {1, 1, 1, 1, 1, 1, 1};
    }
    int opt1 = 0;
    int opt2 = 1;
    if (options.size() > 2){
        opt1 = std::rand() % options.size();
        opt2 = std::rand() % options.size();
        while (opt2==opt1){
            opt2 = std::rand() % options.size();
        }
    }
    std::pair<int, int> random1(options[opt1], levels[opt1]);
    std::pair<int, int> random2(options[opt2], levels[opt2]);
    std::pair<std::pair<int, int>, std::pair<int, int>> randomOptions(random1, random2);
    return randomOptions;
}

std::vector<int> GameScene::getPlayerLevels(){
    std::vector<int> levels(7, 0);
    if (_level != nullptr){
        auto p = _level->getPlayer();
        levels[0] = p->getMeleeUpgrade().getCurrentLevel();
        levels[1] = p->getBowUpgrade().getCurrentLevel();
        levels[2] = p->getMeleeSpeedUpgrade().getCurrentLevel(); // attack speed
        levels[3] = p->getDamageReductionUpgrade().getCurrentLevel();
        levels[4] = p->getDodgeUpgrade().getCurrentLevel();
        levels[5] = p->getStunUpgrade().getCurrentLevel();
        levels[6] = p->getHPUpgrade().getCurrentLevel();
    }
    return levels;
}

std::string GameScene::getLevelKey(int level){
    if (_isTutorial) {
        return "tutorial"+std::to_string(level);
    }
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
            AudioController::playPlayerFX("dash");
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
                            AudioController::playPlayerFX("attackHit");
                        }
                        break;
                case Player::Weapon::RANGED:
                        AudioController::playPlayerFX("loopBow");
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
                AudioController::clearPlayerFX("loopBow");
                AudioController::playPlayerFX("shootBow");
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
                    std::shared_ptr<Projectile> p = Projectile::playerAlloc(player->getPosition().add(0, 64 / player->getDrawScale().y), player->getBowDamage(), player->isCharged(), ang, _assets);
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
        _levelTransition.setActive(false);
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
                AudioController::updateMusic("strand", 1.0f);
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
        if (_levelNumber < MAX_LEVEL || _isUpgradeRoom){
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

#pragma mark - Enemy AI
    auto player = _level->getPlayer();
    _AIController.update(dt);
    // enemy attacks
    int enemyIndex = 0;
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        auto enemy = *it;
        if (!enemy->isEnabled() || enemy->isDying()) continue;
        if (enemy->getHealth() <= 0) {
            //drop health pack
            AudioController::playEnemyFX("death", enemy->getType());
            if (!enemy->_dropped && rand() % 100 < GameConstants::HEALTHPACK_DROP_RATE) {
                auto healthpack = HealthPack::alloc(enemy->getPosition(), _assets);
                healthpack->setDrawScale(Vec2(_scale, _scale));
                _level->addHealthPack(healthpack);
            }

            if (enemy->getType() == "melee lizard" ||
                enemy->getType() == "tank enemy" ||
                enemy->getType() == "boss enemy") {
                std::shared_ptr<MeleeEnemy> m = std::dynamic_pointer_cast<MeleeEnemy>(enemy);
                m->getAttack()->setEnabled(false);
            }

            if (enemy->isEnabled() && !enemy->isDying()) { 
                enemy->setDying(); 
                enemy->getCollider()->setEnabled(false);
                enemy->getColliderShadow()->setEnabled(false);
            }
            enemy->_dropped = true;
        }
        if (enemy->getType() == "melee lizard" ||
            enemy->getType() == "tank enemy" ||
            enemy->getType() == "boss enemy") {
            std::shared_ptr<MeleeEnemy> m = std::dynamic_pointer_cast<MeleeEnemy>(enemy);
            if (m->isStunned()){
                enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                m->getAttack()->setEnabled(false);
            }
        }
        if (enemy->isEnabled() && !enemy->isDying() && enemy->getHealth() > 0) {
            // boss performs its second attack if already attacking
            if (enemy->getType() == "boss enemy") {
                std::shared_ptr<BossEnemy> boss = std::dynamic_pointer_cast<BossEnemy>(enemy);
                if (boss->secondAttack()) {
                    boss->attack2(_assets);
                    boss->setAttacking2();
                    continue;
                }
            }
            // enemy can only begin an attack if not stunned and within range of player and can see them
            bool canBeginNewAttack = enemy->canBeginNewAttack();
            if (enemy->getType() == "exploding alien"){
                auto explode = std::dynamic_pointer_cast<ExplodingAlien>(enemy);
                if (canBeginNewAttack && enemy->getPosition().distance(player->getPosition()) <= enemy->getAttackRange() && enemy->getPlayerInSight()) {
                    if (explode->canExplode()) {
                        explode->setAttacking();
                    } else {
                        explode->updateWindup(true);
                    }
                } else {
                    explode->updateWindup(false);
                }
                continue;
            }
            if (canBeginNewAttack && enemy->getPosition().distance(player->getPosition()) <= enemy->getAttackRange() && enemy->getPlayerInSight()) {
                if (enemy->getType() == "melee lizard" ||
                    enemy->getType() == "tank enemy" ||
                    enemy->getType() == "boss enemy") {
                    enemy->attack(_level, _assets);
                    AudioController::playEnemyFX("attack", std::to_string(enemyIndex));
                }
                enemy->setAttacking();
            }
            if (enemy->isAttacking()) {
                if (enemy->getType() == "ranged lizard" ||
                    enemy->getType() == "mage alien") {
                    std::shared_ptr<RangedEnemy> r = std::dynamic_pointer_cast<RangedEnemy>(enemy);
                    if (r->getCharged()) {
                        enemy->attack(_level, _assets);
                        AudioController::playEnemyFX("attack", std::to_string(enemyIndex));
                    }
                }
            }
            if (enemy->getType() == "boss enemy") {
                std::shared_ptr<BossEnemy> boss = std::dynamic_pointer_cast<BossEnemy>(enemy);
                if (boss->getStormState() == BossEnemy::StormState::CHARGED) {
                    boss->summonStorm(_level, _assets);
                }
            }
        }
        enemyIndex++;
    }

#pragma mark - Component Updates
    
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        std::shared_ptr<MeleeEnemy> m;
        if ((*it)->getType() == "melee lizard" || (*it)->getType() == "tank enemy"
            || (*it)->getType() == "boss enemy") {
            m = std::dynamic_pointer_cast<MeleeEnemy>(*it);
            m->updateCounters();
        }
        else {
            (*it)->updateCounters();
        }
        if ((*it)->getType() == "boss enemy") {
            std::shared_ptr<BossEnemy> boss = std::dynamic_pointer_cast<BossEnemy>(*it);
            boss->_stormTimer.decrement();
        }
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
    _gameRenderer.update(dt);
    
#pragma mark - Upgrade System
    
    if (_isUpgradeRoom && _level->getRelic() != nullptr){
        auto relic = _level->getRelic();
        _upgrades.setActive(relic->getTouched() && relic->getActive());
        if (_upgrades.isActive() && _upgrades.hasSelectedUpgrade()){
            // get current save, make new save based on selection
            SaveData::Data data = SaveData::getGameSave();
            data.upgradeAvailable = false;
            data.weapon = player->getWeapon();
            data.level = _levelNumber;
            data.isUpgradeRoom = true;
            // get the selected upgrade, apply to player
            float prevMaxHP = player->getMaxHP();
            int newLevel = _upgrades.getUpgradeLevel();
            switch (_upgrades.getChoice()) {
                case UpgradeType::SWORD:
                    player->setMeleeLevel(newLevel);
                    data.atkLvl = newLevel;
                    break;
                case PARRY:
                    player->setStunLevel(newLevel);
                    data.parryLvl = newLevel;
                    break;
                case ATK_SPEED:
                    player->setMeleeSpeedLevel(newLevel);
                    data.atkSpLvl = newLevel;
                    break;
                case DASH:
                    player->setDodgeLevel(newLevel);
                    data.dashLvl = newLevel;
                    break;
                case UpgradeType::BOW:
                    player->setBowLevel(newLevel);
                    data.rangedLvl = newLevel;
                    break;
                case HEALTH:
                    player->setMaxHPLevel(newLevel);
                    player->setHP(player->getHP() + player->getMaxHP() - prevMaxHP);
                    data.hpLvl = newLevel;
                    break;
                case SHIELD: case BLOCK:
                    player->setArmorLevel(_upgrades.getUpgradeLevel());
                    player->setBlockLevel(_upgrades.getUpgradeLevel());
                    data.defLvl = newLevel;
                    break;
            }
            // turn off relic
            relic->setActive(false);
            data.hp = player->getHP();
            SaveData::makeSave(data);
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
            if (e->getType() == "melee lizard"
                || e->getType() == "tank enemy"
                || e->getType() == "boss enemy") {
                std::shared_ptr<MeleeEnemy> m = std::dynamic_pointer_cast<MeleeEnemy>(e);
                m->getAttack()->setPosition(e->getPosition().add(0, 64 / e->getDrawScale().y)); //64 is half of the enemy pixel height
            }
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
        _gameRenderer.setActive(value);
        activateInputs(value);
        _levelTransition.setActive(false); // transition should always be off when scene is first on and when game scene is turned off.
        _upgrades.setActive(false); // upgrades is only on by interaction
    }
}

void GameScene::render(const std::shared_ptr<SpriteBatch> &batch){
    _gameRenderer.render(batch);
    _effectsScene.render(batch);
    if (_upgrades.isActive()){
        _upgrades.render(batch);
    }
    if (_levelTransition.isActive()){
        _levelTransition.render(batch);
    }
    Scene2::render(batch); // this is mainly for the debug
}
