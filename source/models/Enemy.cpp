//
//  Enemy.cpp
//  RS
//
//  Created by Zhiyuan Chen on 2/26/24.
//

#include "Enemy.hpp"
#include "CollisionConstants.hpp"
#include "../components/Animation.hpp"
#include "LevelModel.hpp"
#include "GameConstants.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Constructors


bool Enemy::init(std::shared_ptr<JsonValue> data) {
    _position.set(data->getFloat("x"), data->getFloat("y"));
    std::shared_ptr<JsonValue> colliderData = data->get("collider");
    Vec2 colliderPos(colliderData->getFloat("x"), colliderData->getFloat("y"));
    std::vector<float> vertices = colliderData->get("vertices")->asFloatArray();
    Vec2* verts = reinterpret_cast<Vec2*>(&vertices[0]);
    Poly2 polygon(verts,(int)vertices.size()/2);
    EarclipTriangulator triangulator;
    triangulator.set(polygon.vertices);
    triangulator.calculate();
    polygon.setIndices(triangulator.getTriangulation());
    
    
    auto collider = std::make_shared<physics2::PolygonObstacle>();
    collider->init(polygon, colliderPos);
    std::string name("enemy-collider");
    collider->setName(name);
    b2Filter filter;
    // this is an enemy and can collide with a player "shadow", an enemy (when not idle), a wall, or an attack
    filter.categoryBits = CATEGORY_ENEMY;
    filter.maskBits = CATEGORY_PLAYER_SHADOW | CATEGORY_ENEMY | CATEGORY_TALL_WALL | CATEGORY_SHORT_WALL | CATEGORY_ATTACK | CATEGORY_PROJECTILE;
    collider->setFilterData(filter);
    _collider = collider;   // attach the collider to the game object
    
    auto shadow = physics2::PolygonObstacle::alloc(polygon,colliderPos);
    shadow->setBodyType(b2_kinematicBody);
    // this is an enemy "shadow" and can collide with the player
    filter.categoryBits = CATEGORY_ENEMY_SHADOW;
    filter.maskBits = CATEGORY_PLAYER;
    shadow->setFilterData(filter);
    _colliderShadow = shadow;
    
    // set the enemy hitbox sensor
    std::shared_ptr<JsonValue> hitboxData = data->get("hitbox");
    Size hitboxSize(hitboxData->getFloat("width"), hitboxData->getFloat("height"));
    Vec2 hitboxPos(hitboxData->getFloat("x"), hitboxData->getFloat("y"));
    auto hitbox = physics2::BoxObstacle::alloc(hitboxPos, hitboxSize);
    hitbox->setBodyType(b2_kinematicBody);
    hitbox->setSensor(true);
    hitbox->setName(std::string("enemy-hitbox"));
    filter.categoryBits = CATEGORY_ENEMY_HITBOX;
    filter.maskBits = CATEGORY_ATTACK | CATEGORY_PROJECTILE;
    hitbox->setFilterData(filter);
    _sensor = hitbox;
    _sensor->setDebugColor(Color4::RED);
    
    // initialize enemy properties
    _isDefault = true;
    _isAiming = false; // will always be false for melee enemies
    _isCharged = false; // will always be false for melee enemies
    _playerLoc = Vec2::ZERO; // default value = hasn't ever seen the player
    _isAligned = false;
    _sightRange = GameConstants::ENEMY_SIGHT_RANGE;
    _proximityRange = GameConstants::ENEMY_PROXIMITY_RANGE;
    _attackRange = GameConstants::ENEMY_MELEE_ATK_RANGE;
    _moveSpeed = GameConstants::ENEMY_MOVE_SPEED;
    _hitCounter.setMaxCount(GameConstants::ENEMY_IFRAME);
    _atkCD.setMaxCount(GameConstants::ENEMY_ATK_COOLDOWN);
    _stunCD.setMaxCount(GameConstants::ENEMY_STUN_COOLDOWN);
    _sentryCD.setMaxCount(GameConstants::ENEMY_SENTRY_COOLDOWN);
    
    // initialize directions
    _directions[0] = Vec2(0,-1);    //down
    _directions[2] = Vec2(1,0);     //right
    _directions[4] = Vec2(0,1);     //up
    _directions[6] = Vec2(-1,0);    //left
    for (int i = 1; i < 8; i+=2){
        _directions[i] = Vec2(0,-1).rotate(M_PI_4 * i); // diagonal directions
    }
    _directionIndex = 0;
    _facingDirection = _directions[0]; // starts facing downward
    return true;
}


/**
 * Disposes all resources and assets of this rocket
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a rocket may not be used until it is initialized again.
 */
void Enemy::dispose() {
    _enemyTextureKey = "";
    _enemyTexture = nullptr;
}


#pragma mark -
#pragma mark Physics

void Enemy::attack(std::shared_ptr<LevelModel> level, const std::shared_ptr<AssetManager> &assets) {
    // nothing here: each enemy implements its own attack
}


#pragma mark -
#pragma mark Animation


void Enemy::setDrawScale(Vec2 scale) {
    _drawScale = scale;
}

void Enemy::draw(const std::shared_ptr<cugl::SpriteBatch>& batch){
    // TODO: render enemy with appropriate scales
    // batch draw(texture, color, origin, scale, angle, offset)
    auto spriteSheet = _currAnimation->getSpriteSheet();
    
    Vec2 origin = Vec2(spriteSheet->getFrameSize().width / 2, 0);
    Affine2 transform = Affine2();
    // transform.scale(0.5);
    transform.translate(getPosition() * _drawScale);
    
    spriteSheet->draw(batch, _tint, origin, transform);

    if (_hitEffect->isActive()) {
        auto effSheet = _hitEffect->getSpriteSheet();
        transform = Affine2::createScale(2);
        transform.translate(getPosition().add(0, 64 / _drawScale.y) * _drawScale); //64 is half of enemy pixel height
        origin = Vec2(effSheet->getFrameSize().width / 2, effSheet->getFrameSize().height / 2);
        effSheet->draw(batch, origin, transform);
    }
    if (_state == EnemyState::STUNNED) {
        auto effSheet = _stunEffect->getSpriteSheet();
        transform = Affine2::createTranslation(getPosition().add(0, 64 / _drawScale.y) * _drawScale); //64 is half of enemy pixel height
        origin = Vec2(effSheet->getFrameSize().width / 2, effSheet->getFrameSize().height / 2);
        effSheet->draw(batch, origin, transform);
    }
}

void Enemy::loadAssets(const std::shared_ptr<AssetManager> &assets){
    // nothing here: each enemy loads its own assets
}

void Enemy::setIdling() {
    setAnimation(_idleAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _walkAnimation->reset();
    _attackAnimation->reset();
    _stunAnimation->reset();
    _state = EnemyState::IDLE;
    
}

void Enemy::setMoving() {
    setAnimation(_walkAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _attackAnimation->reset();
    _stunAnimation->reset();
    _state = EnemyState::MOVING;
}

void Enemy::setAttacking() {
    setAnimation(_attackAnimation);
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _idleAnimation->reset();
    _walkAnimation->reset();
    _stunAnimation->reset();
    _state = EnemyState::ATTACKING;
}

void Enemy::setStunned() {
    if (_state == EnemyState::STUNNED) {
        return;
    }
    setAnimation(_stunAnimation);
    _stunCD.reset();
    _atkCD.reset(); // stunning should reset attack
    // MAYBE, we don't want to reset ?? (tweening unsure)
    _attackAnimation->reset();
    _hitboxAnimation->reset();
    _idleAnimation->reset();
    _walkAnimation->reset();
    _state = EnemyState::STUNNED;
    _stunEffect->start();
}


void Enemy::hit(cugl::Vec2 atkDir, int damage, float knockback_scl) {
    if (!_hitEffect->isActive()) {
        _hitCounter.reset();
        setHealth(getHealth()-damage);
        _hitEffect->reset();
        _hitEffect->start();
        _collider->setLinearVelocity(atkDir * knockback_scl);
        // allows for a "revenge" attack if the enemy is attacked from behind
        if (!_playerInSight) {
            _facingDirection = -atkDir;
        }
    }
}

void Enemy::updateAnimation(float dt){
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
    _hitEffect->update(dt);
    if (_hitEffect->isActive()){
        _tint = Color4::RED;
    }
    else if (_state == EnemyState::STUNNED && _stunCD.isZero()) {
        _tint = Color4::WHITE;
        setIdling();
    }
    else if (_state == EnemyState::STUNNED){
        // TODO: could possibly use stunned animation and remove this state altogether
        //_tint = Color4::YELLOW;
    }
    else {
        _tint = Color4::WHITE;
    }
    _stunEffect->update(dt);
    _hitboxAnimation->update(dt);
}

void Enemy::updateCounters() {
    _sentryCD.decrement();
    _stunCD.decrement();
    _atkCD.decrement();
    _hitCounter.decrement();
}

void Enemy::setFacingDir(cugl::Vec2 dir) {
    // nothing here: each enemy has its own animations
}
