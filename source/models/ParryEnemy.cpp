//
//  ParryEnemy.cpp
//  RS
//
//  Created by Dylan McIntyre on 4/22/24.
//

#include "ParryEnemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "Player.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool ParryEnemy::init(std::shared_ptr<JsonValue> data) {
    Enemy::init(data);
    _attackRange = GameConstants::ENEMY_MELEE_ATK_RANGE*0.8f;
    return true;
}

/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void ParryEnemy::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
}


#pragma mark -
#pragma mark Physics

void ParryEnemy::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager>& assets) {
    Vec2 direction = level->getPlayer()->getPosition() * level->getPlayer()->getDrawScale() - getPosition() * getDrawScale();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0) {
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }

    _attack->setPosition(_attack->getPosition().add(0, 64 / _drawScale.y)); //64 is half of the pixel height of the enemy
    _attack->setAngle(ang);
}


#pragma mark -
#pragma mark Animation

void ParryEnemy::loadAssets(const std::shared_ptr<AssetManager>& assets) {
    _enemyTexture = assets->get<Texture>("parry-idle");
    //TODO: real animations
    auto walkTexture = assets->get<Texture>("parry-idle");
    auto attackTexture = assets->get<Texture>("parry-idle");
    auto stunTexture = assets->get<Texture>("parry-idle");
    auto hitEffect = assets->get<Texture>("enemy-hit-effect");
    auto stunEffect = assets->get<Texture>("stun-effect");

    auto idleSheet = SpriteSheet::alloc(_enemyTexture, 8, 5);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 5);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 5);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 5);
    auto hitSheet = SpriteSheet::alloc(hitEffect, 2, 3);
    auto stunEffectSheet = SpriteSheet::alloc(stunEffect, 2, 4);

    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 4);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 4);
    _attackAnimation = Animation::alloc(attackSheet, 1.125f, false, 0, 4);
    _stunAnimation = Animation::alloc(stunSheet, 1.0f, false, 0, 4);
    _hitEffect = Animation::alloc(hitSheet, 0.25f, false);
    _stunEffect = Animation::alloc(stunEffectSheet, 0.333f, true);

    _currAnimation = _idleAnimation; // set runnning

    // add callbacks
    _attackAnimation->onComplete([this]() {
        _attackAnimation->reset();
        _hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
        });

    _attackAnimation->addCallback(0.75f, [this]() {
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


void ParryEnemy::setFacingDir(cugl::Vec2 dir) {
    int prevDirection = _directionIndex;
    Vec2 d = dir.normalize();
    _directionIndex = -1;
    float similarity = -INFINITY;
    for (int i = 0; i < 8; i++) {
        Vec2 cardinal = _directions[i];
        float dotprod = cardinal.dot(d);
        if (dotprod > similarity) {
            similarity = dotprod;
            _directionIndex = i;
        }
    }
    assert(_directionIndex >= 0 && _directionIndex < 8);
    _facingDirection = dir;

    if (prevDirection != _directionIndex) {
        _idleAnimation->setFrameRange(5 * _directionIndex, 5 * _directionIndex + 4);
        _walkAnimation->setFrameRange(5 * _directionIndex, 5 * _directionIndex + 4);
        _attackAnimation->setFrameRange(5 * _directionIndex, 5 * _directionIndex + 4);
        _stunAnimation->setFrameRange(5 * _directionIndex, 5 * _directionIndex + 4);
    }
}

void ParryEnemy::hit(cugl::Vec2 atkDir, float damage, float knockback_scl) {
    //this enemy type takes much less damage when not stunned
    if (isStunned()) Enemy::hit(atkDir, damage, knockback_scl);
    else Enemy::hit(atkDir, damage*GameConstants::PARRY_ENEMY_DR, knockback_scl);
};