//
//  AIEngine.hpp
//  RS
//
//  Created by Vincent Fong on 2/26/24.
//

#ifndef AIEngine_hpp
#define AIEngine_hpp

#include <cugl/cugl.h>
#include <stdio.h>

#include "../models/Enemy.hpp"
#include "../models/Player.hpp"

/**
 * This class controls enemy movement.
 * I'm hoping it can control things like inital mob motions, and also handle LOS.
 */
class AIEngine {
private:
    /** The window size, for LOS */
    cugl::Size _size;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new AI engine.
     *
     * This constructor does NOT do anything.
     */
    AIEngine() {}
    
    /**
     * Disposes of this AI engine. Not much to do here either.
     */
    ~AIEngine() { }
    
    /**
     * Initializes the AI engine with the given size.
     */
    bool init(cugl::Size size) {
        _size = size;
        return true;
    }
    
#pragma mark -
#pragma mark Processing
    
    /**
     * Returns a 0,0 vector if the enemy does not have line of sight to the player.
     * Else, returns a unit vector from enemy to player.
     *
     */
    cugl::Vec2 lineOfSight(std::shared_ptr<Enemy> e, std::shared_ptr<Player> p);
    
    /**
     * Gives behavior for enemies to walk
     */
    cugl::Vec2 walk(std::shared_ptr<Enemy> e);
    
};
    
    
#endif /* AIEngine_hpp */
