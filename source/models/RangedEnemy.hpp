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
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(RangedEnemy);
    
protected:
    // TODO: change this to projectile when implemented
    std::shared_ptr<cugl::physics2::WheelObstacle> _projectile;
    
    /** The enemy projectile texture*/
    std::shared_ptr<cugl::Texture> _projectileTexture;
    
    /** The enemy projectile speed*/
    float _projectileSpeed;

    /** The animation to use while charging attack */
    std::shared_ptr<Animation> _chargingAnimation;
    
public:

    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

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
     * Disposes all resources and assets
     *
     * Any assets owned by this object will be immediately released.
     * Requires initialization before next use.
     */
    void dispose();
    
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
};

#endif /* RangedEnemy_hpp */
