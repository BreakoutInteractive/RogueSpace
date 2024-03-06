//
//  CollisionController.hpp
//
//  This controller is primarily responsible for resolving collisions between entities and modifying their states.
//
//  Author: Zhiyuan Chen
//  Version: 3/3/24
//

#ifndef __COLLISION_CONTROLLER_HPP__
#define __COLLISION_CONTROLLER_HPP__
#include <cugl/cugl.h>

class LevelModel;

class CollisionController{

private:

    std::shared_ptr<LevelModel> _level;

public:

#pragma mark -
#pragma mark Accessors and Modifiers


    /**
     * sets the level container and attach listeners to the world
    */
    void setLevel(std::shared_ptr<LevelModel> level);
    
#pragma mark -
#pragma mark Collision Handling
    /**
     * Processes the start of a collision
     *
     * This method is called when we first get a collision between two objects. 
     * We use this method to test if it is the "right" kind of collision.  In 
     * particular, we use it to test if we make it to the win door.
     *
     * @param  contact  The two bodies that collided
     */
    void beginContact(b2Contact* contact);

    /**
     * Handles any modifications necessary before collision resolution
     *
     * This method is called just before Box2D resolves a collision.  We use 
     * this method to implement sound on contact, using the algorithms outlined 
     * in Ian Parberry's "Introduction to Game Physics with Box2D".
     *
     * @param  contact  The two bodies that collided
     * @param  contact  The collision manifold before contact
     */
    void beforeSolve(b2Contact* contact, const b2Manifold* oldManifold);


    // todo: we can add the remaining listener functions for collision detection.

};

#endif /* __COLLISION_CONTROLLER_HPP__ */