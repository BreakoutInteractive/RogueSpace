//
//  Hitbox.cpp
//  RS
//
//  Created by Zhiyuan Chen on 5/14/24.
//

#include "Hitbox.hpp"

void Hitbox::setEnabled(bool value){
    WheelObstacle::setEnabled(value);
    _hitSet.clear(); // clear the flags
}

bool Hitbox::hits(intptr_t ptr, float angle){
    if (_hitSet.find(ptr) != _hitSet.end()){
        return false;
    }
    if (!hitsAngle(angle)){
        return false;
    }
    _hitSet.insert(ptr);
    return true;
}

bool SemiCircleHitbox::hitsAngle(float ang){
    // hitbox still a circle but only hit in semicircle
    float hitboxAngle = getAngle();
    // for some reason the enemy hitbox angles have values ranging from -PI to PI :/
    // this fixes the range to be 0 to 2*PI
    if (hitboxAngle < 0) {
        hitboxAngle = 2 * M_PI + hitboxAngle;
    }
    return abs(ang - hitboxAngle) <= M_PI_2 || abs(ang - hitboxAngle) >= 3 * M_PI_2;
}
