//
//  RangedLizard.cpp
//  RS
//
//  Created by Dylan McIntyre on 3/24/24.
//

#include "RangedLizard.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool RangedLizard::init(std::shared_ptr<JsonValue> data) {
    RangedEnemy::init(data);
    
    return true;
}


#pragma mark -
#pragma mark Physics

void RangedLizard::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    Vec2 direction = getFacingDir();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0){
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    
    setCharged(false);
    std::shared_ptr<Projectile> p = Projectile::lizardAlloc(getPosition().add(0, (_pixelHeight/2) / getDrawScale().y), getDamage(), ang, assets);
    p->setDrawScale(level->getDrawScale());
    level->addProjectile(p);
}


#pragma mark -
#pragma mark Animation

void RangedLizard::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _healthBG =  assets->get<Texture>("hp_back");
    _healthFG =  assets->get<Texture>("hp");
    auto idleTexture = assets->get<Texture>("lizard-ranged-idle");
    auto walkTexture = assets->get<Texture>("lizard-ranged-walk");
    auto attackTexture = assets->get<Texture>("lizard-ranged-attack");
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto deathEffect = assets->get<Texture>("enemy-death-effect");
    auto projectileTexture = assets->get<Texture>("lizard-projectile");
    
    auto idleSheet = SpriteSheet::alloc(idleTexture, 8, 8);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 9);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 20);
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto projectileSheet = SpriteSheet::alloc(projectileTexture, 3, 5);
    auto deathEffectSheet = SpriteSheet::alloc(deathEffect, 2, 4);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 7);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 8);
    _attackAnimation = Animation::alloc(attackSheet, GameConstants::ENEMY_RANGED_ATK_SPEED, false, 0, 19);
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _chargingAnimation = Animation::alloc(projectileSheet, GameConstants::ENEMY_RANGED_ATK_SPEED / 4, false, 0, 4);
    _deathEffect = Animation::alloc(deathEffectSheet, 1.0f, false);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        //_hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
    });
    
    _attackAnimation->addCallback(0.0f, [this](){
        setAiming(true);
    });
    
    _attackAnimation->addCallback(GameConstants::ENEMY_RANGED_ATK_SPEED * 0.4f, [this](){
        _chargingAnimation->start();
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

void RangedLizard::setFacingDir(cugl::Vec2 dir) {
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
        _idleAnimation->setFrameRange(8 * _directionIndex, 8 * _directionIndex + 7);
        _walkAnimation->setFrameRange(9 * _directionIndex, 9 * _directionIndex + 8);
        _attackAnimation->setFrameRange(20 * _directionIndex, 20 * _directionIndex + 19);
    }
}

