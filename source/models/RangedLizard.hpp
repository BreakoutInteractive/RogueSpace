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
    virtual bool init(const cugl::Vec2 pos, const cugl::Size size);
    
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
    static std::shared_ptr<RangedLizard> alloc(const cugl::Vec2 pos, const cugl::Size size) {
        auto result = std::make_shared<RangedLizard>();
        return (result->init(pos,size) ? result : nullptr);
    }
};


#endif /* RangedLizard_hpp */
