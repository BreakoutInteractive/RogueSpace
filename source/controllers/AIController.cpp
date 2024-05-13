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
    _enemies = level->getEnemies();
    _player = level->getPlayer();
    _grid = level->getGrid();
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
            std::shared_ptr<MeleeEnemy> m = std::dynamic_pointer_cast<MeleeEnemy>(e);
            // change state if we're no longer stunned
            if (!m->isStunned()) {
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
        std::shared_ptr<Enemy> enemy = *it;
        changeState(enemy, _player);
        // pass while taking damage to allow for knockback
        if (!enemy->_hitCounter.isZero()) {
            continue;
        }
        Vec2 goal;
        Vec2 dir;
        switch (enemy->getBehaviorState()) {
            case Enemy::BehaviorState::DEFAULT:
                // make sentries rotate 45 degrees counterclockwise at set intervals
                if (enemy->getDefaultState() == "sentry") {
                    if (enemy->_sentryCD.isZero()) {
                        enemy->_sentryCD.reset();
                        enemy->setFacingDir(enemy->getFacingDir().rotate(M_PI_4));
                    }
                    if (enemy->_hitCounter.getCount() < enemy->_hitCounter.getMaxCount() - 5) enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                }
                // make patrolling enemies go to the next location on their patrol route
                if (enemy->getDefaultState() == "patrol") {
                    // enemy is at the center of its tile — we can proceed as normal!
                    if (enemy->getAligned()) {
                        if (enemy->getPosition() == enemy->getGoal()) {
                            enemy->setPathIndex((enemy->getPathIndex() + 1) % enemy->getPath().size());
                            goal = moveToGoal(enemy, enemy->getPath()[enemy->getPathIndex()]);
                            enemy->setGoal(goal);
                            dir = enemy->getGoal() - enemy->getPosition();
                            dir.normalize();
                            enemy->setFacingDir(dir);
                            enemy->getCollider()->setLinearVelocity(enemy->getMoveSpeed()*dir);
                        }
                        else if (enemy->getPosition().distance(enemy->getGoal()) <= 0.1) {
                            enemy->getCollider()->setPosition(enemy->getGoal());
                            enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                        }
                        else if (enemy->_hitCounter.getCount() < enemy->_hitCounter.getMaxCount() - 5) {
                            int newGoalChance = rand() % 10 + 1; // in case enemy gets stuck
                            if (newGoalChance == 1) {
                                goal = moveToGoal(enemy, enemy->getPath()[enemy->getPathIndex()]);
                                enemy->setGoal(goal);
                            }
                            else {
                                goal = enemy->getGoal();
                            }
                            dir = goal - enemy->getPosition();
                            dir.normalize();
                            enemy->setFacingDir(dir);
                            enemy->getCollider()->setLinearVelocity(enemy->getMoveSpeed()*dir);
                        }
                    }
                    // enemy is not aligned — we have to fix that!
                    else {
                        goal = moveToGoal(enemy, enemy->getPosition());
                        enemy->setGoal(goal);
                        if (enemy->getPosition().distance(goal) <= 0.1) {
                            enemy->setAligned(true);
                            enemy->getCollider()->setPosition(goal);
                            enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                            enemy->setGoal(enemy->getPath()[0]);
                        }
                        else {
                            dir = goal - enemy->getPosition();
                            dir.normalize();
                            enemy->setFacingDir(dir);
                            enemy->getCollider()->setLinearVelocity(enemy->getMoveSpeed()*dir);
                        }
                    }
                }
                break;
            case Enemy::BehaviorState::SEEKING:
                if (enemy->getPosition() == enemy->getGoal()) {
                    goal = moveToGoal(enemy, enemy->getAggroLoc());
                    enemy->setGoal(goal);
                    dir = goal - enemy->getPosition();
                    dir.normalize();
                    enemy->setFacingDir(dir);
                    enemy->getCollider()->setLinearVelocity(enemy->getMoveSpeed()*dir);
                }
                if (enemy->getPosition().distance(enemy->getGoal()) <= 0.1) {
                        enemy->getCollider()->setPosition(enemy->getGoal());
                        enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                    }
                else {
                    int newGoalChance = rand() % 10 + 1; // in case enemy gets stuck
                    if (newGoalChance == 1) {
                        goal = moveToGoal(enemy, enemy->getAggroLoc());
                        enemy->setGoal(goal);
                    }
                    else {
                        goal = enemy->getGoal();
                    }
                    dir = goal - enemy->getPosition();
                    dir.normalize();
                    enemy->setFacingDir(dir);
                    enemy->getCollider()->setLinearVelocity(enemy->getMoveSpeed()*dir);
                }
                break;
            case Enemy::BehaviorState::CHASING:
                if (enemy->getPosition().distance(_player->getPosition()) <= enemy->getAttackRange()) {
                    dir = _player->getPosition() - enemy->getPosition();
                    dir.normalize();
                    enemy->setFacingDir(dir);
                    enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                }
                else {
                    if (enemy->getPosition() == enemy->getGoal()) {
                        goal = moveToGoal(enemy, _player->getPosition());
                        enemy->setGoal(goal);
                        dir = goal - enemy->getPosition();
                        dir.normalize();
                        enemy->setFacingDir(dir);
                        enemy->getCollider()->setLinearVelocity(enemy->getMoveSpeed()*dir);
                    }
                    if (enemy->getPosition().distance(enemy->getGoal()) <= 0.1) {
                        enemy->getCollider()->setPosition(enemy->getGoal());
                        enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                    }
                    else {
                        int newGoalChance = rand() % 10 + 1; // in case enemy gets stuck
                        if (newGoalChance == 1) {
                            goal = moveToGoal(enemy, _player->getPosition());
                            enemy->setGoal(goal);
                        }
                        else {
                            goal = enemy->getGoal();
                        }
                        dir = goal - enemy->getPosition();
                        dir.normalize();
                        enemy->setFacingDir(dir);
                        enemy->getCollider()->setLinearVelocity(enemy->getMoveSpeed()*dir);
                    }
                }
                break;
            case Enemy::BehaviorState::ATTACKING:
                if ((enemy->getType() == "ranged lizard" || enemy->getType() == "mage alien")) {
                    std::shared_ptr<RangedEnemy> r = std::dynamic_pointer_cast<RangedEnemy>(enemy);
                    if (r->getAiming()) {
                        dir = _player->getPosition() - enemy->getPosition();
                        dir.normalize();
                        enemy->setFacingDir(dir);
                    }
                }
                enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                break;
            case Enemy::BehaviorState::STUNNED:
                enemy->getCollider()->setLinearVelocity(Vec2::ZERO);
                break;
        }
    }
}
