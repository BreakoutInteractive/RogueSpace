#include <cugl/cugl.h>
#include "../models/LevelModel.hpp"
#include "CollisionController.hpp"
#include "../models/LevelConstants.hpp"
#include "../models/Enemy.hpp"
#include "../models/MeleeEnemy.hpp"
#include "../models/RangedEnemy.hpp"
#include "../models/RangedLizard.hpp"
#include "../models/MageAlien.hpp"
#include "../models/Player.hpp"
#include "../models/Wall.hpp"
#include "../models/Relic.hpp"
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>

#pragma mark -
#pragma mark Physics Initialization
void CollisionController::setLevel(std::shared_ptr<LevelModel> level){
    auto world = level->getWorld();
    world->activateCollisionCallbacks(true);
    world->onBeginContact = [this](b2Contact* contact) {
        beginContact(contact);
    };
    world->beforeSolve = [this](b2Contact* contact, const b2Manifold* oldManifold) {
        beforeSolve(contact,oldManifold);
    };
    world->onEndContact = [this](b2Contact* contact) {
        endContact(contact);
    };
    _level = level;
}

void CollisionController::setAssets(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<AudioController>& audio) {
    _assets = assets;
    _audioController = audio;
}

bool CollisionController::isComboContact(){
    bool value = _comboStriked;
    _comboStriked = false;
    return value;
}



#pragma mark -
#pragma mark Collision Handling
void CollisionController::beginContact(b2Contact* contact){
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();
    std::shared_ptr<Player> player = _level->getPlayer();
    std::shared_ptr<PlayerHitbox> meleeHitbox = player->getMeleeHitbox();
    intptr_t aptr = reinterpret_cast<intptr_t>(meleeHitbox.get());
    intptr_t pptr = reinterpret_cast<intptr_t>(player.get());
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        if ((*it)->isEnabled()) {
            intptr_t eptr = reinterpret_cast<intptr_t>((*it).get());
            //attack
            if ((body1->GetUserData().pointer == aptr && body2->GetUserData().pointer == eptr) ||
                (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == aptr)) {
                //attack hitbox is a circle, but we only want it to hit in a semicircle
                Vec2 dir = (*it)->getPosition() * (*it)->getDrawScale() - player->getPosition() * player->getDrawScale();
                dir.normalize();
                float ang = acos(dir.dot(Vec2::UNIT_X));
                if ((*it)->getPosition().y * (*it)->getDrawScale().y < player->getPosition().y * player->getDrawScale().y) ang = 2 * M_PI - ang;
                float hitboxAngle = player->getMeleeHitbox()->getAngle();
                if (abs(ang - hitboxAngle) <= M_PI_2 || abs(ang - hitboxAngle) >= 3 * M_PI_2) {
                    (*it)->hit(dir, player->meleeDamage, !player->isComboStrike() ? GameConstants::KNOCKBACK : GameConstants::KNOCKBACK_PWR_ATK);
                    _audioController->playPlayerFX("attackHit");
                    CULog("Hit an enemy!");
                    // record the hit
                    if (!meleeHitbox->hitFlag){
                        // the hitbox is active and this is the first hit of the frame
                        meleeHitbox->hitFlag = true;
                        if (player->isComboStrike()){
                            // set flag to request "hit pause" effect
                            _comboStriked = true;
                        }
                    }
                }
            }

            //player ranged attack
            for (std::shared_ptr<Projectile> p : _level->getProjectiles()) {
                intptr_t projptr = reinterpret_cast<intptr_t>(p.get());
                if ((body1->GetUserData().pointer == projptr && body2->GetUserData().pointer == eptr) ||
                    (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == projptr)) {
                    //TODO: give projectiles a modifiable damage value
                    //explosion shouldn't hit enemies (or should it?)
                    if (!p->isExploding() && (*it)->isEnabled()) { //need to check isEnabled because projectiles hit corpses for some reason
                        (*it)->hit(((*it)->getPosition() - p->getPosition()).getNormalization(), p->getDamage()*player->bowDamage);
                        CULog("Shot an enemy!");
                        p->setExploding();
                        //_audioController->playPlayerFX("attackHit"); //enemy projectile hit sfx
                    }
                }
            }
        }
    }
    // enemy attack
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        intptr_t aptr = reinterpret_cast<intptr_t>((*it)->getAttack().get());
        if ((body1->GetUserData().pointer == aptr && body2->GetUserData().pointer == pptr)
            || (body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == aptr)) {
            Vec2 dir = player->getPosition() * player->getDrawScale() - (*it)->getPosition() * (*it)->getDrawScale();
            dir.normalize();
            float ang = acos(dir.dot(Vec2::UNIT_X));
            if (player->getPosition().y * player->getDrawScale().y <
                (*it)->getPosition().y *
                (*it)->getDrawScale().y) ang = 2 * M_PI - ang;
            if (abs(ang - (*it)->getAttack()->getAngle()) <= M_PI_2
                || abs(ang - (*it)->getAttack()->getAngle()) >= 3 * M_PI_2) {
                if (player->_state != Player::state::PARRY) {
                    if (body1->GetUserData().pointer == aptr) {
                        physics2::Obstacle* data1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
                        _audioController->playEnemyFX("attackHit", data1->getName());
                    }
                    else {
                        //body1 userdata pointer = pptr
                        physics2::Obstacle* data2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);
                        _audioController->playEnemyFX("attackHit", data2->getName());
                    }
                    player->hit(dir);
                    CULog("Player took damage!");
                }
                else {
                    //successful parry
                    (*it)->setStunned();
                    player->playParryEffect();
                }
            }
        }
    }
    // enemy ranged attack and projectile-wall collisions
    for (std::shared_ptr<Projectile> p : _level->getProjectiles()) {
        intptr_t projptr = reinterpret_cast<intptr_t>(p.get());
        if ((body1->GetUserData().pointer == projptr && body2->GetUserData().pointer == pptr) ||
            (body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == projptr)) {
            Vec2 dir = player->getPosition() * player->getDrawScale() - p->getPosition() * p->getDrawScale();
            dir.normalize();
            if (!p->isExploding() && player->_state != Player::state::DODGE) {
                player->hit(dir, p->getDamage());
                p->setExploding();
                //_audioController->playPlayerFX("attackHit"); //player projectile hit sfx
                CULog("Player got shot!");
            }
        }
        for (std::shared_ptr<Wall> w : _level->getWalls()) {
            intptr_t wptr = reinterpret_cast<intptr_t>(w.get());
            if ((body1->GetUserData().pointer == projptr && body2->GetUserData().pointer == wptr) ||
                (body1->GetUserData().pointer == wptr && body2->GetUserData().pointer == projptr)) {
                //destroy projectile when hitting a wall
                if (!w->getCollider()->isSensor() && !p->isExploding()) p->setExploding();
            }
        }
    }
    
    // player and end-of-level energy sensor collision
    for (std::shared_ptr<EnergyWall> ewall : _level->getEnergyWalls()) {
        intptr_t wallptr = reinterpret_cast<intptr_t>(ewall.get());
        if ((body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == wallptr) ||
            (body1->GetUserData().pointer == wallptr && body2->GetUserData().pointer == pptr)) {
            // make sure it is a sensor that the player walks into
            if (ewall->getCollider()->isSensor()){
                // set the level to be cleared
                _level->setCompleted(true);
            }
        }
    }
    if (_level->getRelic() != nullptr){
        intptr_t relptr = reinterpret_cast<intptr_t>(_level->getRelic().get());
        if (((body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == relptr) ||
            (body1->GetUserData().pointer == relptr && body2->GetUserData().pointer == pptr)) && (_level->getRelic()->getActive())) {
            _level->getRelic()->contactMade.increment();
            CULog("%d", _level->getRelic()->contactMade.getCount());
        }
    }
}


void CollisionController::endContact(b2Contact* contact){
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();
    std::shared_ptr<Player> player = _level->getPlayer();
    intptr_t pptr = reinterpret_cast<intptr_t>(player.get());
    
    if (_level->getRelic() != nullptr){
        intptr_t relptr = reinterpret_cast<intptr_t>(_level->getRelic().get());
        if ((body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == relptr) ||
            (body1->GetUserData().pointer == relptr && body2->GetUserData().pointer == pptr)) {
            _level->getRelic()->contactMade.decrement();
        }
    }
}



void CollisionController::beforeSolve(b2Contact* contact, const b2Manifold* oldManifold){

    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();

    intptr_t pptr = reinterpret_cast<intptr_t>(_level->getPlayer().get());
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        intptr_t eptr = reinterpret_cast<intptr_t>((*it).get());
        if ((body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == eptr) ||
            (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == pptr)) {
            //phase through enemies while dodging
            if (_level->getPlayer()->_state == Player::state::DODGE) contact->SetEnabled(false);
        }
        for (auto iter = enemies.begin(); iter != enemies.end(); ++iter) {
            intptr_t eptr2 = reinterpret_cast<intptr_t>((*iter).get());
            if (eptr != eptr2 && ((body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == eptr2) ||
                (body1->GetUserData().pointer == eptr2 && body2->GetUserData().pointer == eptr))) {
                //enemies phase through each other if one is idle/stunned
                if (!(*it)->_stunCD.isZero() || !(*iter)->_stunCD.isZero()
                    || (*it)->getCollider()->getLinearVelocity().isZero()
                    || (*iter)->getCollider()->getLinearVelocity().isZero()) contact->SetEnabled(false);
            }
        }
        if (body1->GetUserData().pointer == eptr ||body2->GetUserData().pointer == eptr)
            if (!(*it)->isEnabled()) contact->SetEnabled(false);
    }
    for (std::shared_ptr<Projectile> p : _level->getProjectiles()) {
        intptr_t projptr = reinterpret_cast<intptr_t>(p.get());
        if ((body1->GetUserData().pointer == projptr && body2->GetUserData().pointer == pptr) ||
            (body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == projptr)) {
            // ignore projectile-player collision when player is dodging
            if (_level->getPlayer()->_state == Player::state::DODGE) {
                contact->SetEnabled(false);
            }
        }
    }
}
