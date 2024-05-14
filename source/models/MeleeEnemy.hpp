//
//  MeleeEnemy.hpp
//  RS
//
//  Created by Dylan McIntyre on 3/23/24.
//

#ifndef MeleeEnemy_hpp
#define MeleeEnemy_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "Enemy.hpp"
#include "GameObject.hpp"

class Animation;

/**
 *  This class represents a melee enemy in the game.
 */
class MeleeEnemy : public Enemy {

protected:
    /** The physics object used by this enemy's melee attack */
    std::shared_ptr<cugl::physics2::WheelObstacle> _attack;
    
#pragma mark - Animation Assets
    /** The animation of the hitbox while attacking */
    std::shared_ptr<Animation> _hitboxAnimation;
    /** The animation to use while stunned */
    std::shared_ptr<Animation> _stunAnimation;
    /** The stun effect animation */
    std::shared_ptr<Animation> _stunEffect;
    
public:

    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    MeleeEnemy(void) : Enemy() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~MeleeEnemy(void) { dispose(); }
    
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
    static std::shared_ptr<MeleeEnemy> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<MeleeEnemy>();
        return (result->init(data) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Accessors
    
    /**
     * Gets this enemy's attack hitbox.
     */
    std::shared_ptr<cugl::physics2::WheelObstacle> getAttack() const { return _attack; }
    
    /**
     * Sets this enemy's attack hitbox.
     */
    void setAttack(std::shared_ptr<cugl::physics2::WheelObstacle> value) { _attack = value; }
    
#pragma mark -
#pragma mark Animation and State

    /** Sets this enemy's attack hitbox animation */
    void setHitboxAnimation(std::shared_ptr<Animation> animation) { _hitboxAnimation = animation; }
    
    /** Gets this enemy's attack hitbox animation */
    std::shared_ptr<Animation> getHitboxAnimation() const { return _hitboxAnimation; }
    
    /** Set idle state and change to using the idle animation */
    void setIdling();
    
    /** Set moving state and change to using the walk animation */
    void setMoving();
    
    /** Set attack state and change to using the attack animation */
    void setAttacking();
    
    /** Set stunned state and change to using the stunned animation */
    void setStunned(float duration);
    
    /** whether enemy is stunned */
    bool isStunned(){ return _stunAnimation->isActive() && getBehaviorState() == BehaviorState::STUNNED; }
    
    void updateAnimation(float dt) override;
};

#endif /* MeleeEnemy_hpp */
