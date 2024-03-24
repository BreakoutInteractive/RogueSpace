#include "AIController.hpp"
#include "../models/LevelModel.hpp"
#include "../models/Enemy.hpp"
#include "../models/MeleeEnemy.hpp"
#include "../models/RangedEnemy.hpp"
#include "../models/Player.hpp"
#include "../models/CollisionConstants.hpp"

using namespace cugl;

void AIController::init(std::shared_ptr<LevelModel> level) {
    _world = level->getWorld();
    _enemies = level->getEnemies(); // set enemies here ?
    _player = level->getPlayer();
    // will have to get set from level, need to change levelmodel interface.
    
}

AIController::~AIController(){
    _world = nullptr;
    _enemies.clear();
}

cugl::Vec2 AIController::lineOfSight(std::shared_ptr<Enemy> e, std::shared_ptr<Player> p) {
    // use raycasting for LOS
    float rayLength = e->getSightRange();
    Vec2 rayStart = e->getPosition();
    Vec2 player = Vec2::ZERO;
    float playerFraction;
    float obstacleFraction;
    float angle = e->getFacingDir().getAngle();
    Vec2 rayEnd = Vec2();
    
    // callback function for ObstacleWorld rayCast()
    std::function<float(b2Fixture *, const Vec2, const Vec2, float)> callback;
    callback = [&player, &playerFraction, &obstacleFraction](b2Fixture *fixture, const Vec2 point, const Vec2 normal, float fraction) {
        // track if we see the player
        if (fixture->GetFilterData().categoryBits == CATEGORY_PLAYER) {
            player = point;
            playerFraction = fraction;
            return 1.0f;
        }
        // track if we see an obstacle
        else if (fixture->GetFilterData().categoryBits == CATEGORY_WALL) {
            if (fraction < obstacleFraction) {
                obstacleFraction = fraction;
            }
            return 1.0f;
        }
        else return -1.0f;
    };
    
    // find an intersection not interrupted by a wall
    for (int i = -30; i <= 30; i+=5) {
        player = Vec2::ZERO;
        playerFraction = 2;
        obstacleFraction = 2;
        float rayAngle = angle + (M_PI/180)*i;
        rayEnd = rayStart + rayLength * Vec2(cosf(rayAngle), sinf(rayAngle));
        
        _world->rayCast(callback, rayStart, rayEnd);
        
        // did ray hit player before hitting any obstacle?
        if (!player.isZero() && playerFraction < obstacleFraction) {
            e->setPlayerInSight(true);
            e->setPlayerLoc(player);
            e->setDefault(false);
            return player;
        }
    }
    
    // no valid player intersections found
    e->setPlayerInSight(false);
    return Vec2::ZERO;
}

void AIController::update(float dt) {
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
        //enemies shouldn't move when stunned or while attacking
        if ((*it)->isAttacking()) {
            (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
        }
        if (!(*it)->isStunned() && !(*it)->isAttacking()) {
            Vec2 intersection = lineOfSight((*it), _player);
            // enemies should follow the player if they see them
            if (!intersection.isZero()) {
                //CULog("Player spotted!");
                (*it)->setGoal(intersection);
                Vec2 dir = intersection - (*it)->getPosition();
                dir.normalize();
                (*it)->setFacingDir(dir);
                (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
            }
            else {
                // CULog("Line of sight: %s", lineOfSight((*it), _player).toString().c_str());
                // enemies within proximity but without LOS should stop and face the player
                if (_player->getPosition().distance((*it)->getPosition()) <= (*it)->getProximityRange()) {
                    Vec2 dir = _player->getPosition() - (*it)->getPosition();
                    dir.normalize();
                    (*it)->setFacingDir(dir);
                    (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                }
                else {
                    // enemies without any LOS/proximity should track the player's last known position (if it exists) and go there (unless already there)
                    if (!(*it)->isDefault() && !(*it)->getPlayerLoc().isZero() && (*it)->getPosition().distance((*it)->getPlayerLoc()) > 0.1){
                        Vec2 dir = (*it)->getPlayerLoc() - (*it)->getPosition();
                        dir.normalize();
                        (*it)->setFacingDir(dir);
                        (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                    }
                    // otherwise, enemies resume default behavior
                    else {
                        (*it)->setDefault(true);
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
                                Vec2 dir = (*it)->getGoal() - (*it)->getPosition();
                                dir.normalize();
                                (*it)->setFacingDir(dir);
                                (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                            }
                            // CULog("Patrol position: %f, %f", (*it)->getPosition().x, (*it)->getPosition().y);
                            else if ((*it)->_hitCounter.getCount() < (*it)->_hitCounter.getMaxCount() - 5) {
                                int prevIdx = (*it)->getPathIndex() - 1;
                                if (prevIdx < 0) prevIdx = (*it)->getPath().size() - 1;
                                // Vec2 prev = (*it)->getPath().at(prevIdx);
                                // Vec2 dest = (*it)->getGoal();
                                // float scl = prev.distance(dest);
                                Vec2 dir = (*it)->getGoal() - (*it)->getPosition();
                                dir.normalize();
                                (*it)->setFacingDir(dir);
                                // dir *= scl;
                                (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                            }
                        }
                    }
                }
            }
        }
    }
    // TODO: implement the following
    // if enemy has LOS of player
    //      move along shortest path to player (determined by raycast)
    // otherwise
    //      move to the player's last seen position, stop, and look around for a bit (do we want this?) before either
    //          moving back to default position (for sentries) or
    //          moving back to the closest node on their patrol path (for patrol enemies)
}
