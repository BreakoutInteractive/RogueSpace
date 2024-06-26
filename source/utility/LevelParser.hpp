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
    
    // INTERNAL INTERMEDIATE DATA (this can be modified during the parsing phase of a single map)
    
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
    
    /** maps from identifier to objects in the map */
    std::unordered_map<int, std::shared_ptr<JsonValue>> _objects;
    
private:
    
    /** the set of tilesets (mapped from json name to tileset data structure */
    std::unordered_map<std::string, std::shared_ptr<Tileset>> _sets;
    
#pragma mark -
#pragma mark Internal Parsing Helpers (Parsing)
    
    /**
     * The first pass of parsing the map (run through all layers and see which objects are present, adding them to `_objects` hashmap.
     *
     * This is primarily needed to resolve object references (if one object has a parameter value dependent on the existence of another object, eg. enemies pointing to a path node).
     */
    void cacheAllObjects(std::shared_ptr<JsonValue> layersJson);
    
    /**
     * parses a group layer and produces the data needed to initialize a random or collection layer.
     *
     *  A general group layer or `Collection` will have a list of game data (some of which could be group layers)
     *
     *  A `Random` layer has an associated distribution parameter `cdf` that correspond to the number of elements in the group in addition to carrying the same data as any other group layer.
     */
    const std::shared_ptr<JsonValue> parseGroupLayer(const std::shared_ptr<JsonValue> layer);
    
    /**
     * parses a tiled layer and produces the data needed to initialize a tiled layer (for rendering)
     */
    const std::shared_ptr<JsonValue> parseTiledLayer(const std::shared_ptr<JsonValue> layer);
    
    /**
     * parses an object layer and loads the data needed to initialize various object classes
     */
    const std::shared_ptr<JsonValue> parseObjectLayer(const std::shared_ptr<JsonValue> layer);
    
    /**
     * parses a wall object and produces the corresponding data for Wall model
     */
    const std::shared_ptr<JsonValue> parseWall(const std::shared_ptr<JsonValue>& json);
    
    /**
     * parses a player object and produces the corresponding data for Player model
     */
    const std::shared_ptr<JsonValue> parsePlayer(const std::shared_ptr<JsonValue>& json);
    
    /**
     * parses an enemy object and produces the corresponding data for Enemy model
     */
    const std::shared_ptr<JsonValue> parseEnemy(const std::shared_ptr<JsonValue>& json, std::string enemyType);
    
    /**
     * parses a collider used to denote the regions of gestures
     */
    const std::shared_ptr<JsonValue> parseTutorialColliders(const std::shared_ptr<JsonValue>& json);
    
    /**
     * parses a path (list of nodes) starting with the given node as the first node.
     *
     *  nodes are read by ID and points to one another by path attribute
     */
    const std::shared_ptr<JsonValue> parsePath(const std::shared_ptr<JsonValue>& startNode);
    
    /**
     * parses an object and its physics components
     */
    const std::shared_ptr<JsonValue> parsePhysicsObject(const std::shared_ptr<JsonValue>& objectJson, bool parseAsset, bool parseCollider, bool parseHitbox);
    
    /**
     * wrapper method that invokes either of `parsePolyCollider` or `parseBoxCollider`
     */
    const std::shared_ptr<JsonValue> parseGenericCollider(const std::shared_ptr<JsonValue>& colliderJson, Size tileSize, Vec2 tilePos, Size objectSize);
    
    /**
     * parses a polygon collider component
     * @param colliderJson the raw json data for a polygon
     * @param tileSize the dimensions of the tile (image)
     * @param tilePos where the tile object is placed in cartesian world
     * @param objectSize the size of the object placed (may be the same as tileSize if no scaling applied)
     */
    const std::shared_ptr<JsonValue> parsePolyCollider(const std::shared_ptr<JsonValue>& colliderJson, Size tileSize, Vec2 tilePos, Size objectSize);
    
    /**
     * parses a circular collider component (this is NOT elliptical)
     * @param colliderJson the raw json data for a circle
     * @param tileSize the dimensions of the tile (image)
     * @param tilePos where the tile object is placed in cartesian world
     * @param objectSize the size of the object placed (may be the same as tileSize if no scaling applied)
     */
    const std::shared_ptr<JsonValue> parseCircleCollider(const std::shared_ptr<JsonValue>& colliderJson, Size tileSize, Vec2 tilePos, Size objectSize);
    
    /**
     * parses a box collider component
     * @param colliderJson the raw json data for a box
     * @param tileSize the dimensions of the tile (image)
     * @param tilePos where the tile object is placed in cartesian world
     * @param objectSize the size of the object placed (may be the same as tileSize if no scaling applied)
     */
    const std::shared_ptr<JsonValue> parseBoxCollider(const std::shared_ptr<JsonValue>& colliderJson, Size tileSize, Vec2 tilePos, Size objectSize);

    /**
     * Parses a relic object and produces the corresponding data for Relic model
     */
    const std::shared_ptr<JsonValue> parseRelic(const std::shared_ptr<JsonValue>& json);
    
#pragma mark Internal Parsing Helpers (Asset References)
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
