//
//  Relic.cpp
//  RS
//
//  Created by Dasha Griffiths on 4/25/24.
//

#include "Relic.hpp"
#include "GameObject.hpp"
#include <cugl/cugl.h>
#include "CollisionConstants.hpp"

Relic::Relic(std::shared_ptr<JsonValue> data, const Poly2& poly, const Vec2 origin) : GameObject(){
    _jsonData = data;
    float x = data->getFloat("x");
    float y = data->getFloat("y");
    float width = data->getFloat("width");
    float height = data->getFloat("height");
    _active = false;
    _size.set(width, height);
    GameObject::_position.set(x,y);
    
    // physics
    auto p = std::make_shared<physics2::PolygonObstacle>();
    p->PolygonObstacle::init(poly, origin);
    _collider = p;
    b2Filter filter;
    // this is a Relic
    filter.categoryBits = CATEGORY_RELIC;
    // a Relic can collide with a player or an enemy
    filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_PROJECTILE_SHADOW;
    p->setFilterData(filter);
}


void Relic::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
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
    
    auto activeSheet = SpriteSheet::alloc(assets->get<Texture>("artifact-glow"), 2, 4);
    auto chosenSheet = SpriteSheet::alloc(assets->get<Texture>("artifact-chosen"), 2, 5);
    _activeAnimation = Animation::alloc(activeSheet, 1.0f, true);
    _chosenAnimation = Animation::alloc(chosenSheet, 1.0f, false);
}

void Relic::setActive(bool active) {
    _active = active;
    if (active){
        setAnimation(_activeAnimation);
    }
    else{
        setAnimation(_chosenAnimation);
    }
    _currAnimation->start();
}
void Relic::draw(const std::shared_ptr<cugl::SpriteBatch> &batch){
    if (_texture != nullptr){
        Vec2 origin(_texture->getWidth()/2, 0);
        batch->draw(_texture, origin, _size * _drawScale / _texture->getSize(), 0, _position * _drawScale);
        if (_currAnimation != nullptr && !_currAnimation->isCompleted()){
            auto spriteSheet = _currAnimation->getSpriteSheet();
            Affine2 aniTransform = Affine2::createTranslation(_position * _drawScale);
            spriteSheet->draw(batch, origin, aniTransform);
        }
    }
}
