//
//  Floor.hpp
//  RS
//
//  A floor represents a collection of tiles that
//
//  Created by Zhiyuan Chen on 2/25/24.
//

#ifndef Floor_hpp
#define Floor_hpp

#include <cugl/cugl.h>
#include <vector>
#include <cugl/assets/CUAsset.h>
#include <cugl/io/CUJsonReader.h>

using namespace cugl;

class Tile {
    
protected:
    /** The game position of this tile*/
    cugl::Vec2 _pos;
    
    std::string _textureKey;
    
    std::string _layer;
    
    /** The player texture*/
    std::shared_ptr<cugl::Texture> _texture;
    
    std::shared_ptr<cugl::SpriteSheet> _activeSide;
    
public:
    
#pragma mark -
#pragma mark Tile Constructors
    /**
     * Constructs a tile at the given world position whose texture is given by the texture name
     */
    Tile(Vec2 pos, std::string textureName);
    
    Tile(Vec2 pos, std::string textureName, std::string layer);
    
    static std::shared_ptr<Tile> alloc(const cugl::Vec2 pos, std::string textureName, std::string layer) {
        return std::make_shared<Tile>(pos, textureName, layer);
    }
    
#pragma mark -
#pragma mark Assets and Animation
    
    void loadAssets(const std::shared_ptr<cugl::AssetManager> &assets);
    
    void draw(const std::shared_ptr<cugl::SpriteBatch> &batch, Vec2 size, Vec2 drawScale);
    
};

class Floor {
protected:
    /** The level drawing scale (difference between physics and drawing coordinates) */
    Vec2 _drawScale;
    
    Vec2 _size;
    
    /** The floor tiles */
    std::vector<std::shared_ptr<Tile>> _tiles;
    
public:
#pragma mark Constructors
    /**
     * Creates a new floor set at the origin.
     */
    Floor(void){}
    
    /**
     * Destroys this object, releasing all resources.
     */
    virtual ~Floor(void) { dispose(); }
    
    /**
     * Disposes all resources and assets
     *
     * Any assets owned by this object will be immediately released.
     * Requires initialization before next use.
     */
    void dispose();
    
    /**
     * Initializes a new floor with the given tile size and tiles.
     *
     * The  size is specified in world coordinates.
     *
     * @return  true if the floor is initialized properly, false otherwise.
     */
    virtual bool init(const cugl::Vec2 tileSize, std::vector<std::shared_ptr<Tile>> tiles);
    
    
#pragma mark Static Constructors

    
    static std::shared_ptr<Floor> alloc(cugl::Size size, std::vector<std::shared_ptr<Tile>> tiles) {
        std::shared_ptr<Floor> result = std::make_shared<Floor>();
        return (result->init(size, tiles) ? result : nullptr);
    }

    
#pragma mark -
#pragma mark Animation and Assets
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
 
    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets);
    
    void setDrawScale(Vec2 scale){ _drawScale = scale; }
    
    
    
};

#endif /* Floor_hpp */
