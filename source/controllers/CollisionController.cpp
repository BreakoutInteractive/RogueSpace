#include <cugl/cugl.h>
#include "../models/LevelModel.hpp"
#include "CollisionController.hpp"
#include "../models/LevelConstants.hpp"
#include "../models/Enemy.hpp"
#include "../models/Player.hpp"
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
    _level = level;
}

void CollisionController::setAssets(const std::shared_ptr<AssetManager>& assets) {
    _assets = assets;
}


#pragma mark -
#pragma mark Collision Handling
void CollisionController::beginContact(b2Contact* contact){

    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();
    intptr_t aptr = reinterpret_cast<intptr_t>(_level->getAttack().get());
    intptr_t pptr = reinterpret_cast<intptr_t>(_level->getPlayer().get());
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        intptr_t eptr = reinterpret_cast<intptr_t>((*it).get());
        //attack
        if ((body1->GetUserData().pointer == aptr && body2->GetUserData().pointer == eptr) ||
            (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == aptr)) {
            //attack hitbox is a circle, but we only want it to hit in a semicircle
            Vec2 dir = (*it)->getPosition() * (*it)->getDrawScale() - _level->getPlayer()->getPosition() * _level->getPlayer()->getDrawScale();
            dir.normalize();
            float ang = acos(dir.dot(Vec2::UNIT_X));
            if ((*it)->getPosition().y * (*it)->getDrawScale().y < _level->getPlayer()->getPosition().y * _level->getPlayer()->getDrawScale().y) ang = 2 * M_PI - ang;
            if (abs(ang - _level->getAttack()->getAngle()) <= M_PI_2 || abs(ang - _level->getAttack()->getAngle()) >= 3 * M_PI_2) {
                (*it)->hit(dir);
                CULog("Hit an enemy!");
            }
        }
        //player takes damage if running into enemy while not dodging
        else if ((body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == eptr) ||
            (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == pptr)) {
            Vec2 dir = (_level->getPlayer()->getPosition() - (*it)->getPosition());
            dir.normalize();
            if (_level->getPlayer()->_dodgeDuration.isZero()) _level->getPlayer()->hit(dir);
        }
    }
    // enemy attack
    std::shared_ptr<Player> player = _level->getPlayer();
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
                if (player->_parryCD.isZero()) {
                    player->hit(dir);
                    CULog("Player took damage!");
                }
                else {
                    (*it)->stun();
                }
            }
        }
    }
}


void CollisionController::beforeSolve(b2Contact* contact, const b2Manifold* oldManifold){

    float speed = 0;

    // Use Ian Parberry's method to compute a speed threshold
    b2Body* body1 = contact->GetFixtureA()->GetBody();
    b2Body* body2 = contact->GetFixtureB()->GetBody();
    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);
    b2PointState state1[2], state2[2];
    b2GetPointStates(state1, state2, oldManifold, contact->GetManifold());
    for (int ii = 0; ii < 2; ii++) {
        if (state2[ii] == b2_addState) {
            b2Vec2 wp = worldManifold.points[0];
            b2Vec2 v1 = body1->GetLinearVelocityFromWorldPoint(wp);
            b2Vec2 v2 = body2->GetLinearVelocityFromWorldPoint(wp);
            b2Vec2 dv = v1 - v2;
            speed = b2Dot(dv, worldManifold.normal);
        }
    }

    // Play a sound if above threshold
    // TODO: get the audio controller to play our SFX
//    if (speed > SOUND_THRESHOLD) {
//        // These keys result in a low number of sounds.  Too many == distortion.
//        physics2::Obstacle* data1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
//        physics2::Obstacle* data2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);
//
//        if (data1 != nullptr && data2 != nullptr) {
//            std::string key = (data1->getName() + data2->getName());
//            auto source = _assets->get<Sound>(COLLISION_SOUND);
//            if (!AudioEngine::get()->isActive(key)) {
//                AudioEngine::get()->play(key, source, false, source->getVolume());
//            }
//        }
//    }

    intptr_t pptr = reinterpret_cast<intptr_t>(_level->getPlayer().get());
    std::vector<std::shared_ptr<Enemy>> enemies = _level->getEnemies();
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        intptr_t eptr = reinterpret_cast<intptr_t>((*it).get());
        if ((body1->GetUserData().pointer == pptr && body2->GetUserData().pointer == eptr) ||
            (body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == pptr)) {
            //phase through enemies while dodging
            if (!_level->getPlayer()->_dodgeDuration.isZero()) contact->SetEnabled(false);
        }
        for (auto iter = enemies.begin(); iter != enemies.end(); ++iter) {
            intptr_t eptr2 = reinterpret_cast<intptr_t>((*iter).get());
            if (eptr != eptr2 && ((body1->GetUserData().pointer == eptr && body2->GetUserData().pointer == eptr2) ||
                (body1->GetUserData().pointer == eptr2 && body2->GetUserData().pointer == eptr))) {
                //enemies phase through each other if one is idle/stunned
                if (!(*it)->_stunCD.isZero() || !(*iter)->_stunCD.isZero()
                    ||
                    (*it)->getCollider()->getLinearVelocity().isZero() || (*iter)->getCollider()->getLinearVelocity().isZero()) contact->SetEnabled(false);
            }
        }
    }
}
