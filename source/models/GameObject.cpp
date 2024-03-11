//
//  GameObject.cpp
//  RS
//
//  Created by Zhiyuan Chen on 3/11/24.
//

#include "GameObject.hpp"


void GameObject::fixedUpdate(){
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
