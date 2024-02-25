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
    Vec2 scale = drawScale * size / (Vec2)_texture->getSize();
    Vec2 offset(-size.x / 2, -size.y/4);
    batch->draw(_texture, Vec2::ZERO, scale, 0, (_pos + offset) * drawScale);
}


#pragma mark -
#pragma mark Floor

void Floor::dispose(){
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
