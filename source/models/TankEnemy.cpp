//
//  TankEnemy.cpp
//  RS
//
//  Created by Dylan McIntyre on 4/22/24.
//

#include "TankEnemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "Player.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool TankEnemy::init(std::shared_ptr<JsonValue> data) {
    MeleeEnemy::init(data);
    _attackRange = GameConstants::ENEMY_MELEE_ATK_RANGE*0.8f;
    _attack->setRadius(_attackRange);
    return true;
}


#pragma mark -
#pragma mark Physics

void TankEnemy::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager>& assets) {
    Vec2 direction = getFacingDir();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0) {
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }

    _attack->setPosition(_attack->getPosition() + Vec2(0, (_pixelHeight/2) / _drawScale.y)); //64 is half of the pixel height of the enemy
    _attack->setAngle(ang);
}


#pragma mark -
#pragma mark Animation

void TankEnemy::loadAssets(const std::shared_ptr<AssetManager>& assets) {
    MeleeEnemy::loadAssets(assets);
    auto idleTexture = assets->get<Texture>("tank-idle");
    auto walkTexture = assets->get<Texture>("tank-idle");
    auto attackTexture = assets->get<Texture>("tank-attack");
    auto stunTexture = assets->get<Texture>("tank-stun");
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto stunEffect = assets->get<Texture>("stun-effect");
    auto deathEffect = assets->get<Texture>("enemy-death-effect");

    auto idleSheet = SpriteSheet::alloc(idleTexture, 8, 5);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 5);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 8);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 6);
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto stunEffectSheet = SpriteSheet::alloc(stunEffect, 2, 4);
    auto deathEffectSheet = SpriteSheet::alloc(deathEffect, 2, 4);

    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 4);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 4);
    _attackAnimation = Animation::alloc(attackSheet, GameConstants::ENEMY_MELEE_ATK_SPEED, false, 0, 7);
    _stunAnimation = Animation::alloc(stunSheet, 1.25f, false, 0, 5);
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _stunEffect = Animation::alloc(stunEffectSheet, 0.333f, true);
    _deathEffect = Animation::alloc(deathEffectSheet, 1.0f, false);

    _currAnimation = _idleAnimation; // set runnning

    // add callbacks
    _attackAnimation->onComplete([this]() {
        _attackAnimation->reset();
        _hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
        });

    _attackAnimation->addCallback(GameConstants::ENEMY_MELEE_ATK_SPEED * 0.7f, [this]() {
        if (isEnabled() && _health > 0) {
            _attack->setEnabled(true);
            _hitboxAnimation->start();
            _attack->setAwake(true);
            _attack->setAngle(getFacingDir().getAngle());
            // TODO: clean this code
            _attack->setPosition(getPosition() + Vec2(0, (_pixelHeight/2) / getDrawScale().y)); //64 is half of the enemy pixel height
        }
        });

    setAnimation(_idleAnimation);

    _meleeHitEffect->onComplete([this]() {
        _meleeHitEffect->reset();
    });
    _bowHitEffect->onComplete([this]() {
        _bowHitEffect->reset();
    });
    _deathEffect->onComplete([this]() {
        _deathEffect->reset();
        setEnabled(false);
    });
}


void TankEnemy::setFacingDir(cugl::Vec2 dir) {
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
        _attackAnimation->setFrameRange(8 * _directionIndex, 8 * _directionIndex + 7);
        _stunAnimation->setFrameRange(6 * _directionIndex, 6 * _directionIndex + 5);
    }
}

void TankEnemy::hit(cugl::Vec2 atkDir, bool ranged, float damage, float knockback_scl) {
    //this enemy type takes much less damage and no knockback when not stunned
    //need to scale down by the stun damage bonus so that it's not doubly applied
    if (isStunned()) Enemy::hit(atkDir, ranged, damage/GameConstants::STUN_DMG_BONUS, knockback_scl);
    else Enemy::hit(atkDir, ranged, damage*GameConstants::TANK_ENEMY_DR, 0);
};
