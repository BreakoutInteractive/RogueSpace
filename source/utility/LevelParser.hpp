//
//  LevelParser.hpp
//  RS
//
//  Created by Vincent Fong on 3/5/24.
//

#ifndef LevelParser_hpp
#define LevelParser_hpp

#include <stdio.h>
#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>
#include <cugl/io/CUJsonWriter.h>

// TODO: these should probably be abstracted to LevelConstants
// along with the texture names and a few constants for sizes and stuff
#define BL_FIELD            "bottom_left"
#define BR_FIELD            "bottom_right"
#define FLOOR_FIELD         "floor"
#define PLAYER_FIELD        "player"
#define ENEMY_SPAWN_FIELD   "mob_spawns"
#define ENEMY_PATH_FIELD    "paths"

using namespace cugl;

class LevelParser {
protected:
    /** width of (non-staggered) tile */
    int _tilewidth;
    
    /** height of (non-staggered) tile */
    int _tileheight;
    
    /** width of play area */
    int _width;
    
    /** height of play area */
    int _height;
    
    /** parsing map to correctly direct sub-objects from the JSON */
    std::unordered_map<std::string, int> _layermap;
    
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
    
    /**
     * Loads this game level from the source Tiled json
     *
     * @return the json used directly for game
     */
    const std::shared_ptr<JsonValue> preload(const std::string file) {
        std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(file);
        return parseTiled(reader->readJson());
    }
    
    bool readLayermap(const std::shared_ptr<JsonValue>& json);
    
    /**
     * Returns correct translated values from Tiled origin to game origin
     * Note that inputs/outputs describe lengths of tiles; the given x/y coordinate used in Tiled must be converted (divide by tile length) to use this
     * @param w  the isometric x coordinate to be converted
     * @param h the isometric y coordinate to be converted
     *
     */
    const std::shared_ptr<JsonValue> translateJson(float w, float h);
    
    /**
     * Parses the three floor view layers
     *
     * @param layers  the layers JSON node extracted to take floor data from
     * @return an ArrayType node to attach to the main level JSON
     *
     */
    const std::shared_ptr<JsonValue> parseFloor(const std::shared_ptr<JsonValue>& layers);
    
    /**
     * DEPRECATED
     * Parses all collision boundaries -- includes invisible AND visible walls
     *
     * @param layers  the layers JSON node extracted to take floor data from
     * @return an ArrayType node to attach to the main level JSON
     *
     */
    const std::shared_ptr<JsonValue> parseBoundaries(const std::shared_ptr<JsonValue>& layers);
    
    /**
     * DEPRECATED
     * Parses all walls/objects and relay their data
     *
     * @param layers  the layers JSON node extracted to take floor data from
     * @return an ArrayType node to attach to the main level JSON
     *
     */
    const std::shared_ptr<JsonValue> parseWalls(const std::shared_ptr<JsonValue>& layers);
    
    /**
     * Parses specifications for the player
     *
     * @param layers  the layers JSON node extracted to take floor data from
     * @return an ArrayType node to attach to the main level JSON
     *
     */
    const std::shared_ptr<JsonValue> parsePlayer(const std::shared_ptr<JsonValue>& layers);
    
    /**
     * Parses specifications for enemies
     *
     * @param layers  the layers JSON node extracted to take floor data from
     * @return an ArrayType node to attach to the main level JSON
     *
     */
    const std::shared_ptr<JsonValue> parseEnemies(const std::shared_ptr<JsonValue>& layers);
    
    /**
     * Parses Tiled-produced json to game json for level
     * @param json  the Tiled json
     *
     */
    const std::shared_ptr<JsonValue> parseTiled(const std::shared_ptr<JsonValue>& json);
};

#endif /* LevelParser_hpp */
