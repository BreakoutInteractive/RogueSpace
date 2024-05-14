//
//  ExplodingAlien.cpp
//  RS
//

#include "ExplodingAlien.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "GameConstants.hpp"
#include "Player.hpp"


using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool ExplodingAlien::init(std::shared_ptr<JsonValue> data) {
    Enemy::init(data);
    _attackRange = GameConstants::EXPLODE_PROX_RANGE;
    _attack = Hitbox::alloc(getCollider()->getPosition(), _attackRange);
    _windupCD.setMaxCount(GameConstants::EXPLODE_TIMER);
    _windupCD.reset();
    b2Filter filter;
    _attack->setSensor(true);
    _attack->setName("enemy-attack");
    _attack->setBodyType(b2_dynamicBody);
    // this is an attack
    filter.categoryBits = CATEGORY_ATTACK;
    // since it is an enemy's attack, it can collide with the player
    filter.maskBits = CATEGORY_PLAYER_HITBOX;
    _attack->setFilterData(filter);
    return true;
}

void ExplodingAlien::dispose() {
    // nothing
}

#pragma mark -
#pragma mark Physics


#pragma mark -
#pragma mark Animation

void ExplodingAlien::loadAssets(const std::shared_ptr<AssetManager> &assets){
    Enemy::loadAssets(assets);
    auto idleTexture = assets->get<Texture>("slime-idle");
    auto walkTexture = assets->get<Texture>("slime-walk");
    auto attackTexture = assets->get<Texture>("slime-attack");
    auto walkTextureWhite = assets->get<Texture>("slime-walk-white");
    auto idleTextureWhite = assets->get<Texture>("slime-idle-white");
    auto stunTexture = assets->get<Texture>("lizard-stun");
    
    auto meleeHitEffect = assets->get<Texture>("melee-hit-effect");
    auto bowHitEffect = assets->get<Texture>("bow-hit-effect");
    auto stunEffect = assets->get<Texture>("stun-effect");
    auto deathEffect = assets->get<Texture>("enemy-death-effect");
    auto explodeEffect = assets->get<Texture>("explosion-effect");
    
    auto idleSheet = SpriteSheet::alloc(idleTexture, 8, 4);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 5);
    auto idleSheetWhite = SpriteSheet::alloc(idleTextureWhite, 8, 4);
    auto walkSheetWhite = SpriteSheet::alloc(walkTextureWhite, 8, 5);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 1, 6);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 15);
    auto meleeHitSheet = SpriteSheet::alloc(meleeHitEffect, 2, 3);
    auto bowHitSheet = SpriteSheet::alloc(bowHitEffect, 2, 3);
    auto stunEffectSheet = SpriteSheet::alloc(stunEffect, 2, 4);
    auto deathEffectSheet = SpriteSheet::alloc(deathEffect, 2, 4);
    auto explodeEffectSheet = SpriteSheet::alloc(explodeEffect, 2, 4);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 3);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 4);
    _idleAnimationWhite = Animation::alloc(idleSheetWhite, 1.0f, true, 0, 3);
    _walkAnimationWhite = Animation::alloc(walkSheetWhite, 1.0f, true, 0, 4);
    _attackAnimation = Animation::alloc(attackSheet, 1.0f, false, 0, 5);
    _meleeHitEffect = Animation::alloc(meleeHitSheet, 0.25f, false);
    _bowHitEffect = Animation::alloc(bowHitSheet, 0.25f, false);
    _stunEffect = Animation::alloc(stunEffectSheet, 0.333f, true);
    _deathEffect = Animation::alloc(explodeEffectSheet, 0.333f, false); // this is explosion
    
    _currAnimation = _idleAnimation; // set running
    
    // add callbacks
    _attackAnimation->addCallback(0.5f, [this](){
        if (isEnabled()) {
            _health = 0; // self-destruct
            setDying();
        }
    });
    
    setAnimation(_idleAnimation);

    _meleeHitEffect->onComplete([this]() {
        _meleeHitEffect->reset();
    });
    _bowHitEffect->onComplete([this]() {
        _bowHitEffect->reset();
    });
 
    // the effect is always started on 0 health (which is desired)
    _deathEffect->addCallback(0.0f, [this](){
        _attack->setPosition(_attack->getPosition().add(0, 64 / _drawScale.y)); //64 is half of the pixel height of the enemy
        _attack->setAwake(true);
        _attack->setEnabled(true);
    });
    _deathEffect->onComplete([this]() {
        _deathEffect->reset();
        _attack->setEnabled(false);
        setEnabled(false);
    });
}

void ExplodingAlien::setFacingDir(cugl::Vec2 dir) {
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
        _idleAnimation->setFrameRange(4 * _directionIndex, 4 * _directionIndex + 3);
        _walkAnimation->setFrameRange(5 * _directionIndex, 5 * _directionIndex + 4);
        _idleAnimationWhite->setFrameRange(4 * _directionIndex, 4 * _directionIndex + 3);
        _walkAnimationWhite->setFrameRange(5 * _directionIndex, 5 * _directionIndex + 4);
    }
}

void ExplodingAlien::updateWindup(bool down) {
    if (down) {
        _windupCD.decrement();
    } else {
        _windupCD.increment();
    }
}

void ExplodingAlien::updateAnimation(float dt){
    if (_enabled){
        if (_currAnimation != nullptr){
            if (_currAnimation->isStarted()) {
                _currAnimation->update(dt);
                if (_currAnimation == _idleAnimation) {
                    _idleAnimationWhite->update(dt);
                } else if (_currAnimation == _walkAnimation) {
                    _walkAnimationWhite->update(dt);
                }
            }
            else {
                _currAnimation->start();
                if (_currAnimation == _idleAnimation) {
                    _idleAnimationWhite->start();
                } else if (_currAnimation == _walkAnimation) {
                    _walkAnimationWhite->start();
                }
            }
        }
    }
    _meleeHitEffect->update(dt);
    _bowHitEffect->update(dt);
    _deathEffect->update(dt);
    if (_meleeHitEffect->isActive() || _bowHitEffect->isActive()){
        _tint = Color4::RED;
    }
    else {
        _tint = Color4::WHITE;
    }
}

void ExplodingAlien::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    // there is no "attack". we only play attack animations but attack on death.
    return;
}

void ExplodingAlien::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    std::shared_ptr<Animation> glowup = _currAnimation;
    if (_windupCD.getCount() % 20 > 12) {
        if (glowup == _idleAnimation) {
            glowup = _idleAnimationWhite;
        } else if (glowup == _walkAnimation) {
            glowup = _walkAnimationWhite;
        }
    }
    auto spriteSheet = glowup->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2();
    // transform.scale(0.5);
    transform.translate(_position * _drawScale);
    spriteSheet->draw(batch, _tint, origin, transform);
    
    //enemy health bar
    float idleWidth = _idleAnimation->getSpriteSheet()->getFrameSize().width;
    Vec2 idleOrigin = Vec2(_idleAnimation->getSpriteSheet()->getFrameSize().width / 2, 0);
    
    Rect healthBGRect = Rect(0, spriteSheet->getFrameSize().height, idleWidth, 5);
    Rect healthFGRect = Rect(0, spriteSheet->getFrameSize().height, idleWidth*(_health/_maxHealth), 5);

    batch->draw(_healthBG, healthBGRect, idleOrigin, transform);
    batch->draw(_healthFG, healthFGRect, idleOrigin, transform);
    
    if (_meleeHitEffect->isActive()) {
        drawEffect(batch, _meleeHitEffect, 2);
    }
    if (_bowHitEffect->isActive()) {
        drawEffect(batch, _bowHitEffect, 2);
    }
    if (_deathEffect->isActive()) {
        drawEffect(batch, _deathEffect, 0.8 * GameConstants::EXPLODE_RADIUS);
    }
}
