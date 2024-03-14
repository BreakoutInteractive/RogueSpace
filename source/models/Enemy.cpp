//
//  Enemy.cpp
//  RS
//
//  Created by Zhiyuan Chen on 2/26/24.
//

#include "Enemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"

using namespace cugl;

// should be at least the player's attack time so that it can't get hit twice by the same attack
#define HIT_TIME 16
/**the number of frames an attack will last**/
#define ATK_TIME 45
/**the number of frames we wait before allowing another attack*/
#define ATK_CD 120
/**the number of frames an enemy will be stunned*/
#define STUN_CD 60
/**the number of frames a sentry will wait before rotating**/
#define SENTRY_CD 120
/**the sight range of an enemy**/
#define SIGHT_RANGE 8
/**the attack range of an enemy**/
#define ATK_RANGE 4
/**the default movement speed for an enemy**/
#define MOVE_SPEED 2

#pragma mark -
#pragma mark Constructors


bool Enemy::init(const Vec2 pos, const Size size) {
    auto box = std::make_shared<physics2::BoxObstacle>();
    box->init(pos,size);
    box->setAngle(M_PI_4);
    std::string name("enemy");
    box->setName(name);
    b2Filter filter;
    // this is an enemy and can collide with a player "shadow", an enemy "shadow", a wall, or an attack
    filter.categoryBits = CATEGORY_ENEMY;
    filter.maskBits = CATEGORY_PLAYER_SHADOW | CATEGORY_ENEMY | CATEGORY_WALL | CATEGORY_ATTACK;
    box->setFilterData(filter);
    _collider = box;
    
    auto shadow = physics2::BoxObstacle::alloc(pos, (Size) size);
    shadow->setAngle(M_PI_4);
    shadow->setBodyType(b2_kinematicBody);
    // this is an enemy "shadow" and can collide with the player or an enemy
    filter.categoryBits = CATEGORY_ENEMY_SHADOW;
    filter.maskBits = CATEGORY_PLAYER;
    shadow->setFilterData(filter);
    _colliderShadow = shadow;
    
    _sightRange = SIGHT_RANGE; // TODO: factor this into JSON constants
    _attackRange = ATK_RANGE;
    _moveSpeed = MOVE_SPEED;
    _hitCounter.setMaxCount(HIT_TIME);
    _atkLength.setMaxCount(ATK_TIME);
    _atkCD.setMaxCount(ATK_CD);
    _stunCD.setMaxCount(STUN_CD);
    _sentryCD.setMaxCount(SENTRY_CD);
    
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
    transform.scale(0.5);
    transform.translate(getPosition() * _drawScale);
    
    spriteSheet->draw(batch, _tint, origin, transform);
}

void Enemy::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _enemyTexture = assets->get<Texture>("enemy-idle");
    auto walkTexture = assets->get<Texture>("enemy-walk");
    auto attackTexture = assets->get<Texture>("enemy-attack");
    
    auto idleSheet = SpriteSheet::alloc(_enemyTexture, 1, 1);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 9);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 18);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, false);
    _walkAnimation = Animation::alloc(walkSheet, 0.4f, true, 0, 8);
    _attackAnimation = Animation::alloc(attackSheet, 0.75f, false, 0, 17);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        setAnimation(_idleAnimation);
    });
    
    setAnimation(_idleAnimation);
}

void Enemy::animateDefault() {
    _currAnimation = _idleAnimation;
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _walkAnimation->reset();
    _attackAnimation->reset();
}

void Enemy::animateWalk() {
    _currAnimation = _walkAnimation;
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _attackAnimation->reset();
}

void Enemy::animateAttack() {
    _currAnimation = _attackAnimation;
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _walkAnimation->reset();
}

void Enemy::hit(cugl::Vec2 atkDir) {
    if (_hitCounter.isZero()) {
        _hitCounter.reset();
        setHealth(getHealth()-1);
        _tint = Color4::RED;
        _collider->setLinearVelocity(atkDir*10); //tune this value (10)
    }
}

void Enemy::stun() {
    if (_stunCD.isZero()) {
        _stunCD.reset();
        _tint = Color4::YELLOW;
        _collider->setLinearVelocity(Vec2::ZERO);
    }
}

void Enemy::updateCounters() {
    if (_atkLength.getCount() == _atkLength.getMaxCount() && _currAnimation != _attackAnimation) {
        _attackAnimation->reset();
        animateAttack();
    }
    else if (!getCollider()->getLinearVelocity().isZero() && _atkLength.isZero() && _currAnimation != _walkAnimation) {
        animateWalk();
    }
    else if ((getCollider()->getLinearVelocity().isZero() || !_stunCD.isZero()) && _currAnimation != _idleAnimation) {
        animateDefault();
    }
    _sentryCD.decrement();
    _stunCD.decrement();
    _atkCD.decrement();
    _atkLength.decrement();
    _hitCounter.decrement();
    if (_hitCounter.isZero() && _stunCD.isZero()) _tint = Color4::WHITE;
}

void Enemy::setFacingDir(cugl::Vec2 dir) {
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
    _facingDirection = dir;
    
    if (prevDirection != _directionIndex){
        _walkAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
        _attackAnimation->setFrameRange(18 * _directionIndex, 18 * _directionIndex + 17);
    }
}
