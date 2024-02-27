//
//  Player.cpp
//
//  The player collision-box is represented by a square box rotated on a 45 degree angle.
//  RS
//
//  Created by Zhiyuan Chen on 2/23/24.
//

#include "Player.hpp"

/**the number of frames we wait before allowing another attack, also currently the length of the attack*/
#define ATK_CD 8
/**the number of frames we wait before allowing another parry, also currently the length of the parry*/
#define PARRY_CD 6
/**the number of frames we wait before allowing another dodge*/
#define DODGE_CD 60

#define DODGE_DURATION 10

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool Player::init(const Vec2 pos, const Size size) {
    BoxObstacle::init(pos,size);
    BoxObstacle::setAngle(M_PI_4);
    std::string name("player");
    setName(name);
    // set the counter properties
    _atkCD.setMaxCount(ATK_CD);
    _parryCD.setMaxCount(PARRY_CD);
    _dodgeCD.setMaxCount(DODGE_CD);
    _dodgeDuration.setMaxCount(DODGE_DURATION);
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
    
    // Orient the force with rotation.
    //Vec4 netforce(_force.x,_force.y,0.0f,1.0f);
    //Mat4::createRotationZ(getAngle(),&_affine);
    //netforce *= _affine;
    
    // Apply force to the rocket BODY, not the rocket

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

void Player::update(float delta) {
    Obstacle::update(delta);
}


#pragma mark -
#pragma mark Animation


void Player::setDrawScale(Vec2 scale) {
    _drawScale = scale;
}

void Player::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render player with appropriate scales
    // batch draw(texture, color, origin, scale, angle, offset)
    if (_attacking) { 
        //idk why but the position needs to be offset by -width/2
        Affine2 t = Affine2::createTranslation(getPosition() * _drawScale-Vec2(_playerTexture->getWidth()/2, 0));
        _attackAnimation->draw(batch, Color4::WHITE, Vec2(_activeTexture->getWidth() / 2, 0), t); 
        int newFrame = _attackAnimation->getFrame() + 1;
        //since we are only using the front-facing animation for now, always reset it to the start of that animation if we are out of bounds of it
        _attackAnimation->setFrame(newFrame > 47 || newFrame < 40 ? 40 : newFrame);
    }
    else batch->draw(_activeTexture,Color4::WHITE, Vec2(_activeTexture->getWidth()/2, 0), Vec2::ONE, 0, getPosition() * _drawScale);
}

void Player::loadAssets(const std::shared_ptr<AssetManager> &assets){
    _playerTexture = assets->get<Texture>(_playerTextureKey);
    _parryTexture = assets->get<Texture>(_parryTextureKey);
    _attackTexture = assets->get<Texture>(_attackTextureKey);
    _attackAnimation = SpriteSheet::alloc(_attackTexture, 8, 8);
    //just use forward-facing for now
    _attackAnimation->setFrame(40);
    _activeTexture = _playerTexture;
}

void Player::animateParry() {
    _activeTexture = _parryTexture;
    _attacking = false;
}

void Player::animateDefault() {
    _activeTexture = _playerTexture;
    _attacking = false;
}
void Player::animateAttack() {
    _attacking = true;
    //_activeTexture = _attackTexture;
}

#pragma mark -
#pragma mark State Update

void Player::updateCounters(){
    // if the counter
    _atkCD.decrement();
    _parryCD.decrement();
    _dodgeCD.decrement();
    _dodgeDuration.decrement();
}
