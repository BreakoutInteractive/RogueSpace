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
    bool tall = data->getBool("tall");
    _size.set(width, height);
    GameObject::_position.set(x,y);
    
    // physics
    auto p = std::make_shared<physics2::PolygonObstacle>();
    p->PolygonObstacle::init(poly, origin);
    _collider = p;
    b2Filter filter;
    // this is a wall
    if (tall) {
        filter.categoryBits = CATEGORY_TALL_WALL;
    }
    else {
        filter.categoryBits = CATEGORY_SHORT_WALL;
    }
    // a wall can collide with a player or an enemy
    filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_PROJECTILE_SHADOW;
    p->setFilterData(filter);
}


void Wall::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    // using the json data, figure out the texture subregion
    auto textureData = _jsonData->get("asset");
    CUAssertLog(textureData != nullptr , "missing asset data in object data");
    _texture = assets->get<Texture>(textureData->getString("texture"));
    std::vector<float> region = textureData->get("region")->asFloatArray();
    float minS = region[0] / _texture->getWidth();
    float minT = region[1] / _texture->getHeight();
    float maxS = region[2] / _texture->getWidth();
    float maxT = region[3] / _texture->getHeight();
    _texture = _texture->getSubTexture(minS, maxS, minT, maxT);
}

void Wall::draw(const std::shared_ptr<cugl::SpriteBatch> &batch){
    if (_texture != nullptr){
        Vec2 origin(_texture->getWidth()/2, 0);
        batch->draw(_texture, origin, _size * _drawScale / _texture->getSize(), 0, _position * _drawScale);
    }
}

#pragma mark -

EnergyWall::EnergyWall(std::shared_ptr<JsonValue> data, const Poly2& poly, const Vec2 origin) : Wall(data, poly, origin){
    animationIndex = data->getInt("animation");
}

void EnergyWall::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    auto energyWallSheet = SpriteSheet::alloc(assets->get<Texture>("energy-wall"), 8, 6);
    auto floorArrowSheet = SpriteSheet::alloc(assets->get<Texture>("floor-arrows"), 8, 4);
    _currAnimation = Animation::alloc(energyWallSheet, 1.0f, true, animationIndex * 6, (animationIndex+2) * 6 - 1);
    _arrowAnimation = Animation::alloc(floorArrowSheet, 1.5f, true, animationIndex * 4, (animationIndex + 2) * 4 - 1);
}

void EnergyWall::draw(const std::shared_ptr<cugl::SpriteBatch> &batch){
    if (_currAnimation != nullptr){
        auto spriteSheet = _currAnimation->getSpriteSheet();
        Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
        Affine2 transform = Affine2::createTranslation(_position * _drawScale);
        spriteSheet->draw(batch, _tint, origin, transform);
    }
}

void EnergyWall::deactivate(){
    /*if (_enabled){
        _enabled = false;
        getCollider()->setSensor(true);
    }*/
    if (!getCollider()->isSensor()) {
        _currAnimation->reset();
        _currAnimation = _arrowAnimation;
        _currAnimation->start();
        getCollider()->setSensor(true);
    }
}
