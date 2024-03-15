//
//  GameObject.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/11/24.
//

#include "GameObject.hpp"

GameObject::GameObject(){
    _tint = Color4::WHITE;
    _position.setZero();
    _drawScale.set(1.0f, 1.0f);
    _enabled = true;
}

#pragma mark -
#pragma mark Animation

void GameObject::updateAnimation(float dt){
    if (_enabled){
        if (_currAnimation != nullptr){
            if (_currAnimation->isStarted()){
                _currAnimation->update(dt);
            }
            else {
                _currAnimation->start();
            }
        }
    }
}


#pragma mark -
#pragma mark Physics

void GameObject::setEnabled(bool value){
    _enabled = value;
    if (_collider != nullptr){
        _collider->setEnabled(_enabled);
    }
    if (_colliderShadow != nullptr){
        _colliderShadow->setEnabled(_enabled);
    }
    if (_outlineSensor != nullptr){
        _outlineSensor->setEnabled(_enabled);
    }
}

void GameObject::syncPositions(){
    Vec2 pos = _position;
    if (_collider != nullptr){
        pos = _collider->getPosition();
    }
    if (_colliderShadow != nullptr){
        _colliderShadow->setPosition(pos);
    }
    if (_outlineSensor != nullptr){
        _outlineSensor->setPosition(pos);
    }
    _position.set(pos);
}

void GameObject::addObstaclesToWorld(std::shared_ptr<physics2::ObstacleWorld> world){
    if (_collider != nullptr){
        world->addObstacle(_collider);
        _collider->getBody()->GetUserData().pointer = reinterpret_cast<intptr_t>(this);
    }
    if (_colliderShadow != nullptr){
        world->addObstacle(_colliderShadow);
        _colliderShadow->getBody()->GetUserData().pointer = reinterpret_cast<intptr_t>(this);
    }
    if (_outlineSensor != nullptr){
        world->addObstacle(_outlineSensor);
        _outlineSensor->getBody()->GetUserData().pointer = reinterpret_cast<intptr_t>(this);
    }
}

void GameObject::removeObstaclesFromWorld(std::shared_ptr<physics2::ObstacleWorld> world){
    if (_collider != nullptr){
        world->removeObstacle(_collider);
    }
    if (_colliderShadow != nullptr){
        world->removeObstacle(_colliderShadow);
    }
    if (_outlineSensor != nullptr){
        world->removeObstacle(_outlineSensor);
    }
}
