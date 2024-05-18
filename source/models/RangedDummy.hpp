//
//  RangedDummy.hpp
//  RS
//
//  Created by Dylan McIntyre on 5/17/24.
//

#ifndef RangedDummy_hpp
#define RangedDummy_hpp

#include <cugl/cugl.h>
#include "Enemy.hpp"
#include "GameObject.hpp"
#include "../components/Animation.hpp"
#include "../components/Hitbox.hpp"

/**
 *  This class represents a melee dummy enemy in the game.
 */
class RangedDummy : public Enemy {
    
public:

    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    
    void loadAssets(const std::shared_ptr<cugl::AssetManager> &assets) override;
    
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    RangedDummy(void) : Enemy() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~RangedDummy(void) { dispose(); }
    
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
    static std::shared_ptr<RangedDummy> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<RangedDummy>();
        return (result->init(data) ? result : nullptr);
    }
        
#pragma mark -
#pragma mark Accessors
    
    std::string getType() override { return "ranged dummy"; }
        
#pragma mark -
#pragma mark Animation and State
        
    bool canBeginNewAttack() override {
        return false;
    }
        
    void updateAnimation(float dt) override;
        
    /**
     * Method to call when an enemy is hit by an attack
     * @param atkDir the normal vector of the direction of the attack that hit this enemy
     * @param ranged whether the attack that hit this enemy was a ranged attack
     * @param damage how much damage this enemy takes
     * @param knockback_scl the factor to multiply the direction by for applying knockback
    */
    void hit(cugl::Vec2 atkDir, bool ranged, float damage = 1, float knockback_scl = GameConstants::KNOCKBACK) override;
};

#endif /* RangedDummy_hpp */
