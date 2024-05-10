//
//  Upgradeable.cpp
//  RS
//
//  Created by Dasha Griffiths on 3/26/24.
//

#include "Upgradeable.hpp"

Upgradeable::Upgradeable(float initialVal, float maxValue, int type){
    _currLevel = 0;
    _type = type;
    _tierValues[_currLevel] = initialVal;
    float stepAmt = (maxValue-initialVal)/5;
    for (int idx = 1; idx < 6; idx++) {
        _tierValues[idx] = _tierValues[idx-1]+stepAmt;
    }
}
Upgradeable::Upgradeable(float tiers[6], int type){
    _currLevel = 0;
    _type = type;
    _tierValues[_currLevel] = tiers[_currLevel];
    for (int idx = 1; idx < 6; idx++) {
        _tierValues[idx] = tiers[idx];
    }
}

void Upgradeable::levelUp(){
    if (_currLevel<=5){
        _currLevel+=1;
    }
}

void Upgradeable::levelDown(){
    if (_currLevel>0) {
        _currLevel-=1;
    }
}

void Upgradeable::resetUpgrade(){
    _currLevel = 0;
}
