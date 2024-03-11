#include "AIController.hpp"
#include "../models/LevelModel.hpp"
#include "../models/Enemy.hpp"
#include "../models/Player.hpp"

using namespace cugl;

void AIController::init(std::shared_ptr<LevelModel> level) {
    _world = level->getWorld();
    _enemies = level->getEnemies(); // set enemies here ?
    // will have to get set from level, need to change levelmodel interface.
    
}

AIController::~AIController(){
    _world = nullptr;
    _enemies.clear();
}

// this should return a bool(?)
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
        e->getCollider()->setLinearVelocity(0, 0);
    }
    return Vec2(0, 0);
}

void AIController::update(float dt) {
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
        //enemies shouldn't move when stunned
        if ((*it)->_stunCD.isZero()) {
            // make sentries rotate 45 degrees counterclockwise (?) at set intervals
            if ((*it)->getDefaultState() == "sentry") {
                if ((*it)->_sentryCD.isZero()) {
                    (*it)->_sentryCD.reset();
                    (*it)->setFacingDir((*it)->getFacingDir().rotate(M_PI_4));
                    // CULog("Sentry direction: %f, %f", (*it)->getFacingDir().x, (*it)->getFacingDir().y);
                }
                if ((*it)->_hitCounter.getCount() < (*it)->_hitCounter.getMaxCount() - 5) (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
            }
            // make patrolling enemies go to the next location on their patrol route
            if ((*it)->getDefaultState() == "patrol") {
                if ((*it)->getPosition().distance((*it)->getGoal()) <= 0.1) {
                    (*it)->setPathIndex(((*it)->getPathIndex() + 1) % (*it)->getPath().size());
                    (*it)->setGoal((*it)->getPath()[(*it)->getPathIndex()]);
                    // velocity currently based on distance from goal, may need adjusting
                    (*it)->getCollider()->setLinearVelocity((*it)->getGoal().x - (*it)->getPosition().x, (*it)->getGoal().y - (*it)->getPosition().y);
                }
                // CULog("Patrol position: %f, %f", (*it)->getPosition().x, (*it)->getPosition().y);
                else if ((*it)->_hitCounter.getCount() < (*it)->_hitCounter.getMaxCount() - 5) {
                    int prevIdx = (*it)->getPathIndex() - 1;
                    if (prevIdx < 0) prevIdx = (*it)->getPath().size() - 1;
                    Vec2 prev = (*it)->getPath().at(prevIdx);
                    Vec2 dest = (*it)->getGoal();
                    float scl = prev.distance(dest);
                    Vec2 dir = (*it)->getGoal() - (*it)->getPosition();
                    dir.normalize();
                    dir *= scl;
                    (*it)->getCollider()->setLinearVelocity(dir);
                }
            }
        }
    }
    // TODO: implement the following
    // if enemy has LOS of player
    //      move along shortest path to player
    // otherwise
    //      stop and look around for a bit (do we want this?) before either
    //          moving back to default position (for sentries) or
    //          moving back to the closest node on their patrol path (for patrol enemies)
}
