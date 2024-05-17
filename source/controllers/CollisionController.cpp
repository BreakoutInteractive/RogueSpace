#include <cugl/cugl.h>
#include "../models/LevelModel.hpp"
#include "CollisionController.hpp"
#include "../models/LevelConstants.hpp"
#include "../models/CollisionConstants.hpp"
#include "../models/Enemy.hpp"
#include "../models/MeleeEnemy.hpp"
#include "../models/RangedEnemy.hpp"
#include "../models/RangedLizard.hpp"
#include "../models/MageAlien.hpp"
#include "../models/ExplodingAlien.hpp"
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

void CollisionController::setAssets(const std::shared_ptr<AssetManager>& assets) {
    _assets = assets;
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
    std::shared_ptr<SemiCircleHitbox> meleeHitbox = player->getMeleeHitbox();
    intptr_t aptr = reinterpret_cast<intptr_t>(meleeHitbox.get());
    intptr_t pptr = reinterpret_cast<intptr_t>(player.get());
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    int enemyIndex = 0;
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        if ((*it)->isEnabled() && (*it)->getHealth() > 0) {
            intptr_t eptr = reinterpret_cast<intptr_t>((*it).get());
            //attack
            if ((body1->GetUserData().pointer == aptr && body2->GetUserData().pointer == eptr) ||
                (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == aptr)) {
                Vec2 dir = (*it)->getPosition() * (*it)->getDrawScale() - player->getPosition() * player->getDrawScale();
                dir.normalize();
                float ang = acos(dir.dot(Vec2::UNIT_X));
                if ((*it)->getPosition().y * (*it)->getDrawScale().y < player->getPosition().y * player->getDrawScale().y) ang = 2 * M_PI - ang;
                // make sure this enemy isn't already hit by asking whether the hitbox hits the enemy
                if (player->getMeleeHitbox()->hits(eptr, ang)){
                    (*it)->hit(dir, false, player->getMeleeDamage(), !player->isComboStrike() ? GameConstants::KNOCKBACK : GameConstants::KNOCKBACK_PWR_ATK);
                    AudioController::playEnemyFX("damaged", std::to_string(enemyIndex));
                    CULog("Hit an enemy!");
                    if (meleeHitbox->hitCount() == 1){
                        // the hitbox is active and this is the first hit of the frame
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
                    //explosion shouldn't hit enemies (or should it?)
                    if (!p->isExploding() && (*it)->isEnabled() && (*it)->getHealth() > 0) { //need to check isEnabled because projectiles hit corpses for some reason
                        (*it)->hit(((*it)->getPosition() - p->getPosition()).getNormalization(), true, p->getDamage());
                        AudioController::playEnemyFX("damaged", std::to_string(enemyIndex));
                        CULog("Shot an enemy!");
                        p->setExploding();
                    }
                }
            }
        }
        enemyIndex++;
    }
    //health packs
    for (std::shared_ptr<HealthPack> h : _level->getHealthPacks()) {
        intptr_t hptr = reinterpret_cast<intptr_t>(h.get());
        if ((body1->GetUserData().pointer == hptr && body2->GetUserData().pointer == pptr) ||
            (body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == hptr)) {
            //don't pick up the health pack if at full hp
            if (player->getHP() < player->getMaxHP()) {
                float maxHP = player->getMaxHP();
                float newHP = player->getHP() + maxHP * GameConstants::HEALTHPACK_HEAL_AMT;
                if (newHP > maxHP) newHP = maxHP;
                player->setHP(newHP);
                h->_delMark = true;
            }
        }
    }
    // enemy melee attack
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        std::shared_ptr<MeleeEnemy> melee;
        std::shared_ptr<ExplodingAlien> explode;
        std::shared_ptr<Hitbox> attack;
        if ((*it)->getType() == "melee lizard" || (*it)->getType() == "tank enemy") {
            melee = std::dynamic_pointer_cast<MeleeEnemy>(*it);
            attack = melee->getAttack();
        }
        else if ((*it)->getType() == "exploding alien"){
            explode = std::dynamic_pointer_cast<ExplodingAlien>(*it);
            attack = explode->getAttack();
        }
        if (attack != nullptr){
            intptr_t aptr = reinterpret_cast<intptr_t>(attack.get());
            if ((body1->GetUserData().pointer == aptr && body2->GetUserData().pointer == pptr)
                || (body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == aptr)) {
                Vec2 dir = player->getPosition() * player->getDrawScale() - (*it)->getPosition() * (*it)->getDrawScale();
                dir.normalize();
                float ang = acos(dir.dot(Vec2::UNIT_X));
                if (player->getPosition().y * player->getDrawScale().y < (*it)->getPosition().y * (*it)->getDrawScale().y) ang = 2 * M_PI - ang;
                if (attack->hits(pptr, ang)){
                    if (player->isParrying() && melee != nullptr) {
                        //successful parry
                        melee->setStunned(player->getStunWindow());
                        player->playParryEffect();
                        AudioController::playPlayerFX("parry");
                    }
                    else {
                        player->hit(dir, (*it)->getDamage());
                        AudioController::playPlayerFX("damaged");
                        CULog("Player took damage!");
                    }
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
            if (!p->isExploding() && !player->isDodging()) {
                p->setExploding();
                if (!player->isParrying()) {
                    player->hit(dir, p->getDamage());
                    AudioController::playPlayerFX("damaged");
                    CULog("Player got shot!");
                }
                else player->playParryEffect();
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
            if (_level->getPlayer()->isDodging()) contact->SetEnabled(false);
            //player phase through slime when slime is in self-destruct mode
            if ((*it)->getType() == "exploding alien" && (*it)->getHealth() == 0) contact->SetEnabled(false);
        }
        for (auto iter = enemies.begin(); iter != enemies.end(); ++iter) {
            intptr_t eptr2 = reinterpret_cast<intptr_t>((*iter).get());
            if (eptr != eptr2 && ((body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == eptr2) ||
                (body1->GetUserData().pointer == eptr2 && body2->GetUserData().pointer == eptr))) {
                //enemies phase through each other if one is idle/stunned
                if ((*it)->getType() == "melee lizard" || (*it)->getType() == "tank enemy") {
                    std::shared_ptr<MeleeEnemy> m = std::dynamic_pointer_cast<MeleeEnemy>(*it);
                    if (m->isStunned()) {
                        contact->SetEnabled(false);
                    }
                }
                if ((*iter)->getType() == "melee lizard" || (*iter)->getType() == "tank enemy") {
                    std::shared_ptr<MeleeEnemy> m = std::dynamic_pointer_cast<MeleeEnemy>(*iter);
                    if (m->isStunned()) {
                        contact->SetEnabled(false);
                    }
                }
                if ((*it)->getCollider()->getLinearVelocity().isZero() ||
                    (*iter)->getCollider()->getLinearVelocity().isZero()) contact->SetEnabled(false);
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
            if (_level->getPlayer()->isDodging()) {
                contact->SetEnabled(false);
            }
        }
    }
}
