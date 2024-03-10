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


const std::shared_ptr<JsonValue> LevelParser::parsePlayer(const std::shared_ptr<JsonValue>& layers) {
    // get information
    std::shared_ptr<JsonValue> pNode = layers->get(8);
    if (pNode->get("name")->toString() != "\"player\"") {
        CULogError("incorrect index for player node in JSON, node is %s", pNode->get("name")->toString().c_str());
    }
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    
    // will this always be index 0?
    std::shared_ptr<JsonValue> patt = pNode->get("objects")->get(0);
    float px = patt->get("x")->asFloat();
    float py = patt->get("y")->asFloat();
    
    // construct
    std::shared_ptr<JsonValue> pos = JsonValue::allocArray();
    pos->appendValue(px / 64);
    pos->appendValue(py / 64);
    ans->appendChild("pos", pos);
    
    return ans;
}

const std::shared_ptr<JsonValue> LevelParser::parseFloor(const std::shared_ptr<JsonValue>& layers) {
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    // bottom-right of iso view
    // TODO: learn about frames and retrieving the correct texture for each part of the floor
    // TODO: standardize constants for get, this check is good but prob talk to designers and/or inspect tiled
    std::shared_ptr<JsonValue> br = layers->get(0);
    if (br->get("name")->toString() != "\"bottom_right\"") {
        CULogError("incorrect index for player node in JSON, node is %s", br->get("name")->toString().c_str());
    }
    
    // The blockers of the floor rendering problem right now are:
    // 1)   Have to correctly translate the floor tiles
    // 1.1) Have to fix any OOB tiles from Tiled (perhaps talk to designers to avoid this)
    // 2)   Have to change how floor is rendered right now to accomodate all 3 layers.
    // This function should handle problem 1, to continue I'm reliant on tile conversion function (see below)
    
    // CULog("%s", br->get("name")->toString().c_str());
    return layers;
}

const std::shared_ptr<JsonValue> LevelParser::parseTiled(const std::shared_ptr<JsonValue>& json) {
    std::shared_ptr<JsonValue> ans = JsonValue::allocObject();
    
    // single attributes
    // TODO: create tile coordinate conversion between tiled origin to game origin
    // we also have to translate all walls and objects above 0,0 and need to make sure this problem does not carry from tiled
    float h = json->get("height")->asFloat();
    float w = json->get("width")->asFloat();
    ans->appendValue("width", w + 32);
    ans->appendValue("height", h + 18);
    ans->appendValue("view-width", w + 22);
    ans->appendValue("view-height", h + 8);
    
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
