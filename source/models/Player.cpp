//
//  Player.cpp
//
//  The player collision-box is represented by a square box rotated on a 45 degree angle.
//  RS
//
//  Created by Zhiyuan Chen on 2/23/24.
//

#include "Player.hpp"
#include "CollisionConstants.hpp"
#include "GameObject.hpp"
#include "GameConstants.hpp"
#include "../components/Animation.hpp"
#include "../components/Collider.hpp"
#include "Upgradeable.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool Player::init(std::shared_ptr<JsonValue> playerData) {
    _weapon = MELEE;
    _state = IDLE;
    _dodge = 0;
    _combo = 1;
    _comboTimer = 0;
    _position.set(playerData->getFloat("x"), playerData->getFloat("y"));
    std::shared_ptr<JsonValue> colliderData = playerData->get("collider");
    // set up collider
    auto collider = Collider::makePolygon(colliderData, b2_dynamicBody, "player-collider");
    // this is a player and can collide with an enemy "shadow", wall, or attack
    b2Filter filter;
    filter.categoryBits = CATEGORY_PLAYER;
    filter.maskBits = CATEGORY_ENEMY_SHADOW | CATEGORY_TALL_WALL | CATEGORY_SHORT_WALL | CATEGORY_ATTACK | CATEGORY_PROJECTILE;
    collider->setFilterData(filter);
    _collider = collider;                   // attach Component
    
    // set the player collider-shadow
    auto colliderShadow = Collider::makePolygon(colliderData, b2_kinematicBody, "player-collider-shadow");
    colliderShadow->setBodyType(b2_kinematicBody);
    filter.categoryBits = CATEGORY_PLAYER_SHADOW;
    filter.maskBits = CATEGORY_ENEMY;
    colliderShadow->setFilterData(filter);
    _colliderShadow = colliderShadow;       // attach Component
    _colliderShadow->setDebugColor(Color4::BLACK);
    
    // set the player hurtbox sensor
    std::shared_ptr<JsonValue> hitboxData = playerData->get("hitbox");
    auto hitbox = Collider::makeCollider(hitboxData, b2_kinematicBody, "player-hitbox", true);
    filter.categoryBits = CATEGORY_PLAYER_HITBOX;
    filter.maskBits = CATEGORY_ATTACK | CATEGORY_PROJECTILE;
    hitbox->setFilterData(filter);
    _sensor = hitbox;
    _sensor->setDebugColor(Color4::RED);
    
    // set up melee hitbox
    _meleeHitbox = PlayerHitbox::alloc(Vec2::ZERO, GameConstants::PLAYER_MELEE_ATK_RANGE);
    _meleeHitbox->setSensor(true);
    // this is an attack and, since it is the player's, can collide with enemies
    filter.categoryBits = CATEGORY_ATTACK;
    filter.maskBits = CATEGORY_ENEMY | CATEGORY_ENEMY_HITBOX;
    _meleeHitbox->setFilterData(filter);
    
    // set the counter properties
    hitCounter.setMaxCount(GameConstants::PLAYER_IFRAME);
    // TODO: possibly deprecated, remove.
    atkCD.setMaxCount(GameConstants::PLAYER_ATTACK_COOLDOWN);
    dodgeCD.setMaxCount(GameConstants::PLAYER_DODGE_COOLDOWN);
    _hp = GameConstants::PLAYER_MAX_HP;
    
    defenseUpgrade  =  std::make_shared<Upgradeable>(10, .5, GameConstants::PLAYER_DEFENSE, "DEFENSE");
    attackUpgrade  = std::make_shared<Upgradeable>(10, 2, GameConstants::PLAYER_ATK_DAMAGE, "ATTACK");
    dodgeUpgrade = std::make_shared<Upgradeable>(10, 30, GameConstants::PLAYER_DODGE_COOLDOWN, "DODGECD");
    attributes.push_back(attackUpgrade);
    attributes.push_back(defenseUpgrade);
    attributes.push_back(dodgeUpgrade);
    _moveScale = GameConstants::PLAYER_MOVE_SPEED;

    
    // initialize directions
    _directions[0] = Vec2(0,-1);    //down
    _directions[2] = Vec2(1,0);     //right
    _directions[4] = Vec2(0,1);     //up
    _directions[6] = Vec2(-1,0);    //left
    for (int i = 1; i < 8; i+=2){
        _directions[i] = Vec2(0,-1).rotate(M_PI_4 * i); // diagonal directions
    }
    _directionIndex = 0;
    _facingDirection = _directions[0];
    return true;
}

void Player::dispose() {
    // nothing to clean
}

void Player::applyUpgrade(std::string upgrade){
    if (upgrade=="ATTACK"){
        attackUpgrade->levelUp();
    } else if (upgrade=="DEFENSE"){
        defenseUpgrade->levelUp();
    } else if (upgrade=="DODGECD"){
        dodgeUpgrade->levelUp();
        dodgeCD.setMaxCount(dodgeUpgrade->getCurrentValue());
    }
    
}


#pragma mark -
#pragma mark Properties

int Player::getMaxHP(){
    return GameConstants::PLAYER_MAX_HP;
}

int Player::getMoveScale(){
    return GameConstants::PLAYER_MOVE_SPEED;
}

bool Player::isAttacking() {
    return _state == CHARGING || _state == CHARGED || _state == SHOT || _state == ATTACK;
}

#pragma mark -
#pragma mark Animation

void Player::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render player with appropriate scales (right now default size)
    auto spriteSheet = _currAnimation->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2::createTranslation(_position * _drawScale);
    spriteSheet->draw(batch, _tint, origin, transform);
    
    //effects
    std::shared_ptr<SpriteSheet> sheet;
    Vec2 o = Vec2::ZERO;
    Affine2 t = Affine2::ZERO;
    Vec2 direction = Vec2::ZERO;
    float ang = 0;
    switch (_state) {
    case DODGE:
        // render player differently while dodging (add fading effect)
        for (int i = 2; i < 10; i += 2) {
            auto color = Color4(Vec4(1, 1, 1, 1 - i * 0.1));
            Affine2 localTrans = Affine2::createTranslation((_position - _collider->getLinearVelocity() * (i * 0.01)) * _drawScale);
            spriteSheet->draw(batch, color, origin, localTrans);
        }
        break;
    case CHARGED:
        sheet = _chargedEffect->getSpriteSheet();
        o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
        direction = getFacingDir();
        ang = acos(direction.dot(Vec2::UNIT_X));
        if (direction.y < 0) {
            // handle downwards case, rotate counterclockwise by PI rads and add extra angle
            ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
        }
        t = Affine2::createRotation(ang);
        t.translate(getPosition().add(0, 64 / getDrawScale().y) * _drawScale);
        sheet->draw(batch, o, t);
        break;
    case CHARGING:
        sheet = _chargingEffect->getSpriteSheet();
        o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
        direction = getFacingDir();
        ang = acos(direction.dot(Vec2::UNIT_X));
        if (direction.y < 0) {
            // handle downwards case, rotate counterclockwise by PI rads and add extra angle
            ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
        }
        t = Affine2::createRotation(ang);
        t.translate(getPosition().add(0, 64 / getDrawScale().y) * _drawScale);
        sheet->draw(batch, o, t);
        break;
    case SHOT:
        sheet = _shotEffect->getSpriteSheet();
        o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
        direction = getFacingDir();
        ang = acos(direction.dot(Vec2::UNIT_X));
        if (direction.y < 0) {
            // handle downwards case, rotate counterclockwise by PI rads and add extra angle
            ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
        }
        t = Affine2::createRotation(ang);
        t.translate(getPosition().add(0, 64 / getDrawScale().y) * _drawScale);
        if (_shotEffect->isActive()) sheet->draw(batch, o, t);
        break;
    default:
        break;
    }
    if (_parryEffect->isActive()) { 
        std::shared_ptr<SpriteSheet> effSheet = _parryEffect->getSpriteSheet();
        transform.translate(0, spriteSheet->getFrameSize().height/2);
        origin = Vec2(effSheet->getFrameSize().width / 2, effSheet->getFrameSize().height / 2);
        effSheet->draw(batch, origin, transform); 
    }
    
    if (_swipeEffect->isActive() || _comboSwipeEffect->isActive()){
        sheet = _comboSwipeEffect->isActive() ? _comboSwipeEffect->getSpriteSheet() : _swipeEffect->getSpriteSheet();
        Affine2 atkTrans = Affine2::createScale(GameConstants::PLAYER_MELEE_ATK_RANGE / ((Vec2)sheet->getFrameSize() / 2) * getDrawScale());
        //we subtract pi/2 from the angle since the animation is pointing up but the hitbox points right by default
        atkTrans.rotate(_meleeHitbox->getAngle() - M_PI_2);
        atkTrans.translate(_meleeHitbox->getPosition() * _drawScale);
        sheet->draw(batch, Color4::WHITE, Vec2(sheet->getFrameSize().getIWidth() / 2, 0), atkTrans);
    }
}

void Player::loadAssets(const std::shared_ptr<AssetManager> &assets){
    auto meleeIdleTexture = assets->get<Texture>("player-idle");
    auto parryTexture = assets->get<Texture>("player-parry");
    auto attackTexture = assets->get<Texture>("player-attack");
    auto runTexture = assets->get<Texture>("player-run");
    auto bowIdleTexture = assets->get<Texture>("player-bow-idle");
    auto rangedTexture = assets->get<Texture>("player-ranged");
    auto bowRunTexture = assets->get<Texture>("player-bow-run");
    auto projEffectTexture = assets->get<Texture>("player-projectile");
    auto parryEffectTexture = assets->get<Texture>("parry-effect");
    auto meleeSwipeEffect = assets->get<Texture>("player-swipe");
    auto meleeComboSwipeEffect = assets->get<Texture>("player-swipe-combo");
    
    // make sheets
    auto parrySheet = SpriteSheet::alloc(parryTexture, 8, 16);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 24);
    auto idleSheet = SpriteSheet::alloc(meleeIdleTexture, 8, 8);
    auto runSheet = SpriteSheet::alloc(runTexture, 8, 16);
    auto rangedSheet = SpriteSheet::alloc(rangedTexture, 8, 16);
    auto bowIdleSheet = SpriteSheet::alloc(bowIdleTexture, 8, 8);
    auto bowRunSheet = SpriteSheet::alloc(bowRunTexture, 8, 16);
    auto projEffectSheet = SpriteSheet::alloc(projEffectTexture, 4, 4);
    auto parryEffectSheet = SpriteSheet::alloc(parryEffectTexture, 2, 4);
    auto swipeEffectSheet = SpriteSheet::alloc(meleeSwipeEffect, 1, 6);
    auto comboSwipeEffectSheet = SpriteSheet::alloc(meleeComboSwipeEffect, 1, 6);

    // pass to animations
    _parryStartAnimation = Animation::alloc(parrySheet, 0.1f, false, 0, 1);
    _parryStanceAnimation = Animation::alloc(parrySheet, 0.5f, true, 2, 9);
    _parryAnimation = Animation::alloc(parrySheet, GameConstants::PLAYER_PARRY_TIME, false, 10, 15);
    _attackAnimation1 = Animation::alloc(attackSheet, 0.3f, false, 0, 7);
    _attackAnimation2 = Animation::alloc(attackSheet, 0.3f, false, 8, 13);
    _attackAnimation3 = Animation::alloc(attackSheet, 0.5f, false, 14, 23);
    _runAnimation = Animation::alloc(runSheet, 0.667f, true, 0, 15);
    _idleAnimation = Animation::alloc(idleSheet, 1.2f, true, 0, 7);
    _chargingAnimation = Animation::alloc(rangedSheet, GameConstants::CHARGE_TIME, false, 0, 8);
    _chargedAnimation = Animation::alloc(rangedSheet, 0.1f, true, 8, 8);
    _shotAnimation = Animation::alloc(rangedSheet, 0.125f, false, 9, 11);
    _recoveryAnimation = Animation::alloc(rangedSheet, 0.167f, false, 12, 15);
    _bowRunAnimation = Animation::alloc(bowRunSheet, 0.667f, true, 0, 15);
    _bowIdleAnimation = Animation::alloc(bowIdleSheet, 1.2f, true, 0, 7);
    _chargingEffect = Animation::alloc(projEffectSheet, GameConstants::CHARGE_TIME, false, 0, 3);
    _chargedEffect = Animation::alloc(projEffectSheet, 0.333f, true, 4, 7);
    _shotEffect = Animation::alloc(projEffectSheet, 1/24.0f, false, 8, 8);
    _parryEffect = Animation::alloc(parryEffectSheet, 0.4f, false);
    _swipeEffect = Animation::alloc(swipeEffectSheet, 0.3f, false); // match attack 1 and 2
    _comboSwipeEffect = Animation::alloc(comboSwipeEffectSheet, 0.5f, false); // match attack 3
    
    // add callbacks
    _attackAnimation1->onComplete([this](){
        _attackAnimation1->reset();
        _comboTimer = 0;
        if (!_meleeHitbox->hitFlag) _combo = 1;
        _state = IDLE;
        });
    _attackAnimation2->onComplete([this]() {
        _attackAnimation2->reset();
        _comboTimer = 0;
        if (!_meleeHitbox->hitFlag) _combo = 1;
        _state = IDLE;
        });
    _attackAnimation3->onComplete([this]() {
        _attackAnimation3->reset();
        _comboTimer = 0;
        if (!_meleeHitbox->hitFlag) _combo = 1;
        _state = IDLE;
        });
    _parryStartAnimation->onComplete([this]() {
        _parryStartAnimation->reset();
        setAnimation(_parryStanceAnimation);
        _parryStanceAnimation->start();
        _state = PARRYSTANCE;
        });
    _parryAnimation->onComplete([this](){
        _parryAnimation->reset();
        _state = IDLE;
        });
    _shotAnimation->onComplete([this]() {
        _shotAnimation->reset();
        _shotEffect->reset();
        setAnimation(_recoveryAnimation);
        _recoveryAnimation->start();
        _state = RECOVERY;
        });
    _recoveryAnimation->onComplete([this]() {
        _recoveryAnimation->reset();
        _state = IDLE;
        });
    _chargingAnimation->onComplete([this]() {
        setAnimation(_chargedAnimation);
        _chargedAnimation->start();
        _chargingAnimation->reset();
        });
    _chargingEffect->onComplete([this]() {
        _chargingEffect->reset();
        _chargedEffect->start();
        _state = CHARGED;
        });

    setAnimation(_idleAnimation);
}

void Player::animateParryStart() {
    setAnimation(_parryStartAnimation);
    _state = PARRYSTART;
}
void Player::animateParry() {
    setAnimation(_parryAnimation);
    _state = PARRY;
}

void Player::playParryEffect() {
    _parryEffect->reset();
    _parryEffect->start();
}

void Player::animateDefault() {
    _state = IDLE;
    switch (_weapon) {
    case MELEE:
        setAnimation(_idleAnimation);
        break;
    case RANGED:
        setAnimation(_bowIdleAnimation);
        break;
    }
    _chargingEffect->reset();
    _chargedEffect->reset();
    _shotEffect->reset();
}

void Player::animateAttack() {
    _state = ATTACK;
    _prevAnimation = _currAnimation;
    if (_combo==1) setAnimation(_attackAnimation1);
    else if (_combo == 2) setAnimation(_attackAnimation2);
    else if (_combo == 3) setAnimation(_attackAnimation3);
    else setAnimation(_attackAnimation1); //should never reach this line
    
    if (_combo == 3){
        _swipeEffect->reset();
        _comboSwipeEffect->reset();
        _comboSwipeEffect->start();
    }
    else {
        _comboSwipeEffect->reset();
        _swipeEffect->reset();
        _swipeEffect->start();
    }
}

void Player::animateCharge() {
    setAnimation(_chargingAnimation);
    _chargingEffect->start();
    _state = CHARGING;
}

void Player::animateShot() {
    setAnimation(_shotAnimation);
    _chargedEffect->reset();
    _shotEffect->start();
    _state = SHOT;
}

void Player::setAnimation(std::shared_ptr<Animation> animation){
    if (_currAnimation != animation){
        // MAYBE, we don't want to reset ?? (tweening unsure)
        if (_currAnimation != nullptr){
            _currAnimation->reset();
        }
        GameObject::setAnimation(animation);
    }
}

void Player::updateAnimation(float dt){
    GameObject::updateAnimation(dt); 
    // let all callbacks run (through default update) before custom update
    //the running/idle animations only apply in idle or dodge states; the other states have their own animations
    if (_state == IDLE || _state == DODGE){
        //TODO: sync frames when swapping weapons
        if (!_collider->getLinearVelocity().isZero()){
            switch (_weapon) {
            case MELEE:
                setAnimation(_runAnimation);
                break;
            case RANGED:
                setAnimation(_bowRunAnimation);
                break;
            }
        }
        else {
            switch (_weapon) {
            case MELEE:
                setAnimation(_idleAnimation);
                break;
            case RANGED:
                setAnimation(_bowIdleAnimation);
                break;
            }
        }
    }
    _parryEffect->update(dt);
    _swipeEffect->update(dt);
    _comboSwipeEffect->update(dt);
}

#pragma mark -
#pragma mark State Update

void Player::updateCounters(){
    atkCD.decrement();
    dodgeCD.decrement();
    hitCounter.decrement();
    if (hitCounter.isZero()) _tint = Color4::WHITE;
}

void Player::setFacingDir(cugl::Vec2 dir){
    int prevDirection = _directionIndex;
    Vec2 d = dir.normalize();
    _directionIndex = -1;
    float similarity = -INFINITY;
    for (int i = 0; i < 8; i++){
        Vec2 cardinal = _directions[i];
        float dotprod = cardinal.dot(d);
        if (dotprod > similarity){
            similarity = dotprod;
            _directionIndex = i;
        }
    }
    assert(_directionIndex >= 0 && _directionIndex < 8);
    _facingDirection = d;
    
    // sync animation
    if (prevDirection != _directionIndex){
        int startIndex = 8 * _directionIndex;
        int startIdx16 = 16 * _directionIndex;
        int startIdx24 = 24 * _directionIndex;
        int endIndex = startIndex + 8 - 1;
        _idleAnimation->setFrameRange(startIndex, endIndex);
        _attackAnimation1->setFrameRange(startIdx24, startIdx24+7);
        _attackAnimation2->setFrameRange(startIdx24+8, startIdx24+13);
        _attackAnimation3->setFrameRange(startIdx24+14, startIdx24+23);
        _runAnimation->setFrameRange(startIdx16, startIdx16 + 15);
        _parryStartAnimation->setFrameRange(startIdx16, startIdx16 + 1);
        _parryStanceAnimation->setFrameRange(startIdx16 + 2, startIdx16 + 9);
        _parryAnimation->setFrameRange(startIdx16 + 10, startIdx16 + 15);
        _bowIdleAnimation->setFrameRange(startIndex, endIndex);
        _bowRunAnimation->setFrameRange(startIdx16, startIdx16 + 15);
        _chargingAnimation->setFrameRange(startIdx16, startIdx16 + 8);
        _chargedAnimation->setFrameRange(startIdx16 + 8, startIdx16 + 8);
        _shotAnimation->setFrameRange(startIdx16 + 9, startIdx16 + 11);
        _recoveryAnimation->setFrameRange(startIdx16 + 12, startIdx16 + 15);
    }
}

void Player::hit(Vec2 atkDir, int damage, float knockback_scl) {
    //only get hit if not dodging and not in hitstun
    if (hitCounter.isZero() && _state != DODGE) {
        hitCounter.reset();
        _hp = std::fmax(0, (_hp - damage*defenseUpgrade->getCurrentValue()));
        _tint = Color4::RED;
        _collider->setLinearVelocity(atkDir * knockback_scl);
        if (_state == ATTACK){
            // previously attacking, got interrupted
            _comboTimer = 0;    // equivalent to a "normal" termination of an attack
            _swipeEffect->reset();
            _comboSwipeEffect->reset();
        }
        _state = IDLE; //TODO: hit state ???
    }
}

void Player::update(float dt) {
    updateCounters();
    updateAnimation(dt);
    if (_combo > 1){
        _comboTimer += dt; // incrementing timer to constraint player on making 2nd/3rd hits within time constraint
    }
    if (_comboTimer > GameConstants::COMBO_TIME) {
        _combo = 1; //reset combo if too much time has passed
        _comboTimer = 0;
    }
    switch (_state) {
    case CHARGED:
        _chargedEffect->update(dt);
        break;
    case CHARGING:
        if (_chargingEffect->isActive()) {
            _chargingEffect->update(dt);
        }
        else {
            _chargingEffect->start();
        }
        break;
    case DODGE:
        _dodge += dt;
        if (_dodge >= GameConstants::PLAYER_DODGE_TIME) {
            _state = IDLE;
            _dodge = 0;
        }
        break;
    case SHOT:
        _shotEffect->update(dt);
        break;
    case IDLE: case ATTACK: case RECOVERY: case PARRYSTART: case PARRYSTANCE: case PARRY:
        // no additional updates
        break;
    }
    // make sure hitbox debug node is hidden when not active
    _meleeHitbox->getDebugNode()->setVisible(_meleeHitbox->isEnabled());
    if (_state != ATTACK && _meleeHitbox->isEnabled()) {
        if (_meleeHitbox->hitFlag){
            accumulateCombo();
        }
        disableMeleeAttack();
    }
}

#pragma mark -
#pragma mark Physics

void Player::addObstaclesToWorld(std::shared_ptr<physics2::ObstacleWorld> world){
    GameObject::addObstaclesToWorld(world);
    world->addObstacle(_meleeHitbox);
    _meleeHitbox->setEnabled(false);
}

void Player::removeObstaclesFromWorld(std::shared_ptr<physics2::ObstacleWorld> world){
    GameObject::removeObstaclesFromWorld(world);
    world->removeObstacle(_meleeHitbox);
}

void Player::setDebugNode(const std::shared_ptr<scene2::SceneNode> &debug){
    GameObject::setDebugNode(debug);
    _meleeHitbox->setDebugScene(debug);
    _meleeHitbox->setDebugColor(Color4::RED);
}

void Player::syncPositions(){
    GameObject::syncPositions();
    // TODO: attack position should be based on physics size of player
    _meleeHitbox->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the pixel height of the player
}

void Player::enableMeleeAttack(float angle){
    _meleeHitbox->setEnabled(true);
    _meleeHitbox->setAngle(angle);
    // TODO: attack position should be based on physics size of player
    _meleeHitbox->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the pixel height of the player
}
