//
//  MeleeLizard.hpp
//  RS
//
//  Created by Dylan McIntyre on 4/22/24.
//

#ifndef MeleeLizard_hpp
#define MeleeLizard_hpp

#include <cugl/cugl.h>
#include "Counter.hpp"
#include "MeleeEnemy.hpp"
#include "GameObject.hpp"

class Animation;

/**
 *  This class represents a melee enemy in the game.
 */
class MeleeLizard : public MeleeEnemy {
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(MeleeLizard);
    
protected:
    // std::shared_ptr<cugl::physics2::WheelObstacle> _attack;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new enemy at the origin.
     */
    MeleeLizard(void) : MeleeEnemy() { }
    
    /**
     * Destroys this player, releasing all resources.
     */
    virtual ~MeleeLizard(void) { dispose(); }
    
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
    static std::shared_ptr<MeleeLizard> alloc(std::shared_ptr<JsonValue> data) {
        auto result = std::make_shared<MeleeLizard>();
        return (result->init(data) ? result : nullptr);
    }

#pragma mark -
#pragma mark Accessors
    
    std::string getType() override { return "melee lizard"; }
    
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
    
};

#endif /* MeleeLizard_hpp */
