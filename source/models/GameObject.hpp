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
#include "../components/Animation.hpp"

using namespace cugl;

/**
 * An abstract GameObject is a set of components tied to a given position (transform). Derived classes can instantiate these components and attach them.
 */
class GameObject {
    
protected:
    
    /** the drawing scale of this object */
    Vec2 _drawScale;
    
    /** the color to tint this object */
    cugl::Color4 _tint;
    
    /** the game position of this object */
    Vec2 _position;
    
    /** whether this object is enabled */
    bool _enabled;
    
    /** the collision object associated with this object */
    std::shared_ptr<cugl::physics2::Obstacle> _collider;
    
    /** this component is used to make this gameobject an obstacle to other gameobjects while allowing normal physics simulation */
    std::shared_ptr<cugl::physics2::Obstacle> _colliderShadow;
    
    /** the sprite outline (a sensor) */
    std::shared_ptr<cugl::physics2::Obstacle> _outlineSensor;
    
    /** the current  animation that is running for this game object */
    std::shared_ptr<Animation> _currAnimation;

public:
    
#pragma mark -
#pragma mark Constructor
    
    /**
     * constructs a game object at the origin with unit draw scale.
     */
    GameObject();
    
#pragma mark -
#pragma mark Rendering
    /**
     * draws the game object to the screen through the spritebatch
     */
    virtual void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) = 0;
    
    /**
     * Sets the ratio of the sprite to the physics body
     *
     * This value is needed to convert correctly between the physics
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
    
    /**
     * updates the animation component (if any) of this game object by `dt` seconds
     *
     * if the animation has not started, it will start the animation instead.
     * @pre the object must be enabled for any updates.
     */
    virtual void updateAnimation(float dt);
    
    /**
     * sets the current animation.
     */
    virtual void setAnimation(std::shared_ptr<Animation> animation) { 
        _currAnimation = animation;
    }
    
public:
#pragma mark -
#pragma mark Physics
    
    virtual std::shared_ptr<cugl::physics2::Obstacle> getCollider(){
        return _collider;
    }
    
    virtual std::shared_ptr<cugl::physics2::Obstacle> getOutline(){
        return _outlineSensor;
    }
    
    virtual std::shared_ptr<cugl::physics2::Obstacle> getColliderShadow(){
        return _colliderShadow;
    }
    
    /**
     * adds all attached physics components to the given world
     */
    virtual void addObstaclesToWorld(std::shared_ptr<physics2::ObstacleWorld> world);
    
    /**
     * remove all attached physics components from the given world
     */
    virtual void removeObstaclesFromWorld(std::shared_ptr<physics2::ObstacleWorld> world);
    
    /**
     * synchronizes the physics components in a fixed update game loop.
     * The GameObject matches the positions of all collision elements (by default).
     */
    virtual void syncPositions();
    
#pragma mark -
#pragma mark Properties
    
    /**
     * Many positions may be associated with a given game object due to variations between hitboxes, sprites, animations, etc.
     * An object may need a separate position for rendering or used as a reference point for components.
     * 
     * When a game object is enabled with physics, the returned position by default is the physics location.
     *
     * Note: you can retrieve positions of components directly.
     *
     */
    virtual Vec2 getPosition() const {
        if (_collider != nullptr){
            return _collider->getPosition();
        }
        return _position;
    }
    
    /**
     * sets all of the physics components (if any) to be enabled/disabled depending on `value`.
     * An disabled object should not be drawn and cannot be interacted with.
     */
    virtual void setEnabled(bool value);

    /**
     * gets whether this object is enabled in the game
     */
    virtual bool isEnabled(){ return _enabled; }
    
#pragma mark -
#pragma mark isometric sorting (comparisons)

    bool operator<(const GameObject& other) const
    {
        Vec2 p2 = other.getPosition();
        Vec2 p1 = getPosition();
        return (p1.y > p2.y ? true : (p1.y < p2.y ? false : p1.x <= p2.x));
    }

};

#endif /* GameObject_hpp */
