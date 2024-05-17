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
    std::vector<float> region = textureData->get("region")->asFloatArray();
    float minS = region[0] / _texture->getWidth();
    float minT = region[1] / _texture->getHeight();
    float maxS = region[2] / _texture->getWidth();
    float maxT = region[3] / _texture->getHeight();
    _texture = _texture->getSubTexture(minS, maxS, minT, maxT);
}

void Tile::draw(const std::shared_ptr<cugl::SpriteBatch> &batch, Vec2& drawScale){
    if (_texture != nullptr){
        Vec2 origin(_texture->getWidth()/2, 0);
        batch->draw(_texture, origin, _size * drawScale / _texture->getSize(), 0, _pos * drawScale);
    }
}


#pragma mark -
#pragma mark TileLayer

void TileLayer::addTile(const std::shared_ptr<Tile> tile){
    _tiles.push_back(tile);
}


void TileLayer::draw(const std::shared_ptr<cugl::SpriteBatch> &batch, Rect camRect){
    for (auto tile : _tiles){
        Vec2 tileSize = tile->getSize();
        Vec2 tileBottomLeft = (tile->getPosition().subtract(tileSize.x/2, 0));
        Rect tileRect = Rect(tileBottomLeft * _drawScale, tileSize * _drawScale);
        if (camRect.doesIntersect(tileRect)){
            tile->draw(batch, _drawScale);
        }
    }
}

void TileLayer::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    for (auto tile : _tiles){
        tile->loadAssets(assets);
    }
}
