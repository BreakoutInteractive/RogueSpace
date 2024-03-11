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

using namespace cugl;

class LevelModel;

/**
 *  The motivation to separate this out of GameScene is to let the focus of here to
 *  be dealing with combat collisioos and other collision events
 *  GameScene can focus on player movement, controls, and surface level management.
 */
class CollisionController{

private:

    /** reference to current level */
    std::shared_ptr<LevelModel> _level;

    /** reference to assets directory */
    std::shared_ptr<AssetManager> _assets;

public:

#pragma mark -
#pragma mark Accessors and Modifiers


    /**
     * sets the level container and attach collision callback listeners to the world
    */
    void setLevel(std::shared_ptr<LevelModel> level);

    /**
     * loads the necessary assets for the controller
     */
    void setAssets(const std::shared_ptr<AssetManager>& assets);


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
