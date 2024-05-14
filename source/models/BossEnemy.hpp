//
//  BossEnemy.hpp
//  RS
//
//  Created by Dylan McIntyre on 5/14/24.
//

#ifndef BossEnemy_hpp
#define BossEnemy_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "MeleeEnemy.hpp"
#include "GameObject.hpp"

class Animation;

/**
 *  This class represents a boss enemy in the game.
 */
class BossEnemy : public MeleeEnemy {
    
protected:
#pragma mark - Animation Assets
    /** The second animation to use while attacking */
    std::shared_ptr<Animation> _attackAnimation2;
    
    /** Whether the boss is ready for its second melee attack */
    bool _secondAttack;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    BossEnemy(void) : MeleeEnemy() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~BossEnemy(void) { dispose(); }
    
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
    static std::shared_ptr<BossEnemy> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<BossEnemy>();
        return (result->init(data) ? result : nullptr);
    }

#pragma mark -
#pragma mark Accessors
    
    std::string getType() override { return "boss enemy"; }
    
    /** Gets whether the boss is ready for its second melee attack */
    bool secondAttack() { return _secondAttack; }
    
#pragma mark -
#pragma mark Physics
    
    void attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) override;
    
    void attack2(const std::shared_ptr<AssetManager> &assets);
    
#pragma mark -
#pragma mark Animation and State
    
    /** change to using the second attack animation */
    void setAttacking2();
    
    /** whether the boss is attacking */
    bool isAttacking(){ return (_attackAnimation->isActive() || _attackAnimation2->isActive()) && _state == BehaviorState::ATTACKING; }
    
    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets) override;
    
    /**
     * Sets the direction that the enemy is currently facing
     */
    void setFacingDir(cugl::Vec2 dir) override;
    
    void updateAnimation(float dt) override;
};

#endif /* BossEnemy_hpp */
