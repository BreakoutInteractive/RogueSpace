//
//  DummyEnemy.cpp
//  RS
//
//  Created by Zhiyuan Chen on 5/18/24.
//

#include "DummyEnemy.hpp"

bool DummyEnemy::init(std::shared_ptr<JsonValue> data) {
    Enemy::init(data);
    _jsonData = data;
    float width = data->getFloat("width");
    float height = data->getFloat("height");
    _size.set(width, height);
    _state = BehaviorState::DYING; // opt out of AI or combat
    _dropped = true; // should not drop health drops
    getCollider()->setBodyType(b2_staticBody);
    return true;
}

void DummyEnemy::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
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

void DummyEnemy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    if (_texture != nullptr){
        Vec2 origin(_texture->getWidth()/2, 0);
        batch->draw(_texture, origin, _size * _drawScale / _texture->getSize(), 0, _position * _drawScale);
    }
}
