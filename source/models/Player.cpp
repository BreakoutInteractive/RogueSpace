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
#include "../components/Animation.hpp"

//should be at least the enemy's attack time so that we can't get hit twice by the same attack
#define HIT_TIME 16
/**the number of frames we wait before allowing another attack, also currently the length of the attack*/
#define ATK_CD 16
/**the number of frames we wait before allowing another parry, also currently the length of the parry*/
#define PARRY_CD 6
/**the number of frames we wait before allowing another dodge*/
#define DODGE_CD 60

#define DODGE_DURATION 10

#define MAX_HP 3

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
    _hitCounter.setMaxCount(HIT_TIME);
    _atkCD.setMaxCount(ATK_CD);
    _parryCD.setMaxCount(PARRY_CD);
    _dodgeCD.setMaxCount(DODGE_CD);
    _dodgeDuration.setMaxCount(DODGE_DURATION);
    _idleCycle.setMaxCount(16);
    _idleCycle.reset();
    _hp = 3;
    _hitCounter.setMaxCount(HIT_TIME);
    
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
    return MAX_HP;
}

#pragma mark -
#pragma mark Animation


void Player::setDrawScale(Vec2 scale) {
    _drawScale = scale;
}

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
    _parryAnimation = Animation::alloc(parrySheet, 1.0f, false);
    _attackAnimation = Animation::alloc(attackSheet, 0.3f, false, 0, 7);
    _runAnimation = Animation::alloc(runSheet, 16/24.0, true, 0, 15);
    _idleAnimation = Animation::alloc(idleSheet, 1.2f, true, 0, 7);
    
    _currAnimation = _idleAnimation; // set runnning
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        setAnimation(_idleAnimation);
    });
    
    
    setAnimation(_idleAnimation);
}

void Player::animateParry() {
    _currAnimation = _parryAnimation;
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _runAnimation->reset();
    _attackAnimation->reset();
}

void Player::animateDefault() {
    _currAnimation = _idleAnimation;
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _runAnimation->reset();
    _parryAnimation->reset();
    
}
void Player::animateAttack() {
    _currAnimation = _attackAnimation;
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _runAnimation->reset();
    _parryAnimation->reset();
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

void Player::hit(Vec2 atkDir) {
    //only get hit if not dodging and not in hitstun
    if (_hitCounter.isZero() && _dodgeDuration.isZero()) {
        _hitCounter.reset();
        _hp = std::max(0, _hp - 1);
        _tint = Color4::RED;
        _collider->setLinearVelocity(atkDir * 10); //tune this value (10)
    }
}
