//
//  GameObject.hpp
//  RS
//
//  Created by Zhiyuan Chen on 3/11/24.
//

#ifndef GameObject_hpp
#define GameObject_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"

using namespace cugl;

/**
 * An abstract GameObject is a set of components tied to a given position (transform). Derived classes can instantiate these components and attach them.
 */
class GameObject {
    
protected:
    /**
     * the drawing scale of this object
     */
    Vec2 _drawScale;
    
    /**
     * the game position of this object
     */
    Vec2 _position;
    
    /**
     * the collision object associated with this object
     */
    std::shared_ptr<cugl::physics2::Obstacle> _collider;
    
    /**
     * this component is used to make this gameobject an obstacle to other gameobjects while allowing normal physics simulation
     */
    std::shared_ptr<cugl::physics2::Obstacle> _colliderShadow;
    
    /**
     * the sprite outline (a sensor)
     */
    std::shared_ptr<cugl::physics2::Obstacle> _outlineSensor;

public:
    
#pragma mark -
#pragma mark Constructor
    
    /**
     * constructs a game object at the origin with unit draw scale.
     */
    GameObject(): _drawScale(1.0f, 1.0f), _position(0,0) {}
    
#pragma mark -
#pragma mark Rendering
    /**
     * draws the game object to the screen through the spritebatch
     */
    virtual void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) = 0;
    
    /**
     * Sets the ratio of the player sprite to the physics body
     *
     * The player needs this value to convert correctly between the physics
     * coordinates and the drawing screen coordinates.  Otherwise it will
     * interpret one Box2D unit as one pixel.
     *
     * All physics scaling must be uniform.  Rotation does weird things when
     * attempting to scale physics by a non-uniform factor.
     *
     * @param scale The ratio of the player sprite to the physics body
     */
    virtual void setDrawScale(cugl::Vec2 scale){ _drawScale = scale; }
    
    /**
     * Returns the ratio of the player sprite to the physics body
     *
     * The player needs this value to convert correctly between the physics
     * coordinates and the drawing screen coordinates.  Otherwise it will
     * interpret one Box2D unit as one pixel.
     *
     * All physics scaling must be uniform.  Rotation does weird things when
     * attempting to scale physics by a non-uniform factor.
     *
     * @return the ratio of the player sprite to the physics body
     */
    cugl::Vec2 getDrawScale() const { return _drawScale; }
    
#pragma mark -
#pragma mark Physics
    
    std::shared_ptr<cugl::physics2::Obstacle> getCollider(){
        return _collider;
    }
    
    std::shared_ptr<cugl::physics2::Obstacle> getOutline(){
        return _outlineSensor;
    }
    
    std::shared_ptr<cugl::physics2::Obstacle> getColliderShadow(){
        return _colliderShadow;
    }
    
    /**
     * synchronizes the physics components in a fixed update game loop.
     * The GameObject matches the positions of all collision elements (by default).
     */
    virtual void fixedUpdate();
};

#endif /* GameObject_hpp */
