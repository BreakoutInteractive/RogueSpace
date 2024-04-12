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
#include <string>
#include <sstream>

using namespace cugl;

#define TILE_LAYER      "tilelayer"
#define OBJECT_LAYER    "objectgroup"
#define LAYERS_KEY      "layers"
#define TYPE            "type"
#define VISIBLE         "visible"
#define FIRSTGID        "firstgid"
#define CLASS_WALL      "Wall"
#define CLASS_PLAYER    "Player"
#define CLASS_COLLIDER  "Collider"
#define CLASS_HITBOX    "Hitbox"
#define CLASS_LIZARD    "Lizard"
#define CLASS_CASTER    "Caster"

#pragma mark -
#pragma mark Loading Dependencies (Assets)

void LevelParser::loadTilesets(const std::shared_ptr<AssetManager>& assets){
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

#pragma mark -
#pragma mark Property Retrieval

/**
 * given the array of properties (name, type, value) pairs on an object, find the value of the property with the given name
 */
std::shared_ptr<JsonValue> getPropertyValueByName(std::shared_ptr<JsonValue> objectProperties, std::string name){
    
    for (auto& property : objectProperties->children()){
        std::string propertyName = property->getString("name");
        if (propertyName == name){
            return property->get("value");
        }
    }
    return nullptr;
}


#pragma mark -
#pragma mark Geometry and Shapes Retrieval

/**
 * given the json list of objects on a tile, find the FIRST json shape with the given type
 */
std::shared_ptr<JsonValue> getObjectByType(std::shared_ptr<JsonValue>& tileObjects, std::string type){
    std::shared_ptr<JsonValue> obj = nullptr;
    std::vector<std::shared_ptr<JsonValue>> objectGroup = tileObjects->children();
    for (auto& object : objectGroup){
        std::string objType = object->getString(TYPE);
        if (objType == type){
            obj = object;
            break;
        }
    }
    return obj;
}


#pragma mark -
#pragma mark Polygon

/**
 * Modifies the vertices and origin of a polygon to be in the correct pixel positions in cartesian space.
 *
 * Tiled vertices specify a polygon in screen space coordinates which requires negating vertical direction.
 *
 * (1) specify origin irelative to the BOTTOM CENTER cartesian tile-space. This is (0,0) but shifted to the right by width/2
 * (2) negate y component of each vertex (which is in relative coordinates from the origin),
 * (3) Add the origin and the object position to each vertex so the vertices are  in the same cartesian space as the object position (game cartesian space)
 * (4) find the center of the polygon and replace origin with this new position
 */
void parsePolygonCollider(std::vector<Vec2>& vertices, Vec2& origin, Size tileDimensions, Vec2 objectPosition, Size objectSize){
    Vec2 scaleFactor = (Vec2) (objectSize / tileDimensions);
    // step 1
    origin.set(origin.x - tileDimensions.width/2, tileDimensions.height - origin.y);
    // step 2
    float maxx = -INFINITY;
    float minx = INFINITY;
    float maxy = -INFINITY;
    float miny = INFINITY;
    for (Vec2& vertex: vertices){
        // step 2 and 3
        vertex.y *= -1;
        vertex.scale(scaleFactor);
        vertex.add(origin).add(objectPosition);
        if (vertex.x > maxx){
            maxx = vertex.x;
        }
        if (vertex.x < minx){
            minx = vertex.x;
        }
        if (vertex.y > maxy){
            maxy = vertex.y;
        }
        if (vertex.y < miny){
            miny = vertex.y;
        }
    }
    // step 4
    origin.set((minx + maxx)/2, (miny + maxy)/2);
}

/**
 * given vertices of the collider (in cartesian coordinates relative to its origin), scale the collider by the ratio of
 * the object size to tile size times unit tile size
 */
void scalePolygonCollider(std::vector<Vec2>& vertices, Vec2 scaleFactor){
    for (Vec2& vertex: vertices){
        vertex.scale(scaleFactor);
    }
}

#pragma mark -
#pragma mark Box

/**
 * scales a box collider based on the ratio of the predefined object size (tile dimensions) and the actual object size.
 *
 * the origin is set to the center world position of the collider (vertically and horizontally)
 */
void parseBoxCollider(Vec2& dimension, Vec2& origin, Size tileDimensions, Vec2 objectPosition, Size objectSize){
    Vec2 scaleFactor = (Vec2) (objectSize / tileDimensions);
    origin.add(dimension/2);
    // invert, set collider origin to the bottom center of tile
    origin.set(origin.x - tileDimensions.width/2, tileDimensions.height - origin.y);
    origin.scale(scaleFactor);
    origin.add(objectPosition);
    dimension.scale(scaleFactor);
}


#pragma mark -
#pragma mark Data Converters

std::shared_ptr<JsonValue> convertVerticesToJSON(std::vector<Vec2>& vertices){
    std::shared_ptr<JsonValue> array = JsonValue::allocArray();
    for (Vec2& vertex: vertices){
        array->appendChild(JsonValue::alloc(vertex.x));
        array->appendChild(JsonValue::alloc(vertex.y));
    }
    return array;
}

std::shared_ptr<JsonValue> convertTextureDataToJSON(Tileset::TextureRegionData& textureData){
    std::shared_ptr<JsonValue> textureJson = JsonValue::allocObject();
    textureJson->appendChild("texture", JsonValue::alloc(textureData.source));
    textureJson->appendChild("minx", JsonValue::alloc((long)textureData.startX));
    textureJson->appendChild("miny", JsonValue::alloc((long)textureData.startY));
    textureJson->appendChild("maxx", JsonValue::alloc((long)(textureData.startX + textureData.lengthX)));
    textureJson->appendChild("maxy", JsonValue::alloc((long)(textureData.startY + textureData.lengthY)));
    return textureJson;
}


#pragma mark -
#pragma mark Parsing Dependency

void LevelParser::parseTilesetDependency(std::shared_ptr<JsonValue> tilesets){
    // Maybe we can do a little more to optimize dependency search in `getTilesetNameFromID`
    _mapTilesets.clear();
    std::vector<std::shared_ptr<JsonValue>> dependencies = tilesets->children();
    _mapTilesets = dependencies;
}

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

#pragma mark -
#pragma mark Parsing Layers

void LevelParser::parseObjectLayer(const std::shared_ptr<JsonValue> layer){
    long id = layer->getInt("id");
    auto objects = layer->get("objects")->children();
    for (std::shared_ptr<JsonValue>& object: objects) {
        std::string type = object->getString(TYPE);
        std::shared_ptr<JsonValue> data;
        if (type == CLASS_WALL){
            data = parseWall(object);
            _wallData->appendChild(data);
        }
        else if (type == CLASS_PLAYER){
            CUAssertLog(_playerData == nullptr, "uh oh!, multiple players found");
            data = parsePlayer(object);
        }
        else if (type == CLASS_COLLIDER){
            data = parseCustomCollision(object);
            _boundaryData->appendChild(data);
        }
        else if (type == CLASS_LIZARD || type == CLASS_CASTER){
            data = parseEnemy(object, type);
            _enemyData->appendChild(data);
        }
        
        if (data != nullptr){
            data->appendChild("layer-id", JsonValue::alloc(id));
        }
    }
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
                std::shared_ptr<JsonValue> textureJson = convertTextureDataToJSON(textureData);
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
    CUAssertLog(json->getString("orientation") == "staggered", "Please use staggered isometric maps.");
    CUAssertLog(json->getString("staggerindex") == "odd", "Please use odd-staggered maps");
    
    // retrieve map attributes
    _tileWidth = json->getInt("tilewidth");
    _tileHeight = json->getInt("tileheight");
    int gridWidth = json->getInt("width");
    int gridHeight = json->getInt("height");
    _mapWidth = ( gridWidth + 0.5f) * _tileWidth;
    _mapHeight = (gridHeight/2.0f + 0.5f) * _tileHeight;
    _tileDimension = std::min(_tileWidth, _tileHeight);
    
    // create the data containers
    std::shared_ptr<JsonValue> levelData = JsonValue::allocObject();
    /** the data associated with each layer of tiles (pairs of layer name followed by */
    std::shared_ptr<JsonValue> tileLayersData = JsonValue::allocObject();
    _wallData =  JsonValue::allocArray();
    _playerData = nullptr;
    _enemyData = JsonValue::allocArray();
    _boundaryData = JsonValue::allocArray();
    
    // set box2d map attributes
    levelData->appendValue("width", _mapWidth/_tileDimension);
    levelData->appendValue("height", _mapHeight/_tileDimension);
    
    // set layout properties, eg tile-grid attributes
    std::shared_ptr<JsonValue> gridData = JsonValue::allocObject();
    long mapGridWidth = gridWidth + 1;
    long mapGridHeight = gridHeight % 2 == 1 ? gridHeight + 1 : gridHeight;
    gridData->appendChild("width", JsonValue::alloc(mapGridWidth));
    gridData->appendChild("height", JsonValue::alloc(mapGridHeight));
    auto gridOrigin = JsonValue::allocArray();
    gridOrigin->appendChild(JsonValue::alloc(0.0));
    gridOrigin->appendChild(JsonValue::alloc(gridHeight % 2 == 1 ? -0.5 : 0.0));
    gridData->appendChild("origin", gridOrigin);
    levelData->appendChild("grid", gridData);
    
    // parsing the layers
    parseTilesetDependency(json->get("tilesets"));
    std::vector<std::shared_ptr<JsonValue>> objectLayers;
    std::vector<std::shared_ptr<JsonValue>> layers = json->get(LAYERS_KEY)->children();
    for (auto it = layers.begin(); it != layers.end(); it++){
        std::shared_ptr<JsonValue> layerJson = *it;
        std::string type = layerJson->getString(TYPE);
        if (type == TILE_LAYER){
            std::string uniqueID = "layer-" + std::to_string(layerJson->getInt("id")) + "-";
            if (layerJson->getBool(VISIBLE)){
                tileLayersData->appendChild(uniqueID + layerJson->getString("name"), parseTiledLayer(layerJson));
            }
        }
        else if (type == OBJECT_LAYER){
            objectLayers.push_back(layerJson);
            auto objects = layerJson->get("objects")->children();
            // save all objects into global object collection (helps parsing paths)
            for (std::shared_ptr<JsonValue>& object: objects) {
                _objects[object->getInt("id")] = object;
            }
        }
    }
    // parse all object layers now
    for (std::shared_ptr<JsonValue>& layerJson : objectLayers){
        parseObjectLayer(layerJson);
    }
    // data finalization step
    levelData->appendChild("tiles", tileLayersData);
    levelData->appendChild("walls", _wallData);
    CUAssertLog(_playerData != nullptr, "player not found");
    levelData->appendChild("player", _playerData);
    levelData->appendChild("enemies", _enemyData);
    levelData->appendChild("boundaries", _boundaryData);
    return levelData;
}

#pragma mark -
#pragma mark Parsing Objects

const std::shared_ptr<JsonValue> LevelParser::parseWall(const std::shared_ptr<JsonValue>& json){
    auto data = parsePhysicsObject(json, true, true, false);
    std::shared_ptr<JsonValue> passableProperty = getPropertyValueByName(json->get("properties"), "passable");
    if (passableProperty != nullptr){
        data->appendChild("passable", JsonValue::alloc(passableProperty->asBool()));
    }
    else {
        data->appendChild("passable", JsonValue::alloc(false));
    }
    return data;
}

const std::shared_ptr<JsonValue> LevelParser::parsePlayer(const std::shared_ptr<JsonValue> &json){
    _playerData = parsePhysicsObject(json, false, true, true);
    return _playerData;
}

const std::shared_ptr<JsonValue> LevelParser::parseEnemy(const std::shared_ptr<JsonValue>& json, std::string enemyType){
    std::shared_ptr<JsonValue> enemyData = parsePhysicsObject(json, false, true, true);
    std::shared_ptr<JsonValue> enemyPathData = parsePath(json);
    if (enemyType == CLASS_LIZARD){
        std::shared_ptr<JsonValue> enemyTypeProperty = getPropertyValueByName(json->get("properties"), "enemy_type");
        CUAssertLog(enemyTypeProperty != nullptr, "unable to find lizard type on object id %d", json->getInt("id"));
        std::string lizardType = enemyTypeProperty->asString();
        if (lizardType == "MELEE"){
            enemyData->appendChild("type", JsonValue::alloc(std::string("melee-lizard")));
        }
        else {
            enemyData->appendChild("type", JsonValue::alloc(std::string("ranged-lizard")));
        }
    }
    else if (enemyType == CLASS_CASTER){
        enemyData->appendChild("type", JsonValue::alloc(std::string("caster")));
    }
    enemyData->appendChild("path", enemyPathData);
    enemyData->appendChild("defaultstate", JsonValue::alloc(std::string(enemyPathData->size() > 2 ? "patrol" : "sentry")));
    std::shared_ptr<JsonValue> enemyHpProperty = getPropertyValueByName(json->get("properties"), "hp");
    CUAssertLog(enemyHpProperty != nullptr, "unable to find lizard hp on object id %d", json->getInt("id"));
    enemyData->appendChild("health", JsonValue::alloc(enemyHpProperty->asLong()));
    return enemyData;
}

const std::shared_ptr<JsonValue> LevelParser::parseCustomCollision(const std::shared_ptr<JsonValue> &json){

    // load collider properties
    Vec2 pos(json->getFloat("x"), _mapHeight - json->getFloat("y"));
    Vec2 origin(json->getFloat("x"), json->getFloat("y"));
    std::vector<Vec2> vertices;
    auto points = json->get("polygon")->children();
    for (auto& point : points){
        vertices.push_back(Vec2(point->getFloat("x"), point->getFloat("y")));
    }
    auto artificialSize = Size(origin.x * 2, origin.y);
    parsePolygonCollider(vertices, origin, artificialSize, pos, artificialSize);
    // adjust collider size to be in terms of physics units
    scalePolygonCollider(vertices, Vec2::ONE/_tileDimension);
    
    // finalize data
    std::shared_ptr<JsonValue> data = JsonValue::allocObject();
    // TODO: support other shapes if necessary
    data->appendChild("shape", JsonValue::alloc(std::string("polygon")));
    data->appendChild("vertices", convertVerticesToJSON(vertices));
    data->appendChild("x", JsonValue::alloc(origin.x/_tileDimension));
    data->appendChild("y", JsonValue::alloc(origin.y/_tileDimension));
    return data;
}

const std::shared_ptr<JsonValue> LevelParser::parsePhysicsObject(const std::shared_ptr<JsonValue> &json, bool parseAsset, bool parseCollider, bool parseHitbox){
    
    std::shared_ptr<JsonValue> data = JsonValue::allocObject();
    int tileId = json->getLong("gid") & 0xFFFFFFF;
    CUAssertLog(tileId != 0, "object %lu error: object does not belong to a tile", json->getLong("gid"));
    auto [tilesetName, id] = getTilesetNameFromID(tileId);
    std::shared_ptr<Tileset> ts = _sets.find(tilesetName)->second;
    Tileset::TextureRegionData textureData = ts->getTextureData(id);
    std::shared_ptr<JsonValue> tileData = ts->getTileData(id);
    CUAssertLog(tileData != nullptr, "object gid %d corresponds to tile %d which has no meta data", tileId, id);
    
    // add asset data
    if (parseAsset){
        data->appendChild("asset", convertTextureDataToJSON(textureData));
    }
    
    // load object properties
    Vec2 tilePos(json->getFloat("x"), _mapHeight - json->getFloat("y"));
    Size objectSize(json->getFloat("width"), json->getFloat("height"));
    Size tileSize(textureData.lengthX, textureData.lengthY);
    
    // set object data (for rendering)
    data->appendChild("x", JsonValue::alloc(tilePos.x/_tileDimension));
    data->appendChild("y", JsonValue::alloc(tilePos.y/_tileDimension));
    data->appendChild("width", JsonValue::alloc(objectSize.width / _tileDimension));
    data->appendChild("height", JsonValue::alloc(objectSize.height / _tileDimension));
    
    // get components
    std::shared_ptr<JsonValue> tileObjects = tileData->get(OBJECT_LAYER)->get("objects");
    
    if (parseCollider){
        // load collider properties
        std::shared_ptr<JsonValue> colliderJson = getObjectByType(tileObjects, CLASS_COLLIDER);
        CUAssertLog(colliderJson != nullptr, "tileset %s: the tile %d is missing collider object with class Collider", tilesetName.c_str(), id);
        
        Vec2 origin(colliderJson->getFloat("x"), colliderJson->getFloat("y"));
        std::vector<Vec2> vertices;
        auto points = colliderJson->get("polygon")->children();
        for (auto& point : points){
            vertices.push_back(Vec2(point->getFloat("x"), point->getFloat("y")));
        }
        parsePolygonCollider(vertices, origin, tileSize, tilePos, objectSize);
        
        // adjust collider size to be in terms of physics units
        scalePolygonCollider(vertices, Vec2::ONE/_tileDimension);
        
        // set collider data (for physics)
        std::shared_ptr<JsonValue> colliderData = JsonValue::allocObject();
        colliderData->appendChild("shape", JsonValue::alloc(std::string("polygon")));    // TODO: support other shapes if necessary
        colliderData->appendChild("vertices", convertVerticesToJSON(vertices));
        colliderData->appendChild("x", JsonValue::alloc(origin.x/_tileDimension));
        colliderData->appendChild("y", JsonValue::alloc(origin.y/_tileDimension));
        data->appendChild("collider", colliderData);
    }
    
    if (parseHitbox){
        // load the hitbox
        std::shared_ptr<JsonValue> hitboxJSON = getObjectByType(tileObjects, CLASS_HITBOX);
        CUAssertLog(hitboxJSON != nullptr, "tileset %s: the tile %d is missing hitbox object with class Hitbox", tilesetName.c_str(), id);
        Vec2 origin(hitboxJSON->getFloat("x"), hitboxJSON->getFloat("y"));
        Vec2 dimension(hitboxJSON->getFloat("width"), hitboxJSON->getFloat("height"));
        parseBoxCollider(dimension, origin, tileSize, tilePos, objectSize);
        std::shared_ptr<JsonValue> hitboxData = JsonValue::allocObject();
        hitboxData->appendChild("shape", JsonValue::alloc(std::string("box")));    // TODO: support other shapes if necessary
        hitboxData->appendChild("width", JsonValue::alloc(dimension.x/_tileDimension));
        hitboxData->appendChild("height", JsonValue::alloc(dimension.y/_tileDimension));
        hitboxData->appendChild("x", JsonValue::alloc(origin.x/_tileDimension));
        hitboxData->appendChild("y", JsonValue::alloc(origin.y/_tileDimension));
        data->appendChild("hitbox", hitboxData);
    }
    
    return data;
}

const std::shared_ptr<JsonValue> LevelParser::parsePath(const std::shared_ptr<JsonValue> & startNode){
    std::unordered_set<int> seenNodes;
    int currNode = startNode->getInt("id");
    std::shared_ptr<JsonValue> pathData = JsonValue::allocArray();
    while (currNode != 0 && seenNodes.find(currNode) == seenNodes.end()){
        std::shared_ptr<JsonValue> nodeData = _objects.find(currNode)->second;
        Vec2 nodePos(nodeData->getFloat("x"), _mapHeight - nodeData->getFloat("y"));
        nodePos /= _tileDimension;
        pathData->appendChild(JsonValue::alloc(nodePos.x));
        pathData->appendChild(JsonValue::alloc(nodePos.y));
        seenNodes.insert(currNode);
        std::shared_ptr<JsonValue> nextNodeValue = getPropertyValueByName(nodeData->get("properties"), "path");
        if (nextNodeValue == nullptr){
            currNode = 0;
        }
        else {
            currNode = nextNodeValue->asInt();
        }
    }
    return pathData;
}
