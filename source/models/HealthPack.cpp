//
//  HealthPack.cpp
//  RS
//
//  Created by Miguel Amor on 5/9/24.
//

#include "HealthPack.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "../components/Collider.hpp"
#include "LevelModel.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors

bool HealthPack::init(Vec2 pos, const std::shared_ptr<AssetManager>& assets) {
    _delMark = false;
    _enabled = true;
    _tint = Color4::WHITE;
	_position = pos;
    _drawScale.set(1.0f, 1.0f);
	auto collider = physics2::BoxObstacle::alloc(_position, Size(0.75f,0.75f));
	b2Filter filter;
    filter.categoryBits = CATEGORY_HEALTHPACK;
	filter.maskBits = CATEGORY_PLAYER | CATEGORY_PLAYER_HITBOX;
    collider->setFilterData(filter);
    collider->setSensor(true);
    collider->setAwake(true);
    collider->setEnabled(true);
    collider->setDebugColor(Color4::RED);
    collider->setName("health-pack-collider");
    _collider = collider;

    _texture = assets->get<Texture>("health-pack");
	return true;
}

void HealthPack::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    // batch draw(texture, color, origin, scale, angle, offset)

    Vec2 origin = Vec2(_texture->getWidth() / 2, _texture->getHeight() / 2);
    Affine2 transform = Affine2::createScale(0.75f);
    transform.translate(_position * _drawScale);
    batch->draw(_texture, origin, transform);
}

void HealthPack::dispose() {
    _currAnimation = nullptr;
    _collider = nullptr;
    setEnabled(false);
}