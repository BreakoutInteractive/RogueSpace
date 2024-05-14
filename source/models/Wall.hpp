//
//  Wall.hpp
//  RS
//
//  Created by Zhiyuan Chen on 3/22/24.
//

#ifndef Wall_hpp
#define Wall_hpp

#include <cugl/cugl.h>
#include "GameObject.hpp"
#include "../components/Animation.hpp"

class Wall : public GameObject {
    
protected:
    /** json data reference for this tile (contains information about texture, sizing, position, etc) */
    std::shared_ptr<JsonValue> _jsonData;
    
    /** reference to the texture region */
    std::shared_ptr<cugl::Texture> _texture;
    
    /** the size of the visible object (including transparent region) expressed in game units */
    cugl::Vec2 _size;

    /** animation of the floor arrows */
    std::shared_ptr<Animation> _arrowAnimation;

public:
#pragma mark Constructors
    /**
     * Creates a new wall with the given data, of which the polygon is represented by a set of vertices
     *
     * The polygon is constructed at the given origin (physics position)
     */
    Wall(std::shared_ptr<JsonValue> data, const Poly2& poly, const Vec2 origin);

    /**
     * Destroys this wall, releasing all resources.
     */
    virtual ~Wall(void) {
        _jsonData = nullptr;
        _texture = nullptr;
    }


#pragma mark Animation and Assets

    /**
     * retrieves the texture necessary for rendering
     */
    virtual void loadAssets(const std::shared_ptr<AssetManager> &assets);
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
    
};

#pragma mark -

class EnergyWall : public Wall {
protected:
    
    /** the starting row of the animation */
    int animationIndex;
    
public:
#pragma mark Constructors
    /**
     * Creates a new energy wall with the given data, of which the polygon is represented by a set of vertices
     *
     * The polygon is constructed at the given origin (physics position)
     */
    EnergyWall(std::shared_ptr<JsonValue> data, const Poly2& poly, const Vec2 origin);

#pragma mark Animation
    
    void loadAssets(const std::shared_ptr<AssetManager> &assets) override;
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

#pragma mark Physics

    /**
     * turns off the energy barrier (disables the rendering but keeps the collisions in-tact).
     * The wall becomes a sensor.
     */
    void deactivate();
    
};


#endif /* Wall_hpp */
