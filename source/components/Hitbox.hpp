//
//  Hitbox.hpp
//  RS
//
//  Created by Zhiyuan Chen on 5/14/24.
//

#ifndef Hitbox_hpp
#define Hitbox_hpp

#include <cugl/cugl.h>

using namespace cugl;

/**
 * This class represents the a circular attack hitbox. During the timeframe between the activation of this hitbox, it records whether there is a successful hit of any targets.
 * This is a separate component attached to entities and the parent entity is responsible for the physics of this object.
 */
class Hitbox : public physics2::WheelObstacle {

private:
    /** the set of entities (their pointers) that the hitbox damaged*/
    std::unordered_set<intptr_t> _hitSet;
    
protected:
    
    /**
     * @note override this if only a particular range of angles can be hit
     * @return whether the given angle
     */
    virtual bool hitsAngle(float angle){ return true; }
    
public:
    
    /**
     * Creates a new hitbox located at the world origin
     */
    Hitbox(void): physics2::WheelObstacle() {}
    
    /**
     * Returns a new hitbox sphere of the given radius.
     *
     * @param  pos      Initial position in world coordinates
     * @param  radius   The hitbox radius
     *
     * @return a new hitbox object of the given radius.
     */
    static std::shared_ptr<Hitbox> alloc(const cugl::Vec2 pos, float radius) {
        std::shared_ptr<Hitbox> result = std::make_shared<Hitbox>();
        return (result->init(pos,radius) ? result : nullptr);
    }
    
    /**
     * Sets whether the body is enabled and clears the previous hits.
     * Any processing of the hit data has to be done between calls to this method before the values are cleared.
     *
     * @param value whether the body is enabled
     */
    void setEnabled(bool value);
    
    /**
     * @param target the memory pointer of the entity (or any unique tag)
     * @param angle the angle between the hitbox and target with the x-axis
     * @return whether to handle the collision between this hitbox and the target
     */
    bool hits(intptr_t target, float angle);
    
    /**
     * @return whether the hitbox has hit any targets (during active time)
     */
    bool hasHits(){ return _hitSet.size() > 0; }
    
    /**
     * @return the number of targets damaged by this hitbox (during active time)
     */
    int hitCount(){ return (int) _hitSet.size(); }
};

/**
 * this class is a special variant of the `Hitbox` class. Instead of hitting all targets in a circular region, this restricts the hitbox to only target a semisphere based on the hitbox angle.
 */
class SemiCircleHitbox : public Hitbox {
   
protected:
    
    bool hitsAngle(float angle) override;
    
public:
    /**
     * Returns a new hitbox arc of the given radius.
     *
     * @param  pos      Initial position in world coordinates
     * @param  radius   The hitbox radius
     *
     * @return a new hitbox object of the given radius.
     */
    static std::shared_ptr<SemiCircleHitbox> alloc(const cugl::Vec2 pos, float radius) {
        std::shared_ptr<SemiCircleHitbox> result = std::make_shared<SemiCircleHitbox>();
        return (result->init(pos,radius) ? result : nullptr);
    }
};

#endif /* Hitbox_hpp */
