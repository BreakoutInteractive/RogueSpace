//
//  MeleeEnemy.cpp
//  RS
//
//  Created by Dylan McIntyre on 3/23/24.
//

#include "MeleeEnemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "../components/Hitbox.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool MeleeEnemy::init(std::shared_ptr<JsonValue> data) {
    Enemy::init(data);
    _attackRange = GameConstants::ENEMY_MELEE_ATK_RANGE;
    this->initAttack();
    // attack setup
    b2Filter filter;
    _attack->setSensor(true);
    _attack->setName("enemy-attack");
    _attack->setBodyType(b2_dynamicBody);
    // this is an attack
    filter.categoryBits = CATEGORY_ATTACK;
    // since it is an enemy's attack, it can collide with the player
    filter.maskBits = CATEGORY_PLAYER_HITBOX;
    _attack->setFilterData(filter);
    return true;
}

void MeleeEnemy::initAttack(){
    _attack = SemiCircleHitbox::alloc(getCollider()->getPosition(), _attackRange);
}

#pragma mark - Rendering

void MeleeEnemy::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    Enemy::loadAssets(assets);
    // load enemy swipe animation
    std::shared_ptr<Texture> t2 = assets->get<Texture>("enemy-swipe");
    std::shared_ptr<SpriteSheet> s2 = SpriteSheet::alloc(t2, 2, 4);
    _hitboxAnimation = Animation::alloc(s2, GameConstants::ENEMY_MELEE_ATK_SPEED / 3, false);
}

void MeleeEnemy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    Enemy::draw(batch);
    if (getBehaviorState() == BehaviorState::STUNNED) {
        drawEffect(batch, _stunEffect);
    }
    if (_attack->isEnabled()) {
        auto sheet = _hitboxAnimation->getSpriteSheet();
        Affine2 atkTrans = Affine2::createRotation(_attack->getAngle() - M_PI_2);
        atkTrans.scale(GameConstants::ENEMY_MELEE_ATK_RANGE / ((Vec2)sheet->getFrameSize() / 2) * _drawScale);
        atkTrans.translate(_attack->getPosition() * _drawScale);
        sheet->draw(batch, Color4::WHITE, Vec2(sheet->getFrameSize().getIWidth() / 2, 0), atkTrans);
    }
}

#pragma mark -
#pragma mark Animation

void MeleeEnemy::setIdling() {
    Enemy::setIdling();
    _stunAnimation->reset();
    _stunEffect->reset();
}

void MeleeEnemy::setMoving() {
    Enemy::setMoving();
    _stunAnimation->reset();
    _stunEffect->reset();
}

void MeleeEnemy::setAttacking() {
    Enemy::setAttacking();
    _stunAnimation->reset();
    _stunEffect->reset();
}

void MeleeEnemy::setStunned(float duration) {
    if (getBehaviorState() == BehaviorState::STUNNED) {
        return;
    }
    _stunAnimation->reset();
    _stunAnimation->setDuration(duration);
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
    Enemy::updateAnimation(dt);
    // attack animation must play to completion, as long as enemy is alive.
    if (!_attackAnimation->isActive()) {
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
