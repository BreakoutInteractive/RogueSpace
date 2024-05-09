//
//  HealthPack.hpp
//  RS
//
//  Created by Miguel Amor on 5/9/24.
//

#ifndef HealthPack_hpp
#define HealthPack_hpp

#include <cugl/cugl.h>
#include "GameObject.hpp"

class Animation;

/**
 *  This class represents a health pack drop.
 */
class HealthPack : public GameObject {
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(HealthPack);

    std::shared_ptr<Texture> _texture;

public:
    /** flag that indicates whether this is marked for deletion */
    bool _delMark;
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new health pack at the origin.
     */
    HealthPack(void) : GameObject() { }

    /**
     * Destroys this health pack, releasing all resources.
     */
    virtual ~HealthPack(void) { dispose(); }

    /**
     * Disposes all resources and assets
     *
     * Any assets owned by this object will be immediately released.
     * Requires initialization before next use.
     */
    void dispose();

    /**
     * Initializes a new health pack at the given position
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(Vec2 pos, const std::shared_ptr<AssetManager>& assets);


#pragma mark Static Constructors

    /**
     * allocate a new healthpack at the given position
     *
     * @return a newly allocated health pack with the given position
     */
    static std::shared_ptr<HealthPack> alloc(Vec2 pos, const std::shared_ptr<AssetManager>& assets) {
        auto result = std::make_shared<HealthPack>();
        return (result->init(pos, assets) ? result : nullptr);
    }

#pragma mark -
#pragma mark Animation and State

    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

};
#endif /* HealthPack_hpp */