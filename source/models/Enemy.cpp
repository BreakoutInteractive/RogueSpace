//
//  Enemy.cpp
//  RS
//
//  Created by Zhiyuan Chen on 2/26/24.
//

#include "Enemy.hpp"

using namespace cugl;

#define HIT_TIME 10

#pragma mark -
#pragma mark Constructors


bool Enemy::init(const Vec2 pos, const Size size) {
    BoxObstacle::init(pos,size);
    BoxObstacle::setAngle(M_PI_4);
    std::string name("enemy");
    setName(name);
    _tint = Color4::WHITE;
    _hitCounter.setMaxCount(HIT_TIME);
    return true;
}


/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void Enemy::dispose() {
    _textureKey = "";
    _texture = nullptr;
}


#pragma mark -
#pragma mark Physics

void Enemy::applyForce() {
    if (!BoxObstacle::isEnabled()) {
        return;
    }

//    if (_dodgeDuration.isZero()){
//        // when not dodging, set max speed
//        auto maxGroundSpeed = 5.0f;
//        Vec2 vel = getLinearVelocity();
//        if (vel.length() >= maxGroundSpeed) {
//            vel.normalize();
//            setLinearVelocity(vel * maxGroundSpeed);
//        }
//    }
    auto pos = getPosition();
    _body->ApplyForce(b2Vec2(_force.x, _force.y), b2Vec2(pos.x, pos.y), true);
}


#pragma mark -
#pragma mark Animation


void Enemy::setDrawScale(Vec2 scale) {
    _drawScale = scale;
}

void Enemy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render enemy with appropriate scales
    // batch draw(texture, color, origin, scale, angle, offset)
    batch->draw(_texture,_tint, Vec2(_texture->getWidth()/2, 0), Vec2(0.5, 0.5), 0, getPosition() * _drawScale);
}

void Enemy::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _texture = assets->get<Texture>(_textureKey);
}

void Enemy::hit() {
    if (_hitCounter.isZero()) {
        _hitCounter.reset();
        setHealth(getHealth()-1);
        _tint = Color4::RED;
    }
}

void Enemy::updateCounters() {
    _hitCounter.decrement();
    if (_hitCounter.isZero()) _tint = Color4::WHITE;
}
