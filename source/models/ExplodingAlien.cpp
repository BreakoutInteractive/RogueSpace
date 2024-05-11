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
    _windupCD.setMaxCount(GameConstants::EXPLODE_TIMER);
    _windupCD.setCount(GameConstants::EXPLODE_TIMER);
    return true;
}

/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void ExplodingAlien::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
}

#pragma mark -
#pragma mark Physics


#pragma mark -
#pragma mark Animation

void ExplodingAlien::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _enemyTexture = assets->get<Texture>("explode-idle");
    auto walkTexture = assets->get<Texture>("explode-walk");
    auto attackTexture = assets->get<Texture>("explode-attack");
    auto walkTextureWhite = assets->get<Texture>("explode-walk-white");
    auto idleTextureWhite = assets->get<Texture>("explode-idle-white");
    auto stunTexture = assets->get<Texture>("lizard-stun");
    auto hitEffect = assets->get<Texture>("enemy-hit-effect");
    auto stunEffect = assets->get<Texture>("stun-effect");
    _healthBG =  assets->get<Texture>("hp_back");
    _healthFG =  assets->get<Texture>("hp");
    
    auto idleSheet = SpriteSheet::alloc(_enemyTexture, 8, 4);
    auto walkSheet = SpriteSheet::alloc(walkTexture, 8, 5);
    auto idleSheetWhite = SpriteSheet::alloc(idleTextureWhite, 8, 4);
    auto walkSheetWhite = SpriteSheet::alloc(walkTextureWhite, 8, 5);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 1, 6);
    auto stunSheet = SpriteSheet::alloc(stunTexture, 8, 15); // TODO: can't remove this because of base setAttacking()?
    auto hitSheet = SpriteSheet::alloc(hitEffect, 2, 3);
    auto stunEffectSheet = SpriteSheet::alloc(stunEffect, 2, 4);
    
    _idleAnimation = Animation::alloc(idleSheet, 1.0f, true, 0, 3);
    _walkAnimation = Animation::alloc(walkSheet, 1.0f, true, 0, 4);
    _idleAnimationWhite = Animation::alloc(idleSheetWhite, 1.0f, true, 0, 3);
    _walkAnimationWhite = Animation::alloc(walkSheetWhite, 1.0f, true, 0, 4);
    _attackAnimation = Animation::alloc(attackSheet, 1.0f, false, 0, 5);
    _stunAnimation = Animation::alloc(stunSheet, 1.0f, false, 0, 14);
    _hitEffect = Animation::alloc(hitSheet, 0.25f, false);
    _stunEffect = Animation::alloc(stunEffectSheet, 0.333f, true);
    
    _currAnimation = _idleAnimation; // set running
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _atkCD.reset(); // cooldown begins AFTER the attack is done
        _attack->setEnabled(false);
        setCharged(true); // TODO: take out overloaded functionality?
    });
    
    _attackAnimation->addCallback(0.5f, [this](){
        if (isEnabled()) {
            _attackRange = GameConstants::EXPLODE_RADIUS;
            _attack->setEnabled(true);
            _attack->setAwake(true);
            _attack->setAngle(getFacingDir().getAngle());
            _attack->setPosition(getPosition().add(0, 64 / getDrawScale().y)); //64 is half of the enemy pixel height
        }
    });
    
    setAnimation(_idleAnimation);

    _hitEffect->onComplete([this]() {
        _hitEffect->reset();
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
        _attackAnimation->setFrameRange(0, 5);
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
    // attack animation must play to completion, as long as enemy is alive.
    _hitEffect->update(dt);
    if (_hitEffect->isActive()){
        _tint = Color4::RED;
    }
    else {
        _tint = Color4::WHITE;
    }
}

void ExplodingAlien::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    Vec2 direction = level->getPlayer()->getPosition() * level->getPlayer()->getDrawScale() - getPosition() * getDrawScale();
    direction.normalize();
    float ang = acos(direction.dot(Vec2::UNIT_X));
    if (direction.y < 0){
        // handle downwards case, rotate counterclockwise by PI rads and add extra angle
        ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
    }
    
    _attack->setPosition(_attack->getPosition().add(0, 64 / _drawScale.y)); //64 is half of the pixel height of the enemy
    _attack->setAngle(ang);
}

void ExplodingAlien::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render enemy with appropriate scales
    // batch draw(texture, color, origin, scale, angle, offset)
    
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
    transform.translate(_position * _drawScale); // previously using getPosition()
    
    spriteSheet->draw(batch, _tint, origin, transform);
    
    //enemy health bar
    float idleWidth = _idleAnimation->getSpriteSheet()->getFrameSize().width;
    Vec2 idleOrigin = Vec2(_idleAnimation->getSpriteSheet()->getFrameSize().width / 2, 0);
    
    Rect healthBGRect = Rect(0, spriteSheet->getFrameSize().height, idleWidth, 5);
    Rect healthFGRect = Rect(0, spriteSheet->getFrameSize().height, idleWidth*(_health/_maxHealth), 5);

    batch->draw(_healthBG, healthBGRect, idleOrigin, transform);
    batch->draw(_healthFG, healthFGRect, idleOrigin, transform);
    
    if (_hitEffect->isActive()) {
        auto effSheet = _hitEffect->getSpriteSheet();
        transform = Affine2::createScale(2);
        transform.translate(getPosition().add(0, 64 / _drawScale.y) * _drawScale); //64 is half of enemy pixel height
        origin = Vec2(effSheet->getFrameSize().width / 2, effSheet->getFrameSize().height / 2);
        effSheet->draw(batch, origin, transform);
    }
}
