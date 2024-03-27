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
        _collider->getDebugNode()->setVisible(_enabled);
    }
    if (_colliderShadow != nullptr){
        _colliderShadow->setEnabled(_enabled);
        _colliderShadow->getDebugNode()->setVisible(_enabled);
    }
    if (_sensor != nullptr){
        _sensor->setEnabled(_enabled);
        _sensor->getDebugNode()->setVisible(_enabled);
    }
}

void GameObject::syncPositions(){
    Vec2 pos = _position;
    if (_collider != nullptr){
        pos = _collider->getPosition();
        _position = pos - _colliderOffset;
    }
    if (_colliderShadow != nullptr){
        _colliderShadow->setPosition(pos);
    }
    if (_sensor != nullptr){
        _sensor->setPosition(pos + _sensorOffset);
    }
}

void GameObject::addObstaclesToWorld(std::shared_ptr<physics2::ObstacleWorld> world){
    if (_collider != nullptr){
        world->addObstacle(_collider);
        _collider->getBody()->GetUserData().pointer = reinterpret_cast<intptr_t>(this);
        _colliderOffset.set(_collider->getPosition() - _position);
    }
    if (_colliderShadow != nullptr){
        world->addObstacle(_colliderShadow);
        _colliderShadow->getBody()->GetUserData().pointer = reinterpret_cast<intptr_t>(this);
    }
    if (_sensor != nullptr){
        world->addObstacle(_sensor);
        _sensor->getBody()->GetUserData().pointer = reinterpret_cast<intptr_t>(this);
        _sensorOffset.set(_sensor->getPosition() - _collider->getPosition());
    }
}

void GameObject::removeObstaclesFromWorld(std::shared_ptr<physics2::ObstacleWorld> world){
    if (_collider != nullptr){
        world->removeObstacle(_collider);
    }
    if (_colliderShadow != nullptr){
        world->removeObstacle(_colliderShadow);
    }
    if (_sensor != nullptr){
        world->removeObstacle(_sensor);
    }
}

void GameObject::setDebugNode(const std::shared_ptr<scene2::SceneNode> &debugNode){
    if (_collider != nullptr){
        _collider->setDebugScene(debugNode);
    }
    if (_colliderShadow != nullptr){
        _colliderShadow->setDebugScene(debugNode);
    }
    if (_sensor != nullptr){
        _sensor->setDebugScene(debugNode);
    }
}
