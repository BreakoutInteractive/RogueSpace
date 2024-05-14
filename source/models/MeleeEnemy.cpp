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
    if (_attack->isEnabled()) {
        auto sheet = _hitboxAnimation->getSpriteSheet();
        Affine2 atkTrans = Affine2::createRotation(_attack->getAngle() - M_PI_2);
        atkTrans.scale(GameConstants::ENEMY_MELEE_ATK_RANGE / ((Vec2)sheet->getFrameSize() / 2) * _drawScale);
        atkTrans.translate(_attack->getPosition() * _drawScale);
        sheet->draw(batch, Color4::WHITE, Vec2(sheet->getFrameSize().getIWidth() / 2, 0), atkTrans);
    }
}