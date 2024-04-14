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


bool Player::init(std::shared_ptr<JsonValue> playerData) {
    _weapon = MELEE;
    _state = IDLE;
    _dodge = 0;
    bool success = true;
    _position.set(playerData->getFloat("x"), playerData->getFloat("y"));
    std::shared_ptr<JsonValue> colliderData = playerData->get("collider");
    Vec2 playerColliderPos(colliderData->getFloat("x"), colliderData->getFloat("y"));
    std::vector<float> vertices = colliderData->get("vertices")->asFloatArray();
    success = vertices.size() >= 2 && vertices.size() % 2 == 0;
    Vec2* verts = reinterpret_cast<Vec2*>(&vertices[0]);
    Poly2 polygon(verts,(int)vertices.size()/2);
    EarclipTriangulator triangulator;
    triangulator.set(polygon.vertices);
    triangulator.calculate();
    polygon.setIndices(triangulator.getTriangulation());
    
    // set up collider
    auto collider = std::make_shared<physics2::PolygonObstacle>();
    collider->init(polygon, playerColliderPos);
    collider->setName(std::string("player-collider"));
    // this is a player and can collide with an enemy "shadow", a wall, or an attack
    b2Filter filter;
    filter.categoryBits = CATEGORY_PLAYER;
    filter.maskBits = CATEGORY_ENEMY_SHADOW | CATEGORY_WALL | CATEGORY_ATTACK | CATEGORY_PROJECTILE;
    collider->setFilterData(filter);
    _collider = collider;                   // attach Component
    
    // set the player collider-shadow
    auto colliderShadow = std::make_shared<physics2::PolygonObstacle>();
    colliderShadow->init(polygon, playerColliderPos);
    colliderShadow->setName(std::string("player-collider-shadow"));
    colliderShadow->setBodyType(b2_kinematicBody);
    filter.categoryBits = CATEGORY_PLAYER_SHADOW;
    filter.maskBits = CATEGORY_ENEMY;
    colliderShadow->setFilterData(filter);
    _colliderShadow = colliderShadow;       // attach Component
    _colliderShadow->setDebugColor(Color4::BLACK);
    
    // set the player hitbox sensor
    std::shared_ptr<JsonValue> hitboxData = playerData->get("hitbox");
    Size hitboxSize(hitboxData->getFloat("width"), hitboxData->getFloat("height"));
    Vec2 hitboxPos(hitboxData->getFloat("x"), hitboxData->getFloat("y"));
    auto hitbox = physics2::BoxObstacle::alloc(hitboxPos, hitboxSize);
    hitbox->setBodyType(b2_kinematicBody);
    hitbox->setSensor(true);
    hitbox->setName(std::string("player-hitbox"));
    filter.categoryBits = CATEGORY_PLAYER_HITBOX;
    filter.maskBits = CATEGORY_ATTACK | CATEGORY_PROJECTILE;
    hitbox->setFilterData(filter);
    _sensor = hitbox;
    _sensor->setDebugColor(Color4::RED);
    
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

void Player::dispose() {
    _playerTextureKey = "";
}


#pragma mark -
#pragma mark Properties

int Player::getMaxHP(){
    return GameConstants::PLAYER_MAX_HP;
}

bool Player::isAttacking() {
    return _state == CHARGING || _state == CHARGED || _state == SHOT;
}

#pragma mark -
#pragma mark Animation

void Player::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render player with appropriate scales (right now default size)
    auto spriteSheet = _currAnimation->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2::createTranslation(_position * _drawScale);
    spriteSheet->draw(batch, _tint, origin, transform);
    
    //effects
    std::shared_ptr<SpriteSheet> sheet;
    Vec2 o = Vec2::ZERO;
    Affine2 t = Affine2::ZERO;
    Vec2 direction = Vec2::ZERO;
    float ang = 0;
    switch (_state) {
    case DODGE:
        // render player differently while dodging (add fading effect)
        for (int i = 2; i < 10; i += 2) {
            auto color = Color4(Vec4(1, 1, 1, 1 - i * 0.1));
            Affine2 localTrans = Affine2::createTranslation((_position - _collider->getLinearVelocity() * (i * 0.01)) * _drawScale);
            spriteSheet->draw(batch, color, origin, localTrans);
        }
        break;
    case CHARGED:
        sheet = _chargedEffect->getSpriteSheet();
        o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
        direction = getFacingDir();
        ang = acos(direction.dot(Vec2::UNIT_X));
        if (direction.y < 0) {
            // handle downwards case, rotate counterclockwise by PI rads and add extra angle
            ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
        }
        t = Affine2::createRotation(ang);
        t.translate(getPosition().add(0, 64 / getDrawScale().y) * _drawScale);
        sheet->draw(batch, o, t);
        break;
    case CHARGING:
        sheet = _chargingEffect->getSpriteSheet();
        o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
        direction = getFacingDir();
        ang = acos(direction.dot(Vec2::UNIT_X));
        if (direction.y < 0) {
            // handle downwards case, rotate counterclockwise by PI rads and add extra angle
            ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
        }
        t = Affine2::createRotation(ang);
        t.translate(getPosition().add(0, 64 / getDrawScale().y) * _drawScale);
        sheet->draw(batch, o, t);
        break;
    case SHOT:
        sheet = _shotEffect->getSpriteSheet();
        o = Vec2(sheet->getFrameSize().width / 2, sheet->getFrameSize().height / 2);
        direction = getFacingDir();
        ang = acos(direction.dot(Vec2::UNIT_X));
        if (direction.y < 0) {
            // handle downwards case, rotate counterclockwise by PI rads and add extra angle
            ang = M_PI + acos(direction.rotate(M_PI).dot(Vec2::UNIT_X));
        }
        t = Affine2::createRotation(ang);
        t.translate(getPosition().add(0, 64 / getDrawScale().y) * _drawScale);
        if (_shotEffect->isActive()) sheet->draw(batch, o, t);
        break;
    default:
        break;
    }
}

void Player::loadAssets(const std::shared_ptr<AssetManager> &assets){
    //TODO: create json file of keys. organize assets directory
    //TODO: automatically load spritesheets as opposed to textures.... no need to hardcode sheet dimensions....
    _playerTexture = assets->get<Texture>("player-idle");
    auto parryTexture = assets->get<Texture>("player-parry");
    auto attackTexture = assets->get<Texture>("player-attack");
    auto runTexture = assets->get<Texture>("player-run");
    auto bowIdleTexture = assets->get<Texture>("player-bow-idle");
    auto rangedTexture = assets->get<Texture>("player-ranged");
    auto bowRunTexture = assets->get<Texture>("player-bow-run");
    auto projEffectTexture = assets->get<Texture>("player-projectile");
    
    // make sheets
    auto parrySheet = SpriteSheet::alloc(parryTexture, 8, 16);
    auto attackSheet = SpriteSheet::alloc(attackTexture, 8, 8);
    auto idleSheet = SpriteSheet::alloc(_playerTexture, 8, 8);
    auto runSheet = SpriteSheet::alloc(runTexture, 8, 16);
    auto rangedSheet = SpriteSheet::alloc(rangedTexture, 8, 16);
    auto bowIdleSheet = SpriteSheet::alloc(bowIdleTexture, 8, 8);
    auto bowRunSheet = SpriteSheet::alloc(bowRunTexture, 8, 16);
    auto projEffectSheet = SpriteSheet::alloc(projEffectTexture, 4, 4);

    // pass to animations
    _parryStartAnimation = Animation::alloc(parrySheet, 0.1f, false, 0, 1);
    _parryStanceAnimation = Animation::alloc(parrySheet, 0.5f, true, 2, 9);
    _parryAnimation = Animation::alloc(parrySheet, GameConstants::PLAYER_PARRY_TIME, false, 10, 15);
    _attackAnimation = Animation::alloc(attackSheet, 0.3f, false, 0, 7);
    _runAnimation = Animation::alloc(runSheet, 16/24.0, true, 0, 15);
    _idleAnimation = Animation::alloc(idleSheet, 1.2f, true, 0, 7);
    _chargingAnimation = Animation::alloc(rangedSheet, GameConstants::CHARGE_TIME, false, 0, 8);
    _chargedAnimation = Animation::alloc(rangedSheet, 0.1f, true, 8, 8);
    _shotAnimation = Animation::alloc(rangedSheet, 0.125f, false, 9, 11);
    _recoveryAnimation = Animation::alloc(rangedSheet, 0.167f, false, 12, 15);
    _bowRunAnimation = Animation::alloc(bowRunSheet, 16 / 24.0f, true, 0, 15);
    _bowIdleAnimation = Animation::alloc(bowIdleSheet, 1.2f, true, 0, 7);
    _chargingEffect = Animation::alloc(projEffectSheet, GameConstants::CHARGE_TIME, false, 0, 3);
    _chargedEffect = Animation::alloc(projEffectSheet, 8 / 24.0f, true, 4, 7);
    _shotEffect = Animation::alloc(projEffectSheet, 1/24.0f, false, 8, 8);
    
    // add callbacks
    _attackAnimation->onComplete([this](){
        _attackAnimation->reset();
        _state = IDLE;
        });
    _parryStartAnimation->onComplete([this]() {
        _parryStartAnimation->reset();
        setAnimation(_parryStanceAnimation);
        _parryStanceAnimation->start();
        _state = PARRYSTANCE;
        });
    _parryAnimation->onComplete([this](){
        _parryAnimation->reset();
        _state = IDLE;
        });
    _shotAnimation->onComplete([this]() {
        _shotAnimation->reset();
        _shotEffect->reset();
        setAnimation(_recoveryAnimation);
        _recoveryAnimation->start();
        _state = RECOVERY;
        });
    _recoveryAnimation->onComplete([this]() {
        _recoveryAnimation->reset();
        _state = IDLE;
        });
    _chargingAnimation->onComplete([this]() {
        setAnimation(_chargedAnimation);
        _chargedAnimation->start();
        _chargingAnimation->reset();
        _state = CHARGED;
        });
    _chargingEffect->onComplete([this]() {
        _chargingEffect->reset();
        _chargedEffect->start();
        _state = CHARGED;
        });
    
    setAnimation(_idleAnimation);
}

void Player::animateParryStart() {
    setAnimation(_parryStartAnimation);
    _state = PARRYSTART;
}
void Player::animateParry() {
    setAnimation(_parryAnimation);
    _state = PARRY;
}

void Player::animateDefault() {
    _state = IDLE;
    switch (_weapon) {
    case MELEE:
        setAnimation(_idleAnimation);
        break;
    case RANGED:
        setAnimation(_bowIdleAnimation);
        break;
    }
}
void Player::animateAttack() {
    _state = ATTACK;
    _prevAnimation = _currAnimation;
    setAnimation(_attackAnimation);
}
void Player::animateCharge() {
    setAnimation(_chargingAnimation);
    _chargingEffect->start();
    _state = CHARGING;
}

void Player::animateShot() {
    setAnimation(_shotAnimation);
    _shotEffect->start();
    _state = SHOT;
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
    //the running/idle animations only apply in idle or dodge states; the other states have their own animations
    if (_state == IDLE || _state == DODGE){
        //TODO: sync frames when swapping weapons
        if (!_collider->getLinearVelocity().isZero()){
            switch (_weapon) {
            case MELEE:
                setAnimation(_runAnimation);
                break;
            case RANGED:
                setAnimation(_bowRunAnimation);
                break;
            }
        }
        else {
            switch (_weapon) {
            case MELEE:
                setAnimation(_idleAnimation);
                break;
            case RANGED:
                setAnimation(_bowIdleAnimation);
                break;
            }
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
        int startIdx16 = 16 * _directionIndex;
        int endIndex = startIndex + 8 - 1;
        _idleAnimation->setFrameRange(startIndex, endIndex);
        _attackAnimation->setFrameRange(startIndex, endIndex);
        _runAnimation->setFrameRange(startIdx16, startIdx16 + 15);
        _parryStartAnimation->setFrameRange(startIdx16, startIdx16 + 1);
        _parryStanceAnimation->setFrameRange(startIdx16 + 2, startIdx16 + 9);
        _parryAnimation->setFrameRange(startIdx16 + 10, startIdx16 + 15);
        _bowIdleAnimation->setFrameRange(startIndex, endIndex);
        _bowRunAnimation->setFrameRange(startIdx16, startIdx16 + 15);
        _chargingAnimation->setFrameRange(startIdx16, startIdx16 + 8);
        _chargedAnimation->setFrameRange(startIdx16 + 8, startIdx16 + 8);
        _shotAnimation->setFrameRange(startIdx16 + 9, startIdx16 + 11);
        _recoveryAnimation->setFrameRange(startIdx16 + 12, startIdx16 + 15);
    }
}

void Player::hit(Vec2 atkDir, int damage) {
    //only get hit if not dodging and not in hitstun
    if (_hitCounter.isZero() && _state != DODGE) {
        _hitCounter.reset();
        _hp = std::fmax(0, (_hp - damage));
        _tint = Color4::RED;
        _collider->setLinearVelocity(atkDir * GameConstants::KNOCKBACK);
        _state = IDLE; //TODO: hit state ???
    }
}

void Player::update(float dt) {
    updateCounters();
    updateAnimation(dt);
    switch (_state) {
    case CHARGED:
        _chargedEffect->update(dt);
        break;
    case CHARGING:
        if (_chargingEffect->isActive()) {
            _chargingEffect->update(dt);
        }
        else {
            _chargingEffect->start();
        }
        break;
    case DODGE:
        _dodge += dt;
        if (_dodge >= GameConstants::PLAYER_DODGE_TIME) {
            _state = IDLE;
            _dodge = 0;
        }
        break;
    case SHOT:
        _chargedAnimation->reset();
        _chargedEffect->reset();
        _shotEffect->update(dt);
        break;
    case IDLE: case ATTACK: case RECOVERY: case PARRYSTART: case PARRYSTANCE: case PARRY:
        // no additional updates
        break;
    }
}
