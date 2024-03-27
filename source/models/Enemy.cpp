//
//  Enemy.cpp
//  RS
//
//  Created by Zhiyuan Chen on 2/26/24.
//

#include "Enemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool Enemy::init(const Vec2 pos, const Size size) {
    auto box = std::make_shared<physics2::BoxObstacle>();
    box->init(pos,size);
    box->setAngle(M_PI_4);
    std::string name("enemy");
    box->setName(name);
    b2Filter filter;
    // this is an enemy and can collide with a player "shadow", an enemy (when not idle), a wall, or an attack
    filter.categoryBits = CATEGORY_ENEMY;
    filter.maskBits = CATEGORY_PLAYER_SHADOW | CATEGORY_ENEMY | CATEGORY_WALL | CATEGORY_ATTACK;
    box->setFilterData(filter);
    _collider = box;
    
    auto shadow = physics2::BoxObstacle::alloc(pos, (Size) size);
    shadow->setAngle(M_PI_4);
    shadow->setBodyType(b2_kinematicBody);
    // this is an enemy "shadow" and can collide with the player
    filter.categoryBits = CATEGORY_ENEMY_SHADOW;
    filter.maskBits = CATEGORY_PLAYER;
    shadow->setFilterData(filter);
    _colliderShadow = shadow;
    
    _isDefault = true;
    _playerLoc = Vec2::ZERO; // default value = hasn't ever seen the player
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


#pragma mark -
#pragma mark Animation


void Enemy::setDrawScale(Vec2 scale) {
    _drawScale = scale;
}

void Enemy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render enemy with appropriate scales
    // batch draw(texture, color, origin, scale, angle, offset)
    auto spriteSheet = _currAnimation->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2();
    // transform.scale(0.5);
    transform.translate(getPosition() * _drawScale);
    
    spriteSheet->draw(batch, _tint, origin, transform);

    if (_hitEffect->isActive()) {
        auto effSheet = _hitEffect->getSpriteSheet();
        Affine2 effTrans = Affine2();
        effTrans.scale(2);
        effTrans.translate(getPosition().add(0, 64 / _drawScale.y) * _drawScale); //64 is half of enemy pixel height
        Vec2 effOrigin = Vec2(effSheet->getFrameSize().width / 2, effSheet->getFrameSize().height / 2);
        effSheet->draw(batch, effOrigin, effTrans);
    }
}

void Enemy::loadAssets(const std::shared_ptr<AssetManager> &assets){
    // nothing here: each enemy loads its own assets
//    _enemyTexture = assets->get<Texture>("enemy-idle");
//    auto walkTexture = assets->get<Texture>("enemy-walk");
//    auto attackTexture = assets->get<Texture>("enemy-attack");
//    auto hitEffect = assets->get<Texture>("enemy-hit-effect");
//    
//    auto idleSheet = SpriteSheet::alloc(_enemyTexture, 8, 8);
//    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 9);
//    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 18);
//    auto hitSheet = SpriteSheet::alloc(hitEffect, 2, 3);
//    
//    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 7);
//    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 8);
//    _attackAnimation = Animation::alloc(attackSheet, 0.75f, false, 0, 17);
//    _hitEffect = Animation::alloc(hitSheet, 0.375f, false);
//    
//    _currAnimation = _idleAnimation; // set runnning
//    
//    // add callbacks
//    _attackAnimation->onComplete([this](){
//        _attackAnimation->reset();
//        _hitboxAnimation->reset();
//        _atkCD.reset(); // cooldown begins AFTER the attack is done
//        _attack->setEnabled(false);
//    });
//    
//    _attackAnimation->addCallback(0.5f, [this](){
//        if (isEnabled()) {
//            _attack->setEnabled(true);
//            _hitboxAnimation->start();
//            _attack->setAwake(true);
//            _attack->setAngle(getFacingDir().getAngle());
//            // TODO: clean this code
//            _attack->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the enemy pixel height
//        }
//    });
//    
//    setAnimation(_idleAnimation);
//
//    _hitEffect->onComplete([this]() {
//        _hitEffect->reset();
//    });
}

void Enemy::setIdling() {
    setAnimation(_idleAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _walkAnimation->reset();
    _attackAnimation->reset();
    _stunAnimation->reset();
    _state = EnemyState::IDLE;
    
}

void Enemy::setMoving() {
    setAnimation(_walkAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _attackAnimation->reset();
    _stunAnimation->reset();
    _state = EnemyState::MOVING;
}

void Enemy::setAttacking() {
    setAnimation(_attackAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _walkAnimation->reset();
    _stunAnimation->reset();
    _state = EnemyState::ATTACKING;
}

void Enemy::setStunned() {
    if (_state == EnemyState::STUNNED) {
        return;
    }
    setAnimation(_stunAnimation);
    _stunCD.reset();
    _atkCD.reset(); // stunning should reset attack
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _attackAnimation->reset();
    _idleAnimation->reset();
    _walkAnimation->reset();
    _state = EnemyState::STUNNED;
}


void Enemy::hit(cugl::Vec2 atkDir) {
    if (!_hitEffect->isActive()) {
        _hitCounter.reset();
        setHealth(getHealth()-1);
        _hitEffect->reset();
        _hitEffect->start();
        _collider->setLinearVelocity(atkDir*10); //tune this value (10)
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
    _hitEffect->update(dt);
    if (_hitEffect->isActive()){
        _tint = Color4::RED;
    }
    else if (_state == EnemyState::STUNNED && _stunCD.isZero()) {
        _tint = Color4::WHITE;
        setIdling();
    }
    else if (_state == EnemyState::STUNNED){
        // TODO: could possibly use stunned animation and remove this state altogether
        _tint = Color4::YELLOW;
    }
    else {
        _tint = Color4::WHITE;
    }
    
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
//    int prevDirection = _directionIndex;
//    Vec2 d = dir.normalize();
//    _directionIndex = -1;
//    float similarity = -INFINITY;
//    for (int i = 0; i < 8; i++){
//        Vec2 cardinal = _directions[i];
//        float dotprod = cardinal.dot(d);
//        if (dotprod > similarity){
//            similarity = dotprod;
//            _directionIndex = i;
//        }
//    }
//    assert(_directionIndex >= 0 && _directionIndex < 8);
//    _facingDirection = dir;
//    
//    if (prevDirection != _directionIndex){
//        _idleAnimation->setFrameRange(8 * _directionIndex, 8 * _directionIndex + 7);
//        _walkAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
//        _attackAnimation->setFrameRange(18 * _directionIndex, 18 * _directionIndex + 17);
//    }
}
