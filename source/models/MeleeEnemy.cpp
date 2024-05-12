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
