//
//  TileLayer.cpp
//  RS
//
//  Created by Zhiyuan Chen on 2/25/24.
//

#include "TileLayer.hpp"

#include <cugl/cugl.h>
#include <vector>
#include <cugl/assets/CUAsset.h>
#include <cugl/io/CUJsonReader.h>

using namespace cugl;

#pragma mark -
#pragma mark Tile

Tile::Tile(std::shared_ptr<JsonValue> data){
    _jsonData = data;
    float x = data->getFloat("x");
    float y = data->getFloat("y");
    float width = data->getFloat("width");
    float height = data->getFloat("height");
    _size.set(width, height);
    _pos.set(x,y);
}

void Tile::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    // using the json data, figure out the texture subregion
    auto textureData = _jsonData->get("asset");
    CUAssertLog(textureData != nullptr , "missing asset data in tile data");
    _texture = assets->get<Texture>(textureData->getString("texture"));
    float minS = textureData->getFloat("minx") / _texture->getWidth();
    float maxS = textureData->getFloat("maxx") / _texture->getWidth();
    float minT = textureData->getFloat("miny") / _texture->getHeight();
    float maxT = textureData->getFloat("maxy") / _texture->getHeight();
    _texture = _texture->getSubTexture(minS, maxS, minT, maxT);
}

void Tile::draw(const std::shared_ptr<cugl::SpriteBatch> &batch, Vec2& drawScale){
    if (_texture != nullptr){
        Vec2 origin(_texture->getWidth()/2, 0);
        //CULog("size: %s", (_size * drawScale).toString().c_str());
        batch->draw(_texture, origin, _size * drawScale / _texture->getSize(), 0, _pos * drawScale);
    }
}


#pragma mark -
#pragma mark TileLayer

void TileLayer::addTile(const std::shared_ptr<Tile> tile){
    _tiles.push_back(tile);
}


void TileLayer::draw(const std::shared_ptr<cugl::SpriteBatch> &batch){
    for (auto tile : _tiles){
        tile->draw(batch, _drawScale);
    }
}

void TileLayer::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    for (auto tile : _tiles){
        tile->loadAssets(assets);
    }
}
