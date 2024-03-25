//
//  MeleeEnemy.cpp
//  RS
//
//  Created by Dylan McIntyre on 3/23/24.
//

#include "MeleeEnemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool MeleeEnemy::init(const Vec2 pos, const Size size) {
    Enemy::init(pos, size);
    _attackRange = GameConstants::ENEMY_MELEE_ATK_RANGE;
    
    return true;
}

/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void MeleeEnemy::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
}

#pragma mark -
#pragma mark Physics


#pragma mark -
#pragma mark Animation

void MeleeEnemy::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _enemyTexture = assets->get<Texture>("lizard-idle");
    auto walkTexture = assets->get<Texture>("lizard-walk");
    auto attackTexture = assets->get<Texture>("lizard-attack");
    auto stunTexture = assets->get<Texture>("lizard-stun");
    auto hitEffect = assets->get<Texture>("enemy-hit-effect");
    
    auto idleSheet = SpriteSheet::alloc(_enemyTexture, 8, 8);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 9);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 18);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 15);
    auto hitSheet = SpriteSheet::alloc(hitEffect, 2, 3);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 7);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 8);
    _attackAnimation = Animation::alloc(attackSheet, 0.75f, false, 0, 17);
    _stunAnimation = Animation::alloc(stunSheet, 1.0f, false, 0, 14);
    _hitEffect = Animation::alloc(hitSheet, 0.375f, false);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        _hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
    });
    
    _attackAnimation->addCallback(0.5f, [this](){
        if (isEnabled()) {
            _attack->setEnabled(true);
            _hitboxAnimation->start();
            _attack->setAwake(true);
            _attack->setAngle(getFacingDir().getAngle());
            // TODO: clean this code
            _attack->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the enemy pixel height
        }
    });
    
    setAnimation(_idleAnimation);

    _hitEffect->onComplete([this]() {
        _hitEffect->reset();
    });
}


void MeleeEnemy::setFacingDir(cugl::Vec2 dir) {
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
        _idleAnimation->setFrameRange(8 * _directionIndex, 8 * _directionIndex + 7);
        _walkAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
        _attackAnimation->setFrameRange(18 * _directionIndex, 18 * _directionIndex + 17);
        _stunAnimation->setFrameRange(15 * _directionIndex, 15 * _directionIndex + 14);
    }
}
