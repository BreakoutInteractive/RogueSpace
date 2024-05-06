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
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>

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

#define NUM_LEVELS_TO_UPGRADE       3

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
    
    // initalize controllers with the assets
    _assets = assets;
    _parser.loadTilesets(assets);
    _levelNumber = 1;
    _upgradeLevelActive = false;
    _gameRenderer.init(_assets);
    _input.init([this](Vec2 pos){
        return _gameRenderer.isInputProcessed(pos);
    });
    _audioController = std::make_shared<AudioController>();
    CameraController::CameraConfig config;
    config.speed = GameConstants::GAME_CAMERA_SPEED;
    config.minSpeed = GameConstants::GAME_CAMERA_SPEED;
    config.maxSpeed = GameConstants::GAME_CAMERA_MAX_SPEED;
    config.maxZoom = GameConstants::GAME_CAMERA_MAX_ZOOM_OUT;
    _camController.init(getCamera(), config);
    _audioController->init(_assets);
    _collisionController.setAssets(_assets, _audioController);
    
    _lvlsToUpgrade.setMaxCount(NUM_LEVELS_TO_UPGRADE);
    _lvlsToUpgrade.reset();
    upgradeScreenActive=false;
    upgradeChosen=false;

    std::shared_ptr<Upgradeable> meleeUpgrade = std::make_shared<Upgradeable>(5, 2, GameConstants::PLAYER_ATK_DAMAGE);
    std::shared_ptr<Upgradeable> parryUpgrade = std::make_shared<Upgradeable>(5, 2, GameConstants::ENEMY_STUN_COOLDOWN);
    std::shared_ptr<Upgradeable> defenseUpgrade =  std::make_shared<Upgradeable>(5, .5, GameConstants::PLAYER_DEFENSE);
    std::shared_ptr<Upgradeable> meleeSpeedUpgrade = std::make_shared<Upgradeable>(5, GameConstants::PLAYER_ATTACK_COOLDOWN/2, GameConstants::PLAYER_ATTACK_COOLDOWN); 
    std::shared_ptr<Upgradeable> dodgeCDUpgrade = std::make_shared<Upgradeable>(5, 5, 1, 1);
    std::shared_ptr<Upgradeable> bowUpgrade = std::make_shared<Upgradeable>(5, 2, GameConstants::PROJ_DAMAGE_P);
    
    playerUpgrades[SWORD] = meleeUpgrade;
    playerUpgrades[PARRY] = parryUpgrade;
    playerUpgrades[SHIELD] = defenseUpgrade;
    playerUpgrades[ATK_SPEED] = meleeSpeedUpgrade;
    playerUpgrades[DASH] = dodgeCDUpgrade;
    playerUpgrades[BOW] = bowUpgrade;
    
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
      
    addChild(_debugNode);  //this we keep
    addChild(_winNode);   //TODO: remove
    addChild(_loseNode); //TODO: remove

    _debugNode->setContentSize(Size(SCENE_WIDTH,SCENE_HEIGHT));
    
    _areaClearEffect = Animation::alloc(SpriteSheet::alloc(assets->get<Texture>("area-clear"), 5, 2), 1.0f, false);
    
    _levelTransition.init(assets);
    _levelTransition.setInitialColor(Color4(255, 255, 255, 0));
    _levelTransition.setFadeIn(GameConstants::TRANSITION_FADE_IN_TIME);
    _levelTransition.setFadeOut(GameConstants::TRANSITION_FADE_OUT_TIME, Color4(255, 255, 255, 0));
    _levelTransition.setFadeInCallBack([this](){
        this->_levelNumber+=1;
        this->setLevel(_levelNumber);
        this->upgradeChosen=false;
    });
  
#pragma mark - Game State Initialization
    setActive(false);
    _complete = false;
    _defeat = false;
    hitPauseCounter.setMaxCount(GameConstants::HIT_PAUSE_FRAMES + 3);
    Application::get()->setClearColor(Color4("#c9a68c"));
    return true;
}

void GameScene::dispose() {
    if (isActive()) {
        _input.dispose();
        _debugNode = nullptr;
        _winNode = nullptr; // TODO: remove
        _loseNode = nullptr; //TODO: remove
        _level = nullptr;
        _complete = false;
        _defeat = false;
        _debug = false;
        Scene2::dispose();
    }
}

void GameScene::restart(){
    _winNode->setVisible(false);
    _lvlsToUpgrade.reset();
    for (int i = 0; i < playerUpgrades.size(); i++){
        playerUpgrades[i]->resetUpgrade();
    } 
    setLevel(1); // reload the first level
    _level->getPlayer()->_hp = _level->getPlayer()->getMaxHP();
}

void GameScene::setLevel(int level){
    _debugNode->removeAllChildren();
    float currentHp = GameConstants::PLAYER_MAX_HP;
    std::string levelToParse = "";
    
    if (_level!=nullptr) {
        currentHp = _level->getPlayer()->_hp;
    }
    
    if (_upgradeLevelActive){
        _levelNumber-=1;
        levelToParse = getLevelKey(_levelNumber);
        _lvlsToUpgrade.reset();
        _upgradeLevelActive=false;
        upgradeChosen=false;
        
    }
    else if (_lvlsToUpgrade.isZero()){
        levelToParse = "upgrades";
        generateRandomUpgrades();
        _upgradeLevelActive=true;
    }
    else{
        _levelNumber = level;
        levelToParse = getLevelKey(_levelNumber);
    }
    
    _lvlsToUpgrade.decrement();
    
    CULog("currLevel %d", _levelNumber);
    auto parsed = _parser.parseTiled(_assets->get<JsonValue>(levelToParse));
    Size dimen = computeActiveSize();

    _level = LevelModel::alloc(_assets->get<JsonValue>("constants"), parsed);
    _level->setAssets(_assets);
        
    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    _scale = dimen.width == SCENE_WIDTH ? dimen.width/_level->getViewBounds().width :
                                              dimen.height/_level->getViewBounds().height;
    _level->setDrawScale(Vec2(_scale, _scale));
    _level->setDebugNode(_debugNode);
    setDebug(isDebug());
    _AIController.init(_level);
    _collisionController.setLevel(_level);
    _gameRenderer.setGameElements(getCamera(), _level);
    
    if (_upgradeLevelActive) {
        _level->getRelic()->setActive(true);
    }
    
    setComplete(false);
    setDefeat(false);
    _input.activateMeleeControls();
    _input.setActive(true);

    auto p = _level->getPlayer();
    _camController.setCamPosition(p->getPosition() * p->getDrawScale());
    setPlayerAttributes(currentHp);
    CULog("level %d", _levelNumber);
    
    // TODO: edit the function later, has temporary side effect: sets the swap button to be down (since player always gets sword) ...
    _gameRenderer.setSwapButtonCallback([this](){
        _level->getPlayer()->swapWeapon();
        _input.swapControlMode();
    });
}

std::string GameScene::getLevelKey(int level){
    return "level"+std::to_string(level);
}


#pragma mark -
#pragma mark Physics Handling

void GameScene::preUpdate(float dt) {
    if (_level == nullptr) {
        return;
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
    
    _input.update(dt);
    
    // Process the toggled key commands
    if (_input.didDebug()) {
        CULog("debug toggled");
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
    
    if (!isComplete() && !isDefeat()){
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
            auto energyWalls = _level->getEnergyWalls();
            for (auto it = energyWalls.begin(); it != energyWalls.end(); ++it) {
                (*it)->deactivate();
            }
            if (_level->getEnemies().size() > 0){
                // no more enemies remain, but there were enemies initially
                _areaClearEffect->reset();
                _areaClearEffect->start();
            }
        }

        if (_level->getPlayer()->_hp == 0) setDefeat(true);
    }
    
    int MAX_LEVEL = 6; // TODO: what defines final victory of a run?
    // level is completed when player successfully exits the room
    if (_level->isCompleted()){
        if (_levelNumber < MAX_LEVEL){
            // begin transitioning to next level
            if (!_levelTransition.isActive()){
                _levelTransition.setActive(true);
            }
        }
        else {
            if (_upgradeLevelActive) {
                _levelNumber+=1;
                setLevel(_levelNumber);
                upgradeChosen=false;
                return;
            }
            _winNode->setVisible(true); // for now
        }
    }

#pragma mark - handle player input

    std::shared_ptr<Player> player = _level->getPlayer();
    Vec2 moveForce = _input.getMoveDirection();
        
    
#ifdef CU_TOUCH_SCREEN
    
    // if player just got hit, cancel any combat requiring hold-times
    if (player->hitCounter.isMaximum()){
        _input.clearHeldGesture();
        _gameRenderer.updateAimJoystick(false, _input.getInitCombatLocation(), _input.getCombatTouchLocation());
    }
    
    if (player->_state == Player::state::CHARGED || player->_state == Player::state::CHARGING){
        _gameRenderer.updateAimJoystick(_input.isCombatActive(), _input.getInitCombatLocation(), _input.getCombatTouchLocation());
    }
    _gameRenderer.updateMoveJoystick(_input.isMotionActive(), _input.getInitTouchLocation(), _input.getTouchLocation());
    
#endif
    
    if (player->_state != Player::state::DODGE && player->getCollider()->isBullet()){
        player->getCollider()->setBullet(false);
    }
    
    // set player direction
    if (moveForce.length() > 0 && player->_state != Player::state::DODGE && !player->isAttacking()){
        player->setFacingDir(moveForce);
    }

    // Priority order: Dodge, Attack/Shoot,  Parry
    if (player->_state == Player::state::IDLE || player->_state == Player::state::CHARGING || player->_state == Player::state::CHARGED
        || player->_state == Player::state::PARRYSTART || player->_state == Player::state::PARRYSTANCE) {
        //for now, give highest precedence to dodge
        int dashExpense = round(player->dodgeCD.getMaxCount()*(float)1/playerUpgrades[DASH]->getCurrentValue());
        if (_input.didDodge() && player->dodgeCD.getCount()+dashExpense<=player->dodgeCD.getMaxCount()) {
            //count=0 Max = 120 dash full, reset starts countdown at max to 0
            player->dodgeCD.setCount(player->dodgeCD.getCount()+dashExpense); 
            //dodge
            auto force = _input.getDodgeDirection(player->getFacingDir());
            if (force.length() == 0) {
                // dodge in the direction currently facing. normalize so that the dodge is constant speed
                force = player->getFacingDir().getNormalization();
            }
            Vec2 velocity = force * GameConstants::PLAYER_DODGE_SPEED;
            player->getCollider()->setLinearVelocity(velocity);
            player->getCollider()->setBullet(true);
            player->setFacingDir(force);
            player->_state = Player::state::DODGE;
        }
        else if (player->_state != Player::state::DODGE && player->_state != Player::state::RECOVERY) { //not dodging or recovering
            //for now, give middle precedence to attack
            if (_input.didAttack()) {
                Vec2 direction = Vec2::ZERO;
                float ang = 0;
                switch(player->_weapon){
                case Player::weapon::MELEE:
                    direction = _input.getAttackDirection(player->getFacingDir());
                    ang = acos(direction.dot(Vec2::UNIT_X));
                    if (direction.y < 0) {
                        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
                        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
                    }
                    player->enableMeleeAttack(ang);
                    player->animateAttack();
                    player->atkCD.reset();
                    break;
                case Player::weapon::RANGED:
                    player->animateCharge();
                    player->getCollider()->setLinearVelocity(Vec2::ZERO);
                    break;
                }
            }
            else if (_input.didCharge()) {
                Vec2 direction = _input.getAttackDirection(player->getFacingDir());
                if (player->_weapon == Player::weapon::RANGED && (player->_state == Player::state::CHARGING || player->_state == Player::state::CHARGED)) {
                    // this lets you rotate the player while holding the bow in charge mode
                    player->setFacingDir(direction);
                }
            }
            else if (_input.didShoot()) {
                Vec2 direction = Vec2::ZERO;
                float ang = 0;
                if (player->_weapon == Player::weapon::RANGED) {
                    //only shoot when fully charged
                    if (player->_state == Player::state::CHARGED) {
                        //ranged attack
                        direction = _input.getAttackDirection(player->getFacingDir());
                        ang = acos(direction.dot(Vec2::UNIT_X));
                        if (direction.y < 0) {
                            // handle downwards case, rotate counterclockwise by PI rads and add extra angle
                            ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
                        }
                        std::shared_ptr<Projectile> p = Projectile::playerAlloc(player->getPosition().add(0, 64 / player->getDrawScale().y), player->bowDamage, ang, _assets);
                        p->setDrawScale(Vec2(_scale, _scale));
                        _level->addProjectile(p);
                        player->animateShot();
                    }
                    else player->animateDefault();
                }
            }
            //for now, give lowest precendence to parry. only allow it with the melee weapon
            else if (_input.didParry() && player->_weapon == Player::weapon::MELEE) {
                player->animateParryStart();
            }
            else if (_input.didParryRelease() && player->_weapon == Player::weapon::MELEE) {
                if (player->_state == Player::state::PARRYSTANCE) { //maybe allow parry during PARRYSTART state?
                    CULog("parried");
                    player->animateParry();
                }
                else player->animateDefault();
            }
        }
    }
    
    // disable the swap button based on player state
    _gameRenderer.setSwapButtonActive(player->_state == Player::state::IDLE || player->_state == Player::state::DODGE);

    // TODO: could remove, this is PC-only
    if (_input.didSwap()){
        if (player->_state == Player::state::IDLE || player->_state == Player::state::DODGE){
            //other states are weapon-dependent, so don't allow swapping while in them
            player->swapWeapon();
            _input.swapControlMode(); // must do for mobile controls
        }
    }
    
    //only move if we're not parrying or dodging or recovering
    if (player->_state != Player::state::PARRY && player->_state != Player::state::PARRYSTART && player->_state != Player::state::PARRYSTANCE
        && player->_state != Player::state::DODGE && player->_state != Player::state::RECOVERY
        && (player->hitCounter.getCount() < player->hitCounter.getMaxCount() - 5)) {
        switch (player->_weapon) {
        case Player::weapon::MELEE:
            if (player->isAttacking()){
                player->getCollider()->setLinearVelocity(moveForce * GameConstants::PLAYER_ATK_MOVE_SPEED);
            }
            else {
                player->getCollider()->setLinearVelocity(moveForce * player->getMoveScale());
            }
            break;
        case Player::weapon::RANGED:
            //with the ranged weapon, dont move while attacking
            if (!player->isAttacking()){
                player->getCollider()->setLinearVelocity(moveForce * player->getMoveScale());
            }
            else {
                player->getCollider()->setLinearVelocity(Vec2::ZERO);
            }
            break;
        }
        
    } else if (player->_state != Player::state::DODGE && (player->hitCounter.getCount() < player->hitCounter.getMaxCount() - 5)) {
        player->getCollider()->setLinearVelocity(Vec2::ZERO);
    }

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
                if (enemy->getType() == "melee lizard") {
                    enemy->attack(_level, _assets);
                }
                enemy->setAttacking();
            }
            if (enemy->getState() == Enemy::EnemyState::ATTACKING) {
                if (enemy->getType() == "ranged lizard" || 
                    enemy->getType() == "mage alien") {
                    if (enemy->getCharged()) {
                        enemy->attack(_level, _assets);
                    }
                }
            }
        }
        
    }
    if (_level->getRelic()!=nullptr && !upgradeChosen){
        upgradeScreenActive =_level->getRelic()->getTouched();
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
    
    // update every animation in game objects
    for (auto& gameobject : _level->getDynamicObjects()){
        gameobject->updateAnimation(dt);
    }
    
    player->update(dt); // updates counters, hitboxes
    _areaClearEffect->update(dt); // does nothing when not active
    _levelTransition.update(dt); // also does nothing when not active
}


void GameScene::fixedUpdate(float step) {
    if (_level != nullptr){
        auto player = _level->getPlayer();
        if (player->_state == Player::state::DODGE){
            _camController.setAcceleration(GameConstants::GAME_CAMERA_ACCEL);
        }
        else {
            _camController.setAcceleration(GameConstants::GAME_CAMERA_DECEL);
        }
        _camController.setTarget(player->getPosition() * player->getDrawScale());
        
        // use conditional if (player->_state == Player::state::CHARGED || player->_state == Player::state::CHARGING) in the event of restricted zoom
        if (player->_weapon == Player::weapon::RANGED){
            _camController.setZoomSpeed(GameConstants::GAME_CAMERA_ZOOM_SPEED);
        }
        else {
            _camController.setZoomSpeed(-GameConstants::GAME_CAMERA_ZOOM_SPEED);
        }
        
        _camController.update(step);
        _winNode->setPosition(_camController.getPosition());
        _loseNode->setPosition(_camController.getPosition());
        
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

void GameScene::generateRandomUpgrades(){
    int displayedAttribute1 = std::rand()%playerUpgrades.size();
    upgradesForLevel[0] = displayedAttribute1;

    int displayedAttribute2 = std::rand()%playerUpgrades.size();
    while (displayedAttribute2==displayedAttribute1){
        displayedAttribute2 =std::rand()%playerUpgrades.size();
    }
    upgradesForLevel[1] = displayedAttribute2;
}

void GameScene::updatePlayerAttributes(int selectedAttribute){
    switch (selectedAttribute) {
        case SWORD:
            playerUpgrades[selectedAttribute]->levelUp();
            _level->getPlayer()->meleeDamage = playerUpgrades[selectedAttribute]->getCurrentValue();
            break;
        case PARRY: //unimplemented
            break;
        case SHIELD:
            playerUpgrades[selectedAttribute]->levelUp();
            _level->getPlayer()->defense = playerUpgrades[selectedAttribute]->getCurrentValue();
            break;
        case ATK_SPEED: //unimplemented
            break;
        case DASH:
            playerUpgrades[selectedAttribute]->levelUp();
            _level->getPlayer()->dodgeCD.setMaxCount(playerUpgrades[selectedAttribute]->getCurrentValue());
            break;
        case BOW:
            playerUpgrades[selectedAttribute]->levelUp();
            _level->getPlayer()->bowDamage = playerUpgrades[selectedAttribute]->getCurrentValue();
            break;
        default:
            _level->getPlayer()->_hp = _level->getPlayer()->getMaxHP();
    }
}

void GameScene::setPlayerAttributes(float hp){
    _level->getPlayer()->_hp = hp;
    _level->getPlayer()->meleeDamage = playerUpgrades[SWORD]->getCurrentValue();
    _level->getPlayer()->parryWindow = playerUpgrades[PARRY]->getCurrentValue(); //unimplemented
    _level->getPlayer()->defense = playerUpgrades[SHIELD]->getCurrentValue();
//    _level->getPlayer()->atkSpeed = atkCD.setMaxCount(playerUpgrades[ATK_SPEED]->getCurrentValue());
    _level->getPlayer()->dodgeCD.setMaxCount(GameConstants::PLAYER_DODGE_COOLDOWN*playerUpgrades[DASH]->getCurrentValue());
    _level->getPlayer()->bowDamage = playerUpgrades[BOW]->getCurrentValue();
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

void GameScene::render(const std::shared_ptr<SpriteBatch> &batch){
    _gameRenderer.render(batch);
    if (_areaClearEffect->isActive()){
        batch->begin(_camera->getCombined());
        auto sheet = _areaClearEffect->getSpriteSheet();
        Size frameSize = sheet->getFrameSize();
        // take up around half the screen height
        float verticalScale = 0.35f * Application::get()->getDisplaySize().height / frameSize.height;
        float horizontalScale = 0.35f * Application::get()->getDisplaySize().width / frameSize.width;
        Affine2 transform = Affine2::createScale(std::min(horizontalScale, verticalScale));
        transform.translate(_camera->getPosition());
        sheet->draw(batch, frameSize/2, transform);
        batch->end();
    }
    Scene2::render(batch);
    if (_levelTransition.isActive()){
        _levelTransition.render(batch);
    }
}
