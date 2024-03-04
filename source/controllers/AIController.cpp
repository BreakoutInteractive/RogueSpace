#include "AIController.hpp"
#include "../models/LevelModel.h"
#include "../models/Enemy.hpp"
#include "../models/Player.hpp"

using namespace cugl;

void AIController::init(std::shared_ptr<LevelModel> level) {
    _world = level->getWorld();
    // set enemies here ? 
    // will have to get set from level, need to change levelmodel interface.
    
}

AIController::~AIController(){
    // TODO: complete destructor
}

cugl::Vec2 AIController::lineOfSight(std::shared_ptr<Enemy> e, std::shared_ptr<Player> p) {
    Vec2 ePos = e->getPosition();
    Vec2 pPos = p->getPosition();
    
    Vec2 delta = pPos - ePos;
    float range = e->getRange();
    
    // Current stopping point is hardcoded as 2.5 units
    // Change this / factor this out as you wish
    float stopRange = 4.5;
    if (delta.length() < range && delta.length() >= stopRange) {
        for (int i = 0; i < 10; ++i) {
            // We don't have obstacles coded yet, so I can't create obstacles to check for.
            // But the generic algorithm I had was:
            // 1. Check along dividing points (tenths) of path between enemy and player
            // 2. If there is an obstacle in a certain radius to our point, we break the loop and return <0,0>.
            // 3. Else, continue in loop.
        }
        // Make them move towards player
        return delta.normalize();
    } else {
        // If outside of sight radius or too close to player, stop the enemy.
        e->setLinearVelocity(0, 0);
    }
    return Vec2(0, 0);
}

void AIController::update(float dt){
    // TODO: make any 
}
