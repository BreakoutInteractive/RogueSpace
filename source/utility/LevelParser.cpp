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
#include "../models/LevelConstants.hpp" // import data classes from LevelConstants

using namespace cugl;

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
std::shared_ptr<JsonValue> getPropertyValueByName(std::shared_ptr<JsonValue> propertiesJson, std::string name){
    if (propertiesJson == nullptr){
        return nullptr;
    }
    for (auto& property : propertiesJson->children()){
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
void parsePolygon(std::vector<Vec2>& vertices, Vec2& origin, Size tileDimensions, Vec2 objectPosition, Size objectSize){
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
void parseRect(Vec2& dimension, Vec2& origin, Size tileDimensions, Vec2 objectPosition, Size objectSize){
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
    std::shared_ptr<JsonValue> region = JsonValue::allocArray();
    region->appendChild(JsonValue::alloc((long)textureData.startX));
    region->appendChild(JsonValue::alloc((long)textureData.startY));
    region->appendChild(JsonValue::alloc((long)(textureData.startX + textureData.lengthX)));
    region->appendChild(JsonValue::alloc((long)(textureData.startY + textureData.lengthY)));
    textureJson->appendChild("region", region);
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

void LevelParser::cacheAllObjects(std::shared_ptr<JsonValue> layersJson){
    std::vector<std::shared_ptr<JsonValue>> layers = layersJson->children();
    for (auto it = layers.begin(); it != layers.end(); it++){
        std::shared_ptr<JsonValue> layerJson = *it;
        std::string type = layerJson->getString(TYPE);
        if (type == OBJECT_LAYER){
            auto objects = layerJson->get("objects")->children();
            // save all objects into global object collection (helps parsing paths)
            for (std::shared_ptr<JsonValue>& object: objects) {
                _objects[object->getInt("id")] = object;
            }
        }
        else if (type == GROUP_LAYER){
            auto childrenLayers = layerJson->get(LAYERS_KEY);
            cacheAllObjects(childrenLayers); // recursively go through the folder to find object layers
        }
    }
}

const std::shared_ptr<JsonValue> LevelParser::parseGroupLayer(const std::shared_ptr<JsonValue> layer){
    std::shared_ptr<JsonValue> groupLayerData = JsonValue::allocObject();
    std::shared_ptr<JsonValue> groupLayerContent = JsonValue::allocArray();
    auto layers = layer->get(LAYERS_KEY)->children();
    std::vector<int> weights;
    for (std::shared_ptr<JsonValue>& childLayer: layers) {
        std::shared_ptr<JsonValue> childLayerData; //output per child
        std::string type = childLayer->getString(TYPE);
        std::shared_ptr<JsonValue> weightProperty = getPropertyValueByName(childLayer->get("properties"), "weight");
        weights.push_back(weightProperty == nullptr ? 1 : std::min(1, weightProperty->asInt()));
        
        if (type == OBJECT_LAYER){
            childLayerData = parseObjectLayer(childLayer);
        }
        else if (type == TILE_LAYER){
            childLayerData = parseTiledLayer(childLayer);
        }
        else if (type == GROUP_LAYER){
            childLayerData = parseGroupLayer(childLayer);
        }
        
        if (childLayerData != nullptr){
            groupLayerContent->appendChild(childLayerData);
        }
    }
    std::string layerClass = layer->getString(CLASS, CLASS_COLLECTION);
    if (layerClass == CLASS_RANDOM){
        // convert weights to CDF if this group layer is a randomized set of layers
        int prev = 0;
        double sum = 0;
        for (int i = 0; i < weights.size(); i++){
            sum += weights[i];
            weights[i] += prev;
            prev = weights[i];
        }
        std::shared_ptr<JsonValue> weightsData = JsonValue::allocArray();
        for (int i = 0; i < weights.size(); i++){
            weightsData->appendChild(JsonValue::alloc(weights[i]/sum));
        }
        groupLayerData->appendChild("cdf", weightsData);
    }
    groupLayerData->appendChild("contents", groupLayerContent);
    groupLayerData->appendChild(CLASS, JsonValue::alloc(layerClass));
    return groupLayerData;
}

const std::shared_ptr<JsonValue> LevelParser::parseObjectLayer(const std::shared_ptr<JsonValue> layer){
    long id = layer->getInt("id");
    auto objects = layer->get("objects")->children();
    std::shared_ptr<JsonValue> objectLayerData = JsonValue::allocObject();
    std::shared_ptr<JsonValue> objectLayerContents = JsonValue::allocArray();
    for (std::shared_ptr<JsonValue>& object: objects) {
        std::string type = object->getString(TYPE);
        std::shared_ptr<JsonValue> data;
        if (type == CLASS_WALL || type == "EnergyWall"){
            data = parseWall(object);
            data->appendChild("type", JsonValue::alloc(std::string(type == CLASS_WALL ? "Wall" : "Energy")));
        }
        else if (type == CLASS_PLAYER){
            data = parsePlayer(object);
        }
        else if (type== CLASS_RELIC){
            data = parseRelic(object);
        }
        else if (type == CLASS_TUTORIAL_REGION){
            data = parseTutorialColliders(object);
        }
        else if (type == CLASS_LIZARD || type == CLASS_CASTER || type == CLASS_TANK || type == CLASS_RANGEDLIZARD || type == CLASS_SLIME || type == CLASS_BOSS){
            data = parseEnemy(object, type);
        }
        if (data != nullptr){
            data->appendChild("layer-id", JsonValue::alloc(id));
            objectLayerContents->appendChild(data);
        }
    }
    objectLayerData->appendChild("contents", objectLayerContents);
    objectLayerData->appendChild(CLASS, JsonValue::alloc(std::string(CLASS_COLLECTION)));
    return objectLayerData;
}

const std::shared_ptr<JsonValue> LevelParser::parseTiledLayer(const std::shared_ptr<JsonValue> layer){
    std::shared_ptr<JsonValue> tiledLayerObject = JsonValue::allocObject();
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
    tiledLayerObject->appendChild("tiles", tiledLayerData);
    tiledLayerObject->appendChild(CLASS, JsonValue::alloc(std::string(CLASS_TILELAYER)));
    return tiledLayerObject;
    
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
    
    // create the data container and set box2d map attributes
    std::shared_ptr<JsonValue> levelData = JsonValue::allocObject();
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
    
    // parsing the layers (needs 2 pass)
    parseTilesetDependency(json->get("tilesets"));
    std::shared_ptr<JsonValue> layersJson = json->get(LAYERS_KEY);
    cacheAllObjects(layersJson); // first pass
    
    // parse all group layers (this is done recursively)
    auto contents = parseGroupLayer(json); // entire map is a group
    auto mapData = contents->removeChild("contents");
    levelData->appendChild(MAP_FIELD, mapData);
    return levelData;
}

#pragma mark -
#pragma mark Parsing Objects

const std::shared_ptr<JsonValue> LevelParser::parseWall(const std::shared_ptr<JsonValue>& json){
    
    bool parseAsset = true;
    std::shared_ptr<JsonValue> animationProperty = getPropertyValueByName(json->get("properties"), "animation"); // only for energy walls
    if (animationProperty != nullptr){
        parseAsset = false;
    }
    
    auto data = parsePhysicsObject(json, parseAsset, true, false);
    data->appendChild(CLASS, JsonValue::alloc(std::string(CLASS_WALL)));
    std::shared_ptr<JsonValue> heightProperty = getPropertyValueByName(json->get("properties"), "tall");
    if (heightProperty != nullptr) {
        data->appendChild("tall", JsonValue::alloc(heightProperty->asBool()));
    }
    else {
        data->appendChild("tall", JsonValue::alloc(true));
    }
    if (animationProperty != nullptr){
        data->appendChild("animation", JsonValue::alloc(animationProperty->asLong()));
    }
    return data;
}

const std::shared_ptr<JsonValue> LevelParser::parseRelic(const std::shared_ptr<JsonValue>& json){
    auto data = parsePhysicsObject(json, true, true, false);
    data->appendChild(CLASS, JsonValue::alloc(std::string(CLASS_RELIC)));
    return data;
}

const std::shared_ptr<JsonValue> LevelParser::parsePlayer(const std::shared_ptr<JsonValue> &json){
    auto playerData = parsePhysicsObject(json, false, true, true);
    playerData->appendChild(CLASS, JsonValue::alloc(std::string(CLASS_PLAYER)));
    return playerData;
}

const std::shared_ptr<JsonValue> LevelParser::parseEnemy(const std::shared_ptr<JsonValue>& json, std::string enemyType){
    std::shared_ptr<JsonValue> enemyData = parsePhysicsObject(json, false, true, true);
    std::shared_ptr<JsonValue> enemyPathData = parsePath(json);
    enemyData->appendChild("type", JsonValue::alloc(std::string(enemyType)));
    enemyData->appendChild("path", enemyPathData);
    enemyData->appendChild("defaultstate", JsonValue::alloc(std::string(enemyPathData->size() > 2 ? "patrol" : "sentry")));
    std::shared_ptr<JsonValue> enemyHpProperty = getPropertyValueByName(json->get("properties"), ENEMY_HP_FIELD);
    CUAssertLog(enemyHpProperty != nullptr, "unable to find enemy hp on object id %d", json->getInt("id"));
    std::shared_ptr<JsonValue> enemyDmgProperty = getPropertyValueByName(json->get("properties"), ENEMY_DMG_FIELD);
    CUAssertLog(enemyDmgProperty != nullptr, "unable to find enemy damage on object id %d", json->getInt("id"));
    enemyData->appendChild(ENEMY_HP_FIELD, JsonValue::alloc(enemyHpProperty->asFloat()));
    enemyData->appendChild(ENEMY_DMG_FIELD, JsonValue::alloc(enemyDmgProperty->asFloat()));
    enemyData->appendChild(CLASS, JsonValue::alloc(std::string(CLASS_ENEMY)));
    return enemyData;
}

const std::shared_ptr<JsonValue> LevelParser::parseTutorialColliders(const std::shared_ptr<JsonValue> &json){

    // load collider properties
    Vec2 pos(json->getFloat("x"), _mapHeight - json->getFloat("y"));
    Vec2 origin(json->getFloat("x"), json->getFloat("y"));
    std::vector<Vec2> vertices;
    auto points = json->get("polygon")->children();
    for (auto& point : points){
        vertices.push_back(Vec2(point->getFloat("x"), point->getFloat("y")));
    }
    auto artificialSize = Size(origin.x * 2, origin.y);
    parsePolygon(vertices, origin, artificialSize, pos, artificialSize);
    // adjust collider size to be in terms of physics units
    scalePolygonCollider(vertices, Vec2::ONE/_tileDimension);
    
    // finalize data
    std::shared_ptr<JsonValue> data = JsonValue::allocObject();
    data->appendChild("shape", JsonValue::alloc(std::string("polygon")));
    data->appendChild("vertices", convertVerticesToJSON(vertices));
    data->appendChild("x", JsonValue::alloc(origin.x/_tileDimension));
    data->appendChild("y", JsonValue::alloc(origin.y/_tileDimension));
    data->appendChild(CLASS, JsonValue::alloc(std::string(CLASS_TUTORIAL_REGION)));
    
    auto gesture = getPropertyValueByName(json->get("properties"), "gesture");
    CUAssertLog(gesture != nullptr, "gesture needs to be specified");
    data->appendChild("gesture", JsonValue::alloc(gesture->asString()));
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

#pragma mark -
#pragma mark Parsing Structures

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
        std::shared_ptr<JsonValue> colliderJson = getObjectByType(tileObjects, CLASS_COLLIDER);
        CUAssertLog(colliderJson != nullptr, "tileset %s: the tile %d is missing collider object with class Collider", tilesetName.c_str(), id);
        auto colliderData = parseGenericCollider(colliderJson, tileSize, tilePos, objectSize);
        data->appendChild("collider", colliderData);
    }
    
    if (parseHitbox){
        // load the hitbox
        std::shared_ptr<JsonValue> hitboxJSON = getObjectByType(tileObjects, CLASS_HITBOX);
        CUAssertLog(hitboxJSON != nullptr, "tileset %s: the tile %d is missing hitbox object with class Hitbox", tilesetName.c_str(), id);
        auto hitboxData = parseGenericCollider(hitboxJSON, tileSize, tilePos, objectSize);
        data->appendChild("hitbox", hitboxData);
    }
    
    return data;
}

const std::shared_ptr<JsonValue> LevelParser::parseGenericCollider(const std::shared_ptr<JsonValue>& colliderJson, Size tileSize, Vec2 tilePos, Size objectSize){
    
    if (colliderJson->has("polygon")){
        return parsePolyCollider(colliderJson, tileSize, tilePos, objectSize);
    }
    if (colliderJson->has("ellipse")){
        float width = colliderJson->getFloat("width");
        float height = colliderJson->getFloat("height");
        CUAssertLog(width == height, "ellipse(%f, %f) cannot be supported, but circles can", width, height);
        return parseCircleCollider(colliderJson, tileSize, tilePos, objectSize);
    }
    return parseBoxCollider(colliderJson, tileSize, tilePos, objectSize);
}
    
const std::shared_ptr<JsonValue> LevelParser::parsePolyCollider(const std::shared_ptr<JsonValue>& colliderJson, Size tileSize, Vec2 tilePos, Size objectSize){
    Vec2 origin(colliderJson->getFloat("x"), colliderJson->getFloat("y"));
    std::vector<Vec2> vertices;
    auto points = colliderJson->get("polygon")->children();
    for (auto& point : points){
        vertices.push_back(Vec2(point->getFloat("x"), point->getFloat("y")));
    }
    parsePolygon(vertices, origin, tileSize, tilePos, objectSize);
    // adjust collider size to be in terms of physics units
    scalePolygonCollider(vertices, Vec2::ONE/_tileDimension);
    // set collider data (for physics)
    std::shared_ptr<JsonValue> collisionData = JsonValue::allocObject();
    collisionData->appendChild("shape", JsonValue::alloc(std::string("polygon")));
    collisionData->appendChild("vertices", convertVerticesToJSON(vertices));
    collisionData->appendChild("x", JsonValue::alloc(origin.x/_tileDimension));
    collisionData->appendChild("y", JsonValue::alloc(origin.y/_tileDimension));
    return collisionData;
}

const std::shared_ptr<JsonValue> LevelParser::parseCircleCollider(const std::shared_ptr<JsonValue>& colliderJson, Size tileSize, Vec2 tilePos, Size objectSize){

    Vec2 origin(colliderJson->getFloat("x"), colliderJson->getFloat("y"));
    Vec2 dimension(colliderJson->getFloat("width"), colliderJson->getFloat("height"));
    Vec2 scaleFactor = (Vec2) (objectSize / tileSize);
    // for some design-choice reason, the tiled circle position is at its leftmost point, so we have to adjust by half the horizontal dimension
    origin.add(dimension.x/2, 0); // this is still tiled coordinates (screen space in tile)
    // invert, set collider origin to the bottom center of tile
    origin.set(origin.x - tileSize.width/2, tileSize.height - origin.y); // cartesian with respect to bottom center of a tile
    origin.scale(scaleFactor); // scale the origin vector
    dimension.scale(scaleFactor);
    origin.add(tilePos); // adding the tile position to the origin gives world position of circle
    // set collider data (for physics)
    std::shared_ptr<JsonValue> collisionData = JsonValue::allocObject();
    collisionData->appendChild("shape", JsonValue::alloc(std::string("circle")));
    collisionData->appendChild("radius", JsonValue::alloc(dimension.x/_tileDimension));
    collisionData->appendChild("x", JsonValue::alloc(origin.x/_tileDimension));
    collisionData->appendChild("y", JsonValue::alloc(origin.y/_tileDimension));
    return collisionData;
}

const std::shared_ptr<JsonValue> LevelParser::parseBoxCollider(const std::shared_ptr<JsonValue>& colliderJson, Size tileSize, Vec2 tilePos, Size objectSize){
    Vec2 origin(colliderJson->getFloat("x"), colliderJson->getFloat("y"));
    Vec2 dimension(colliderJson->getFloat("width"), colliderJson->getFloat("height"));
    parseRect(dimension, origin, tileSize, tilePos, objectSize);
    std::shared_ptr<JsonValue> collisionData = JsonValue::allocObject();
    collisionData->appendChild("shape", JsonValue::alloc(std::string("box")));
    collisionData->appendChild("width", JsonValue::alloc(dimension.x/_tileDimension));
    collisionData->appendChild("height", JsonValue::alloc(dimension.y/_tileDimension));
    collisionData->appendChild("x", JsonValue::alloc(origin.x/_tileDimension));
    collisionData->appendChild("y", JsonValue::alloc(origin.y/_tileDimension));
    return collisionData;
}
