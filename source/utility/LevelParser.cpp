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

// BIG ASSUMPTION: the play area is always a square -- simply make designers use square even if it's bigger than necessary?
const std::shared_ptr<JsonValue> LevelParser::translateJson(float w, float h) {
    float x = _width * 2 - w + h;
    float y = _height * 1.5 - (w + h) / 2;
    std::shared_ptr<JsonValue> ans = JsonValue::allocArray();
    ans->appendValue(x);
    ans->appendValue(y);
    return ans;
}

const std::shared_ptr<JsonValue> LevelParser::parsePlayer(const std::shared_ptr<JsonValue>& layers) {
    // get information
    std::shared_ptr<JsonValue> pNode = layers->get(8);
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
    // TODO: learn about frames and retrieving the correct texture for each part of the floor?
    // TODO: standardize get indices for get, this check is good but should order Tiled layers
    // TODO: append size[]???

    // bottom-right of iso view
    std::shared_ptr<JsonValue> br = layers->get(0);
    if (br->get("name")->toString() != "\"bottom_right\"") {
        CULogError("incorrect index for player node in JSON, node is %s", br->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> br_ans = JsonValue::allocObject();
    std::shared_ptr<JsonValue> br_tiles = JsonValue::allocArray();
    int c = 0;
    std::vector<int> br_dat = br->get("data")->asIntArray();
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (br_dat[_width * i + j] != 0) {
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
    std::shared_ptr<JsonValue> bl = layers->get(1);
    if (bl->get("name")->toString() != "\"bottom_left\"") {
        CULogError("incorrect index for player node in JSON, node is %s", bl->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> bl_ans = JsonValue::allocObject();
    std::shared_ptr<JsonValue> bl_tiles = JsonValue::allocArray();
    std::vector<int> bl_dat = bl->get("data")->asIntArray();
    c = 0;
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (bl_dat[_width * i + j] != 0) {
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
    std::shared_ptr<JsonValue> fl = layers->get(2);
    if (fl->get("name")->toString() != "\"floor\"") {
        CULogError("incorrect index for player node in JSON, node is %s", fl->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> fl_ans = JsonValue::allocObject();
    std::shared_ptr<JsonValue> fl_tiles = JsonValue::allocArray();
    std::vector<int> fl_dat = fl->get("data")->asIntArray();
    c = 0;
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (fl_dat[_width * i + j] != 0) {
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
    ans->appendValue("size", 3.0);
    // TODO: abstract texture file name away
    ans->appendValue("texture", (std::string) "floor-new");
    
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
    ans->appendValue("view-width", 2.0 * _width);
    ans->appendValue("view-height", 1.0 * _height);
    
    std::shared_ptr<JsonValue> layers = json->get("layers");
    std::shared_ptr<JsonValue> floor = LevelParser::parseFloor(layers);
    ans->appendChild("floor", floor);
    
    std::shared_ptr<JsonValue> player = LevelParser::parsePlayer(layers);
    ans->appendChild("player", player);
    
    CULog("%s", ans->toString().c_str());
    
    return ans;
}
