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

using namespace cugl;

class LevelParser {
protected:
    int _tilewidth;
    
    int _tileheight;
    
    int _width;
    
    int _height;
    
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
    
    // TODO: come up with a better name please
    /**
     * Returns correct translated values from Tiled origin to game origin
     * Note that inputs/outputs describe lengths of tiles; the given x/y coordinate used in Tiled must be converted (divide by tile length) to use this
     * @param w  the isometric x coordinate to be converted
     * @param h the isometric y coordinate to be converted
     * @param arr whether to return Array or Object 
     *
     */
    const std::shared_ptr<JsonValue> translateJson(float w, float h, bool arr);
    
    /**
     * Parses the three floor view layers 
     *
     * @param layers  the layers JSON node extracted to take floor data from
     * @return an ArrayType node to attach to the main level JSON
     *
     */
    const std::shared_ptr<JsonValue> parseFloor(const std::shared_ptr<JsonValue>& layers);
    
    // TODO: reconsider this spec
    // My current process is that it shouldn't matter what is drawn (or isn't drawn) for the polygonal obstacles created by this layer
    // This layer should always exist and be transmitted for the physics engine
    // Perhaps we can make wall-specific boundaries by specifying it within Tiled, and then deferring these to parseWalls
    // But for now, this should be fine?
    /**
     * Parses all collision boundaries -- includes invisible AND visible walls
     *
     * @param layers  the layers JSON node extracted to take floor data from
     * @return an ArrayType node to attach to the main level JSON
     *
     */
    const std::shared_ptr<JsonValue> parseBoundaries(const std::shared_ptr<JsonValue>& layers);
    
    // TODO: what should I need to do here? let's think about it...
    // I'm relaying the associated image, the location from the json, and the height?
    // Rendering is a models problem, but I'll probably need to jump back here as walls are being implemented...?
    // The 'stickers' probably also go here
    /**
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
    
    // TODO: include path data? very long way away
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
