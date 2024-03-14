//
//  LevelParser.cpp
//  RS
//
//  Created by Vincent Fong on 3/5/24.
//

#include "LevelParser.hpp"
#include <string>
#include <cugl/io/CUJsonReader.h>
#include <cugl/io/CUJsonWriter.h>

using namespace cugl;

bool LevelParser::readLayermap(const std::shared_ptr<JsonValue>& layers) {
    for (int i = 0; i < layers->size(); i++) {
        CULog(layers->get(i)->getString("name").c_str());
        _layermap[layers->get(i)->getString("name")] = i;
    }
    return true;
}

const std::shared_ptr<JsonValue> LevelParser::translateJson(float w, float h) {
    float x = _width * 2 + w - h;
    float y = _height * 1.5 - (w + h) / 2;
    std::shared_ptr<JsonValue> ans = JsonValue::allocArray();
    ans->appendValue(x);
    ans->appendValue(y);
    return ans;
}

// deprecated
const std::shared_ptr<JsonValue> LevelParser::parseBoundaries(const std::shared_ptr<JsonValue>& layers) {
    std::shared_ptr<JsonValue> pNode = layers->get(7);
    if (pNode->get("name")->toString() != "\"collision\"") {
        CULogError("incorrect index for mobs in JSON, node is %s", pNode->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    
    std::shared_ptr<JsonValue> bounds = pNode->get("objects");
    
    for (int i = 0; i < (int) bounds->size(); i++) {
        std::shared_ptr<JsonValue> bound = bounds->get(i);
        std::shared_ptr<JsonValue> new_bound = JsonValue::allocObject();
        std::shared_ptr<JsonValue> vertices = JsonValue::allocArray();
        for (int s = 0; s < bound->get("polygon")->size(); s++) {
            std::shared_ptr<JsonValue> vertex = bound->get("polygon")->get(s);
            float px = vertex->get("x")->asFloat();
            float py = vertex->get("y")->asFloat();
            std::shared_ptr<JsonValue> pos = translateJson(px / (_tilewidth / 2), py / _tileheight);
            vertices->appendValue(pos->get(0)->asFloat());
            vertices->appendValue(pos->get(1)->asFloat());
        }
        new_bound->appendChild("boundary", vertices);
        new_bound->appendValue("vertices", (int) bound->get("polygon")->size() / 2.0);
//        float px = enemy->get("x")->asFloat();
//        float py = enemy->get("y")->asFloat();
//        std::shared_ptr<JsonValue> pos = translateJson(px / (_tilewidth / 2), py / _tileheight);
        
        new_bound->appendValue("density", 0.0);
        new_bound->appendValue("friction", 0.2);
        new_bound->appendValue("restitution", 0.1);
        new_bound->appendValue("texture", (std::string) "earth");
        new_bound->appendValue("debugcolor", (std::string) "green");
        new_bound->appendValue("debugopacity", 192.0);
        ans->appendChild(std::to_string(i), new_bound);
    }
    
    return ans;
}

const std::shared_ptr<JsonValue> LevelParser::parseEnemies(const std::shared_ptr<JsonValue>& layers) {
    // get information
     std::shared_ptr<JsonValue> pNode = layers->get(_layermap[ENEMIES_FIELD]);
//    std::shared_ptr<JsonValue> pNode = layers->get(9);
    if (pNode->get("name")->toString() != "\"mob_spawns\"") {
        CULogError("incorrect index for mobs in JSON, node is %s", pNode->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    std::shared_ptr<JsonValue> enemies = pNode->get("objects");
    
    for (int i = 0; i < (int) enemies->size(); i++) {
        std::shared_ptr<JsonValue> enemy = enemies->get(i);
        std::shared_ptr<JsonValue> new_mob = JsonValue::allocObject();
        float px = enemy->get("x")->asFloat();
        float py = enemy->get("y")->asFloat();
        std::shared_ptr<JsonValue> pos = translateJson(px / (_tilewidth / 2), py / _tileheight);
        std::shared_ptr<JsonValue> size = JsonValue::allocArray();
        size->appendValue(0.6);
        size->appendValue(0.6);
        new_mob->appendChild("size", size);
        new_mob->appendChild("pos", pos);
        new_mob->appendValue("density", 1.0);
        new_mob->appendValue("friction", 0.1);
        new_mob->appendValue("restitution", 0.0);
        new_mob->appendValue("rotation", false);
        new_mob->appendValue("health", 2.0);
        new_mob->appendValue("texture", (std::string) "enemy");
        new_mob->appendValue("debugcolor", (std::string) "green");
        ans->appendChild(enemy->getString("name"), new_mob);
    }
    return ans;
}

const std::shared_ptr<JsonValue> LevelParser::parsePlayer(const std::shared_ptr<JsonValue>& layers) {
    // get information
    std::shared_ptr<JsonValue> pNode = layers->get(_layermap[PLAYER_FIELD]);
//    std::shared_ptr<JsonValue> pNode = layers->get(8);
    if (pNode->get("name")->toString() != "\"player\"") {
        CULogError("incorrect index for player node in JSON, node is %s", pNode->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    
    std::shared_ptr<JsonValue> patt = pNode->get("objects")->get(0);
    float px = patt->get("x")->asFloat();
    float py = patt->get("y")->asFloat();
    std::shared_ptr<JsonValue> pos = translateJson(px / (_tilewidth / 2), py / _tileheight);
    
    // construct
    ans->appendChild("pos", pos);
    
    // TODO: offload to LevelModel function?
    std::shared_ptr<JsonValue> size = JsonValue::allocArray();
    size->appendValue(1.0);
    size->appendValue(1.0);
    ans->appendChild("size", size);
    ans->appendValue("density", 0.5);
    ans->appendValue("friction", 0.1);
    ans->appendValue("restitution", 0.0);
    ans->appendValue("rotation", false);
    ans->appendValue("texture", (std::string) "player-idle");
    ans->appendValue("parry", (std::string) "parry");
    ans->appendValue("attack", (std::string) "attack");
    ans->appendValue("debugcolor", (std::string) "blue");
    return ans;
}

const std::shared_ptr<JsonValue> LevelParser::parseFloor(const std::shared_ptr<JsonValue>& layers) {
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    std::shared_ptr<JsonValue> tiles = JsonValue::allocObject();

    // bottom-right of iso view
    std::shared_ptr<JsonValue> br = layers->get(_layermap[BR_FIELD]);
    if (br->get("name")->toString() != "\"bottom_right\"") {
        CULogError("incorrect index for br in JSON, node is %s", br->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> br_ans = JsonValue::allocObject();
    std::shared_ptr<JsonValue> br_tiles = JsonValue::allocArray();
    int c = 0;
    std::vector<int> br_dat = br->get("data")->asIntArray();
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (br_dat[_width * j + i] != 0) {
                std::shared_ptr<JsonValue> tile = translateJson(i, j);
                br_tiles->appendChild(tile);
                c++;
            }
        }
    }
    br_ans->appendChild("tiles", br_tiles);
    br_ans->appendValue("size", c * 1.0);
    tiles->appendChild("bottom-right", br_ans);
    
    // bottom-left of iso view
    std::shared_ptr<JsonValue> bl = layers->get(_layermap[BL_FIELD]);
    if (bl->get("name")->toString() != "\"bottom_left\"") {
        CULogError("incorrect index for bl in JSON, node is %s", bl->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> bl_ans = JsonValue::allocObject();
    std::shared_ptr<JsonValue> bl_tiles = JsonValue::allocArray();
    std::vector<int> bl_dat = bl->get("data")->asIntArray();
    c = 0;
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (bl_dat[_width * j + i] != 0) {
                std::shared_ptr<JsonValue> tile = translateJson(i, j);
                bl_tiles->appendChild(tile);
                c++;
            }
        }
    }
    bl_ans->appendChild("tiles", bl_tiles);
    bl_ans->appendValue("size", c * 1.0);
    tiles->appendChild("bottom-left", bl_ans);
    
    // top (floor) of iso view
    std::shared_ptr<JsonValue> fl = layers->get(_layermap[FLOOR_FIELD]);
    if (fl->get("name")->toString() != "\"floor\"") {
        CULogError("incorrect index for floor in JSON, node is %s", fl->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> fl_ans = JsonValue::allocObject();
    std::shared_ptr<JsonValue> fl_tiles = JsonValue::allocArray();
    std::vector<int> fl_dat = fl->get("data")->asIntArray();
    c = 0;
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (fl_dat[_width * j + i] != 0) {
                std::shared_ptr<JsonValue> tile = translateJson(i, j);
                fl_tiles->appendChild(tile);
                c++;
            }
        }
    }
    fl_ans->appendChild("tiles", fl_tiles);
    fl_ans->appendValue("size", c * 1.0);
    tiles->appendChild("floor", fl_ans);
    
    ans->appendChild("tiles", tiles);
    ans->appendValue("use-grid", false);
    ans->appendValue("layers", 3.0);
    // TODO: abstract texture file name away
    ans->appendValue("texture", (std::string) "floor-new");
    std::shared_ptr<JsonValue> size = JsonValue::allocArray();
    // LITERAL
    size->appendValue(2.0);
    size->appendValue(2.0);
    ans->appendChild("size", size);
    // CULog("%s", ans->toString().c_str());
    return ans;
}

const std::shared_ptr<JsonValue> LevelParser::parseTiled(const std::shared_ptr<JsonValue>& json) {
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    
    // single attributes
    _width = json->get("width")->asInt();
    _height = json->get("height")->asInt();
    _tilewidth = json->get("tilewidth")->asInt();
    _tileheight = json->get("tileheight")->asInt();
    ans->appendValue("width", 4.0 * _width);
    ans->appendValue("height", 2.0 * _height);
    ans->appendValue("view-width", 24.0f);
    ans->appendValue("view-height", 13.5f);
    
    std::shared_ptr<JsonValue> layers = json->get("layers");
    readLayermap(layers);
    std::shared_ptr<JsonValue> floor = LevelParser::parseFloor(layers);
    ans->appendChild("floor", floor);
    
    std::shared_ptr<JsonValue> player = LevelParser::parsePlayer(layers);
    ans->appendChild("player", player);
    
    std::shared_ptr<JsonValue> enemies = LevelParser::parseEnemies(layers);
    ans->appendChild("enemies", enemies);
    
//    std::shared_ptr<JsonValue> boundaries = LevelParser::parseBoundaries(layers);
//    ans->appendChild("enemies", boundaries);
    
//    CULog("%s", ans->toString().c_str());
    
    return ans;
}
