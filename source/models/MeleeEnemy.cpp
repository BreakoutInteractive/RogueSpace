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


bool MeleeEnemy::init(std::shared_ptr<JsonValue> data) {
    Enemy::init(data);
    _attackRange = GameConstants::ENEMY_MELEE_ATK_RANGE;
    
    return true;
}

void MeleeEnemy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    Enemy::draw(batch);
    if (getBehaviorState() == BehaviorState::STUNNED) {
        drawEffect(batch, _stunEffect);
    }
}

#pragma mark -
#pragma mark Animation

void MeleeEnemy::setIdling() {
    Enemy::setIdling();
    _stunAnimation->reset();
}

void MeleeEnemy::setMoving() {
    Enemy::setMoving();
    _stunAnimation->reset();
}

void MeleeEnemy::setAttacking() {
    Enemy::setAttacking();
    _stunAnimation->reset();
}

void MeleeEnemy::setStunned() {
    if (getBehaviorState() == BehaviorState::STUNNED) {
        return;
    }
    setAnimation(_stunAnimation);
    _atkCD.reset(); // stunning should reset attack
    _attackAnimation->reset();
    _hitboxAnimation->reset();
    _idleAnimation->reset();
    _walkAnimation->reset();
    Enemy::setStunned();
    _stunEffect->start();
    _stunAnimation->start();
}

void MeleeEnemy::updateAnimation(float dt){
    GameObject::updateAnimation(dt);
    // attack animation must play to completion, as long as enemy is alive.
    if (!_attackAnimation->isActive()) {
        if ((getCollider()->getLinearVelocity().isZero() && !_stunAnimation->isActive()) && _currAnimation != _idleAnimation) {
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
    else if (getBehaviorState() == BehaviorState::STUNNED && !_stunAnimation->isActive()) {
        _tint = Color4::WHITE;
        setIdling();
    }
    else {
        _tint = Color4::WHITE;
    }
    _stunEffect->update(dt);
    _hitboxAnimation->update(dt);
}
