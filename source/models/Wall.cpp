//
//  Wall.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/22/24.
//

#include "Wall.hpp"
#include "GameObject.hpp"
#include <cugl/cugl.h>
#include "CollisionConstants.hpp"

Wall::Wall(std::shared_ptr<JsonValue> data, const Poly2& poly, const Vec2 origin) : GameObject(){
    _jsonData = data;
    float x = data->getFloat("x");
    float y = data->getFloat("y");
    float width = data->getFloat("width");
    float height = data->getFloat("height");
    _size.set(width, height);
    GameObject::_position.set(x,y);
    
    // physics
    auto p = std::make_shared<physics2::PolygonObstacle>();
    p->PolygonObstacle::init(poly, origin);
    _collider = p;
    b2Filter filter;
    // this is a wall
    filter.categoryBits = CATEGORY_WALL;
    // a wall can collide with a player or an enemy
    filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
    p->setFilterData(filter);
    if (data->getBool("passable")){
        p->setSensor(true);
    }
}


void Wall::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    // using the json data, figure out the texture subregion
    auto textureData = _jsonData->get("asset");
    CUAssertLog(textureData != nullptr , "missing asset data in object data");
    _texture = assets->get<Texture>(textureData->getString("texture"));
    float minS = textureData->getFloat("minx") / _texture->getWidth();
    float maxS = textureData->getFloat("maxx") / _texture->getWidth();
    float minT = textureData->getFloat("miny") / _texture->getHeight();
    float maxT = textureData->getFloat("maxy") / _texture->getHeight();
    _texture = _texture->getSubTexture(minS, maxS, minT, maxT);
}

void Wall::draw(const std::shared_ptr<cugl::SpriteBatch> &batch){
    if (_texture != nullptr){
        Vec2 origin(_texture->getWidth()/2, 0);
        batch->draw(_texture, origin, _size * _drawScale / _texture->getSize(), 0, _position * _drawScale);
    }
}
