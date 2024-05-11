//
//  RangedLizard.hpp
//  RS
//
//  Created by Dylan McIntyre on 3/24/24.
//

#ifndef RangedLizard_hpp
#define RangedLizard_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "RangedEnemy.hpp"
#include "GameObject.hpp"

class Animation;

/**
 *  This class represents a ranged enemy in the game.
 */
class RangedLizard : public RangedEnemy {
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(RangedLizard);
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    RangedLizard(void) : RangedEnemy() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~RangedLizard(void) { dispose(); }
    
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
    static std::shared_ptr<RangedLizard> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<RangedLizard>();
        return (result->init(data) ? result : nullptr);
    }

#pragma mark -
#pragma mark Accessors
    
    std::string getType() override { return "ranged lizard"; }
    
#pragma mark -
#pragma mark Physics
    
    void attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) override;
    
#pragma mark -
#pragma mark Animation and State
    
    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets) override;
    
    void updateAnimation(float dt) override;
    
    /**
     * Sets the direction that the enemy is currently facing
     */
    void setFacingDir(cugl::Vec2 dir) override;
    
};


#endif /* RangedLizard_hpp */
