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

void PlayerHitbox::setEnabled(bool value){
    WheelObstacle::setEnabled(value);
    hitFlag = false; // clear the flag
    hitSet.clear();
}

bool PlayerHitbox::hits(intptr_t enemyPtr){
    if (hitSet.find(enemyPtr) != hitSet.end()){
        return false;
    }
    hitSet.insert(enemyPtr);
    return true;
}

#pragma mark -
#pragma mark Constructors


bool Player::init(std::shared_ptr<JsonValue> playerData, std::shared_ptr<JsonValue> upgradesJson) {
    auto meleeStats = upgradesJson->get("melee")->asFloatArray();
    auto parryStats = upgradesJson->get("parry")->asFloatArray();
    auto meleeSpStats = upgradesJson->get("meleeSpeed")->asFloatArray();
    auto dodgeStats = upgradesJson->get("dodge")->asFloatArray();
    auto bowStats = upgradesJson->get("range")->asFloatArray();
    auto healthStats = upgradesJson->get("health")->asFloatArray();
    auto armorStats = upgradesJson->get("armor")->asFloatArray();
    auto blockStats = upgradesJson->get("block")->asFloatArray();
    
    _meleeDamage = Upgradeable(meleeStats, UpgradeType::SWORD);
    _stunWindow = Upgradeable(parryStats, UpgradeType::PARRY);
    _attackCooldown = Upgradeable(meleeSpStats, UpgradeType::ATK_SPEED);
    _dodgeCount = Upgradeable(dodgeStats, UpgradeType::DASH);
    _bowDamage = Upgradeable(bowStats, UpgradeType::BOW);
    _maxHP = Upgradeable(healthStats, UpgradeType::HEALTH);
    _damageReduction = Upgradeable(armorStats, UpgradeType::SHIELD);
    _blockReduction = Upgradeable(blockStats, UpgradeType::SHIELD);


    _weapon = MELEE;
    _state = IDLE;
    _dodgeDuration = 0;
    _attackActiveCooldown = 0;
    _stamina = GameConstants::PLAYER_STAMINA;
    _combo = 1;
    _comboTimer = 0;
    _position.set(playerData->getFloat("x"), playerData->getFloat("y"));
    std::shared_ptr<JsonValue> colliderData = playerData->get("collider");
    // set up collider
    auto collider = Collider::makePolygon(colliderData, b2_dynamicBody, "player-collider");
    // this is a player and can collide with an enemy "shadow", wall, or attack
    b2Filter filter;
    filter.categoryBits = CATEGORY_PLAYER;
    filter.maskBits = CATEGORY_ENEMY_SHADOW | CATEGORY_TALL_WALL | CATEGORY_SHORT_WALL | CATEGORY_ATTACK | CATEGORY_PROJECTILE | CATEGORY_RELIC | CATEGORY_HEALTHPACK;
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
    filter.maskBits = CATEGORY_ATTACK | CATEGORY_PROJECTILE | CATEGORY_HEALTHPACK;
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
    _iframeCounter.setMaxCount(GameConstants::PLAYER_IFRAME);
    
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

#pragma mark -
#pragma mark Animation

void Player::drawRangeIndicator(const std::shared_ptr<SpriteBatch>& batch, const std::shared_ptr<physics2::ObstacleWorld>& world) {
    Vec2 direction = getFacingDir();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0) {
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    Affine2 t = Affine2::createRotation(ang);
    t.scale(_drawScale);
    t.translate(getPosition() * _drawScale);
    float rayLength = GameConstants::PROJ_DIST_P + GameConstants::PROJ_SIZE_P_HALF;
    Vec2 rayEnd = getPosition() + rayLength * getFacingDir();
    float frac = 1;
    Vec2 loc = rayEnd;
    std::function<float(b2Fixture*, const Vec2, const Vec2, float)> callback
        = [&frac, &loc](b2Fixture* fixture, const Vec2 point, const Vec2 normal, float fraction) {
        if (fixture->GetFilterData().categoryBits != CATEGORY_SHORT_WALL && fixture->GetFilterData().categoryBits != CATEGORY_PROJECTILE 
            && fixture->GetFilterData().categoryBits != CATEGORY_PROJECTILE_SHADOW && !fixture->IsSensor()) {
            if (fraction < frac){
                frac = fraction;
                loc = point;
            }
            return fraction;
        }
        else return -1.0f;
        };
    world->rayCast(callback, getPosition(), rayEnd);
    if (abs(frac-1)>0.01) {
        float dist = getPosition().distance(loc);
        if (dist >= GameConstants::PROJ_SIZE_P_HALF) {
            std::vector<Vec2> vec = std::vector<Vec2>();
            vec.push_back(Vec2(0, GameConstants::PROJ_SIZE_P_HALF / 2));
            vec.push_back(Vec2(0, -GameConstants::PROJ_SIZE_P_HALF / 2));
            vec.push_back(Vec2(dist - GameConstants::PROJ_SIZE_P_HALF, -GameConstants::PROJ_SIZE_P_HALF / 2));
            vec.push_back(Vec2(dist, 0));
            vec.push_back(Vec2(dist - GameConstants::PROJ_SIZE_P_HALF, GameConstants::PROJ_SIZE_P_HALF / 2));
            EarclipTriangulator et = EarclipTriangulator(vec);
            et.calculate();
            Poly2 poly = et.getPolygon();
            batch->draw(nullptr, Color4(0, 0, 0, 90), poly, Vec2::ZERO, t);
        }
    }
    else {
        std::vector<Vec2> vec = std::vector<Vec2>();
        vec.push_back(Vec2(0, GameConstants::PROJ_SIZE_P_HALF / 2));
        vec.push_back(Vec2(0, -GameConstants::PROJ_SIZE_P_HALF / 2));
        vec.push_back(Vec2(GameConstants::PROJ_DIST_P, -GameConstants::PROJ_SIZE_P_HALF / 2));
        vec.push_back(Vec2(GameConstants::PROJ_DIST_P + GameConstants::PROJ_SIZE_P_HALF, 0));
        vec.push_back(Vec2(GameConstants::PROJ_DIST_P, GameConstants::PROJ_SIZE_P_HALF / 2));
        EarclipTriangulator et = EarclipTriangulator(vec);
        et.calculate();
        Poly2 poly = et.getPolygon();
        batch->draw(nullptr, Color4(0, 0, 0, 90), poly, Vec2::ZERO, t);
    }
}

void Player::drawEffect(const std::shared_ptr<cugl::SpriteBatch>& batch, const std::shared_ptr<Animation>& effect, float ang, float scale) {
    auto sheet = effect->getSpriteSheet();
    Vec2 o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
    Vec2 direction = getFacingDir();
    
    Affine2 t = Affine2::createRotation(ang);
    t.scale(scale);
    t.translate((_position+Vec2(0, 64 / scale / getDrawScale().y)) * _drawScale);
    sheet->draw(batch, o, t);
}

void Player::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    if (_state == State::DEAD) return;
    
    auto spriteSheet = _currAnimation->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2::createTranslation(_position * _drawScale);
    // make sure to NOT draw the player first when attacking + facing backwards
    if (!(isAttacking() && _directionIndex >= 3 && _directionIndex <= 5)){
        spriteSheet->draw(batch, _tint, origin, transform);
    }
    
    //effects
    std::shared_ptr<SpriteSheet> sheet;
    Vec2 o = Vec2::ZERO;
    Affine2 t = Affine2::ZERO;
    Vec2 direction = Vec2::ZERO;
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0) {
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
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
        drawEffect(batch, _chargedEffect, ang);
        break;
    case CHARGING:
        drawEffect(batch, _chargingEffect, ang);
        break;
    case SHOT:
        if (_shotEffect->isActive()) drawEffect(batch, _shotEffect, ang);
        break;
    default:
        break;
    }
    if (_swipeEffect->isActive() || _comboSwipeEffect->isActive()){
        sheet = _comboSwipeEffect->isActive() ? _comboSwipeEffect->getSpriteSheet() : _swipeEffect->getSpriteSheet();
        Affine2 atkTrans = Affine2::createScale(GameConstants::PLAYER_MELEE_ATK_RANGE / ((Vec2)sheet->getFrameSize() / 2) * getDrawScale());
        //we subtract pi/2 from the angle since the animation is pointing up but the hitbox points right by default
        atkTrans.rotate(_meleeHitbox->getAngle() - M_PI_2);
        atkTrans.translate(_meleeHitbox->getPosition() * _drawScale);
        sheet->draw(batch, Color4::WHITE, Vec2(sheet->getFrameSize().getIWidth() / 2, 0), atkTrans);
    }
    
    if ((isAttacking() && _directionIndex >= 3 && _directionIndex <= 5)){
        spriteSheet->draw(batch, _tint, origin, transform);
    }

    // this is always drawn on top of player
    if (_parryEffect->isActive()) {
        drawEffect(batch, _parryEffect, ang);
    }
    
    if (_hitEffect->isActive()) {
        drawEffect(batch, _hitEffect, 0, 2);
    }

    if (_deathEffect->isActive()) {
        drawEffect(batch, _deathEffect);
    }
}

void Player::loadAssets(const std::shared_ptr<AssetManager> &assets){
    // make animation sheets and animations
    auto parrySheet = SpriteSheet::alloc(assets->get<Texture>("player-parry"), 8, 16);
    auto attackSheet = SpriteSheet::alloc(assets->get<Texture>("player-attack"), 8, 24);
    auto idleSheet = SpriteSheet::alloc(assets->get<Texture>("player-idle"), 8, 8);
    auto runSheet = SpriteSheet::alloc(assets->get<Texture>("player-run"), 8, 16);
    auto rangedSheet = SpriteSheet::alloc(assets->get<Texture>("player-ranged"), 8, 16);
    auto bowIdleSheet = SpriteSheet::alloc(assets->get<Texture>("player-bow-idle"), 8, 8);
    auto bowRunSheet = SpriteSheet::alloc(assets->get<Texture>("player-bow-run"), 8, 16);
    auto projEffectSheet = SpriteSheet::alloc(assets->get<Texture>("player-projectile"), 4, 4);

    // pass to animations
    _parryStartAnimation = Animation::alloc(parrySheet, 0.1f, false, 0, 1);
    _parryStanceAnimation = Animation::alloc(parrySheet, 0.5f, true, 2, 9);
    _parryAnimation = Animation::alloc(parrySheet, GameConstants::PLAYER_PARRY_TIME, false, 10, 15);
    _attackAnimation1 = Animation::alloc(attackSheet, 0.5f, false, 0, 7);
    _attackAnimation2 = Animation::alloc(attackSheet, 0.5f, false, 8, 13);
    _attackAnimation3 = Animation::alloc(attackSheet, 0.5f, false, 14, 23);
    _runAnimation = Animation::alloc(runSheet, 0.667f, true, 0, 15);
    _idleAnimation = Animation::alloc(idleSheet, 1.2f, true, 0, 7);
    _chargingAnimation = Animation::alloc(rangedSheet, GameConstants::CHARGE_TIME, false, 0, 8);
    _chargedAnimation = Animation::alloc(rangedSheet, 0.1f, true, 8, 8);
    _shotAnimation = Animation::alloc(rangedSheet, 0.125f, false, 9, 11);
    _recoveryAnimation = Animation::alloc(rangedSheet, 0.167f, false, 12, 15);
    _bowRunAnimation = Animation::alloc(bowRunSheet, 0.667f, true, 0, 15);
    _bowIdleAnimation = Animation::alloc(bowIdleSheet, 1.2f, true, 0, 7);
    
    // make effect sheets and animation
    auto parryEffectSheet = SpriteSheet::alloc(assets->get<Texture>("parry-effect"), 2, 4);
    auto swipeEffectSheet = SpriteSheet::alloc(assets->get<Texture>("player-swipe"), 2, 4);
    auto comboSwipeEffectSheet = SpriteSheet::alloc(assets->get<Texture>("player-swipe-combo"), 2, 4);
    auto hitSheet = SpriteSheet::alloc(assets->get<Texture>("hit-effect"), 2, 3);
    auto deathSheet = SpriteSheet::alloc(assets->get<Texture>("player-death-effect"), 2, 4);
    _chargingEffect = Animation::alloc(projEffectSheet, GameConstants::CHARGE_TIME, false, 0, 3);
    _chargedEffect = Animation::alloc(projEffectSheet, 0.333f, true, 4, 7);
    _shotEffect = Animation::alloc(projEffectSheet, 1/24.0f, false, 8, 8);
    _parryEffect = Animation::alloc(parryEffectSheet, 0.4f, false);
    _swipeEffect = Animation::alloc(swipeEffectSheet, 0.4f, false); // tries to match attack 1 and 2, but played a bit faster
    _comboSwipeEffect = Animation::alloc(comboSwipeEffectSheet, 0.4f, false);
    _hitEffect = Animation::alloc(hitSheet, 0.25f, false);
    _deathEffect = Animation::alloc(deathSheet, 1.0f, false);
    
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
    _hitEffect->onComplete([this]() {
        _hitEffect->reset();
        });
    _deathEffect->onComplete([this]() {
        _deathEffect->reset();
        _state = DEAD;
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

void Player::animateDying() {
    _currAnimation->stopAnimation();
    _hitEffect->reset();
    _deathEffect->start();
    _state = DYING;
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
        // TODO: (just a comment) knockback has no visual animation so having the player quickly play a run animation seems ok
        // the player is running usually when velocity isn't zero but this is not the case when getting hit (hence hit counter)
        // if (!_collider->getLinearVelocity().isZero() && _iframeCounter.isZero()){
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
    _hitEffect->update(dt);
    if (_hitEffect->isActive()) {
        _tint = Color4::RED;
    }
}

#pragma mark -
#pragma mark State Update

void Player::updateCounters(){
    _iframeCounter.decrement();
    if (_iframeCounter.isZero()) _tint = Color4::WHITE;
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

void Player::hit(Vec2 atkDir, float damage, float knockback_scl) {
    //only get hit if not dodging and not in hitstun
    if (!_hitEffect->isActive() && _state != DODGE) {
        float reduction = _damageReduction.getCurrentValue() + (isBlocking() ? _blockReduction.getCurrentValue() : 0);
        _hp = std::fmax(0, (_hp - damage * (1 - reduction)));
        _tint = Color4::RED;
        _collider->setLinearVelocity(atkDir * knockback_scl);
        _hitEffect->start();
        if (_state == ATTACK){
            // previously attacking, got interrupted
            _comboTimer = 0;    // equivalent to a "normal" termination of an attack
            _swipeEffect->reset();
            _comboSwipeEffect->reset();
        }
        if (_hp == 0) animateDying();
    }
}

void Player::update(float dt) {
    updateCounters();
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
        _dodgeDuration += dt;
        if (_dodgeDuration >= GameConstants::PLAYER_DODGE_TIME) {
            _state = IDLE;
            _dodgeDuration = 0;
        }
        break;
    case SHOT:
        _shotEffect->update(dt);
        break;
    case DYING:
        _deathEffect->update(dt);
        _meleeHitbox->setEnabled(false);
        break;
    case IDLE: case ATTACK: case RECOVERY: case PARRYSTART: case PARRYSTANCE: case PARRY: case DEAD:
        // no additional updates
        break;
    }
    // make sure hitbox debug node is hidden when not active
    _meleeHitbox->getDebugNode()->setVisible(_meleeHitbox->isEnabled());
    if (_state != ATTACK) {
        if (_meleeHitbox->isEnabled()){
            if (_meleeHitbox->hitFlag){
                accumulateCombo();
            }
            disableMeleeAttack();
        }
        // decrement cooldown on melee attack
        _attackActiveCooldown -= dt;
        _attackActiveCooldown = std::fmax(0, _attackActiveCooldown);
    }
    
    if (_state != DODGE){
        // increase stamina
        _stamina = std::fmin(GameConstants::PLAYER_STAMINA, _stamina+1);
    }
}

#pragma mark -
#pragma mark Stats and Properties

float Player::getBowDamage() {
    if (_state == CHARGING) return _bowDamage.getCurrentValue() * (0.5f + _chargingAnimation->elapsed() / GameConstants::CHARGE_TIME);
    else if (_state == CHARGED) return _bowDamage.getCurrentValue() * 1.5f;
    else return _bowDamage.getCurrentValue();
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
    // move the melee hitbox with the player
    _meleeHitbox->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the pixel height of the player
}

void Player::enableMeleeAttack(float angle){
    _meleeHitbox->setEnabled(true);
    _meleeHitbox->setAngle(angle);
    _meleeHitbox->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the pixel height of the player
}
