//
//  RangedLizard.cpp
//  RS
//
//  Created by Dylan McIntyre on 3/24/24.
//

#include "RangedLizard.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool RangedLizard::init(const Vec2 pos, const Size size) {
    RangedEnemy::init(pos, size);
    
    return true;
}

/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void RangedLizard::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
}
