//
//  MageAlien.cpp
//  RS
//
//  Created by Dylan McIntyre on 3/24/24.
//

#include "MageAlien.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool MageAlien::init(std::shared_ptr<JsonValue> data) {
    RangedEnemy::init(data);
    
    return true;
}

#pragma mark -
#pragma mark Physics

void MageAlien::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    Vec2 direction = getFacingDir();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0){
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    
    setCharged(false);
    std::shared_ptr<Projectile> p = Projectile::mageAlloc(getPosition() + Vec2(0, (_pixelHeight/2) / getDrawScale().y), getDamage(), ang, assets);
    p->setDrawScale(level->getDrawScale());
    level->addProjectile(p);
}


#pragma mark -
#pragma mark Animation

void MageAlien::loadAssets(const std::shared_ptr<AssetManager> &assets){
    Enemy::loadAssets(assets); // health bar
    auto idleTexture = assets->get<Texture>("mage-idle");
    auto walkTexture = assets->get<Texture>("mage-walk");
    auto attackTexture = assets->get<Texture>("mage-attack");
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto deathEffect = assets->get<Texture>("enemy-death-effect");
    auto projectileTexture = assets->get<Texture>("mage-projectile");
    
    auto idleSheet = SpriteSheet::alloc(idleTexture, 8, 9);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 16);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 14);
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto deathEffectSheet = SpriteSheet::alloc(deathEffect, 2, 4);
    auto projectileSheet = SpriteSheet::alloc(projectileTexture, 4, 4);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 8);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 6, 13);
    _attackAnimation = Animation::alloc(attackSheet, GameConstants::ENEMY_RANGED_ATK_SPEED, false, 0, 13);
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _chargingAnimation = Animation::alloc(projectileSheet, GameConstants::ENEMY_RANGED_ATK_SPEED / 2, false, 0, 13);
    _deathEffect = Animation::alloc(deathEffectSheet, 1.0f, false);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        //_hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
    });
    
    _attackAnimation->addCallback(0.0f, [this](){
        if (isEnabled()) {
            _chargingAnimation->start();
        }
        setAiming(true);
    });
    
    _attackAnimation->addCallback(GameConstants::ENEMY_RANGED_ATK_SPEED * 0.4f, [this](){
        setAiming(false);
    });
    
    _chargingAnimation->onComplete([this](){
        _chargingAnimation->reset();
        setCharged(true);
    });
    
    setAnimation(_idleAnimation);

    _meleeHitEffect->onComplete([this]() {
        _meleeHitEffect->reset();
    });
    _bowHitEffect->onComplete([this]() {
        _bowHitEffect->reset();
    });

    _deathEffect->onComplete([this]() {
        _deathEffect->reset();
        setEnabled(false);
    });
}


void MageAlien::setFacingDir(cugl::Vec2 dir) {
    int prevDirection = _directionIndex;
    Vec2 d = dir.normalize();
    _directionIndex = -1;
    float similarity = -INFINITY;
    for (int i = 0; i < 8; i++){
        Vec2 cardinal = _directions[i];
        float dotprod = cardinal.dot(d);
        if (dotprod > similarity){
            similarity = dotprod;
            _directionIndex = i;
        }
    }
    assert(_directionIndex >= 0 && _directionIndex < 8);
    _facingDirection = dir;
    
    if (prevDirection != _directionIndex){
        _idleAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
        _walkAnimation->setFrameRange(16 * _directionIndex + 6, 16 * _directionIndex + 13);
        _attackAnimation->setFrameRange(14 * _directionIndex, 14 * _directionIndex + 13);
    }
}
