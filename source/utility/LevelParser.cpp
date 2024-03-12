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

const std::shared_ptr<JsonValue> LevelParser::translateJson(float w, float h, bool arr) {
    float x = _width / 2 + _width - w + h;
    float y = _height / 2 + _height - (w + h) / 2;
    std::shared_ptr<JsonValue> ans;
    if (arr) {
        ans = JsonValue::allocArray();
        ans->appendValue(x);
        ans->appendValue(y);
        
    } else {
        ans = JsonValue::allocObject();
        ans->appendValue("x", x);
        ans->appendValue("y", y);
    }
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
    std::shared_ptr<JsonValue> pos = translateJson(px / (_tilewidth / 2), py / (_tileheight), true);
    
    // construct
    ans->appendChild("pos", pos);
    // TODO: fill the rest out
    
    return ans;
}

const std::shared_ptr<JsonValue> LevelParser::parseFloor(const std::shared_ptr<JsonValue>& layers) {
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    // TODO: learn about frames and retrieving the correct texture for each part of the floor?
    // TODO: standardize get indices for get, this check is good but should order Tiled layers

    // bottom-right of iso view
    std::shared_ptr<JsonValue> br = layers->get(0);
    if (br->get("name")->toString() != "\"bottom_right\"") {
        CULogError("incorrect index for player node in JSON, node is %s", br->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> br_ans = JsonValue::allocArray();
    std::vector<int> br_dat = br->get("data")->asIntArray();
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (br_dat[_width * i + j] != 0) {
                std::shared_ptr<JsonValue> tile = translateJson(i, j, false);
                br_ans->appendChild(tile);
            }
        }
    }
    ans->appendChild("bottom_right", br_ans);
    
    // bottom-left of iso view
    std::shared_ptr<JsonValue> bl = layers->get(1);
    if (bl->get("name")->toString() != "\"bottom_left\"") {
        CULogError("incorrect index for player node in JSON, node is %s", bl->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> bl_ans = JsonValue::allocArray();
    std::vector<int> bl_dat = bl->get("data")->asIntArray();
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (bl_dat[_width * i + j] != 0) {
                std::shared_ptr<JsonValue> tile = translateJson(i, j, false);
                bl_ans->appendChild(tile);
            }
        }
    }
    ans->appendChild("bottom_left", bl_ans);
    
    // top (floor) of iso view
    std::shared_ptr<JsonValue> fl = layers->get(1);
    if (fl->get("name")->toString() != "\"bottom_left\"") {
        CULogError("incorrect index for player node in JSON, node is %s", fl->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> fl_ans = JsonValue::allocArray();
    std::vector<int> fl_dat = fl->get("data")->asIntArray();
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if (fl_dat[_width * i + j] != 0) {
                std::shared_ptr<JsonValue> tile = translateJson(i, j, false);
                fl_ans->appendChild(tile);
            }
        }
    }
    ans->appendChild("floor", fl_ans);
    
    CULog("%s", ans->toString().c_str());
    return ans;
}

const std::shared_ptr<JsonValue> LevelParser::parseTiled(const std::shared_ptr<JsonValue>& json) {
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    
    // single attributes
    // TODO: create tile coordinate conversion between tiled origin to game origin
    // we also have to translate all walls and objects above 0,0 and need to make sure this problem does not carry from tiled
    _width = json->get("width")->asInt();
    _height = json->get("height")->asInt();
    _tilewidth = json->get("tilewidth")->asInt();
    _tileheight = json->get("tileheight")->asInt();
    ans->appendValue("width", 2.0 * _width);
    ans->appendValue("height", 2.0 * _height);
    ans->appendValue("view-width", 1.0 * _width);
    ans->appendValue("view-height", 1.0 * _height);
    
    std::shared_ptr<JsonValue> layers = json->get("layers");
    std::shared_ptr<JsonValue> floor = LevelParser::parseFloor(layers);
    
    std::shared_ptr<JsonValue> player = LevelParser::parsePlayer(layers);
    ans->appendChild("player", player);
    
//    if (bl->isArray()) {
//        CULog("hello, your bool is right");
//    }
    
    CULog("%s", ans->toString().c_str());
    
    return ans;
}
