#include "AIController.hpp"
#include "../models/LevelModel.hpp"
#include "../models/LevelGrid.hpp"
#include "../models/Enemy.hpp"
#include "../models/MeleeEnemy.hpp"
#include "../models/MeleeLizard.hpp"
#include "../models/RangedEnemy.hpp"
#include "../models/RangedLizard.hpp"
#include "../models/MageAlien.hpp"
#include "../models/Player.hpp"
#include "../models/CollisionConstants.hpp"

using namespace cugl;

void AIController::init(std::shared_ptr<LevelModel> level) {
    _world = level->getWorld();
    _enemies = level->getEnemies(); // set enemies here ?
    _player = level->getPlayer();
    _grid = level->getGrid();
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
        else if (fixture->GetFilterData().categoryBits == CATEGORY_TALL_WALL) {
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
            e->setAggroLoc(player);
            return player;
        }
    }
    
    // no valid player intersections found
    e->setPlayerInSight(false);
    return Vec2::ZERO;
}

cugl::Vec2 AIController::moveToGoal(std::shared_ptr<Enemy> e, cugl::Vec2 goal) {
    Vec2 goalTile = _grid->worldToTile(goal);
    std::queue<Vec2> frontier = std::queue<Vec2>();
    Vec2 startTile =_grid->worldToTile(e->getPosition());
    if (startTile == goalTile) {
        return (_grid->tileToWorld(startTile));
    }
    frontier.push(startTile);
    std::set<Vec2> visited = std::set<Vec2>();
    std::map<Vec2, Vec2> parents = std::map<Vec2, Vec2>();
    
    // BFS loop
    while (!frontier.empty()) {
        Vec2 tile = frontier.front();
        frontier.pop();
        std::set<Vec2>::const_iterator val = visited.find(tile);
        // mark tile as visited if it hasn't been marked already
        if (val == visited.end()) {
            visited.insert(tile);
        }
        if (tile == goalTile) {
            Vec2 newGoal = tile;
            while (parents[newGoal] != startTile) {
                newGoal = parents[newGoal];
            }
            return (_grid->tileToWorld(newGoal));
        }
        Vec2 left = Vec2(tile.x - 1, tile.y);
        Vec2 down = Vec2(tile.x, tile.y - 2);
        Vec2 right = Vec2(tile.x + 1, tile.y);
        Vec2 up = Vec2(tile.x, tile.y + 2);
        Vec2 bottomLeft, bottomRight, topRight, topLeft;
        if ((int)tile.y % 2 == 0) {
            bottomLeft = Vec2(tile.x - 1, tile.y - 1);
            bottomRight = Vec2(tile.x, tile.y - 1);
            topRight = Vec2(tile.x, tile.y + 1);
            topLeft = Vec2(tile.x - 1, tile.y + 1);
        }
        else {
            bottomLeft = Vec2(tile.x, tile.y - 1);
            bottomRight = Vec2(tile.x + 1, tile.y - 1);
            topRight = Vec2(tile.x + 1, tile.y + 1);
            topLeft = Vec2(tile.x, tile.y + 1);
        }
        
        val = visited.find(bottomLeft);
        // add bottom left tile to frontier if it is in bounds, walkable, and not yet visited
        if (_grid->getNode(bottomLeft) != 0 && val == visited.end()) {
            frontier.push(bottomLeft);
            parents[bottomLeft] = tile;
            visited.insert(bottomLeft);
        }
        val = visited.find(bottomRight);
        // add bottom right tile to frontier if it is in bounds, walkable, and not yet visited
        if (_grid->getNode(bottomRight) != 0 && val == visited.end()) {
            frontier.push(bottomRight);
            parents[bottomRight] = tile;
            visited.insert(bottomRight);
        }
        val = visited.find(topRight);
        // add top right tile to frontier if it is in bounds, walkable, and not yet visited
        if (_grid->getNode(topRight) != 0 && val == visited.end()) {
            frontier.push(topRight);
            parents[topRight] = tile;
            visited.insert(topRight);
        }
        val = visited.find(topLeft);
        // add top left tile to frontier if it is in bounds, walkable, and not yet visited
        if (_grid->getNode(topLeft) != 0 && val == visited.end()) {
            frontier.push(topLeft);
            parents[topLeft] = tile;
            visited.insert(topLeft);
        }
        val = visited.find(left);
        // add left tile to frontier if it is in bounds, walkable, and not yet visited
        if (_grid->getNode(left) != 0 && _grid->getNode(topLeft) != 0 && _grid->getNode(bottomLeft) != 0 && val == visited.end()) {
            frontier.push(left);
            parents[left] = tile;
            visited.insert(left);
        }
        val = visited.find(down);
        // add down tile to frontier if it is in bounds, walkable, and not yet visited
        if (_grid->getNode(down) != 0 && _grid->getNode(bottomLeft) != 0 && _grid->getNode(bottomRight) != 0 && val == visited.end()) {
            frontier.push(down);
            parents[down] = tile;
            visited.insert(down);
        }
        val = visited.find(right);
        // add right tile to frontier if it is in bounds, walkable, and not yet visited
        if (_grid->getNode(right) != 0 && _grid->getNode(topRight) != 0 && _grid->getNode(bottomRight) != 0 && val == visited.end()) {
            frontier.push(right);
            parents[right] = tile;
            visited.insert(right);
        }
        val = visited.find(up);
        // add up tile to frontier if it is in bounds, walkable, and not yet visited
        if (_grid->getNode(up) != 0 && _grid->getNode(topLeft) != 0 && _grid->getNode(topRight) != 0 && val == visited.end()) {
            frontier.push(up);
            parents[up] = tile;
            visited.insert(up);
        }
    }
    return (_grid->tileToWorld(startTile));
}

void AIController::changeState(std::shared_ptr<Enemy> e, std::shared_ptr<Player> p) {
    Vec2 intersection = lineOfSight(e, p);
    switch (e->getBehaviorState()) {
        case Enemy::BehaviorState::DEFAULT:
            // if we see or are close to the player, chase them
            if (!intersection.isZero() || p->getPosition().distance(e->getPosition()) <= e->getProximityRange()) {
                e->setChasing();
            }
            // if we take damage from the player, search where the damage came from
            else if (!e->_hitCounter.isZero()) {
                e->setSeeking();
                e->setAggroLoc(p->getPosition());
            }
            break;
        case Enemy::BehaviorState::SEEKING:
            // if we find the player, chase them
            if (!intersection.isZero() || p->getPosition().distance(e->getPosition()) <= e->getProximityRange()) {
                e->setChasing();
            }
            // if we reach the aggro location without finding the player, return to default
            else if (e->getPosition() == _grid->tileToWorld(_grid->worldToTile(e->getAggroLoc()))) {
                e->setDefault();
            }
            break;
        case Enemy::BehaviorState::CHASING:
            // if we are out of range of the player, search their last known position
            if (intersection.isZero() && p->getPosition().distance(e->getPosition()) > e->getProximityRange()) {
                e->setSeeking();
            }
            break;
        case Enemy::BehaviorState::ATTACKING:
            // change state if we're no longer attacking
            if (!e->isAttacking()) {
                // if we're still close to the player, chase them
                if (!intersection.isZero() || p->getPosition().distance(e->getPosition()) <= e->getProximityRange()) {
                    e->setChasing();
                }
                // if we're no longer close to the player, search their last known position
                else {
                    e->setSeeking();
                }
            }
            break;
        case Enemy::BehaviorState::STUNNED:
            // change state if we're no longer stunned
            if (!e->isStunned()) {
                // if we're still close to the player, chase them
                if (!intersection.isZero() || p->getPosition().distance(e->getPosition()) <= e->getProximityRange()) {
                    e->setChasing();
                }
                // if we're no longer close to the player, search their last known position
                else {
                    e->setSeeking();
                }
            }
            break;
    }
}

void AIController::update(float dt) {
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
        changeState((*it), _player);
        // pass while taking damage to allow for knockback
        if (!(*it)->_hitCounter.isZero()) {
            continue;
        }
        Vec2 goal;
        Vec2 dir;
        switch ((*it)->getBehaviorState()) {
            case Enemy::BehaviorState::DEFAULT:
                // make sentries rotate 45 degrees counterclockwise at set intervals
                if ((*it)->getDefaultState() == "sentry") {
                    if ((*it)->_sentryCD.isZero()) {
                        (*it)->_sentryCD.reset();
                        (*it)->setFacingDir((*it)->getFacingDir().rotate(M_PI_4));
                    }
                    if ((*it)->_hitCounter.getCount() < (*it)->_hitCounter.getMaxCount() - 5) (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                }
                // make patrolling enemies go to the next location on their patrol route
                if ((*it)->getDefaultState() == "patrol") {
                    // enemy is at the center of its tile — we can proceed as normal!
                    if ((*it)->getAligned()) {
                        if ((*it)->getPosition() == (*it)->getGoal()) {
                            (*it)->setPathIndex(((*it)->getPathIndex() + 1) % (*it)->getPath().size());
                            goal = moveToGoal((*it), (*it)->getPath()[(*it)->getPathIndex()]);
                            (*it)->setGoal(goal);
                            dir = (*it)->getGoal() - (*it)->getPosition();
                            dir.normalize();
                            (*it)->setFacingDir(dir);
                            (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                        }
                        else if ((*it)->getPosition().distance((*it)->getGoal()) <= 0.1) {
                            (*it)->getCollider()->setPosition((*it)->getGoal());
                            (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                        }
                        else if ((*it)->_hitCounter.getCount() < (*it)->_hitCounter.getMaxCount() - 5) {
                            int newGoalChance = rand() % 10 + 1; // in case enemy gets stuck
                            if (newGoalChance == 1) {
                                goal = moveToGoal((*it), (*it)->getPath()[(*it)->getPathIndex()]);
                                (*it)->setGoal(goal);
                            }
                            else {
                                goal = (*it)->getGoal();
                            }
                            dir = goal - (*it)->getPosition();
                            dir.normalize();
                            (*it)->setFacingDir(dir);
                            (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                        }
                    }
                    // enemy is not aligned — we have to fix that!
                    else {
                        goal = moveToGoal((*it), (*it)->getPosition());
                        (*it)->setGoal(goal);
                        if ((*it)->getPosition().distance(goal) <= 0.1) {
                            (*it)->setAligned(true);
                            (*it)->getCollider()->setPosition(goal);
                            (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                            (*it)->setGoal((*it)->getPath()[0]);
                        }
                        else {
                            dir = goal - (*it)->getPosition();
                            dir.normalize();
                            (*it)->setFacingDir(dir);
                            (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                        }
                    }
                }
                break;
            case Enemy::BehaviorState::SEEKING:
                if ((*it)->getPosition() == (*it)->getGoal()) {
                    goal = moveToGoal((*it), (*it)->getAggroLoc());
                    (*it)->setGoal(goal);
                    dir = goal - (*it)->getPosition();
                    dir.normalize();
                    (*it)->setFacingDir(dir);
                    (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                }
                if ((*it)->getPosition().distance((*it)->getGoal()) <= 0.1) {
                        (*it)->getCollider()->setPosition((*it)->getGoal());
                        (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                    }
                else {
                    int newGoalChance = rand() % 10 + 1; // in case enemy gets stuck
                    if (newGoalChance == 1) {
                        goal = moveToGoal((*it), (*it)->getAggroLoc());
                        (*it)->setGoal(goal);
                    }
                    else {
                        goal = (*it)->getGoal();
                    }
                    dir = goal - (*it)->getPosition();
                    dir.normalize();
                    (*it)->setFacingDir(dir);
                    (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                }
                break;
            case Enemy::BehaviorState::CHASING:
                if ((*it)->getPosition().distance(_player->getPosition()) <= 1) {
                    dir = _player->getPosition() - (*it)->getPosition();
                    dir.normalize();
                    (*it)->setFacingDir(dir);
                    (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                }
                else {
                    if ((*it)->getPosition() == (*it)->getGoal()) {
                        goal = moveToGoal((*it), _player->getPosition());
                        (*it)->setGoal(goal);
                        dir = goal - (*it)->getPosition();
                        dir.normalize();
                        (*it)->setFacingDir(dir);
                        (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                    }
                    if ((*it)->getPosition().distance((*it)->getGoal()) <= 0.1) {
                        (*it)->getCollider()->setPosition((*it)->getGoal());
                        (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                    }
                    else {
                        int newGoalChance = rand() % 10 + 1; // in case enemy gets stuck
                        if (newGoalChance == 1) {
                            goal = moveToGoal((*it), _player->getPosition());
                            (*it)->setGoal(goal);
                        }
                        else {
                            goal = (*it)->getGoal();
                        }
                        dir = goal - (*it)->getPosition();
                        dir.normalize();
                        (*it)->setFacingDir(dir);
                        (*it)->getCollider()->setLinearVelocity((*it)->getMoveSpeed()*dir);
                    }
                }
                break;
            case Enemy::BehaviorState::ATTACKING:
                if (((*it)->getType() == "ranged lizard" || (*it)->getType() == "mage alien") && (*it)->getAiming()) {
                    dir = _player->getPosition() - (*it)->getPosition();
                    dir.normalize();
                    (*it)->setFacingDir(dir);
                }
                (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                break;
            case Enemy::BehaviorState::STUNNED:
                (*it)->getCollider()->setLinearVelocity(Vec2::ZERO);
                break;
        }
        // TODO: implement the following
        // if enemy has LOS of player
        //      move along shortest path to player (determined by raycast)
        // otherwise
        //      move to the player's last seen position, stop, and look around for a bit (do we want this?) before either
        //          moving back to default position (for sentries) or
        //          moving back to the closest node on their patrol path (for patrol enemies)
    }
}
