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
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(MeleeEnemy);
    
protected:
    // std::shared_ptr<cugl::physics2::WheelObstacle> _attack;
    
public:
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
};

#endif /* MeleeEnemy_hpp */
