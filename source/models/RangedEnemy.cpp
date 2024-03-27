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
    
    return true;
}

/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void RangedEnemy::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
}
