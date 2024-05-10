//
//  MageAlien.cpp
//  RS
//
//  Created by Dylan McIntyre on 3/24/24.
//

#include "MageAlien.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool MageAlien::init(std::shared_ptr<JsonValue> data) {
    RangedEnemy::init(data);
    
    return true;
}

/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void MageAlien::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
}


#pragma mark -
#pragma mark Physics

void MageAlien::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    Vec2 direction = getFacingDir();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0){
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    
    setCharged(false);
    std::shared_ptr<Projectile> p = Projectile::mageAlloc(getPosition().add(0, 64 / getDrawScale().y), 1, ang, assets);
    p->setDrawScale(level->getDrawScale());
    level->addProjectile(p);
}


#pragma mark -
#pragma mark Animation

void MageAlien::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _enemyTexture = assets->get<Texture>("mage-idle");
    _healthBG =  assets->get<Texture>("hp_back");
    _healthFG =  assets->get<Texture>("hp");
    auto walkTexture = assets->get<Texture>("mage-walk");
    auto attackTexture = assets->get<Texture>("mage-attack");
    auto stunTexture = assets->get<Texture>("mage-idle"); // use idle animation for now
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto stunEffect = assets->get<Texture>("stun-effect");
    auto projectileTexture = assets->get<Texture>("mage-projectile");
    
    auto idleSheet = SpriteSheet::alloc(_enemyTexture, 8, 9);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 16);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 14);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 9);
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto stunEffectSheet = SpriteSheet::alloc(stunEffect, 2, 4);
    auto projectileSheet = SpriteSheet::alloc(projectileTexture, 3, 7);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 8);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 6, 13);
    _attackAnimation = Animation::alloc(attackSheet, 1.125f, false, 0, 13);
    _stunAnimation = Animation::alloc(stunSheet, 1.0f, false, 0, 8);
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _stunEffect = Animation::alloc(stunEffectSheet, 0.333f, true);
    _chargingAnimation = Animation::alloc(projectileSheet, 0.5625f, false, 0, 13);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        //_hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
    });
    
    _attackAnimation->addCallback(0.0f, [this](){
        if (isEnabled()) {
            _chargingAnimation->start();
        }
        setAiming(true);
    });
    
    _attackAnimation->addCallback(0.45f, [this](){
        setAiming(false);
    });
    
    _chargingAnimation->onComplete([this](){
        _chargingAnimation->reset();
        setCharged(true);
    });
    
    setAnimation(_idleAnimation);

    _meleeHitEffect->onComplete([this]() {
        _meleeHitEffect->reset();
    });
    _bowHitEffect->onComplete([this]() {
        _bowHitEffect->reset();
    });
}

void MageAlien::updateAnimation(float dt){
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
    if (_meleeHitEffect->isActive() || _bowHitEffect->isActive()) {
        _tint = Color4::RED;
    }
    else if (getState() == EnemyState::STUNNED && _stunCD.isZero()) {
        _tint = Color4::WHITE;
        setIdling();
    }
    else if (getState() == EnemyState::STUNNED){
        // TODO: could possibly use stunned animation and remove this state altogether
        _tint = Color4::YELLOW;
    }
    else {
        _tint = Color4::WHITE;
    }
    
    _hitboxAnimation->update(dt);
    
    _chargingAnimation->update(dt);
}


void MageAlien::setFacingDir(cugl::Vec2 dir) {
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
        _idleAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
        _walkAnimation->setFrameRange(16 * _directionIndex + 6, 16 * _directionIndex + 13);
        _attackAnimation->setFrameRange(14 * _directionIndex, 14 * _directionIndex + 13);
        _stunAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
    }
}
