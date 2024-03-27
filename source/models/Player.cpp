//
//  Player.cpp
//
//  The player collision-box is represented by a square box rotated on a 45 degree angle.
//  RS
//
//  Created by Zhiyuan Chen on 2/23/24.
//

#include "Player.hpp"
#include "CollisionConstants.hpp"
#include "GameObject.hpp"
#include "GameConstants.hpp"
#include "../components/Animation.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool Player::init(const Vec2 pos, const Size size) {
    // set up collider
    auto box = std::make_shared<physics2::BoxObstacle>();
    box->init(pos, size);
    std::string name("player-collider");
    box->setName(name);
    // this is a player and can collide with an enemy "shadow", a wall, or an attack
    b2Filter filter;
    filter.categoryBits = CATEGORY_PLAYER;
    filter.maskBits = CATEGORY_ENEMY_SHADOW | CATEGORY_WALL | CATEGORY_ATTACK;
    box->setFilterData(filter);
    _collider = box;    // attach Component
    
    // set the player collider-shadow
    auto boxShadow = std::make_shared<physics2::BoxObstacle>();
    boxShadow->init(pos, size);
    std::string name2("player-obstacle");
    boxShadow->setName(name2);
    boxShadow->setBodyType(b2_kinematicBody);
    filter.categoryBits = CATEGORY_PLAYER_SHADOW;
    filter.maskBits = CATEGORY_ENEMY;
    boxShadow->setFilterData(filter);
    _colliderShadow = boxShadow; // attach Component
    
    // set the counter properties
    _hitCounter.setMaxCount(GameConstants::PLAYER_IFRAME);
    _atkCD.setMaxCount(GameConstants::PLAYER_ATTACK_COOLDOWN);
    _parryCD.setMaxCount(GameConstants::PLAYER_PARRY_COOLDOWN);
    _dodgeCD.setMaxCount(GameConstants::PLAYER_DODGE_COOLDOWN);
    _dodgeDuration.setMaxCount(GameConstants::PLAYER_DODGE_DURATION);
    _hp = GameConstants::PLAYER_MAX_HP;
    _defenseUpgrade =GameConstants::PLAYER_DEFENSE;
    _atkDamage =GameConstants::PLAYER_ATK_DAMAGE;
    _moveScale = GameConstants::PLAYER_MOVE_SPEED;

    
    // initialize directions
    _directions[0] = Vec2(0,-1);    //down
    _directions[2] = Vec2(1,0);     //right
    _directions[4] = Vec2(0,1);     //up
    _directions[6] = Vec2(-1,0);    //left
    for (int i = 1; i < 8; i+=2){
        _directions[i] = Vec2(0,-1).rotate(M_PI_4 * i); // diagonal directions
    }
    _directionIndex = 0;
    _facingDirection = _directions[0];
    return true;
}

/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void Player::dispose() {
    _playerTextureKey = "";
}


#pragma mark -
#pragma mark Properties

int Player::getMaxHP(){
    return GameConstants::PLAYER_MAX_HP;
}

#pragma mark -
#pragma mark Animation

void Player::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    
    // TODO: render player with appropriate scales (right now default size)
    auto spriteSheet = _currAnimation->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2::createTranslation(getPosition() * _drawScale);
    spriteSheet->draw(batch, _tint, origin, transform);
    
    // render player differently while dodging (add fading effect)
    if (!_dodgeDuration.isZero()) {
        for (int i = 2; i < 10; i += 2) {
            auto color = Color4(Vec4(1, 1, 1, 1 - i * 0.1));
            Affine2 localTrans = Affine2::createTranslation((getPosition() - _collider->getLinearVelocity() * (i * 0.01)) * _drawScale);
            spriteSheet->draw(batch, color, origin, localTrans);
        }
    }
}

void Player::loadAssets(const std::shared_ptr<AssetManager> &assets){
    //TODO: create json file of keys. organize assets directory
    //TODO: automatically load spritesheets as opposed to textures.... no need to hardcode sheet dimensions....
    _playerTexture = assets->get<Texture>("player-idle");
    auto parryTexture = assets->get<Texture>("player-parry");
    auto attackTexture = assets->get<Texture>("player-attack");
    auto runTexture = assets->get<Texture>("player-run");
    
    // make sheets
    auto parrySheet = SpriteSheet::alloc(parryTexture, 1, 1); // 1 by 1 texture into animation
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 8);
    auto idleSheet = SpriteSheet::alloc(_playerTexture, 8, 8);
    auto runSheet = SpriteSheet::alloc(runTexture, 8, 16);

    // pass to animations
    _parryAnimation = Animation::alloc(parrySheet, 0.5f, false);
    _attackAnimation = Animation::alloc(attackSheet, 0.3f, false, 0, 7);
    _runAnimation = Animation::alloc(runSheet, 16/24.0, true, 0, 15);
    _idleAnimation = Animation::alloc(idleSheet, 1.2f, true, 0, 7);
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
    });
    
    _parryAnimation->onComplete([this](){
        _parryAnimation->reset();
    });
    
    
    setAnimation(_idleAnimation);
}

void Player::animateParry() {
    setAnimation(_parryAnimation);
}

void Player::animateDefault() {
    setAnimation(_idleAnimation);
    
}
void Player::animateAttack() {
    _prevAnimation = _currAnimation;
    setAnimation(_attackAnimation);
}

void Player::setAnimation(std::shared_ptr<Animation> animation){
    if (_currAnimation != animation){
        // MAYBE, we don't want to reset ?? (tweening unsure)
        if (_currAnimation != nullptr){
            _currAnimation->reset();
        }
        GameObject::setAnimation(animation);
    }
}

void Player::updateAnimation(float dt){
    GameObject::updateAnimation(dt); 
    // let all callbacks run (through default update) before custom update
    if (!_attackAnimation->isActive() && !_parryAnimation->isActive()){
        if (!_collider->getLinearVelocity().isZero()){
            setAnimation(_runAnimation);
        }
        else {
            setAnimation(_idleAnimation);
        }
    }
}

#pragma mark -
#pragma mark State Update

void Player::updateCounters(){
    _atkCD.decrement();
    _parryCD.decrement();
    _dodgeCD.decrement();
    _dodgeDuration.decrement();
    _hitCounter.decrement();
    if (_hitCounter.isZero()) _tint = Color4::WHITE;
}

void Player::setFacingDir(cugl::Vec2 dir){
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
    _facingDirection = d;
    
    // sync animation
    if (prevDirection != _directionIndex){
        int startIndex = 8 * _directionIndex;
        int endIndex = startIndex + 8 - 1;
        _idleAnimation->setFrameRange(startIndex, endIndex);
        _attackAnimation->setFrameRange(startIndex, endIndex);
        _runAnimation->setFrameRange(16 * _directionIndex, 16 * _directionIndex + 15);
        // TODO: when parry animation is done, do the same range update.
    }
}

void Player::hit(Vec2 atkDir, int damage) {
    //only get hit if not dodging and not in hitstun
    if (_hitCounter.isZero() && _dodgeDuration.isZero()) {
        _hitCounter.reset();
        _hp = std::fmax(0, (_hp - damage));
        _tint = Color4::RED;
        _collider->setLinearVelocity(atkDir * 10); //tune this value (10)
    }
}
