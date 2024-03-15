//
//  Floor.cpp
//  RS
//
//  Created by Zhiyuan Chen on 2/25/24.
//

#include "Floor.hpp"

#include <cugl/cugl.h>
#include <vector>
#include <cugl/assets/CUAsset.h>
#include <cugl/io/CUJsonReader.h>

using namespace cugl;

#pragma mark -
#pragma mark Tile

Tile::Tile(Vec2 pos, std::string textureKey){
    _pos = pos;
    _textureKey = textureKey;
}

Tile::Tile(Vec2 pos, std::string textureKey, std::string layer) {
    _pos = pos;
    _textureKey = textureKey;
    _layer = layer;
}

void Tile::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    _texture = assets->get<Texture>(_textureKey);
}

void Tile::draw(const std::shared_ptr<cugl::SpriteBatch> &batch, Vec2 size, Vec2 drawScale){
    /**
     texture    The new active texture
     origin    The image origin in pixel space
     scale    The amount to scale the texture
     angle    The amount to rotate the texture
     offset    The texture offset in world coordinates
     */
    
    int k;
    if (_layer == "bottom-right") {
        k = 3;
    } else if (_layer == "bottom-left") {
        k = 2;
    } else if (_layer == "floor") {
        k = 0;
    } else {
        k = 1;
    }
    std::shared_ptr<cugl::Texture> subtex = _texture->getSubTexture(0.25 * k, 0.25 + 0.25 * k, 0, 1);
    Vec2 scale = drawScale * size / (Vec2) subtex->getSize();
    Vec2 offset(-size.x / 2, -size.y / 4);
    batch->draw(subtex, Vec2::ZERO, scale, 0, (_pos + offset) * drawScale);

}


#pragma mark -
#pragma mark Floor

void Floor::dispose() {
    // nothing to do
    // the floor destroys itself, destroying the vector object
    // which reduces pointer counts to each tile and subsequently destroys all tiles
}

bool Floor::init(const cugl::Vec2 tileSize, std::vector<std::shared_ptr<Tile>> tiles){
    this->_size = tileSize;
    this->_tiles = tiles;
    return true;
}

#pragma mark -
#pragma mark View and Rendering

void Floor::draw(const std::shared_ptr<cugl::SpriteBatch> &batch){
    for (auto tile : _tiles){
        tile->draw(batch, _size, _drawScale);
    }
}

void Floor::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    for (auto tile : _tiles){
        tile->loadAssets(assets);
    }
}
