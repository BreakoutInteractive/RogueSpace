//
//  Player.cpp
//
//  The player collision-box is represented by a square box rotated on a 45 degree angle.
//  RS
//
//  Created by Zhiyuan Chen on 2/23/24.
//

#include "Player.hpp"

#define HIT_TIME 10
/**the number of frames we wait before allowing another attack, also currently the length of the attack*/
#define ATK_CD 16
/**the number of frames we wait before allowing another parry, also currently the length of the parry*/
#define PARRY_CD 6
/**the number of frames we wait before allowing another dodge*/
#define DODGE_CD 60

#define DODGE_DURATION 10

#define HIT_TIME 10

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool Player::init(const Vec2 pos, const Size size) {
    BoxObstacle::init(pos,size);
    //BoxObstacle::setAngle(M_PI_4);
    std::string name("player");
    setName(name);
    _tint = Color4::WHITE;
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
    _tint = Color4::WHITE;
    
    // TODO: removal later, GAMEPLAY PROTOTYPE set 1-2 frame delay so player does not attack immediately if play button is held for too long
    _atkCD.setCount(1);
    
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
#pragma mark Physics

void Player::applyForce() {
    if (!BoxObstacle::isEnabled()) {
        return;
    }

    //_body->ApplyLinearImpulseToCenter(b2Vec2(_force.x,_force.y), true);

    if (_dodgeDuration.isZero()){
        // when not dodging, set max speed
        auto maxGroundSpeed = 5.0f;
        Vec2 vel = getLinearVelocity();
        if (vel.length() >= maxGroundSpeed) {
            vel.normalize();
            setLinearVelocity(vel * maxGroundSpeed);
        }
    }
    auto pos = getPosition();
    _body->ApplyForce(b2Vec2(_force.x, _force.y), b2Vec2(pos.x, pos.y), true);
}


#pragma mark -
#pragma mark Animation


void Player::setDrawScale(Vec2 scale) {
    _drawScale = scale;
}

void Player::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    
    // TODO: render player with appropriate scales
    
    Vec2 origin = Vec2(_activeAnimation->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2::createTranslation(getPosition() * _drawScale);
    _activeAnimation->draw(batch, _tint, origin, transform);
    if (_attacking) {
        //this weird-looking operation is to advance the animation every other frame instead of every frame so that it is more visible
        int newFrame = _attackAnimation->getFrame() + (_atkCD.getCount() % 2 == 1);
        _attackAnimation->setFrame(newFrame >= _attackAnimation->getSize() ? newFrame - 1 : newFrame);
    }
    else {
        // render player differently while dodging (add fading effect)
        if (!_dodgeDuration.isZero()) {
            for (int i = 2; i < 10; i += 2) {
                auto color = Color4(Vec4(1, 1, 1, 1 - i * 0.1));
                Affine2 localTrans = Affine2::createTranslation((getPosition() - getLinearVelocity() * (i * 0.01)) * _drawScale);
                _activeAnimation->draw(batch, color, origin, localTrans);
            }
        }
    }
    
    // cycle through active animation
    if (_activeAnimation == _idleAnimation){
        // check if we want to advance frame
        int newFrame = _idleAnimation->getFrame() + (_idleCycle.getCount() % 2  == 1);
        _idleAnimation->setFrame(newFrame);
    }
}

void Player::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _playerTexture = assets->get<Texture>(_playerTextureKey);
    auto parryTexture = assets->get<Texture>(_parryTextureKey);
    auto attackTexture = assets->get<Texture>(_attackTextureKey);
    _parryAnimation = SpriteSheet::alloc(parryTexture, 1, 1); // 1 by 1 texture into animation
    _attackAnimation = SpriteSheet::alloc(attackTexture, 8, 8);
    //just use forward-facing for now
    _idleAnimation = SpriteSheet::alloc(_playerTexture, 8, 8);
    _idleAnimation->setFrame(8 * _directionIndex);
    _activeAnimation = _idleAnimation;
}

void Player::animateParry() {
    _activeAnimation = _parryAnimation;
    _attacking = false;
}

void Player::animateDefault() {
    _activeAnimation = _idleAnimation;
    _idleAnimation->setFrame(8 * _directionIndex);
    _attacking = false;
}
void Player::animateAttack() {
    _attacking = true;
    _attackAnimation->setFrame(8 * _directionIndex);
    _activeAnimation = _attackAnimation;
}


#pragma mark -
#pragma mark State Update

void Player::updateCounters(){
    _atkCD.decrement();
    _parryCD.decrement();
    _dodgeCD.decrement();
    _dodgeDuration.decrement();
    _idleCycle.decrement();
    _hitCounter.decrement();
    if (_idleCycle.isZero()){
        _idleCycle.reset();
        _idleAnimation->setFrame(8 * _directionIndex);
    }
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
        _idleAnimation->setFrame(8 * _directionIndex);
        _attackAnimation->setFrame(8 * _directionIndex);
    }
}

void Player::hit() {
    if (_hitCounter.isZero()) {
        _hitCounter.reset();
        _hp -= 1;
        _tint = Color4::RED;
    }
}
