//
//  TileLayer.hpp
//  RS
//
//  A TileLayer represents a collection of tiles
//
//  Created by Zhiyuan Chen on 2/25/24.
//

#ifndef TileLayer_hpp
#define TileLayer_hpp

#include <cugl/cugl.h>
#include <vector>
#include <cugl/assets/CUAsset.h>
#include <cugl/io/CUJsonReader.h>

using namespace cugl;

class Tile {
    
protected:
    /** The game position of this tile*/
    cugl::Vec2 _pos;
    
    /** json data reference for this tile (contains information about texture, sizing, position, etc) */
    std::shared_ptr<JsonValue> _jsonData;
    
    /** reference to the texture region */
    std::shared_ptr<cugl::Texture> _texture;
    
    /** the size of the tile (including transparent region) expressed in game units */
    cugl::Vec2 _size;
        
public:
    
#pragma mark Tile Constructors
    /**
     * Constructs a tile at the given world position and draws using the texture data
     *
     * data should be formatted as follows:
     *
     * {
     * x = 1,
     * y = 1.5,
     * width = 2,
     * height = 2.5,
     * assets = { ... }
     * }
     */
    Tile(std::shared_ptr<JsonValue> data);
    
    ~Tile(){
        _jsonData = nullptr;
        _texture = nullptr;
    }
    
    /**
     * allocates a tile with json data
     */
    static std::shared_ptr<Tile> alloc(std::shared_ptr<JsonValue> data) {
        return std::make_shared<Tile>(data);
    }
    
    /**
     * @return the world position of the bottom center of this tile
     */
    Vec2 getPosition(){
        return _pos;
    }
    
#pragma mark Assets and Animation
    
    void loadAssets(const std::shared_ptr<cugl::AssetManager> &assets);
    
    void draw(const std::shared_ptr<cugl::SpriteBatch> &batch, Vec2& drawScale);
    
};

#pragma mark -
/**
 * render layer (collection of tiles). Represents an isometric Tiled grid of individual tiles
 */
class TileLayer {
protected:
    /** The level drawing scale (difference between physics and drawing coordinates) */
    Vec2 _drawScale;
    
    /** The TileLayer tiles */
    std::vector<std::shared_ptr<Tile>> _tiles;
    
public:
#pragma mark Constructors
    /**
     * Creates a new TileLayer set at the origin.
     */
    TileLayer(void){}
    
    /**
     * adds a new tile into this layer
     */
    void addTile(const std::shared_ptr<Tile> tile);
    
    
    /**
     * allocates a new tile layer with no tiles
     */
    static std::shared_ptr<TileLayer> alloc(){
        std::shared_ptr<TileLayer> result = std::make_shared<TileLayer>();
        return result;
    }

#pragma mark Animation and Assets
    
    /**
     * render this layer through the sprite batch.
     */
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
 
    /**
     * Retrieve all needed assets (textures, filmstrips) from the asset directory AFTER all assets are loaded.
     */
    void loadAssets(const std::shared_ptr<cugl::AssetManager>& assets);
    
    /**
     * sets the drawing scale for every tile in this layer
     */
    void setDrawScale(Vec2 scale){ _drawScale = scale; }
    
};

#endif /* TileLayer_hpp */
