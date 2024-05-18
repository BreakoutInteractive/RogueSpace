//
//  MeleeDummy.cpp
//  RS
//
//  Created by Dylan McIntyre on 5/17/24.
//

#include "MeleeDummy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Hitbox.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool MeleeDummy::init(std::shared_ptr<JsonValue> data) {
    Enemy::init(data);
    _dropped = true; // dummies don't drop healthpacks
    return true;
}

#pragma mark - Rendering

void MeleeDummy::loadAssets(const std::shared_ptr<cugl::AssetManager> &assets){
    Enemy::loadAssets(assets);
    auto idleTexture = assets->get<Texture>("lizard-idle"); // change this to dummy texture
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto deathEffect = assets->get<Texture>("enemy-death-effect");
    
    auto idleSheet = SpriteSheet::alloc(idleTexture, 8, 8); // change this to dummy sheet
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto deathEffectSheet = SpriteSheet::alloc(deathEffect, 2, 4);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 0); // for now just a single lizard frame
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _deathEffect = Animation::alloc(deathEffectSheet, 1.0f, false);
}

void MeleeDummy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    Enemy::draw(batch);
}

void MeleeDummy::updateAnimation(float dt){
    Enemy::updateAnimation(dt);
}

void MeleeDummy::hit(cugl::Vec2 atkDir, bool ranged, float damage, float knockback_scl) {
    if (!_meleeHitEffect->isActive() && !_bowHitEffect->isActive()) {
        _hitCounter.reset();
        // only takes damage from melee attacks
        if (!ranged) {
            setHealth(std::fmax(0, getHealth() - damage));
        }
        _meleeHitEffect->reset();
        _bowHitEffect->reset();
        if (ranged) _bowHitEffect->start();
        else _meleeHitEffect->start();
        _collider->setLinearVelocity(atkDir * knockback_scl);
    }
}
