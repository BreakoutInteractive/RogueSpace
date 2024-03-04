#include <cugl/cugl.h>
#include "../models/LevelModel.hpp"
#include "CollisionController.hpp"

// The motivation to separate this out of GameScene is to let the focus of here to be dealing with combat collisioos and other collision events
// GameScene can focus on player movement, controls, and surface level management.

void CollisionController::setLevel(std::shared_ptr<LevelModel> level){
    // TODO:
    auto world = level->getWorld();
    world->activateCollisionCallbacks(true);
    world->onBeginContact = [this](b2Contact* contact) {
        beginContact(contact);
    };
    world->beforeSolve = [this](b2Contact* contact, const b2Manifold* oldManifold) {
        beforeSolve(contact,oldManifold);
    };
}

void CollisionController::beginContact(b2Contact* contact){

}


void CollisionController::beforeSolve(b2Contact* contact, const b2Manifold* oldManifold){

}
