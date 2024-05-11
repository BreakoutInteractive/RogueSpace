//
//  AIController.hpp
//  RS
//
//  This class is responsible for moving enemies around and modifying enemy internal states.
//  The controller will decide when an enemy should pathfind, follow a player, patrol or remain stationary.
//
//  Created by Vincent Fong on 2/26/24.
//

#ifndef __AI_CONTROLLER_HPP__
#define __AI_CONTROLLER_HPP__

#include <cugl/cugl.h>
#include <stdio.h>


class Player;
class Enemy;
class MeleeEnemy;
class MeleeLizard;
class RangedEnemy;
class RangedLizard;
class MageAlien;
class LevelModel;
class LevelGrid;

class AIController {
private:
    
    // TODO: maybe consider getting the entire level instead? but for steering/movement, the world should suffice.
    /** the game world */
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world;

    /** the list of enemies */
    std::vector<std::shared_ptr<Enemy>> _enemies;
    
    /** the player */
    std::shared_ptr<Player> _player;
    
    /** the level grid */
    std::shared_ptr<LevelGrid> _grid;

public:
#pragma mark -
#pragma mark Constructors

    /**
     * Default Constructor
    */
    AIController(){}

    /**
     * Initializes the controller for the given level
     */
    void init(std::shared_ptr<LevelModel> level);
    
    /**
     * Disposes of all resources/references
     */
    ~AIController();
    
#pragma mark -
#pragma mark Enemy 
    
    //TODO: this needs to be done with ObstacleWorld's raycast instead for accuracy.
    /**
     * Returns a 0,0 vector if the enemy does not have line of sight to the player.
     * Else, returns a unit vector from enemy to player.
     *
     */
    cugl::Vec2 lineOfSight(std::shared_ptr<Enemy> e, std::shared_ptr<Player> p);
    
    /**
     * Returns the first node along the path toward the goal
     */
    cugl::Vec2 moveToGoal(std::shared_ptr<Enemy> e, cugl::Vec2 goal);
    
    /**
     * Changes an enemy's behavior state
     */
    void changeState(std::shared_ptr<Enemy> e, std::shared_ptr<Player> p);
    
    /**
     * TODO: document this function
     * Updates locations and states for all enemies
    */
    void update(float dt);
    
};
    
    
#endif /* __AI_CONTROLLER_HPP__ */
