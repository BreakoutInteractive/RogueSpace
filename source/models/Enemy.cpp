//
//  Enemy.cpp
//  RS
//
//  Created by Zhiyuan Chen on 2/26/24.
//

#include "Enemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "../components/Collider.hpp"
#include "LevelModel.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool Enemy::init(std::shared_ptr<JsonValue> data) {
    _position.set(data->getFloat("x"), data->getFloat("y"));
    std::shared_ptr<JsonValue> colliderData = data->get("collider");
    auto collider = Collider::makeCollider(colliderData, b2_dynamicBody, "enemy-collider");
    b2Filter filter;
    // this is an enemy and can collide with a player "shadow", an enemy (when not idle), a wall, or an attack
    filter.categoryBits = CATEGORY_ENEMY;
    filter.maskBits = CATEGORY_PLAYER_SHADOW | CATEGORY_ENEMY | CATEGORY_TALL_WALL | CATEGORY_SHORT_WALL | CATEGORY_RELIC | CATEGORY_ATTACK | CATEGORY_PROJECTILE ;
    collider->setFilterData(filter);
    _collider = collider;   // attach the collider to the game object
    
    auto shadow = Collider::makeCollider(colliderData, b2_kinematicBody, "enemy-shadow");
    // this is an enemy "shadow" and can collide with the player
    filter.categoryBits = CATEGORY_ENEMY_SHADOW;
    filter.maskBits = CATEGORY_PLAYER;
    shadow->setFilterData(filter);
    _colliderShadow = shadow;
    
    // set the enemy hitbox sensor
    std::shared_ptr<JsonValue> hitboxData = data->get("hitbox");
    auto hitbox = Collider::makeCollider(hitboxData, b2_kinematicBody, "enemy-hurtbox", true);
    filter.categoryBits = CATEGORY_ENEMY_HITBOX;
    filter.maskBits = CATEGORY_ATTACK | CATEGORY_PROJECTILE;
    hitbox->setFilterData(filter);
    _sensor = hitbox;
    _sensor->setDebugColor(Color4::RED);
    
    // initialize enemy properties
    _isAiming = false; // will always be false for melee enemies
    _isCharged = false; // will always be false for melee enemies
    _aggroLoc = Vec2::ZERO; // default value = hasn't been aggro'd
    _isAligned = false;
    _state = BehaviorState::DEFAULT;
    _sightRange = GameConstants::ENEMY_SIGHT_RANGE;
    _proximityRange = GameConstants::ENEMY_PROXIMITY_RANGE;
    _attackRange = GameConstants::ENEMY_MELEE_ATK_RANGE;
    _moveSpeed = GameConstants::ENEMY_MOVE_SPEED;
    _hitCounter.setMaxCount(GameConstants::ENEMY_IFRAME);
    _atkCD.setMaxCount(GameConstants::ENEMY_ATK_COOLDOWN);
    _stunCD.setMaxCount(GameConstants::ENEMY_STUN_COOLDOWN);
    _sentryCD.setMaxCount(GameConstants::ENEMY_SENTRY_COOLDOWN);
    
    // initialize directions
    _directions[0] = Vec2(0,-1);    //down
    _directions[2] = Vec2(1,0);     //right
    _directions[4] = Vec2(0,1);     //up
    _directions[6] = Vec2(-1,0);    //left
    for (int i = 1; i < 8; i+=2){
        _directions[i] = Vec2(0,-1).rotate(M_PI_4 * i); // diagonal directions
    }
    _directionIndex = 0;
    _facingDirection = _directions[0]; // starts facing downward
    return true;
}


/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void Enemy::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
}


#pragma mark -
#pragma mark Physics

void Enemy::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    // nothing here: each enemy implements its own attack
}


#pragma mark -
#pragma mark Animation


void Enemy::setDrawScale(Vec2 scale) {
    _drawScale = scale;
}

void Enemy::drawEffect(const std::shared_ptr<cugl::SpriteBatch>& batch, const std::shared_ptr<Animation>& effect, float scale) {    
    auto effSheet = effect->getSpriteSheet();
    Affine2 transform = Affine2::createScale(scale);
    transform.translate((_position + Vec2(0, 64 / scale / _drawScale.y)) * _drawScale); //64 is half of enemy pixel height
    Vec2 origin = Vec2(effSheet->getFrameSize().width / 2, effSheet->getFrameSize().height / 2);
    effSheet->draw(batch, origin, transform);
}

void Enemy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render enemy with appropriate scales
    // batch draw(texture, color, origin, scale, angle, offset)
    auto spriteSheet = _currAnimation->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2();
    // transform.scale(0.5);
    transform.translate(_position * _drawScale); // previously using getPosition()
    
    spriteSheet->draw(batch, _tint, origin, transform);
    
    //enemy health bar
    float idleWidth = _idleAnimation->getSpriteSheet()->getFrameSize().width;
    Vec2 idleOrigin = Vec2(_idleAnimation->getSpriteSheet()->getFrameSize().width / 2, 0);
    
    Rect healthBGRect = Rect(0, spriteSheet->getFrameSize().height, idleWidth, 5);
    Rect healthFGRect = Rect(0, spriteSheet->getFrameSize().height, idleWidth*(_health/_maxHealth), 5);

    batch->draw(_healthBG, healthBGRect, idleOrigin, transform);
    batch->draw(_healthFG, healthFGRect, idleOrigin, transform);
    
    if (_meleeHitEffect->isActive()) {
        drawEffect(batch, _meleeHitEffect, 2);
    }
    if (_bowHitEffect->isActive()) {
        drawEffect(batch, _bowHitEffect, 2);
    }
    if (_state == BehaviorState::STUNNED) {
        drawEffect(batch, _stunEffect);
    }
}

void Enemy::loadAssets(const std::shared_ptr<AssetManager> &assets){
    // nothing here: each enemy loads its own assets
}

void Enemy::setIdling() {
    setAnimation(_idleAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _walkAnimation->reset();
    _attackAnimation->reset();
    _stunAnimation->reset();
}

void Enemy::setMoving() {
    setAnimation(_walkAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _attackAnimation->reset();
    _stunAnimation->reset();
}

void Enemy::setAttacking() {
    setAnimation(_attackAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _walkAnimation->reset();
    _stunAnimation->reset();
    _state = BehaviorState::ATTACKING;
}

void Enemy::setStunned() {
    if (_state == BehaviorState::STUNNED) {
        return;
    }
    setAnimation(_stunAnimation);
    _stunCD.reset();
    _atkCD.reset(); // stunning should reset attack
    _attackAnimation->reset();
    _hitboxAnimation->reset();
    _idleAnimation->reset();
    _walkAnimation->reset();
    _state = BehaviorState::STUNNED;
    _stunEffect->start();
}

void Enemy::setDefault() {
    _state = BehaviorState::DEFAULT;
}

void Enemy::setSeeking() {
    _state = BehaviorState::SEEKING;
}

void Enemy::setChasing() {
    _state = BehaviorState::CHASING;
}


void Enemy::hit(cugl::Vec2 atkDir, bool ranged, float damage, float knockback_scl) {
    if (!_meleeHitEffect->isActive() && !_bowHitEffect->isActive()) {
        _hitCounter.reset();
        if (_state == BehaviorState::STUNNED) damage *= GameConstants::STUN_DMG_BONUS;
        setHealth(getHealth()-damage);
        _meleeHitEffect->reset();
        _bowHitEffect->reset();
        if (ranged) _bowHitEffect->start();
        else _meleeHitEffect->start();
        _collider->setLinearVelocity(atkDir * knockback_scl);
        // allows for a "revenge" attack if the enemy is attacked from behind
        if (!_playerInSight) {
            _facingDirection = -atkDir;
        }
    }
}

void Enemy::updateAnimation(float dt){
    GameObject::updateAnimation(dt);
    // attack animation must play to completion, as long as enemy is alive.
    if (!_attackAnimation->isActive()) {
        if ((getCollider()->getLinearVelocity().isZero() && _stunCD.isZero()) && _currAnimation != _idleAnimation) {
            setIdling();
        }
        else if (!getCollider()->getLinearVelocity().isZero() && _currAnimation != _walkAnimation) {
            setMoving();
        }
    }
    _meleeHitEffect->update(dt);
    _bowHitEffect->update(dt);
    if (_meleeHitEffect->isActive() || _bowHitEffect->isActive()){
        _tint = Color4::RED;
    }
    else if (_state == BehaviorState::STUNNED && _stunCD.isZero()) {
        _tint = Color4::WHITE;
        setIdling();
    }
    else {
        _tint = Color4::WHITE;
    }
    _stunEffect->update(dt);
    _hitboxAnimation->update(dt);
}

void Enemy::updateCounters() {
    _sentryCD.decrement();
    _stunCD.decrement();
    _atkCD.decrement();
    _hitCounter.decrement();
}

void Enemy::setFacingDir(cugl::Vec2 dir) {
    // nothing here: each enemy has its own animations
}
