//
//  BossEnemy.cpp
//  RS
//
//  Created by Dylan McIntyre on 5/14/24.
//

#include "BossEnemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "Player.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors

bool BossEnemy::init(std::shared_ptr<JsonValue> data) {
    MeleeEnemy::init(data);
    _secondAttack = false;
    return true;
}

#pragma mark -
#pragma mark Physics

void BossEnemy::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    Vec2 direction = level->getPlayer()->getPosition() * level->getPlayer()->getDrawScale() - getPosition() * getDrawScale();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0){
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    
    _attack->setPosition(_attack->getPosition().add(0, 64 / _drawScale.y)); //64 is half of the pixel height of the enemy
    _attack->setAngle(ang);
}

void BossEnemy::attack2(const std::shared_ptr<AssetManager> &assets) {
    Vec2 direction = getFacingDir();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0){
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    
    _attack->setPosition(_attack->getPosition().add(0, 64 / _drawScale.y)); //64 is half of the pixel height of the enemy
    _attack->setAngle(ang);
}


#pragma mark -
#pragma mark Animation

void BossEnemy::loadAssets(const std::shared_ptr<AssetManager> &assets){
    MeleeEnemy::loadAssets(assets);
    auto idleTexture = assets->get<Texture>("boss-idle");
    auto walkTexture = assets->get<Texture>("boss-walk");
    auto attackTexture = assets->get<Texture>("boss-attack-1");
    auto attackTexture2 = assets->get<Texture>("boss-attack-2");
    auto stunTexture = assets->get<Texture>("boss-idle"); // same as idle for now
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto stunEffect = assets->get<Texture>("stun-effect");
    auto deathEffect = assets->get<Texture>("enemy-death-effect");
    
    auto idleSheet = SpriteSheet::alloc(idleTexture, 8, 9);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 6);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 9);
    auto attackSheet2 = SpriteSheet::alloc(attackTexture2, 8, 9);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 9);
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto stunEffectSheet = SpriteSheet::alloc(stunEffect, 2, 4);
    auto deathEffectSheet = SpriteSheet::alloc(deathEffect, 2, 4);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 8);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 5);
    _attackAnimation = Animation::alloc(attackSheet, GameConstants::ENEMY_MELEE_ATK_SPEED, false, 0, 8);
    _attackAnimation2 = Animation::alloc(attackSheet2, GameConstants::ENEMY_MELEE_ATK_SPEED, false, 0, 8);
    _stunAnimation = Animation::alloc(stunSheet, 1.25f, false, 0, 8);
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _stunEffect = Animation::alloc(stunEffectSheet, 0.333f, true);
    _deathEffect = Animation::alloc(deathEffectSheet, 1.0f, false);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _secondAttack = true;;
        _attackAnimation->reset();
        _hitboxAnimation->reset();
//         _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
    });
    
    _attackAnimation->addCallback(GameConstants::ENEMY_MELEE_ATK_SPEED * 2 / 3, [this](){
        if (isEnabled() && _health > 0) {
            _attack->setEnabled(true);
            _hitboxAnimation->start();
            _attack->setAwake(true);
            _attack->setAngle(getFacingDir().getAngle());
            _attack->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the enemy pixel height
        }
    });
    
    _attackAnimation2->onComplete([this](){
        _attackAnimation2->reset();
        _hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
        _secondAttack = false;
    });
    
    _attackAnimation2->addCallback(GameConstants::ENEMY_MELEE_ATK_SPEED * 2 / 3, [this](){
        if (isEnabled() && _health > 0) {
            _attack->setEnabled(true);
            _hitboxAnimation->start();
            _attack->setAwake(true);
            _attack->setAngle(getFacingDir().getAngle());
            _attack->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the enemy pixel height
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

void BossEnemy::setAttacking2() {
    setAnimation(_attackAnimation2);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _walkAnimation->reset();
    _stunAnimation->reset();
    _stunEffect->reset();
    _state = BehaviorState::ATTACKING;
}

void BossEnemy::setFacingDir(cugl::Vec2 dir) {
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
        _walkAnimation->setFrameRange(6 * _directionIndex, 6 * _directionIndex + 5);
        _attackAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
        _attackAnimation2->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
        _stunAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
    }
}

void BossEnemy::updateAnimation(float dt){
    Enemy::updateAnimation(dt);
    // attack animation must play to completion, as long as enemy is alive.
    if (!_attackAnimation->isActive() && !_attackAnimation2->isActive()) {
        if ((getCollider()->getLinearVelocity().isZero() && !_stunAnimation->isActive()) && _currAnimation != _idleAnimation) {
            setIdling();
        }
        else if (!getCollider()->getLinearVelocity().isZero() && _currAnimation != _walkAnimation) {
            setMoving();
        }
    }
        
    if (getBehaviorState() == BehaviorState::STUNNED && !_stunAnimation->isActive()) {
        _tint = Color4::WHITE;
        setIdling();
    }
    _stunEffect->update(dt);
    _hitboxAnimation->update(dt);
}
