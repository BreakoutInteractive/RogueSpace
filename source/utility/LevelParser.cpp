//
//  LevelParser.cpp
//  RS
//
//  Created by Vincent Fong on 3/5/24.
//

#include "LevelParser.hpp"
#include <string>
#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>
#include <cugl/io/CUJsonWriter.h>
#include "Tileset.hpp"
#include "Helper.hpp"

using namespace cugl;

#define TILE_LAYER      "tilelayer"
#define OBJECT_LAYER    "objectgroup"
#define LAYERS_KEY      "layers"
#define TYPE            "type"
#define VISIBLE         "visible"
#define FIRSTGID        "firstgid"


#pragma mark -
#pragma mark Loading Dependencies (Assets)

void LevelParser::loadTilesets(const std::shared_ptr<AssetManager>& assets){
    // TODO: make the following into a loop, using a single json file to store all available tiled sets
    // 1. get the list of all Tiled tileset json files
    // 2. allocate Tileset objects
    std::shared_ptr<JsonValue> loadedAssets = assets->get<JsonValue>("assets-tileset");
    std::shared_ptr<JsonValue> loadedJsonTilesets = loadedAssets->get("jsons");
    CUAssertLog(loadedJsonTilesets != nullptr, "<tilesetName>.json files not specified in correct loading file");
    auto jsons = loadedJsonTilesets->children();
    for (int ii = 0; ii < jsons.size(); ii++){
        std::string name = jsons[ii]->key();
        std::shared_ptr<JsonValue> tilesetJsonFile = assets->get<JsonValue>(name);
        CUAssertLog(tilesetJsonFile != nullptr, "tileset json file not found");
        _sets[name] = std::make_shared<Tileset>(tilesetJsonFile);
    }
}

void LevelParser::parseTilesetDependency(std::shared_ptr<JsonValue> tilesets){
    // Maybe we can do a little more to optimize dependency search in `getTilesetNameFromID`
    _mapTilesets.clear();
    std::vector<std::shared_ptr<JsonValue>> dependencies = tilesets->children();
    _mapTilesets = dependencies;
}

#pragma mark -
#pragma mark Parsing

const std::pair<std::string, int> LevelParser::getTilesetNameFromID(int id){
    std::string name = "";
    int newId = id;
    // iterate over the list of used tilesets
    // find the tileset that has the largest firstgid that is smaller than or equal to the tile’s GID. Once you have identified the tileset, subtract its firstgid from the tile’s GID to get the local ID of the tile within the tileset.
    for (int i = 0; i < _mapTilesets.size(); i++){
        auto json = _mapTilesets[i];
        int firstGid = json->getInt(FIRSTGID);
        if (firstGid <= id){
            newId = id - firstGid;
            name = Helper::fileName(Helper::baseName(json->getString("source")));
        }
        else {
            break;
        }
    }
    CUAssertLog(name != "", "id not valid in map, is the tileset referenced by the map?");
    return std::make_pair(name, newId);
}

const std::shared_ptr<JsonValue> LevelParser::parseTiledLayer(const std::shared_ptr<JsonValue> layer){
    
    std::shared_ptr<JsonValue> tiledLayerData = JsonValue::allocArray();
    
    // staggered maps arrange in even - odd row patterns
    // if no flipping of tiles is ever needed, the matrix might as well be an int array.
    auto matrix = layer->get("data")->asLongArray();
    int height = layer->getInt("height");
    int width = layer->getInt("width");
    for (int h = 0; h < height; h++){
        for (int w = 0; w < width; w++){
            int idx = h * width + w;
            long gid = matrix[idx];
            if (gid != 0){
                // bottom LEFT coordinates of tile (later adjusted so that we get the bottom CENTER)
                float x,y;
                if (h % 2 == 0){
                    x = w * _tileWidth;
                    y = (h / 2 + 1) * _tileHeight;
                }
                else {
                    x = (w + 0.5) * _tileWidth;
                    y = (h / 2 + 1.5) * _tileHeight;
                }
                y = _mapHeight - y; // tile coordinates are inverted
                // find the tile's corresponding tileset and retrieve texture information
                int tileId = (int)gid & 0xFFFFFFF;
                auto [tilesetName, id] = getTilesetNameFromID(tileId);
                std::shared_ptr<Tileset> ts = _sets.find(tilesetName)->second;
                auto textureData = ts->getTextureData(id);
                std::shared_ptr<JsonValue> tileData = JsonValue::allocObject();
                std::shared_ptr<JsonValue> textureJson = JsonValue::allocObject();
                textureJson->appendChild("texture", JsonValue::alloc(textureData.source));
                textureJson->appendChild("minx", JsonValue::alloc((long)textureData.startX));
                textureJson->appendChild("miny", JsonValue::alloc((long)textureData.startY));
                textureJson->appendChild("maxx", JsonValue::alloc((long)(textureData.startX + textureData.lengthX)));
                textureJson->appendChild("maxy", JsonValue::alloc((long)(textureData.startY + textureData.lengthY)));
                tileData->appendChild("asset", textureJson);
                // adjust x location to the center of the tile, since tiles MAY cover multiple tiles, it is not necessarily half way point of a tile width
                x += textureData.lengthX/2.0f;
                tileData->appendChild("x", JsonValue::alloc(x/_tileDimension));
                tileData->appendChild("y", JsonValue::alloc(y/_tileDimension));
                float width = textureData.lengthX / _tileDimension;
                float height = textureData.lengthY / _tileDimension;
                tileData->appendChild("width", JsonValue::alloc(width));
                tileData->appendChild("height", JsonValue::alloc(height));
                tileData->appendChild("texture-id", JsonValue::alloc((long)id));
                tiledLayerData->appendChild(tileData);
            }
        }
    }
    // CULog("%s", tiledLayerData->toString().c_str());
    return tiledLayerData;
    
}

const std::shared_ptr<JsonValue> LevelParser::parseTiled(const std::shared_ptr<JsonValue>& json) {
    
    // IMPLEMENTATION NOTES:
    // JsonValue get(Key) is O(#children)
    // JsonValue get(index) is constant time
    
    std::shared_ptr<JsonValue> levelData = JsonValue::allocObject();
    
    CUAssertLog(json->getString("orientation") == "staggered", "Please use staggered isometric maps.");
    
    // retrieve map attributes
    _tileWidth = json->getInt("tilewidth");
    _tileHeight = json->getInt("tileheight");
    _mapWidth = (json->getInt("width") + 0.5f) * _tileWidth;
    _mapHeight = (json->getInt("height")/2.0f + 0.5f) * _tileHeight;
    _tileDimension = std::min(_tileWidth, _tileHeight);
    // set box2d map and camera attributes
    levelData->appendValue("width", _mapWidth/_tileDimension);
    levelData->appendValue("height", _mapHeight/_tileDimension);
    levelData->appendValue("view-width", 16.0f);
    levelData->appendValue("view-height", 9.0f);
    
    parseTilesetDependency(json->get("tilesets"));
    
    std::shared_ptr<JsonValue> tileLayersData = JsonValue::allocObject();
    std::vector<std::shared_ptr<JsonValue>> layers = json->get(LAYERS_KEY)->children();
    int layerCount = 0;
    for (auto it = layers.begin(); it != layers.end(); it++){
        std::shared_ptr<JsonValue> layerData = *it;
        std::string type = layerData->getString(TYPE);
        if (type == TILE_LAYER){
            layerCount++;
            if (layerData->getBool(VISIBLE)){
                std::string uniqueID = "layer-" + std::to_string(layerCount) + "-";
                tileLayersData->appendChild(uniqueID + layerData->getString("name"), parseTiledLayer(layerData));
            }
        }
        else if (type == OBJECT_LAYER){
            // TODO: next up
        }
    }
    levelData->appendChild("tiles", tileLayersData);
    
    return levelData;
}
