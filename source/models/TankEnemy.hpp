//
//  TankEnemy.hpp
//  RS
//
//  Created by Miguel Amor on 4/25/24.
//

#ifndef TankEnemy_hpp
#define TankEnemy_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "Enemy.hpp"
#include "GameObject.hpp"

class Animation;

/**
 *  This class represents a melee enemy in the game. It can only be damaged while stunned
 */
class TankEnemy : public Enemy {
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(TankEnemy);

protected:
    // std::shared_ptr<cugl::physics2::WheelObstacle> _attack;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    TankEnemy(void) : Enemy() { }

    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~TankEnemy(void) { dispose(); }

    /**
     * Disposes all resources and assets
     *
     * Any assets owned by this object will be immediately released.
     * Requires initialization before next use.
     */
    void dispose();

    /**
     * Initializes a new enemy with the given position and size.
     *
     * The enemy size is specified in world coordinates.
     *
     * @param  pos      Initial position in world coordinates
     * @param  size       The dimensions of the box.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(std::shared_ptr<JsonValue> data) override;

#pragma mark Static Constructors

    /**
     * Returns a newly allocated enemy with the given position and size
     *
     * The enemy size is specified in world coordinates.
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
    static std::shared_ptr<TankEnemy> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<TankEnemy>();
        return (result->init(data) ? result : nullptr);
    }

#pragma mark -
#pragma mark Accessors

    std::string getType() override { return "tank enemy"; }

#pragma mark -
#pragma mark Physics

    void attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager>& assets) override;

#pragma mark -
#pragma mark Animation and State

    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets) override;

    /**
     * Sets the direction that the enemy is currently facing
     */
    void setFacingDir(cugl::Vec2 dir) override;

    /**
     * Method to call when an enemy is hit by an attack
     * @param atkDir the normal vector of the direction of the attack that hit this enemy
     * @param damage how much damage this enemy takes
     * @param knockback_scl the factor to multiply the direction by for applying knockback
     */
    void hit(cugl::Vec2 atkDir, float damage = 1, float knockback_scl = GameConstants::KNOCKBACK) override;

};

#endif /* TankEnemy_hpp */
