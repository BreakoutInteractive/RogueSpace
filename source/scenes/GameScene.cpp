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
#include "../components/Animation.hpp"

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
    
    // initalize controllers with the assets
    _assets = assets;
    _parser.loadTilesets(assets);
    _levelNumber = 0;
    _upgradeLevelActive = false;
    _gameRenderer.init(_assets);
    _input.init([this](Vec2 pos){
        return _gameRenderer.isInputProcessed(pos);
    });
    _audioController = std::make_shared<AudioController>();
    _audioController->init(_assets);
    _collisionController.setAssets(_assets, _audioController);
    
    CameraController::CameraConfig config;
    config.speed = GameConstants::GAME_CAMERA_SPEED;
    config.minSpeed = GameConstants::GAME_CAMERA_SPEED;
    config.maxSpeed = GameConstants::GAME_CAMERA_MAX_SPEED;
    config.maxZoom = GameConstants::GAME_CAMERA_MAX_ZOOM_OUT;
    _camController.init(getCamera(), config);
    
    _lvlsToUpgrade.setMaxCount(NUM_LEVELS_TO_UPGRADE);
    _lvlsToUpgrade.reset();
    upgradeScreenActive=false;
    upgradeChosen=false;

    std::shared_ptr<Upgradeable> meleeUpgrade = std::make_shared<Upgradeable>(5, 2, GameConstants::PLAYER_ATK_DAMAGE);
    std::shared_ptr<Upgradeable> parryUpgrade = std::make_shared<Upgradeable>(5, 2, GameConstants::PLAYER_ATK_DAMAGE); //placeholder
    std::shared_ptr<Upgradeable> defenseUpgrade =  std::make_shared<Upgradeable>(5, .5, GameConstants::PLAYER_DEFENSE);
    std::shared_ptr<Upgradeable> meleeSpeedUpgrade = std::make_shared<Upgradeable>(5, 2, GameConstants::PLAYER_ATK_DAMAGE); //placeholder
    std::shared_ptr<Upgradeable> dodgeCDUpgrade = std::make_shared<Upgradeable>(5, 30, GameConstants::PLAYER_DODGE_COOLDOWN);
    std::shared_ptr<Upgradeable> bowUpgrade = std::make_shared<Upgradeable>(5, 2, GameConstants::PROJ_DAMAGE_P);
    
    availableUpgrades.push_back(std::move(meleeUpgrade));
    availableUpgrades.push_back(std::move(parryUpgrade));
    availableUpgrades.push_back(std::move(defenseUpgrade));
    availableUpgrades.push_back(std::move(meleeSpeedUpgrade));
    availableUpgrades.push_back(std::move(dodgeCDUpgrade));
    availableUpgrades.push_back(std::move(bowUpgrade));

    
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
        this->_levelNumber+=1;
        this->setLevel(_levelNumber);
        this->upgradeChosen=false;
    });
    
  
#pragma mark - Game State Initialization
    setActive(false);
    setComplete(false);
    setDefeat(false);
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
    _lvlsToUpgrade.reset();
    for (auto it = availableUpgrades.begin(); it != availableUpgrades.end(); ++it){
        (*it)->resetUpgrade();
    }
    setLevel(0); // reload the first level
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
        upgradesForLevel.clear();
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
    
    // hide effect nodes
    _areaClearNode->setVisible(false);
    _deadEffectNode->setVisible(false);
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
    
    _actionManager.update(dt);
    _areaClearNode->setVisible(_actionManager.isActive(AREA_CLEAR_KEY));
    _deadEffectNode->setVisible(_actionManager.isActive(DEAD_EFFECT_KEY));
    
    if (!isComplete() && !isDefeat()){
        // game not won, check if any enemies active
        int activeCount = 0;
        auto enemies = _level->getEnemies();
        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            if ((*it)->getCollider()->isEnabled()) {
                activeCount += 1;
            }
        }
        // player dies
        if (_level->getPlayer()->_hp == 0){
            setDefeat(true);
            // start playing dead effect
            _actionManager.remove(DEAD_EFFECT_KEY);
            _actionManager.activate(DEAD_EFFECT_KEY, _deadEffectAnimation, _deadEffectNode);
        }
        // player finishes current level
        else if (activeCount == 0){
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
        if (_input.didDodge() && player->dodgeCD.isZero()) {
            player->dodgeCD.reset(); // reset cooldown
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
                    if (player->_state == Player::state::CHARGED || player->_state == Player::state::CHARGING) {
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
        && (player->hitCounter.getCount() < player->hitCounter.getMaxCount() - 10)) {
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
        CULog("%f", enemy->getHealth());
        if (enemy->getHealth() <= 0) {
            if (enemy->getType() == "exploding alien" && !enemy->getCharged()) {
                enemy->setAttacking();
            } else {
                enemy->setEnabled(false);
                enemy->getAttack()->setEnabled(false);
            }
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
                    enemy->getType() == "mage alien" ) {
                    if (enemy->getCharged()) {
                        enemy->attack(_level, _assets);
                    }
                }
            }
            if (enemy->getType() == "exploding alien") {
                if (enemy->getCharged()) {
                    enemy->setHealth(0);
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
    int displayedAttribute1 = std::rand()%availableUpgrades.size();
    upgradesForLevel.push_back(displayedAttribute1);

    int displayedAttribute2 = std::rand()%availableUpgrades.size();
    while (displayedAttribute2==displayedAttribute1){
        displayedAttribute2 =std::rand()%availableUpgrades.size();
    }
    upgradesForLevel.push_back(displayedAttribute2);
}

void GameScene::updatePlayerAttributes(int selectedAttribute){
    switch (selectedAttribute) {
        case SWORD:
            availableUpgrades.at(selectedAttribute)->levelUp();
            _level->getPlayer()->meleeDamage = availableUpgrades.at(selectedAttribute)->getCurrentValue();
            break;
        case PARRY: //unimplemented
//            availableUpgrades.at(selectedAttribute)->levelUp();
//            _level->getPlayer()->dodgeCD.setMaxCount(availableUpgrades.at(selectedAttribute)->getCurrentValue());
            break;
        case SHIELD:
            availableUpgrades.at(selectedAttribute)->levelUp();
            _level->getPlayer()->defense = availableUpgrades.at(selectedAttribute)->getCurrentValue();
            break;
        case ATK_SPEED: //unimplemented
//            availableUpgrades.at(selectedAttribute)->levelUp();
//            _level->getPlayer()->meleeDamage = availableUpgrades.at(selectedAttribute)->getCurrentValue();
            break;
        case DASH:
            availableUpgrades.at(selectedAttribute)->levelUp();
            _level->getPlayer()->dodgeCD.setMaxCount(availableUpgrades.at(selectedAttribute)->getCurrentValue());
            break;
        case BOW:
            availableUpgrades.at(selectedAttribute)->levelUp();
            _level->getPlayer()->bowDamage = availableUpgrades.at(selectedAttribute)->getCurrentValue();
            break;
        default:
            _level->getPlayer()->_hp = _level->getPlayer()->getMaxHP();
    }
}

void GameScene::setPlayerAttributes(float hp){
    _level->getPlayer()->_hp = hp;
    _level->getPlayer()->meleeDamage = availableUpgrades.at(SWORD)->getCurrentValue();
    _level->getPlayer()->parryWindow = availableUpgrades.at(PARRY)->getCurrentValue(); //unimplemented
    _level->getPlayer()->defense = availableUpgrades.at(SHIELD)->getCurrentValue();
    _level->getPlayer()->atkSpeed = (availableUpgrades.at(ATK_SPEED)->getCurrentValue()); //unimplemented
    _level->getPlayer()->dodgeCD.setMaxCount(availableUpgrades.at(DASH)->getCurrentValue());
    _level->getPlayer()->bowDamage = availableUpgrades.at(BOW)->getCurrentValue(); 
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
    _effectsScene.render(batch);
    if (_levelTransition.isActive()){
        _levelTransition.render(batch);
    }
    Scene2::render(batch); // this is mainly for the debug
}
