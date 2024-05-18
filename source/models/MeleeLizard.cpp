//
//  MeleeLizard.cpp
//  RS
//
//  Created by Dylan McIntyre on 4/22/24.
//

#include "MeleeLizard.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "Player.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool MeleeLizard::init(std::shared_ptr<JsonValue> data) {
    MeleeEnemy::init(data);
    return true;
}


#pragma mark -
#pragma mark Physics

void MeleeLizard::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    Vec2 direction = getFacingDir();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0){
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    
    _attack->setPosition(_attack->getPosition() + Vec2(0, (_pixelHeight/2) / _drawScale.y)); //64 is half of the pixel height of the enemy
    _attack->setAngle(ang);
}


#pragma mark -
#pragma mark Animation

void MeleeLizard::loadAssets(const std::shared_ptr<AssetManager> &assets){
    MeleeEnemy::loadAssets(assets);
    auto idleTexture = assets->get<Texture>("lizard-idle");
    auto walkTexture = assets->get<Texture>("lizard-walk");
    auto attackTexture = assets->get<Texture>("lizard-attack");
    auto stunTexture = assets->get<Texture>("lizard-stun");
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto stunEffect = assets->get<Texture>("stun-effect");
    auto deathEffect = assets->get<Texture>("enemy-death-effect");
    
    auto idleSheet = SpriteSheet::alloc(idleTexture, 8, 8);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 9);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 18);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 15);
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto stunEffectSheet = SpriteSheet::alloc(stunEffect, 2, 4);
    auto deathEffectSheet = SpriteSheet::alloc(deathEffect, 2, 4);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 7);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 8);
    _attackAnimation = Animation::alloc(attackSheet, GameConstants::ENEMY_MELEE_ATK_SPEED, false, 0, 17);
    _stunAnimation = Animation::alloc(stunSheet, 1.25f, false, 0, 14);
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _stunEffect = Animation::alloc(stunEffectSheet, 0.333f, true);
    _deathEffect = Animation::alloc(deathEffectSheet, 1.0f, false);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        _hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
    });
    
    _attackAnimation->addCallback(GameConstants::ENEMY_MELEE_ATK_SPEED * 0.7f, [this](){
        if (isEnabled() && _health > 0) {
            _attack->setEnabled(true);
            _hitboxAnimation->start();
            _attack->setAwake(true);
            _attack->setAngle(getFacingDir().getAngle());
            _attack->setPosition(getPosition() + Vec2(0, (_pixelHeight/2) / getDrawScale().y)); //64 is half of the enemy pixel height
        }
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


void MeleeLizard::setFacingDir(cugl::Vec2 dir) {
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
        _attackAnimation->setFrameRange(18 * _directionIndex, 18 * _directionIndex + 17);
        _stunAnimation->setFrameRange(15 * _directionIndex, 15 * _directionIndex + 14);
    }
}
