//
//  LevelParser.hpp
//  RS
//
//  Created by Vincent Fong on 3/5/24.
//

#ifndef LevelParser_hpp
#define LevelParser_hpp

#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>
#include <cugl/io/CUJsonWriter.h>
#include "Tileset.hpp"

using namespace cugl;

class LevelParser {
protected:
    
    // INTERNAL INTERMEDIATE DATA (this can be modified during the parsing phase)
    
    /** pixel width of tile */
    int _tileWidth;
    
    /** pixel height of tile */
    int _tileHeight;
    
    /** the number of pixels corresponding to a single box 2d unit */
    float _tileDimension;
    
    /** pixel width of tiled map*/
    float _mapWidth;
    
    /** pixel height of tiled map */
    float _mapHeight;
    
    /** the tilesets used by the given map, sorted by their firstgid */
    std::vector<std::shared_ptr<JsonValue>> _mapTilesets;
    
private:
    
    /** the set of tilesets (mapped from json name to tileset data structure */
    std::unordered_map<std::string, std::shared_ptr<Tileset>> _sets;
    
#pragma mark -
#pragma mark Internal Parsing Helpers
    
    /**
     * load the dependencies of the current tiled map to resolve references
     */
    void parseTilesetDependency(std::shared_ptr<JsonValue> tilesets);
    
    /**
     * find the tileset name for the given global identifier (with flags removed)
     * constructs the corresponding id in the tileset
     * @return the pair containing the name of the tileset (eg. example.json) and the index of the tile in the tileset
     */
    const std::pair<std::string, int> getTilesetNameFromID(int id);
    
    /**
     * find the texture region to draw given the global identifier
     */
    const Tileset::TextureRegionData getRegionFromID(int id);
    
    
    /**
     * parses a tiled layer and produces the data needed to initialize an  tiled layer (for rendering)
     */
    const std::shared_ptr<JsonValue> parseTiledLayer(const std::shared_ptr<JsonValue> layer);
    
public:
#pragma mark -
#pragma mark Constructors

    /**
     * Default Constructor
    */
    LevelParser(){}

    /**
     * Disposes of all resources/references
     */
    ~LevelParser(){}
    
    
#pragma mark
#pragma mark Assets
    
    /**
     * populate parser with preloaded tilesets to assist data parsing of tiles
     */
    void loadTilesets(const std::shared_ptr<AssetManager>& assets);
    
#pragma mark
#pragma mark Utility
    
    /**
     * Parses Tiled-produced json to game json for level
     * @param json  the Tiled json
     *
     */
    const std::shared_ptr<JsonValue> parseTiled(const std::shared_ptr<JsonValue>& json);
};

#endif /* LevelParser_hpp */
