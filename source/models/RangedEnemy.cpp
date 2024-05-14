//
//  RangedEnemy.cpp
//  RS
//
//  Created by Dylan McIntyre on 3/23/24.
//

#include "RangedEnemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool RangedEnemy::init(std::shared_ptr<JsonValue> data) {
    Enemy::init(data);
    _attackRange = GameConstants::ENEMY_RANGED_ATK_RANGE;
    _isAiming = false;
    _isCharged = false;
    
    return true;
}

void RangedEnemy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    Enemy::draw(batch);
    std::shared_ptr<SpriteSheet>sheet = _chargingAnimation->getSpriteSheet();
    Vec2 o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
    Vec2 dir = getFacingDir();
    float ang = acos(dir.dot(Vec2::UNIT_X));
    if (dir.y < 0) {
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(dir.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    Affine2 t = Affine2::createRotation(ang);
    t.scale(_drawScale / 32);
    t.translate((_position + Vec2(0, 64 / getDrawScale().y)) * _drawScale);
    if (_chargingAnimation->isActive()) sheet->draw(batch, o, t);
}

void RangedEnemy::updateAnimation(float dt) {
    Enemy::updateAnimation(dt);
    // attack animation must play to completion, as long as enemy is alive.
    if (!_attackAnimation->isActive()) {
        if (getCollider()->getLinearVelocity().isZero() && _currAnimation != _idleAnimation) {
            setIdling();
        }
        else if (!getCollider()->getLinearVelocity().isZero() && _currAnimation != _walkAnimation) {
            setMoving();
        }
    }
    _chargingAnimation->update(dt);
}
