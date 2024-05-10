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

/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void RangedLizard::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
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
    std::shared_ptr<Projectile> p = Projectile::lizardAlloc(getPosition().add(0, 64 / getDrawScale().y), 1, ang, assets);
    p->setDrawScale(level->getDrawScale());
    level->addProjectile(p);
}


#pragma mark -
#pragma mark Animation

void RangedLizard::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _enemyTexture = assets->get<Texture>("lizard-ranged-idle");
    _healthBG =  assets->get<Texture>("hp_back");
    _healthFG =  assets->get<Texture>("hp");
    auto walkTexture = assets->get<Texture>("lizard-ranged-walk");
    auto attackTexture = assets->get<Texture>("lizard-ranged-attack");
    auto stunTexture = assets->get<Texture>("lizard-stun");
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto stunEffect = assets->get<Texture>("stun-effect");
    auto projectileTexture = assets->get<Texture>("lizard-projectile");
    
    auto idleSheet = SpriteSheet::alloc(_enemyTexture, 8, 8);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 9);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 20);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 15);
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto projectileSheet = SpriteSheet::alloc(projectileTexture, 3, 5);
    auto stunEffectSheet = SpriteSheet::alloc(stunEffect, 2, 4);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 7);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 8);
    _attackAnimation = Animation::alloc(attackSheet, 1.125f, false, 0, 19);
    _stunAnimation = Animation::alloc(stunSheet, 1.0f, false, 0, 14);
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _chargingAnimation = Animation::alloc(projectileSheet, 0.28125f, false, 0, 4);
    _stunEffect = Animation::alloc(stunEffectSheet, 0.333f, true);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        //_hitboxAnimation->reset();
        _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
    });
    
    _attackAnimation->addCallback(0.0f, [this](){
        setAiming(true);
    });
    
    _attackAnimation->addCallback(0.45f, [this](){
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
}

void RangedLizard::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render enemy with appropriate scales
    // batch draw(texture, color, origin, scale, angle, offset)
    auto spriteSheet = _currAnimation->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2();
    // transform.scale(0.5);
    transform.translate(getPosition() * _drawScale);
    
    spriteSheet->draw(batch, _tint, origin, transform);
    
    //enemy health bar
    float idleWidth = _idleAnimation->getSpriteSheet()->getFrameSize().width;
    Vec2 idleOrigin = Vec2(_idleAnimation->getSpriteSheet()->getFrameSize().width / 2, 0);
    
    Rect healthFGRect = Rect(0, spriteSheet->getFrameSize().height, idleWidth*(_health/_maxHealth), 5);
    Rect healthBGRect = Rect(0, spriteSheet->getFrameSize().height, idleWidth, 5);

    batch->draw(_healthBG, healthBGRect, idleOrigin, transform);
    batch->draw(_healthFG, healthFGRect, idleOrigin, transform);

    if (_meleeHitEffect->isActive()) {
        auto effSheet = _meleeHitEffect->getSpriteSheet();
        transform = Affine2::createScale(2);
        transform.translate(getPosition().add(0, 32 / _drawScale.y) * _drawScale); //64 is half of enemy pixel height
        origin = Vec2(effSheet->getFrameSize().width / 2, effSheet->getFrameSize().height / 2);
        effSheet->draw(batch, origin, transform);
    }
    if (_bowHitEffect->isActive()) {
        auto effSheet = _bowHitEffect->getSpriteSheet();
        transform = Affine2::createScale(2);
        transform.translate(getPosition().add(0, 32 / _drawScale.y) * _drawScale); //64 is half of enemy pixel height
        origin = Vec2(effSheet->getFrameSize().width / 2, effSheet->getFrameSize().height / 2);
        effSheet->draw(batch, origin, transform);
    }
    std::shared_ptr<SpriteSheet>sheet = _chargingAnimation->getSpriteSheet();
    Vec2 o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
    Vec2 dir = getFacingDir();
    float ang = acos(dir.dot(Vec2::UNIT_X));
    if (dir.y < 0) {
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(dir.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    Affine2 t = Affine2::createRotation(ang);
    t.scale(_drawScale/32);
    t.translate((_position + Vec2(0, 64 / getDrawScale().y)) * _drawScale);
    if (_chargingAnimation->isActive()) sheet->draw(batch, o, t);
}

void RangedLizard::updateAnimation(float dt){
    GameObject::updateAnimation(dt);
    // attack animation must play to completion, as long as enemy is alive.
    if (!_attackAnimation->isActive()) {
        if ((getCollider()->getLinearVelocity().isZero() && _stunCD.isZero()) && _currAnimation != _idleAnimation) {
            setIdling();
        }
        else if (!getCollider()->getLinearVelocity().isZero() && _currAnimation != _walkAnimation) {
            setMoving();
        }
    }
    _meleeHitEffect->update(dt);
    _bowHitEffect->update(dt);
    if (_meleeHitEffect->isActive() || _bowHitEffect->isActive()) {
        _tint = Color4::RED;
    }
    else if (getState() == EnemyState::STUNNED && _stunCD.isZero()) {
        _tint = Color4::WHITE;
        setIdling();
    }
    else if (getState() == EnemyState::STUNNED){
        // TODO: could possibly use stunned animation and remove this state altogether
        _tint = Color4::YELLOW;
    }
    else {
        _tint = Color4::WHITE;
    }
    
    _hitboxAnimation->update(dt);
    
    _chargingAnimation->update(dt);
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
        _stunAnimation->setFrameRange(15 * _directionIndex, 15 * _directionIndex + 14);
    }
}

