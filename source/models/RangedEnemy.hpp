//
//  RangedEnemy.hpp
//  RS
//
//  Created by Dylan McIntyre on 3/23/24.
//

#ifndef RangedEnemy_hpp
#define RangedEnemy_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "Enemy.hpp"
#include "GameObject.hpp"

class Animation;

/**
 *  This class represents a ranged enemy in the game.
 */
class RangedEnemy : public Enemy {
    
protected:
    
    /** The animation to use while charging attack */
    std::shared_ptr<Animation> _chargingAnimation;
    
    /** Whether the enemy is aiming its ranged attack */
    bool _isAiming;
    /** Whether the enemy's ranged attack is charged */
    bool _isCharged;
    
public:

    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    void updateAnimation(float dt) override;

#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    RangedEnemy(void) : Enemy() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~RangedEnemy(void) { dispose(); }
    
    /**
     * Initializes a new player with the given position and size.
     *
     * The player size is specified in world coordinates.
     *
     * @param  pos      Initial position in world coordinates
     * @param  size       The dimensions of the box.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(std::shared_ptr<JsonValue> data) override;
    
#pragma mark Static Constructors
    
    /**
     * Returns a newly allocated player with the given position and size
     *
     * The player size is specified in world coordinates.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The dimensions of the box.
     *
     * @return a newly allocated player with the given position
     */
    static std::shared_ptr<RangedEnemy> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<RangedEnemy>();
        return (result->init(data) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Accessors
    
    /**
     * Gets whether this enemy's ranged attack is charged
     */
    bool getCharged() const { return _isCharged; }
    
    /**
     * Sets whether this enemy's ranged attack is charged
     */
    void setCharged(bool value) { _isCharged = value; }
    
    /**
     * Gets whether this enemy is aiming its ranged attack
     */
    bool getAiming() const { return _isAiming; }
    
    /**
     * Sets whether this enemy is aiming its ranged attack
     */
    void setAiming(bool value) { _isAiming = value; }
    
    bool canBeginNewAttack() override {
        return !isAttacking() && _atkCD.isZero();
    }
};

#endif /* RangedEnemy_hpp */
